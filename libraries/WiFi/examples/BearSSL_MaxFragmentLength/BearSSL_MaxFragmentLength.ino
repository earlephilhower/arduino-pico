// Shows how to use the Maximum Fragment Length option in
// BearSSL to reduce SSL memory needs.
//
// Mar 2018, Jul 2022 by Earle F. Philhower, III
// Released to the public domain

#include <WiFi.h>
#include <PolledTimeout.h>

#ifndef STASSID
#define STASSID "your-ssid"
#define STAPSK  "your-password"
#endif

const char *ssid = STASSID;
const char *pass = STAPSK;

const char *site = "docs.oracle.com";

void fetch(BearSSL::WiFiClientSecure *client) {
  client->write("GET / HTTP/1.0\r\nHost: ");
  client->write(site);
  client->write("\r\nUser-Agent: Raspberry Pi Pico W\r\n\r\n");
  client->flush();
  using oneShot = esp8266::polledTimeout::oneShot;
  oneShot timeout(5000);
  do {
    char tmp[32];
    int rlen = client->read((uint8_t *)tmp, sizeof(tmp) - 1);
    yield();
    if (rlen < 0) {
      break;
    }
    if (rlen == 0) {
      delay(10);  // Give background processes some time
      continue;
    }
    tmp[rlen] = '\0';
    Serial.print(tmp);
  } while (!timeout);
  client->stop();
  Serial.printf("\n-------\n");
}

int fetchNoMaxFragmentLength() {
  int ret = rp2040.getFreeHeap();

  Serial.printf("\nConnecting to https://%s\n", site);
  Serial.printf("No MFLN attempted\n");

  BearSSL::WiFiClientSecure client;
  client.setInsecure();
  client.connect(site, 443);
  if (client.connected()) {
    Serial.printf("Memory used: %d\n", ret - rp2040.getFreeHeap());
    ret -= rp2040.getFreeHeap();
    fetch(&client);
  } else {
    Serial.printf("Unable to connect\n");
  }
  return ret;
}

int fetchMaxFragmentLength() {
  int ret = rp2040.getFreeHeap();

  // Servers which implement RFC6066's Maximum Fragment Length Negotiation
  // can be configured to limit the size of TLS fragments they transmit.
  // This lets small clients, like the Pico, use a smaller memory buffer
  // on the receive end (all the way down to under 1KB).  Unfortunately,
  // as of March 2018, there are not many public HTTPS servers which
  // implement this option.  You can deploy your own HTTPS or MQTT server
  // with MFLN enabled, of course.
  //
  // To determine if MFLN is supported by a server use the
  // ::probeMaxFragmentLength() method before connecting, and if it
  // returns true then you can use the ::setBufferSizes(rx, tx) to shrink
  // the needed BearSSL memory while staying within protocol limits.
  //
  // If MFLN is not supported, you may still be able to minimize the buffer
  // sizes assuming you can ensure the server never transmits fragments larger
  // than the size (i.e. by using HTTP GET RANGE methods, etc.).

  BearSSL::WiFiClientSecure client;
  client.setInsecure();
  bool mfln = client.probeMaxFragmentLength(site, 443, 512);
  Serial.printf("\nConnecting to https://%s\n", site);
  Serial.printf("MFLN supported: %s\n", mfln ? "yes" : "no");
  if (mfln) {
    client.setBufferSizes(512, 512);
  }
  client.connect(site, 443);
  if (client.connected()) {
    Serial.printf("MFLN status: %s\n", client.getMFLNStatus() ? "true" : "false");
    Serial.printf("Memory used: %d\n", ret - rp2040.getFreeHeap());
    ret -= rp2040.getFreeHeap();
    fetch(&client);
  } else {
    Serial.printf("Unable to connect\n");
  }
  return ret;
}

void setup() {
  Serial.begin(115200);

  delay(5000);
  Serial.println();
  Serial.println();

  // We start by connecting to a WiFi network
  Serial.print("Connecting to ");
  Serial.print(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");

  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

}

void loop() {
  Serial.printf("\n\n\n\n\nFREE MEM: %d     TOTAL HEAP:%d\n", rp2040.getFreeHeap(), rp2040.getTotalHeap());

  int a = fetchNoMaxFragmentLength();
  int b = fetchMaxFragmentLength();

  Serial.printf("\n\n");
  Serial.printf("Default SSL:       %d bytes used\n", a);
  Serial.printf("512 byte MFLN SSL: %d bytes used\n", b);

  delay(10000);
}
