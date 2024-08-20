/*
  RAM Test

  This section of code tests the onboard ram of RP2350 based boards with external
  PSRAM.
  
  This example code is in the public domain.

*/

#define CHUNK_SIZE    131072
uint8_t tmp[CHUNK_SIZE];
uint8_t mems[1024*1024*8] PSRAM;

// the setup function runs once when you press reset or power the board
void setup() {
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(LED_BUILTIN, OUTPUT);
  while(!Serial)
    delay(10);
  Serial.begin(115200);
  Serial.printf("Memory size: %d\r\n", rp2040.getPSRAMSize());
}

// the loop function runs over and over again forever
void loop() {
  int i;
  int cntr = 1;

  uint8_t *mem = mems;
  Serial.printf("%05d: Filling %d memory locations @0x%08x with random values and verifying in %d byte chunks.\r\n", cntr++, _psram_size, mem, CHUNK_SIZE);
  
  for (int m=0; m < (_psram_size / CHUNK_SIZE); m++) {
    for (i=0; i<CHUNK_SIZE; i++) {
      tmp[i] = (char)random(0, 255);
      mem[i] = tmp[i];
    }
    
    for (i=0; i<CHUNK_SIZE; i++) {
      if (mem[i] != tmp[i]) {
        Serial.printf("Memory error @0x%08x(%d), was 0x%02x, should be 0x%02x\n", mem, i, *mem, tmp[i]);
        delay(10);
      }
    }
    Serial.write('.');
    Serial.flush();
    mem += CHUNK_SIZE;
  }
  Serial.printf("\nDone, testing %d bytes again\r\n", _psram_size);
}
