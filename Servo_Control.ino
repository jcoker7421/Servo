#include "UbidotsEsp32Mqtt.h"
/****************************************
 * Define Constants
 ****************************************/
const char *UBIDOTS_TOKEN = "BBFF-FRoz6ZRKA09lqmZcUMMBqM0u59cJZE";  // Put here your Ubidots TOKEN
const char *WIFI_SSID = "3156 Rosser St";      // Put here your Wi-Fi SSID
const char *WIFI_PASS = "Coker3156";      // Put here your Wi-Fi password
const char *DEVICE_LABEL = "esp32";   // Put here your Device label to which data  will be published
const char *VARIABLE_LABEL = "relaypin"; // Put here your Variable label to which data  will be published
const char *BATTERY1_LABEL = "battery1";
const char *BATTERY2_LABEL = "battery2";
const char *PAYLOAD_CONFIRM = "payload-delivery";
const char *MQTT_CONFIRM = "mqtt-connection";

const int PUBLISH_FREQUENCY = 5000; // Update rate in milliseconds
int relayPin = 25;
int h1Pin = 26;
int h2Pin = 27;
unsigned long timer;

Ubidots ubidots(UBIDOTS_TOKEN);

/****************************************
 * Auxiliar Functions
 ****************************************/
void servo(String message) {
  int value = 0; //message set to 0
  if (message == "0.0") {
    digitalWrite(relayPin, LOW);
  } else {
    digitalWrite(relayPin, HIGH); //turn relay on
    delay(500);
    digitalWrite(h1Pin, HIGH); //set servo forward
    digitalWrite(h2Pin, LOW);
    delay(7000);
    digitalWrite(h1Pin, LOW); //leave valve open
    delay(180000);
    digitalWrite(h2Pin, HIGH); //reverse servo/close valve
    delay(8500);
    digitalWrite(h2Pin, LOW);
    digitalWrite(relayPin, LOW);
    value = 1;
    ubidots.add(PAYLOAD_CONFIRM, value); //send confirmation message
    ubidots.publish(DEVICE_LABEL);
    Serial.println("SERVO SUCCESS... SENDING CONFIRMATION");
    delay(10000);
    value = 0;
    ubidots.add(PAYLOAD_CONFIRM, value);
    ubidots.publish(DEVICE_LABEL);
  }
}

void callback(char *topic, byte *payload, unsigned int length)
{
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++)
  {
    Serial.print((char)payload[i]);
  }
  char p[length + 1];
  memcpy(p, payload, length);
  p[length] = NULL;
  String message(p);
  servo(message);

  Serial.println();
}

/****************************************
 * Main Functions
 ****************************************/

void setup()
{
  pinMode(relayPin, OUTPUT);
  pinMode(h1Pin, OUTPUT);
  pinMode(h2Pin, OUTPUT);
  // put your setup code here, to run once:
  Serial.begin(115200);
  ubidots.setDebug(true);  // uncomment this to make debug messages available
  ubidots.connectToWifi(WIFI_SSID, WIFI_PASS);
  ubidots.setCallback(callback);
  ubidots.setup();
  ubidots.reconnect();
  ubidots.subscribeLastValue(DEVICE_LABEL, VARIABLE_LABEL);
  timer = millis();
}

void loop()
{
  // put your main code here, to run repeatedly:
  if (!ubidots.connected())
  {
    ubidots.reconnect();
    ubidots.subscribeLastValue(DEVICE_LABEL, VARIABLE_LABEL);
  }
  if (millis() - timer > PUBLISH_FREQUENCY) // triggers the routine every 5 seconds
  {
    float battery1Level = map(analogRead(32), 0.0f, 4095.0f, 0, 100);
    ubidots.add(BATTERY1_LABEL, battery1Level); // Insert your variable Labels and the value to be sent
    float battery2Level = map(analogRead(35), 0.0f, 4095.0f, 0, 100);
    ubidots.add(BATTERY2_LABEL, battery2Level); // Insert your variable Labels and the value to be sent
    ubidots.publish(DEVICE_LABEL);
    timer = millis();
  }
  ubidots.loop();
}