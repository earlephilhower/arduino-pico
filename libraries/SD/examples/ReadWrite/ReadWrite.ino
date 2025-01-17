/*
  SD card read/write

  This example shows how to read and write data to and from an SD card file
  The circuit:
   SD card attached to SPI bus as follows:
   ** MISO - pin 4
   ** MOSI - pin 7
   ** CS   - pin 5
   ** SCK  - pin 6

  created   Nov 2010
  by David A. Mellis
  modified 9 Apr 2012
  by Tom Igoe

  This example code is in the public domain.

*/

// This are GP pins for SPI0 on the Raspberry Pi Pico board, and connect
// to different *board* level pinouts.  Check the PCB while wiring.
// Only certain pins can be used by the SPI hardware, so if you change
// these be sure they are legal or the program will crash.
// See: https://datasheets.raspberrypi.com/picow/PicoW-A4-Pinout.pdf
const int _MISO = 4;
const int _MOSI = 7;
const int _CS = 5;
const int _SCK = 6;

// If you have all 4 DAT pins wired up to the Pico you can use SDIO mode
const int RP_CLK_GPIO = -1; // Set to CLK GPIO
const int RP_CMD_GPIO = -1; // Set to CMD GPIO
const int RP_DAT0_GPIO = -1; // Set to DAT0 GPIO. DAT1..3 must be consecutively connected.

#include <SPI.h>
#include <SD.h>

File myFile;

void setup() {
  // Open serial communications and wait for port to open:
  Serial.begin(115200);

  Serial.print("Initializing SD card...");

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

  // open the file. note that only one file can be open at a time,
  // so you have to close this one before opening another.
  myFile = SD.open("test.txt", FILE_WRITE);

  // if the file opened okay, write to it:
  if (myFile) {
    Serial.print("Writing to test.txt...");
    myFile.println("testing 1, 2, 3.");
    // close the file:
    myFile.close();
    Serial.println("done.");
  } else {
    // if the file didn't open, print an error:
    Serial.println("error opening test.txt");
  }

  // re-open the file for reading:
  myFile = SD.open("test.txt");
  if (myFile) {
    Serial.println("test.txt:");

    // read from the file until there's nothing else in it:
    while (myFile.available()) {
      Serial.write(myFile.read());
    }
    // close the file:
    myFile.close();
  } else {
    // if the file didn't open, print an error:
    Serial.println("error opening test.txt");
  }
}

void loop() {
  // nothing happens after setup
}


