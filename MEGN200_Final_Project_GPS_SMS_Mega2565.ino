/*
Titus Smith, Henry Wrothert
CSM MEGN200 Autonomus RC Plane
12/11/2023
https://www.youtube.com/channel/UC8rHDX951rChz6TMxnTbdBA

This program runs on arduino Mega2565 and runs GPS and SMS recieving code and relays that info back to the main arduino nano fligt computer
*/

#include <TinyGPSPlus.h> //https://github.com/mikalhart/TinyGPSPlus
//Note we must use arduino mega as it has multiple built in hardware serial pins.
//We cannot just use Software Serial because it only allows for listening on one Serial port at a time and we want to listen on two ports and broadcast on another


TinyGPSPlus gps;  // the TinyGPS++ object

bool readingmsg = false;// SMS input stuff
char sms = ' '; 
String latitude;
String longitude;

float targetGPS[] = { 0.00, 0.00 };  //Variables to hold the target and current gps cordinates once updated
float currentGPS[] = { 0.00, 0.00 };


void setup() {
    //Begin serial communication with Arduino and SIM800L
  Serial.begin(9600);
  Serial1.begin(9600);
  Serial2.begin(9600);
  delay(1000);
  // Begin Startup sequence with SIM800L SMS module
  Serial2.println("AT");  //Once the handshake test is successful, it will back to OK
  updateSerial();
  Serial2.println("AT+CSQ");  //Signal quality test, value range is 0-31 , 31 is the best
  updateSerial();
  Serial2.println("AT+CCID");  //Read SIM information to confirm whether the SIM is plugged
  updateSerial();
  Serial2.println("AT+CREG?");  //Check whether it has registered in the network
  updateSerial();

  Serial2.println("AT+CMGF=1");  // Configuring TEXT mode
  updateSerial();
  Serial2.println("AT+CNMI=1,2,0,0,0");  // Decides how newly arrived SMS messages should be handled
  updateSerial();
}

void loop() {
  // put your main code here, to run repeatedly:
  if (Serial1.available()) { // If gps date is availible
    if (gps.encode(Serial1.read())) {
      if (gps.location.isValid()) {  // check if the data is valid
        currentGPS[0] = gps.location.lat();
        currentGPS[1] = gps.location.lng();  // stor ethe longitude
        
        // Send updated values to the arduino Nano over Serial using custom Serial packet protocol
        Serial.print("A");
        Serial.print(currentGPS[0], 7);
        Serial.print("B");
        Serial.print(currentGPS[1], 7);
        Serial.print("C");
        Serial.print(targetGPS[0], 7);
        Serial.print("D");
        Serial.println(targetGPS[1], 7);
        
      }
    }
  }

  updateSerial();// Check if there is Data from the SMS board
}

void updateSerial() {

  if (Serial2.available()) {
    //Serial.write(Serial2.read()); // Echo the data to serial  ** Testing only**
    char sms = Serial2.read();
    
    // Note the reason we send and recieve the lat and lng separately is that that is the only length of message that we could get working reliably

    //Read lattitude if the message starts and ends with @ and &
    if (sms == '@') {

      readingmsg = true;

    } else if (sms == '&') {
      readingmsg = false;
      if (latitude != "") {
        float num = latitude.toFloat(); // Parse input to float
        if (38 < num && 41 > num) {// Make sure that the data is valid and save it   ***Change this check if you are not in Colorado ***
          targetGPS[0] = num;
        }
      }

      latitude = "";
      longitude = "";
    } else if (readingmsg == true) {
      latitude += sms;
    }

    //Read Longitude if message starts with ; and ends with !
    if (sms == ';') {

      readingmsg = true;

    } else if (sms == '!') {
      readingmsg = false;
      //Serial.println(longitude);
      if (longitude != "") {
        float num = longitude.toFloat(); // Parse input to float
        if (-110 < num && -100 > num) { // Make sure that the data is valid and save it ****Change the check if you are not in Colorado****
          targetGPS[1] = num;
        }
      }

      longitude = "";
      latitude = "";
    } else if (readingmsg == true) {
      longitude += sms;
    }
  }
}
