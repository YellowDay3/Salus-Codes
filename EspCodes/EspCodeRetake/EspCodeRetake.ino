#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <StreamString.h>
#include <string.h>

//#include <SoftwareSerial.h> // to commucate with the arduino                 //Removing this to use hardware serial instead of software serial

/*
  IMPORTANT : 
      To make a succesful connection with the arduino, you need to use a 
      
         __                __  ______   _ _____         
        / /  ___ _  _____ / / / _/ /  () / /___ ____
       / // -) |/ / -) / \ \/ _ \/ / _/ __/ -) __/    -levelshifter: is a device which converts
      //\/|/\// /////// \/\//                 a 3.3v to 5v and 3.3v to 5v.                                      
                                                         
*/

#ifndef STASSID
#define STASSID "head magnet"               //The name of the wifi network
#define STAPSK "huntsman"                   //The password of the wifi network
#endif

#define RXD0 D6 //Don't change this pins, it is to connect to levelshifter to the rx and tx of the arduino.
#define TXD0 D7

// SoftwareSerial Serial(RXD0,TXD0);

String arduinoResponse;

const char *ssid = STASSID;
const char *password = STAPSK;

//HardwareSerial Serial(2);

ESP8266WebServer server(80);

String getResponse() {
  if (Serial.available() > 0) {
    arduinoResponse = Serial.readStringUntil('\n'); //Stop at the return to the line aka stop at Serial.println()
    arduinoResponse.trim(); //Remove any trailing whitespace
    return arduinoResponse;
  } else {
    return "";
  }
}

void sendResponse() {
  if (Serial.available()) {
    
  }
}

void setup(void) {
  //Serial.setRxBufferSize(RXD0); //Setting the RX beforehand because you can't do it in begin.
  //Serial.begin(9600,SERIAL_8N1,SERIAL_FULL,TXD0); //The channel which the esp communicate with the arduino. Plus using SERIAL_FULl due using RX and TX.
  Serial.begin(9600);
  //Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("");
  Serial.print("Searching for a connection");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("Connection Successfull!");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  if (MDNS.begin("esp8266")) { Serial.println("MDNS responder started"); }

  server.on("/", []() {  //Root, why the hell he is here?
    server.send(200, "text/plain", "Root"); 
    Serial.println("The Root has been called.");
  });

  server.on("/check", []() { //The App checks if the server target exists.
    server.send(200, "text/plain", "240");
    Serial.println("App detected!");
  });

  server.on("/isindanger", []() { //The App checks if the server target have detected an accident.
    Serial.println("is the user in danger?"); //The server target sends to the arduino to see if the car is in danger.
    if (getResponse()=="yes") { //This means that the arduino have detected the danger and the server target sends it to the app to proceed with the countdown.
      server.send(200, "text/plain", "yes");
      Serial.println("Danger detected and sent!");
    } else { //This means that the arduino have not detected the danger and the server target sends no which means nothing detected.
      server.send(200, "text/plain", "no");
      Serial.println("Nope, there is nothing, still proceeding to find the danger.");
    }
    //Serial.println("App detected!");
  });

  server.on("/abort", []() { //The server target receive the command, sends it to the arduino to stop the timer.
    //Serial.println("Abort Received, sending abortion...");
    Serial.println("abort the thing"); //THe server target sends to the arduino to abort the timer to send the thing.
    if (getResponse()=="aborted") { //If the arduino have aborted successfully.
      server.send(200, "text/plain", "aborted");
      Serial.println("Abort succesfull!");
    } else { //If the arduino failed to abort, retry again...
      server.send(200, "text/plain", "error");
      Serial.println("Abort failed! Try again...");
    }
  });

  server.on("/timerended", []() { //The App sends that it have ended it timer.
    server.send(200, "text/plain", "good");
    Serial.println("finished");
    //sendTimerended();
    Serial.println("Timer ended! Sending the smg via arduino.");
  });

  server.onNotFound([]() { //Bad routing.
    server.send(404, "text/plain", "error"); 
    Serial.println("Error! Wrong direction.");
  });

  server.begin();
  Serial.println("HTTP server started");
}

void loop(void) {
  server.handleClient();
  MDNS.update();
}