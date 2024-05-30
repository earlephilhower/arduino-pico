/*
    Parsing.cpp - HTTP request parsing.

    Copyright (c) 2015 Ivan Grokhotkov. All rights reserved.

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
    Modified 8 May 2015 by Hristo Gochkov (proper post and file upload handling)
*/

#include <Arduino.h>
#include "WiFiServer.h"
#include "WiFiClient.h"
#include "HTTPServer.h"
#include "detail/mimetable.h"

#ifndef log_e
#define log_e(...)
#define log_w(...)
#define log_v(...)
#endif

#ifndef WEBSERVER_MAX_POST_ARGS
#define WEBSERVER_MAX_POST_ARGS 32
#endif

#define __STR(a) #a
#define _STR(a) __STR(a)
static const char * _http_method_str[] = {
#define XX(num, name, string) _STR(name),
    HTTP_METHOD_MAP(XX)
#undef XX
};

static const char Content_Type[] PROGMEM = "Content-Type";
static const char filename[] PROGMEM = "filename";

static char* readBytesWithTimeout(WiFiClient* client, size_t maxLength, size_t& dataLength, int timeout_ms) {
    char *buf = nullptr;
    dataLength = 0;
    while (dataLength < maxLength) {
        int tries = timeout_ms;
        size_t newLength;
        while (!(newLength = client->available()) && tries--) {
            delay(1);
        }
        if (!newLength) {
            break;
        }
        if (!buf) {
            buf = (char *) malloc(newLength + 1);
            if (!buf) {
                return nullptr;
            }
        } else {
            char* newBuf = (char *) realloc(buf, dataLength + newLength + 1);
            if (!newBuf) {
                free(buf);
                return nullptr;
            }
            buf = newBuf;
        }
        client->readBytes(buf + dataLength, newLength);
        dataLength += newLength;
        buf[dataLength] = '\0';
    }
    return buf;
}

