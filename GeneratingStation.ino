#include <TinyGPS++.h>
TinyGPSPlus gps; // gps is an object of the TinyGPS++ library by Mikal Hart

/*
   lcd is an object of the LiquidCrystal_I2C library.
   It has 3 parameters, the i2c address, the number of columns and the number of rows respectively
*/
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 16, 2);

#include <SPI.h>
#include <SD.h>
// MISO - 50 || MOSI - 51 || CLK - 52 || CS - 53
File myFile;
int chipSelect = 53; //CS for Arduino Mega is pin 53 by default
String MY_SSID = "", MY_PWD = "", writeAPIKey = "";

/* The SD card will have 5 files in it. The files are as follows:

   1. logs.txt - Logs all data. Behaves like a black box
   2. SSID.txt - Stores the WiFi SSID. Unnecessary with regards to current needs.
   3. PWD.txt - Stores the WiFi password. Unnecessary with regards to current needs.
   4. PN.txt - Stores the phone number that the admin will use.
   5. WAK.txt - Stores the write API Key given by Thingspeak.com
   6. STAT.txt - To see if the system is stationary or not
*/

#include <SoftwareSerial.h>

// Configure software serial port
SoftwareSerial smsSerial(10, 11); //RX,TX
// RED PINS - DIGITAL
//D10 NEARER TO ANTENNA
int shouldGSM = 1; //if shouldGSM = 1, then data will be uploaded using GSM or else by using Wifi.

int intBuff; //Buffer integer value

String shutdownMessage = "shutdown"; //If the GSM recieves this message, then the station will shut down
String restartMessage = "restart"; //If the GSM recieves this message, then the station will start up
String phoneNumber = "+97335348601"; // This is the default number that will be considered as the admin number.
String smsMessage = ""; // Initialisation
int stationNumber = 200;

float totalPower = 0.0, totalEnergy = 0.0, avgPower = 0.0, totalInstCurrent = 0.0, totalInstVoltage = 0.0, totalInstPower = 0.0, temperature = 0.0, rpm = 0.0; // Initialisation
float samples = 1; //Used to calculate the average
float instCurrent = 0.0, instPower = 0.0, instVoltage = 0.0;
char buf_state = '\0'; //A buffer of character type. Will be used while choosing whether to get location using GPS or not

String outData = " ";
float counter = 1;
bool stationary = true; // If true, then the station is stationary, otherwise it is constantly moving
String Latitude = "26.1650783";
String Longitude = "50.5472988";
String currentTime;
char* uploadBuf = "";

int shutDownState = 0; // 1 if the station should shutdown, 0 otherwise

unsigned long prevTime;
unsigned long lastEnergyTime;
unsigned long uploadDelayTime = 5; //in minutes
unsigned long noLoadStartUpTime;

int shutDownPin = 6;
int ackPin = 29;
int VT_PIN = A1;
int AT_PIN = A0;
int tempPin = A2;
int loadPin = 9;
int motorRelay = 7;
int pwmMotor = 8;
int noLoadRelay = 30;
int switchRelay = 31;
int hallPin = 2;

