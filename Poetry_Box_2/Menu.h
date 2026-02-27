#include "esp32-hal-gpio.h"
// #DEFINE Fav

void home_screen() {
  lcd.println("GREEN:PRINT RANDOM");
  lcd.println("BLUE:BROWSE HISTORY");
  lcd.println("YELLOW:BROWSE *");
  lcd.println("RED:SHOW CONTROLS");
}

void d_lcd_controls() {
  lcd.print("GREEN:PRINT RANDOM");
  lcd.print("YELLOW:ADD POEM TO *");
  lcd.print("BLUE:REPRINT POEM");
  lcd.print("RED:HOME SCREEN");
}

void d_instruct() {
  lcd.print("YELLOW/BLUE:UP/DOWN");
  lcd.print("GREEN:SELECT");
  lcd.print("RED:HOME SCREEN");
}

//maybe hitting yellow+green adds poet to fav poets?
/*
instructions:
01234567890123456789
GREEN:PRINT RANDOM
YELLOW:ADD POEM TO *
BLUE:REPRINT POEM
RED:HOME SCREEN

history:
01234567890123456789
YELLOW/BLUE:UP/DOWN
GREEN:SELECT
RED:HOME SCREEN
history will just show the past printed poems (should we include randoms?)

* page:
01234567890123456789
VIEW FAVORITE POEMS
VIEW FAVORITE POETS

fav poems page:
PRINT RANDOM FAV
...
list of every favorite
...
SELECT POET TO 
*/


void green_press() {
  Serial.println("Green Press");
}
void yellow_press() {
  Serial.println("Yellow Press");
}
void blue_press() {
  Serial.println("Blue Press");
}
void red_press() {
  Serial.println("Red Press");
}

uint8_t colors[4];
bool last_colors[4];

void check_input() {
  for (int i = 0; i < 4; i++) {
    if (last_colors[i] != digitalRead(colors[i])) {  //if state has changed...
      last_colors[i] = !last_colors[i];              //reflect the change in new state
      if (!last_colors[i]) {                         //and if that new state is low, meaning we are pressed
        switch (i) {
          case 0: green_press(); break;
          case 1: yellow_press(); break;
          case 2: blue_press(); break;
          case 3: red_press(); break; }
      }
    }
  }
}

#define polling_rate 15 //in millis
void poll_input(){
  if(!(millis() % polling_rate)){ check_input(); }
}





