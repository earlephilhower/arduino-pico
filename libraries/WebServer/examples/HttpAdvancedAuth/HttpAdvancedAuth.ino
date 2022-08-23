/*
  HTTP Advanced Authentication example
  Created Mar 16, 2017 by Ahmed El-Sharnoby.
  This example code is in the public domain.
*/

#include <WiFi.h>
#include <LEAmDNS.h>
#include <ArduinoOTA.h>
#include <WebServer.h>

#ifndef STASSID
#define STASSID "your-ssid"
#define STAPSK "your-password"
#endif

const char* ssid = STASSID;
const char* password = STAPSK;

WebServer server(80);

const char* www_username = "admin";
const char* www_password = "picow";
// allows you to set the realm of authentication Default:"Login Required"
const char* www_realm = "Custom Auth Realm";
// the Content of the HTML response in case of Unautherized Access Default:empty
String authFailResponse = "Authentication Failed";

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  if (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("WiFi Connect Failed! Rebooting...");
    delay(1000);
    rp2040.restart();
  }
  ArduinoOTA.begin();

  server.on("/", []() {
    if (!server.authenticate(www_username, www_password))
      // Basic Auth Method with Custom realm and Failure Response
      // return server.requestAuthentication(BASIC_AUTH, www_realm, authFailResponse);
      // Digest Auth Method with realm="Login Required" and empty Failure Response
      // return server.requestAuthentication(DIGEST_AUTH);
      // Digest Auth Method with Custom realm and empty Failure Response
      // return server.requestAuthentication(DIGEST_AUTH, www_realm);
      // Digest Auth Method with Custom realm and Failure Response
    {
      return server.requestAuthentication(DIGEST_AUTH, www_realm, authFailResponse);
    }
    server.send(200, "text/plain", "Login OK");
  });
  server.begin();

  Serial.print("Open http://");
  Serial.print(WiFi.localIP());
  Serial.println("/ in your browser to see it working");
}

void loop() {
  ArduinoOTA.handle();
  server.handleClient();
}
