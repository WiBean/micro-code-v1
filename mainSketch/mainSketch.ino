/*  Base library for the WiBean microcontroller

This code controls 4 things via GPIO.
1. Built-in heater relay
2. Secondary theristor circuit
3. Water Pump
4. Mystery Valve


 Possible commands created in this sketch:
 * "/arduino/digital/13/1"   -> digitalWrite(13, HIGH)
 * "/arduino/analog/2"       -> analogRead(2)

 * "/arduino/thermometer/0"  -> Reads and writes thermo to output (any argument as the 0 will do)
 * "/arduino/heat/34.5"      -> Will turn the heat on and maintain the temperature at 34.5C.
                                This command returns the current thermometer temp in C on success.
                                On error, undefined return.
 * "/arduino/stop/0"
 * "/arduino/off/0"          -> Both of these commands will turn everything off, including heat and
                                pump.  The argument given after the stop or off can be anything.
                                Returns "STOP REQUESTED!" on success.
 * "/arduino/reset/0"        -> resets to default state.  This means the relay is CLOSED, and the
                                theristor is off.  Heating control is off.
 * "/arduino/isControllingHeat/0"
                             -> will respond with boolean 1 or 0 whether the heating control
                                loop is active.  Argument after the action word is discarded.
                                
 * "/arduino/pump/on1/off1/on2/off2/on3/off3/onN/offN"
                             -> This command will turn the pump on and off in the order given above.
                                Time is given in units of 100ms.  So a value of 10 -> 1000ms.
                                Minimum steps is a single ON time.
                                Maximum on/off pairs is 5 (10 arguments).
                                Minimum time value is 2.
                                Maximum time value is 300.
                                If any value is non-conforming, the program will not execute.
                                See the pumpControl file for more information.
* "/arduino/version/0"       -> This command returns the current version of the microcontroller
                                firmware.  This command can be used to check if the unit is alive
                                and ready for commands.

 */

#include <Bridge.h>
#include <YunServer.h>
#include <YunClient.h>

//#include <heaterCode.ino>
//#include <thermistorCode.ino>

// Listen on default port 5555, the webserver on the Yun
// will forward there all the HTTP requests for us.
YunServer server;

int const MAX_PUMP_STEPS = 5;


// VALVE IS HIGH_ACTIVE
int const VALVE_CONTROL_PIN = 11;


// UTILITIES
void printArray(YunClient & client, short numel, int* array)
{
  client.print('[');
  for(short k=0;k<numel-1;++k) {
    client.print(array[k]);
    client.print(", ");
  }
  client.print(array[numel-1]);
  client.println("]");
}

void printArray(short numel, int* array)
{
  Serial.print('[');
  for(short k=0;k<numel-1;++k) {
    Serial.print(array[k]);
    Serial.print(", ");
  }
  Serial.print(array[numel-1]);
  Serial.println("]");
}
void printArray(short numel, unsigned long* array)
{
  Serial.print('[');
  for(short k=0;k<numel-1;++k) {
    Serial.print(array[k]);
    Serial.print(", ");
  }
  Serial.print(array[numel-1]);
  Serial.println("]");
}



// CODE
// *****
void setup() {
  // Console
  Serial.begin(9600);
  // Bridge startup (13, is the LED)
  pinMode(13, OUTPUT);
  digitalWrite(13, LOW);
  Bridge.begin();
  digitalWrite(13, HIGH);
  
  setupPins();
  everythingOff();
  stopHeatingLoop(false);
  // Listen for incoming connection only from localhost
  // (no one from the external network could connect)
  server.listenOnLocalhost();
  server.begin();
}

void loop() {
  // Get clients coming from server
  YunClient client = server.accept();
  // There is a new client?
  if (client) {
    process(client);
    // Close connection and free resources.
    client.stop();
  }
  pumpLoop();
  heatingLoop();
  delay(50); // Poll every 50ms
};

void setupPins(void)
{
  setupHeaterPins();
  setupPumpPins();
  pinMode(VALVE_CONTROL_PIN, OUTPUT);
};

void closeValve(void)
{ 
  digitalWrite(VALVE_CONTROL_PIN, LOW);
};
void openValve(void)
{ 
  digitalWrite(VALVE_CONTROL_PIN, HIGH);
};

