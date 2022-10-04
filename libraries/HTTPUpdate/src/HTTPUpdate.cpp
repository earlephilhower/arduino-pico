/**

    @file HTTPUpdate.cpp
    @date 21.06.2015
    @author Markus Sattler

    Copyright (c) 2015 Markus Sattler. All rights reserved.
    This file is part of the  Http Updater.

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

#include "HTTPUpdate.h"
#include <StreamString.h>

extern uint8_t _FS_start;
extern uint8_t _FS_end;

HTTPUpdate::HTTPUpdate(void)
    : _httpClientTimeout(8000) {
}

HTTPUpdate::HTTPUpdate(int httpClientTimeout)
    : _httpClientTimeout(httpClientTimeout) {
}

HTTPUpdate::~HTTPUpdate(void) {
}

/**
    set the Authorization for the http request
    @param user const String&
    @param password const String&
*/
void HTTPUpdate::setAuthorization(const String &user, const String &password) {
    _user = user;
    _password = password;
}

/**
    set the Authorization for the http request
    @param auth const String& base64
*/
void HTTPUpdate::setAuthorization(const String &auth) {
    _auth = auth;
}

HTTPUpdateResult HTTPUpdate::update(WiFiClient& client, const String& url, const String& currentVersion) {
    HTTPClient http;
    http.begin(client, url);
    return handleUpdate(http, currentVersion, false);
}

HTTPUpdateResult HTTPUpdate::updateFS(WiFiClient& client, const String& url, const String& currentVersion) {
    HTTPClient http;
    http.begin(client, url);
    return handleUpdate(http, currentVersion, true);
}

HTTPUpdateResult HTTPUpdate::update(WiFiClient& client, const String& host, uint16_t port, const String& uri,
                                    const String& currentVersion) {
    HTTPClient http;
    http.begin(client, host, port, uri);
    return handleUpdate(http, currentVersion, false);
}

HTTPUpdateResult HTTPUpdate::update(const String& url, const String& currentVersion) {
    HTTPClient http;
    http.begin(url);
    return handleUpdate(http, currentVersion, false);
}

HTTPUpdateResult HTTPUpdate::update(const String& host, uint16_t port, const String& uri, const String& currentVersion) {
    HTTPClient http;
    http.begin(host, port, uri);
    return handleUpdate(http, currentVersion, false);
}

HTTPUpdateResult HTTPUpdate::updateFS(const String& url, const String& currentVersion) {
    HTTPClient http;
    http.begin(url);
    return handleUpdate(http, currentVersion, true);
}

/**
    return error code as int
    @return int error code
*/
int HTTPUpdate::getLastError(void) {
    return _lastError;
}

/**
    return error code as String
    @return String error
*/
String HTTPUpdate::getLastErrorString(void) {

    if (_lastError == 0) {
        return String(); // no error
    }

    // error from Update class
    if (_lastError > 0) {
        StreamString error;
        Update.printError(error);
        error.trim(); // remove line ending
        return String(F("Update error: ")) + error;
    }

    // error from http client
    if (_lastError > -100) {
        return String(F("HTTP error: ")) + HTTPClient::errorToString(_lastError);
    }

    switch (_lastError) {
    case HTTP_UE_TOO_LESS_SPACE:
        return F("Not Enough space");
    case HTTP_UE_SERVER_NOT_REPORT_SIZE:
        return F("Server Did Not Report Size");
    case HTTP_UE_SERVER_FILE_NOT_FOUND:
        return F("File Not Found (404)");
    case HTTP_UE_SERVER_FORBIDDEN:
        return F("Forbidden (403)");
    case HTTP_UE_SERVER_WRONG_HTTP_CODE:
        return F("Wrong HTTP Code");
    case HTTP_UE_SERVER_FAULTY_MD5:
        return F("Wrong MD5");
    case HTTP_UE_BIN_VERIFY_HEADER_FAILED:
        return F("Verify Bin Header Failed");
    case HTTP_UE_BIN_FOR_WRONG_FLASH:
        return F("New Binary Does Not Fit Flash Size");
    case HTTP_UE_SERVER_UNAUTHORIZED:
        return F("Unauthorized (401)");
    }

    return String();
}


