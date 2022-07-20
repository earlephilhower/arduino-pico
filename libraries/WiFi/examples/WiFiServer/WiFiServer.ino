// Placed in the public domain by Earle F. Philhower, III, 2022

#include <WiFi.h>

#ifndef STASSID
#define STASSID "your-ssid"
#define STAPSK "your-password"
#endif

const char* ssid = STASSID;
const char* password = STAPSK;

int port = 4242;

WiFiServer server(port);

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.setHostname("PicoW2");
  Serial.printf("Connecting to '%s' with '%s'\n", ssid, password);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(100);
  }
  Serial.printf("\nConnected to WiFi\n\nConnect to server at %s:%d\n", WiFi.localIP().toString().c_str(), port);

  server.begin();
}

void loop() {
  static int i;
  delay(1000);
  Serial.printf("--loop %d\n", ++i);
  delay(10);
  WiFiClient client = server.available();
  if (!client) {
    return;
  }
  client.println("Type anything and hit return");
  while (!client.available()) {
    delay(10);
  }
  String req = client.readStringUntil('\n');
  Serial.println(req);
  client.printf("Hello from Pico-W\r\n");
  client.flush();
}
