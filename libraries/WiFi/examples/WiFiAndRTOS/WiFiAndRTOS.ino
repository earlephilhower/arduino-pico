#include <Arduino.h>
#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>
#include <WiFi.h>
#include <WiFiMulti.h>

#define WIFI_SSID "your-ssid"
#define WIFI_PASS "your-password"

WiFiMulti multi;
int _beginLoopCount;
bool skip = false;

void setup() {
    while(!Serial);
    multi.addAP(WIFI_SSID, WIFI_PASS);
    delay(1000);    
    Serial.println("Connecting to " + String(WIFI_SSID));
    Serial.flush();
}

void loop() {
    if(skip) return;
    Serial.println("Running on core " + String(get_core_num()) + " task ID " + String(uxTaskGetTaskNumber(xTaskGetCurrentTaskHandle())));
    unsigned long startConnect = millis();
    if (multi.run() != WL_CONNECTED) {
      if (_beginLoopCount < 5) {
        Serial.println("\nFailed to connect to Wi-Fi. Trying again...");
        _beginLoopCount++;
        delay(1000);
      } else {
        Serial.println("\nFailed to connect to Wi-Fi. Quiting");
        _beginLoopCount = 0;
      }
    } else {
      _beginLoopCount = 0;
      Serial.print("\nWiFi connected, SSID: ");
      Serial.print(WiFi.SSID());
      Serial.print(", IP address: ");
      Serial.println(WiFi.localIP());
      Serial.printf("Connected in %d ms\n", millis() - startConnect);
      skip = true;
    }
}