/*
 * Poetry Box 2
 * ESP32 + I2C LCD (20x4) + Adafruit Thermal Printer + SD card (SPI) + 4 buttons
 *
 * Hardware connections:
 *   LCD  : I2C (SDA/SCL = GPIO 21/22 on ESP32)
 *   SD   : SPI  SCK=8  MISO=9  MOSI=10  CS=18
 *   Printer: UART Serial1 (TX=17  RX=16)  9600 baud
 *   Buttons (INPUT_PULLUP, active-LOW):
 *     GREEN  = GPIO 32
 *     YELLOW = GPIO 33
 *     BLUE   = GPIO 25
 *     RED    = GPIO 26
 */

#include "Adafruit_LiquidCrystal.h"
#include "Adafruit_Thermal.h"
#include <SPI.h>
#include <SD.h>
#include "LittleFS.h"

#define FORMAT_LITTLEFS_IF_FAILED true
#define PRINTER_BAUD  9600
#define SD_CS_PIN     18

// LCD: I2C, MCP23008 address offset 0 (A0-A2 not jumpered → 0x20)
Adafruit_LiquidCrystal lcd(0);

// Thermal printer on Serial1
Adafruit_Thermal printer(&Serial1);

#include "Database.h"
#include "Menu.h"

void setup() {
  Serial.begin(115200);
  delay(100);

  // ----- LCD -----
  if (!lcd.begin(20, 4)) {
    Serial.println("LCD init failed");
  }
  lcd.setBacklight(HIGH);
  lcd.clear();
  lcd.print("Poetry Box 2");
  lcd.setCursor(0, 1);
  lcd.print("Initializing...");

  // ----- Thermal printer -----
  Serial1.begin(PRINTER_BAUD);
  printer.begin();

  // ----- SD card (SPI) -----
  SPI.begin(8, 9, 10);  // SCK, MISO, MOSI (CS is managed by SD.begin)
  if (!SD.begin(SD_CS_PIN)) {
    Serial.println("SD init failed");
    lcd.setCursor(0, 2);
    lcd.print("SD CARD ERROR!");
    while (1) { delay(1000); }
  }
  Serial.println("SD ready");

  // ----- LittleFS (internal flash for history & index cache) -----
  if (!LittleFS.begin(FORMAT_LITTLEFS_IF_FAILED)) {
    Serial.println("LittleFS failed");
    lcd.setCursor(0, 2);
    lcd.print("FLASH FS ERROR!");
    while (1) { delay(1000); }
  }
  Serial.println("LittleFS ready");

  // ----- Buttons -----
  init_buttons();

  // ----- Load poem index (from LittleFS cache or scan SD) -----
  lcd.setCursor(0, 2);
  lcd.print("Loading poems...");
  load_index();
  init_history();

  Serial.printf("Loaded %d authors, %d poems total\n", num_auth, total_poems);

  // ----- Show home screen -----
  show_home();
}

void loop() {
  handle_input();
}
