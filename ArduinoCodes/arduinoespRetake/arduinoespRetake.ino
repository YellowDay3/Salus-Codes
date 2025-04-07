#include <Arduino.h>
#include <SoftwareSerial.h>
 
//SoftwareSerial Serial(7,6);                Note: replacing the software serial with hardware serial
SoftwareSerial SIM900(10, 9); 
//TX de sim900 avec 10, Gyro : A5=SCL, A4=SDA
#include <MPU6050.h>
#include <Wire.h>
#include <I2Cdev.h>     
 
MPU6050 mpu;
int16_t ax, ay, az;
int16_t gx, gy, gz;
 
struct MyData {
  byte X;
  byte Y;
  byte Z;
};
 
MyData data;
 
void setup() {
  // put your setup code here, to run once:
  //Serial.begin(9600);
  Serial.begin(9600);                              // Note: changed baud rate to 9600 for more stable connection to the esp
  SIM900.begin(19200);
  delay(2000);
  Wire.begin();
  mpu.initialize();
  Serial.println("Salus is here!");
}
 
void loop() {
  // put your main code here, to run repeatedly:
  //geroscope
  mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);
  data.X = map(ax, -17000, 17000, 0, 255 ); 
  data.Y = map(ay, -17000, 17000, 0, 255); 
  data.Z = map(az, -17000, 17000, 0, 255);  
  delay(500);
//   Serial.print("Axis X = ");                         // Removing this :))
//   Serial.print(data.X);
//   Serial.print("  ");
//   Serial.print("Axis Y = ");
//   Serial.print(data.Y);
//   Serial.print("  ");
//   Serial.print("Axis Z  = ");
//   Serial.println(data.Z);
  if (Serial.available() > 0) {
    String received = Serial.readStringUntil('\n');                  //Still wanting to change it to char but let test it from now
    received.trim();
    Serial.println("Received from esp: " + received);
    if (received == "is the user in danger?") {
      if (data.X < 50 && data.Y > 30 && data.Y < 230 && data.Z < 101){       //Updating this due being impossible become true
        Serial.print("There is a problem");
        Serial.println("yes");}
      else {
        Serial.println("no");}
    } 
    else if (received =="abort the thing") {                        // Updating this due the abort SHOULD NOT send a sms
        //sendSMS();
        Serial.println("aborted");
        //exit(0);
    }
    else if (received =="finished") {                             // Adding the finshed command to stop the program
      sendSMS();
      Serial.println("done");
      exit(0);
    }
  }
}
 
void sendSMS(){
  // AT command to set SIM900 to SMS mode
  SIM900.print("AT+CMGF=1\r"); 
  delay(100);
 
  // REPLACE THE X's WITH THE RECIPIENT'S MOBILE NUMBER
  // USE INTERNATIONAL FORMAT CODE FOR MOBILE NUMBERS
  SIM900.println("AT+CMGS=\"+212642550919\""); 
  delay(100);
  
  // REPLACE WITH YOUR OWN SMS MESSAGE CONTENT
  SIM900.println("There is an accident! Please send help! "); 
  delay(100);
 
  // End AT command with a ^Z, ASCII code 26
  SIM900.println((char)26); 
  delay(100);
  SIM900.println();
  // Give module time to send SMS
  delay(13000); 
}