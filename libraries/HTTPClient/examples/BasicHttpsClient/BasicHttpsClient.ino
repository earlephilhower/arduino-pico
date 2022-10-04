/**
   BasicHTTPSClient.ino

    Created on: 20.08.2018

*/

#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>

#ifndef STASSID
#define STASSID "your-ssid"
#define STAPSK "your-password"
#endif

const char *ssid = STASSID;
const char *pass = STAPSK;

WiFiMulti WiFiMulti;

void setup() {

  Serial.begin(115200);
  // Serial.setDebugOutput(true);

  Serial.println();
  Serial.println();
  Serial.println();

  for (uint8_t t = 4; t > 0; t--) {
    Serial.printf("[SETUP] WAIT %d...\n", t);
    Serial.flush();
    delay(1000);
  }

  WiFi.mode(WIFI_STA);
  WiFiMulti.addAP(ssid, pass);
}

const char *jigsaw_cert = R"EOF(
-----BEGIN CERTIFICATE-----
MIIFKTCCBM+gAwIBAgIQAbTKhAICxb7iDJbE6qU/NzAKBggqhkjOPQQDAjBKMQsw
CQYDVQQGEwJVUzEZMBcGA1UEChMQQ2xvdWRmbGFyZSwgSW5jLjEgMB4GA1UEAxMX
Q2xvdWRmbGFyZSBJbmMgRUNDIENBLTMwHhcNMjIwMzE3MDAwMDAwWhcNMjMwMzE2
MjM1OTU5WjB1MQswCQYDVQQGEwJVUzETMBEGA1UECBMKQ2FsaWZvcm5pYTEWMBQG
A1UEBxMNU2FuIEZyYW5jaXNjbzEZMBcGA1UEChMQQ2xvdWRmbGFyZSwgSW5jLjEe
MBwGA1UEAxMVc25pLmNsb3VkZmxhcmVzc2wuY29tMFkwEwYHKoZIzj0CAQYIKoZI
zj0DAQcDQgAEYnkGDyrIltjRnxoVdy/xgndo+WGMOASzs2hHeCjbJ1KplKJc/ciK
XCWq/4+pTzSiVgTFhRmCdLcU1Fa05YFNQaOCA2owggNmMB8GA1UdIwQYMBaAFKXO
N+rrsHUOlGeItEX62SQQh5YfMB0GA1UdDgQWBBRIzOWGCDBB/PMrMucSrjIKqlgE
uDAvBgNVHREEKDAmghVzbmkuY2xvdWRmbGFyZXNzbC5jb22CDWppZ3Nhdy53My5v
cmcwDgYDVR0PAQH/BAQDAgeAMB0GA1UdJQQWMBQGCCsGAQUFBwMBBggrBgEFBQcD
AjB7BgNVHR8EdDByMDegNaAzhjFodHRwOi8vY3JsMy5kaWdpY2VydC5jb20vQ2xv
dWRmbGFyZUluY0VDQ0NBLTMuY3JsMDegNaAzhjFodHRwOi8vY3JsNC5kaWdpY2Vy
dC5jb20vQ2xvdWRmbGFyZUluY0VDQ0NBLTMuY3JsMD4GA1UdIAQ3MDUwMwYGZ4EM
AQICMCkwJwYIKwYBBQUHAgEWG2h0dHA6Ly93d3cuZGlnaWNlcnQuY29tL0NQUzB2
BggrBgEFBQcBAQRqMGgwJAYIKwYBBQUHMAGGGGh0dHA6Ly9vY3NwLmRpZ2ljZXJ0
LmNvbTBABggrBgEFBQcwAoY0aHR0cDovL2NhY2VydHMuZGlnaWNlcnQuY29tL0Ns
b3VkZmxhcmVJbmNFQ0NDQS0zLmNydDAMBgNVHRMBAf8EAjAAMIIBfwYKKwYBBAHW
eQIEAgSCAW8EggFrAWkAdQDoPtDaPvUGNTLnVyi8iWvJA9PL0RFr7Otp4Xd9bQa9
bgAAAX+aFPh6AAAEAwBGMEQCICivjuh2ywUYvVpTKHo65JEheR8dFq8QvBgEiXfw
m6q6AiAkxAgz77oboGQGetNmab45+peY+nAGOfyW9vi9S1gMaAB3ADXPGRu/sWxX
vw+tTG1Cy7u2JyAmUeo/4SrvqAPDO9ZMAAABf5oU+GEAAAQDAEgwRgIhANKeTNMy
GqUsCo7ph7YMWzrhMuDeyP8xPSiCtFzKcn/eAiEAyv5lgCUQ6K14V13zYfL99wZD
LFcIP/KZ1y7nuPAksTAAdwCzc3cH4YRQ+GOG1gWp3BEJSnktsWcMC4fc8AMOeTal
mgAAAX+aFPiWAAAEAwBIMEYCIQD6535jWw776D4vjyupP2fBw26CBMpVT5++k4rR
xqeOXwIhAIbEaEKkEq6JtpWWfVpTyDkMpMfTuiqYVe6REy2XsmEhMAoGCCqGSM49
BAMCA0gAMEUCIH3r/puXZcX1bfUoBq2njuHe0bxWtvzDaz5k6WLYrazTAiEA+ePL
N6K5xrmaof185pVCxACPLc/BoKyUwMeC8iXCm00=
-----END CERTIFICATE-----
)EOF";

static int cnt = 0;

void loop() {
  // wait for WiFi connection
  if ((WiFiMulti.run() == WL_CONNECTED)) {
    HTTPClient https;
    switch (cnt) {
      case 0:
        Serial.println("[HTTPS] using insecure SSL, not validating certificate");
        https.setInsecure(); // Note this is unsafe against MITM attacks
        cnt++;
        break;
      case 1:
        Serial.println("[HTTPS] using secure SSL, validating certificate");
        https.setCACert(jigsaw_cert);
        cnt++;
        break;
      default:
        Serial.println("[HTTPS] not setting any SSL verification settings, will fail");
        cnt = 0;
    }

    Serial.print("[HTTPS] begin...\n");
    if (https.begin("https://jigsaw.w3.org/HTTP/connection.html")) {  // HTTPS

      Serial.print("[HTTPS] GET...\n");
      // start connection and send HTTP header
      int httpCode = https.GET();

      // httpCode will be negative on error
      if (httpCode > 0) {
        // HTTP header has been send and Server response header has been handled
        Serial.printf("[HTTPS] GET... code: %d\n", httpCode);

        // file found at server
        if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
          String payload = https.getString();
          Serial.println(payload);
        }
      } else {
        Serial.printf("[HTTPS] GET... failed, error: %s\n", https.errorToString(httpCode).c_str());
      }

      https.end();
    } else {
      Serial.printf("[HTTPS] Unable to connect\n");
    }
  }

  Serial.println("Wait 10s before next round...");
  delay(10000);
}