float highThreshRPM = 150.0;
float midThreshRPM = 60.0;
float lowThreshRPM = 30.0;
float maxTemp = 2000.0;
float minThreshVoltage = 1.0;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial1.begin(9600);
  Serial2.begin(115200);
  serialFlushf();

  lcd.init();
  lcd.backlight();
  lcd.clear();
  // Print a message to the LCD.
  lcd.print("WELCOME.");
  delay(250);
  lcd.clear();
  lcd.print("WELCOME..");
  delay(250);
  lcd.clear();
  lcd.print("WELCOME...");
  delay(250);
  lcd.clear();
  lcd.print("WELCOME....");
  delay(250);
  lcd.clear();
  lcd.print("WELCOME.....");
  delay(250);
  lcd.clear();
  lcd.print("WELCOME......");
  delay(250);
  lcd.clear();
  lcd.print("WELCOME.......");
  delay(250);
  lcd.clear();
  lcd.print("WELCOME........");
  delay(250);
  lcd.clear();
  lcd.print("WELCOME.........");
  delay(250);
  lcd.clear();

  /*  while(!Serial.available())
    {Serial2.println(outData);}*/

  if (!SD.begin(chipSelect)) {
    lcd.clear();
    lcd.print("sd failed!");
    Serial.println("sd failed!");
    //startUpErrorShutDown();
    delay(1000);
  }
  else
  {
    lcd.clear();
    //lcd.println("sd reading!");
    readData(); //Reading data from the SD card

    serialFlushf();
    lcd.clear();
    lcd.print("sd done!");
    delay(1000);
  }

  smsSerial.begin(19200);
  delay(100);
  smsMessage += "Generating station number ";
  smsMessage += String(stationNumber);
  smsMessage += " was reset.";
  Serial.println(smsMessage);
  serialFlushf();


  sendSMS(); //Send initial SMS to the admin >>>>>>>>>>>>>> UNCOMMENT THIS ASAP

  smsSerial.print("AT+CMGF=1\r"); //Set it to SMS mode
  delay(100);
  // Set module to send SMS data to not receive any serial data
  smsSerial.print("AT+CNMI=0,0,0,0,0\r");
  delay(100);

  lcd.clear();
  lcd.println("sms done!");
  delay(100);

  pinMode(loadPin, OUTPUT);
  digitalWrite(loadPin, LOW); //NO LOAD

  pinMode(shutDownPin, OUTPUT);
  digitalWrite(shutDownPin, LOW); //NO JAMMING
  pinMode(noLoadRelay, OUTPUT);
  digitalWrite(noLoadRelay, LOW);

  pinMode(motorRelay, OUTPUT);
  digitalWrite(motorRelay, LOW);

  pinMode(switchRelay, OUTPUT); //For switching between shutdown and normal condition
  digitalWrite(switchRelay, LOW); //

  pinMode(pwmMotor, OUTPUT);
  digitalWrite(pwmMotor, LOW);

  pinMode(ackPin, INPUT);
  pinMode(VT_PIN, INPUT);
  pinMode(AT_PIN, INPUT);
  pinMode(tempPin, INPUT);
  pinMode(hallPin, INPUT);

  serialFlushf();

  //gradualStartUp();

  prevTime = millis();
  uploadDelayTime = uploadDelayTime * 60 * 1000;
  Serial.println("Setup done");
}

void loop() {
  // put your main code here, to run repeatedly:

  //checkRPM();
  intBuff = checkIfShutDown();
  if (intBuff == 0) //If 0 then we must connect the load completely
  {
    normalCondition();
  }

  if (intBuff == 1)
  {
    Serial.println("checkIfShutDown() == 1");
    digitalWrite(loadPin, LOW);
    digitalWrite(switchRelay, LOW);
    digitalWrite(motorRelay, LOW);
    totalShutDown();
  }
}

void normalCondition()
{
  Serial.println("Load connected completely"); //for debugging only
  analogWrite(loadPin, 32); //Connecting load completely
  digitalWrite(shutDownPin, LOW);
  serialFlushf();
  //checkRPM();

  DisplayData();
  Serial.print("DisplayData done"); //for debugging only
  serialFlushf();
  //checkRPM();

  sumAll();
  Serial.print("sumAll done"); //for debugging only
  serialFlushf();
  //checkRPM();

  writeData();
  Serial.print("writeData done"); //for debugging only
  serialFlushf();
  //checkRPM();

  if (millis() - prevTime > 5000) //uploadDelayTim
  {
    Serial.print("entered if");
    serialFlushf();
    //checkRPM();

    uploadData();
    serialFlushf();
    //checkRPM();

    Serial.print("uploadData done");
    serialFlushf();
    while (smsSerial.available() > 0)
      smsSerial.read();

    //checkRPM();

    prevTime = millis();
  }
  return;
}

