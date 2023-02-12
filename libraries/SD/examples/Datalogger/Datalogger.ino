/*
  SD card datalogger

  This example shows how to log data from three analog sensors
  to an SD card using the SD library.

  The circuit:
   analog sensors on analog ins 0, 1, and 2
   SD card attached to SPI bus as follows (Raspberry Pi Pico):
   ** MISO - pin 4
   ** MOSI - pin 7
   ** CS   - pin 5
   ** SCK  - pin 6

  created  24 Nov 2010
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

#include <SPI.h>
#include <SD.h>

void setup() {
  // Open serial communications and wait for port to open:
  Serial.begin(115200);

  Serial.print("Initializing SD card...");

  // Ensure the SPI pinout the SD card is connected to is configured properly
  SPI.setRX(_MISO);
  SPI.setTX(_MOSI);
  SPI.setSCK(_SCK);

  // see if the card is present and can be initialized:
  if (!SD.begin(_CS)) {
    Serial.println("Card failed, or not present");
    // don't do anything more:
    return;
  }
  Serial.println("card initialized.");
}

void loop() {
  // make a string for assembling the data to log:
  String dataString = "";

  // read three sensors and append to the string:
  for (int analogPin = 0; analogPin < 3; analogPin++) {
    int sensor = analogRead(analogPin);
    dataString += String(sensor);
    if (analogPin < 2) {
      dataString += ",";
    }
  }

  // open the file. note that only one file can be open at a time,
  // so you have to close this one before opening another.
  File dataFile = SD.open("datalog.txt", FILE_WRITE);

  // if the file is available, write to it:
  if (dataFile) {
    dataFile.println(dataString);
    dataFile.close();
    // print to the serial port too:
    Serial.println(dataString);
  }
  // if the file isn't open, pop up an error:
  else {
    Serial.println("error opening datalog.txt");
  }
}









