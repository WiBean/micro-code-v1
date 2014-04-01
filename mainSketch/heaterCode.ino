


// HEATER RELAY is LOW_ACTIVE
int const HEATER_RELAY_PIN = 8;
// THERISTOR is HIGH_ACTIVE
int const THERISTOR_PIN = 9;

//float const SAFETY_TEMPERATURE_ABSOLUTE_MAXIMUM_CELSIUS = 150.0;
float const SAFETY_TEMPERATURE_ABSOLUTE_MAXIMUM_CELSIUS = 100.0;
float const GOAL_TEMPERATURE_MAXIMUM_CELSIUS = 100.0;
float const GOAL_TEMPERATURE_MINIMUM_CELSIUS = 30.0;

float const SAFETY_GOAL_TEMPERATURE_CELSIUS = -50.0;
float const HEATER_PRECUT_MARGIN_CELSIUS = 5.0;
float const HEATER_RELAY_CUT_MARGIN = 0.8;

bool _requestHeatingOn = false;
float _goalTempInCelsius = 0;
float _goalTempCutInCelsius = 0;
float _heaterRelayCutCelsius = 0;


void setupHeaterPins(void)
{
  pinMode(HEATER_RELAY_PIN, OUTPUT);
  pinMode(THERISTOR_PIN, OUTPUT);
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

bool startHeatingLoopForTemperature(float tempInCelsius)
{
  if( (tempInCelsius <= SAFETY_TEMPERATURE_ABSOLUTE_MAXIMUM_CELSIUS) && (tempInCelsius >= GOAL_TEMPERATURE_MINIMUM_CELSIUS) && (tempInCelsius <= GOAL_TEMPERATURE_MAXIMUM_CELSIUS) ) {
    _requestHeatingOn = true;
    _goalTempInCelsius = tempInCelsius;
    _goalTempCutInCelsius = _goalTempInCelsius - HEATER_PRECUT_MARGIN_CELSIUS;
    _heaterRelayCutCelsius = _goalTempCutInCelsius * HEATER_RELAY_CUT_MARGIN;
    Serial.print(F("_goalTempInCelsius: ")); Serial.println(_goalTempInCelsius);
    Serial.print(F("_goalTempCutInCelsius: ")); Serial.println(_goalTempCutInCelsius);
    Serial.print(F("_heaterRelayCutCelsius: ")); Serial.println(_heaterRelayCutCelsius);
    return true;
  }
  else {
    _requestHeatingOn = false;
    return false;
  }
};
void stopHeatingLoop(void)
{
  startHeatingLoopForTemperature(SAFETY_GOAL_TEMPERATURE_CELSIUS);
  _requestHeatingOn = false;
};
void heatingLoop(void)
{
  float const currentTempInCelsius = readThermometerInCelsius();
  if( _requestHeatingOn && (currentTempInCelsius < SAFETY_TEMPERATURE_ABSOLUTE_MAXIMUM_CELSIUS) ) {
    if( _requestPumpOn ) {
      // we want to turn heat on because water going through head will quickly kill heat
      enableHeat();
      return;
    }
    // if we actually need to heat
    if( currentTempInCelsius < _goalTempCutInCelsius ) {
      // as the main heater is controlled by a relay, we can't switch it all the time.
      // so to save some duty cycle on the theristor, we use the main relay to get to a basic
      // "operating" temp, and then use the theristor to drive to the goal temperature
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
  }
  // if we get here, stop
  disableHeat();
  return;
}
