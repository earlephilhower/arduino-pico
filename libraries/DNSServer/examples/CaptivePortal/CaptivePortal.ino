
#define USE_WIFI_NINA         false
#define USE_WIFI101           false
#define USE_WIFI_CUSTOM       false

#include <WiFi.h>
#include <DNSServer.h>
#include <WiFiWebServer.h>

const byte DNS_PORT = 53;
IPAddress apIP(192, 168, 42, 1);

DNSServer dnsServer;
WiFiWebServer webServer(80);

String responseHTML = ""
                      "<!DOCTYPE html><html lang='en'><head>"
                      "<meta name='viewport' content='width=device-width'>"
                      "<title>CaptivePortal</title></head><body>"
                      "<h1>Hello World!</h1><p>This is a captive portal example."
                      " All requests will be redirected here.</p></body></html>";

void setup()
{
  // Debug console
  Serial.begin(115200);
  while (!Serial && millis() < 5000);

  delay(1000);

  Serial.print("\nStart CaptivePortal on "); Serial.println(BOARD_NAME);
  
  WiFi.mode(WIFI_AP);

  WiFi.beginAP("DNSServer CaptivePortal example");

  // if DNSServer is started with "*" for domain name, it will reply with
  // provided IP to all DNS request
  if (dnsServer.start(DNS_PORT, "*", apIP))
    Serial.println("dnsServer starting OK");
  else
    Serial.println("dnsServer starting failure");  

  // replay to all requests with same HTML
  webServer.onNotFound([]()
  {
    webServer.send(200, "text/html", responseHTML);
  });

  webServer.begin();
}

void loop()
{
  dnsServer.processNextRequest();
  webServer.handleClient();
}
