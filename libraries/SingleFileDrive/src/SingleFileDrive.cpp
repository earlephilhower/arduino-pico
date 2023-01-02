/*
    SingleFileDrive - Emulates a USB stick for easy data transfer
    Copyright (c) 2022 Earle F. Philhower, III.  All rights reserved.

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include <SingleFileDrive.h>
#include <LittleFS.h>
#include <class/msc/msc.h>

SingleFileDrive singleFileDrive;

static const uint32_t _hddsize = (256 * 1024 * 1024); // 256MB
static const uint32_t _hddsects = _hddsize / 512;

// Ensure we are logged in to the USB framework
void __USBInstallMassStorage() {
    /* dummy */
}

SingleFileDrive::SingleFileDrive() {
}

SingleFileDrive::~SingleFileDrive() {
    end();
}

void SingleFileDrive::onDelete(void (*cb)(uint32_t), uint32_t cbData) {
    _cbDelete = cb;
    _cbDeleteData = cbData;
}

void SingleFileDrive::onPlug(void (*cb)(uint32_t), uint32_t cbData) {
    _cbPlug = cb;
    _cbPlugData = cbData;
}

void SingleFileDrive::onUnplug(void (*cb)(uint32_t), uint32_t cbData) {
    _cbUnplug = cb;
    _cbUnplugData = cbData;
}

bool SingleFileDrive::begin(const char *localFile, const char *dosFile) {
    if (_started) {
        return false;
    }
    _localFile = strdup(localFile);
    _dosFile = strdup(dosFile);
    _started = true;
    return true;
}

void SingleFileDrive::end() {
    _started = false;
    free(_localFile);
    free(_dosFile);
    _localFile = nullptr;
    _dosFile = nullptr;
}

void SingleFileDrive::bootSector(char buff[512]) {
    // 256MB FAT16 stolen from mkfs.fat
    // dd if=/dev/zero of=/tmp/fat.bin bs=1M seek=255 count=1
    // mkfs.fat -F 16 -r 16 -n PICODISK -i 12345678 -s 128 -m ':(' /tmp/fat.bin
    const uint8_t hdr[] = {
        0xeb, 0x3c, 0x90, 0x6d, 0x6b, 0x66, 0x73, 0x2e, 0x66, 0x61, 0x74, 0x00,
        0x02, 0x80, 0x80, 0x00, 0x02, 0x00, 0x08, 0x00, 0x00, 0xf8, 0x80, 0x00,
        0x20, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x00,
        0x80, 0x00, 0x29, 0x78, 0x56, 0x34, 0x12, 0x50, 0x49, 0x43, 0x4f, 0x44,
        0x49, 0x53, 0x4b, 0x20, 0x20, 0x20, 0x46, 0x41, 0x54, 0x31, 0x36, 0x20,
        0x20, 0x20, 0x0e, 0x1f, 0xbe, 0x5b, 0x7c, 0xac, 0x22, 0xc0, 0x74, 0x0b,
        0x56, 0xb4, 0x0e, 0xbb, 0x07, 0x00, 0xcd, 0x10, 0x5e, 0xeb, 0xf0, 0x32,
        0xe4, 0xcd, 0x16, 0xcd, 0x19, 0xeb, 0xfe, 0x3a, 0x28, 0x0d, 0x0a, 0x00
    };
    memset(buff, 0, 512);
    memcpy(buff, hdr, sizeof(hdr));
    buff[0x1fe] = 0x55;
    buff[0x1ff] = 0xff;
}

static char _toLegalFATChar(char c) {
    const char *odds = "!#$%&'()-@^_`{}~";
    c = toupper(c);
    if (((c >= '0') && (c <= '9')) || ((c >= 'A') && (c <= 'Z')) || strchr(odds, c)) {
        return c;
    } else {
        return '~';
    }
}

