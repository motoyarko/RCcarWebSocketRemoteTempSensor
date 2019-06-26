#include <ESP8266WiFi.h>
//#include <PubSubClient.h>
#include <OneWire.h>
#include <DallasTemperature.h>

// Data wire is plugged into pin 2 on the Arduino
#define ONE_WIRE_BUS 5

// Setup a oneWire instance to communicate with any OneWire devices 
// (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);

DallasTemperature sensors(&oneWire);

// Update these with values suitable for your network.
//const char* ssid = "motoyarko";
//const char* password = "1q2w3e4r5t6y";
//const char* mqtt_server = "192.168.1.103";
//const char* mqtt_server_login = "motoyarko";
//const char* mqtt_server_password = "0675744496";
//WiFiClient espClient;
//PubSubClient client(espClient);
long lastMsg = 0;
float temp = 0;
int inPin = 5;
int inPin2 = 4;
int hallStatus = 0;


void setup()
{
  Serial.begin(115200);
//  setup_wifi(); 
//  client.setServer(mqtt_server, 1883);
  pinMode(inPin, INPUT);
  pinMode(inPin2, INPUT_PULLUP);
  sensors.begin();
}

void loop()
{


  long now = millis();
  if (now - lastMsg > 1000) {
    lastMsg = now;
    hallStatus = digitalRead(inPin2);
    sensors.setResolution(9);
    sensors.requestTemperatures(); // Send the command to get temperatures
    temp = sensors.getTempCByIndex(0);
    Serial.println(temp);
    Serial.println(hallStatus);
    
    
  }
}
