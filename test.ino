#include <Wire.h>

#include <SoftwareSerial.h>
#include <Servo.h>

//LCD display
SoftwareSerial LCD(17,16); // RX, TX The LCD's RX is connected to the TX pin of the arduino which is pin 16

//Servomotors
Servo rightServo;
Servo leftServo;
Servo headServo;

//encoder
const int encoderRight = 36;
const int encoderLeft = 35;

//PING sensor
const int pingPin = 22;
unsigned int duration, inches;

//Variables
int distance;
int numcycles = 0;
char turndirection; //Gets 'l', 'r' or 'f' depending on which direction is obstacle free
const int turntime = 900; //Time the robot spends turning (miliseconds)
int thereis;
int leftscanval, centerscanval, rightscanval, ldiagonalscanval, rdiagonalscanval;
char choice;
const int distancelimit = 30; //Distance limit for obstacles in front           
const int sidedistancelimit = 12; //Minimum distance in cm to obstacles at both sides (the robot will allow a shorter distance sideways)


void setup() {
  // put your setup code here, to run once:
  rightServo.attach(8);
  leftServo.attach(45);
  headServo.attach(7);
  headServo.write(80); //position to center
  pinMode(encoderRight, OUTPUT);
  pinMode(encoderLeft, OUTPUT);
  pinMode(pingPin, );          // Set pin to OUTPUT
  digitalWrite(encoderRight,LOW);
  digitalWrite(encoderLeft,LOW);
  LCD.begin(9600);// all SerLCDs come at 9600 Baud by default
}

//----------------------------------START--LCD----------------------------------------------
//------------------------------------------------------------------------------------------
void clearScreen()
{
  //clears the screen, you will use this a lot!
  LCD.write(254); //command flag 0xFE
  LCD.write(1); //0x01
}
//-------------------------------------------------------------------------------------------
void selectLineOne()
{ 
  //puts the cursor at line 0 char 0.
  LCD.write(254); //command flag 0xFE
  LCD.write(128); //position
}
//-------------------------------------------------------------------------------------------
void selectLineTwo()
{ 
  //puts the cursor at line 0 char 0.
  LCD.write(254); //command flag 0xFE
  LCD.write(192); //position
}
//-------------------------------------------------------------------------------------------
void moveCursorRightOne()
{
  //moves the cursor right one space
  LCD.write(254); //command flag 0xFE
  LCD.write(20); // 0x14
}
//-------------------------------------------------------------------------------------------
void moveCursorLeftOne()
{
  //moves the cursor left one space
  LCD.write(254); //command flag 0xFE
  LCD.write(16); // 0x10
}
//-------------------------------------------------------------------------------------------
void scrollRight()
{
  //same as moveCursorRightOne
  LCD.write(254); //command flag 0xFE
  LCD.write(20); // 0x14
}
//-------------------------------------------------------------------------------------------
void scrollLeft()
{
  //same as moveCursorLeftOne
  LCD.write(254); //command flag 0xFE
  LCD.write(24); // 0x18
}
//-------------------------------------------------------------------------------------------
void turnDisplayOff()
{
  //this tunrs the display off, but leaves the backlight on. 
  LCD.write(254); //command flag 0xFE
  LCD.write(8); // 0x08
}
//-------------------------------------------------------------------------------------------
void turnDisplayOn()
{
  //this turns the dispaly back ON
  LCD.write(254); //command flag 0xFE
  LCD.write(12); // 0x0C
}
//-------------------------------------------------------------------------------------------
void underlineCursorOn()
{
  //turns the underline cursor on
  LCD.write(254); //command flag 0xFE
  LCD.write(14); // 0x0E
}
//-------------------------------------------------------------------------------------------
void underlineCursorOff()
{
  //turns the underline cursor off
  LCD.write(254); //command flag 0xFE
  LCD.write(12); // 0x0C
}
//-------------------------------------------------------------------------------------------
void boxCursorOn()
{
  //this turns the box cursor on
  LCD.write(254); //command flag 0xFE
  LCD.write(13); // 0x0D
}
//-------------------------------------------------------------------------------------------
void boxCursorOff()
{
  //this turns the box cursor off
  LCD.write(254); //command flag 0xFE
  LCD.write(12); // 0x0C
}
//-------------------------------------------------------------------------------------------
void toggleSplash()
{
  //this toggles the spalsh screenif off send this to turn onif on send this to turn off
  LCD.write(124); //command flag = 124 dec 0x7C
  LCD.write(9); // 0x09
}
//-------------------------------------------------------------------------------------------
int backlight(int brightness)// 128 = OFF, 157 = Fully ON, everything inbetween = varied brightnbess 
{
  //this function takes an int between 128-157 and turns the backlight on accordingly
  LCD.write(124); //NOTE THE DIFFERENT COMMAND FLAG 0x7C = 124 dec
  LCD.write(brightness); // any value between 128 and 157 or 0x80 and 0x9D
}

