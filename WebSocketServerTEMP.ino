/*
 * WebSocketServer.ino
 *
 *  Created on: 22.05.2015
 *
 */

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WebSocketsServer.h>
#include <Hash.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#define ONE_WIRE_BUS 5
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
WebSocketsServer webSocket = WebSocketsServer(81);


const char* ssid     = "motoyarkoRC";         // The SSID (name) of the Wi-Fi network you want to connect to
const char* password = "1q2w3e4r5t6y";     // The password of the Wi-Fi network


long lastMsg = 0;
float temp = 0;
int pinTemp = 5;
int pinHall = 4;
int hallStatus = 0;

volatile byte interruptCounter = 0;
int numberOfInterrupts = 0;

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {

    switch(type) {
        case WStype_DISCONNECTED:
            Serial.printf("[%u] Disconnected!\n", num);
            break;
        case WStype_CONNECTED:
            {
                IPAddress ip = webSocket.remoteIP(num);
                Serial.printf("[%u] Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);
				
				// send message to client
				webSocket.sendTXT(num, "Connected_MessageFromServer");
            }
            break;
        case WStype_TEXT:
            Serial.printf("[%u] get Text: %s\n", num, payload);

            // send message to client
            // webSocket.sendTXT(num, "message here");

            // send data to all connected clients
            //webSocket.broadcastTXT("message here");
            break;
        case WStype_BIN:
            Serial.printf("[%u] get binary length: %u\n", num, length);
            hexdump(payload, length);

            // send message to client
            // webSocket.sendBIN(num, payload, length);
            break;
    }

}

void setup() {
    pinMode(pinTemp, INPUT);
    pinMode(pinHall, INPUT_PULLUP);
// debug RPM
//    pinMode(14, OUTPUT);
//    tone(14, 2000, 3600000);
//end debug    
    attachInterrupt(digitalPinToInterrupt(pinHall), handleInterrupt, FALLING);
    sensors.begin();
    sensors.setResolution(9);
    Serial.begin(115200);

    //Serial.setDebugOutput(true);
    Serial.setDebugOutput(true);

    Serial.println();
    Serial.println();
    Serial.println();

    for(uint8_t t = 4; t > 0; t--) {
        Serial.printf("[SETUP] BOOT WAIT %d...\n", t);
        Serial.flush();
        delay(1000);
    }

    WiFi.softAP(ssid, password);

    webSocket.begin();
    webSocket.onEvent(webSocketEvent);    
}

void handleInterrupt() {
  interruptCounter++;
}

void loop() {
  webSocket.loop();

  if(interruptCounter>0){
 
      interruptCounter--;
      numberOfInterrupts++;
 //debug
     // Serial.print("An interrupt has occurred. Total: ");
     // Serial.println(numberOfInterrupts);
  }

  long now = millis();
    if (now - lastMsg >= 1000) {
      lastMsg = now;
    
      sensors.requestTemperatures(); // Send the command to get temperatures
      temp = sensors.getTempCByIndex(0);
//      Serial.print("TEMP:");
//      Serial.println(temp);
//      Serial.print("RPM :");
//      Serial.println(numberOfInterrupts*60/1000);
int rpm = numberOfInterrupts*60/1000;

      webSocket.broadcastTXT("t"+String((int)temp));
      webSocket.broadcastTXT("r"+String(rpm).substring(0, 4));
      numberOfInterrupts = 0;
    }
}

