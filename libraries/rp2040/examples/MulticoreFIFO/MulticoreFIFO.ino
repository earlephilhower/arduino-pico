// Demonstrates a simple use of the rp2040.fifo functions
// for a multiprocessor run to parse variables as pointers.

// Will output something like, where C0 is running on core 0 and
// C1 is on core 1, in parallel.

// C0 Char pointer: C0 commands C1 0x20041fc8
// C1 Char Pointer: C0 commands C1 0x20041fc8
// C1 Got the following: C0 commands C1

// The normal, core0 setup
void setup() {
  Serial.begin(9600);
  delay(5000);
}

char c0_char[] = "C0 commands C1"; // Declare as char* to allow modification
void loop() {
  delay(1000);
  Serial.printf("C0 Char pointer: %s %p\n", c0_char, reinterpret_cast<uint32_t>(c0_char));
  rp2040.fifo.push(reinterpret_cast<uint32_t>(c0_char));
}

char* c1_charP;

// Running on core1
void setup1() {
  delay(1000);
}

void loop1() {
  delay(500);
  char *c1_char; // Pointer to char
  c1_char = reinterpret_cast<char*>(rp2040.fifo.pop()); // Blocking  
  Serial.printf("C1 Char Pointer: %s %p\n", c1_char, c1_char);
}
