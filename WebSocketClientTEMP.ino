/*
 * WebSocketClient.ino
 *
 *  Created on: 24.05.2015
 *
 */

#include <Arduino.h>
#include <Bounce2.h>
#include <ESP8266WiFi.h>
#include <WebSocketsClient.h>
#include <Hash.h>
#include <TM1637Display.h>

#define CLK 4
#define DIO 5
String str;
WebSocketsClient webSocket;
TM1637Display display(CLK, DIO);
Bounce bouncer = Bounce();

const char* ssid     = "motoyarkoRC";         // The SSID (name) of the Wi-Fi network you want to connect to
const char* password = "1q2w3e4r5t6y";     // The password of the Wi-Fi network

boolean displayMode = true; //true-tmp, false-rpm, 
int tempAlarm = 124; // temp for starting alarm
long buzzerTimer = 0;
int buttonPin = 13;
int buzzerPin = 14;
long lastMsg = 0;
int tmp = 0;
int rpm = 0;
//String messageFromServer;
int inPin = 5;
int inPin2 = 4;
int hallStatus = 0;



  const uint8_t blank[] = { 0x00, 0x00, 0x00, 0x00 };
  const uint8_t celsius[] = { SEG_A | SEG_F | SEG_E | SEG_D};
  const uint8_t err[] = {
  SEG_A | SEG_F | SEG_G | SEG_E | SEG_D,           // E
  SEG_G | SEG_E,   // r
  SEG_G | SEG_E    // r
  };

void webSocketEvent(WStype_t type, uint8_t * payload, size_t length) {

	switch(type) {
		case WStype_DISCONNECTED:
			Serial.printf("[WSc] Disconnected!\n");
			break;
		case WStype_CONNECTED: {
			Serial.printf("[WSc] Connected to url: %s\n", payload);

			// send message to server when Connected
			webSocket.sendTXT("Connected_messageFromClient");
		}
			break;
		case WStype_TEXT:
			Serial.printf("[WSc] get text: %s\n", payload); // parse the message from server. r==rpm; t==temp. update int variables for temp and rpm
      if (payload[0] == 't') {
        String tempString = ((char*)payload);
        Serial.println("tempString:"+tempString); //debug
        tmp = tempString.substring(1).toInt();
        Serial.println("tmp:"+tmp); //debug
      }
     else Serial.println(" IF #1 doesn't work");
     if (payload[0] == 'r') {
        String rpmString = ((char*)payload);
        rpm = rpmString.substring(1).toInt();
      }
     else Serial.println(" IF #2 doesn't work"); 

			// send message to server
			// webSocket.sendTXT("message here");
			break;
		case WStype_BIN:
			Serial.printf("[WSc] get binary length: %u\n", length);
			hexdump(payload, length);

			// send data to server
			// webSocket.sendBIN(payload, length);
			break;
	}

}

void setup() {
  pinMode(buttonPin, INPUT_PULLUP);
  bouncer.attach(buttonPin);
  bouncer .interval(5);
  pinMode(buzzerPin, OUTPUT);
  display.setBrightness(0x0f);
  display.setSegments(blank);
  
	Serial.begin(115200);         // Start the Serial communication to send messages to the computer
  delay(100);
  Serial.println('\n');

  WiFi.mode(WIFI_STA);  //disable AP
WiFi.softAPdisconnect(true);
  
  WiFi.begin(ssid, password);             // Connect to the network
  Serial.print("Connecting to ");
  Serial.print(ssid); Serial.println(" ...");

  int i = 0;
  while (WiFi.status() != WL_CONNECTED) { // Wait for the Wi-Fi to connect
    delay(1000);
    Serial.print(++i); Serial.print(' ');
    display.showNumberDec(i, false, 4, 0);
  }

  Serial.println('\n');
  Serial.println("Connection established!");  
  Serial.print("IP address:\t");
  Serial.println(WiFi.localIP());         // Send the IP address of the ESP8266 to the computer
  Serial.print("Default gateway:\t");

String gateway = WiFi.gatewayIP().toString();
 
  Serial.println(gateway);

	// server address, port and URL
	webSocket.begin(gateway, 81, "/");

	// event handler
	webSocket.onEvent(webSocketEvent);

	// use HTTP Basic Authorization this is optional remove if not needed
	//webSocket.setAuthorization("motoyarko", "0675744496");

	// try ever 5000 again if connection has failed
	webSocket.setReconnectInterval(5000);
}

void loop() {
	webSocket.loop();
  unsigned long now = millis();
if (WiFi.status() != WL_CONNECTED) {
    display.setSegments(err, 4, 0);
  } else {
  if (bouncer.update())
    { //если произошло событие
      if (bouncer.read()==0)
        { //если кнопка нажата
          displayMode = ! displayMode;
          lastMsg = 0;
        }
    }
    
   if ((now - lastMsg > 1000) && (displayMode == true)) {
    lastMsg = now;   
    display.showNumberDec(tmp, false, 3, 0);
    display.setSegments(celsius, 1, 3);
  }
  if ((now - lastMsg > 1000) && (displayMode == false)){
    lastMsg = now;
    display.showNumberDec(rpm, false, 4, 0);
    }
 
  if (tmp >= tempAlarm) {
    if (now - buzzerTimer > 2000) {
      buzzerTimer = now;
      tone(buzzerPin, 2000, 1000);  
    }
    
  } else noTone(buzzerPin);

  } 
}
