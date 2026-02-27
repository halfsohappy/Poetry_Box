#include <SPI.h>
#include <SD.h>
#include "FS.h"
#include "Adafruit_Thermal.h"
#include "SoftwareSerial.h"
#define TX_PIN 0 
#define RX_PIN 1 //remember TX goes to RX and vice versa


const int buttonPin = 3; // the number of the pushbutton pin
int buttonState = 0; 
const int SD_PIN = 5;
File currentPoem;
File storedPosition;
//pinMode(3, OUTPUT);
//digitalWrite(3, HIGH);
int p = 0;
String nameStart = String("emily");
String nameEnd = String(".txt");
String fileName = nameStart + p + nameEnd;
SoftwareSerial mySerial(RX_PIN, TX_PIN); // Declare SoftwareSerial obj first
Adafruit_Thermal printer(&mySerial);     // Pass addr to printer constructor


void waitForButton()
{
  buttonState = digitalRead(buttonPin);
  // check if the pushbutton is pressed. If it is, the buttonState is HIGH:
  while (buttonState != HIGH) {buttonState = digitalRead(buttonPin);}
  Serial.println("Button Pressed!");
}

void printPoemMonitor()
{
  String fileName = nameStart + p + nameEnd;
 // Serial.println(fileName);
  currentPoem = SD.open(fileName);
  if (currentPoem) {
    // read from the file until there's nothing else in it:
      while (currentPoem.available())
        {
          Serial.write(currentPoem.read());
        }
    // close the file:
     currentPoem.close();
  }
}

void recallPositionSD()
{
   storedPosition = SD.open("TRACKING.txt");
  if (storedPosition) {

    // read from the file until there's nothing else in it:
    while (storedPosition.available()) {
      p = storedPosition.parseInt();
    //  Serial.println(p);
    }
    // close the file:
    storedPosition.close();
  } else {
    // if the file didn't open, print an error:
    Serial.println("error opening tracking.txt");
  }
}

void deleteOldPosition()
{
  if (SD.exists("tracking.txt")) {
      SD.remove("tracking.txt");
   }
  else{
    Serial.println("tracking.txt doesn't exist.");
  }
}
void savePositionSD(int saver)
{
  storedPosition = SD.open("TRACKING.txt", FILE_WRITE);
  storedPosition.print(String(saver));
  storedPosition.close();
}

void printPoemThermal()
{
  printer.setFont('A');
   printer.boldOn();
  String fileName = nameStart + p + nameEnd;
  currentPoem = SD.open(fileName);
  if (currentPoem) {
    // read from the file until there's nothing else in it:
      while (currentPoem.available())
        {
          printer.write(currentPoem.read());
        }
    // close the file:
     currentPoem.close();
     printer.println("");
       printer.feed(2);
  }
}
void setup() {
  // Open serial communications and wait for port to open:
  pinMode(3, OUTPUT);
digitalWrite(3, HIGH);
delay(1000);
  Serial.begin(9600);
  pinMode(buttonPin, INPUT_PULLUP);
  mySerial.begin(19200);
  printer.begin();
  while (!Serial) {}
    ; // wait for serial port to connect. Needed for native USB port only
      
  if (!SD.begin(SD_PIN)) {
    Serial.println("initialization failed!");
    while (1);
  }

}

void loop() {
 //   waitForButton();
 // recallPositionSD();
  //Serial.println("position recalled!");
  p++;
   //Serial.println("count increased!");
  printPoemMonitor();
  Serial.print("--------------------------------------------");
    //Serial.println("printed to serial!");
 //printPoemThermal();
   //Serial.println("printed to thermal!");
  deleteOldPosition();
    //Serial.println("deleted old position!");
  savePositionSD(p);
  //  Serial.println("saved position!");
}
