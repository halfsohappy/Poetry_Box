#include <sys/_stdint.h>
#include <cstring>
#include <stdlib.h>


typedef struct author{
  String name;
  int num_poems;
} author;

author* auth_array; // this will hold all authors


//initialize with dir = SD.open("/")
uint16_t count_dir(File dir) {
  uint16_t dir_count = 0;
  while (true) {
    File entry =  dir.openNextFile();
    if (! entry) { break; }
    if (entry.isDirectory()) {
      Serial.print(entry.name());
      dir_count++;
    }
    entry.close();
  }
  dir.rewindDirectory();
}

uint16_t count_files(File dir) {
  uint16_t file_count = 0;
  while (true) {
    File entry =  dir.openNextFile();
    if (! entry) { break; }
    if (!entry.isDirectory()) {
      Serial.print(entry.name());
      file_count++;
    }
    entry.close();
  }
  dir.rewindDirectory();
  dir.close();
}

uint16_t count_lines_in_file(const char *file_name) {
  File file = SD.open(file_name);
  if (!file) {
    return -1;  // Error opening file
  }
  int line_count = 0;
  while (file.available()) {
    char c = file.read();
    if (c == '\n') {
      line_count++;
    }
  }
  file.close();
  return line_count;
}




// void locate_random(){


File gen_rand_poem() { //return a random file based off of auth_array
  uint16_t poem_index = 0;
  uint16_t auth_index = 0;
  File entry;

  long rval = random(total_poems); //of all poems,
  for(int i=0; i<num_auth; i++){ //go through each author
    rval -= auth_array[i].num_poems; //to find the author the poem sits in!
    if(rval < 0){
      poem_index = abs(rval); //get index within author list
      auth_index = i; //get author index within array
      break;
    } 
  }

  File dir = SD.open(auth_array[auth_index].name);  
  for (int i = 0; i <= poem_index; i++) {
    entry =  dir.openNextFile();
  }
  dir.close();
  return entry;
}


void append_h64(File entry){ // take a given file and add it's name to the history, removing oldest record to stay at 64
  File history64 = LittleFS.open("last_64.txt", FILE_APPEND);
  if(count_lines_in_file(history64.name()) >= 64){
    File history642 = LittleFS.open("last_642.txt", FILE_WRITE);

    history64.seek(0);
    history64.find("\n");
    history64.read();

    while(history64.available()){
      history642.write(history64.read());
    }
    history642.println(entry.name());

    history642.close();
    history64.close();
    LittleFS.remove("last_64.txt");
    LittleFS.rename("last_642.txt", "last_64.txt");
  }
  else{
    history64.println(entry.name());
  }
}


File history_check(File entry){ //see if a file is in history, return a new random file if it is
  File history64 = LittleFS.open("last_64.txt", FILE_READ); //open the list of last 64 poems
  File post_check = entry;

  while(!history64.find(post_check.name())){ //while your file is in the history, get new one
    post_check = gen_rand_poem();
  }

  history64.close();
  append_h64(post_check);
  return post_check;
}




char* history_entries[64];

// history_to_array(){
//   File history64 = LittleFS.open("last_64.txt", FILE_READ); //open the list of last 64 poems

// }



//   File favs = SD.open("/favorite poets.txt");
//   char* fav_poets[64];
//   uint8_t num_of_fav = 0;
//   while(favs.available()){
//     fav_poets[num_of_fav] = (char*) malloc(128);
//     favs.readBytesUntil(0x0a, fav_poets[num_of_fav], 128); //read through until new line (ASCII 10)
//     num_of_fav++;
//   }
//   favs.close();


void green_random(){
  File green_bean = history_check(gen_rand_poem());
  append_h64(green_bean);
  while(green_bean.available()){
    printer.write(green_bean.read())
  }
  green_bean.close();
}

//lcd


// print random should:
//gen random
//