/**

    @param http HTTPClient
    @param currentVersion const char
    @return HTTPUpdateResult
*/
HTTPUpdateResult HTTPUpdate::handleUpdate(HTTPClient& http, const String& currentVersion, bool spiffs) {

    HTTPUpdateResult ret = HTTP_UPDATE_FAILED;

    // use HTTP/1.0 for update since the update handler not support any transfer Encoding
    http.useHTTP10(true);
    http.setTimeout(_httpClientTimeout);
    http.setFollowRedirects(_followRedirects);
    http.setUserAgent(F("Pico-HTTP-Update"));
    http.addHeader(F("x-Pico-Chip-ID"), String(rp2040.getChipID()));
    http.addHeader(F("x-Pico-STA-MAC"), WiFi.macAddress());
    http.addHeader(F("x-Pico-AP-MAC"), WiFi.softAPmacAddress());

    if (spiffs) {
        http.addHeader(F("x-Pico-Mode"), F("spiffs"));
    } else {
        http.addHeader(F("x-Pico-Mode"), F("sketch"));
    }

    if (currentVersion && currentVersion[0] != 0x00) {
        http.addHeader(F("x-Pico-Version"), currentVersion);
    }

    if (_user != "" && _password != "") {
        http.setAuthorization(_user.c_str(), _password.c_str());
    }

    if (_auth != "") {
        http.setAuthorization(_auth.c_str());
    }

    const char * headerkeys[] = { "x-MD5" };
    size_t headerkeyssize = sizeof(headerkeys) / sizeof(char*);

    // track these headers
    http.collectHeaders(headerkeys, headerkeyssize);


    int code = http.GET();
    int len = http.getSize();

    if (code <= 0) {
        DEBUG_HTTP_UPDATE("[httpUpdate] HTTP error: %s\n", http.errorToString(code).c_str());
        _setLastError(code);
        http.end();
        return HTTP_UPDATE_FAILED;
    }


    DEBUG_HTTP_UPDATE("[httpUpdate] Header read fin.\n");
    DEBUG_HTTP_UPDATE("[httpUpdate] Server header:\n");
    DEBUG_HTTP_UPDATE("[httpUpdate]  - code: %d\n", code);
    DEBUG_HTTP_UPDATE("[httpUpdate]  - len: %d\n", len);

    String md5;
    if (_md5Sum.length()) {
        md5 = _md5Sum;
    } else if (http.hasHeader("x-MD5")) {
        md5 = http.header("x-MD5");
    }
    if (md5.length()) {
        DEBUG_HTTP_UPDATE("[httpUpdate]  - MD5: %s\n", md5.c_str());
    }

    DEBUG_HTTP_UPDATE("[httpUpdate]  info:\n");

    if (currentVersion && currentVersion[0] != 0x00) {
        DEBUG_HTTP_UPDATE("[httpUpdate]  - current version: %s\n", currentVersion.c_str());
    }

    switch (code) {
    case HTTP_CODE_OK:  ///< OK (Start Update)
        if (len > 0) {
            bool startUpdate = true;
            if (spiffs) {
                size_t spiffsSize = ((size_t)&_FS_end - (size_t)&_FS_start);
                if (len > (int) spiffsSize) {
                    DEBUG_HTTP_UPDATE("[httpUpdate] spiffsSize to low (%d) needed: %d\n", spiffsSize, len);
                    startUpdate = false;
                }
            }

            if (!startUpdate) {
                _setLastError(HTTP_UE_TOO_LESS_SPACE);
                ret = HTTP_UPDATE_FAILED;
            } else {
                // Warn main app we're starting up...
                if (_cbStart) {
                    _cbStart();
                }

                WiFiClient * tcp = http.getStreamPtr();
                if (!tcp) {
                    DEBUG_HTTP_UPDATE("[httpUpdate] WiFiClient connection unexpectedly absent\n");
                    _setLastError(HTTPC_ERROR_CONNECTION_LOST);
                    http.end();
                    return HTTP_UPDATE_FAILED;
                }

                if (_closeConnectionsOnUpdate) {
                    WiFiUDP::stopAll();
                    WiFiClient::stopAllExcept(tcp);
                }

                delay(100);

                int command;

                if (spiffs) {
                    command = U_FS;
                    DEBUG_HTTP_UPDATE("[httpUpdate] runUpdate filesystem...\n");
                } else {
                    command = U_FLASH;
                    DEBUG_HTTP_UPDATE("[httpUpdate] runUpdate flash...\n");
                }

                if (runUpdate(*tcp, len, md5, command)) {
                    ret = HTTP_UPDATE_OK;
                    DEBUG_HTTP_UPDATE("[httpUpdate] Update ok\n");
                    http.end();
                    // Warn main app we're all done
                    if (_cbEnd) {
                        _cbEnd();
                    }

#ifdef ATOMIC_FS_UPDATE
                    if (_rebootOnUpdate) {
#else
                    if (_rebootOnUpdate && !spiffs) {
#endif
                        rp2040.restart();
                    }

                } else {
                    ret = HTTP_UPDATE_FAILED;
                    DEBUG_HTTP_UPDATE("[httpUpdate] Update failed\n");
                }
            }
        } else {
            _setLastError(HTTP_UE_SERVER_NOT_REPORT_SIZE);
            ret = HTTP_UPDATE_FAILED;
            DEBUG_HTTP_UPDATE("[httpUpdate] Content-Length was 0 or wasn't set by Server?!\n");
        }
        break;
    case HTTP_CODE_NOT_MODIFIED:
        ///< Not Modified (No updates)
        ret = HTTP_UPDATE_NO_UPDATES;
        break;
    case HTTP_CODE_NOT_FOUND:
        _setLastError(HTTP_UE_SERVER_FILE_NOT_FOUND);
        ret = HTTP_UPDATE_FAILED;
        break;
    case HTTP_CODE_FORBIDDEN:
        _setLastError(HTTP_UE_SERVER_FORBIDDEN);
        ret = HTTP_UPDATE_FAILED;
        break;
    case HTTP_CODE_UNAUTHORIZED:
        _setLastError(HTTP_UE_SERVER_UNAUTHORIZED);
        ret = HTTP_UPDATE_FAILED;
        break;
    default:
        _setLastError(HTTP_UE_SERVER_WRONG_HTTP_CODE);
        ret = HTTP_UPDATE_FAILED;
        DEBUG_HTTP_UPDATE("[httpUpdate] HTTP Code is (%d)\n", code);
        //http.writeToStream(&Serial1);
        break;
    }

    http.end();
    return ret;
}

/**
    write Update to flash
    @param in Stream&
    @param size uint32_t
    @param md5 String
    @return true if Update ok
*/
bool HTTPUpdate::runUpdate(Stream& in, uint32_t size, const String& md5, int command) {

    StreamString error;

    if (_cbProgress) {
        Update.onProgress(_cbProgress);
    }

    if (!Update.begin(size, command)) {
        _setLastError(Update.getError());
        Update.printError(error);
        error.trim(); // remove line ending
        DEBUG_HTTP_UPDATE("[httpUpdate] Update.begin failed! (%s)\n", error.c_str());
        return false;
    }

    if (_cbProgress) {
        _cbProgress(0, size);
    }

    if (md5.length()) {
        if (!Update.setMD5(md5.c_str())) {
            _setLastError(HTTP_UE_SERVER_FAULTY_MD5);
            DEBUG_HTTP_UPDATE("[httpUpdate] Update.setMD5 failed! (%s)\n", md5.c_str());
            return false;
        }
    }

    if (Update.writeStream(in) != size) {
        _setLastError(Update.getError());
        Update.printError(error);
        error.trim(); // remove line ending
        DEBUG_HTTP_UPDATE("[httpUpdate] Update.writeStream failed! (%s)\n", error.c_str());
        return false;
    }

    if (_cbProgress) {
        _cbProgress(size, size);
    }

    if (!Update.end()) {
        _setLastError(Update.getError());
        Update.printError(error);
        error.trim(); // remove line ending
        DEBUG_HTTP_UPDATE("[httpUpdate] Update.end failed! (%s)\n", error.c_str());
        return false;
    }

    return true;
}

#if !defined(NO_GLOBAL_INSTANCES) && !defined(NO_GLOBAL_HTTPUPDATE)
HTTPUpdate httpUpdate;
#endif
