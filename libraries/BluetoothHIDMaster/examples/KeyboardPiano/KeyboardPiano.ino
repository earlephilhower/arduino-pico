// KeyboardPiano example - Released to the public domain in 2024 by Earle F. Philhower, III
//
// Demonstrates using the BluetoothHIDMaster class to use a Bluetooth keyboard as a
// piano keyboard on the PicoW
//
// Hook up a phono plug to GP0 and GP1 (and GND of course...the 1st 3 pins on the PCB)
// Connect wired earbuds up and connect over BT from your keyboard and play some music.


#include <BluetoothHIDMaster.h>
#include <PWMAudio.h>

// We need the inverse map, borrow from the Keyboard library
#include <HID_Keyboard.h>
extern const uint8_t KeyboardLayout_en_US[128];

BluetoothHIDMaster hid;
PWMAudio pwm;
HIDKeyStream keystream;


int16_t sine[1000]; // One complete precalculated sine wave for oscillator use
void precalculateSine() {
  for (int i = 0; i < 1000; i++) {
    sine[i] = (int16_t)(2000.0 * sin(i * 2 * 3.14159 / 1000.0));  // Only make amplitude ~1/16 max so we can sum up w/o clipping
  }
}

// Simple variable frequency resampling oscillator state
typedef struct {
  uint32_t key;  // Identifier of which key started this tone
  uint32_t pos;  // Current sine offset
  uint32_t step; // Delta in fixed point 16p16 format
} Oscillator;
Oscillator osc[6]; // Look, ma! 6-note polyphony!

// Quiet down, now!
void silenceOscillators() {
  noInterrupts();
  for (int i = 0; i < 6; i++) {
    osc[i].pos = 0;
    osc[i].step = 0;
  }
  interrupts();
}

// PWM callback, generates sum of online oscillators
void fill() {
  int num_samples = pwm.availableForWrite() / 2;
  int16_t buff[32 * 2];

  while (num_samples > 63) {
    // Run in 32 LR sample chunks for speed, less loop overhead
    for (int o = 0; o < 32; o++) {
      int32_t sum = 0;
      for (int i = 0; i < 6; i++) {
        if (osc[i].step) {
          sum += sine[osc[i].pos >> 16];
          osc[i].pos += osc[i].step;
          while (osc[i].pos >= 1000 << 16) {
            osc[i].pos -= 1000 << 16;
          }
        }
      }
      if (sum > 32767) {
        sum = 32767;
      } else if (sum < -32767) {
        sum = -32767;
      }
      buff[o * 2] = (int16_t) sum;
      buff[o * 2 + 1] = (int16_t) sum;
    }
    pwm.write((const uint8_t *)buff, sizeof(buff));
    num_samples -= 64;
  }
}

// Mouse callbacks. Could keep track of global mouse position, update a cursor, etc.
void mm(void *cbdata, int dx, int dy, int dw) {
  (void) cbdata;
  Serial.printf("Mouse: X:%d  Y:%d  Wheel:%d\n", dx, dy, dw);
}

// Buttons are sent separately from movement
void mb(void *cbdata, int butt, bool down) {
  (void) cbdata;
  Serial.printf("Mouse: Button %d %s\n", butt, down ? "DOWN" : "UP");
}

// Convert a hertz floating point into a step fixed point 16p16
inline uint32_t stepForHz(float hz) {
  const float stepHz = 1000.0 / 44100.0;
  const float step = hz * stepHz;
  return (uint32_t)(step * 65536.0);
}

