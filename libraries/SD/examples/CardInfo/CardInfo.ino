/*
  SD card test

  This example shows how use the utility libraries on which the'
  SD library is based in order to get info about your SD card.
  Very useful for testing a card when you're not sure whether its working or not.

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

  created  28 Mar 2011
  by Limor Fried
  modified 9 Apr 2012
  by Tom Igoe
  modified 26 Dec 2023
  by Richard Teel from code provided by Renzo Mischianti
    SOURCE: https://mischianti.org/raspberry-pi-pico-and-rp2040-boards-how-to-use-sd-card-5/
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

// include the SD library:
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
    Serial.println("initialization failed. Things to check:");
    Serial.println("* is a card inserted?");
    Serial.println("* is your wiring correct?");
    Serial.println("* did you change the chipSelect pin to match your shield or module?");
    return;
  } else {
    Serial.println("Wiring is correct and a card is present.");
  }
  // 0 - SD V1, 1 - SD V2, or 3 - SDHC/SDXC
  // print the type of card
  Serial.println();
  Serial.print("Card type:         ");
  switch (SD.type()) {
    case 0:
      Serial.println("SD1");
      break;
    case 1:
      Serial.println("SD2");
      break;
    case 3:
      Serial.println("SDHC/SDXC");
      break;
    default:
      Serial.println("Unknown");
  }

  Serial.print("Cluster size:          ");
  Serial.println(SD.clusterSize());
  Serial.print("Blocks x Cluster:  ");
  Serial.println(SD.blocksPerCluster());
  Serial.print("Blocks size:  ");
  Serial.println(SD.blockSize());

  Serial.print("Total Blocks:      ");
  Serial.println(SD.totalBlocks());
  Serial.println();

  Serial.print("Total Cluster:      ");
  Serial.println(SD.totalClusters());
  Serial.println();

  // print the type and size of the first FAT-type volume
  uint64_t volumesize;
  Serial.print("Volume type is:    FAT");
  Serial.println(SD.fatType(), DEC);

  volumesize = SD.totalClusters();
  volumesize *= SD.clusterSize();
  volumesize /= 1000;
  Serial.print("Volume size (Kb):  ");
  Serial.println(volumesize);
  Serial.print("Volume size (Mb):  ");
  volumesize /= 1024;
  Serial.println(volumesize);
  Serial.print("Volume size (Gb):  ");
  Serial.println((float)volumesize / 1024.0);

  Serial.print("Card size:  ");
  Serial.println((float)SD.size64() / 1000);

  FSInfo fs_info;
  SDFS.info(fs_info);

  Serial.print("Total bytes: ");
  Serial.println(fs_info.totalBytes);

  Serial.print("Used bytes: ");
  Serial.println(fs_info.usedBytes);

  root = SD.open("/");
  printDirectory(root, 0);
}

void loop(void) {
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
