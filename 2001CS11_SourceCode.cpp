#include <LiquidCrystal_I2C.h>
#include <hd44780.h>
#include<Wire.h>
#include <Keypad.h> // library for keyboard
#include <Password.h> // library for password
#include <SoftwareSerial.h>
SoftwareSerial mySerial(10  , 11);
LiquidCrystal_I2C lcd(0x27, 16, 2);
Password password = Password("8979"); 
const byte rows = 4; // four rows
const byte cols = 4; // three columns
char keys[rows][cols] = { // keys on keypad
{'1','2','3','A'},
{'4','5','6','B'},
{'7','8','9','C'},
{'*','0','#','D'},
};
byte rowPins[rows] = {9,8,7,6};
byte colPins[cols] = {5,4,3,2};
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, rows, cols);
#define sensor A3 // pin for PIR sensor data
#define alrm 12 // pin for siren, buzzer
#define redLed A2 // pin for red led
#define yellowLed A0 // pin for blue led
#define orangeLed A1 // pin for orange led
int contact = 10; //used to immediately on/off alarm
int val;
int ledBlink;
int sensorData; //Variable for collecting the sensor data
unsigned long clk, temp;
int activation_delay = 20; // To delay for standby to armed
int deactivation_delay = 10; // To delay for triggered to alarm activated
int alarm_on = 10; // Time of alarm is on
char state = 0; // This is the variable for states "0"
int set_up = 0; // system is 0 for off and 1 for on
int count=0;
int cnt=0;
void setup()
{
keypad.addEventListener(keypadEvent); // an object is created for tracking keystrokes
Serial.begin(9600);
mySerial.begin(9600);   // Setting the baud rate of GSM Module 
pinMode(alrm, OUTPUT);
pinMode(sensor, INPUT);
pinMode(contact, INPUT);
pinMode(redLed, OUTPUT);
pinMode(orangeLed, OUTPUT);
pinMode(yellowLed, OUTPUT);
digitalRead(contact);
Serial.println("System startup"); //Used for troubleshooting
Serial.println("Alarm button status:"); //used for troubleshooting
Serial.println(digitalRead(contact)); //used for troubleshooting
lcd.begin(); //Defining 16 columns and 2 rows of lcd display
lcd.backlight();
lcd.setCursor(0,0);
Serial.print("CS225-226");
lcd.print("CS225-226");
lcd.setCursor(0,1);
lcd.print("PROJECT");
delay(8000);
lcd.clear();
}
void loop()
{
clk = millis(); // read the internal clock
val = digitalRead(contact);
keypad.getKey();
if (set_up%2 == 0)
{
digitalWrite(yellowLed, LOW);
digitalWrite(redLed, LOW);
digitalWrite(orangeLed, HIGH);
digitalWrite(alrm, LOW);
state = 0;
}
else
{
if(state == 0) // alarm is on
{
state = 1;
temp = clk;
digitalWrite(orangeLed, HIGH);
}
if(state == 1) // system waiting
{
if ((clk%1000)<500) digitalWrite(yellowLed, HIGH);
else digitalWrite(yellowLed, LOW);
keypad.getKey();
if(clk >= temp + activation_delay*1000) {
state = 2;
}
}
if(state == 2) // system is armed
{
digitalWrite(yellowLed, HIGH);
keypad.getKey();
sensorData = digitalRead(sensor);
if(sensorData == HIGH)
{
state = 3;
temp = clk;
digitalWrite(yellowLed, LOW);
}
}
if(state == 3) // system is triggered
{
if ((clk%500)<100) digitalWrite(redLed, HIGH);
else digitalWrite(redLed, LOW);
keypad.getKey();
if(clk >= temp + deactivation_delay * 10)
{
state = 4;
temp = clk;
}
Serial.println("System is triggered!"); //Used for troubleshooting
}
if(state == 4) // buzzer (siren) is active
{
digitalWrite(redLed, HIGH);
lcd.setCursor(0,0);
lcd.print("MOTION DETECTED!");

Serial.println("Siren is active !"); //Used for troubleshooting
// For buzzer (siren)
for(int i=0; i<80; i++)
{
digitalWrite(alrm,HIGH);
delay(2);
digitalWrite(alrm,LOW);
delay(2);
}


if(clk >= temp + alarm_on * 1000) {
state = 2; 
digitalWrite(alrm, LOW); 
digitalWrite(redLed, LOW);
if(cnt==0){
  SendMessage();
  cnt++;
}
lcd.clear();
}
}
}
}

void keypadEvent(KeypadEvent eKey){ // Taking care of the cases in which if password is wrong indication would be given
switch (keypad.getState()){
case PRESSED:
Serial.print("Pressed: ");
Serial.println(eKey);
switch (eKey){
case '*': checkPassword(); break;
case '#': password.reset(); break;
default: password.append(eKey);
}
}
}
void checkPassword(){
if (password.evaluate()&& count==0){
Serial.println("Success");
lcd.setCursor(0,0);
lcd.print("SUCCESS");
delay(1000);
lcd.clear();
lcd.setCursor(0,0);
lcd.print("ACTIVATING");
lcd.setCursor(0,1);
lcd.print("THE SYSTEM");
delay(2000);
lcd.clear();
set_up++;
password.reset();
count++;
}
else if (password.evaluate()&& count==1){
Serial.println("Deactivating!");
lcd.setCursor(0,0);
lcd.print("DE-ACTIVATING");
lcd.setCursor(0,1);
lcd.print("THE SYSTEM");
delay(2000);
lcd.clear();
set_up++;
password.reset();
count--;
}
else{
Serial.println("Wrong Password!");
lcd.setCursor(0,0);
lcd.print("WRONG");
lcd.setCursor(0,1);
lcd.print("PASSWORD");
delay(2000);
lcd.clear();
ledBlink = 0;
while (ledBlink <= 5){
digitalWrite(redLed, HIGH);
delay(100);
digitalWrite(redLed, LOW);
delay(100);
ledBlink++;
}
password.reset();
}
}
void SendMessage()
{
  Serial.println("Messaging  ");
  mySerial.println("AT+CMGF=1");    //Sets the GSM Module in Text Mode
  delay(1000);  // Delay of 1000 milli seconds or 1 second
  mySerial.println("AT+CMGS=\"+919905163752\"\r"); // Replace x with mobile number
  delay(1000);
  mySerial.println("motion detected");// The SMS text you want to send
  delay(100);
   mySerial.println((char)26);// ASCII code of CTRL+Z
  delay(1000);
}