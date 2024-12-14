// This example uses semihosting to send serial output to the OpenOcD screen
// and write binary files to the host system.
//
// Semihosting **ONLY** works with an OpenOCD and GDB setup.  If you build
// and run a semihosting app without GDB connected, it **WILL CRASH**
//
// Start OpenOCD normally, but leave the terminal window visible because
// is it OpenOCD, not GDB, which will display the semihosting output.
// OpenOCD will also create files in the current working directory, so
// be sure it is a place you can find and write to.
//
// In GDB,connect to OpenOCD and then enable semihosting
// (gdb) target extended-remote localhost:3333
// (gdb) monitor arm semihosting enable

#ifdef __riscv
void setup() {
  // No semihosting for RISCV yet
}
void loop() {
}
#else

#include <SemiFS.h> // For SemiFS.open()

int c = 0;

void setup() {
  SerialSemi.begin();
  SerialSemi.printf("HELLO, GDB!\n");
  SemiFS.begin();
  File f = SemiFS.open("out.bin", "w");
  f.printf("I made a file!\n");
  f.close();
  SerialSemi.printf("Just wrote a file 'out.bin'\n");
}

void loop() {
  SerialSemi.printf("SH Loop Count: %d\n", c++);
  Serial.printf("USB Loop Count: %d\n", c++);
  delay(1000);
}
#endif
