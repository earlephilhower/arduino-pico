/*
    This sketch establishes a TCP connection to a "quote of the day" service.
    It sends a "hello" message, and then prints received data.
*/

#include <W5500lwIP.h>

const char* host = "djxmmx.net";
const uint16_t port = 17;

#include <SoftwareSPI.h>
const int _SCK = 0;   // Any pin allowed
const int _CS = 1;    // Must be SCK+1 for HW CS support
const int _MISO = 28; // Note that MOSI and MISO don't need to be contiguous.  Any pins allowed
const int _MOSI = 3;  // Any pin not used elsewhere
const int _INT = 4;   // W5500 IRQ line

SoftwareSPI softSPI(_SCK, _MISO, _MOSI, _CS);

Wiznet5500lwIP eth(_CS, softSPI, _INT);

void setup() {
  Serial.begin(115200);
  delay(5000);
  if (_CS != _SCK + 1) {
    Serial.printf("Error, CS (%d) must be defined as SCK (%d) + 1 \n", _CS, _SCK);
    return;
  }
  Serial.println();
  Serial.println();
  Serial.println("Starting Ethernet port");

  // Start the Ethernet port
  if (!eth.begin()) {
    Serial.println("No wired Ethernet hardware detected. Check pinouts, wiring.");
    while (1) {
      delay(1000);
    }
  }

  while (!eth.connected()) {
    Serial.print(".");
    delay(500);
  }

  Serial.println("");
  Serial.println("Ethernet connected");
  Serial.println("IP address: ");
  Serial.println(eth.localIP());
}

void loop() {
  static bool wait = false;

  Serial.print("connecting to ");
  Serial.print(host);
  Serial.print(':');
  Serial.println(port);

  // Use WiFiClient class to create TCP connections
  WiFiClient client;
  if (!client.connect(host, port)) {
    Serial.println("connection failed");
    delay(5000);
    return;
  }

  // This will send a string to the server
  Serial.println("sending data to server");
  if (client.connected()) {
    client.println("hello from RP2040");
  }

  // wait for data to be available
  unsigned long timeout = millis();
  while (client.available() == 0) {
    if (millis() - timeout > 5000) {
      Serial.println(">>> Client Timeout !");
      client.stop();
      delay(60000);
      return;
    }
  }

  // Read all the lines of the reply from server and print them to Serial
  Serial.println("receiving from remote server");
  // not testing 'client.connected()' since we do not need to send data here
  while (client.available()) {
    char ch = static_cast<char>(client.read());
    Serial.print(ch);
  }

  // Close the connection
  Serial.println();
  Serial.println("closing connection");
  client.stop();

  if (wait) {
    delay(300000);  // execute once every 5 minutes, don't flood remote service
  }
  wait = true;
}
