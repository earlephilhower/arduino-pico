// Simple I2C master and slave demo - Earle F. Philhower, III
// Released into the public domain
//
// Using both onboard I2C interfaces, have one master and one slave
// and send data both ways between them
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

  // Write a value over I2C to the slave
  Serial.println("Sending...");
  Wire.beginTransmission(0x30);
  sprintf(b, "pass %d", p++);
  Wire.write(b, strlen(b));
  Wire.endTransmission();

  // Ensure the slave processing is done and print it out
  delay(1000);
  Serial.printf("buff: '%s'\r\n", buff);

  // Read from the slave and print out
  Wire.requestFrom(0x30, 6);
  Serial.print("\nrecv: '");
  while (Wire.available()) {
    Serial.print((char)Wire.read());
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
  char buff[7];
  // Return a simple incrementing hex value
  sprintf(buff, "%06X", (ctr++) % 65535);
  Wire1.write(buff, 6);
}