void everythingOff(void)
{
  stopHeatingLoop(true);
  resetPump();
  closeValve();
};


void readAndDisplayThermometer(YunClient client)
{
  client.print(F("thermometerTemperatureInCelsius: "));
  client.println( readThermometerInCelsius() );
};


void heatCommand(YunClient client)
{
  float tempInCelsius = client.parseFloat();
  Serial.print(F("requestedHeatTo: ")); Serial.println(tempInCelsius);
  if( startHeatingLoopForTemperature(tempInCelsius) ) {
    readAndDisplayThermometer(client); // return current thermo temp
  }
  else {
    client.print(F("Invalid heater command.  Requested temperature: "));
    client.println(tempInCelsius);
  }
};


bool takeIntIf(YunClient & client, int & dest)
{
  if (client.read() == '/') {
    dest = client.parseInt();
    return true;
  }
  else {
    return false;
  }
};


void pumpCommand(YunClient & client)
{
  int onFor[MAX_PUMP_STEPS] = {0,0,0,0,0};
  int offFor[MAX_PUMP_STEPS] = {0,0,0,0,0};
  
  short currentIndex = 0;
  onFor[currentIndex] = client.parseInt();
  if( takeIntIf(client, offFor[currentIndex]) ) {
    ++currentIndex;
    for(;currentIndex < MAX_PUMP_STEPS; ++currentIndex) {
      if( !takeIntIf(client, onFor[currentIndex]) || !takeIntIf(client, offFor[currentIndex]) ) {
        break;
      }
    }
  }
  printArray(client, MAX_PUMP_STEPS, onFor);
  printArray(client, MAX_PUMP_STEPS, offFor);
  prepCycleProgram(MAX_PUMP_STEPS, onFor, offFor);
};


void process(YunClient client) {
  // read the command
  String command = client.readStringUntil('/');
  String lower = command; lower.toLowerCase();
  if( (lower == "stop") || (lower == "off") ) {
    everythingOff();
    client.println(F("STOP REQUESTED!"));
  }
  else if( command == "reset" ) {
    everythingOff();
    stopHeatingLoop(false);
    client.println(F("RESET REQUESTED!"));
  }
  else if( command == "thermometer" ) {
    readAndDisplayThermometer(client);
  }
  else if( command == "heat" ) {
    heatCommand(client);
  }
  else if( command == "pump" ) {
    //cycleProgram(client);
    pumpCommand(client);
  }
  else if( command == "isControllingHeat" ) {
    client.print(isHeating());
  }
  else if( command == "version" ) {
    client.println(F("WiBean_v1_0"));
  }
  
  // ARDUINO STOCK FUNCTIONS
  // is "digital" command?
  else if (command == "digital") {
    digitalCommand(client);
  }
  // is "analog" command?
  else if (command == "analog") {
    analogCommand(client);
  }
  else {
    client.print(F( "Unknown command: "));
    client.println(command);
  }
};

// *******************************
// BEGIN STOCK ARDUINO FUNCTIONS
// *******************************

void digitalCommand(YunClient client) {
  int pin, value;
  // Read pin number
  pin = client.parseInt();
  // If the next character is a '/' it means we have an URL
  // with a value like: "/digital/13/1"
  if (client.read() == '/') {
    value = client.parseInt();
    digitalWrite(pin, value);
  }
  // Send feedback to client
  client.print(F("Pin D"));
  client.print(pin);
  client.print(F(" set to "));
  client.println(value);
  // Update datastore key with the current pin value
  String key = "D";
  key += pin;
  Bridge.put(key, String(value));
}

void analogCommand(YunClient client) {
  int pin, value;
  // Read pin number
  pin = client.parseInt();
  // If the next character is a '/' it means we have an URL
  // with a value like: "/analog/5/120"
  if (client.read() == '/') {
    // Read value and execute command
    /*
    Bridge.put(key, String(value));
    */
  }
  else {
    // Read analog pin
    value = analogRead(pin);
    // Send feedback to client
    client.print(F("Pin A"));
    client.print(pin);
    client.print(F(" reads analog "));
    client.println(value);

    // Update datastore key with the current pin value
    String key = "A";
    key += pin;
    Bridge.put(key, String(value));
  }
}

// ***************************
// END STOCK ARDUINO COMMANDS
// ***************************
