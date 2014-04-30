/*
// This file contains control code for the pumping circuits
//
// The basic pump routine walks through 5 on and off cycles.
// Each cycle can last a maximum of XYZ seconds (practical limit?).
// Cycles defined with a length of 0 complete instantly and execution
// moves to the next cycle-step.
//
// NOTE:
// - NUM_PUMP_STEPS is defined in the mainSketch
*/

unsigned long _onTimes[MAX_PUMP_STEPS];
unsigned long _offTimes[MAX_PUMP_STEPS];
int const STEP_MIN_VALUE = 5;
int const STEP_MAX_VALUE = 300;
int const MILLIS_PER_STEP = 100;
// WATER PUMP IS HIGH_ACTIVE
int const WATER_PUMP_PIN = 10;

bool _requestPumpOn = false;

void setupPumpPins(void)
{
  pinMode(WATER_PUMP_PIN, OUTPUT);
};
void resetPump(void)
{
  for(int k=0;k<MAX_PUMP_STEPS;++k) {
    _onTimes[k] = 0;
    _offTimes[k] = 0;
  }
  _requestPumpOn = false;
  disableWaterPump();
}
void disableWaterPump(void)
{
  digitalWrite(WATER_PUMP_PIN, LOW);
};
void enableWaterPump(void)
{
  digitalWrite(WATER_PUMP_PIN, HIGH);
};

bool prepCycleProgram(int numSteps, int onFor[], int offFor[])
{
  if( numSteps <= 0 ) {
    return false;
  }
  numSteps = min(numSteps, MAX_PUMP_STEPS);
  bool success = true;
  for(int k=0;k<numSteps;++k) {
    success &= stepValid(onFor[k]);
    success &= stepValid(offFor[k]);
  }
  Serial.print("prepCycle validated: ");
  Serial.println(success);
  //printArray(MAX_PUMP_STEPS, onFor);
  //printArray(MAX_PUMP_STEPS, offFor);
  if( success ) {
    unsigned long tNow = millis();
    int const waitBuffer = 100; // time to wait in ms before starting execution

    int k=0;
    _onTimes[k] = tNow + waitBuffer;
    _offTimes[k] = _onTimes[k] + onFor[k]*MILLIS_PER_STEP;
    ++k;
    for(;k<numSteps;++k) {
      _onTimes[k] = _offTimes[k-1] + offFor[k-1]*MILLIS_PER_STEP;      
      _offTimes[k] = _onTimes[k] + onFor[k]*MILLIS_PER_STEP;
    }
    for(;k<MAX_PUMP_STEPS;++k) {
      _onTimes[k] = 0;
      _offTimes[k] = 0;
    }
    printArray(MAX_PUMP_STEPS, _onTimes);
    printArray(MAX_PUMP_STEPS, _offTimes);
    return true;
  }
  else {
    return false;
  }
};
bool stepValid(int const& value)
{
  if( (value != 0) && ((value > STEP_MAX_VALUE) || (value < STEP_MIN_VALUE)) ) {
    //Serial.print("stepValid FALSE: ");
    //Serial.println(value);
    return false;
  }
  else {
    //Serial.print("stepValid TRUE: ");
    //Serial.println(value);
    return true;
  }
};


void pumpLoop(void)
{
  unsigned long currTime = millis();
  for(int k=0;k<MAX_PUMP_STEPS;++k) {
    if( (currTime >= _onTimes[k]) && (currTime < _offTimes[k]) ) {
      if(!_requestPumpOn) {
        Serial.print(F("Enable pump at: "));
        Serial.println(millis());
      }
      _requestPumpOn = true;
      enableWaterPump();
      return;
    }
  }
  if(_requestPumpOn) {
    Serial.print(F("Disable pump at: "));
    Serial.println(millis());
  }
  _requestPumpOn = false;
  disableWaterPump();
};