HTTPServer::ClientFuture HTTPServer::_parseRequest(WiFiClient* client) {
    // Read the first line of HTTP request
    String req = client->readStringUntil('\r');
    client->readStringUntil('\n');
    //reset header value
    for (int i = 0; i < _headerKeysCount; ++i) {
        _currentHeaders[i].value = String();
    }

    // First line of HTTP request looks like "GET /path HTTP/1.1"
    // Retrieve the "/path" part by finding the spaces
    int addr_start = req.indexOf(' ');
    int addr_end = req.indexOf(' ', addr_start + 1);
    if (addr_start == -1 || addr_end == -1) {
        log_e("Invalid request: %s", req.c_str());
        return CLIENT_MUST_STOP;
    }

    String methodStr = req.substring(0, addr_start);
    String url = req.substring(addr_start + 1, addr_end);
    String versionEnd = req.substring(addr_end + 8);
    _currentVersion = atoi(versionEnd.c_str());
    String searchStr = "";
    int hasSearch = url.indexOf('?');
    if (hasSearch != -1) {
        searchStr = url.substring(hasSearch + 1);
        url = url.substring(0, hasSearch);
    }
    _currentUri = url;
    _chunked = false;
    _clientContentLength = 0;  // not known yet, or invalid

    if (_hook) {
        auto whatNow = _hook(methodStr, url, client, mime::getContentType);
        if (whatNow != CLIENT_REQUEST_CAN_CONTINUE) {
            return whatNow;
        }
    }

    HTTPMethod method = HTTP_ANY;
    size_t num_methods = sizeof(_http_method_str) / sizeof(const char *);
    for (size_t i = 0; i < num_methods; i++) {
        if (methodStr == _http_method_str[i]) {
            method = (HTTPMethod)i;
            break;
        }
    }
    if (method == HTTP_ANY) {
        log_e("Unknown HTTP Method: %s", methodStr.c_str());
        return CLIENT_MUST_STOP;
    }
    _currentMethod = method;

    log_v("method: %s url: %s search: %s", methodStr.c_str(), url.c_str(), searchStr.c_str());

    //attach handler
    RequestHandler* handler;
    for (handler = _firstHandler; handler; handler = handler->next()) {
        if (handler->canHandle(_currentMethod, _currentUri)) {
            break;
        }
    }
    _currentHandler = handler;

    String formData;
    // below is needed only when POST type request
    if (method == HTTP_POST || method == HTTP_PUT || method == HTTP_PATCH || method == HTTP_DELETE) {
        String boundaryStr;
        String headerName;
        String headerValue;
        bool isForm = false;
        bool isEncoded = false;
        //parse headers
        while (1) {
            req = client->readStringUntil('\r');
            client->readStringUntil('\n');
            if (req == "") {
                break;    //no moar headers
            }
            int headerDiv = req.indexOf(':');
            if (headerDiv == -1) {
                break;
            }
            headerName = req.substring(0, headerDiv);
            headerValue = req.substring(headerDiv + 1);
            headerValue.trim();
            _collectHeader(headerName.c_str(), headerValue.c_str());

            log_v("headerName: %s", headerName.c_str());
            log_v("headerValue: %s", headerValue.c_str());

            if (headerName.equalsIgnoreCase(FPSTR(Content_Type))) {
                using namespace mime;
                if (headerValue.startsWith(FPSTR(mimeTable[txt].mimeType))) {
                    isForm = false;
                } else if (headerValue.startsWith(F("application/x-www-form-urlencoded"))) {
                    isForm = false;
                    isEncoded = true;
                } else if (headerValue.startsWith(F("multipart/"))) {
                    boundaryStr = headerValue.substring(headerValue.indexOf('=') + 1);
                    boundaryStr.replace("\"", "");
                    isForm = true;
                }
            } else if (headerName.equalsIgnoreCase(F("Content-Length"))) {
                _clientContentLength = headerValue.toInt();
            } else if (headerName.equalsIgnoreCase(F("Host"))) {
                _hostHeader = headerValue;
            }
        }

        if (!isForm) {
            size_t plainLength;
            char* plainBuf = readBytesWithTimeout(client, _clientContentLength, plainLength, HTTP_MAX_POST_WAIT);
            if ((int)plainLength < (int)_clientContentLength) {
                free(plainBuf);
                return CLIENT_MUST_STOP;
            }
            if (_clientContentLength > 0) {
                if (isEncoded) {
                    //url encoded form
                    if (searchStr != "") {
                        searchStr += '&';
                    }
                    searchStr += plainBuf;
                }
                _parseArguments(searchStr);
                if (!isEncoded) {
                    //plain post json or other data
                    RequestArgument& arg = _currentArgs[_currentArgCount++];
                    arg.key = F("plain");
                    arg.value = String(plainBuf);
                }

                log_v("Plain: %s", plainBuf);
                free(plainBuf);
            } else {
                // No content - but we can still have arguments in the URL.
                _parseArguments(searchStr);
            }
        } else {
            // it IS a form
            _parseArguments(searchStr);
            if (!_parseForm(client, boundaryStr, _clientContentLength)) {
                return CLIENT_MUST_STOP;
            }
        }
    } else {
        String headerName;
        String headerValue;
        //parse headers
        while (1) {
            req = client->readStringUntil('\r');
            client->readStringUntil('\n');
            if (req == "") {
                break;    //no moar headers
            }
            int headerDiv = req.indexOf(':');
            if (headerDiv == -1) {
                break;
            }
            headerName = req.substring(0, headerDiv);
            headerValue = req.substring(headerDiv + 2);
            _collectHeader(headerName.c_str(), headerValue.c_str());

            log_v("headerName: %s", headerName.c_str());
            log_v("headerValue: %s", headerValue.c_str());

            if (headerName.equalsIgnoreCase("Host")) {
                _hostHeader = headerValue;
            }
        }
        _parseArguments(searchStr);
    }
    client->flush();

    log_v("Request: %s", url.c_str());
    log_v(" Arguments: %s", searchStr.c_str());

    return CLIENT_REQUEST_CAN_CONTINUE;
}

bool HTTPServer::_collectHeader(const char* headerName, const char* headerValue) {
    for (int i = 0; i < _headerKeysCount; i++) {
        if (_currentHeaders[i].key.equalsIgnoreCase(headerName)) {
            _currentHeaders[i].value = headerValue;
            return true;
        }
    }
    return false;
}

