// Simple asynchronous I2C master and slave demo - Earle F. Philhower, III
// Released into the public domain.
//
// Using both onboard I2C interfaces, have one master and one slave
// and send data both ways between them.
//
// Uses the async Wire calls to allow applications to do other work while
// I2C transactions are ongoing.
//
// To run, connect GPIO0 to GPIO2, GPIO1 to GPIO3 on a single Pico

#include <Wire.h>

void setup() {
  Serial.begin(115200);
  delay(5000);
  Wire.setSDA(0);
  Wire.setSCL(1);
  Wire.begin();
  Wire1.setSDA(2);
  Wire1.setSCL(3);
  Wire1.begin(0x30);
  Wire1.onReceive(recv);
  Wire1.onRequest(req);
}

static char buff[100];

void loop() {
  static int p;
  char b[90];
  int loops;

  // Write a value over I2C to the slave
  Serial.println("\n\nSending...");
  sprintf(b, "This buffer is larger than the I2C FIFO by a whole lot, Pass #%d", p++);
  Wire.writeAsync(0x30, b, strlen(b), true);

  // A real application would go do some useful work here and check the
  // finishedAsync value when it needs the I2S operation to be completed
  // Here' we'll just increment a counter to show how much work could be done...
  loops = 0;
  while (!Wire.finishedAsync()) {
    loops++;
  }
  Serial.printf("Write idle loops: %d\n", loops);

  // Ensure the slave processing is done and print it out
  delay(1000);
  Serial.printf("buff: '%s'\n", buff);

  Serial.printf("Receiving...\n");

  // Read from the slave and print out
  bzero(b, sizeof(b));
  Wire.readAsync(0x30, b, 73, true);
  loops = 0;
  while (!Wire.finishedAsync()) {
    loops++;
  }
  Serial.printf("Read idle loops: %d\n", loops);
  Serial.print("recv: '");
  for (int i = 0; i < 73; i++) {
    Serial.print(b[i]);
  }
  Serial.println("'");
  delay(1000);
}

// These are called in an **INTERRUPT CONTEXT** which means NO serial port
// access (i.e. Serial.print is illegal) and no memory allocations, etc.

// Called when the I2C slave gets written to
void recv(int len) {
  int i;
  // Just stuff the sent bytes into a global the main routine can pick up and use
  for (i = 0; i < len; i++) {
    buff[i] = Wire1.read();
  }
  buff[i] = 0;
}

// Called when the I2C slave is read from
void req() {
  static int ctr = 765;
  char buff[100];
  // Return a simple incrementing hex value
  sprintf(buff, "Slave responds with a message that's longer than FIFO as well, id #%06X", (ctr++) % 65535);
  Wire1.write(buff, 73);
}
