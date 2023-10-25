// Demonstrates a simple use of the rp2040.fifo functions
// for a multiprocessor run to parse variables as pointers.

// Will output something like, where C0 is running on core 0 and
// C1 is on core 1, in parallel.

// [C1] Char* [C0 commands C1] Pointer [0x10014a62]
// [C1] Char* [C0 commands C1] Pointer [0x10014a62]
// [C0] Serial input: Hello
// [C1] Char* [Hello] Pointer [0x20003a4d]
// [C0] Serial input: Raspberry Pi Pico
// [C1] Char* [Raspberry Pi Pico] Pointer [0x20003a4d]
// [C0] Serial input: RP2040
// [C1] Char* [RP2040] Pointer [0x20003a4d]

// The normal, core0 setup
const char* c0_char = "C0 commands C1"; // Non modificable

void setup() {
  Serial.begin(9600);
  delay(5000);
  Serial.printf("[C1] Char* [%s] Pointer [%p]\n", c0_char, reinterpret_cast<uint32_t>(c0_char));
  rp2040.fifo.push(reinterpret_cast<uint32_t>(c0_char));
}

const byte bufferSize = 64;  // Maximum buffer size
char inputBuffer[bufferSize];  // Buffer for storing data

void loop() {
  //delay(1000);
  static uint8_t bufferIndex = 0; // Current index in the buffer

  while (Serial.available() > 0) {
    char incomingByte = Serial.read();

    if (incomingByte == '\n') {
      // A newline character is received, process the data
      inputBuffer[bufferIndex] = '\0';  // Add the null character at the end
      Serial.printf("[C0] Serial input: %s \n",inputBuffer);
      //Serial.println(inputBuffer);
      rp2040.fifo.push(reinterpret_cast<uint32_t>(inputBuffer));
      bufferIndex = 0;  // Reset the index for the next message
    } else {
      // Add the character to the buffer
      if (bufferIndex < bufferSize - 1) {
        inputBuffer[bufferIndex] = incomingByte;
        bufferIndex++;
      } else {
        // The buffer is full, you can take appropriate action if necessary
        // For example, ignore the data or print an error message
      }
    }
  }

}

// Running on core1
void setup1() {
  delay(1000);
}

void loop1() {
  delay(500);
  char *c1_char; // Pointer to char
  c1_char = reinterpret_cast<char*>(rp2040.fifo.pop()); // Blocking  
  Serial.printf("[C1] Char* [%s] Pointer [%p]\n", c1_char, c1_char);
}