void HTTPServer::_parseArguments(String data) {
    log_v("args: %s", data.c_str());
    if (_currentArgs) {
        delete[] _currentArgs;
    }
    _currentArgs = 0;
    if (data.length() == 0) {
        _currentArgCount = 0;
        _currentArgs = new RequestArgument[1];
        return;
    }
    _currentArgCount = 1;

    for (int i = 0; i < (int)data.length();) {
        i = data.indexOf('&', i);
        if (i == -1) {
            break;
        }
        ++i;
        ++_currentArgCount;
    }
    log_v("args count: %d", _currentArgCount);

    _currentArgs = new RequestArgument[_currentArgCount + 1];
    int pos = 0;
    int iarg;
    for (iarg = 0; iarg < _currentArgCount;) {
        int equal_sign_index = data.indexOf('=', pos);
        int next_arg_index = data.indexOf('&', pos);
        log_v("pos %d =@%d &@%d", pos, equal_sign_index, next_arg_index);
        if ((equal_sign_index == -1) || ((equal_sign_index > next_arg_index) && (next_arg_index != -1))) {
            log_e("arg missing value: %d", iarg);
            if (next_arg_index == -1) {
                break;
            }
            pos = next_arg_index + 1;
            continue;
        }
        RequestArgument& arg = _currentArgs[iarg];
        arg.key = urlDecode(data.substring(pos, equal_sign_index));
        arg.value = urlDecode(data.substring(equal_sign_index + 1, next_arg_index));
        log_v("arg %d key: %s value: %s", iarg, arg.key.c_str(), arg.value.c_str());
        ++iarg;
        if (next_arg_index == -1) {
            break;
        }
        pos = next_arg_index + 1;
    }
    _currentArgCount = iarg;
    log_v("args count: %d", _currentArgCount);

}

void HTTPServer::_uploadWriteByte(uint8_t b) {
    if (_currentUpload->currentSize == HTTP_UPLOAD_BUFLEN) {
        if (_currentHandler && _currentHandler->canUpload(_currentUri)) {
            _currentHandler->upload(*this, _currentUri, *_currentUpload);
        }
        _currentUpload->totalSize += _currentUpload->currentSize;
        _currentUpload->currentSize = 0;
    }
    _currentUpload->buf[_currentUpload->currentSize++] = b;
}

int HTTPServer::_uploadReadByte(WiFiClient* client) {
    int res = client->read();
    if (res < 0) {
        // keep trying until you either read a valid byte or timeout
        unsigned long startMillis = millis();
        unsigned long timeoutIntervalMillis = client->getTimeout();
        bool timedOut = false;
        for (;;) {
            if (!client->connected()) {
                return -1;
            }
            // loosely modeled after blinkWithoutDelay pattern
            while (!timedOut && !client->available() && client->connected()) {
                delay(2);
                timedOut = millis() - startMillis >= timeoutIntervalMillis;
            }

            res = client->read();
            if (res >= 0) {
                return res; // exit on a valid read
            }
            // NOTE: it is possible to get here and have all of the following
            //       assertions hold true
            //
            //       -- client->available() > 0
            //       -- client->connected == true
            //       -- res == -1
            //
            //       a simple retry strategy overcomes this which is to say the
            //       assertion is not permanent, but the reason that this works
            //       is elusive, and possibly indicative of a more subtle underlying
            //       issue

            timedOut = millis() - startMillis >= timeoutIntervalMillis;
            if (timedOut) {
                return res; // exit on a timeout
            }
        }
    }

    return res;
}

