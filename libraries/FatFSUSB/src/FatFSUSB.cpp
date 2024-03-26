/*
    FatFSUSB - Export onboard FatFS/FTL to host for data movement
    Copyright (c) 2024 Earle F. Philhower, III.  All rights reserved.

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

#include "FatFSUSB.h"
#include <FatFS.h>
#include <class/msc/msc.h>

FatFSUSBClass FatFSUSB;

// Ensure we are logged in to the USB framework
void __USBInstallMassStorage() {
    /* dummy */
}

FatFSUSBClass::FatFSUSBClass() {
}

FatFSUSBClass::~FatFSUSBClass() {
    end();
}

void FatFSUSBClass::onPlug(void (*cb)(uint32_t), uint32_t cbData) {
    _cbPlug = cb;
    _cbPlugData = cbData;
}

void FatFSUSBClass::onUnplug(void (*cb)(uint32_t), uint32_t cbData) {
    _cbUnplug = cb;
    _cbUnplugData = cbData;
}

void FatFSUSBClass::driveReady(bool (*cb)(uint32_t), uint32_t cbData) {
    _driveReady = cb;
    _driveReadyData = cbData;
}

bool FatFSUSBClass::begin() {
    if (_started) {
        return false;
    }
    _started = true;
    fatfs::disk_initialize(0);
    fatfs::WORD ss;
    fatfs::disk_ioctl(0, GET_SECTOR_SIZE, &ss);
    _sectSize = ss;
    _sectBuff = new uint8_t[_sectSize];
    _sectNum = -1;
    return true;
}

void FatFSUSBClass::end() {
    if (_started) {
        _started = false;
        delete[] _sectBuff;
    }
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

bool FatFSUSBClass::testUnitReady() {
    bool ret = _started;
    if (_driveReady) {
        ret &= _driveReady(_driveReadyData);
    }
    return ret;
}

// Invoked when received Test Unit Ready command.
// return true allowing host to read/write this LUN e.g SD card inserted
extern "C" bool tud_msc_test_unit_ready_cb(uint8_t lun) {
    (void) lun;

    return FatFSUSB.testUnitReady();
}

// Invoked when received SCSI_CMD_READ_CAPACITY_10 and SCSI_CMD_READ_FORMAT_CAPACITY to determine the disk size
// Application update block count and block size
extern "C" void tud_msc_capacity_cb(uint8_t lun, uint32_t* block_count, uint16_t* block_size) {
    (void) lun;
    fatfs::LBA_t p;
    fatfs::disk_ioctl(0, GET_SECTOR_COUNT, &p);
    *block_count = p;
    fatfs::WORD ss;
    fatfs::disk_ioctl(0, GET_SECTOR_SIZE, &ss);
    *block_size  = ss;
}


// Callback invoked when received READ10 command.
// Copy disk's data to buffer (up to bufsize) and return number of copied bytes.
extern "C" int32_t tud_msc_read10_cb(uint8_t lun, uint32_t lba, uint32_t offset, void* buffer, uint32_t bufsize) {
    (void) lun;
    return FatFSUSB.read10(lba, offset, buffer, bufsize);
}

int32_t FatFSUSBClass::read10(uint32_t lba, uint32_t offset, void* buffer, uint32_t bufsize) {
    fatfs::LBA_t _hddsects;
    fatfs::disk_ioctl(0, GET_SECTOR_COUNT, &_hddsects);

    if (!_started || (lba >= _hddsects)) {
        return -1;
    }

    assert(offset + bufsize <= _sectSize);

    if (_sectNum >= 0) {
        // Flush the temp data out, we need to use the space
        fatfs::disk_write(0, _sectBuff, _sectNum, 1);
        _sectNum = -1;
    }
    fatfs::disk_read(0, _sectBuff, lba, 1);
    memcpy(buffer, _sectBuff + offset, bufsize);
    return bufsize;
}

extern "C" bool tud_msc_is_writable_cb(uint8_t lun) {
    (void) lun;

    return true;
}

// Callback invoked when received WRITE10 command.
// Process data in buffer to disk's storage and return number of written bytes
extern "C" int32_t tud_msc_write10_cb(uint8_t lun, uint32_t lba, uint32_t offset, uint8_t* buffer, uint32_t bufsize) {
    (void) lun;
    return FatFSUSB.write10(lba, offset, buffer, bufsize);
}

int32_t FatFSUSBClass::write10(uint32_t lba, uint32_t offset, uint8_t* buffer, uint32_t bufsize) {
    fatfs::LBA_t _hddsects;
    fatfs::disk_ioctl(0, GET_SECTOR_COUNT, &_hddsects);

    if (!_started || (lba >= _hddsects)) {
        return -1;
    }

    assert(offset + bufsize <= _sectSize);

    if ((offset == 0) && (bufsize == _sectSize)) {
        fatfs::disk_write(0, buffer, lba, 1);
        return _sectSize;
    }

    if ((int)_sectNum == (int)lba) {
        memcpy(_sectBuff + offset, buffer, bufsize);
    } else {
        if (_sectNum >= 0) {
            // Need to flush old sector out
            fatfs::disk_write(0, _sectBuff, _sectNum, 1);
        }
        fatfs::disk_read(0, _sectBuff, lba, 1);
        memcpy(_sectBuff + offset, buffer, bufsize);
        _sectNum = lba;
    }

    if (offset + bufsize >= _sectSize) {
        // We've filled up a sector, write it out!
        fatfs::disk_write(0, _sectBuff, lba, 1);
        _sectNum = -1;
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
            FatFSUSB.plug();
        }
        resplen = 0;
        break;
    case SCSI_CMD_START_STOP_UNIT:
        // Host try to eject/safe remove/poweroff us. We could safely disconnect with disk storage, or go into lower power
        if (!start_stop->start && start_stop->load_eject) {
            FatFSUSB.unplug();
        } else if (start_stop->start && start_stop->load_eject) {
            FatFSUSB.plug();
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

void FatFSUSBClass::plug() {
    if (_started && _cbPlug) {
        _cbPlug(_cbPlugData);
    }
}

void FatFSUSBClass::unplug() {
    if (_started) {
        if (_sectNum >= 0) {
            // Flush the temp data out
            fatfs::disk_write(0, _sectBuff, _sectNum, 1);
            _sectNum = -1;
        }
        fatfs::disk_ioctl(0, CTRL_SYNC, nullptr);
        if (_cbUnplug) {
            _cbUnplug(_cbUnplugData);
        }
    }
}

// Callback invoked on start/stop
extern "C" bool tud_msc_start_stop_cb(uint8_t lun, uint8_t power_condition, bool start, bool load_eject) {
    (void) lun;
    (void) power_condition;
    if (start && load_eject) {
        FatFSUSB.plug();
    } else if (!start && load_eject) {
        FatFSUSB.unplug();
    }
    return true;
}
