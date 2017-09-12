#include <WireExt.h>
#include <Wire.h>

#include <SoftwareSerial.h>
#include <Servo.h>

//For camera
#include <Adafruit_VC0706.h>
#include <SPI.h>
#include <SD.h>

//Alarm - Piezo buzzer
#define alarmspeaker 3

float alarmsinVal;
int alarmtoneVal;

//LCD display
SoftwareSerial LCD(19,18); // RX, TX The LCD's RX is connected to the TX pin of the arduino which is pin 16

//Servomotors
Servo rightServo;
Servo leftServo;
Servo headServo;

//PING sensor
const int pingPin = 22;

//Termo sensor //SDA pin 20, SLC pin 21
#define D6T_addr 0x0A
#define D6T_cmd 0x4C

//Variables
int distance;
int numcycles = 0;
char turndirection; //Gets 'l', 'r' or 'f' depending on which direction is obstacle free
const int turntime = 1000; //Time the robot spends turning (miliseconds)
int thereis;
int leftscanval, centerscanval, rightscanval, ldiagonalscanval, rdiagonalscanval;
char choice;
const int distancelimit = 40; //Distance limit for obstacles in front           
const int sidedistancelimit = 30; //Minimum distance in cm to obstacles at both sides

int rbuf[35];
int tdata[16];
int heatval;

//Camera and SD card
#define chipSelect 53
Adafruit_VC0706 cam = Adafruit_VC0706(&Serial2);

