/*
  PSRAM Test

  This section of code tests the onboard ram of RP2350 based boards with external
  PSRAM.

  This example code is in the public domain.

*/

#if !defined(RP2350_PSRAM_CS)

void setup() {
  Serial.println("This example needs an RP2350 with PSRAM attached");
}

void loop() {
}

#else

#define CHUNK_SIZE 131072
#define PMALLOCSIZE (CHUNK_SIZE * 13)
uint8_t tmp[CHUNK_SIZE];
uint8_t mems[1024 * 1024 * 1] PSRAM;

// the setup function runs once when you press reset or power the board
void setup() {
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(LED_BUILTIN, OUTPUT);
  while (!Serial) {
    delay(10);
  }
  Serial.begin(115200);
  Serial.printf("PSRAM Size: %d\r\n", rp2040.getPSRAMSize());
}

// the loop function runs over and over again forever
void loop() {
  int i;
  static int cntr = 1;

  uint8_t *mem = mems;
  Serial.printf("%05d: Filling %d memory locations @%p with random values and verifying in %d byte chunks.\r\n", cntr++, sizeof(mems), mem, CHUNK_SIZE);

  for (size_t m = 0; m < (sizeof(mems) / CHUNK_SIZE); m++) {
    for (i = 0; i < CHUNK_SIZE; i++) {
      tmp[i] = (char)random(0, 255);
      mem[i] = tmp[i];
    }

    for (i = 0; i < CHUNK_SIZE; i++) {
      if (mem[i] != tmp[i]) {
        Serial.printf("Memory error @%p(%d), was 0x%02x, should be 0x%02x\r\n", mem, i, *mem, tmp[i]);
        delay(10);
      }
    }
    Serial.write('.');
    Serial.flush();
    mem += CHUNK_SIZE;
  }
  Serial.printf("\r\nDone, testing %d bytes\r\n", sizeof(mems));

  Serial.printf("\r\nBefore pmalloc, total PSRAM heap: %d, available PSRAM heap: %d\r\n", rp2040.getTotalPSRAMHeap(), rp2040.getFreePSRAMHeap());
  uint8_t *pmem = (uint8_t *)pmalloc(PMALLOCSIZE);
  if (!pmem) {
    Serial.printf("Error: Unable to allocate PSRAM chunk!\r\n");
    return;
  }
  Serial.printf("After pmalloc, total PSRAM heap: %d, available PSRAM heap: %d\r\n", rp2040.getTotalPSRAMHeap(), rp2040.getFreePSRAMHeap());

  Serial.printf("Allocated block @%p, size %d\r\n", pmem, PMALLOCSIZE);
  delay(3000);
  mem = pmem;
  for (size_t m = 0; m < (PMALLOCSIZE / CHUNK_SIZE); m++) {
    for (i = 0; i < CHUNK_SIZE; i++) {
      tmp[i] = (char)random(0, 255);
      mem[i] = tmp[i];
    }

    for (i = 0; i < CHUNK_SIZE; i++) {
      if (mem[i] != tmp[i]) {
        Serial.printf("Memory error @%p(%d), was 0x%02x, should be 0x%02x\r\n", mem, i, *mem, tmp[i]);
        delay(10);
      }
    }
    Serial.write('.');
    Serial.flush();
    mem += CHUNK_SIZE;
  }
  Serial.printf("\nDone, testing %d allocated bytes\r\n", sizeof(mems));
  free(pmem); // Release allocation for next pass
  Serial.printf("After free, total PSRAM heap: %d, available PSRAM heap: %d\r\n", rp2040.getTotalPSRAMHeap(), rp2040.getFreePSRAMHeap());

  delay(1000);
}

#endif // RAM_CHIP_SELECT
