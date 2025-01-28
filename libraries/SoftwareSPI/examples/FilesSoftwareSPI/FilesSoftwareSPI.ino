/*
  SD card basic file example with Software SPI

  This example shows how to create and destroy an SD card file
  The circuit:
   SD card attached to Pico as follows:
   ** SCK           - GPIO0
   ** CS            - GPIO1
   ** MISO (AKA RX) - GPIO2
   ** MOSI (AKA TX) - GPIO3

  created   Nov 2010
  by David A. Mellis
  modified 9 Apr 2012
  by Tom Igoe

  This example code is in the public domain.
*/

#include <SoftwareSPI.h>

const int _SCK = 0;
const int _CS = 1; // Must be SCK+1 for HW CS support
const int _MISO = 2;
const int _MOSI = 3;
SoftwareSPI softSPI(_SCK, _MISO, _MOSI, _CS);

#include <SD.h>

File myFile;

void setup() {
  // Open serial communications and wait for port to open:
  Serial.begin(115200);

  do {
    delay(100);  // wait for serial port to connect. Needed for native USB port only
  } while (!Serial);

  if (_CS != _SCK + 1) {
    Serial.printf("Error, CS (%d) must be defined as SCK (%d) + 1 \n", _CS, _SCK);
    return;
  }

  Serial.print("Initializing SD card...");

  bool sdInitialized = false;
  sdInitialized = SD.begin(_CS, softSPI);
  if (!sdInitialized) {
    Serial.println("initialization failed!");
    return;
  }
  Serial.println("initialization done.");

  if (SD.exists("example.txt")) {
    Serial.println("example.txt exists.");
  } else {
    Serial.println("example.txt doesn't exist.");
  }

  // open a new file and immediately close it:
  Serial.println("Creating example.txt...");
  myFile = SD.open("example.txt", FILE_WRITE);
  myFile.close();

  // Check to see if the file exists:
  if (SD.exists("example.txt")) {
    Serial.println("example.txt exists.");
  } else {
    Serial.println("example.txt doesn't exist.");
  }

  // delete the file:
  Serial.println("Removing example.txt...");
  SD.remove("example.txt");

  if (SD.exists("example.txt")) {
    Serial.println("example.txt exists.");
  } else {
    Serial.println("example.txt doesn't exist.");
  }
}

void loop() {
  // nothing happens after setup finishes.
}



