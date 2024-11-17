#include <LowPower.h>

#define GPIO_EXIT_DORMANT_MODE 3 /* connect GP3 to GND once in DORMANT mode */

void setup() {
  Serial1.setTX(12);
  Serial1.setRX(13);
  Serial1.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
  pinMode(GPIO_EXIT_DORMANT_MODE, INPUT_PULLUP); // pull pin that will get us out of sleep mode
}

void loop() {
  if (Serial1.available() > 0) {
    (void) Serial1.read();
    digitalWrite(LED_BUILTIN, LOW);
    Serial1.end(); // disable the UART
    pinMode(12, INPUT_PULLUP); // Serial TX
    gpio_set_input_enabled(12, false);
    pinMode(13, INPUT_PULLUP); // Serial RX
    gpio_set_input_enabled(13, false);
    pinMode(24, INPUT); // can measure VBUS on the pico
    gpio_set_input_enabled(24, false);
    pinMode(23, INPUT_PULLDOWN); // connected to PS of switching regulator; default pulldown
    gpio_set_input_enabled(23, false);
    pinMode(29, INPUT); // connected to Q1 / GPIO29_ADC3, which is a 3:1 voltage divider for VSYS
    gpio_set_input_enabled(29, false);
    // free standing / floating GPIOs
    for(auto p : {0, 1, 2, /*3,*/ 4, 5, 6, 7, 8, 9, 10, 11, /*12, 13,*/ 14, 15, 16, 17, 18, 19, 20, 21, 22, /*23, 24,*/ 25, 26, 27, 28, 29}) {
      pinMode(p, INPUT); // best performance!
      //pinMode(p, INPUT_PULLDOWN);
      //pinMode(p, INPUT_PULLUP);
      //pinMode(p, OUTPUT); digitalWrite(p, HIGH); // drive HIGH
      //pinMode(p, OUTPUT); digitalWrite(p, LOW); // drive LOW
      gpio_set_input_enabled(p, false); // disable input gate
      gpio_set_input_hysteresis_enabled(p, false); // additionally disable schmitt input gate
      // ^-- makes no difference
    } 
    gpio_set_input_enabled(30, false); // SWCLK
    gpio_set_input_enabled(31, false); // SWD(IO)

    // we will get out of sleep when an interrupt occurs.
    // this will shutdown the crystal oscillator until an interrupt occurs.
    LowPower.dormantUntilGPIO(GPIO_EXIT_DORMANT_MODE, FALLING);
    // this will only be reached after wakeup.
    // we don't actually know the time duration during which we were dormant.
    // so, the absolute value of millis() will be messed up.
    digitalWrite(LED_BUILTIN, HIGH);
    Serial1.begin(115200); // start UART again
  }
  Serial1.println("hello, world");
  digitalWrite(LED_BUILTIN, digitalRead(LED_BUILTIN) ^ 1);
  delay(500);
}