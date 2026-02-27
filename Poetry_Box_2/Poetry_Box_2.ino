//Created File
#include "Adafruit_LiquidCrystal.h"
#include "Adafruit_Thermal.h"
#include <SPI.h>
#include <SD.h>
#define FORMAT_LITTLEFS_IF_FAILED true
#define printer_baud 9600



// Connect via i2c, default address #0 (A0-A2 not jumpered)
Adafruit_LiquidCrystal lcd(0); //input is offset from 20
Adafruit_Thermal printer(&Serial1);     // Pass addr to printer constructor

uint16_t total_poems;
uint16_t num_auth;
#include "Database.h"




void setup(){
//INITIALIZE ALL SYSTEMS (I2C LCD, SPI SD CARD, SERIAL PRINTER, LITTLEFS)
  if (!lcd.begin(20, 4)) {
    Serial.println("failed lcd init");
  //  while(1);
  }
  Serial.println("i2c lcd init :)");

  Serial.begin(115200); //virtual, usb serial
  delay(100);


  Serial1.begin(printer_baud); //UART on pins 0,1


 // SPI.begin(12,13,11,10);
  SPI.begin(8, 9, 10,18)
  if (!SD.begin()) {
    Serial.println("failed SD init");
    return;
  }
  Serial.println("SD init");

  if(!LittleFS.begin(FORMAT_LITTLEFS_IF_FAILED)){
        Serial.println("LittleFS Mount Failed");
        return;
    }
    
//CREATE AUTHOR ARRAY/DIRECTORY, POPULATE WITH AUTHOR STRUCTS
  File root = SD.open("/");
  num_auth = count_dir(root);
  author auth_array[num_auth];
  File histemp;

  //this creates array of authors and populates them
  for(int i = 0; i < num_auth-1; i++){
    File entry =  root.openNextFile();
    if (entry.isDirectory() && !strcmp(entry.name(), "Favorites") && entry.name()[0] != '.') {
      auth_array[i].name = entry.name();
      auth_array[i].num_poems = count_files(entry.openNextFile());
      total_poems += auth_array[i].num_poems;
      if(!LittleFS.exists(entry.name())){
        histemp = LittleFS.open(entry.name(), FILE_WRITE);
        //histemp.println("init");
        histemp.close();
      }
    }
  }

  if(!LittleFS.exists("last_64.txt")){ //create history file in littlefs if doesn't exist
    histemp = LittleFS.open("last_64.txt", FILE_WRITE);
    //histemp.println("init");
    histemp.close();
  }

//SETUP FAVORITE POETS (READ FAV POETS SD FILE AND PUT INTO ARRAY OF STRING CHAR POINTERS TO NAMES OF POETS)
  File favs = SD.open("/favorite_poets.txt");
  char* fav_poets[64];
  uint8_t num_of_fav = 0;
  while(favs.available()){
    fav_poets[num_of_fav] = (char*) malloc(128);
    favs.readBytesUntil(0x0a, fav_poets[num_of_fav], 128); //read through until new line (ASCII 10)
    num_of_fav++;
  }
  favs.close();

//test code
  File test_random_poem = gen_rand_poem();
  test_random_poem = history_check(test_random_poem);
  while(test_random_poem.available()){
    Serial1.write(test_random_poem.read());
  }
  test_random_poem.close();

}

void loop(){


}