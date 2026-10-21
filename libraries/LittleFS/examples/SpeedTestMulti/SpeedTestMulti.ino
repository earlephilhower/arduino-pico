// Simple speed test for filesystem objects
// Released to the public domain by Earle F. Philhower, III

#include <FS.h>
#include <LittleFS.h>
#include <SDFS.h>

// Choose the filesystem to test
// WARNING:  The filesystem will be formatted at the start of the test!

#define TESTFS LittleFS
//#define TESTFS SDFS

// How large of a file to test
#define TESTSIZEKB 256

// Format speed in bytes/second.  Static buffer so not re-entrant safe
const char *rate(unsigned long start, unsigned long stop, unsigned long bytes) {
  static char buff[64];
  if (stop == start) {
    strcpy_P(buff, PSTR("Inf b/s"));
  } else {
    unsigned long delta = stop - start;
    float r = 1000.0f * (float)bytes / (float)delta;
    if (r >= 1000000.0f) {
      sprintf_P(buff, PSTR("%0.2f MB/s"), (double)r / 1000000.0);
    } else if (r >= 1000.0f) {
      sprintf_P(buff, PSTR("%0.2f KB/s"), (double)r / 1000.0);
    } else {
      sprintf_P(buff, PSTR("%d bytes/s"), (int)r);
    }
  }
  return buff;
}


void DoTest(FS *fs, int id) {
  Serial.printf("%d: Beginning test\n", id);
  Serial.flush();
  if (!fs->begin()) {
    Serial.printf("Unable to begin(), aborting\n");
    return;
  }
  uint8_t data[256];
  for (int i = 0; i < 256; i++) {
    data[i] = (uint8_t) i;
  }

  Serial.printf("%d: Creating %dKB file, may take a while...\n", id, TESTSIZEKB);
  unsigned long start = millis();
  String filename = "/testwrite";
  filename += id;
  filename += ".bin";
  File f = fs->open(filename, "w");
  if (!f) {
    Serial.printf("%d: Unable to open file for writing, aborting\n", id);
    return;
  }
  for (int i = 0; i < TESTSIZEKB; i++) {
    for (int j = 0; j < 4; j++) {
      f.write(data, 256);
    }
  }
  f.close();
  unsigned long stop = millis();
  Serial.printf("%d: ==> Time to write %dKB in 256b chunks = %lu milliseconds\n", id, TESTSIZEKB, stop - start);

  f = fs->open(filename, "r");
  Serial.printf("%d: ==> Created file size = %zu\n", id, f.size());
  f.close();

  Serial.printf("%d: Reading %dKB file sequentially in 256b chunks\n", id, TESTSIZEKB);
  start = millis();
  f = fs->open(filename, "r");
  for (int i = 0; i < TESTSIZEKB; i++) {
    for (int j = 0; j < 4; j++) {
      f.read(data, 256);
    }
  }
  f.close();
  stop = millis();
  Serial.printf("%d: ==> Time to read %dKB sequentially in 256b chunks = %lu milliseconds = %s\n", id, TESTSIZEKB, stop - start, rate(start, stop, TESTSIZEKB * 1024));

  Serial.printf("%d: Reading %dKB file MISALIGNED in flash and RAM sequentially in 256b chunks\n", id, TESTSIZEKB);
  start = millis();
  f = fs->open(filename, "r");
  f.read();
  for (int i = 0; i < TESTSIZEKB; i++) {
    for (int j = 0; j < 4; j++) {
      f.read(data + 1, 256);
    }
  }
  f.close();
  stop = millis();
  Serial.printf("%d: ==> Time to read %dKB sequentially MISALIGNED in flash and RAM in 256b chunks = %lu milliseconds = %s\n", id, TESTSIZEKB, stop - start, rate(start, stop, TESTSIZEKB * 1024));

  Serial.printf("%d: Reading %dKB file in reverse by 256b chunks\n", id, TESTSIZEKB);
  start = millis();
  f = fs->open(filename, "r");
  for (int i = 0; i < TESTSIZEKB; i++) {
    for (int j = 0; j < 4; j++) {
      if (!f.seek(-256 - 256 * j * i, SeekEnd)) {
        Serial.printf("%d: Unable to seek to %d, aborting\n", id, -256 - 256 * j * i);
        return;
      }
      if (256 != f.read(data, 256)) {
        Serial.printf("%d: Unable to read 256 bytes, aborting\n", id);
        return;
      }
    }
  }
  f.close();
  stop = millis();
  Serial.printf("%d: ==> Time to read %dKB in reverse in 256b chunks = %lu milliseconds = %s\n", id, TESTSIZEKB, stop - start, rate(start, stop, TESTSIZEKB * 1024));

  Serial.printf("%d: Writing 64K file in 1-byte chunks\n", id);
  start = millis();
  f = fs->open(filename, "w");
  for (int i = 0; i < 65536; i++) {
    f.write((uint8_t*)&i, 1);
  }
  f.close();
  stop = millis();
  Serial.printf("%d: ==> Time to write 64KB in 1b chunks = %lu milliseconds = %s\n", id, stop - start, rate(start, stop, 65536));

  Serial.printf("%d: Reading 64K file in 1-byte chunks\n", id);
  start = millis();
  f = fs->open(filename, "r");
  for (int i = 0; i < 65536; i++) {
    char c;
    f.read((uint8_t*)&c, 1);
  }
  f.close();
  stop = millis();
  Serial.printf("%d: ==> Time to read 64KB in 1b chunks = %lu milliseconds = %s\n", id, stop - start, rate(start, stop, 65536));

  Serial.printf("%d: Test done\n", id);
}

void setup() {
  Serial.begin(115200);
  if (!TESTFS.format()) {
    Serial.printf("Unable to format(), aborting\n");
    return;
  }
#ifdef __FREERTOS
  TaskHandle_t task;
  xTaskCreate(setup2, "setup2", 1024, NULL, configMAX_PRIORITIES / 2, &task);
#endif
  delay(5000);
  DoTest(&TESTFS, 0);
}

void loop() {
  delay(10000);
}

void setup1() {
  delay(5000);
  DoTest(&TESTFS, 1);
}

void setup2(void *param) {
  (void) param;
  delay(5000);
  DoTest(&TESTFS, 2);
}