//---------------------------------END--LCD--BASIC--FUNCTIONS-------------------------------
//------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------
void scrollingMarquee()
{
//This function scroll text across the screen on both lines
  clearScreen(); // it's always good to clear the screen before movonh onto a new print
  for(int j = 0; j < 17; j++)
  {
    selectLineOne();
    for(int i = 0; i < j;i++)
      moveCursorRightOne();
    LCD.print("SPARK");
    selectLineTwo();
    for(int i = 0; i < j;i++)
      moveCursorRightOne();
    LCD.print(" FUN");
    delay(2000); // you must have a delay, otherwise the screen will print and clear before you can see the text
    clearScreen();
  }
}
//-------------------------------------------------------------------------------------------
void counter()
{
  //this function prints a simple counter that counts to 10
  clearScreen();
  for(int i = 0; i <= 10; i++)
  {
    LCD.print("Counter = ");
    LCD.print(i, DEC);
    delay(500);
    clearScreen();
  }
}
//-------------------------------------------------------------------------------------------
void tempAndHumidity()
{
  //this function shows how you could read the data from a temerature and humidity 
  //sensor and then print that data to the SerLCD.
  
  //these could be varaibles instead of static numbers 
  float tempF = 77.0; 
  float tempC = 25.0;
  float humidity = 67.0;
  
  clearScreen();
  selectLineOne();
  LCD.print(" Temp = ");
  LCD.print((long)tempF, DEC);
  LCD.print("F ");
  LCD.print((long)tempC, DEC);
  LCD.print("C");
  selectLineTwo();
  LCD.print(" Humidity = ");
  LCD.print((long)humidity, DEC); 
  LCD.print("%");
  delay(2500);
}
//-------------------------------------------------------------------------------------------
void backlight()
{
  //this function shows the different brightnesses to which the backlight can be set 
  clearScreen();
  for(int i = 128; i < 158; i+=2)// 128-157 are the levels from off to full brightness
  {
    backlight(i);
    delay(100);
    LCD.print("Backlight = ");
    LCD.print(i, DEC);
    delay(500);
    clearScreen();
  }
}
//-------------------------------------------------------------------------------------------
void cursors()
{
  //this function shows the different cursors avaiable on the SerLCD
  clearScreen();
  
  boxCursorOn();
  LCD.print("Box On");
  delay(1500);
  clearScreen();
  
  boxCursorOff();
  LCD.print("Box Off");
  delay(1000);
  clearScreen();
  
  underlineCursorOn();
  LCD.print("Underline On");
  delay(1500);
  clearScreen();
  
  underlineCursorOff();
  LCD.print("Underline Off");
  delay(1000);
  clearScreen();
}
//----------------------------------END--LCD--TEST--FUNCTIONS-------------------------------
//------------------------------------------------------------------------------------------


//--------------------------------------WHEEL---FUNCTIONS-----------------------------------
//------------------------------------------------------------------------------------------
void forward(){ 
   digitalWrite (encoderLeft, HIGH);                              
   digitalWrite (encoderRight, HIGH); 
}

void backwards(){
  digitalWrite (encoderLeft , LOW);                              
  digitalWrite (encoderRight, LOW); 
}

int watch(){
  long howfar;
  digitalWrite(pingPin,LOW);
  delayMicroseconds(5);                                                                              
  digitalWrite(pingPin,HIGH);
  delayMicroseconds(15);
  digitalWrite(pingPin,LOW);
  howfar=pulseIn(pingPin,HIGH);
  howfar=howfar*0.01657; //how far away is the object in cm
  return round(howfar);
}

void turnleft(int t){
  digitalWrite (encoderLeft, LOW);                              
  digitalWrite (encoderRight, HIGH); 
  delay(t);
}

void turnright(int t){
  digitalWrite (encoderLeft, HIGH);                              
  digitalWrite (encoderRight, LOW); 
  delay(t);
}

void stopmove(){
  digitalWrite (encoderLeft ,LOW);                              
  digitalWrite (encoderRight, LOW); 
}




void pingSensor(){
  pinMode(pingPin, OUTPUT);          // Set pin to OUTPUT
  digitalWrite(pingPin, LOW);        // Ensure pin is low
  delayMicroseconds(2);
  digitalWrite(pingPin, HIGH);       // Start ranging
  delayMicroseconds(5);              //   with 5 microsecond burst
  digitalWrite(pingPin, LOW);        // End ranging
  pinMode(pingPin, INPUT);           // Set pin to INPUT
  duration = pulseIn(pingPin, HIGH); // Read echo pulse
  inches = duration / 74 / 2;        // Convert to inches
  LCD.println(inches);            // Display result
  delay(200);  
}

void loop() {
  // put your main code here, to run repeatedly:

}
