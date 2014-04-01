/*  Base library for the WiBean microcontroller

This code controls 4 things via GPIO.
1. Built-in heater relay
2. Secondary theristor circuit
3. Water Pump
4. Mystery Valve

/*
  Modified Arduino Yun Bridge example

 This example for the Arduino Yun shows how to use the
 Bridge library to access the digital and analog pins
 on the board through REST calls. It demonstrates how
 you can create your own API when using REST style
 calls through the browser.

 Possible commands created in this shetch:

 * "/arduino/digital/13"     -> digitalRead(13)
 * "/arduino/digital/13/1"   -> digitalWrite(13, HIGH)
 * "/arduino/analog/2/123"   -> analogWrite(2, 123)
 * "/arduino/analog/2"       -> analogRead(2)
 * "/arduino/mode/13/input"  -> pinMode(13, INPUT)
 * "/arduino/mode/13/output" -> pinMode(13, OUTPUT)

 This example code is part of the public domain

 http://arduino.cc/en/Tutorial/Bridge

 */

#include <Bridge.h>
#include <YunServer.h>
#include <YunClient.h>

//#include <heaterCode.ino>
//#include <thermistorCode.ino>

// Listen on default port 5555, the webserver on the Yun
// will forward there all the HTTP requests for us.
YunServer server;



// WATER PUMP IS HIGH_ACTIVE
int const WATER_PUMP_PIN = 10;
// VALVE IS HIGH_ACTIVE
int const VALVE_CONTROL_PIN = 11;


bool _requestPumpOn = false;

void setup() {
  // Console
  Serial.begin(9600);
  
  // Bridge startup
  pinMode(13, OUTPUT);
  digitalWrite(13, LOW);
  Bridge.begin();
  digitalWrite(13, HIGH);
  
  setupPins();
  everythingOff();
  
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
  
  heatingLoop();
  delay(100); // Poll every 50ms
};

void setupPins(void)
{
  setupHeaterPins();
  pinMode(WATER_PUMP_PIN, OUTPUT);
  pinMode(VALVE_CONTROL_PIN, OUTPUT);
};

void disableWaterPump(void)
{
  digitalWrite(WATER_PUMP_PIN, LOW);
};
void enableWaterPump(void)
{
  digitalWrite(WATER_PUMP_PIN, HIGH);
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
  stopHeatingLoop();
  disableHeat();
  disableWaterPump();
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


void process(YunClient client) {
  // read the command
  String command = client.readStringUntil('/');
  String lower = command; lower.toLowerCase();
  if( (lower == "stop") || (lower == "off") ) {
    everythingOff();
  }
  else if( command == "thermometer" ) {
    readAndDisplayThermometer(client);
  }
  else if( command == "heat" ) {
    heatCommand(client);
  }
  else if( command == "cycleProgram" ) {
    //cycleProgram(client);
  }
  // is "digital" command?
  else if (command == "digital") {
    digitalCommand(client);
  }
  // is "analog" command?
  else if (command == "analog") {
    analogCommand(client);
  }
  // is "mode" command?
  else if (command == "mode") {
    modeCommand(client);
  }
  else {
    client.print(F( "Unknown command: "));
    client.println(command);
  }
};


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
  else {
    value = digitalRead(pin);
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
    value = client.parseInt();
    analogWrite(pin, value);

    // Send feedback to client
    client.print(F("Pin D"));
    client.print(pin);
    client.print(F(" set to analog "));
    client.println(value);

    // Update datastore key with the current pin value
    String key = "D";
    key += pin;
    Bridge.put(key, String(value));
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

void modeCommand(YunClient client) {
  int pin;

  // Read pin number
  pin = client.parseInt();

  // If the next character is not a '/' we have a malformed URL
  if (client.read() != '/') {
    client.println(F("error"));
    return;
  }

  String mode = client.readStringUntil('\r');

  if (mode == "input") {
    pinMode(pin, INPUT);
    // Send feedback to client
    client.print(F("Pin D"));
    client.print(pin);
    client.print(F(" configured as INPUT!"));
    return;
  }

  if (mode == "output") {
    pinMode(pin, OUTPUT);
    // Send feedback to client
    client.print(F("Pin D"));
    client.print(pin);
    client.print(F(" configured as OUTPUT!"));
    return;
  }

  client.print(F("error: invalid mode "));
  client.print(mode);
}