void uploadData()
{
  serialFlushf();
  //checkRPM();
  Serial.println("Going to upload");
  if (stationary == false)
  {
    findLocation();
  }

  instCurrent = totalInstCurrent / counter; instVoltage = totalInstVoltage / counter; instPower = totalInstPower / counter;
  if (shouldGSM == 0)
  {

    outData = "< 0" + String(instCurrent) + " " + String(instVoltage) + " " + String(instPower) + " " + String(avgPower) + " " + String(totalEnergy) + " " + String(Latitude) + " " + String(Longitude) + String(rpm) + "#>";
    outData.toCharArray(uploadBuf, outData.length() + 1);
    /*Serial.println(uploadBuf);
      while (Serial.available() > 0)
      Serial.read();
      while (Serial2.available() > 0)
      Serial2.read();
      Serial2.write(uploadBuf);
      uploadBuf = "";*/
    Serial2.write(uploadBuf); //recheck this
    resetUploader();
    serialFlushf();
    //checkRPM();
  }

  else
  {
    uploadGSM();
    resetUploader();
    serialFlushf();
    //checkRPM();
  }

}

void serialFlushf()
{
  while (Serial.available() > 0)
    Serial.read();
  while (Serial1.available() > 0)
    Serial1.read();
  while (Serial2.available() > 0)
    Serial2.read();
  while (Serial3.available() > 0)
    Serial3.read();
}

void sendSMS()
{

  smsSerial.print("AT+CMGF=1\r");
  delay(100);

  String initialATmessage = "AT + CMGS = \"";
  initialATmessage += phoneNumber;
  initialATmessage += "\"";

  //initialATmessage = "AT + CMGS = \"+97336664765\"";
  smsSerial.println(initialATmessage);
  delay(100);

  // REPLACE WITH YOUR OWN SMS MESSAGE CONTENT
  smsSerial.println(smsMessage);
  delay(100);

  // End AT command with a ^Z, ASCII code 26
  smsSerial.println((char)26);
  delay(100);
  smsSerial.println();
  // Give module time to send SMS
  delay(5000);
  Serial.println(smsMessage);
  smsMessage = "";
}


int checkSMS()
{
  int smsFlag = 0;
  serialFlushf();
  // if(smsSerial.available()>0)
  // smsSerial.readString();

  smsSerial.print("AT+CNMI=2,2,0,0,0\r");
  if (smsSerial.available() > 0) {
    delay(100);
    String buffstr = smsSerial.readString();
    buffstr.toLowerCase();
    /*Serial.print(buffstr.length());*/ Serial.println(buffstr);
    if (buffstr.indexOf("cmt") != -1 )
    {
      if ((buffstr.indexOf(shutdownMessage) != -1 ) && (buffstr.indexOf(phoneNumber) != -1 ))
      {
        smsSerial.print("AT+CNMI=0,0,0,0,0\r");
        smsFlag = 2;
        serialFlushf();
        Serial.print("smsFlag = 2");
        Serial.println(buffstr); //debugging only
        return smsFlag; //shutdown
      }
      else if ((buffstr.indexOf(restartMessage) != -1 ) && (buffstr.indexOf(phoneNumber) != -1 ))
      {
        smsFlag = 1;
        smsSerial.print("AT+CNMI=0,0,0,0,0\r");
        serialFlushf();
        Serial.print("smsFlag = 1");
        Serial.println(buffstr); //debugging only
        return smsFlag; //restart
      }

      smsFlag = 0;
      serialFlushf();
      Serial.print("smsFlag = 0");
      smsSerial.print("AT+CNMI=0,0,0,0,0\r");
      //serialFlushf();
      //checkRPM();
      return smsFlag; //continue as it is
    }
    smsSerial.print("AT+CNMI=0,0,0,0,0\r");
    serialFlushf();
    //checkRPM();
    smsFlag = 0;
    return smsFlag;
  }
  smsSerial.print("AT+CNMI=0,0,0,0,0\r");
  serialFlushf();
  //checkRPM();
  smsFlag = 0;
  return smsFlag;
}

