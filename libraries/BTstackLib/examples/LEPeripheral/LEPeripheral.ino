// LE Peripheral Example - not working yet
#include <BTstackLib.h>
#include <SPI.h>

/*
   EXAMPLE_START(LEPeripheral): LE Peripheral

   @text BTstack allows to setup a GATT Services and Characteristics directly
   from the setup function without using other tools outside of the Arduino IDE.

   @section Setup

   @text First, a number of callbacks are set. Then, a Service with a Read-only
   Characteristic and a dynamic Characteristic is added to the GATT database.
   In BTstack, a dynamic Characteristic is a Characteristic where reads and writes
   are forwarded to the Sketch. In this example, the dynamic Characteristic is
   provided by the single byte variable characteristic_data.
*/

/* LISTING_START(LEPeripheralSetup): Setup */
static char characteristic_data = 'H';

void setup(void) {

  Serial.begin(9600);

  // set callbacks
  BTstack.setBLEDeviceConnectedCallback(deviceConnectedCallback);
  BTstack.setBLEDeviceDisconnectedCallback(deviceDisconnectedCallback);
  BTstack.setGATTCharacteristicRead(gattReadCallback);
  BTstack.setGATTCharacteristicWrite(gattWriteCallback);

  // setup GATT Database
  BTstack.addGATTService(new UUID("B8E06067-62AD-41BA-9231-206AE80AB551"));
  BTstack.addGATTCharacteristic(new UUID("f897177b-aee8-4767-8ecc-cc694fd5fcef"), ATT_PROPERTY_READ, "This is a String!");
  BTstack.addGATTCharacteristicDynamic(new UUID("f897177b-aee8-4767-8ecc-cc694fd5fce0"), ATT_PROPERTY_READ | ATT_PROPERTY_WRITE | ATT_PROPERTY_NOTIFY, 0);

  // startup Bluetooth and activate advertisements
  BTstack.setup();
  BTstack.startAdvertising();
}
/* LISTING_END(LEPeripheralSetup): Setup */

void loop(void) {
  BTstack.loop();
}

/*
   @section Device Connected Callback

   @text When a remove device connects, device connected callback is callec.
*/
/* LISTING_START(LEPeripheralDeviceConnectedCallback): Device Connected Callback */
void deviceConnectedCallback(BLEStatus status, BLEDevice *device) {
  (void) device;
  switch (status) {
    case BLE_STATUS_OK:
      Serial.println("Device connected!");
      break;
    default:
      break;
  }
}
/* LISTING_END(LEPeripheralDeviceConnectedCallback): Device Connected Callback */

/*
   @section Device Disconnected Callback

   @text If the connection to a device breaks, the device disconnected callback
   is called.
*/
/* LISTING_START(LEPeripheralDeviceDisconnectedCallback): Device Disconnected Callback */
void deviceDisconnectedCallback(BLEDevice * device) {
  (void) device;
  Serial.println("Disconnected.");
}
/* LISTING_END(LEPeripheralDeviceDisconnectedCallback): Device Disconnected Callback */

/*
   @section Read Callback

   @text In BTstack, the Read Callback is first called to query the size of the
   Characteristic Value, before it is called to provide the data.
   Both times, the size has to be returned. The data is only stored in the provided
   buffer, if the buffer argument is not NULL.
   If more than one dynamic Characteristics is used, the value handle is used
   to distinguish them.
*/
/* LISTING_START(LEPeripheralReadCallback): Read Callback */
uint16_t gattReadCallback(uint16_t value_handle, uint8_t * buffer, uint16_t buffer_size) {
  (void) value_handle;
  (void) buffer_size;
  if (buffer) {
    Serial.print("gattReadCallback, value: ");
    Serial.println(characteristic_data, HEX);
    buffer[0] = characteristic_data;
  }
  return 1;
}
/* LISTING_END(LEPeripheralDeviceDisconnectedCallback): Read Callback */

/*
   @section Write Callback

   @text When the remove device writes a Characteristic Value, the Write callback
   is called. The buffer arguments points to the data of size size/
   If more than one dynamic Characteristics is used, the value handle is used
   to distinguish them.
*/
/* LISTING_START(LEPeripheralWriteCallback): Write Callback */
int gattWriteCallback(uint16_t value_handle, uint8_t *buffer, uint16_t size) {
  (void) value_handle;
  (void) size;
  characteristic_data = buffer[0];
  Serial.print("gattWriteCallback , value ");
  Serial.println(characteristic_data, HEX);
  return 0;
}
/* LISTING_END(LEPeripheralWriteCallback): Write Callback */