bool HTTPServer::_parseForm(WiFiClient* client, String boundary, uint32_t len) {
    (void) len;
    log_v("Parse Form: Boundary: %s Length: %d", boundary.c_str(), len);
    String line;
    int retry = 0;
    do {
        line = client->readStringUntil('\r');
        ++retry;
    } while (line.length() == 0 && retry < 3);

    client->readStringUntil('\n');
    //start reading the form
    if (line == ("--" + boundary)) {
        if (_postArgs) {
            delete[] _postArgs;
        }
        _postArgs = new RequestArgument[WEBSERVER_MAX_POST_ARGS];
        _postArgsLen = 0;
        while (1) {
            String argName;
            String argValue;
            String argType;
            String argFilename;
            bool argIsFile = false;

            line = client->readStringUntil('\r');
            client->readStringUntil('\n');
            if (line.length() > 19 && line.substring(0, 19).equalsIgnoreCase(F("Content-Disposition"))) {
                int nameStart = line.indexOf('=');
                if (nameStart != -1) {
                    argName = line.substring(nameStart + 2);
                    nameStart = argName.indexOf('=');
                    if (nameStart == -1) {
                        argName = argName.substring(0, argName.length() - 1);
                    } else {
                        argFilename = argName.substring(nameStart + 2, argName.length() - 1);
                        argName = argName.substring(0, argName.indexOf('"'));
                        argIsFile = true;
                        log_v("PostArg FileName: %s", argFilename.c_str());
                        //use GET to set the filename if uploading using blob
                        if (argFilename == F("blob") && hasArg(FPSTR(filename))) {
                            argFilename = arg(FPSTR(filename));
                        }
                    }
                    log_v("PostArg Name: %s", argName.c_str());
                    using namespace mime;
                    argType = FPSTR(mimeTable[txt].mimeType);
                    line = client->readStringUntil('\r');
                    client->readStringUntil('\n');
                    if (line.length() > 12 && line.substring(0, 12).equalsIgnoreCase(FPSTR(Content_Type))) {
                        argType = line.substring(line.indexOf(':') + 2);
                        //skip next line
                        client->readStringUntil('\r');
                        client->readStringUntil('\n');
                    }
                    log_v("PostArg Type: %s", argType.c_str());
                    if (!argIsFile) {
                        while (1) {
                            line = client->readStringUntil('\r');
                            client->readStringUntil('\n');
                            if (line.startsWith("--" + boundary)) {
                                break;
                            }
                            if (argValue.length() > 0) {
                                argValue += "\n";
                            }
                            argValue += line;
                        }
                        log_v("PostArg Value: %s", argValue.c_str());

                        RequestArgument& arg = _postArgs[_postArgsLen++];
                        arg.key = argName;
                        arg.value = argValue;

                        if (line == ("--" + boundary + "--")) {
                            log_v("Done Parsing POST");
                            break;
                        } else if (_postArgsLen >= WEBSERVER_MAX_POST_ARGS) {
                            log_e("Too many PostArgs (max: %d) in request.", WEBSERVER_MAX_POST_ARGS);
                            return false;
                        }
                    } else {
                        _currentUpload.reset(new HTTPUpload());
                        _currentUpload->status = UPLOAD_FILE_START;
                        _currentUpload->name = argName;
                        _currentUpload->filename = argFilename;
                        _currentUpload->type = argType;
                        _currentUpload->totalSize = 0;
                        _currentUpload->currentSize = 0;
                        log_v("Start File: %s Type: %s", _currentUpload->filename.c_str(), _currentUpload->type.c_str());
                        if (_currentHandler && _currentHandler->canUpload(_currentUri)) {
                            _currentHandler->upload(*this, _currentUri, *_currentUpload);
                        }
                        _currentUpload->status = UPLOAD_FILE_WRITE;
                        int argByte = _uploadReadByte(client);
readfile:

                        while (argByte != 0x0D) {
                            if (argByte < 0) {
                                return _parseFormUploadAborted();
                            }
                            _uploadWriteByte(argByte);
                            argByte = _uploadReadByte(client);
                        }

                        argByte = _uploadReadByte(client);
                        if (argByte < 0) {
                            return _parseFormUploadAborted();
                        }
                        if (argByte == 0x0A) {
                            argByte = _uploadReadByte(client);
                            if (argByte < 0) {
                                return _parseFormUploadAborted();
                            }
                            if ((char)argByte != '-') {
                                //continue reading the file
                                _uploadWriteByte(0x0D);
                                _uploadWriteByte(0x0A);
                                goto readfile;
                            } else {
                                argByte = _uploadReadByte(client);
                                if (argByte < 0) {
                                    return _parseFormUploadAborted();
                                }
                                if ((char)argByte != '-') {
                                    //continue reading the file
                                    _uploadWriteByte(0x0D);
                                    _uploadWriteByte(0x0A);
                                    _uploadWriteByte((uint8_t)('-'));
                                    goto readfile;
                                }
                            }

                            uint8_t endBuf[boundary.length()];
                            uint32_t i = 0;
                            while (i < boundary.length()) {
                                argByte = _uploadReadByte(client);
                                if (argByte < 0) {
                                    return _parseFormUploadAborted();
                                }
                                if ((char)argByte == 0x0D) {
                                    _uploadWriteByte(0x0D);
                                    _uploadWriteByte(0x0A);
                                    _uploadWriteByte((uint8_t)('-'));
                                    _uploadWriteByte((uint8_t)('-'));
                                    for (uint32_t j = 0; j < i; j++) {
                                        _uploadWriteByte(endBuf[j]);
                                    }
                                    goto readfile;
                                }
                                endBuf[i++] = (uint8_t)argByte;
                            }

                            if (strstr((const char*)endBuf, boundary.c_str()) != nullptr) {
                                if (_currentHandler && _currentHandler->canUpload(_currentUri)) {
                                    _currentHandler->upload(*this, _currentUri, *_currentUpload);
                                }
                                _currentUpload->totalSize += _currentUpload->currentSize;
                                _currentUpload->status = UPLOAD_FILE_END;
                                if (_currentHandler && _currentHandler->canUpload(_currentUri)) {
                                    _currentHandler->upload(*this, _currentUri, *_currentUpload);
                                }
                                log_v("End File: %s Type: %s Size: %d", _currentUpload->filename.c_str(), _currentUpload->type.c_str(), _currentUpload->totalSize);
                                line = client->readStringUntil(0x0D);
                                client->readStringUntil(0x0A);
                                if (line == "--") {
                                    log_v("Done Parsing POST");
                                    break;
                                }
                                continue;
                            } else {
                                _uploadWriteByte(0x0D);
                                _uploadWriteByte(0x0A);
                                _uploadWriteByte((uint8_t)('-'));
                                _uploadWriteByte((uint8_t)('-'));
                                for (uint32_t j = 0; j < boundary.length(); j++) {
                                    _uploadWriteByte(endBuf[j]);
                                }
                                argByte = _uploadReadByte(client);
                                goto readfile;
                            }
                        } else {
                            _uploadWriteByte(0x0D);
                            goto readfile;
                        }
                        break;
                    }
                }
            }
        }

        int iarg;
        int totalArgs = ((WEBSERVER_MAX_POST_ARGS - _postArgsLen) < _currentArgCount) ? (WEBSERVER_MAX_POST_ARGS - _postArgsLen) : _currentArgCount;
        for (iarg = 0; iarg < totalArgs; iarg++) {
            RequestArgument& arg = _postArgs[_postArgsLen++];
            arg.key = _currentArgs[iarg].key;
            arg.value = _currentArgs[iarg].value;
        }
        if (_currentArgs) {
            delete[] _currentArgs;
        }
        _currentArgs = new RequestArgument[_postArgsLen];
        for (iarg = 0; iarg < _postArgsLen; iarg++) {
            RequestArgument& arg = _currentArgs[iarg];
            arg.key = _postArgs[iarg].key;
            arg.value = _postArgs[iarg].value;
        }
        _currentArgCount = iarg;
        if (_postArgs) {
            delete[] _postArgs;
            _postArgs = nullptr;
            _postArgsLen = 0;
        }
        return true;
    }
    log_e("Error: line: %s", line.c_str());
    return false;
}

String HTTPServer::urlDecode(const String& text) {
    String decoded = "";
    char temp[] = "0x00";
    unsigned int len = text.length();
    unsigned int i = 0;
    while (i < len) {
        char decodedChar;
        char encodedChar = text.charAt(i++);
        if ((encodedChar == '%') && (i + 1 < len)) {
            temp[2] = text.charAt(i++);
            temp[3] = text.charAt(i++);

            decodedChar = strtol(temp, nullptr, 16);
        } else {
            if (encodedChar == '+') {
                decodedChar = ' ';
            } else {
                decodedChar = encodedChar;  // normal ascii char
            }
        }
        decoded += decodedChar;
    }
    return decoded;
}

bool HTTPServer::_parseFormUploadAborted() {
    _currentUpload->status = UPLOAD_FILE_ABORTED;
    if (_currentHandler && _currentHandler->canUpload(_currentUri)) {
        _currentHandler->upload(*this, _currentUri, *_currentUpload);
    }
    return false;
}
