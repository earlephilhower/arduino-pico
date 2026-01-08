// This example shows writing your own class to run a BLE device.
// The class will get all the callbacks#include <BLE.h>

#include <BLE.h>

// Note that this class implements the BLEService and the Characteristic Callbacks so it's self-contained
// Also implements the server onConnect/disconnect callbacks
class BLECustomService : public BLEService, public BLECharacteristicCallbacks, public BLEServerCallbacks {
  public:
    // Generated via uuidgen or a web service
    static constexpr const char *SERVICE_UUID = "4ae60001-a1ad-46b0-8234-88a23ad055b9";
    static constexpr const char *CHARACTERISTIC_UUID_1 = "4ae60002-a1ad-46b0-8234-88a23ad055b9";
    static constexpr const char *CHARACTERISTIC_UUID_2 = "4ae60003-a1ad-46b0-8234-88a23ad055b9";

    // Be sure to call the BLEService constructor first
    BLECustomService() : BLEService(BLEUUID(SERVICE_UUID)) {
      // Create the characteristics
      c1 = new BLECharacteristic(BLEUUID(CHARACTERISTIC_UUID_1), BLEWrite, "Write to me and I'll send it back");
      c2 = new BLECharacteristic(BLEUUID(CHARACTERISTIC_UUID_2), BLERead | BLENotify, "Echoed back data");
      // Have the characteristic call this class when something happens
      c1->setCallbacks(this);
      c2->setCallbacks(this);
      // Give some default values
      c1->setValue(String("Write Me"));
      c2->setValue(String("Echo"));
      // Add them to the service
      addCharacteristic(c1);
      addCharacteristic(c2);
    }

    // Let the main app set the readable value
    void out(String str) {
      c2->setValue(str);
    }

    bool connected = false;

  private:
    // These implement the BLECharacteristicCallbacks
    void onWrite(BLECharacteristic *c) {
      // Do the right thing depending on what characteristic was just written
      if (c != c1) {
        return;  // Shouldn't ever happen
      }
      c2->setValue((const uint8_t *)c->valueData(), c->valueLen());
    }
    // We could also implement onRead()

    // BLEServerCallbacks
    void onConnect(BLEServer *s) {
      connected = true;
    }

    void onDisconnect(BLEServer *s) {
      connected = false;
    }

    BLECharacteristic *c1; // Writable
    BLECharacteristic *c2; // Readable
};

// Actual instance of the service
BLECustomService svc;

void setup() {
  BLE.begin("PicoBongo");
  BLE.server()->addService(&svc);
  BLE.startAdvertising();
}

void loop() {
  if (BOOTSEL) {
    svc.out("BONGO!");
    while (BOOTSEL);
  }
}