uint32_t keyStepMap[128]; // The frequency of any raw HID key
void setupKeyStepMap() {
  for (int i = 0; i < 128; i++) {
    keyStepMap[i] = 0;
  }
  // Implements the "standard" PC keyboard to piano setup
  // https://ux.stackexchange.com/questions/46669/mapping-piano-keys-to-computer-keyboard
  keyStepMap[KeyboardLayout_en_US['a']] = stepForHz(261.6256);
  keyStepMap[KeyboardLayout_en_US['w']] = stepForHz(277.1826);
  keyStepMap[KeyboardLayout_en_US['s']] = stepForHz(293.6648);
  keyStepMap[KeyboardLayout_en_US['e']] = stepForHz(311.1270);
  keyStepMap[KeyboardLayout_en_US['d']] = stepForHz(329.6276);
  keyStepMap[KeyboardLayout_en_US['f']] = stepForHz(349.2282);
  keyStepMap[KeyboardLayout_en_US['t']] = stepForHz(369.9944);
  keyStepMap[KeyboardLayout_en_US['g']] = stepForHz(391.9954);
  keyStepMap[KeyboardLayout_en_US['y']] = stepForHz(415.3047);
  keyStepMap[KeyboardLayout_en_US['h']] = stepForHz(440.0000);
  keyStepMap[KeyboardLayout_en_US['u']] = stepForHz(466.1638);
  keyStepMap[KeyboardLayout_en_US['j']] = stepForHz(493.8833);
  keyStepMap[KeyboardLayout_en_US['k']] = stepForHz(523.2511);
  keyStepMap[KeyboardLayout_en_US['o']] = stepForHz(554.3653);
  keyStepMap[KeyboardLayout_en_US['l']] = stepForHz(587.3295);
  keyStepMap[KeyboardLayout_en_US['p']] = stepForHz(622.2540);
  keyStepMap[KeyboardLayout_en_US[';']] = stepForHz(659.2551);
  keyStepMap[KeyboardLayout_en_US['\'']] = stepForHz(698.4565);
}

// We get make/break for every key which lets us hold notes while a key is depressed
void kb(void *cbdata, int key) {
  bool state = (bool)cbdata;
  if (state && key < 128) {
    // Starting a new note
    for (int i = 0; i < 6; i++) {
      if (osc[i].step == 0) {
        // This one is free
        osc[i].key = key;
        osc[i].pos = 0;
        osc[i].step = keyStepMap[key];
        break;
      }
    }
  } else {
    for (int i = 0; i < 6; i++) {
      if (osc[i].key == (uint32_t)key) {
        osc[i].step = 0;
        break;
      }
    }
  }
  // The HIDKeyStream object converts a key and state into ASCII.  HID key IDs do not map 1:1 to ASCII!
  // Write the key and make/break state, then read 1 ASCII char back out.
  keystream.write((uint8_t)key);
  keystream.write((uint8_t)state);
  Serial.printf("Keyboard: %02x %s = '%c'\n", key, state ? "DOWN" : "UP", state ? keystream.read() : '-');
}


// Consumer keys are the special media keys on most modern keyboards (mute/etc.)
void ckb(void *cbdata, int key) {
  bool state = (bool)cbdata;
  Serial.printf("Consumer: %02x %s\n", key, state ? "DOWN" : "UP");
}


// Joystick can get reports of 4 analog axes, 1 d-pad bitfield, and up to 32 buttons
// Axes and hats that aren't reported by the joystick are read as 0
void joy(void *cbdata, int x, int y, int z, int rz, uint8_t hat, uint32_t buttons) {
  (void) cbdata;
  const char *hats[16] = { "U", "UR", "R", "DR", "D", "DL", "L", "UL", "", "", "", "", "", "", "", "." };
  Serial.printf("Joystick: (%4d, %4d) (%4d, %4d), Hat: %-2s, Buttons:", x, y, z, rz, hats[hat & 15]);
  for (int i = 0; i < 32; i++) {
    Serial.printf(" %c", (buttons & 1 << i) ? '*' : '.');
  }
  Serial.println();
}

void setup() {
  Serial.begin();
  delay(3000);

  Serial.printf("Starting HID master, put your device in pairing mode now.\n");

  // Init the sound generator
  precalculateSine();
  silenceOscillators();
  setupKeyStepMap();

  // Setup the HID key to ASCII conversion
  keystream.begin();

  // Init the PWM audio output
  pwm.setStereo(true);
  pwm.setBuffers(16, 64);
  pwm.onTransmit(fill);
  pwm.begin(44100);

  // Mouse buttons and movement reported separately
  hid.onMouseMove(mm);
  hid.onMouseButton(mb);

  // We can use the cbData as a flag to see if we're making or breaking a key
  hid.onKeyDown(kb, (void *)true);
  hid.onKeyUp(kb, (void *)false);

  // Consumer keys are the special function ones like "mute" or "home"
  hid.onConsumerKeyDown(ckb, (void *)true);
  hid.onConsumerKeyUp(ckb, (void *)false);

  hid.onJoystick(joy);

  hid.begin();

  hid.connectAny();
  // or hid.connectMouse();
}

void loop() {
  if (BOOTSEL) {
    while (BOOTSEL) {
      delay(1);
    }
    hid.disconnect();
    hid.clearPairing();
    Serial.printf("Restarting HID master, put your device in pairing mode now.\n");
    hid.connectAny();
  }
}
