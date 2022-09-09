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

        ClientType client = _server.available();
        if (!client) {
            if (_nullDelay) {
                delay(1);
            }
            return;
        }

        _currentClient = new ClientType(client);
        _currentStatus = HC_WAIT_READ;
        _statusChange = millis();
    }
    httpHandleClient();
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
