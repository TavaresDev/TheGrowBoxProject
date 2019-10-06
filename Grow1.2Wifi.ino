/*************************************************
      The Grow Box Project
      
      by Andre Tavares


      using code created by Brea Parker to connect to wifi module.
**************************************************/

/************************************************
      ENTER WIFI ACCESS POINT CREDENTIALS
*************************************************/
String wifiUser = "Andre";
String wifiPass = "12345678";

/************************************************
         ENTER THING SPEAK CREDENTIALS

*************************************************/
String writeAPIKey = "K7SAZDHR7NJJQRUF";
String readAPIKey = "NFR8OG4HGQ1MS6HB";
String channel = "845795";
/*********************************************/

// LCD libraries
#include <Wire.h>
#include <LCD.h>
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);

// Pins
const int mSensorPin = A0;
const int ldrSensorPin = A1;
const int relayLamp = 12; //in1
const int relayPump = 11; //in2

// global variables
int gMoisture;
int gLdr;

// wifi variables

char readData[6] = "000000";   //Used to store data read from Thing Speak.
int RGBRedPin = 2;    //Stores value of the outlet pin number.
bool waiting = false;   //Used to check if a function is currently executing.

#define TIMER   500    //Global value for milliseconds to wait between actions.
#define DST_IP    "api.thingspeak.com"    //Address to connect to.



////////////////SETUP////////////////
void setup() {
  
  Serial.begin(9600);   //Start the serial for the Arduino.
  Serial1.begin(115200);    //Start the serial for the ESP8266.
  pinMode(RGBRedPin, OUTPUT);   //Setup the red LED pin as an output pin.

  delay(100);
  clearBuffer();    //Empty the buffer before starting.
  wifiConnect(wifiUser,wifiPass);   //Connect to the wifi.

  //Read values
    pinMode(mSensorPin, INPUT);
    pinMode(ldrSensorPin, INPUT);
//    pinMode(tSensor, INPUT);

   // Output controls
    pinMode(relayLamp, OUTPUT);
    pinMode(relayPump, OUTPUT) ;
   //start LCD
   lcd.begin(16,2);
  
}

////////////// MAIN LOOP //////////////

void loop() {

    moistureFunction();
    ldrFunction();
    lcdFunction();
    pumpFunction();
    lampFunction();
   
  
   
    
    sendWriteRequest(1,gLdr);
    sendWriteRequest(2,gMoisture);
    delay(500);
    
}

void moistureFunction() {
     /*    moisture variable control

     dryAirValue = 630 to 640
     waterValue = 300 to 310 
     Takes about 30sec to normalize the value
    persentage 0% = air dry, 100% = water

     Moist Soil  = 380 = 78%
    */
    int mSensor = analogRead(mSensorPin);
    int moisturePercent = map(mSensor, 300, 650, 100, 0);
    gMoisture = moisturePercent;
    

    //  Serial.println(val); //print the value to serial port
     Serial.print("Moisture Sensor: ");
     Serial.print(mSensor); 
     
     Serial.print(" "); 
     Serial.print(moisturePercent); //print the value to serial port
     Serial.println("%"); 
     delay(1000);
}

void ldrFunction() {
    /*    moisture variable control

    IndoorNormal Ligth = 430 to 440
    tipFinger < 130
    Takes about 5 sec to normalize the value

    normal bulb 9 w about 20 cm = 971 256%

    persentage 0% = air dry, 100% = water

    */
    int lSensor = analogRead(ldrSensorPin);
    int ldrPercent = map(lSensor, 130, 450, 0, 100);
    gLdr = ldrPercent;

    Serial.print("LDR Sensor: ");
    Serial.print(lSensor); //print the value to serial port
    Serial.print(" "); 
    Serial.print(ldrPercent); //print the value to serial port
    Serial.println("%"); 
    delay(1000);
}
void lcdFunction() {
  lcd.clear();
  
  lcd.setCursor(0,0);
  lcd.print("Moisture: ");
  lcd.print(gMoisture); 
  lcd.print("%");
  lcd.setCursor(0,1);
  lcd.print("LDR: ");
  lcd.print( gLdr);
  lcd.print("%");
}
void pumpFunction() {
  
//   digitalWrite(relayPump, LOW); //open
  
  // normaly open  == OFF
   if( gMoisture >  30) { // its oposit i dont know why
      digitalWrite(relayPump, HIGH); //closed = ON
      delay(1000);
      digitalWrite(relayPump, LOW); //open
      delay(2000); 
  } else {
      digitalWrite(relayPump, LOW); //open
  }
// delay(10000); //10 sec for security
}
void lampFunction() {

   if( gLdr >  20) {
      digitalWrite(relayLamp, HIGH);
      delay(1000);
  } else {
      digitalWrite(relayLamp, LOW);
  }
   delay(5000); //security, dnt burn another relay

}



