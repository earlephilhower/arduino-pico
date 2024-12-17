/*
    WebServerTemplate - Makes an actual Server class from a HTTPServer
    Copyright (c) 2022 Earle F. Philhower, III All rights reserved.

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

#pragma once

#include "HTTPServer.h"

template<typename ServerType, int DefaultPort = 80>
class WebServerTemplate;

template<typename ServerType, int DefaultPort>
class WebServerTemplate : public HTTPServer {
public:
    using WebServerType = WebServerTemplate<ServerType>;
    using ClientType = typename ServerType::ClientType;

    WebServerTemplate(IPAddress addr, int port = DefaultPort);
    WebServerTemplate(int port = DefaultPort);
    virtual ~WebServerTemplate();

    virtual void begin();
    virtual void begin(uint16_t port);
    virtual void handleClient();

    virtual void close();
    virtual void stop();

    ServerType &getServer() {
        return _server;
    }

    ClientType& client() {
        // _currentClient is always a WiFiClient*, so we need to coerce to the proper type for SSL
        return *(ClientType*)_currentClient;
    }

private:
    ServerType _server;
};

template <typename ServerType, int DefaultPort>
WebServerTemplate<ServerType, DefaultPort>::WebServerTemplate(IPAddress addr, int port) : HTTPServer(), _server(addr, port) {
}

template <typename ServerType, int DefaultPort>
WebServerTemplate<ServerType, DefaultPort>::WebServerTemplate(int port) : HTTPServer(), _server(port) {
}

template <typename ServerType, int DefaultPort>
WebServerTemplate<ServerType, DefaultPort>::~WebServerTemplate() {
    _server.close();
}

template <typename ServerType, int DefaultPort>
void WebServerTemplate<ServerType, DefaultPort>::begin() {
    close();
    _server.begin();
    _server.setNoDelay(true);
}

template <typename ServerType, int DefaultPort>
void WebServerTemplate<ServerType, DefaultPort>::begin(uint16_t port) {
    close();
    _server.begin(port);
    _server.setNoDelay(true);
}

template <typename ServerType, int DefaultPort>
void WebServerTemplate<ServerType, DefaultPort>::handleClient() {
    if (_currentStatus == HC_NONE) {
        if (_currentClient) {
            delete _currentClient;
            _currentClient = nullptr;
        }

        _currentClient = new ClientType(_server.accept());
        if (!_currentClient) {
            if (_nullDelay) {
                delay(1);
            }
            return;
        }
        _currentStatus = HC_WAIT_READ;
        _statusChange = millis();
    }
    bool keepCurrentClient = false;
    bool callYield = false;

    if (_currentClient->connected()) {
        switch (_currentStatus) {
        case HC_NONE:
            // No-op to avoid C++ compiler warning
            break;
        case HC_WAIT_READ:
            // Wait for data from client to become available
            if (_currentClient->available()) {
                _currentClient->setTimeout(HTTP_MAX_SEND_WAIT);
                switch (_parseRequest(_currentClient)) {
                case CLIENT_REQUEST_CAN_CONTINUE:
                    _contentLength = CONTENT_LENGTH_NOT_SET;
                    _handleRequest();
                /* fallthrough */
                case CLIENT_REQUEST_IS_HANDLED:
                    if (_currentClient->connected() || _currentClient->available()) {
                        _currentStatus = HC_WAIT_CLOSE;
                        _statusChange = millis();
                        keepCurrentClient = true;
                    } else {
                        //log_v("webserver: peer has closed after served\n");
                    }
                    break;
                case CLIENT_MUST_STOP:
                    //log_v("Close client\n");
                    _currentClient->stop();
                    break;
                case CLIENT_IS_GIVEN:
                    // client must not be stopped but must not be handled here anymore
                    // (example: tcp connection given to websocket)
                    //log_v("Give client\n");
                    break;
                } // switch _parseRequest()
            } else {
                // !_currentClient.available(): waiting for more data
                unsigned long timeSinceChange = millis() - _statusChange;
                // Use faster connection drop timeout if any other client has data
                // or the buffer of pending clients is full
                if ((_server.hasClientData() || _server.hasMaxPendingClients())
                        && timeSinceChange > HTTP_MAX_DATA_AVAILABLE_WAIT) {
                    //log_v("webserver: closing since there's another connection to read from\r\n");
                } else {
                    if (timeSinceChange > HTTP_MAX_DATA_WAIT) {
                        //log_v("webserver: closing after read timeout\r\n");
                    } else {
                        keepCurrentClient = true;
                    }
                }
                callYield = true;
            }
            break;
        case HC_WAIT_CLOSE:
            // Wait for client to close the connection
            if (millis() - _statusChange <= HTTP_MAX_CLOSE_WAIT) {
                keepCurrentClient = true;
                callYield = true;
            }
        }
    }

    if (!keepCurrentClient) {
        if (_currentClient) {
            delete _currentClient;
            _currentClient = nullptr;
        }
        _currentStatus = HC_NONE;
        _currentUpload.reset();
        _currentRaw.reset();
    }

    if (callYield) {
        yield();
    }

}

template <typename ServerType, int DefaultPort>
void WebServerTemplate<ServerType, DefaultPort>::close() {
    _server.close();
    httpClose();
}

template <typename ServerType, int DefaultPort>
void WebServerTemplate<ServerType, DefaultPort>::stop() {
    close();
}
