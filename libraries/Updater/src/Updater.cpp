/*
    Updater - Handles FS or app updates using PicoOTA
    Adapted from the ESP8266 Updater class
    Copyright (c) 2022 Earle F. Philhower, III <earlephilhower@yahoo.com>

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

#include "Updater.h"
#include <PolledTimeout.h>
#include "StackThunk.h"
#include "LittleFS.h"
#include <hardware/flash.h>
#include <PicoOTA.h>

#include <Updater_Signing.h>
#ifndef ARDUINO_SIGNING
#define ARDUINO_SIGNING 0
#endif

extern uint8_t _FS_start;
extern uint8_t _FS_end;


#if ARDUINO_SIGNING
namespace esp8266 {
extern UpdaterHashClass& updaterSigningHash;
extern UpdaterVerifyClass& updaterSigningVerifier;
}
#endif

UpdaterClass::UpdaterClass() {
#if ARDUINO_SIGNING
    installSignature(&esp8266::updaterSigningHash, &esp8266::updaterSigningVerifier);
    stack_thunk_add_ref();
#endif
}

UpdaterClass::~UpdaterClass() {
#if ARDUINO_SIGNING
    stack_thunk_del_ref();
#endif
}

UpdaterClass& UpdaterClass::onProgress(THandlerFunction_Progress fn) {
    _progress_callback = fn;
    return *this;
}

void UpdaterClass::_reset() {
    if (_buffer) {
        delete[] _buffer;
    }
    _buffer = 0;
    _bufferLen = 0;
    _startAddress = 0;
    _currentAddress = 0;
    _size = 0;
    _command = U_FLASH;
}

bool UpdaterClass::begin(size_t size, int command) {
    uint32_t updateStartAddress;
    if (_size > 0) {
#ifdef DEBUG_UPDATER
        DEBUG_UPDATER.println(F("[begin] already running"));
#endif
        return false;
    }

#ifdef DEBUG_UPDATER
    if (command == U_FS) {
        DEBUG_UPDATER.println(F("[begin] Update Filesystem."));
    }
#endif

    if (size == 0) {
        _setError(UPDATE_ERROR_SIZE);
        return false;
    }

    _reset();
    clearError(); //  _error = 0
    _target_md5 = "";
    _md5 = MD5Builder();

    if (command == U_FLASH) {
        LittleFS.begin();
        _fp = LittleFS.open("firmware.bin", "w+");
        if (!_fp) {
#ifdef DEBUG_UPDATER
            DEBUG_UPDATER.println(F("[begin] unable to create file"));
#endif
            return false;
        }
        updateStartAddress = 0;  // Not used
    } else if (command == U_FS) {
        if (&_FS_start + size > &_FS_end) {
            _setError(UPDATE_ERROR_SPACE);
            return false;
        }

        updateStartAddress = (uint32_t)&_FS_start;
    } else {
        // unknown command
#ifdef DEBUG_UPDATER
        DEBUG_UPDATER.println(F("[begin] Unknown update command."));
#endif
        return false;
    }

    //initialize
    _startAddress = updateStartAddress;
    _size = size;
    _bufferSize = 4096;
    _buffer = new uint8_t[_bufferSize];
    _command = command;

#ifdef DEBUG_UPDATER
    DEBUG_UPDATER.printf_P(PSTR("[begin] _startAddress:     0x%08X (%d)\n"), _startAddress, _startAddress);
    DEBUG_UPDATER.printf_P(PSTR("[begin] _size:             0x%08zX (%zd)\n"), _size, _size);
#endif

    if (!_verify) {
        _md5.begin();
    }
    return true;
}

bool UpdaterClass::setMD5(const char * expected_md5) {
    if (strlen(expected_md5) != 32) {
        return false;
    }
    _target_md5 = expected_md5;
    return true;
}

bool UpdaterClass::end(bool evenIfRemaining) {
    if (_size == 0) {
#ifdef DEBUG_UPDATER
        DEBUG_UPDATER.println(F("no update"));
#endif
        _reset();
        return false;
    }

    // Updating w/o any data is an error we detect here
    if (!progress()) {
        _setError(UPDATE_ERROR_NO_DATA);
    }

    if (hasError() || (!isFinished() && !evenIfRemaining)) {
#ifdef DEBUG_UPDATER
        DEBUG_UPDATER.printf_P(PSTR("premature end: res:%u, pos:%zu/%zu\n"), getError(), progress(), _size);
#endif
        _reset();
        return false;
    }

    if (evenIfRemaining) {
        if (_bufferLen > 0) {
            _writeBuffer();
        }
        _size = progress();
    }

    if (_verify && (_command == U_FLASH)) {
        const uint32_t expectedSigLen = _verify->length();
        // If expectedSigLen is non-zero, we expect the last four bytes of the buffer to
        // contain a matching length field, preceded by the bytes of the signature itself.
        // But if expectedSigLen is zero, we expect neither a signature nor a length field;
        uint32_t sigLen = 0;

        if (expectedSigLen > 0) {
            _fp.seek(_size - sizeof(uint32_t));
            _fp.read((uint8_t *)&sigLen, sizeof(uint32_t));
        }
#ifdef DEBUG_UPDATER
        DEBUG_UPDATER.printf_P(PSTR("[Updater] sigLen: %d\n"), sigLen);
#endif
        if (sigLen != expectedSigLen) {
            _setError(UPDATE_ERROR_SIGN);
            _reset();
            return false;
        }

        int binSize = _size;
        if (expectedSigLen > 0) {
            binSize -= (sigLen + sizeof(uint32_t) /* The siglen word */);
        }
        _hash->begin();
