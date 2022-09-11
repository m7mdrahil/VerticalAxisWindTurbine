#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 16, 2);

int interruptPin = 2; //connected to coin sensor
// Pin 2 and 3 are the interrupt pins on Arduino UNO

#include <Keypad.h>
const byte ROWS = 4; //four rows
const byte COLS = 3; //three columns
char keys[ROWS][COLS] = {
  {'1', '2', '3'},
  {'4', '5', '6'},
  {'7', '8', '9'},
  {'*', '0', '#'}
};
byte rowPins[ROWS] = {5, 10, 9, 7}; //connect to the row pinouts of the keypad
byte colPins[COLS] = {6, 4, 8}; //connect to the column pinouts of the keypad
Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );

String outData = "";

unsigned long defaultTime = 10.0; // In minutes

int state[4] = { 0, 0, 0, 0}; // if state[portnumber-1] = 0, then unoccupied, else occupied
unsigned long timeRemainingPort1 =  0.0; //self explanatory
unsigned long timeRemainingPort2 =  0.0;
unsigned long timeRemainingPort3 =  0.0;
unsigned long timeRemainingPort4 =  0.0;

unsigned long runTime1 = 0.0; // self explanatory
unsigned long runTime2 = 0.0;
unsigned long runTime3 = 0.0;
unsigned long runTime4 = 0.0;

unsigned long startingTimePort1 =  0.0; //self explanatory
unsigned long startingTimePort2 =  0.0;
unsigned long startingTimePort3 =  0.0;
unsigned long startingTimePort4 =  0.0;

//const int portPin1 = 3;    // the number of the pushbutton pin that will be near the port
const int chargePin1 = A0;     // the number of the pin that will be connected to the charging circuit

//const int portPin2 = 5;    // the number of the pushbutton pin that will be near the port
const int chargePin2 = A1;     // the number of the pin that will be connected to the charging circuit

//const int portPin3 = 7;    // the number of the pushbutton pin that will be near the port
const int chargePin3 = A2;     // the number of the pin that will be connected to the charging circuit

//const int portPin4 = A2;    // the number of the pushbutton pin that will be near the port
const int chargePin4 = A3;     // the number of the pin that will be connected to the charging circuit


// Variables will change:
int buttonState;             // the current reading from the input pin

// the following variables are unsigned longs because the time, measured in
// milliseconds, will quickly become a bigger number than can be stored in an int.
unsigned long lastDebounceTime = 0;  // the last time the output pin was toggled
unsigned long debounceDelay = 50;    // the debounce time; increase if the output flickers

volatile int shouldInterrupt = 0; //if 0, no interrupt, else interrupt

void setup() {

  Serial.begin(115200);
  defaultTime = defaultTime * 60000;
  pinMode(interruptPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(interruptPin), interruptFunction, HIGH);//connect coin directly to pin 2 of Arduino Uno
  // put your setup code here, to run once:

  //pinMode(portPin1, INPUT);
  pinMode(chargePin1, OUTPUT);
  //pinMode(portPin2, INPUT);
  pinMode(chargePin2, OUTPUT);
  //pinMode(portPin3, INPUT);
  pinMode(chargePin3, OUTPUT);
  //pinMode(portPin4, INPUT);
  pinMode(chargePin4, OUTPUT);

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

}

void loop() {
  // put your main code here, to run repeatedly:
  if (shouldInterrupt == 1)
  {
    //Serial.println("Interrupt");
    choosePortnum();
  }
  checkTimeRemaining();
  displayData();
  //char key = keypad.getKey();
  //if (key == '0'||key == '1'||key == '2'||key == '3'||key == '4'||key == '5'||key == '6'||key == '7'||key == '8'||key == '9'||key == '*'||key == '#')
  //Serial.println(key);

  delay(700);
}

void choosePortnum()
{
  int exitloop = 0;
  while (exitloop == 0)
  {
    char key = keypad.getKey();
    if (key == '1')
    {
      state[0] = 1;
      startingTimePort1 =  millis();
      runTime1 = 60000;
      exitloop = 1;
    }
    else if (key == '2')
    {
      state[1] = 1;
      startingTimePort2 =  millis();
      runTime2 = 60000;
      exitloop = 1;
    }
    else if (key == '3')
    {
      state[2] = 1;
      startingTimePort3 =  millis();
      runTime3 = 60000;
      exitloop = 1;
    }
    else if (key == '4')
    {
      state[3] = 1;
      startingTimePort4 =  millis();
      runTime4 = 60000;
      exitloop = 1;
    }
    else
      return;
  }
  shouldInterrupt = 0;
}

void displayData()
{
  lcd.clear();
  lcd.setCursor(0, 0);
  if (state[0] == 1) {
    lcd.print("OCC");
    lcd.setCursor(0, 1);
    lcd.print("00");
  }
  else
    lcd.print("VAC");

  lcd.setCursor(4, 0);
  if (state[1] == 1) {
    lcd.print("OCC");
    lcd.setCursor(4, 1);
     lcd.print("00");
  }
  else
    lcd.print("VAC");

  lcd.setCursor(8, 0);
  if (state[2] == 1) {
    lcd.print("OCC");
    lcd.setCursor(8, 1);
    lcd.print(String(timeRemainingPort3));
  }
  else
    lcd.print("VAC");

  lcd.setCursor(12, 0);
  if (state[3] == 1) {
    lcd.print("OCC");
    lcd.setCursor(12, 1);
    lcd.print(String(timeRemainingPort4));
  }
  else
    lcd.print("VAC");
  Serial.print(timeRemainingPort1); Serial.print("\t"); // for debugging
  Serial.print(timeRemainingPort2); Serial.print("\t"); // for debugging
  Serial.print(timeRemainingPort3); Serial.print("\t"); // for debugging
  Serial.print(timeRemainingPort4); Serial.print("\n"); // for debugging
}
//Make into seconds

void checkTimeRemaining()
{

  timeRemainingPort1 = (startingTimePort1 + runTime1 - millis()) / 1000;
  timeRemainingPort2 = (startingTimePort2 + runTime2 - millis()) / 1000;
  timeRemainingPort3 = (startingTimePort3 + runTime3 - millis()) / 1000;
  timeRemainingPort4 = (startingTimePort4 + runTime4 - millis()) / 1000;

  runTime1 = 0.0;    state[0] = 1; digitalWrite(chargePin1, HIGH); //newly changed

  runTime2 = 0.0;    state[1] = 1; digitalWrite(chargePin2, HIGH); //newly changed

  if (millis() - startingTimePort3 > runTime3) {
    state[2] = 0; digitalWrite(chargePin3, LOW); runTime3 = 0.0;
  }
  else  {
    state[2] = 1; digitalWrite(chargePin3, HIGH);
  }

  if (millis() - startingTimePort4 > runTime4) {
    state[3] = 0; digitalWrite(chargePin4, LOW); runTime4 = 0.0;
  }
  else  {
    state[3] = 1; digitalWrite(chargePin4, HIGH);
  }


}

void interruptFunction()
{
  shouldInterrupt = 1;
  return;
}
