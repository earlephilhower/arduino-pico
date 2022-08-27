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

#include <WebServer.h>

template <typename ServerType, int ServerPort>
class HTTPUpdateServerTemplate {
public:
    HTTPUpdateServerTemplate(bool serial_debug = false);

    void setup(WebServerTemplate<ServerType, ServerPort> *server) {
        setup(server, "", "");
    }

    void setup(WebServerTemplate<ServerType, ServerPort> *server, const String& path) {
        setup(server, path, "", "");
    }

    void setup(WebServerTemplate<ServerType, ServerPort> *server, const String& username, const String& password) {
        setup(server, "/update", username, password);
    }

    void setup(WebServerTemplate<ServerType, ServerPort> *server, const String& path, const String& username, const String& password);

    void updateCredentials(const String& username, const String& password) {
        _username = username;
        _password = password;
    }

protected:
    void _setUpdaterError();

private:
    bool _serial_output;
    WebServerTemplate<ServerType, ServerPort> *_server;
    String _username;
    String _password;
    bool _authenticated;
    String _updaterError;
};

#include "HTTPUpdateServer-impl.h"

using HTTPUpdateServer = HTTPUpdateServerTemplate<WiFiServer, 80>;
using HTTPUpdateServerSecure = HTTPUpdateServerTemplate<WiFiServerSecure, 443>;