#ifdef DEBUG_UPDATER
        DEBUG_UPDATER.printf_P(PSTR("[Updater] Adjusted binsize: %d\n"), binSize);
#endif
        // Calculate the MD5 and hash using proper size
        uint8_t buff[128] __attribute__((aligned(4)));
        _fp.seek(0);
        for (int i = 0; i < binSize; i += sizeof(buff)) {
            _fp.read(buff, sizeof(buff));
            size_t read = std::min((int)sizeof(buff), binSize - i);
            _hash->add(buff, read);
        }
        _hash->end();
#ifdef DEBUG_UPDATER
        unsigned char *ret = (unsigned char *)_hash->hash();
        DEBUG_UPDATER.printf_P(PSTR("[Updater] Computed Hash:"));
        for (int i = 0; i < _hash->len(); i++) {
            DEBUG_UPDATER.printf(" %02x", ret[i]);
        }
        DEBUG_UPDATER.printf("\n");
#endif

        uint8_t *sig = nullptr; // Safe to free if we don't actually malloc
        if (expectedSigLen > 0) {
            sig = (uint8_t*)malloc(sigLen);
            if (!sig) {
                _setError(UPDATE_ERROR_SIGN);
                _reset();
                return false;
            }
            _fp.seek(_startAddress + binSize);
            _fp.read((uint8_t *)sig, sigLen);
#ifdef DEBUG_UPDATER
            DEBUG_UPDATER.printf_P(PSTR("[Updater] Received Signature:"));
            for (size_t i = 0; i < sigLen; i++) {
                DEBUG_UPDATER.printf(" %02x", sig[i]);
            }
            DEBUG_UPDATER.printf("\n");
#endif
        }
        if (!_verify->verify(_hash, (void *)sig, sigLen)) {
            free(sig);
            _setError(UPDATE_ERROR_SIGN);
            _reset();
            return false;
        }
        free(sig);
        _size = binSize; // Adjust size to remove signature, not part of bin payload

#ifdef DEBUG_UPDATER
        DEBUG_UPDATER.printf_P(PSTR("[Updater] Signature matches\n"));
#endif
    } else if (_target_md5.length()) {
        _md5.calculate();
        if (strcasecmp(_target_md5.c_str(), _md5.toString().c_str())) {
            _setError(UPDATE_ERROR_MD5);
            return false;
        }
#ifdef DEBUG_UPDATER
        else {
            DEBUG_UPDATER.printf_P(PSTR("MD5 Success: %s\n"), _target_md5.c_str());
        }
#endif
    }

    if (!_verifyEnd()) {
        _reset();
        return false;
    }

    if (_command == U_FLASH) {
        _fp.close();
        picoOTA.begin();
        picoOTA.addFile("firmware.bin");
        picoOTA.commit();
#ifdef DEBUG_UPDATER
        DEBUG_UPDATER.printf_P(PSTR("Staged: address:0x%08X, size:0x%08zX\n"), _startAddress, _size);
#endif
    }

    _reset();
    return true;
}

bool UpdaterClass::_writeBuffer() {
    if (_command == U_FLASH) {
        if (_bufferLen != _fp.write(_buffer, _bufferLen)) {
            return false;
        }
    } else {
        noInterrupts();
        rp2040.idleOtherCore();
        flash_range_erase((intptr_t)_currentAddress - (intptr_t)XIP_BASE, 4096);
        flash_range_program((intptr_t)_currentAddress - (intptr_t)XIP_BASE, _buffer, 4096);
        rp2040.resumeOtherCore();
        interrupts();
    }
    if (!_verify) {
        _md5.add(_buffer, _bufferLen);
    }
    _currentAddress += _bufferLen;
    _bufferLen = 0;
    return true;
}

