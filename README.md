WiBean micro-controller overview
=========================

Components
----------

Heater
------
The heater circuit controls a relay which heats the HEAD??? of the espresso machine.  This requires the temperature probe is calibrated.  The temperature circuit has an hard-coded limit of running for 123 minutes, after which it will be automatically switched off.  The heater circuit will also switch off if the probe? detects a temperature in excess of 123 degrees celsius or if the on-board temperature sensor temperatures goes above 123 degrees celsius.

```c++
bool turnOff(void)
```

Returns true on success, false on failure or uncertainty.

```c++
bool driveToTemperatureInCelsius(int temperatureInCelsius)
```
This command begins an open loop which attempts to drive the temperature sensor to the desired temperature.  It operates by...

Returns true if process has begun, false on failure or uncertainty.

```c++
bool isHeating(void)
```
Returns true if the open loop is running, false if not.

Water Pump
-----------
The pump circuit controls the water pump which drives water from the tank through the espresso head.  It has a hard-coded limit which will not allow it to run for more than 123 seconds continuously or at 123% duty cycle in a continuous running window.

```c++
bool turnOff(void)
```
Returns true on success false on failure or uncertainty

```c++
bool turnOnForMilliseconds(int milliseconds)
```
Returns true if process has begun, false on failure or uncertainty

Temperature Sensor
------------------
The temperature sensor is designed to read the temperature of the WATER_TANK???.  The temperature sensor is assumed to operate in a linear range, and must be calibrated before use.
The temperature sensor is read as:
```math
degreesInCelsius = currentReadingInRawUnits * calibrationSlope + calibrationOffset
```
When the code is initialized calibrationSlope is intialized to 1 and calibrationOffset is set to 0.

```c++
bool setCalibrationParameters(float slope, float offset)
```
Returns true if slope is within range: `floatMin <= slope <= floatMax` and offset is within the range: `floatMin <= offset <= floatMax`

Returns false otherwise.

```c++
float getTemperatureInCelsius(void)
```
User must ensure setCalibrationParameters has been called before this function, or return value is undefined.

Returns temperature, as calibrated, in degrees celsius.


GPIO Lines
-----------
1. Relay control closing built-in heater circuit.  Automatically opens at fixed temperature of 85C (check?) - LOW ACTIVE
2. Theriostor control for driving secondary heating stage.  Max safe duty cycle 70% - HIGH ACTIVE
3. Pump switch - HIGH ACTIVE
4. Valve -- seems important, HIGH ACTIVE

Input Lines (ADC)
----------------
1. Thermister on block (CALIBRATION DATA TO FOLLOW)
