/*
    Arduino OTA.h - Simple Arduino IDE OTA handler
    Modified 2022 Earle F. Philhower, III.  All rights reserved.

    Taken from the ESP8266 core libraries, (c) various authors.

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

#pragma once

#include <WiFi.h>
#include <functional>
#include <LittleFS.h>
#include <Updater.h>

class UdpContext;

typedef enum {
    OTA_IDLE,
    OTA_WAITAUTH,
    OTA_RUNUPDATE
} ota_state_t;

typedef enum {
    OTA_AUTH_ERROR,
    OTA_BEGIN_ERROR,
    OTA_CONNECT_ERROR,
    OTA_RECEIVE_ERROR,
    OTA_END_ERROR
} ota_error_t;


class ArduinoOTAClass {
public:
    typedef std::function<void(void)> THandlerFunction;
    typedef std::function<void(ota_error_t)> THandlerFunction_Error;
    typedef std::function<void(unsigned int, unsigned int)> THandlerFunction_Progress;

    ArduinoOTAClass();
    ~ArduinoOTAClass();

    //Sets the service port. Default 2040
    void setPort(uint16_t port);

    //Sets the device hostname. Default pico-xxxxxx
    void setHostname(const char *hostname);
    String getHostname();

    //Sets the password that will be required for OTA. Default nullptr
    void setPassword(const char *password);

    //Sets the password as above but in the form MD5(password). Default nullptr
    void setPasswordHash(const char *password);

    //Sets if the device should be rebooted after successful update. Default true
    void setRebootOnSuccess(bool reboot);

    //This callback will be called when OTA connection has begun
    void onStart(THandlerFunction fn);

    //This callback will be called when OTA has finished
    void onEnd(THandlerFunction fn);

    //This callback will be called when OTA encountered Error
    void onError(THandlerFunction_Error fn);

    //This callback will be called when OTA is receiving data
    void onProgress(THandlerFunction_Progress fn);

    //Starts the ArduinoOTA service
    void begin(bool useMDNS = true);

    //Ends the ArduinoOTA service
    void end();
    //Call this in loop() to run the service. Also calls MDNS.update() when begin() or begin(true) is used.
    void handle();

    //Gets update command type after OTA has started. Either U_FLASH or U_FS
    int getCommand();

private:
    void _runUpdate(void);
    void _onRx(void);
    int parseInt(void);
    String readStringUntil(char end);

    int _port = 0;
    String _password;
    String _hostname;
    String _nonce;
    UdpContext *_udp_ota = nullptr;
    bool _initialized = false;
    bool _rebootOnSuccess = true;
    bool _useMDNS = true;
    ota_state_t _state = OTA_IDLE;
    int _size = 0;
    int _cmd = 0;
    uint16_t _ota_port = 0;
    uint16_t _ota_udp_port = 0;
    IPAddress _ota_ip;
    String _md5;

    THandlerFunction _start_callback = nullptr;
    THandlerFunction _end_callback = nullptr;
    THandlerFunction_Error _error_callback = nullptr;
    THandlerFunction_Progress _progress_callback = nullptr;
};

#if !defined(NO_GLOBAL_INSTANCES) && !defined(NO_GLOBAL_ARDUINOOTA)
extern ArduinoOTAClass ArduinoOTA;
#endif