size_t UpdaterClass::write(uint8_t *data, size_t len) {
    if (hasError() || !isRunning()) {
        return 0;
    }

    if (progress() + _bufferLen + len > _size) {
        _setError(UPDATE_ERROR_SPACE);
        return 0;
    }

    size_t left = len;

    while ((_bufferLen + left) > _bufferSize) {
        size_t toBuff = _bufferSize - _bufferLen;
        memcpy(_buffer + _bufferLen, data + (len - left), toBuff);
        _bufferLen += toBuff;
        if (!_writeBuffer()) {
            return len - left;
        }
        left -= toBuff;
        if (!_async) {
            yield();
        }
    }
    //lets see what's left
    memcpy(_buffer + _bufferLen, data + (len - left), left);
    _bufferLen += left;
    if (_bufferLen == remaining()) {
        //we are at the end of the update, so should write what's left to flash
        if (!_writeBuffer()) {
            return len - left;
        }
    }
    return len;
}

bool UpdaterClass::_verifyHeader(uint8_t data) {
    (void) data;
    // No special header on RP2040
    return true;
}

bool UpdaterClass::_verifyEnd() {
    return true;
}

size_t UpdaterClass::writeStream(Stream &data, uint16_t streamTimeout) {
    size_t written = 0;
    size_t toRead = 0;
    if (hasError() || !isRunning()) {
        return 0;
    }

    if (!_verifyHeader(data.peek())) {
#ifdef DEBUG_UPDATER
        printError(DEBUG_UPDATER);
#endif
        _reset();
        return 0;
    }
    esp8266::polledTimeout::oneShotMs timeOut(streamTimeout);
    if (_progress_callback) {
        _progress_callback(0, _size);
    }

    while (remaining()) {
        size_t bytesToRead = _bufferSize - _bufferLen;
        if (bytesToRead > remaining()) {
            bytesToRead = remaining();
        }
        toRead = data.readBytes(_buffer + _bufferLen,  bytesToRead);
        if (toRead == 0) { //Timeout
            if (timeOut) {
                _currentAddress = (_startAddress + _size);
                _setError(UPDATE_ERROR_STREAM);
                _reset();
                return written;
            }
            delay(100);
        } else {
            timeOut.reset();
        }
        _bufferLen += toRead;
        if ((_bufferLen == remaining() || _bufferLen == _bufferSize) && !_writeBuffer()) {
            return written;
        }
        written += toRead;
        if (_progress_callback) {
            _progress_callback(progress(), _size);
        }
        yield();
    }
    if (_progress_callback) {
        _progress_callback(progress(), _size);
    }
    return written;
}

void UpdaterClass::_setError(int error) {
    _error = error;
#ifdef DEBUG_UPDATER
    printError(DEBUG_UPDATER);
#endif
    _reset(); // Any error condition invalidates the entire update, so clear partial status
}

void UpdaterClass::printError(Print &out) {
    String err;
    err = "ERROR[";
    err += _error;
    err += "]: ";
    if (_error == UPDATE_ERROR_OK) {
        err += "No Error";
    } else if (_error == UPDATE_ERROR_WRITE) {
        err += "Flash Write Failed";
    } else if (_error == UPDATE_ERROR_ERASE) {
        err += "Flash Erase Failed";
    } else if (_error == UPDATE_ERROR_READ) {
        err += "Flash Read Failed";
    } else if (_error == UPDATE_ERROR_SPACE) {
        err += "Not Enough Space";
    } else if (_error == UPDATE_ERROR_SIZE) {
        err += "Bad Size Given";
    } else if (_error == UPDATE_ERROR_STREAM) {
        err += "Stream Read Timeout";
    } else if (_error == UPDATE_ERROR_NO_DATA) {
        err += "No data supplied";
    } else if (_error == UPDATE_ERROR_MD5) {
        err += "MD5 Failed: expected:";
        err += _target_md5.c_str();
        err += " calculated:";
        err += _md5.toString();
    } else if (_error == UPDATE_ERROR_SIGN) {
        err += "Signature verification failed";
    } else {
        err += "UNKNOWN";
    }
    out.println(err.c_str());
}

UpdaterClass Update;
