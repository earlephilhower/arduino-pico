// KeyboardPiano example - Released to the public domain in 2024 by Earle F. Philhower, III
//
// Demonstrates using the BluetoothHIDMaster class to use a Bluetooth keyboard as a
// piano keyboard on the PicoW
//
// Hook up a phono plug to GP0 and GP1 (and GND of course...the 1st 3 pins on the PCB)
// Connect wired earbuds up and connect over BT from your keyboard and play some music.


#include <BluetoothHIDMaster.h>
#include <PWMAudio.h>

// There are better ways to store data, but this lets us not require the user
// to select a FS size in the IDE for it to work.  Please use LittleFS in your
// own applications.
#include <EEPROM.h>



// We need the inverse map, borrow from the Keyboard library
#include <HID_Keyboard.h>
extern const uint8_t KeyboardLayout_en_US[128];

BluetoothHIDMaster hid;
PWMAudio pwm;
HIDKeyStream keystream;

const char *DATAHEADER = "Last BLE Address"; // exactly 16 chars + \0

typedef struct {
  char hdr[16];     // The header marker so we know this is our data
  uint8_t addr[6];  // MAC of the peer we last connected to
  uint8_t addrType; // Type of address (normal or randomized) of the last peer
} EEPROMDATA;


const char *macToString(const uint8_t *addr, uint8_t addrType) {
  static char mac[32];
  sprintf(mac, "%02x:%02x:%02x:%02x:%02x:%02x,%d", addr[0], addr[1], addr[2], addr[3], addr[4], addr[5], addrType ? 1 : 0);
  return mac;
}

uint8_t lastAddress[6] = {};
uint8_t lastAddressType = 0;

void loadEEPROM() {
  EEPROMDATA data;
  EEPROM.begin(sizeof(EEPROMDATA));
  EEPROM.get(0, data);
  EEPROM.end();
  if (memcmp(data.hdr, DATAHEADER, sizeof(data.hdr))) {
    // Not our data, just clear things
    Serial.println("No previous paired device found in EEPROM");
    bzero(lastAddress, 6);
    lastAddressType = 0;
  } else {
    // Our baby, copy it to local storage
    Serial.printf("Previoudly paired device found: %s\n", macToString(data.addr, data.addrType));
    memcpy(lastAddress, data.addr, sizeof(lastAddress));
    lastAddressType = data.addrType;
  }
}

void saveEEPROM() {
  EEPROMDATA data;
  memcpy(data.hdr, DATAHEADER, sizeof(data.hdr));
  memcpy(data.addr, lastAddress, sizeof(data.addr));
  data.addrType = lastAddressType;
  EEPROM.begin(sizeof(EEPROMDATA));
  EEPROM.put(0, data);
  EEPROM.commit();
  EEPROM.end();
  Serial.printf("Wrote paired device to EEPROM: %s\n", macToString(lastAddress, lastAddressType));
}


// Either try continually to reconnect to the last device connected,
// or if it's invalid or user holds BOOTSEL then we'll start a new
// pairing process (i.e. the peripheral will need to be put into
// pairing mode to bond)
void connectOrPair() {
  uint8_t x = 0;
  for (int i = 0; i < 6; i++) {
    x |= lastAddress[i];
  }
  if (x) {
    // There's a valid address, attempt to reconnect forever until connect or BOOTSEL
    Serial.printf("Attempting to reconnect to %s...\n", macToString(lastAddress, lastAddressType));
    do {
      delay(10);
      hid.connectBLE(lastAddress, lastAddressType);
    } while (!hid.connected() && !BOOTSEL);
    if (hid.connected()) {
      Serial.println("Reconnected!\n");
      return;
    }
    // Fall through to pair
  }
  pinMode(LED_BUILTIN, OUTPUT);
  bool l = true;
  Serial.println("Entering pairing mode.  Set the peripheral to pair state");
  hid.clearPairing();
  do {
    digitalWrite(LED_BUILTIN, l);
    l = !l;
    delay(10);
    hid.connectBLE();
  } while (!hid.connected());
  Serial.printf("Connected to device: %s\n", macToString(hid.lastConnectedAddress(), hid.lastConnectedAddressType()));
  memcpy(lastAddress, hid.lastConnectedAddress(), sizeof(lastAddress));
  lastAddressType = hid.lastConnectedAddressType();
  // We've update the connection, store away
  saveEEPROM();
}




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
  return (uint32_t)(step * 65536.0f);
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
  keystream.flush();
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

  Serial.println("Starting HID master");

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

  loadEEPROM(); // See about reconnecting

  hid.begin(true);
}

void loop() {
  if (!hid.connected()) {
    connectOrPair();
  }
}