void readData()
{
  MY_SSID = "", MY_PWD = "", writeAPIKey = "";
  phoneNumber = "";

  serialFlushf();
  myFile = SD.open("SSID.txt"); //UOB-Events
  while (myFile.available()) {
    MY_SSID += String(char(myFile.read()));
  }
  myFile.close();
  //Serial.print(MY_SSID);

  myFile = SD.open("PWD.txt"); //uob.2013
  while (myFile.available()) {
    MY_PWD += String(char(myFile.read()));
  }
  myFile.close();
  //Serial.print(MY_PWD);

  myFile = SD.open("PN.txt"); //+97336664765
  while (myFile.available()) {
    phoneNumber += String(char(myFile.read()));
  }
  myFile.close();

  myFile = SD.open("WAK.txt"); // KFZQEEF487BI3NHD
  while (myFile.available()) {
    writeAPIKey += String(char(myFile.read()));
  }
  myFile.close();
  serialFlushf();
  //checkRPM();
  //Serial.print(writeAPIKey);


  myFile = SD.open("STAT.txt"); //y
  if (myFile.available()) {
    buf_state = char(myFile.read());
  }
  if (buf_state == 'y')
    stationary = true;
  else
    stationary = false;
  myFile.close();
  //Serial.print(buf_state);
  //lcd.print("sd done!");
  //return;

  //writeAPIKey = "KFZQEEF487BI3NHD";
  //Serial2.write(uploadBuf);
  /*outData = "<" + MY_SSID + "," + MY_PWD + "," + writeAPIKey + "#>";
    outData.toCharArray(uploadBuf, outData.length() + 1);
    outData = ""; MY_SSID = "";  MY_PWD = ""; // writeAPIKey = "";*/
  //Serial.print(uploadBuf);
  lcd.setCursor(0, 1);
  lcd.print("sd done");
  //Serial.print(uploadBuf);
  //while(Serial.available()>0)
  //Serial.read();
  serialFlushf();
  //checkRPM();
  //Serial.write(uploadBuf);
  return;
}


void writeData()  // For the SD module and WiFi
{
  outData = "";
  outData = String(instCurrent) + " " + String(instVoltage) + " " + String(instPower) + " " + String(avgPower) + " " + String(totalEnergy) + " " + String(rpm) + " " + String(Latitude) + " " + String(Longitude);

  myFile = SD.open("logs.txt", FILE_WRITE);

  // if the file opened okay, write to it:
  if (myFile) {
    myFile.println(outData);
    // close the file:
    myFile.close();
  } else {
    // if the file didn't open, print an error:
    // Serial2.println("error opening logs.txt");   // For debugging only. Comment it while running the program
  }
  outData = "";
  //Serial2.println(outData);
  serialFlushf();
  //checkRPM();
  return;
}

void findLocation()
{
  //double gpsStartTime = millis();
  String Date;
  String Time;
  String hh;
  String mm;
  String ss;
  int errorCounter = 1;

  while (errorCounter != 0)
    if (Serial1.available() > 0)
      if (gps.encode(Serial1.read()))
      {
        errorCounter = 0;
        if (gps.location.isValid())
        {
          Latitude = String(gps.location.lat(), 6);
          Longitude = String(gps.location.lng(), 6);
        }
        else
          errorCounter += 1;

        if (gps.date.isValid())
        {
          Date = String(gps.date.day()) + "/" + String(gps.date.month()) + "/" + String(gps.date.year());
        }

        else
          errorCounter += 1;
        if (gps.time.isValid())
        {
          if (gps.time.hour() < 10)
            hh = "0" + String(gps.time.hour());
          else
            hh = String(gps.time.hour());

          if (gps.time.minute() < 10)
            mm = "0" + String(gps.time.minute());
          else
            mm = String(gps.time.minute());

          if (gps.time.second() < 10)
            ss = "0" + String(gps.time.second());
          else
            ss = String(gps.time.second());
          Time = hh + ":" + mm + ":" + ss;
        }
        else
          errorCounter += 1;
      }

  currentTime = Date + " " + Time;
  myFile = SD.open("POS.txt", FILE_WRITE);

  outData = "Lat,Long,Time : " + Latitude + "," + Longitude +  + "," + currentTime;

  // if the file opened okay, write to it:
  if (myFile) {
    myFile.println(outData);
    // close the file:
    myFile.close();
  }

  serialFlushf();
  //checkRPM();
}

