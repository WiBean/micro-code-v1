WiBean micro-controller overview
=========================

Components
----------

Heater
------
The heater circuit controls a relay which heats the head of the espresso machine.  This requires the temperature probe is calibrated.  By default, the heating circuit is switched on, which means the button on the machine control all functionality and the machine will heat AS NORMAL.  If the user requests the WiBean to heat to a given temperature, or if the user has a wake-up timer set, the heating circuit will take full control of the machine.

When the heating circuit has control the following behavior applies.  The heater will switch off if the temperature probe detects a temperature in excess of 150 degrees celsius at any time.  Then, if the temperature detected is below a threshold temperature, the circuit will heat with the relay.  Once the relay cutoff temperature is reached, the final heating regime and ongoing regulation are performed via a theristor.  Finally, heat will be on (via the channel described in the temperature rules above) if the pump is running.

```c++
bool turnOff(void)
```
Returns true on success, false on failure or uncertainty.

```c++
bool driveToTemperatureInCelsius(int temperatureInCelsius)
```
This command begins an open loop which attempts to drive the temperature sensor to the desired temperature.  It operates by a two part heating logic.  If the requested temperature is below a lower threshold, heating is applied via a relay.  Otherwise, heating is applied via theristor.

Returns true if process has begun, false on failure or uncertainty.

```c++
bool isHeating(void)
```
Returns true if the open loop is running, false if not.

Water Pump
-----------
The pump circuit controls the water pump which drives water from the tank through the espresso head.  It has a hard-coded limit which will not allow it to run for more than 30 seconds continuously.

```c++
void resetPump(void)
```
Resets pump loop logic and disables pump;

```c++
void disableWaterPump(void)
```
Disables pump via GPIO signal.

Temperature Sensor
------------------
The temperature sensor is designed to read the temperature of the heat.  The temperature sensor is non-linear and read out via a lookup table.
The temperature sensor is read as:
```math
degreesInCelsius = lookupTableSearch(currentReadingInRawUnits);
```

```c++
float getTemperatureInCelsius(void)
```
Returns temperature, as calibrated by hard-coded lookup table, in degrees Celsius.


GPIO Lines
-----------
1. Relay control closing built-in heater circuit.  Automatically opens at fixed temperature of 85C (check?) - LOW ACTIVE
2. Theriostor control for driving secondary heating stage.  Max safe duty cycle 70% - HIGH ACTIVE
3. Pump switch - HIGH ACTIVE
4. Valve -- does what???, HIGH ACTIVE

Input Lines (ADC)
----------------
1. Thermister on block
