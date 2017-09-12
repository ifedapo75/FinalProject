// LinkSprite.com
// Note:
// 1. SD must be formated to FAT16
// 2. As the buffer of softserial has 64 bytes, so the code read 32 bytes each time
// 3. Please add the libaray to the lib path
 
//  * SD card attached to SPI bus as follows:
//** MOSI - pin 11
// ** MISO - pin 12
// ** CLK - pin 52
// ** CS - pin 53
 
#include <SoftwareSerial.h>
#include <SPI.h>
#include <SD.h>
 
byte ZERO = 0x00;
 
byte incomingbyte;
SoftwareSerial mySerial(17,16);
 
long int a=0x0000,j=0,k=0,count=0,i=0;
uint8_t MH,ML;
boolean EndFlag=0;
File  myFile;
 
 
void setup()
{
 
Serial.begin(9600);
while (!Serial) {
; // wait for serial port to connect. Needed for Leonardo only
}
 
//mySerial.begin(38400);
 
Serial.print("\nInitializing SD card...");
// On the Ethernet Shield, CS is pin 4. It's set as an output by default.
// Note that even if it's not used as the CS pin, the hardware SS pin
// (10 on most Arduino boards, 53 on the Mega) must be left as an output
// or the SD library functions will not work.
pinMode(53, OUTPUT);
 
if (!SD.begin(53)) {
Serial.println("\ninitialization failed!");
return;
}
Serial.println("\ninitialization done.");
Serial.println("\nWaiting!");
}
 

 
void SendResetCmd()
{
mySerial.write(0x56);
mySerial.write(ZERO);
mySerial.write(0x26);
mySerial.write(ZERO);
}
 
void SetImageSizeCmd()
{
mySerial.write(0x56);
mySerial.write(ZERO);
mySerial.write(0x31);
mySerial.write(0x05);
mySerial.write(0x04);
mySerial.write(0x01);
mySerial.write(ZERO);
mySerial.write(0x19);
mySerial.write(0x11);
}
 
void SetBaudRateCmd()
{
mySerial.write(0x56);
mySerial.write(ZERO);
mySerial.write(0x24);
mySerial.write(0x03);
mySerial.write(0x01);
mySerial.write(0x2A);
mySerial.write(0xC8);
 
}
 
void SendTakePhotoCmd()
{
mySerial.write(0x56);
mySerial.write(ZERO);
mySerial.write(0x36);
mySerial.write(0x01);
mySerial.write(ZERO);
}
 
void SendReadDataCmd()
{
MH=a/0x100;
ML=a%0x100;
mySerial.write(0x56);
mySerial.write(ZERO);
mySerial.write(0x32);
mySerial.write(0x0c);
mySerial.write(ZERO);
mySerial.write(0x0a);
mySerial.write(ZERO);
mySerial.write(ZERO);
mySerial.write(MH);
mySerial.write(ML);
mySerial.write(ZERO);
mySerial.write(ZERO);
mySerial.write(ZERO);
mySerial.write(0x20);
mySerial.write(ZERO);
mySerial.write(0x0a);
a+=0x20;
}
 
void StopTakePhotoCmd()
{
mySerial.write(0x56);
mySerial.write(ZERO);
mySerial.write(0x36);
mySerial.write(0x01);
mySerial.write(0x03);
}

/*************************************/
/* Set ImageSize :
/* <1> 0x22 : 160*120
/* <2> 0x11 : 320*240
/* <3> 0x00 : 640*480
/* <4> 0x1D : 800*600
/* <5> 0x1C : 1024*768
/* <6> 0x1B : 1280*960
/* <7> 0x21 : 1600*1200
/************************************/
void SetImageSizeCmd(byte Size)
{
  mySerial.write(0x56);
  mySerial.write(ZERO);
  mySerial.write(0x54);
  mySerial.write(0x01);
  mySerial.write(Size);
}
 
/*************************************/
/* Set BaudRate :
/* <1>　0xAE  :   9600
/* <2>　0x2A  :   38400
/* <3>　0x1C  :   57600
/* <4>　0x0D  :   115200
/* <5>　0xAE  :   128000
/* <6>　0x56  :   256000
/*************************************/
void SetBaudRateCmd(byte baudrate)
{
  mySerial.write(0x56);
  mySerial.write(ZERO);
  mySerial.write(0x24);
  mySerial.write(0x03);
  mySerial.write(0x01);
  mySerial.write(baudrate);
}

void loop()
{
 
byte a[32];
int ii;

  mySerial.begin(38400);
  delay(200);
  SendResetCmd();//Wait 2-3 second to send take picture command
  delay(2000);
  SetBaudRateCmd(0x2A);
  delay(100);
  mySerial.begin(38400);
  delay(100);
  SetImageSizeCmd(0x21);
  delay(100);
  SendTakePhotoCmd();
  delay(3000);
 
//SendResetCmd();
//delay(4000);                            //Wait 2-3 second to send take picture command
 
//SendTakePhotoCmd();

if(mySerial.available()>0)
while(mySerial.available()>0)
{
  incomingbyte=mySerial.read();
  //Serial.println("\
}

myFile = SD.open("pic.jpg", FILE_WRITE); //<strong><span style="color: #ff0000;">The file name should not be too long</span></strong>
 
while(!EndFlag)
{
  j=0;
  k=0;
  count=0;
  SendReadDataCmd();
  delay(5); //20 for regular
  
  while(mySerial.available()>0)
  {
    incomingbyte=mySerial.read();
    k++;
    delayMicroseconds(100);
    
      if((k>5)&&(j<32)&&(!EndFlag))
      {
        a[j]=incomingbyte;
        if((a[j-1]==0xFF)&&(a[j]==0xD9))     //tell if the picture is finished
        EndFlag=1;
        j++;
       count++; 
      }
  }
  
  for(j=0;j<count;j++)
  {
    if(a[j]<0x10) Serial.print("0");
    Serial.print(a[j],HEX);           // observe the image through serial port
    Serial.print(" ");
  }
  
  for(ii=0; ii<count; ii++)
  {
    myFile.write(a[ii]);
    Serial.println();
  }
  myFile.close();
  Serial.print("Finished writing data to file");
  while(1);
}
}
}