void DisplayData()
{
  getSamples();

  serialFlushf();
  Serial.println("Displaying data"); //For debugging

  //Instantaneous current
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Inst. Current(A)");
  lcd.setCursor(0, 1);
  lcd.print(instCurrent, 3);
  delay(3000);

  //Instantaneous Voltage
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Inst. Voltage(V)");
  lcd.setCursor(0, 1);
  lcd.print(instVoltage, 3);
  delay(3000);

  // Instantaneous power
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Inst. P (W)");
  lcd.setCursor(0, 1);
  lcd.print(instPower, 3);
  delay(3000);

  // Average Power
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Avg Power (W)");
  lcd.setCursor(0, 1);
  lcd.print(avgPower, 3);
  delay(3000);

  // Total Energy
  lcd.setCursor(0, 0);
  lcd.print("Total Energy(J)");
  lcd.setCursor(0, 1);
  lcd.print(totalEnergy, 3);
  delay(3000);

  // RPM
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Revolutions/min");
  lcd.setCursor(0, 1);
  lcd.print(rpm, 3);
  delay(3000);
  serialFlushf();

  return;
}

void getSamples()
{
  instVoltage = analogRead(VT_PIN) * (5.0 / 1023.0) * 5.0;

  instCurrent = analogRead(AT_PIN) * (5.0 / 1023.0);

  instPower = instVoltage * instCurrent;

  calcAvgPower();

  totalEnergy = totalEnergy + instPower * (millis() - lastEnergyTime) / 1000;

  lastEnergyTime = millis();

  rpm = getRPM(5.0);
  serialFlushf();

  return;
}

void sumAll()
{
  totalInstCurrent += instCurrent;
  totalInstVoltage += instVoltage;
  totalInstPower += instPower;

  counter++;
}

void calcAvgPower()
{
  totalPower += instPower;
  avgPower = totalPower / samples;
  samples = samples + 1.0;
}

void resetUploader()
{
  counter = 0.0;
  totalPower = 0.0;
  totalInstCurrent = 0.0;
  totalInstVoltage = 0.0;
  totalInstPower = 0.0;
}

float getRPM(float hallThresh)
{
  // preallocate values for tach
  float hallCount = 1.0;
  float startTime = micros();
  bool onState = false;
  // counting number of times the hall sensor is tripped
  // but without double counting during the same trip
  while (true) {
    if (digitalRead(hallPin) == 0) {
      if (onState == false) {
        onState = true;
        hallCount += 1.0;
      }
    } else {
      onState = false;
    }

    if (hallCount >= hallThresh) {
      break;
    }
  }

  // print information about Time and RPM
  float endTime = micros();
  float timePassed = ((endTime - startTime) / 1000000.0);
  rpm = (hallCount / timePassed) * 60.0;
  //Serial.print(rpm);
  //Serial.println(" RPM");
  delay(1);        // delay in between reads for stability

  return rpm;
}


