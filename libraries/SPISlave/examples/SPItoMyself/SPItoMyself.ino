// Shows how to use SPISlave on a single device.
// Core0 runs as an SPI master and initiates a transmission to the slave
// Core1 runs the SPI Slave mode and provides a unique reply to messages from the master
//
// Released to the public domain 2023 by Earle F. Philhower, III <earlephilhower@yahoo.com>

#include <SPI.h>
#include <SPISlave.h>

// Wiring:
// Master RX  GP0 <-> GP11  Slave TX
// Master CS  GP1 <-> GP9   Slave CS
// Master CK  GP2 <-> GP10  Slave CK
// Master TX  GP3 <-> GP8   Slave RX

SPISettings spisettings(1000000, MSBFIRST, SPI_MODE0);

// Core 0 will be SPI master
void setup() {
  SPI.setRX(0);
  SPI.setCS(1);
  SPI.setSCK(2);
  SPI.setTX(3);
  SPI.begin(true);

  delay(5000);
}

int transmits = 0;
void loop() {
  char msg[42];
  memset(msg, 0, sizeof(msg));
  sprintf(msg, "What's up? This is transmission %d", transmits);
  Serial.printf("\n\nM-SEND: '%s'\n", msg);
  SPI.beginTransaction(spisettings);
  SPI.transfer(msg, sizeof(msg));
  SPI.endTransaction();
  Serial.printf("M-RECV: '%s'\n", msg);
  transmits++;
  delay(5000);
}

// Core 1 will be SPI slave

volatile bool recvBuffReady = false;
char recvBuff[42] = "";
int recvIdx = 0;
void recvCallback(uint8_t *data, size_t len) {
  memcpy(recvBuff + recvIdx, data, len);
  recvIdx += len;
  if (recvIdx == sizeof(recvBuff)) {
    recvBuffReady = true;
    recvIdx = 0;
  }
}

int sendcbs = 0;
// Note that the buffer needs to be long lived, the SPISlave doesn't copy it.  So no local stack variables, only globals or heap(malloc/new) allocations.
char sendBuff[42];
void sentCallback() {
  memset(sendBuff, 0, sizeof(sendBuff));
  sprintf(sendBuff, "Slave to Master Xmission %d", sendcbs++);
  SPISlave1.setData((uint8_t*)sendBuff, sizeof(sendBuff));
}

// Note that we use SPISlave1 here **not** because we're running on
// Core 1, but because SPI0 is being used already.  You can use
// SPISlave or SPISlave1 on any core.
void setup1() {
  SPISlave1.setRX(8);
  SPISlave1.setCS(9);
  SPISlave1.setSCK(10);
  SPISlave1.setTX(11);
  // Ensure we start with something to send...
  sentCallback();
  // Hook our callbacks into the slave
  SPISlave1.onDataRecv(recvCallback);
  SPISlave1.onDataSent(sentCallback);
  SPISlave1.begin(spisettings);
  delay(3000);
  Serial.println("S-INFO: SPISlave started");
}

void loop1() {
  if (recvBuffReady) {
    Serial.printf("S-RECV: '%s'\n", recvBuff);
    recvBuffReady = false;
  }
}
