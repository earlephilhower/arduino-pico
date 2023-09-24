#include <Arduino.h>
#include <FatFS.h>
#include <Adafruit_TinyUSB.h>

#define FORCE_FATFS_FORMAT false

Adafruit_USBD_MSC usb_msc;

// Callback invoked when received READ10 command.
// Copy disk's data to buffer (up to bufsize) and return number of copied bytes.
int32_t my_tud_msc_read10_cb(uint32_t lba, void* buffer, uint32_t bufsize) {
    Serial.println("Read callback: bufsize is " + String(bufsize));
    disk_read_direct((BYTE*)buffer, lba, bufsize);
    return bufsize;
}

// Callback invoked when received WRITE10 command.
// Process data in buffer to disk's storage and return number of written bytes
int32_t my_tud_msc_write10_cb(uint32_t lba, uint8_t* buffer, uint32_t bufsize) {
    Serial.println("Write callback: bufsize is " + String(bufsize));
    // WITH MISCONFIGURED TINYUSB, bufsize IS ONLY 512 BYTES
    // BUT FF_MAX_SS = 4096
    // HENCE THE DIVISION GIVES A SECTOR COUNT OF 0.
    // We need to be able to support smaller buffer sizes too
    // we can do this here when CFG_TUD_MSC_EP_BUFSIZE == 4096
    disk_write(0, buffer, lba, bufsize/FF_MAX_SS);
    return bufsize;
}

void msc_flush_cb (void) { /* NOP */}
bool is_writable() { return true; }

void setup() {
    Serial.begin(115200);
    FatFSConfig cfg;
    cfg.setAutoFormat(false); // we will do this ourselves
    cfg.setLabel("Pico FatFS");  //USB disk name as it appears on the computer

    FatFS.setConfig(cfg);
    if (FORCE_FATFS_FORMAT || !FatFS.begin()) {
        FatFS.format();
        FatFS.begin();
        // prepare initial file
        File f = FatFS.open("test.txt", "w");
        f.write("Hello from FatFS C++ class!");
        f.flush();
        f.close();
    }

    usb_msc.setID("Pico", "Internal Flash", "1.0");
    DWORD sect_cnt = 0;
    disk_ioctl(0, GET_SECTOR_COUNT, &sect_cnt);
    usb_msc.setCapacity(sect_cnt, FF_MAX_SS);
    usb_msc.setReadWriteCallback(my_tud_msc_read10_cb, my_tud_msc_write10_cb, msc_flush_cb);
    usb_msc.setWritableCallback(is_writable);
    usb_msc.setUnitReady(true);
    bool ok = usb_msc.begin();
    Serial.println("MSC begin ok: " + String(ok));
}

void loop() {
    Serial.println("Hello");
    delay(5000);
}