
#include <avr/pgmspace.h>


// Thermistor calibration settings.
int const THERMISTOR_ADC_PIN = 0;

PROGMEM prog_int16_t const inputs[] =
{ 1020,1019,1016,1013,1008,1002,993,983,
969,952,930,905,874,839,800,756,708,658,606,554,503,453,406,362,321,
284,251,221,194,171,150,132,117,103,
 91, 80, 71, 63, 56, 50, 45};

PROGMEM prog_int16_t const outputs[] =
{  -40, -35, -30, -25, -20, -15,-10, -5,
  0,  5, 10, 15, 20, 25, 30, 35, 40, 45, 50, 55, 60, 65, 70, 75, 80,
 85, 90, 95,100,105,110,115,120,125,
130,135,140,145,150,155,160};

const int LUT_SIZE = 41;

// this does a linear search, requires inputs and outputs have same size!
float linearSearch(int const*const inputs, int const*const outputs, int input)
{
  //Serial.print(F("linSearch input: "));
  //Serial.println(input);
  float first, second;
  if( pgm_read_word(&inputs[0]) < input ) {
    Serial.print(F("linSearch LOW_BOUND output: "));
    Serial.println(pgm_read_word(&outputs[0]));
    return pgm_read_word(&outputs[0]);
  }
  for(int k=1;k<LUT_SIZE;++k)
  {
    if( pgm_read_word(&inputs[k]) == input ) {
      //Serial.print(F("linSearch k: "));
      //Serial.print(k);
      //Serial.print(F(" output: "));
      //Serial.println(pgm_read_word(&outputs[k-1]));
      return pgm_read_word(&outputs[k]);
    }
    else if( pgm_read_word(&inputs[k]) < input ) {
      // Dumb Return
      //Serial.print(F("linSearch k: ")); Serial.print(k);
      //Serial.print(F(" output: ")); Serial.println(pgm_read_word(&outputs[k-1]));
      
      // Linear Interp
      float const inputDelta = (int)pgm_read_word(&inputs[k]) - (int)pgm_read_word(&inputs[k-1]);
      float const outputDelta = (int)pgm_read_word(&outputs[k]) - (int)pgm_read_word(&outputs[k-1]);
      float const slope = outputDelta / inputDelta;
      float const result = (int)pgm_read_word(&outputs[k-1]) + slope* (float)(input - (int)(pgm_read_word(&inputs[k-1])) );
      //Serial.print(F("linSearch k: ")); Serial.print(k);
      //Serial.print(F(" inputDelta: ")); Serial.print(inputDelta);
      //Serial.print(F(" outuptDelta: ")); Serial.print(outputDelta);
      //Serial.print(F(" slope: ")); Serial.print(slope);
      //Serial.print(F(" result: ")); Serial.println(result);
      return result;
    }
  }
  Serial.print(F("linSearch end: "));
  Serial.println(pgm_read_word(&outputs[LUT_SIZE-1]));
  return pgm_read_word(&outputs[LUT_SIZE-1]);
};

float readThermometerInCelsius()
{
  int value = analogRead(THERMISTOR_ADC_PIN);
  return linearSearch(inputs, outputs, value);
};
