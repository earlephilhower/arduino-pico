/*
    This sketch establishes a TCP connection to a "quote of the day" service.
    It sends a "hello" message, and then prints received data.
*/

#include <Arduino.h>
#include <NCMEthernetlwIP.h>
#include <USB.h>

const char* host = "djxmmx.net";
const uint16_t port = 17;

NCMEthernetlwIP eth;
IPAddress my_static_ip_addr(192, 168, 137, 100);
IPAddress my_static_gateway(192, 168, 137, 1);
IPAddress my_static_dns(192, 168, 137, 1);

// #define USE_REAL_UART

#if defined(USE_REAL_UART)
#define SER Serial1
#else
#define SER Serial
#endif

void setup() {
  // enable Serial1 so it can be used by USE_REAL_UART or by DEBUG_RP2040_PORT
  Serial1.end();
  Serial1.setTX(0);
  Serial1.setRX(1);
  Serial1.begin(115200);

  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);

  Serial.begin(115200);
  SER.println();
  SER.println();
  SER.println("Starting NCM Ethernet port");


  //static config
  //eth.config(my_static_ip_addr, my_static_gateway, IPAddress(255, 255, 255, 0), my_static_dns);

  // Start the Ethernet port
  // This starts DHCP in case config() was not called before
  bool ok = eth.begin();
  if (!ok) {
    while (1) {
      SER.println("Failed to initialize NCM Ethernet.");
      delay(1000);
    }
  } else {
    SER.println("NCM Ethernet started successfully.");
  }
  delay(5000);
}

void loop() {
  static unsigned long next_msg = 0;
  static bool led_on = false;

  static bool connected = false;
  if (!eth.connected()) {
    connected = false;
    if (millis() > next_msg) {
      next_msg = millis() + 5000;
      SER.println("Ethernet not connected");
      digitalWrite(LED_BUILTIN, led_on);
      led_on ^= 1;
    }
    return;
  } else if (!connected || millis() > next_msg) {
    SER.println("");
    if (!connected) {
      SER.println("Ethernet connected");
    }
    SER.println("IP address: ");
    SER.println(eth.localIP());
#if LWIP_IPV6
    for (int i = 0; i < LWIP_IPV6_NUM_ADDRESSES; i++) {
      IPAddress address = IPAddress(&eth.getNetIf()->ip6_addr[i]);
      if (!address.isSet()) {
        continue;
      }
      SER.println(address);
    }
#endif
    SER.println("Gateway:");
    SER.println(eth.gatewayIP());
    SER.println("DNS server:");
    SER.println(eth.dnsIP());
    next_msg = millis() + 20000;
    connected = true;
  }

  static bool wait = false;

  SER.printf("connecting to %s:%i ...", host, port);

  // Use WiFiClient class to create TCP connections
  WiFiClient client;
  if (!client.connect(host, port)) {
    SER.println("connection failed");
    delay(500);
    return;
  }

  // This will send a string to the server
  SER.print("sending request...");
  if (client.connected()) {
    client.println("hello from NCM RP2040");
  }

  // wait for data to be available
  unsigned long timeout = millis();
  while (true) {
    if (client.available() != 0) {
      break;
    }

    if (millis() - timeout > 1000) {
      SER.println(">>> Client Timeout !");
      client.stop();
      delay(5);
      return;
    }
  }

  // Read all the lines of the reply from server and print them to Serial
  // SER.println("receiving from remote server");
  // not testing 'client.connected()' since we do not need to send data here
  SER.println();
  int i = 0;
  while (true) {
    if (client.available() == 0) {
      break;
    }
    char ch = static_cast<char>(client.read());
    if (i++ < 200) {
      SER.print(ch);
    }
  }
  SER.println();

  // Close the connection
  //SER.println();
  SER.println("closing connection");
  SER.println();
  client.stop();

  if (wait) {
    delay(2000);
  }
  wait = true;
}