void startUpErrorShutDown()
{
  smsMessage += "Generating station number ";
  smsMessage += String(stationNumber);
  smsMessage += " had an unsuccessful startup. Safety shutdown measures taken";
  Serial.println(smsMessage);
  sendSMS();
  digitalWrite(switchRelay, HIGH);
  for (intBuff = 0; intBuff < 255; intBuff++)
  {
    analogWrite(shutDownPin, intBuff);
    delay(1); //delay(100);
  }
  digitalWrite(shutDownPin, 255);
  delay(3000);
  Serial.println("Paused. Please restart later");
  while (checkSMS() != 1)
  {
    Serial.print(".");
    delay(10000);
  }
  smsMessage += "Generating station number ";
  smsMessage += String(stationNumber);
  smsMessage += " has restarted.";
  Serial.println(smsMessage);
  sendSMS();
  digitalWrite(switchRelay, LOW);
  digitalWrite(shutDownPin, LOW);
  noLoadStartUpTime = millis(); //newly added
  gradualStartUp();
  return;
}

float checkTemperature()
{
  temperature = 500.0 * analogRead(tempPin) / 1023.0;
  return temperature;
}

void uploadGSM()
{

  smsSerial.println("AT");
  delay(1000);
  // while (smsSerial.available() > 0)   // To check for responses. Necessary for debugging
  //   smsSerial.read();

  smsSerial.println("AT+CPIN?"); // AT+CPIN    Enter  PIN
  delay(1000);
  //while (smsSerial.available() > 0)
  //smsSerial.read();

  smsSerial.println("AT+CREG?"); //Network  Registration
  delay(1000);
  //while (smsSerial.available() > 0)
  //smsSerial.read();

  smsSerial.println("AT+CGATT?"); //Attach or Detach from GPRS Service
  delay(1000);
  //while (smsSerial.available() > 0)
  //smsSerial.read();

  smsSerial.println("AT+CIPSHUT"); //important //Deactivate  GPRS  PDP  Context
  delay(1000);
  //while (smsSerial.available() > 0)
  //smsSerial.read();

  smsSerial.println("AT+CIPSTATUS"); // Query Current Connection Status
  delay(2000);
  //while (smsSerial.available() > 0)
  //smsSerial.read();

  smsSerial.println("AT+CIPMUX=0"); //Start up single IP connection
  delay(2000);
  //while (smsSerial.available() > 0)
  //smsSerial.read();

  smsSerial.println("AT+CSTT=\"myAPN\""); // Start Task and Set APN
  delay(1000);
  //while (smsSerial.available() > 0)
  // smsSerial.read();

  smsSerial.println("AT+CIICR"); //Bring Up Wireless Connection with GPRS
  delay(3000);
  //while (smsSerial.available() > 0)
  // smsSerial.read();

  smsSerial.println("AT+CIFSR");//get local IP adress
  delay(2000);
  //while (smsSerial.available() > 0)
  // smsSerial.read();

  smsSerial.println("AT+CIPSPRT=0"); //Set prompt of "send ok" When Module Sends Data
  delay(3000);
  //while (smsSerial.available() > 0)
  //smsSerial.read();

  smsSerial.println("AT+CIPSTART=\"TCP\",\"api.thingspeak.com\",\"80\"");//start up the connection. Here we can see the type of connection, server name, port number
  delay(6000);
  //while (smsSerial.available() > 0)
  //smsSerial.read();

  smsSerial.println("AT+CIPSEND"); // Send Data Through TCP or UDP Connection
  delay(4000);
  //while (smsSerial.available() > 0)
  //smsSerial.read();

  //// NEWLY ADDED v

  /*  writeAPIKey = "";
    myFile = SD.open("WAK.txt"); // KFZQEEF487BI3NHD
    while (myFile.available()) {
      writeAPIKey += String(char(myFile.read()));
    }
    myFile.close();*/

  String postData = writeAPIKey;
  Serial.println(writeAPIKey);
  postData += "&field1=";
  postData += String(instCurrent);
  postData += "&field2=";
  postData += String(instVoltage);
  postData += "&field3=";
  postData += String(instPower);
  postData += "&field4=";
  postData += String(avgPower);
  postData += "&field5=";
  postData += String(totalEnergy);
  postData += "&field6=";
  postData += Latitude;
  postData += "&field7=";
  postData += Longitude;
  postData += "&field8=";
  postData += String(rpm);
  postData += "\r\n\r\n";
  Serial.println(postData);

  smsSerial.print("POST /update HTTP/1.1\n");
  smsSerial.print("Host: api.thingspeak.com\n");
  smsSerial.print("Connection: close\n");
  smsSerial.print("X-THINGSPEAKAPIKEY: " + writeAPIKey + "\n");
  smsSerial.print("Content-Type: application/x-www-form-urlencoded\n");
  smsSerial.print("Content-Length: ");
  smsSerial.print(postData.length());
  smsSerial.print("\n\n");
  smsSerial.print(postData);

  smsSerial.println((char)26);//sending
  delay(5000);//waiting for reply, important! the time is base on the condition of internet
  postData = "";
  //while (smsSerial.available() > 0)
  // smsSerial.read();
  smsSerial.println("AT+CIPSHUT");//close the connection
  delay(1000);
  while (smsSerial.available() > 0)
    smsSerial.read();
}

