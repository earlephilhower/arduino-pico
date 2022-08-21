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

void setup()
{
  // Debug console
  Serial.begin(115200);
  while (!Serial && millis() < 5000);

  delay(1000);

  Serial.print("\nStart DNSServer on "); Serial.println(BOARD_NAME);
  
  WiFi.mode(WIFI_AP);

  WiFi.beginAP("DNSServer example");

  // modify TTL associated  with the domain name (in seconds)
  // default is 60 seconds
  dnsServer.setTTL(300);

  // set which return code will be used for all other domains (e.g. sending
  // ServerFailure instead of NonExistentDomain will reduce number of queries
  // sent by clients)
  // default is DNSReplyCode::NonExistentDomain
  dnsServer.setErrorReplyCode(DNSReplyCode::ServerFailure);

  // start DNS server for a specific domain name
  if (dnsServer.start(DNS_PORT, "www.example.com", apIP))
    Serial.println("dnsServer starting OK");
  else
    Serial.println("dnsServer starting failure"); 

  // simple HTTP server to see that DNS server is working
  webServer.onNotFound([]()
  {
    String message = "Hello World!\n\n";
    message += "URI: ";
    message += webServer.uri();

    webServer.send(200, "text/plain", message);
  });

  webServer.begin();
}

void loop()
{
  dnsServer.processNextRequest();
  webServer.handleClient();
}
