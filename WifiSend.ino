#include <ESP8266WiFi.h>

const char server[] PROGMEM = "api.thingspeak.com";
const char MY_SSID[] PROGMEM = "UOB-Events"; // WiFi name
const char MY_PWD[] PROGMEM = "uob.2013"; // WiFi password
String writeAPIKey = "KFZQEEF487BI3NHD"; // Unique API key generated from the Thingspeak website

char* inData = "";

int i, j;
String instCurrent = "1.0", instVoltage = "1.0", instPower = "1.0", avgPower = "1.0", totalEnergy = "1.0", Latitude = "25.0", Longitude = "50.0", rpm = "0.0";
String buffStr = "";

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  while (Serial.available() > 0)
  {
    Serial.read();
  }

  connectWifi();

}


void loop()
{
  if (Serial.available() > 0)
  {
    buffStr = Serial.readString();
    if (shouldIgnore() == false)
      parseData();

  }


  while (Serial.available() > 0)
    Serial.read();
}

void parseData()
{

  inData = "";
  instCurrent = ""; instVoltage = ""; instPower = ""; avgPower = ""; totalEnergy = ""; Latitude = ""; Longitude = "";
  char startMarker = '<';
  char endMarker = '#';
  j = 0;
  float buffFloat;

  int highIndex = 0;
  int lowIndex = 0;
  int startIndex = 0;
  int endIndex = buffStr.length();
  j = 0;
  //          Serial.print(buffStr);
  while (buffStr.charAt(j) != '>')
  {
    if (buffStr.charAt(j) == startMarker)
    {
      highIndex = j;
      if (startIndex <= highIndex)
      {
        startIndex = highIndex;
      }
    }

    else if (buffStr.charAt(j) == endMarker)
    {
      lowIndex = j;
      if (lowIndex <= endIndex)
      {
        endIndex = lowIndex;
      }
    }
    j++;
  }

  i = 0;


  for (j = startIndex + 1; j < endIndex + 1; j++)
  {
    inData[i] = buffStr.charAt(j);
    i++;
  }
  inData[i] = '\0';

  instCurrent = " "; instVoltage = " "; instPower = " "; avgPower = " "; totalEnergy = " "; Latitude = " "; Longitude = " ";

  i = 1;
  j = 0;
  while (inData[i] != ' ')
  {
    //instCurrent.charAt(j) = inData.charAt(i);
    if (j == 0)
      instCurrent.setCharAt(j, inData[i]);
    else
      instCurrent += String(inData[i]);
    i = i + 1;
    j = j + 1;
  }
  buffFloat = instCurrent.toFloat();
  instCurrent = String(buffFloat);
  //    instCurrent.setCharAt(j,'\0');

  i = i + 1;
  j = 0;
  while (inData[i] != ' ')
  {
    //instVoltage.charAt(j) = inData.charAt(i);
    if (j == 0)
      instVoltage.setCharAt(j, inData[i]);
    else
      instVoltage += String(inData[i]);
    i = i + 1;
    j = j + 1;
  }
  //    buffFloat = instVoltage.toFloat();
  //    instVoltage = String(buffFloat);
  //    instVoltage.setCharAt(j,'\0');

  i = i + 1;
  j = 0;
  while (inData[i] != ' ')
  {
    //instPower.charAt(j) = inData.charAt(i);
    if (j == 0)
      instPower.setCharAt(j, inData[i]);
    else
      instPower += String(inData[i]);
    i = i + 1;
    j = j + 1;
  }
  //    buffFloat = instPower.toFloat();
  //    instPower = String(buffFloat);
  // instPower.setCharAt(j,'\0');

  i = i + 1;
  j = 0;
  while (inData[i] != ' ')
  {
    //avgPower.charAt(j) = inData.charAt(i);
    if (j == 0)
      avgPower.setCharAt(j, inData[i]);
    else
      avgPower += String(inData[i]);
    i = i + 1;
    j = j + 1;
  }
  //     buffFloat = avgPower.toFloat();
  //     avgPower = String(buffFloat);
  //   avgPower.setCharAt(j,'\0');

  i = i + 1;
  j = 0;
  while (inData[i] != ' ')
  {
    //totalEnergy.charAt(j) = inData.charAt(i);
    if (j == 0)
      totalEnergy.setCharAt(j, inData[i]);
    else
      totalEnergy += String(inData[i]);
    i = i + 1;
    j = j + 1;
  }
  //   buffFloat = totalEnergy.toFloat();
  //   totalEnergy = String(buffFloat);
  //   totalEnergy.setCharAt(j,'\0');

  i = i + 1;
  j = 0;
  while (inData[i] != ' ')
  {
    //totalEnergy.charAt(j) = inData.charAt(i);
    if (j == 0)
      Latitude.setCharAt(j, inData[i]);
    else
      Latitude += String(inData[i]);
    i = i + 1;
    j = j + 1;
  }
  //     buffFloat = Latitude.toFloat();
  //     Latitude = String(buffFloat);
  //     Latitude.setCharAt(j,'\0');

  while (inData[i] != ' ')
  {
    //totalEnergy.charAt(j) = inData.charAt(i);
    if (j == 0)
      Longitude.setCharAt(j, inData[i]);
    else
      Longitude += String(inData[i]);
    i = i + 1;
    j = j + 1;
  }


  i = i + 1;
  j = 0;
  while (inData[i] != '#')
  {
    //totalEnergy.charAt(j) = inData.charAt(i);
    if (j == 0)
      rpm.setCharAt(j, inData[i]);
    else
      rpm += String(inData[i]);
    i = i + 1;
    j = j + 1;
  }
  //       buffFloat = Longitude.toFloat();
  //       Longitude = String(buffFloat);
  //       Longitude.setCharAt(j,'\0');

  //Serial.println(instVoltage);
  int maxlen = buffStr.length() - 3;
  if (instCurrent.length() >= maxlen || instVoltage.length() >= maxlen || instPower.length() >= maxlen || avgPower.length() >= maxlen || totalEnergy.length() >= maxlen || Latitude.length() >= maxlen || Longitude.length() >= maxlen)
  {
    //errorFlag = 1;
    Serial.print("error"); // For debugging
    // Serial.print(instVoltage);
  }
  else //No error
  {

    // Serial.println(String(inData));
    uploadData();
    // delay(15000);
    // Serial.println("."); // For debugging only
    //  Serial.print("instCurrent."); Serial.print(instCurrent); Serial.print(".\n"); //Serial.print(" ");
    //  Serial.print("instVoltage."); Serial.print(instVoltage);Serial.print(".\n"); //Serial.print(" ");
    //  Serial.print("instPower."); Serial.print(instPower);Serial.print(".\n"); //Serial.print(" ");
    //  Serial.print("avgPower."); Serial.print(avgPower);Serial.print(".\n"); //Serial.print(" ");
    //  Serial.print("totalEnergy."); Serial.print(totalEnergy);Serial.print(".\n"); //Serial.print(" ");
    //  Serial.print("Latitude."); Serial.print(Latitude);Serial.print(".\n"); //Serial.print(" ");
    //  Serial.print("Longitude."); Serial.print(Longitude);Serial.print(".\n"); //Serial.print(" ");
  }

  return;
}