void checkRPM()
{
  Serial.print("CheckRPM() initiated"); //for debugging only
  int rpmCounter = 0;
  float prevRPM = getRPM(5.0);
  if (prevRPM > midThreshRPM)
  {
    return;
  }
  else
  {

    rpmCounter = 0;
    prevRPM = getRPM(5.0);
    delay(10);
    for (intBuff = 0; intBuff < 10; intBuff++)
    {
      rpm = getRPM(5.0);
      if (rpm - prevRPM > 5.0)
      {
        rpmCounter ++;
      }
      prevRPM = rpm;
    }

    if (rpmCounter == 10) //rpm is increasing
    {
      if ( rpm - lowThreshRPM < 5.0)
      {
        digitalWrite(loadPin, LOW);
        delay(1000);
        digitalWrite(noLoadRelay, HIGH);
        digitalWrite(motorRelay, HIGH);
      }
      else
      {
        return;
      }
    }
    else
    {
      digitalWrite(loadPin, LOW);
      delay(1000);
      digitalWrite(noLoadRelay, HIGH);
      digitalWrite(motorRelay, HIGH);
    }


    rpmCounter = 0;
    for (intBuff = 0; intBuff < 10; intBuff++)
    {
      rpm = getRPM(5.0);
      if (rpm - prevRPM > 5.0)
      {
        rpmCounter ++;
      }
      prevRPM = rpm;
    }

    if (rpmCounter == 10) //rpm is increasing
    {
      delay(1000);
      digitalWrite(switchRelay, LOW); //// gradual engaging lines
      digitalWrite(motorRelay, LOW);
      digitalWrite(noLoadRelay, LOW);
      Serial.println("Gradually connecting load");
      for (intBuff = 0; intBuff < 32; intBuff++)
      {
        analogWrite(loadPin, intBuff);
        delay(100);
      }
      delay(3000);
      analogWrite(loadPin, 32);
      return;
    }
    else
    {
      digitalWrite(loadPin, LOW);
      digitalWrite(switchRelay, LOW);
      digitalWrite(motorRelay, LOW);
      digitalWrite(noLoadRelay, LOW);
      totalShutDownRPM();
    }
  }
}



void totalShutDownRPM()
{
  smsMessage += "Generating station number ";
  smsMessage += String(stationNumber);
  smsMessage += " was shutdown due to low windflow.";
  Serial.println(smsMessage);
  sendSMS();
  serialFlushf();
  digitalWrite(switchRelay, HIGH);
  Serial.println("Gradual jamming initiated");
  for (intBuff = 0; intBuff < 255; intBuff++)
  {
    analogWrite(shutDownPin, intBuff);
    delay(1); //delay(100);
  }
  analogWrite(shutDownPin, 255);
  delay(3000);
  Serial.println("Paused. Please restart later");
  while (checkSMS() != 1)
  {
    Serial.print(".");
    delay(10000);
  }
  smsMessage += "Generating station number ";
  smsMessage += String(stationNumber);
  smsMessage += " has restarted.";
  Serial.println(smsMessage);
  sendSMS();
  digitalWrite(switchRelay, LOW);
  digitalWrite(shutDownPin, LOW);
  noLoadStartUpTime = millis(); //newly added
  gradualStartUp();
  return;
}

