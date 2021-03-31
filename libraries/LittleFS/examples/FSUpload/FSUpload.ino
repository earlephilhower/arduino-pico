// Released to the public domain
//
// This sketch will read an uploaded file and increment a counter file
// each time the sketch is booted.

// Be sure to install the Pico LittleFS Data Upload extension for the
// Arduino IDE from:
//    https://github.com/earlephilhower/arduino-pico-littlefs-plugin/
// The latest release is available from:
//    https://github.com/earlephilhower/arduino-pico-littlefs-plugin/releases

// Before running:
// 1) Select Tools->Flash Size->(some size with a FS/filesystem)
// 2) Use the Tools->Pico Sketch Data Upload tool to transfer the contents of
//    the sketch data directory to the Pico

#include <LittleFS.h>

void setup() {
  Serial.begin(115200);
  delay(5000);
  LittleFS.begin();
  char buff[32];
  int cnt = 1;
  File f = LittleFS.open("counts.txt", "r");
  if (f) {
    bzero(buff, 32);
    if (f.read((uint8_t *)buff, 31)) {
      sscanf(buff, "%d", &cnt);
      Serial.printf("I have been run %d times\n", cnt);
    }
    f.close();
  }

  cnt++;
  sprintf(buff, "%d\n", cnt);
  f = LittleFS.open("counts.txt", "w");
  if (f) {
    f.write(buff, strlen(buff));
    f.close();
  }

  Serial.println("---------------");
  File i = LittleFS.open("file1.txt", "r");
  if (i) {
    while (i.available()) {
      Serial.write(i.read());
    }
    Serial.println("---------------");
    i.close();
  }
}


void loop() {
}
