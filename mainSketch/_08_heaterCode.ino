/*
// This file contains the heating related code.
// It functions off the basic loop heatingLoop().  Start with
// this code.
// Functionality of note:
// - Logic controlled use of the relay OR thyristor for heating
// - Absolute safety temperature cutoff
// - The heat is activated while pumping as this rapidly removes
// heat from the block.

*/


// HEATER RELAY is LOW_ACTIVE
int const HEATER_RELAY_PIN = 8;
// THERISTOR is HIGH_ACTIVE
int const THERISTOR_PIN = 9;

//float const SAFETY_TEMPERATURE_ABSOLUTE_MAXIMUM_CELSIUS = 150.0;
float const SAFETY_TEMPERATURE_ABSOLUTE_MAXIMUM_CELSIUS = 100.0;
float const GOAL_TEMPERATURE_MAXIMUM_CELSIUS = 100.0;
float const SAFETY_GOAL_TEMPERATURE_CELSIUS = -30.0;

float const HEATER_PRECUT_MARGIN_CELSIUS = 2.0;
float const HEATER_RELAY_CUT_MARGIN = 0.8;

bool _requestHeatingControl = false;
float _goalTempInCelsius = 0;
float _goalTempCutInCelsius = 0;
float _heaterRelayCutCelsius = 0;


void setupHeaterPins(void)
{
  pinMode(HEATER_RELAY_PIN, OUTPUT);
  pinMode(THERISTOR_PIN, OUTPUT);
};


bool isHeating(void)
{
  return _requestHeatingControl;
};
void disableHeaterRelay(void)
{
  digitalWrite(HEATER_RELAY_PIN, HIGH); // HEATER_RELAY IS LOW_ACTIVE, IT IS WEIRD
};
void enableHeaterRelay(void)
{
  digitalWrite(HEATER_RELAY_PIN, LOW); // HEATER_RELAY IS LOW_ACTIVE, IT IS WEIRD
};
void disableTheristor(void)
{
  digitalWrite(THERISTOR_PIN, LOW);
};
void enableTheristor(void)
{
  digitalWrite(THERISTOR_PIN, HIGH);
};

void disableHeat(void)
{
  disableHeaterRelay();
  disableTheristor();
  
};
void enableHeat(void)
{
  enableHeaterRelay();
  enableTheristor();
};
// This method might be a bit counter intuitive.  When the unit is first powered
// on, we need to enable the relay and disable the theristor.  This is because
// when the relay is ON the user can use the machine like normal (with the buttons).
// When the relay is off, the whole machine turns off.  This behavior would thus
// allow the user to continue to use his machine the 'old fashioned' way even if
// our hardware breaks.
void setHeatInitialState(void)
{
  enableHeaterRelay();
  disableTheristor();
};


bool startHeatingLoopForTemperature(float tempInCelsius)
{
  if( (tempInCelsius <= SAFETY_TEMPERATURE_ABSOLUTE_MAXIMUM_CELSIUS) && (tempInCelsius >= SAFETY_GOAL_TEMPERATURE_CELSIUS) && (tempInCelsius <= GOAL_TEMPERATURE_MAXIMUM_CELSIUS) ) {
    _requestHeatingControl = true;
    _goalTempInCelsius = tempInCelsius;
    _goalTempCutInCelsius = _goalTempInCelsius - HEATER_PRECUT_MARGIN_CELSIUS;
    _heaterRelayCutCelsius = _goalTempCutInCelsius * HEATER_RELAY_CUT_MARGIN;
    Serial.print(F("_goalTempInCelsius: ")); Serial.println(_goalTempInCelsius);
    Serial.print(F("_goalTempCutInCelsius: ")); Serial.println(_goalTempCutInCelsius);
    Serial.print(F("_heaterRelayCutCelsius: ")); Serial.println(_heaterRelayCutCelsius);
    return true;
  }
  else {
    _requestHeatingControl = false;
    return false;
  }
};
void stopHeatingLoop(bool maintainControl)
{
  startHeatingLoopForTemperature(SAFETY_GOAL_TEMPERATURE_CELSIUS);
  _requestHeatingControl = maintainControl;
  Serial.print(F("Stop heating, maintain control: "));
  Serial.println(_requestHeatingControl);
};
void heatingLoop(void)
{
  float const currentTempInCelsius = readThermometerInCelsius();
  if( _requestHeatingControl ) {
    if( currentTempInCelsius < SAFETY_TEMPERATURE_ABSOLUTE_MAXIMUM_CELSIUS ) {
      // if we need to heat, or are pumping (water going through head will quickly kill heat)
      if( _requestPumpOn || (currentTempInCelsius < _goalTempCutInCelsius) ) {
        // as the main heater is controlled by a relay, we can't switch it all the time.
        // so to save some duty cycle on the thyristor, we use the main relay to get to a basic
        // "operating" temp, and then use the thyristor to drive to the goal temperature
        if( currentTempInCelsius < _heaterRelayCutCelsius ) {
          enableHeaterRelay();
          disableTheristor();
          return;
        }
        else {
          disableHeaterRelay();
          enableTheristor();
          return;
        }
      }
      else {
        disableHeat();
        return;
      }
    }
    else {
      disableHeat();
      return;
    }
  }
  //if we get here, enable default state (machine buttons work as normal)
  setHeatInitialState();
  return;
}
