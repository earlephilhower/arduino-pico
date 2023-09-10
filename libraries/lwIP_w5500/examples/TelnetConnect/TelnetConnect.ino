#include <LwipEthernet.h>
#include <W5500lwIP.h>
#include <WiFi.h>
#include <list>
#include <pico/async_context_threadsafe_background.h>
//#include <pico/lwip_nosys.h>
#if 0
extern "C" {
  auto_init_recursive_mutex(__ethernetMutex); // Only for non-PicoW case

  void ethernet_arch_lwip_begin() {
    recursive_mutex_enter_blocking(&__ethernetMutex);
  }
  void ethernet_arch_lwip_end() {
    recursive_mutex_exit(&__ethernetMutex);
  }
  bool ethernet_arch_lwip_try() {
    uint32_t unused;
    return recursive_mutex_try_enter(&__ethernetMutex, &unused);
  }
};
#endif
//#include <pico/async_context.h>



Wiznet5500lwIP eth(/*SS*/ 1);  // <== adapt to your hardware

const char* host = "djxmmx.net";
const uint16_t port = 17;

#if 0
static async_context_threadsafe_background_t lwip_ethernet_async_context_threadsafe_background;

async_context_t *lwip_ethernet_init_default_async_context(void) {
  async_context_threadsafe_background_config_t config = async_context_threadsafe_background_default_config();
  if (async_context_threadsafe_background_init(&lwip_ethernet_async_context_threadsafe_background, &config)) {
    return &lwip_ethernet_async_context_threadsafe_background.core;
  }
  return NULL;
}


static void update_next_timeout(async_context_t *context, async_when_pending_worker_t *worker);
static void ethernet_timeout_reached(async_context_t *context, async_at_time_worker_t *worker);

static async_when_pending_worker_t always_pending_update_timeout_worker = {
  .next = 0,
  .do_work = update_next_timeout,
  .work_pending = 0,
  .user_data = 0,
};

static async_at_time_worker_t ethernet_timeout_worker = {
  .next = 0,
  .do_work = ethernet_timeout_reached,
  .next_time = 0,
  .user_data = 0,
};
bool go = false;

static void ethernet_timeout_reached(__unused async_context_t *context, __unused async_at_time_worker_t *worker) {
  assert(worker == &ethernet_timeout_worker);
  if (go) {
    eth.handlePackets();
    if (ethernet_arch_lwip_try()) {
      sys_check_timeouts();
      ethernet_arch_lwip_end();
    }
  }
}


static void update_next_timeout(async_context_t *context, async_when_pending_worker_t *worker) {
  assert(worker == &always_pending_update_timeout_worker);
  // we want to run on every execution of the helper to re-reflect any changes
  // to the underlying lwIP timers which may have happened in the interim
  // (note that worker will be called on every outermost exit of the async_context
  // lock, and lwIP timers should not be modified whilst not holding the lock.
  worker->work_pending = true;
  uint32_t sleep_ms = sys_timeouts_sleeptime();
  if (sleep_ms == SYS_TIMEOUTS_SLEEPTIME_INFINITE) {
    ethernet_timeout_worker.next_time = at_the_end_of_time;
  } else {
    ethernet_timeout_worker.next_time = make_timeout_time_ms(sleep_ms);
  }
  //            async_context_add_at_time_worker_in_ms(context, &sleep_timeout_worker, CYW43_SLEEP_CHECK_MS);
  async_context_add_at_time_worker_in_ms(context, &ethernet_timeout_worker, 50);
}

#endif
extern void ethinit();
void setup() {
#if 0
  async_context_t *context  = lwip_ethernet_init_default_async_context();

  //lwip_nosys_init(context);
  always_pending_update_timeout_worker.work_pending = true;
  async_context_add_when_pending_worker(context, &always_pending_update_timeout_worker);
#endif

  SPI.setSCK(2);
  SPI.setTX(3);
  SPI.setRX(0);
  SPI.setCS(1);
  SPI.begin();

  delay(5000);
  Serial.begin(115200);

  Serial.println("\nEthernet\n");

  // 1. Currently when no default is set, esp8266-Arduino uses the first
  //    DHCP client interface receiving a valid address and gateway to
  //    become the new lwIP default interface.
  // 2. Otherwise - when using static addresses - lwIP for every packets by
  //    defaults selects automatically the best suited output interface
  //    matching the destination address.  If several interfaces match,
  //    the first one is picked.  On esp8266/Arduno: WiFi interfaces are
  //    checked first.
  // 3. Or, use `::setDefault()` to force routing through this interface.
  // eth.setDefault(); // default route set through this interface
  ethinit();
  //  std::function<void(void)> b = std::bind(&Wiznet5500lwIP::handlePackets, &eth);
  //ddEthernetInterface(std::bind(&Wiznet5500lwIP::begin, &eth), std::bind(&Wiznet5500lwIP::handlePackets, &eth));
  if (!ethInitDHCP(eth)) {
    Serial.printf("no hardware found\n");
    while (1) {
      delay(1000);
    }
  }

  while (!eth.connected()) {//eth.handlePackets(); sys_check_timeouts();
    Serial.printf(".");
    delay(1000);
  }
  //go=true;
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
    client.println("hello from ESP8266");
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
