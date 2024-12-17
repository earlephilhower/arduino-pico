// Released to the piublic domain by Earle F. Philhower, III in 2024

#include <LittleFS.h>
#include <VFS.h>
#include <SPI.h>
#include <SDFS.h>

// This are GP pins for SPI0 on the Raspberry Pi Pico board, and connect
// to different *board* level pinouts.  Check the PCB while wiring.
// Only certain pins can be used by the SPI hardware, so if you change
// these be sure they are legal or the program will crash.
// See: https://datasheets.raspberrypi.com/picow/PicoW-A4-Pinout.pdf
const int _MISO = 4;  // AKA SPI RX
const int _MOSI = 7;  // AKA SPI TX
const int _CS = 5;
const int _SCK = 6;
// SPI1
//const int _MISO = 8;  // AKA SPI RX
//const int _MOSI = 11;  // AKA SPI TX
//const int _CS = 9;
//const int _SCK = 10;

void setup() {
  delay(5000);
  if (!LittleFS.begin()) {
    Serial.printf("ERROR: Unable to start LittleFS. Did you select a filesystem size in the menus?\n");
    return;
  }

  SDFSConfig cfg;
  bool sd = false;
  if (_MISO == 0 || _MISO == 4 || _MISO == 16) {
    SPI.setRX(_MISO);
    SPI.setTX(_MOSI);
    SPI.setSCK(_SCK);
    SDFS.setConfig(SDFSConfig(_CS, SPI_HALF_SPEED, SPI));
    sd = SDFS.begin();
  } else if (_MISO == 8 || _MISO == 12) {
    SPI1.setRX(_MISO);
    SPI1.setTX(_MOSI);
    SPI1.setSCK(_SCK);
    SDFS.setConfig(SDFSConfig(_CS, SPI_HALF_SPEED, SPI1));
    sd = SDFS.begin();
  } else {
    Serial.println(F("ERROR: Unknown SPI Configuration"));
  }

  VFS.map("/lfs", LittleFS); // Onboard flash at /lfs
  if (sd) {
    VFS.map("/sd", SDFS); // SD card mapped to /sd
  }
  VFS.root(LittleFS); // Anything w/o a prefix maps to LittleFS

  Serial.printf("Writing to /lfs/text.txt\n");
  FILE *f = fopen("/lfs/text.txt", "wb");
  fwrite("hello littlefs", 14, 1, f);
  fclose(f);

  if (sd) {
    Serial.printf("Writing to /sd/test.txt, should not overwrite /lfs/text.txt!\n");
    f = fopen("/sd/text.txt", "wb");
    fwrite("hello sdfs", 10, 1, f);
    fclose(f);
  }

  f = fopen("/lfs/text.txt", "rb");
  char buff[33];
  bzero(buff, 33);
  fread(buff, 1, 32, f);
  fclose(f);
  Serial.printf("READ LFS> '%s'\n", buff);

  if (sd) {
    f = fopen("/sd/text.txt", "rb");
    bzero(buff, 33);
    fread(buff, 1, 32, f);
    fclose(f);
    Serial.printf("READ SDFS> '%s'\n", buff);
  }

  f = fopen("/text.txt", "rb");
  bzero(buff, 33);
  fread(buff, 1, 32, f);
  fclose(f);
  Serial.printf("READ default FS (LittleFS)> '%s'\n", buff);

  Serial.printf("\nTesting seeking within a file\n");
  f = fopen("/lfs/text.txt", "rb");
  for (int i = 0; i < 10; i ++) {
    fseek(f, i, SEEK_SET);
    bzero(buff, 33);
    fread(buff, 1, 32, f);
    Serial.printf("LFS SEEK %d> '%s'\n", i, buff);
  }
  fclose(f);

  Serial.printf("\nTesting fprintf and fgetc from LFS\n");
  f = fopen("/lfs/printout.txt", "w");
  for (int i = 0; i < 10; i++) {
    fprintf(f, "INT: %d\n", i);
  }
  fclose(f);

  Serial.printf("----\n");
  f = fopen("/printout.txt", "r");
  int x;
  while ((x = fgetc(f)) >= 0) {
    Serial.printf("%c", x);
  }
  Serial.printf("----\n");
}

void loop() {
}
