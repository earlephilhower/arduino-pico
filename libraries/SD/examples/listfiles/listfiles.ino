/*
  Listfiles

  This example shows how print out the files in a
  directory on a SD card

  The circuit:
   SD card attached to SPI bus as follows on RP2040:
   ************ SPI0 ************
   ** MISO (AKA RX) - pin 0, 4, or 16
   ** MOSI (AKA TX) - pin 3, 7, or 19
   ** CS            - pin 1, 5, or 17
   ** SCK           - pin 2, 6, or 18
   ************ SPI1 ************
   ** MISO (AKA RX) - pin  8 or 12
   ** MOSI (AKA TX) - pin 11 or 15
   ** CS            - pin  9 or 13
   ** SCK           - pin 10 or 14

  created   Nov 2010
  by David A. Mellis
  modified 9 Apr 2012
  by Tom Igoe
  modified 2 Feb 2014
  by Scott Fitzgerald
  modified 12 Feb 2023
  by Earle F. Philhower, III
  modified 26 Dec 2023
  by Richard Teel

  This example code is in the public domain.

*/

// This are GP pins for SPI0 on the Raspberry Pi Pico board, and connect
// to different *board* level pinouts.  Check the PCB while wiring.
// Only certain pins can be used by the SPI hardware, so if you change
// these be sure they are legal or the program will crash.
// See: https://datasheets.raspberrypi.com/picow/PicoW-A4-Pinout.pdf
const int _MISO = 4;  // AKA SPI RX
const int _MOSI = 7;  // AKA SPI TX
const int _CS = 5;
const int _SCK = 6;

// If you have all 4 DAT pins wired up to the Pico you can use SDIO mode
const int RP_CLK_GPIO = -1; // Set to CLK GPIO
const int RP_CMD_GPIO = -1; // Set to CMD GPIO
const int RP_DAT0_GPIO = -1; // Set to DAT0 GPIO. DAT1..3 must be consecutively connected.

#include <SPI.h>
#include <SD.h>

File root;

void setup() {
  // Open serial communications and wait for port to open:
  Serial.begin(115200);

  while (!Serial) {
    delay(1);  // wait for serial port to connect. Needed for native USB port only
  }

  Serial.println("\nInitializing SD card...");

  bool sdInitialized = false;
  if (RP_CLK_GPIO >= 0) {
    // No special requirements on pin locations, this is PIO programmed
    sdInitialized = SD.begin(RP_CLK_GPIO, RP_CMD_GPIO, RP_DAT0_GPIO);
  } else {
    // Ensure the SPI pinout the SD card is connected to is configured properly
    // Select the correct SPI based on _MISO pin for the RP2040
    if (_MISO == 0 || _MISO == 4 || _MISO == 16) {
      SPI.setRX(_MISO);
      SPI.setTX(_MOSI);
      SPI.setSCK(_SCK);
      sdInitialized = SD.begin(_CS);
    } else if (_MISO == 8 || _MISO == 12) {
      SPI1.setRX(_MISO);
      SPI1.setTX(_MOSI);
      SPI1.setSCK(_SCK);
      sdInitialized = SD.begin(_CS, SPI1);
    } else {
      Serial.println(F("ERROR: Unknown SPI Configuration"));
      return;
    }
  }

  if (!sdInitialized) {
    Serial.println("initialization failed!");
    return;
  }
  Serial.println("initialization done.");

  root = SD.open("/");

  printDirectory(root, 0);

  Serial.println("done!");
}

void loop() {
  // nothing happens after setup finishes.
}

void printDirectory(File dir, int numTabs) {
  while (true) {

    File entry = dir.openNextFile();
    if (!entry) {
      // no more files
      break;
    }
    for (uint8_t i = 0; i < numTabs; i++) {
      Serial.print('\t');
    }
    Serial.print(entry.name());
    if (entry.isDirectory()) {
      Serial.println("/");
      printDirectory(entry, numTabs + 1);
    } else {
      // files have sizes, directories do not
      Serial.print("\t\t");
      Serial.print(entry.size(), DEC);
      time_t cr = entry.getCreationTime();
      time_t lw = entry.getLastWrite();
      struct tm* tmstruct = localtime(&cr);
      Serial.printf("\tCREATION: %d-%02d-%02d %02d:%02d:%02d", (tmstruct->tm_year) + 1900, (tmstruct->tm_mon) + 1, tmstruct->tm_mday, tmstruct->tm_hour, tmstruct->tm_min, tmstruct->tm_sec);
      tmstruct = localtime(&lw);
      Serial.printf("\tLAST WRITE: %d-%02d-%02d %02d:%02d:%02d\n", (tmstruct->tm_year) + 1900, (tmstruct->tm_mon) + 1, tmstruct->tm_mday, tmstruct->tm_hour, tmstruct->tm_min, tmstruct->tm_sec);
    }
    entry.close();
  }
}
