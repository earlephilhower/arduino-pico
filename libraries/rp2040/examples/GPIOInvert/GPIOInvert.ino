
/*
  GPIOInvert

  This example shows how to enable the pin output inversion mechanism for a GPIO pin.

  This example code is in the public domain.
*/

int led = LED_BUILTIN; // the pin the LED is attached to

// the setup routine runs once when you press reset:
void setup() {
  // declare pin to be an output:
  pinMode(led, OUTPUT);
  //  Now enable the pin inversion function. 
  // This must be done after the pinMode command.
  setPinInvert(led, true);

  while(!Serial)
    delay(10);

  Serial.begin(115200);
}

// the loop routine runs over and over again forever:
void loop() {
    // Turn the LED off
    digitalWrite(led, HIGH);
    Serial.println("LED off");
    delay(2000);

    // Turn the LED on
    digitalWrite(led, LOW);
    Serial.println("LED on");
    delay(2000);
}