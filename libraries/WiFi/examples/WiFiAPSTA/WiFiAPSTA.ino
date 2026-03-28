/*
    WiFi AP+STA Concurrent Mode Example

    Demonstrates running the Pico W (or Pico 2 W) as both a WiFi client
    (Station) and an Access Point at the same time.

    The sketch:
    1. Connects to your existing WiFi network as a station (STA)
    2. Starts a soft Access Point so other devices can connect to the Pico
    3. Runs a simple web server on both interfaces

    Clients connecting to the AP (192.168.42.1) or the STA IP can both
    reach the web server.

    NOTE: In AP+STA mode the AP will operate on the same channel as the
    STA connection. This is a hardware limitation of the CYW43 chip.

    Copyright (c) 2024 Earle F. Philhower, III. All rights reserved.
    This example is released under the LGPLv2.1 license.
*/

#include <WiFi.h>

// --- Station credentials (your home/office router) ---
#ifndef STASSID
#define STASSID "your-ssid"
#define STAPSK  "your-password"
#endif

// --- Access Point settings ---
#define AP_SSID "PicoW-AP"
#define AP_PSK  "picow12345"  // min 8 characters, or nullptr for open

WiFiServer server(80);

void setup() {
  Serial.begin(115200);
  while (!Serial && millis() < 5000) {
    delay(10);
  }
  Serial.println("\n\nWiFi AP+STA Example");

  // Tell the WiFi stack we want both modes
  WiFi.mode(WIFI_AP_STA);

  // (Optional) Configure the AP network before calling beginAP
  // Default is 192.168.42.1 / 255.255.255.0
  WiFi.softAPConfig(
    IPAddress(192, 168, 42, 1),   // AP IP
    IPAddress(192, 168, 42, 1),   // Gateway
    IPAddress(255, 255, 255, 0)   // Subnet
  );

  // --- Start the Station side first ---
  Serial.printf("Connecting to %s ...\n", STASSID);
  WiFi.begin(STASSID, STAPSK);

  // Wait for STA connection (non-blocking alternative: check WiFi.status() in loop)
  uint32_t t0 = millis();
  while (!WiFi.connected() && millis() - t0 < 20000) {
    delay(250);
    Serial.print(".");
  }
  Serial.println();

  if (WiFi.connected()) {
    Serial.print("STA connected!  IP: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("STA connection failed – AP will still work.");
  }

  // --- Now start the Access Point ---
  Serial.printf("Starting AP \"%s\" ...\n", AP_SSID);
  if (WiFi.beginAP(AP_SSID, AP_PSK) == WL_CONNECTED) {
    Serial.print("AP started!     IP: ");
    Serial.println(WiFi.softAPIP());
  } else {
    Serial.println("AP failed to start!");
  }

  // Print mode confirmation
  Serial.printf("WiFi mode: %s\n",
                WiFi.getMode() == WIFI_AP_STA ? "AP+STA" :
                WiFi.getMode() == WIFI_AP     ? "AP" :
                WiFi.getMode() == WIFI_STA    ? "STA" : "OFF");

  Serial.print("STA MAC: ");
  Serial.println(WiFi.macAddress());
  Serial.print("AP  MAC: ");
  Serial.println(WiFi.softAPmacAddress());

  // Start the web server
  server.begin();
  Serial.println("HTTP server started on port 80.");
}

void loop() {
  WiFiClient client = server.accept();
  if (!client) {
    return;
  }

  // Wait for the client to send data
  while (client.connected() && !client.available()) {
    delay(1);
  }

  // Read the first line of the request
  String request = client.readStringUntil('\r');
  client.readStringUntil('\n');

  // Build a simple HTML response
  String html = "<!DOCTYPE html><html><head><title>Pico W AP+STA</title></head><body>";
  html += "<h1>Pico W &mdash; AP+STA Mode</h1>";
  html += "<table border='1' cellpadding='6'>";
  html += "<tr><th>Interface</th><th>IP</th><th>MAC</th></tr>";

  html += "<tr><td>Station (STA)</td><td>";
  html += WiFi.localIP().toString();
  html += "</td><td>";
  html += WiFi.macAddress();
  html += "</td></tr>";

  html += "<tr><td>Access Point (AP)</td><td>";
  html += WiFi.softAPIP().toString();
  html += "</td><td>";
  html += WiFi.softAPmacAddress();
  html += "</td></tr>";

  html += "</table>";
  html += "<p>Connected stations on AP: ";
  html += String(WiFi.softAPgetStationNum());
  html += "</p>";
  html += "<p>Uptime: ";
  html += String(millis() / 1000);
  html += " seconds</p>";
  html += "</body></html>";

  // Send HTTP response
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/html");
  client.print("Content-Length: ");
  client.println(html.length());
  client.println("Connection: close");
  client.println();
  client.print(html);

  delay(1);
  client.stop();
}