void SingleFileDrive::directorySector(char buff[512]) {
    const uint8_t lbl[] = {
        0x50, 0x49, 0x43, 0x4f, 0x44, 0x49, 0x53, 0x4b, 0x20, 0x20, 0x20, 0x08, 0x00, 0x00, 0xac, 0x56,
        0x82, 0x55, 0x82, 0x55, 0x00, 0x00, 0xac, 0x56, 0x82, 0x55
    }; //, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    memset(buff, 0, 512);
    memcpy(buff, lbl, sizeof(lbl));
    buff += 32; // Skip the just-set label

    // Create a legal 11-char UPPERCASE FILENAME WITH 0x20 PAD
    char SFN[11];
    memset(SFN, ' ', 11);
    for (int i = 0; (i < 8) && _dosFile[i] && (_dosFile[i] != '.'); i++) {
        SFN[i] = _toLegalFATChar(_dosFile[i]);
    }
    char *dot = _dosFile + strlen(_dosFile) - 1;
    while ((dot >= _dosFile) && (*dot != '.')) {
        dot--;
    }
    if (*dot == '.') {
        dot++;
        for (int i = 0; (i < 3) && dot[i]; i++) {
            SFN[8 + i] = _toLegalFATChar(dot[i]);
        }
    }
    uint8_t chksum = 0; // for LFN
    for (int i = 0; i < 11; i++) {
        chksum = (chksum >> 1) + (chksum << 7) + SFN[i];
    }

    // Create LFN structure
    int entries = (strlen(_dosFile) + 12) / 13; // round up
    for (int i = 0; i < entries; i++) {
        *buff++ = (entries - i) | (i == 0 ? 0x40 : 0);
        const char *partname = _dosFile + 13 * (entries - i - 1);
        for (int j = 0; j < 13; j++) {
            uint16_t u;
            if (j > (int)strlen(partname)) {
                u = 0xffff;
            } else {
                u = partname[j] & 0xff;
            }
            *buff++ = u & 0xff;
            *buff++ = (u >> 8) & 0xff;
            if (j == 4) {
                *buff++ = 0x0f; // LFN ATTR
                *buff++ = 0;
                *buff++ = chksum;
            } else if (j == 10) {
                *buff++ = 0;
                *buff++ = 0;
            }
        }
    }

    // Create SFN
    memset(buff, 0, 32);
    for (int i = 0; i < 11; i++) {
        buff[i] = SFN[i];
    }
    buff[0x0b] = 0x20; // ATTR = Archive
    // Ignore creation data/time, etc.
    buff[0x1a] = 0x03; // Starting cluster 3
    File f = LittleFS.open(_localFile, "r");
    int size = f.size();
    f.close();
    buff[0x1c] = size & 255;
    buff[0x1d] = (size >> 8) & 255;
    buff[0x1e] = (size >> 16) & 255; // 16MB or smaller
}

void SingleFileDrive::fatSector(char fat[512]) {
    memset(fat, 0, 512);
    fat[0x00] = 0xff;
    fat[0x01] = 0xf8;
    fat[0x02] = 0xff;
    fat[0x03] = 0xff;
    int cluster = 3;
    File f = LittleFS.open(_localFile, "r");
    int size = f.size();
    f.close();
    while (size > 65536) {
        fat[cluster * 2] = (cluster + 1) & 0xff;
        fat[cluster * 2 + 1] = ((cluster + 1) >> 8) & 0xff;
        cluster++;
        size -= 65536;
    }
    fat[cluster * 2] = 0xff;
    fat[cluster * 2 + 1] = 0xff;
}

// Invoked to determine max LUN
extern "C" uint8_t tud_msc_get_maxlun_cb(void) {
    return 1;
}

// Invoked when received SCSI_CMD_INQUIRY
// Application fill vendor id, product id and revision with string up to 8, 16, 4 characters respectively
extern "C" void tud_msc_inquiry_cb(uint8_t lun, uint8_t vendor_id[8], uint8_t product_id[16], uint8_t product_rev[4]) {
    (void) lun;

    const char vid[] = "PicoDisk";
    const char pid[] = "Mass Storage";
    const char rev[] = "1.0";

    memcpy(vendor_id, vid, strlen(vid));
    memcpy(product_id, pid, strlen(pid));
    memcpy(product_rev, rev, strlen(rev));
}

bool SingleFileDrive::testUnitReady() {
    return _started;
}

// Invoked when received Test Unit Ready command.
// return true allowing host to read/write this LUN e.g SD card inserted
extern "C" bool tud_msc_test_unit_ready_cb(uint8_t lun) {
    (void) lun;

    return singleFileDrive.testUnitReady();
}

// Invoked when received SCSI_CMD_READ_CAPACITY_10 and SCSI_CMD_READ_FORMAT_CAPACITY to determine the disk size
// Application update block count and block size
extern "C" void tud_msc_capacity_cb(uint8_t lun, uint32_t* block_count, uint16_t* block_size) {
    (void) lun;
    *block_count = _hddsects;
    *block_size  = 512;
}


// Callback invoked when received READ10 command.
// Copy disk's data to buffer (up to bufsize) and return number of copied bytes.
extern "C" int32_t tud_msc_read10_cb(uint8_t lun, uint32_t lba, uint32_t offset, void* buffer, uint32_t bufsize) {
    (void) lun;
    return singleFileDrive.read10(lba, offset, buffer, bufsize);
}

