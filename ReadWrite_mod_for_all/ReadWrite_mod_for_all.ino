
/*
  SD card read/write

  This example shows how to read and write data to and from an SD card file
  The circuit:
   SD card attached to SPI bus as follows:
 ** MOSI - pin 11
 ** MISO - pin 12
 ** CLK - pin 13
 ** CS - pin 4 (for MKRZero SD: SDCARD_SS_PIN)

  created   Nov 2010
  by David A. Mellis
  modified 9 Apr 2012
  by Tom Igoe

  This example code is in the public domain.

*/

#include <SPI.h>
#include <SD.h>
int C;
File myFile;
String counter = "fi";
String ender = ".txt";
String place = "00";

#include <LiquidCrystal.h>

// initialize the library by associating any needed LCD interface pin
// with the arduino pin number it is connected to
//const int rs = 9, en = 10, d4 = 5, d5 = 6, d6 = 3, d7 = 2;
//LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

const int buttonPin = 8;     // the number of the pushbutton pin
int buttonState = 0; 


void setup() {
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  pinMode(buttonPin, INPUT);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
C = 0;
String file = "fi" + place + C + ".txt";

 // Serial.print("Initializing SD card...");

  if (!SD.begin(4)) {
   Serial.println("initialization failed!");
    while (1);
  }
  //myFile = SD.open(file, FILE_WRITE);

  // if the file opened okay, write to it:
 // if (myFile) {
   // Serial.print("Writing to test.txt...");
    //myFile.println("testing 1, 2, 3.");
    // close the file:
    //myFile.close();    }

}

void loop() {
  String file = counter + place + C + ender;
   myFile = SD.open(file);
  if (myFile) {
    //Serial.println(file);

    // read from the file until there's nothing else in it:
    while (myFile.available()) {
      Serial.write(myFile.read());
    }
    // close the file:
    myFile.close();
  } else {
    // if the file didn't open, print an error:
  //  Serial.println("error opening test.txt");
  }
buttonState = digitalRead(buttonPin);

  // check if the pushbutton is pressed. If it is, the buttonState is HIGH:
  while (buttonState != HIGH) { buttonState = digitalRead(buttonPin);}
  C++;

  delay(1);
  if (C == 10) {
    place = "0";}
    else {}
  if (C == 100) {
    place = "";}
    else {}
  }