int checkIfShutDown()
{
  int VT = analogRead(VT_PIN);
  rpm = getRPM(20.0);

  instVoltage = VT * (5.0 / 1023.0) * 5.0;

  if (instVoltage > 20.0 || rpm > highThreshRPM || checkSMS() == 2 || checkTemperature() > maxTemp)
  {
    Serial.print("excessive voltage or rpm or temp");
    shutDownState = 1; //Shutdown  completely
  }
  else
    shutDownState = 0; //Continue as it is

  return shutDownState;
}

void totalShutDown() //Will be activated when there is an SMS ordering it to shutdown
{
  smsMessage += "Generating station number ";
  smsMessage += String(stationNumber);
  smsMessage += " was shutdown.";
  Serial.println(smsMessage);
  sendSMS();
  serialFlushf();
  digitalWrite(switchRelay, HIGH);
  for (intBuff = 250; intBuff < 255; intBuff++)
  {
    analogWrite(shutDownPin, intBuff);
    delay(1); //delay(100);
  }
  analogWrite(shutDownPin, 255);
  digitalWrite(shutDownPin, HIGH);
  delay(5000);
  Serial.println("Paused. Please restart later");
  while (checkSMS() != 1)
  {
    Serial.print(".");
    delay(1000);
    serialFlushf();
    while (smsSerial.available() > 0)
      smsSerial.read();
  }
  smsMessage += "Generating station number ";
  smsMessage += String(stationNumber);
  smsMessage += " has restarted.";
  Serial.println(smsMessage);
  sendSMS();
  digitalWrite(switchRelay, LOW);
  digitalWrite(shutDownPin, LOW);
  //gradualStartUp();
  return;
}

void gradualStartUp()
{
  digitalWrite(shutDownPin, LOW);
  Serial.println("Gradual startup initiated");
  int VT = analogRead(VT_PIN);
  instVoltage = VT * (5.0 / 1023.0) * 5.0;

  if (instVoltage < minThreshVoltage)
  {
    digitalWrite(motorRelay, HIGH); //connects the motor branch, will be activated only when noloadrelay is LOW
    delay(100);
    for (intBuff = 0; intBuff < 255; intBuff + 5)
    {
      analogWrite(pwmMotor, intBuff);
      delay(100);
    }
    analogWrite(pwmMotor, 255);
    delay(3000);
    digitalWrite(pwmMotor, LOW);

    noLoadStartUpTime = millis(); //newly added
    while (getRPM(5.0) < midThreshRPM)
    {
      digitalWrite(noLoadRelay, HIGH); //Load is not connected
      if (millis() - noLoadStartUpTime > 60000)
      {
        digitalWrite(motorRelay, LOW);
        digitalWrite(noLoadRelay, LOW); // recheck this //Load may be connected
        startUpErrorShutDown();
        noLoadStartUpTime = millis(); //newly added
      }
    }

    digitalWrite(motorRelay, LOW);
    digitalWrite(noLoadRelay, LOW); //connect to the charging ckt
  }
  digitalWrite(switchRelay, LOW);
  digitalWrite(motorRelay, LOW);
  for (intBuff = 0; intBuff < 32; intBuff++)
  {
    analogWrite(loadPin, intBuff);
    delay(10 * intBuff * intBuff);
  }
  analogWrite(loadPin, 32);
  noLoadStartUpTime = millis(); //newly added
  return;
}
