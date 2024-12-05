// This example should be run with profiling enabled from the IDE and
// under GDB/OpenOCD.  It uses semihosting to write a gmon.out file
// the host system with the profiled application results.
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
// (gdb) file /path/to/sketch.ino.elf
// (gdb) load
//
// Run the app from GDB and watch OpenOCD, it will display messages when
// the app is done and "gmon.out" is on the host system.
//
// (gdb) run
// .. pop to OpenOCD window
// [OpenOCD] BEGIN
// [OpenOCD] Result = 2417697592
// [OpenOCD] Writing GMON.OUT
// [OpenOCD] END
//
// From command line, decode the gmon.out using the ELF and gprof tool
//
// $ /path/to/arm-none-eabi/bin/arm-none-eabi-gprof /path/to/sketch.ino.elf /path/to/gmon.out | less
// Flat profile:
//
// Each sample counts as 0.0001 seconds.
//   %   cumulative   self              self     total
//  time   seconds   seconds    calls  ms/call  ms/call  name
//  50.56      1.74     1.74  3500020     0.00     0.00  __wrap___getreent
//  24.05      2.57     0.83                             rand
//   8.32      2.86     0.29        5    57.36    57.36  fcn1(unsigned long)
// ...
// index % time    self  children    called     name
//                                                 <spontaneous>
// [1]     74.6    0.83    1.74                 rand [1]
//                 1.74    0.00 3500000/3500020     __wrap___getreent [2]
// -----------------------------------------------
//                 0.00    0.00       1/3500020     realloc [106]
//                 0.00    0.00       3/3500020     vsnprintf [54]
//                 0.00    0.00       7/3500020     srand [7]
//                 0.00    0.00       9/3500020     malloc [105]
//                 1.74    0.00 3500000/3500020     rand [1]
// ...

#ifndef __PROFILE
void setup() {
  Serial.printf("Enable profiling to run this example.\n");
}

void loop() {
}
#else

#ifdef __riscv
void setup() {
  // No semihosting for RISCV yet
}
void loop() {
}
#else

#include <SemiFS.h>

uint32_t fcn1(uint32_t st) {
  srand(st);
  for (int i = 0; i < 500000; i++) {
    st += rand();
  }
  return st;
}

uint32_t fcn2(uint32_t st) {
  srand(st * st);
  for (int i = 0; i < 500000; i++) {
    st += rand();
  }
  return st;
}

void setup() {
  SerialSemi.printf("BEGIN\n");
  SerialSemi.printf("Result = %lu\n", fcn2(fcn2(fcn1(3)) * fcn1(fcn1(fcn1(fcn1(2))))));
  SerialSemi.printf("Writing GMON.OUT\n");
  SemiFS.begin();
  File gmon = SemiFS.open("gmon.out", "w");
  rp2040.writeProfiling(&gmon);
  gmon.close();
  SerialSemi.printf("END\n");
}

void loop() {
}

#endif

#endif // !__PROFILE
