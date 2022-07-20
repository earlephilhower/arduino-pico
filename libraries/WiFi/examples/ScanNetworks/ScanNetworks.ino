// Simple WiFi network scanner application
// Released to the public domain in 2022 by Earle F. Philhower, III
#include <WiFi.h>

void setup() {
  Serial.begin(115200);
}

const char *macToString(uint8_t mac[6]) {
  static char s[20];
  sprintf(s, "%02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  return s;
}

const char *encToString(uint8_t enc) {
  switch (enc) {
    case ENC_TYPE_NONE: return "NONE";
    case ENC_TYPE_TKIP: return "WPA";
    case ENC_TYPE_CCMP: return "WPA2";
    case ENC_TYPE_AUTO: return "AUTO";
  }
  return "UNKN";
}

void loop() {
  delay(5000);
  Serial.printf("Beginning scan at %d\n", millis());
  auto cnt = WiFi.scanNetworks();
  if (!cnt) {
    Serial.printf("No networks found\n");
  } else {
    Serial.printf("Found %d networks\n\n", cnt);
    Serial.printf("%32s %5s %17s %2s %4s\n", "SSID", "ENC", "BSSID        ", "CH", "RSSI");
    for (auto i = 0; i < cnt; i++) {
      uint8_t bssid[6];
      WiFi.BSSID(i, bssid);
      Serial.printf("%32s %5s %17s %2d %4d\n", WiFi.SSID(i), encToString(WiFi.encryptionType(i)), macToString(bssid), WiFi.channel(i), WiFi.RSSI(i));
    }
  }
  Serial.printf("\n--- Sleeping ---\n\n\n");
  delay(5000);
}
