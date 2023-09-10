#include <W5500lwIP.h>

Wiznet5500lwIP eth(/*SS*/ 1);

const char* host = "djxmmx.net";
const uint16_t port = 17;

void setup() {
  SPI.setSCK(2);
  SPI.setTX(3);
  SPI.setRX(0);
  SPI.setCS(1);

  delay(5000);
  Serial.begin(115200);

  Serial.println("\nEthernet\n");

  if (!eth.begin()) {
    Serial.printf("no hardware found\n");
    while (1) {
      delay(1000);
    }
  }

  while (!eth.connected()) {
    Serial.printf(".");
    delay(1000);
  }
  Serial.printf("Ethernet: IP Address: %s\n",
                eth.localIP().toString().c_str());
}

void loop() {

  Serial.print("connecting to ");
  Serial.print(host);
  Serial.print(':');
  Serial.println(port);

  Serial.printf("Link sense: %d (detectable: %d)\n", eth.isLinked(), eth.isLinkDetectable());

  // Use WiFiClient class to create TCP connections
  // (this class could have been named TCPClient)
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
  while (client.available()) {
    char ch = static_cast<char>(client.read());
    Serial.print(ch);
  }

  // Close the connection
  Serial.println();
  Serial.println("closing connection");
  client.stop();

  delay(300000);  // execute once every 5 minutes, don't flood remote service
}
