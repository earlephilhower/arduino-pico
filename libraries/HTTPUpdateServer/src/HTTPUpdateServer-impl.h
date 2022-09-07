/*
    HTTPUpdateServer - Support simple web based OTA
    Modified 2022 Earle F. Philhower, III.  All rights reserved.

    Ported from the ESP8266 Arduino core, (c) copyright multiple authors

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

#include <Arduino.h>
#include <WiFiClient.h>
#include <WiFiServer.h>
#include <WebServer.h>
#include <WiFiUdp.h>
#include <LittleFS.h>
#include "StreamString.h"
#include "HTTPUpdateServer.h"

extern uint8_t _FS_start;
extern uint8_t _FS_end;


static const char serverIndex[] PROGMEM =
    R"(<!DOCTYPE html>
     <html lang='en'>
     <head>
         <meta charset='utf-8'>
         <meta name='viewport' content='width=device-width,initial-scale=1'/>
     </head>
     <body>
     <form method='POST' action='' enctype='multipart/form-data'>
         Firmware:<br>
         <input type='file' accept='.bin,.bin.gz' name='firmware'>
         <input type='submit' value='Update Firmware'>
     </form>
     <form method='POST' action='' enctype='multipart/form-data'>
         FileSystem:<br>
         <input type='file' accept='.bin,.bin.gz' name='filesystem'>
         <input type='submit' value='Update FileSystem'>
     </form>
     </body>
     </html>)";
static const char successResponse[] PROGMEM =
    "<META http-equiv=\"refresh\" content=\"15;URL=/\">Update Success! Rebooting...";

template <typename ServerType, int ServerPort>
HTTPUpdateServerTemplate<ServerType, ServerPort>::HTTPUpdateServerTemplate(bool serial_debug) {
    _serial_output = serial_debug;
    _server = nullptr;
    _username = "";
    _password = "";
    _authenticated = false;
}

template <typename ServerType, int ServerPort>
void HTTPUpdateServerTemplate<ServerType, ServerPort>::setup(WebServerTemplate<ServerType, ServerPort> *server, const String& path, const String& username, const String& password) {
    _server = server;
    _username = username;
    _password = password;

    // handler for the /update form page
    _server->on(path.c_str(), HTTP_GET, [&]() {
        if (_username != "" && _password != "" && !_server->authenticate(_username.c_str(), _password.c_str())) {
            return _server->requestAuthentication();
        }
        _server->send_P(200, PSTR("text/html"), serverIndex);
    });

    // handler for the /update form page - preflight options
    _server->on(path.c_str(), HTTP_OPTIONS, [&]() {
        _server->sendHeader("Access-Control-Allow-Headers", "*");
        _server->sendHeader("Access-Control-Allow-Origin", "*");
        _server->send(200, F("text/html"), String(F("y")));
    }, [&]() {
        _authenticated = (_username == "" || _password == "" || _server->authenticate(_username.c_str(), _password.c_str()));
        if (!_authenticated) {
            if (_serial_output) {
                Serial.printf("Unauthenticated Update\n");
            }
            return;
        }
    });

    // handler for the /update form POST (once file upload finishes)
    _server->on(path.c_str(), HTTP_POST, [&]() {
        _server->sendHeader("Access-Control-Allow-Headers", "*");
        _server->sendHeader("Access-Control-Allow-Origin", "*");
        if (!_authenticated) {
            return _server->requestAuthentication();
        }
        if (Update.hasError()) {
            _server->send(200, F("text/html"), String(F("Update error: ")) + _updaterError);
        } else {
            _server->client().setNoDelay(true);
            _server->send_P(200, PSTR("text/html"), successResponse);
            delay(100);
            _server->client().stop();
            rp2040.restart();
        }
    }, [&]() {
        // handler for the file upload, gets the sketch bytes, and writes
        // them through the Update object
        HTTPUpload& upload = _server->upload();

        if (upload.status == UPLOAD_FILE_START) {
            _updaterError = "";

            _authenticated = (_username == "" || _password == "" || _server->authenticate(_username.c_str(), _password.c_str()));
            if (!_authenticated) {
                if (_serial_output) {
                    Serial.printf("Unauthenticated Update\n");
                }
                return;
            }

            if (_serial_output) {
                Serial.printf("Update: %s\n", upload.filename.c_str());
            }
            if (upload.name == "filesystem") {
                size_t fsSize = ((size_t)&_FS_end - (size_t)&_FS_start);
                LittleFS.end();
                if (!Update.begin(fsSize, U_FS)) { //start with max available size
                    if (_serial_output) {
                        Update.printError(Serial);
                    }
                }
            } else {
                FSInfo64 i;
                LittleFS.begin();
                LittleFS.info64(i);
                uint32_t maxSketchSpace = i.totalBytes - i.usedBytes;
                if (!Update.begin(maxSketchSpace, U_FLASH)) { //start with max available size
                    _setUpdaterError();
                }
            }
        } else if (_authenticated && upload.status == UPLOAD_FILE_WRITE && !_updaterError.length()) {
            if (_serial_output) {
                Serial.printf(".");
            }
            if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
                _setUpdaterError();
            }
        } else if (_authenticated && upload.status == UPLOAD_FILE_END && !_updaterError.length()) {
            if (Update.end(true)) { //true to set the size to the current progress
                if (_serial_output) {
                    Serial.printf("Update Success: %zu\nRebooting...\n", upload.totalSize);
                }
            } else {
                _setUpdaterError();
            }
        } else if (_authenticated && upload.status == UPLOAD_FILE_ABORTED) {
            Update.end();
            if (_serial_output) {
                Serial.println("Update was aborted");
            }
        }
    });
}

template <typename ServerType, int ServerPort>
void HTTPUpdateServerTemplate<ServerType, ServerPort>::_setUpdaterError() {
    if (_serial_output) {
        Update.printError(Serial);
    }
    StreamString str;
    Update.printError(str);
    _updaterError = str.c_str();
}