//Setup
void setup() {
  Wire.begin();
  pinMode(alarmspeaker, OUTPUT);
  LCD.begin(9600);// all SerLCDs come at 9600 Baud by default
  Serial.begin(9600);
  Serial.println("VC0706 Camera test");
  
  // see if the card is present and can be initialized:
  if (!SD.begin(chipSelect)) {
    Serial.println("Card failed, or not present");
    // don't do anything more:
    return;
  }
  
  // Try to locate the camera
  if (cam.begin()) {
    LCD.println("Camera Found:");
  } else {
    Serial.println("No camera found?");
    return;
  }
  // Print out the camera version information (optional)
  char *reply = cam.getVersion();
  if (reply == 0) {
    Serial.print("Failed to get version");
  } else {
    Serial.println("-----------------");
    Serial.print(reply);
    Serial.println("-----------------");
  }
 
  // Set the picture size - you can choose one of 640x480, 320x240 or 160x120
  // Remember that bigger pictures take longer to transmit!
  
  //cam.setImageSize(VC0706_640x480); // biggest
  cam.setImageSize(VC0706_320x240); // medium
  //cam.setImageSize(VC0706_160x120); // small
 
  // You can read the size back from the camera (optional, but maybe useful?)
  uint8_t imgsize = cam.getImageSize();
  Serial.print("Image size: ");
  if (imgsize == VC0706_640x480) Serial.println("640x480");
  if (imgsize == VC0706_320x240) Serial.println("320x240");
  if (imgsize == VC0706_160x120) Serial.println("160x120");
 
 
  // Motion detection system can alert you when the camera 'sees' motion!
  cam.setMotionDetect(true); // turn it on
  //cam.setMotionDetect(false); // turn it off (default)
 
  // You can also verify whether motion detection is active!
  Serial.print("Motion detection is ");
  if (cam.getMotionDetect())
    Serial.println("ON");
  else
    Serial.println("OFF");
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



//--------------------------------------SPEAKER--FUNCTION-----------------------------------
//------------------------------------------------------------------------------------------

void runalarm() {
  for (int x=0; x<180; x++) {
    // convert degrees to radians then obtain sin value
    alarmsinVal = (sin(x*(3.1412/180)));
    // generate a frequency from the sin value
    alarmtoneVal = 2000+(int(alarmsinVal*1000));
    tone(alarmspeaker, alarmtoneVal);
  }     
}
void stopalarm(){
  noTone(alarmspeaker);
}

//------------------------------------END--SPEAKER--FUNCTION--------------------------------
//------------------------------------------------------------------------------------------

//------------------------------------PING--SENSOR--FUNCTIONS-------------------------------
//------------------------------------------------------------------------------------------

int watch(){
  long howfar;
  pinMode(pingPin, OUTPUT);
  digitalWrite(pingPin,LOW);
  delayMicroseconds(5);                                                                              
  digitalWrite(pingPin,HIGH);
  delayMicroseconds(15);
  digitalWrite(pingPin,LOW);
  pinMode(pingPin, INPUT);
  howfar=pulseIn(pingPin,HIGH);
  howfar=howfar / 29 / 2 ; //how far away is the object in cm
  return round(howfar);
}

void lookstraight(){
  headServo.write(80);
}

void watchsurrounding(){ //Meassures distances to the right, left, front, left diagonal, right diagonal and asign them in cm to the variables rightscanval, 
                         //leftscanval, centerscanval, ldiagonalscanval and rdiagonalscanval (there are 5 points for distance testing)
  centerscanval = watch();
  if(centerscanval<distancelimit){stoprobot();}
  headServo.write(120);
  delay(100);
  rdiagonalscanval = watch();
  if(rdiagonalscanval<distancelimit){stoprobot();}
  headServo.write(170);
  delay(100);
  rightscanval = watch();
  if(rightscanval<sidedistancelimit){stoprobot();}
  headServo.write(120);
  delay(100);
  rdiagonalscanval = watch();
  if(rdiagonalscanval<distancelimit){stoprobot();}
  lookstraight();
  delay(100);
  centerscanval = watch();
  if(centerscanval<distancelimit){stoprobot();}
  headServo.write(30);
  delay(100);
  ldiagonalscanval = watch();
  if(ldiagonalscanval<distancelimit){stoprobot();}
  headServo.write(0);
  delay(100);
  leftscanval = watch();
  if(leftscanval<sidedistancelimit){stoprobot();}

  lookstraight(); //Finish looking around (look forward again)
  delay(100);
}

char decide(){
  watchsurrounding();
  if (leftscanval>rightscanval && leftscanval>centerscanval){
    choice = 'l';
  }
  else if (rightscanval>leftscanval && rightscanval>centerscanval){
    choice = 'r';
  }
  else{
    choice = 'f';
  }
  return choice;
}

//--------------------------------END--PING--SENSOR--FUNCTIONS------------------------------
//------------------------------------------------------------------------------------------

//--------------------------------------WHEEL---FUNCTIONS-----------------------------------
//------------------------------------------------------------------------------------------

void stoprobot(){
  rightServo.detach();
  leftServo.detach();
}

void startrobot(){
  lookstraight();
  rightServo.attach(8);
  leftServo.attach(9);
}

void moveForward(){
  startrobot();
  rightServo.write(180);
  leftServo.write(0);
}

//void moveBackwards(){
//  startrobot();
//  rightServo.write(0);
//  leftServo.write(180);
//}

void turnleft(int t){
  startrobot();
  leftServo.write(100); //left
  rightServo.write(100);
  delay(t);
}

void turnright(int t){
  startrobot();
  leftServo.write(40);
  rightServo.write(40); //right
  delay(t);
}

//--------------------------------------THERMAL---SENSOR-----------------------------------
//-----------------------------------------------------------------------------------------

void measurTemp() {
  int i;
      Wire.beginTransmission(D6T_addr);
      Wire.write(D6T_cmd);
      Wire.endTransmission();
 
      if (WireExt.beginReception(D6T_addr) >= 0) {
        i = 0;
        for (i = 0; i < 35; i++) {
          rbuf[i] = WireExt.get_byte();
        }
        WireExt.endReception();
 
        for (i = 0; i < 16; i++) {
          tdata[i]=(rbuf[(i*2+2)]+(rbuf[(i*2+3)]<<8))*0.1;
        } 
      }

      clearScreen();
      selectLineOne();
      LCD.print(tdata[0],1);
      moveCursorRightOne();
      LCD.print(tdata[1],1);
      moveCursorRightOne();
      LCD.print(tdata[2],1);
      moveCursorRightOne();
      LCD.print(tdata[3],1);
        
      moveCursorRightOne();
      LCD.print(tdata[4],1);
      moveCursorRightOne();
      LCD.print(tdata[5],1);
      moveCursorRightOne();
      LCD.print(tdata[6],1);
      moveCursorRightOne();
      LCD.print(tdata[7],1);
      
      selectLineTwo();
      LCD.print(tdata[8],1);
      moveCursorRightOne();
      LCD.print(tdata[9],1);
      moveCursorRightOne();
      LCD.print(tdata[10],1);
      moveCursorRightOne();
      LCD.print(tdata[11],1);
          
      moveCursorRightOne();
      LCD.print(tdata[12],1);
      moveCursorRightOne();
      LCD.print(tdata[13],1);
      moveCursorRightOne();
      LCD.print(tdata[14],1);
      moveCursorRightOne();
      LCD.print(tdata[15],1);
      delay(100);
}

int heatsource(){
  int heat=0;
  for (int i = 0; i < 16; i++){
    if (tdata[i] > 25){
      heat++;
    }
  }
  return heat;
}

//--------------------------------END---THERMAL---SENSOR-----------------------------------
//-----------------------------------------------------------------------------------------

//Camera
void takePicture(){
 if (cam.motionDetected()) {
   Serial.println("Motion!");
   cam.setMotionDetect(false);
   
  if (! cam.takePicture())
    Serial.println("Failed to snap!");
  else
    Serial.println("Picture taken!");
  
  char filename[13];
  strcpy(filename, "IMAGE00.JPG");
  for (int i = 0; i < 100; i++) {
    filename[5] = '0' + i/10;
    filename[6] = '0' + i%10;
    // create if does not exist, do not open existing, write, sync after write
    if (! SD.exists(filename)) {
      break;
    }
  }
  
  File imgFile = SD.open(filename, FILE_WRITE);
  
  uint16_t jpglen = cam.frameLength();
  Serial.print(jpglen, DEC);
  Serial.println(" byte image");
 
  Serial.print("Writing image to "); Serial.print(filename);
  
  while (jpglen > 0) {
    // read 32 bytes at a time;
    uint8_t *buffer;
    uint8_t bytesToRead = min(32, jpglen); // change 32 to 64 for a speedup but may not work with all setups!
    buffer = cam.readPicture(bytesToRead);
    imgFile.write(buffer, bytesToRead);
 
    //Serial.print("Read "); Serial.print(bytesToRead, DEC); Serial.println(" bytes");
 
    jpglen -= bytesToRead;
  }
  clearScreen();
  imgFile.close();
  Serial.println("...Done!");
  cam.resumeVideo();
  cam.setMotionDetect(true);
 }  
}

//main

void loop() {
  measurTemp();
  moveForward();  // start moving
  ++numcycles;
  if(numcycles>100){ //Watch if something is around every 130 loops while moving forward 
    watchsurrounding();
    if(leftscanval<sidedistancelimit || ldiagonalscanval<distancelimit){
      turnright(turntime);
    }
    if(rightscanval<sidedistancelimit || rdiagonalscanval<distancelimit){
      turnleft(turntime);
    }
    numcycles=0; //Restart count of cycles
  }
    
  distance = watch(); // use the watch() function to see if anything is ahead (when the robot is just moving forward and not looking around it will test the distance in front)
  if (distance<distancelimit-1){ // The robot will just stop if it is completely sure there's an obstacle ahead (must test 20 times) (needed to ignore ultrasonic sensor's false signals)
      stoprobot();
      measurTemp();
      heatval = heatsource();
      if(heatval >0){
        runalarm();
        takePicture();
      }
      else{
        stoprobot();
        turndirection = decide();
        switch (turndirection){
          case 'l':
          turnleft(turntime);
          break;
          case 'r':
          startrobot();
          turnright(turntime);
          break;
          case 'f':
          ;
          break;
        }
      }
  }
}