/* Code adapdet from class*/
/* I culdnt make the library from think speak work, so im using this code*/
/************************************************************************************************************************
* Program Name: SMART Outlet                                                                                            *
* Description: Using the ESP8266, connect to ThingSpeak.com, read the latest value from field 1, if a 1 or 2 is found,  *    
*              this signifies a new request from the user. A 1 will be a manual command and a 2 will be a timer command.*
*              The arduino will turn the outlet power ON or OFF based on the command requested.                         *
* Instructions: Enter your wifi credentials into the space below. Then enter your Thing Speak channel and API keys into *
*               the following spaces. Make sure your Thing Speak channel has fields 1, 2, and 3 available and that you  *
*               also have the SMART Outlet app installed on your phone and the Smart Outlet hardware wired up to the    *
*               Arduino. Commands are sent through the app. Internet connection strength can affect the arduino's       *
*               reliability. If you're having trouble, try increasing the TIMER variable.                               *                                                              *
* Author: Brea Parker                                                                                                   *
* Last Edit: 10/23/18                                                                                                   *
************************************************************************************************************************/

////////////// WIFI FUNCTIONS//////////////

//Used to connect to your entered wifi credentials.
bool wifiConnect(String user, String pass){
  String cmd = "AT+CWJAP=\""; cmd += user; cmd += "\",\""; cmd += pass; cmd += "\"";    //Builds serial command for wifi connection.
  cmdExecute(cmd, 8);   //Send built command to the command executor function. Integer value is a multiplier for the global wait time.
  delay(100);   //Wait briefly before continuing.
  while(waiting);   //Make sure any previous functions have finished executing before continuing. (Maybe not necessary. Was having issues with serial commands getting mixed up so it stays for now)
}

//Used to connect to Thing Speak.
void connectTCP(){
  String cmd = "AT+CIPSTART=\"TCP\",\""; cmd += DST_IP; cmd += "\",80";   //Builds serial command for TCP connection to Thing Speak.
  cmdExecute(cmd, 1);
  delay(100);
  while(waiting);
}

//Pre request send. Sends how large the following request will be.
void startRequest(int data){
  String cmd = "AT+CIPSEND="; cmd += data;
  cmdExecute(cmd, 1);
  delay(100);
  while(waiting);
}

//Builds and sends a request to write the specified value to the specified field.
void sendWriteRequest(byte field, int value){
  connectTCP();   //Connection to Thing Speak is established first.
  delay(100);
  while(waiting);
  //Build the request
  String cmd = "GET /update?api_key="; cmd += writeAPIKey+"&field1=0&field"; cmd += field; cmd += "="; cmd += value; cmd += " HTTP/1.1\r\n"; cmd += "Host: api.thingspeak.com\n"; cmd += "Connection: closed\r\n\r\n";
  startRequest(cmd.length());   //Start the request using the size of the request to be sent.
  delay(100);
  while(waiting);
  cmdExecute(cmd, 1);   //Send the request.
  delay(100);
  while(waiting);
}

//Builds and sends a request to read the most recent value from a specified field.
void sendReadRequest(byte field){
  connectTCP();   //Connection to Thing Speak is established first.
  delay(100);
  while(waiting);
  //Build the request
  String cmd = "GET /channels/"; cmd+= channel; cmd += "/fields/"; cmd += field; cmd += "/last.json?api_key="; cmd += readAPIKey; cmd += " HTTP/1.1\r\n"; cmd += "Host: api.thingspeak.com\n"; cmd += "Connection: closed\r\n\r\n";
  startRequest(cmd.length());   //Start the request using the size of the request to be sent.
  delay(100);
  while(waiting);
  pullData(cmd, field, 1);    //Send the request and pull out the value of the field.
  delay(100);
  while(waiting);
}

