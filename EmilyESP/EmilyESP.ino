#include "SD.h"
#include "SPI.h"

#include "Adafruit_Thermal.h"
#include "SoftwareSerial.h"
#define TX_PIN A1 // Arduino transmit  GREEN
#define RX_PIN A2 // Arduino receive   BLUE 

SoftwareSerial mySerial(RX_PIN, TX_PIN); // Declare SoftwareSerial obj first
Adafruit_Thermal printer(&mySerial);     // Pass addr to printer constructor

int BUTTON_R = 4;
int BUTTON_L = 2;
int RING_LED = 0;

int poemPosition;
int secretMode = 0;
String track = String("/TRACKING.TXT");
String nameStart = String("/emily");
String nameEnd = String(".txt");
String fileName = nameStart + poemPosition + nameEnd;

void readFile(fs::FS &fs, String fileNameTemp){

    File tempFile = fs.open(fileNameTemp);
    if(!tempFile){
        Serial.println("Failed to open file for reading");
        return;
    }

    Serial.print("Read from file: ");
    while(tempFile.available()){
        Serial.write(tempFile.read());
    }
    tempFile.close();
}

void writeFile(fs::FS &fs, String path, int message){
    Serial.printf("Writing file: %s\n", path);

    File file = fs.open(path, FILE_WRITE);
    if(!file){
        Serial.println("Failed to open file for writing");
        return;
    }
    if(file.print(message)){
        Serial.println("File written");
    } else {
        Serial.println("Write failed");
    }
    file.close();
}

void thermalPrintFile(fs::FS &fs, String fileNameTemp){

    File thermFile = fs.open(fileNameTemp);
     printer.setFont('B');
    while(thermFile.available()){
        printer.write(thermFile.read());
    }
    thermFile.close();
}

int recallPosition(){
  int tempPos;
  File posFile = SD.open(track);
    if(!posFile){
        Serial.println("Failed to open file for reading");
        return tempPos;
    }

    Serial.print("Read from file: ");
    while(posFile.available()){
       tempPos = posFile.parseInt();
       Serial.println(tempPos);
    }
    posFile.close();
    return tempPos;
}

int increasedPosition(){
  int tempIncPos = recallPosition();
  tempIncPos++;
  return tempIncPos;
}

void setup() {
  // put your setup code here, to run once:
pinMode(BUTTON_R, INPUT_PULLUP);
pinMode(BUTTON_L, INPUT_PULLUP);
pinMode(RING_LED, OUTPUT);
  mySerial.begin(9600);
  printer.begin();
  printer.setFont('B');
  transRights();
  //printer.println("testTESTTESTTEST");
  Serial.begin(115200);
    if(!SD.begin()){
        printer.println("Card Mount Failed");
        return;
    }
    Serial.println(fileName);
    digitalWrite(RING_LED, HIGH);
    Serial.println(recallPosition());
    delay(1000);
  //  writeFile(SD, track, 1);
//readFile(SD, fileName);

//writeFile(SD, track, increasedPosition());
//thermalPrintFile(SD, fileName);
if(digitalRead(BUTTON_R) == LOW){
   delay(1500);
    digitalWrite(RING_LED, LOW);
    delay(1500);
          if(digitalRead(BUTTON_R) == LOW){
               secretMode = 100;
               printer.setSize('L');
               printer.println("SECRET MODE");
               printer.println("ACTIVATED");
               printer.feed(4);
               printer.setSize('S');
               
                                              }
                                               }}
                                              
void loop() {
int buttonStateL = digitalRead(BUTTON_L);
 int buttonStateR = digitalRead(BUTTON_R);
  Serial.println(buttonStateR);

 switch (secretMode){
  case 0:
    if(buttonStateL == LOW && buttonStateR == HIGH){
      Serial.println("L PRESS");
      fileName = nameStart + recallPosition() + nameEnd;
        printer.setFont('B');
      thermalPrintFile(SD, fileName);
      printer.feed(4);
    }
    if(buttonStateR == LOW && buttonStateL == HIGH){
      Serial.println("R PRESS");
      fileName = nameStart + increasedPosition() + nameEnd;
        printer.setFont('B');
      thermalPrintFile(SD, fileName);
      printer.feed(4);
      writeFile(SD, track, increasedPosition());
    }

    Serial.println(buttonStateL);
    Serial.println(buttonStateR);
    break;

    case 100:
    digitalWrite(RING_LED, HIGH);
    delay(4000);
    digitalWrite(RING_LED, LOW);
    while(digitalRead(BUTTON_R) == HIGH){
digitalRead(BUTTON_R);
     Serial.println(buttonStateR);
    }
    secretMode = 101;
    delay(1000);
    break;

    case 101:
    digitalWrite(RING_LED, HIGH);
    delay(500);
    digitalWrite(RING_LED, LOW);
    while(digitalRead(BUTTON_R) == HIGH){
    digitalRead(BUTTON_R);
    if(digitalRead(BUTTON_L) == LOW){
      raven();
    }
    }
    secretMode = 102;
    delay(1000);
    break;
    
    
    case 102:
    digitalWrite(RING_LED, HIGH);
    delay(500);
    digitalWrite(RING_LED, LOW);
    delay(500);
    digitalWrite(RING_LED, HIGH);
    delay(500);
    digitalWrite(RING_LED, LOW);
    while(digitalRead(BUTTON_R) == HIGH){
    digitalRead(BUTTON_R);
    if(digitalRead(BUTTON_L) == LOW){
      navy();
    }
    }
    secretMode = 103;
    delay(1000);
    break;

     case 103:
    digitalWrite(RING_LED, HIGH);
    delay(500);
    digitalWrite(RING_LED, LOW);
    delay(500);
    digitalWrite(RING_LED, HIGH);
    delay(500);
    digitalWrite(RING_LED, LOW);
    delay(500);
    digitalWrite(RING_LED, HIGH);
    delay(500);
    digitalWrite(RING_LED, LOW);
    while(digitalRead(BUTTON_R) == HIGH){
    digitalRead(BUTTON_R);
    if(digitalRead(BUTTON_L) == LOW){
      transRights();
    }
    }
    secretMode = 101;
    delay(1000);
    break;
    
}


}