int32_t SingleFileDrive::read10(uint32_t lba, uint32_t offset, void* buffer, uint32_t bufsize) {
    if (!_started || (lba >= _hddsects)) {
        return -1;
    }

    uint32_t toread = bufsize;
    uint8_t *curbuff = (uint8_t *)buffer;

    while (bufsize > 0) {
        if (lba == 0) {
            bootSector(_sectBuff);
        } else if ((lba == 128) || (lba == 256)) {
            fatSector(_sectBuff);
        } else if (lba == 384) {
            directorySector(_sectBuff);
        } else if (lba >= 640) {
            File f = LittleFS.open(_localFile, "r");
            f.seek((lba - 640) * 512);
            f.read((uint8_t*)_sectBuff, 512);
            f.close();
        } else {
            memset(_sectBuff, 0, sizeof(_sectBuff));
        }

        uint32_t cplen = 512 - offset;
        if (bufsize < cplen) {
            cplen = bufsize;
        }
        memcpy(curbuff, _sectBuff + offset, cplen);
        curbuff += cplen;
        offset = 0;
        lba++;
        bufsize -= cplen;
    }

    return toread;
}

extern "C" bool tud_msc_is_writable_cb(uint8_t lun) {
    (void) lun;

    return true;
}

// Callback invoked when received WRITE10 command.
// Process data in buffer to disk's storage and return number of written bytes
extern "C" int32_t tud_msc_write10_cb(uint8_t lun, uint32_t lba, uint32_t offset, uint8_t* buffer, uint32_t bufsize) {
    (void) lun;
    return singleFileDrive.write10(lba, offset, buffer, bufsize);
}

int32_t SingleFileDrive::write10(uint32_t lba, uint32_t offset, uint8_t* buffer, uint32_t bufsize) {
    if (!_started || (lba >= _hddsects)) {
        return -1;
    }

    uint32_t addr = lba * 512 + offset;
    uint32_t hotspot = 384 * 512 + 0x20;
    if ((addr > hotspot) || (addr + bufsize < hotspot)) {
        // Did not try and erase the file entry, ignore
        return bufsize;
    }
    int off = hotspot - addr;
    uint8_t *ptr = (uint8_t *)buffer;
    ptr += off;
    if (*ptr == 0xe5) {
        if (_cbDelete) {
            _cbDelete(_cbDeleteData);
        }
    }

    return bufsize;
}

extern "C" bool tud_msc_set_sense(uint8_t lun, uint8_t sense_key, uint8_t add_sense_code, uint8_t add_sense_qualifier);

// Callback invoked when received an SCSI command not in built-in list below
// - READ_CAPACITY10, READ_FORMAT_CAPACITY, INQUIRY, MODE_SENSE6, REQUEST_SENSE
// - READ10 and WRITE10 has their own callbacks
extern "C" int32_t tud_msc_scsi_cb(uint8_t lun, uint8_t const scsi_cmd[16], void* buffer, uint16_t bufsize) {
    const int SCSI_CMD_PREVENT_ALLOW_MEDIUM_REMOVAL = 0x1E;
    const int SCSI_CMD_START_STOP_UNIT              = 0x1B;
    const int SCSI_SENSE_ILLEGAL_REQUEST = 0x05;

    void const* response = NULL;
    int32_t resplen = 0;

    // most scsi handled is input
    bool in_xfer = true;
    scsi_start_stop_unit_t const * start_stop = (scsi_start_stop_unit_t const *) scsi_cmd;
    switch (scsi_cmd[0]) {
    case SCSI_CMD_PREVENT_ALLOW_MEDIUM_REMOVAL:
        // Host is about to read/write etc ... better not to disconnect disk
        if (scsi_cmd[4] & 1) {
            singleFileDrive.plug();
        }
        resplen = 0;
        break;
    case SCSI_CMD_START_STOP_UNIT:
        // Host try to eject/safe remove/poweroff us. We could safely disconnect with disk storage, or go into lower power
        if (!start_stop->start && start_stop->load_eject) {
            singleFileDrive.unplug();
        } else if (start_stop->start && start_stop->load_eject) {
            singleFileDrive.plug();
        }
        resplen = 0;
        break;
    default:
        // Set Sense = Invalid Command Operation
        tud_msc_set_sense(lun, SCSI_SENSE_ILLEGAL_REQUEST, 0x20, 0x00);
        // negative means error -> tinyusb could stall and/or response with failed status
        resplen = -1;
        break;
    }

    // return resplen must not larger than bufsize
    if (resplen > bufsize) {
        resplen = bufsize;
    }

    if (response && (resplen > 0)) {
        if (in_xfer) {
            memcpy(buffer, response, resplen);
        } else {
            // SCSI output
        }
    }

    return resplen;
}

void SingleFileDrive::plug() {
    if (_started && _cbPlug) {
        _cbPlug(_cbPlugData);
    }
}

void SingleFileDrive::unplug() {
    if (_started && _cbUnplug) {
        _cbUnplug(_cbUnplugData);
    }
}

// Callback invoked on start/stop
extern "C" bool tud_msc_start_stop_cb(uint8_t lun, uint8_t power_condition, bool start, bool load_eject) {
    (void) lun;
    (void) power_condition;
    if (start && load_eject) {
        singleFileDrive.plug();
    } else if (!start && load_eject) {
        singleFileDrive.unplug();
    }
    return true;
}
