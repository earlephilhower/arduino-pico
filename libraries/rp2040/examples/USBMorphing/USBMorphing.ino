// Simple example showing multiple devices, one at a time, over USB.
// Released to the public domain 2025 by Earle F. Philhower, III

#include <USB.h>
#include <Keyboard.h>
#include <Mouse.h>

String readline() {
  String s;
  while (true) {
    int c = Serial.read();
    if ((c == '\r') || (c == '\n')) {
      return s;
    }
    if (c >= 1) {
      s += (char)c;
    }
  }
}

void setup() {
  Serial.begin();
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
  delay(5000);
  Serial.println("USB morphing device demo");
  Serial.println("Examine the `lsusb -v` or Windows Device Manager to see the changing USB devices");
  Serial.println("Wait for the LED on to press BOOTSEL in these demos.\r\n\r\n");
  Serial.println("First we'll become a keyboard.");
  Serial.println("What should we type when you hit BOOTSEL?");
  String toType = readline();
  Serial.println("OK, I'll disconnect from the USB and reappear as a keyboard.");
  Serial.println("Press BOOTSEL to type your message and disconnect the keyboard and reappear.");
  // Ensure the messages get sent then disconnect and turn into a keyboard
  Serial.flush();
  delay(5000);

  Serial.end();
  Keyboard.begin();
  delay(3000); // Give the computer time to recognize the change
  digitalWrite(LED_BUILTIN, HIGH);
  while (!BOOTSEL) {
    delay(10);
  }
  Keyboard.print(toType);
  delay(100); // Ensure PC has eaten all keys
  while (BOOTSEL) {
    delay(10);
  }
  digitalWrite(LED_BUILTIN, LOW);

  Keyboard.end();
  Serial.begin(115200);
  delay(5000); // Give PC time to reconnect
  Serial.println("Now I'll become a mouse and move down diagonally when you press BOOTSEL...");

  // Ensure the messages get sent then disconnect and turn into a keyboard
  Serial.flush();
  delay(5000);

  Serial.end();
  Mouse.begin();
  delay(3000); // Give the computer time to recognize the change
  digitalWrite(LED_BUILTIN, HIGH);
  while (!BOOTSEL) {
    delay(10);
  }
  for (int i = 0; i < 30; i++) {
    Mouse.move(3, 3, 0);
    delay(10);
  }
  delay(100);
  while (BOOTSEL) {
    delay(10);
  }
  digitalWrite(LED_BUILTIN, HIGH);

  Mouse.end();
  Serial.begin(115200);
  delay(5000); // Again. give PC time to set device back up
  Serial.println("Now I'm back to just a serial port.  The end.");
}

void loop() {
}