boolean shouldIgnore()
{
  if (buffStr.length() > 30)
    return true;
           else
             return false;
}


void uploadData()
{

  WiFiClient client;
  if (client.connect(server, 80)) { // use ip 184.106.153.149 or api.thingspeak.com
    //Serial.println("WiFi Client connected ");
    String postData = writeAPIKey;
    postData += "&field1=";
    postData += instCurrent;
    postData += "&field2=";
    postData += instVoltage;
    postData += "&field3=";
    postData += instPower;
    postData += "&field4=";
    postData += avgPower;
    postData += "&field5=";
    postData += totalEnergy;
    postData += "&field6=";
    postData += Latitude;
    postData += "&field7=";
    postData += Longitude;
    postData += "&field8=";
    postData += rpm;
    postData += "\r\n\r\n";
    Serial.println(postData);
    client.print("POST /update HTTP/1.1\n");
    client.print("Host: api.thingspeak.com\n");
    client.print("Connection: close\n");
    client.print("X-THINGSPEAKAPIKEY: " + writeAPIKey + "\n");
    client.print("Content-Type: application/x-www-form-urlencoded\n");
    client.print("Content-Length: ");
    client.print(postData.length());
    client.print("\n\n");
    client.print(postData);
    delay(1000);
    postData = "";
  }//end if
  else
  {
    Serial.println("Error: Client not connected");
  }
  client.stop();
  while (Serial.available())
    Serial.read();
}


void connectWifi()
{
  Serial.print("Connecting to " + *MY_SSID);
  WiFi.begin(MY_SSID, MY_PWD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    //  getInitialValues();
    Serial.print(".");
  }

  Serial.println("Connected");
}//end connect
