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
