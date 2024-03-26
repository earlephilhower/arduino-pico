// FatFS + FatFSUSB listFiles example

#include <FatFS.h>
#include <FatFSUSB.h>

volatile bool updated = false;
volatile bool driveConnected = false;
volatile bool inPrinting = false;

// Called by FatFSUSB when the drive is released.  We note this, restart FatFS, and tell the main loop to rescan.
void unplug(uint32_t i) {
  (void) i;
  driveConnected = false;
  updated = true;
  FatFS.begin();
}

// Called by FatFSUSB when the drive is mounted by the PC.  Have to stop FatFS, since the drive data can change, note it, and continue.
void plug(uint32_t i) {
  (void) i;
  driveConnected = true;
  FatFS.end();
}

// Called by FatFSUSB to determine if it is safe to let the PC mount the USB drive.  If we're accessing the FS in any way, have any Files open, etc. then it's not safe to let the PC mount the drive.
bool mountable(uint32_t i) {
  (void) i;
  return !inPrinting;
}

void setup() {
  Serial.begin(115200);
  while (!Serial) {
    delay(1);
  }
  delay(5000);

  if (!FatFS.begin()) {
    Serial.println("FatFS initialization failed!");
    while (1) {
      delay(1);
    }
  }
  Serial.println("FatFS initialization done.");

  inPrinting = true;
  printDirectory("/", 0);
  inPrinting = false;

  // Set up callbacks
  FatFSUSB.onUnplug(unplug);
  FatFSUSB.onPlug(plug);
  FatFSUSB.driveReady(mountable);
  // Start FatFS USB drive mode
  FatFSUSB.begin();
  Serial.println("FatFSUSB started.");
  Serial.println("Connect drive via USB to upload/erase files and re-display");
}

void loop() {
  if (updated && !driveConnected) {
    inPrinting = true;
    Serial.println("\n\nDisconnected, new file listing:");
    printDirectory("/", 0);
    updated = false;
    inPrinting = false;
  }
}

void printDirectory(String dirName, int numTabs) {
  Dir dir = FatFS.openDir(dirName);

  while (true) {

    if (!dir.next()) {
      // no more files
      break;
    }
    for (uint8_t i = 0; i < numTabs; i++) {
      Serial.print('\t');
    }
    Serial.print(dir.fileName());
    if (dir.isDirectory()) {
      Serial.println("/");
      printDirectory(dirName + "/" + dir.fileName(), numTabs + 1);
    } else {
      // files have sizes, directories do not
      Serial.print("\t\t");
      Serial.print(dir.fileSize(), DEC);
      time_t cr = dir.fileCreationTime();
      struct tm* tmstruct = localtime(&cr);
      Serial.printf("\t%d-%02d-%02d %02d:%02d:%02d\n", (tmstruct->tm_year) + 1900, (tmstruct->tm_mon) + 1, tmstruct->tm_mday, tmstruct->tm_hour, tmstruct->tm_min, tmstruct->tm_sec);
    }
  }
}