//Executes commands while displaying all information to the serial monitor.
void cmdExecute(String cmd, int mult){
  clearBuffer();    //Write out any characters left in the buffer.
  waiting = true;   //Command is executing; set waiting to true to prevent early execution of other functions.
  unsigned long deadline = millis() + TIMER * mult;   //Set the deadline for characters to be found.
  Serial1.println(cmd);   //Execute the command.
  while(millis() < deadline){   //Continues to execute as long as the deadline has not been passed.
    if(Serial1.available() > 0){    //Only true if there are characters to be read in the serial monitor.
      Serial.write(Serial1.read());   //Write the available characters to the serial monitor.
      deadline = millis() + TIMER * mult;   //Since a character was found, reset the deadline.
    }
  }
  waiting = false;    //Execution is finished so other functions can proceed.
}

//Executes commands while displaying all information to the serial monitor and saving the relevant values. (Can probably merge with cmdExecute eventually)
void pullData(String cmd, int field, int mult){
  byte matchCount = 0;    //Used to track how many of the key characters have been matched in succession.
  byte dataCount = 0;   //Used to increment the character index of readData to store multiple values.
  char storage = "0";   //Temporary storage for the current character being read.
  String key = "\"field"; key += field; key += "\":\"";   //Field data will always follow this string. Used to isolate the value of the field.
  
  clearBuffer();
  waiting = true;
  unsigned long deadline = millis() + TIMER * mult;
  Serial1.println(cmd);
  while(millis() < deadline){
    if(Serial1.available() > 0){
      storage = Serial1.read();   //Store the current character into storage.
      Serial.write(storage);    //Write the character in storage to the serial monitor.
      //Check if the character in storage is equal to the character in the key at the current number of matches found. If so, increase the number of matches found. If not, reset the matches found.
      if(storage == key[matchCount]){
        matchCount++;
      }
      else{
        matchCount = 0;
      }
      deadline = millis() + TIMER * mult;   //Reset the deadline since a character was found.
      while(matchCount == key.length()){    //Start this execution once we have matched the enitre key.
        if(Serial1.available() > 0){    //Proceed if there are characters available.
          storage = Serial1.read();
          Serial.write(storage);
          if(storage != 34){    //If the value being read isn't 34 (or " in ascii) add the value to the readData and increment the index of readData for any additional values.
            readData[dataCount] = storage;
            dataCount++;
          }
          else{   //If the value is 34 it signifies the end of the field data. Reset the match counter and clean out the buffer.
            matchCount = 0;
            clearBuffer();
          }
        }
      }
    }
  }
//  Serial.print("HERE IS THE DATA: ");   //Prints the read value to the serial monitor. (Not necessary, just for your own visuals)
//  Serial.print(readData[0]);
//  Serial.print(readData[1]);
//  Serial.print(readData[2]);
//  Serial.print(readData[3]);
//  Serial.print(readData[4]);
//  Serial.println(readData[5]);
  waiting = false;
}

//Used to read and write out any characters that may be left in the buffer.
void clearBuffer(){
  waiting = true;
  unsigned long deadline = millis() + TIMER;
  while(millis() < deadline){
    if(Serial1.available() > 0){
      Serial.write(Serial1.read());
      deadline = millis() + TIMER;
    }
  }
  waiting = false;
}

//Checks for updates on Thing Speak and acts appropriately based on the data received.
void checkForUpdate(){
  sendReadRequest(1);
  if(readData[0] - 48 == 0){
    digitalWrite(RGBRedPin, LOW);
    delay(1000);
    sendWriteRequest(1,1);
  }
  else if(readData[0] - 48 == 1){
    digitalWrite(RGBRedPin, HIGH);
    delay(1000);
    sendWriteRequest(1,0);
  }
}
