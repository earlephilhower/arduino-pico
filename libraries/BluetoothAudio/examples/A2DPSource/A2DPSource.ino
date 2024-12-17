// Plays tones and Au Claire De La Lune over an A2DP connection
// Released to the public domain by Earle Philhower <earlephilhowwer@yahoo.com>

#include <BluetoothAudio.h>
#include "raw.h"

A2DPSource a2dp;

int16_t pcm[64 * 2];
uint32_t phase = 0;
volatile uint32_t fr = 32;
volatile uint32_t fl = 16;
volatile bool paused = false;
volatile bool playwav = false;

void avrcpCB(void *param, avrcp_operation_id_t op, int pressed) {
  (void) param;
  if (pressed && op == AVRCP_OPERATION_ID_FORWARD) {
    // Change the sound when they hit play
    fr = random(8, 64);
    fl = random(8, 64);
    Serial.printf("Now generating %lu, %lu\n", fr, fl);
  } else if (pressed && op == AVRCP_OPERATION_ID_PLAY) {
    paused = !paused;
    if (paused) {
      Serial.printf("Pausing\n");
    } else {
      Serial.printf("Resuming\n");
    }
  } else if (pressed && op == AVRCP_OPERATION_ID_BACKWARD) {
    playwav = !playwav;
    Serial.println(playwav ? "Playing 'Au Claire De La Lune'" : "Playing tones");
  }
}

void volumeCB(void *param, int pct) {
  (void) param;
  Serial.printf("Speaker volume changed to %d%%\n", pct);
}

void connectCB(void *param, bool connected) {
  (void) param;
  if (connected) {
    Serial.printf("A2DP connection started to %s\n", bd_addr_to_str(a2dp.getSinkAddress()));
  } else {
    Serial.printf("A2DP connection stopped\n");
  }
}

void fillPCM() {
  if (paused) {
    bzero(pcm, sizeof(pcm));
    return;
  }
  if (playwav) {
    for (int i = 0; i < 64; i++) {
      // Audio in flash is 8-bit signed mono, so shift left to make 16-bit and then play over both channels
      pcm[i * 2] = auclairedelalune_raw[(i + phase) % sizeof(auclairedelalune_raw)] << 8;
      pcm[i * 2 + 1] = auclairedelalune_raw[(i + phase) % sizeof(auclairedelalune_raw)] << 8;
    }
  } else {
    for (int i = 0; i < 64; i++) {
      pcm[i * 2] = ((i + phase) / fr) & 1 ? 3000 : -3000;
      pcm[i * 2 + 1] = ((i + phase) / fl) & 1 ? 1000 : -1000;
    }
  }
  phase += 64;
}

void setup() {
  delay(2000);
  a2dp.onAVRCP(avrcpCB);
  a2dp.onVolume(volumeCB);
  a2dp.onConnect(connectCB);
  a2dp.begin();
  Serial.printf("Starting, press BOOTSEL to pair to first found speaker\n");
  Serial.printf("Use the forward button on speaker to change tones\n");
  Serial.printf("Use the reverse button on speaker to alternate between tones and Au Claire De La Lune\n");
  Serial.printf("Use the play button on speaker to pause/unpause the tone\n");
}

void loop() {
  while ((size_t)a2dp.availableForWrite() > sizeof(pcm)) {
    fillPCM();
    a2dp.write((const uint8_t *)pcm, sizeof(pcm));
  }
  if (BOOTSEL) {
    while (BOOTSEL) {
      delay(1);
    }
    a2dp.disconnect();
    a2dp.clearPairing();
    Serial.printf("Connecting...");
    if (a2dp.connect()) {
      Serial.printf("Connected!\n");
    } else {
      Serial.printf("Failed!  :(\n");
    }
  }
}
