/**
    HTTPClient.cpp

    Created on: 02.11.2015

    Copyright (c) 2015 Markus Sattler. All rights reserved.
    This file is part of the ESP8266HTTPClient for Arduino.

    Modified 2022 by Earle F. Philhower, III for the Pico RP2040

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
#include <Arduino.h>
#include "HTTPClient.h"
#include <WiFi.h>
#include <time.h>
#include "base64.h"
extern "C" char *strptime(const char *__restrict, const char *__restrict, struct tm *__restrict);  // Not exposed by headers?


// per https://github.com/esp8266/Arduino/issues/8231
// make sure HTTPClient can be utilized as a movable class member
static_assert(std::is_default_constructible_v<HTTPClient>, "");
static_assert(!std::is_copy_constructible_v<HTTPClient>, "");
static_assert(std::is_move_constructible_v<HTTPClient>, "");
static_assert(std::is_move_assignable_v<HTTPClient>, "");

static const char defaultUserAgentPstr[] PROGMEM = "Pico";
const String HTTPClient::defaultUserAgent = defaultUserAgentPstr;

//static int StreamReportToHttpClientReport (Stream::Report streamSendError)
//{
//    switch (streamSendError)
//    {
//    case Stream::Report::TimedOut: return HTTPC_ERROR_READ_TIMEOUT;
//    case Stream::Report::ReadError: return HTTPC_ERROR_NO_STREAM;
//    case Stream::Report::WriteError: return HTTPC_ERROR_STREAM_WRITE;
//    case Stream::Report::ShortOperation: return HTTPC_ERROR_STREAM_WRITE;
//    case Stream::Report::Success: return 0;
//    }
//    return 0; // never reached, keep gcc quiet
//}

// Wrappers for ESP8266-specific Arduino API changes
static size_t StreamSendSize(Stream *s, Print *c, int size) {
    int sent = 0;
    if (size < 0) {
        size = 999999; // Transfer until read fails
    }
    uint32_t start = millis();
    while ((sent < size) && (millis() - start < 5000)) {
        int x = s->read();
        if (x < 0) {
            break;
        } else if (c->write(x)) {
            sent++;
        } else {
            break;
        }
    }
    return sent;
}

class StreamConstPtr {
public:
    StreamConstPtr(const uint8_t *payload, size_t size) {
        _payload = payload;
        _size = size;
    }
    StreamConstPtr(const String& string) {
        _payload = (const uint8_t *)string.c_str();
        _size = string.length();
    }
    size_t sendAll(Client *dst) {
        uint32_t start = millis();
        size_t sent = 0;
        while ((sent < _size) && (millis() - start < 5000)) {
            size_t towrite = std::min((size_t)128, _size - sent);
            auto wrote = dst->write(_payload, towrite);
            if (wrote <= 0) {
                break;
            }
            sent += wrote;
            _payload += wrote;
        }
        return sent;
    }
    const uint8_t *_payload;
    size_t _size;
};


void HTTPClient::clear() {
    _returnCode = 0;
    _size = -1;
    _headers = "";
    _location = "";
    _payload.reset();
}



/**
    parsing the url for all needed parameters
    @param client Client&
    @param url String
    @param https bool
    @return success bool
*/
bool HTTPClient::begin(String url) {
    // check for : (http: or https:)
    int index = url.indexOf(':');
    if (index < 0) {
        DEBUG_HTTPCLIENT("[HTTP-Client][begin] failed to parse protocol\n");
        return false;
    }

    String protocol = url.substring(0, index);
    protocol.toLowerCase();
    if (protocol != "http" && protocol != "https") {
        DEBUG_HTTPCLIENT("[HTTP-Client][begin] unknown protocol '%s'\n", protocol.c_str());
        return false;
    }

    _port = (protocol == "https" ? 443 : 80);
    if (!_client()) {
        if (protocol == "https") {
            _tls();
        } else {
            _clientMade = new WiFiClient();
            _clientGiven = false;
        }
    }

    return beginInternal(url, protocol.c_str());
}


/**
    directly supply all needed parameters
    @param client Client&
    @param host String
    @param port uint16_t
    @param uri String
    @param https bool
    @return success bool
*/
bool HTTPClient::begin(String host, uint16_t port, String uri, bool https) {
    // Disconnect when reusing HTTPClient to talk to a different host
    if (_host != host) {
        _canReuse = false;
        disconnect(true);
    }

    clear();

    _host = host;
    _port = port;
    _uri = uri;
    _protocol = (https ? "https" : "http");
    if (!_client()) {
        if (https) {
            _tls();
        } else {
            _clientMade = new WiFiClient();
            _clientGiven = false;
        }
    }
    return true;
}



/**
    parsing the url for all needed parameters
    @param client Client&
    @param url String
    @param https bool
    @return success bool
*/
bool HTTPClient::begin(WiFiClient &client, const String& url) {
    // check for : (http: or https:)
    int index = url.indexOf(':');
    if (index < 0) {
        DEBUG_HTTPCLIENT("[HTTP-Client][begin] failed to parse protocol\n");
        return false;
    }

    String protocol = url.substring(0, index);
    protocol.toLowerCase();
    if (protocol != "http" && protocol != "https") {
        DEBUG_HTTPCLIENT("[HTTP-Client][begin] unknown protocol '%s'\n", protocol.c_str());
        return false;
    }

    _port = (protocol == "https" ? 443 : 80);
    _secure = (protocol == "https");
    _clientIn = client.clone();
    _clientGiven = true;
    if (_clientMade) {
        delete _clientMade;
        _clientMade = nullptr;
    }


    return beginInternal(url, protocol.c_str());
}


/**
    directly supply all needed parameters
    @param client Client&
    @param host String
    @param port uint16_t
    @param uri String
    @param https bool
    @return success bool
*/
bool HTTPClient::begin(WiFiClient &client, const String& host, uint16_t port, const String& uri, bool https) {
    // Disconnect when reusing HTTPClient to talk to a different host
    if ((_host != "") && (_host != host)) {
        _canReuse = false;
        disconnect(true);
    }

    _clientIn = client.clone();
    _clientGiven = true;
    if (_clientMade) {
        delete _clientMade;
        _clientMade = nullptr;
    }

    clear();

    _host = host;
    _port = port;
    _uri = uri;
    _protocol = (https ? "https" : "http");
    _secure = https;
    return true;
}





bool HTTPClient::beginInternal(const String& __url, const char* expectedProtocol) {
    String url(__url);

    DEBUG_HTTPCLIENT("[HTTP-Client][begin] url: %s\n", url.c_str());
    clear();

    // check for : (http: or https:
    int index = url.indexOf(':');
    if (index < 0) {
        DEBUG_HTTPCLIENT("[HTTP-Client][begin] failed to parse protocol\n");
        return false;
    }

    _protocol = url.substring(0, index);
    _protocol.toLowerCase();
    url.remove(0, (index + 3)); // remove http:// or https://

    if (_protocol == "http") {
        // set default port for 'http'
        _port = 80;
    } else if (_protocol == "https") {
        // set default port for 'https'
        _port = 443;
    } else {
        DEBUG_HTTPCLIENT("[HTTP-Client][begin] unsupported protocol: %s\n", _protocol.c_str());
        return false;
    }

    index = url.indexOf('/');
    String host = url.substring(0, index);
    url.remove(0, index); // remove host part

    // get Authorization
    index = host.indexOf('@');
    if (index >= 0) {
        // auth info
        String auth = host.substring(0, index);
        host.remove(0, index + 1); // remove auth part including @
        _base64Authorization = base64::encode(auth, false /* doNewLines */);
    }

    const String oldHost = _host;

    // get port
    index = host.indexOf(':');
    if (index >= 0) {
        _host = host.substring(0, index); // hostname
        host.remove(0, (index + 1)); // remove hostname + :
        _port = host.toInt(); // get port
    } else {
        _host = host;
    }

    // Disconnect when reusing HTTPClient to talk to a different host
    if (oldHost != "" && _host != oldHost) {
        _canReuse = false;
        disconnect(true);
    }

    _uri = url;

    if (expectedProtocol != nullptr && _protocol != expectedProtocol) {
        DEBUG_HTTPCLIENT("[HTTP-Client][begin] unexpected protocol: %s, expected %s\n", _protocol.c_str(), expectedProtocol);
        return false;
    }
    DEBUG_HTTPCLIENT("[HTTP-Client][begin] host: %s port: %d url: %s\n", _host.c_str(), _port, _uri.c_str());
    return true;
}


/**
    end
    called after the payload is handled
*/
void HTTPClient::end(void) {
    disconnect(false);
    clear();
    if (_clientMade) {
        delete _clientMade;
        _clientMade = nullptr;
        _clientTLS = false;
    }
}

/**
    disconnect
    close the TCP socket
*/
void HTTPClient::disconnect(bool preserveClient) {
    if (connected()) {
        if (_client()->available() > 0) {
            DEBUG_HTTPCLIENT("[HTTP-Client][end] still data in buffer (%d), clean up.\n", _client()->available());
            while (_client()->available() > 0) {
                _client()->read();
            }
        }

        if (_reuse && _canReuse) {
            DEBUG_HTTPCLIENT("[HTTP-Client][end] tcp keep open for reuse\n");
        } else {
            DEBUG_HTTPCLIENT("[HTTP-Client][end] tcp stop\n");
            if (_client()) {
                _client()->stop();
                if (!preserveClient) {
                    _clientIn = nullptr;
                    if (_clientMade) {
                        delete _clientMade;
                        _clientMade = nullptr;
                    }
                    _clientGiven = false;
                }
            }
        }
    } else {
        if (!preserveClient && _client()) { // Also destroy _client if not connected()
            _clientIn = nullptr;
            if (_clientMade) {
                delete _clientMade;
                _clientMade = nullptr;
            }
            _clientGiven = false;
        }

        DEBUG_HTTPCLIENT("[HTTP-Client][end] tcp is closed\n");
    }
}

/**
    connected
    @return connected status
*/
bool HTTPClient::connected() {
    if (_client()) {
        return (_client()->connected() || (_client()->available() > 0));
    }
    return false;
}

/**
    try to reuse the connection to the server
    keep-alive
    @param reuse bool
*/
void HTTPClient::setReuse(bool reuse) {
    _reuse = reuse;
}

/**
    set User Agent
    @param userAgent const char
*/
void HTTPClient::setUserAgent(const String& userAgent) {
    _userAgent = userAgent;
}

/**
    set the Authorizatio for the http request
    @param user const char
    @param password const char
*/
void HTTPClient::setAuthorization(const char * user, const char * password) {
    if (user && password) {
        String auth = user;
        auth += ':';
        auth += password;
        _base64Authorization = base64::encode(auth, false /* doNewLines */);
    }
}

/**
    set the Authorization for the http request
    @param auth const char * base64
*/
void HTTPClient::setAuthorization(const char * auth) {
    if (auth) {
        setAuthorization(String(auth));
    }
}

/**
    set the Authorization for the http request
    @param auth String base64
*/
void HTTPClient::setAuthorization(String auth) {
    _base64Authorization = std::move(auth);
    _base64Authorization.replace(String('\n'), "");
}

/**
    set the timeout for the TCP connection
    @param timeout unsigned int
*/
void HTTPClient::setTimeout(uint16_t timeout) {
    _tcpTimeout = timeout;
    if (connected()) {
        _client()->setTimeout(timeout);
    }
}

/**
    set the URL to a new value. Handy for following redirects.
    @param url
*/
bool HTTPClient::setURL(const String& url) {
    // if the new location is only a path then only update the URI
    if (url && url[0] == '/') {
        _uri = url;
        clear();
        return true;
    }

    if (!url.startsWith(_protocol + ':')) {
        DEBUG_HTTPCLIENT("[HTTP-Client][setURL] new URL not the same protocol, expected '%s', URL: '%s'\n", _protocol.c_str(), url.c_str());
        return false;
    }
    // disconnect but preserve _client (clear _canReuse so disconnect will close the connection)
    _canReuse = false;
    disconnect(true);
    return beginInternal(url, nullptr);
}

/**
    set redirect follow mode. See `followRedirects_t` enum for available modes.
    @param follow
*/
void HTTPClient::setFollowRedirects(followRedirects_t follow) {
    _followRedirects = follow;
}

void HTTPClient::setRedirectLimit(uint16_t limit) {
    _redirectLimit = limit;
}

/**
    use HTTP1.0
    @param useHTTP10 bool
*/
void HTTPClient::useHTTP10(bool useHTTP10) {
    _useHTTP10 = useHTTP10;
    _reuse = !useHTTP10;
}

/**
    send a GET request
    @return http code
*/
int HTTPClient::GET() {
    return sendRequest("GET");
}
/**
    send a DELETE request
    @return http code
*/
int HTTPClient::DELETE() {
    return sendRequest("DELETE");
}

/**
    sends a post request to the server
    @param payload const uint8_t
    @param size size_t
    @return http code
*/
int HTTPClient::POST(const uint8_t* payload, size_t size) {
    return sendRequest("POST", payload, size);
}

int HTTPClient::POST(const String& payload) {
    return POST((uint8_t *) payload.c_str(), payload.length());
}

/**
    sends a put request to the server
    @param payload uint8_t
    @param size size_t
    @return http code
*/
int HTTPClient::PUT(const uint8_t* payload, size_t size) {
    return sendRequest("PUT", payload, size);
}

int HTTPClient::PUT(const String& payload) {
    return PUT((const uint8_t *) payload.c_str(), payload.length());
}

/**
    sends a patch request to the server
    @param payload const uint8_t
    @param size size_t
    @return http code
*/
int HTTPClient::PATCH(const uint8_t * payload, size_t size) {
    return sendRequest("PATCH", payload, size);
}

int HTTPClient::PATCH(const String& payload) {
    return PATCH((const uint8_t *) payload.c_str(), payload.length());
}

/**
    sendRequest
    @param type const char *     "GET", "POST", ....
    @param payload String        data for the message body
    @return
*/
int HTTPClient::sendRequest(const char * type, const String& payload) {
    return sendRequest(type, (const uint8_t *) payload.c_str(), payload.length());
}

/**
    sendRequest
    @param type const char *           "GET", "POST", ....
    @param payload const uint8_t *     data for the message body if null not send
    @param size size_t                 size for the message body if 0 not send
    @return -1 if no info or > 0 when Content-Length is set by server
*/
int HTTPClient::sendRequest(const char * type, const uint8_t * payload, size_t size) {
    int code;
    bool redirect = false;
    uint16_t redirectCount = 0;
    do {
        // wipe out any existing headers from previous request
        for (size_t i = 0; i < _headerKeysCount; i++) {
            if (_currentHeaders[i].value.length() > 0) {
                _currentHeaders[i].value = "";
            }
        }

        DEBUG_HTTPCLIENT("[HTTP-Client][sendRequest] type: '%s' redirCount: %d\n", type, redirectCount);

        // connect to server
        if (!connect()) {
            return returnError(HTTPC_ERROR_CONNECTION_FAILED);
        }

        addHeader(F("Content-Length"), String(payload && size > 0 ? size : 0));

        // add cookies to header, if present
        String cookie_string;
        if (generateCookieString(&cookie_string)) {
            addHeader("Cookie", cookie_string);
        }

        // send Header
        if (!sendHeader(type)) {
            return returnError(HTTPC_ERROR_SEND_HEADER_FAILED);
        }

        // transfer all of it, with send-timeout
        if (size && StreamConstPtr(payload, size).sendAll(_client()) != size) {
            return returnError(HTTPC_ERROR_SEND_PAYLOAD_FAILED);
        }

        // handle Server Response (Header)
        code = handleHeaderResponse();

        //
        // Handle redirections as stated in RFC document:
        // https://www.w3.org/Protocols/rfc2616/rfc2616-sec10.html
        //
        // Implementing HTTP_CODE_FOUND as redirection with GET method,
        // to follow most of existing user agent implementations.
        //
        redirect = false;
        if (
            _followRedirects != HTTPC_DISABLE_FOLLOW_REDIRECTS &&
            redirectCount < _redirectLimit &&
            _location.length() > 0
        ) {
            switch (code) {
            // redirecting using the same method
            case HTTP_CODE_MOVED_PERMANENTLY:
            case HTTP_CODE_TEMPORARY_REDIRECT: {
                if (
                    // allow to force redirections on other methods
                    // (the RFC require user to accept the redirection)
                    _followRedirects == HTTPC_FORCE_FOLLOW_REDIRECTS ||
                    // allow GET and HEAD methods without force
                    !strcmp(type, "GET") ||
                    !strcmp(type, "HEAD")
                ) {
                    redirectCount += 1;
                    DEBUG_HTTPCLIENT("[HTTP-Client][sendRequest] following redirect (the same method): '%s' redirCount: %d\n", _location.c_str(), redirectCount);
                    if (!setURL(_location)) {
                        DEBUG_HTTPCLIENT("[HTTP-Client][sendRequest] failed setting URL for redirection\n");
                        // no redirection
                        break;
                    }
                    // redirect using the same request method and payload, different URL
                    redirect = true;
                }
                break;
            }
            // redirecting with method dropped to GET or HEAD
            // note: it does not need `HTTPC_FORCE_FOLLOW_REDIRECTS` for any method
            case HTTP_CODE_FOUND:
            case HTTP_CODE_SEE_OTHER: {
                redirectCount += 1;
                DEBUG_HTTPCLIENT("[HTTP-Client][sendRequest] following redirect (dropped to GET/HEAD): '%s' redirCount: %d\n", _location.c_str(), redirectCount);
                if (!setURL(_location)) {
                    DEBUG_HTTPCLIENT("[HTTP-Client][sendRequest] failed setting URL for redirection\n");
                    // no redirection
                    break;
                }
                // redirect after changing method to GET/HEAD and dropping payload
                type = "GET";
                payload = nullptr;
                size = 0;
                redirect = true;
                break;
            }

            default:
                break;
            }
        }
    } while (redirect);

    // handle Server Response (Header)
    return returnError(code);
}

/**
    sendRequest
    @param type const char *     "GET", "POST", ....
    @param stream Stream *       data stream for the message body
    @param size size_t           size for the message body if 0 not Content-Length is send
    @return -1 if no info or > 0 when Content-Length is set by server
*/
int HTTPClient::sendRequest(const char * type, Stream * stream, size_t size) {

    if (!stream) {
        return returnError(HTTPC_ERROR_NO_STREAM);
    }

    // connect to server
    if (!connect()) {
        return returnError(HTTPC_ERROR_CONNECTION_FAILED);
    }

    if (size > 0) {
        addHeader(F("Content-Length"), String(size));
    }

    // add cookies to header, if present
    String cookie_string;
    if (generateCookieString(&cookie_string)) {
        addHeader("Cookie", cookie_string);
    }

    // send Header
    if (!sendHeader(type)) {
        return returnError(HTTPC_ERROR_SEND_HEADER_FAILED);
    }

    // transfer all of it, with timeout
    size_t transferred = StreamSendSize(stream, _client(), size);
    if (transferred != size) {
        DEBUG_HTTPCLIENT("[HTTP-Client][sendRequest] short write, asked for %zu but got %zu failed.\n", size, transferred);
        return returnError(HTTPC_ERROR_SEND_PAYLOAD_FAILED);
    }

    // handle Server Response (Header)
    return returnError(handleHeaderResponse());
}

/**
    size of message body / payload
    @return -1 if no info or > 0 when Content-Length is set by server
*/
int HTTPClient::getSize(void) {
    return _size;
}

/**
    Location if redirect
*/
const String& HTTPClient::getLocation(void) {
    return _location;
}

/**
    returns the stream of the tcp connection
    @return WiFiClient
*/
WiFiClient& HTTPClient::getStream(void) {
    if (connected()) {
        return *_client();
    }

    DEBUG_HTTPCLIENT("[HTTP-Client] getStream: not connected\n");
    static WiFiClient empty;
    return empty;
}

/**
    returns the stream of the tcp connection
    @return WiFiClient
*/
WiFiClient* HTTPClient::getStreamPtr(void) {
    if (connected()) {
        return _client();
    }

    DEBUG_HTTPCLIENT("[HTTP-Client] getStreamPtr: not connected\n");
    return nullptr;
}

/**
    write all  message body / payload to Stream
    @param stream Stream
    @return bytes written ( negative values are error codes )
*/
int HTTPClient::writeToStream(Stream * stream) {
    return writeToPrint(stream);
}

/**
    write all  message body / payload to Print
    @param print Print
    @return bytes written ( negative values are error codes )
*/
int HTTPClient::writeToPrint(Print * print) {

    if (!print) {
        return returnError(HTTPC_ERROR_NO_STREAM);
    }

    // Only return error if not connected and no data available, because otherwise ::getString() will return an error instead of an empty
    // string when the server returned a http code 204 (no content)
    if (!connected() && _transferEncoding != HTTPC_TE_IDENTITY && _size > 0) {
        return returnError(HTTPC_ERROR_NOT_CONNECTED);
    }

    // get length of document (is -1 when Server sends no Content-Length header)
    int len = _size;
    int ret = 0;

    if (_transferEncoding == HTTPC_TE_IDENTITY) {
        // len < 0: transfer all of it, with timeout
        // len >= 0: max:len, with timeout
        ret = StreamSendSize(_client(), print, len);

        if (len > 0 && ret != len) {
            return HTTPC_ERROR_NO_STREAM;
        }
        // do we have an error?
        //        if(_client->getLastSendReport() != Stream::Report::Success) {
        //            return returnError(StreamReportToHttpClientReport(_client->getLastSendReport()));
        //        }
    } else if (_transferEncoding == HTTPC_TE_CHUNKED) {
        int size = 0;
        while (1) {
            if (!connected()) {
                return returnError(HTTPC_ERROR_CONNECTION_LOST);
            }
            String chunkHeader = _client()->readStringUntil('\n');

            if (chunkHeader.length() <= 0) {
                return returnError(HTTPC_ERROR_READ_TIMEOUT);
            }

            chunkHeader.trim(); // remove \r
            DEBUG_HTTPCLIENT("[HTTP-Client] chunk header: '%s'\n", chunkHeader.c_str());

            // read size of chunk
            len = (uint32_t) strtol((const char *) chunkHeader.c_str(), nullptr, 16);
            size += len;
            DEBUG_HTTPCLIENT("[HTTP-Client] read chunk len: %d\n", len);

            // data left?
            if (len > 0) {
                // read len bytes with timeout
                int r = StreamSendSize(_client(), print, len);
                if (r != len) {
                    return HTTPC_ERROR_NO_STREAM;
                }

                //               if (_client->getLastSendReport() != Stream::Report::Success)
                //                    // not all data transferred
                //                    return returnError(StreamReportToHttpClientReport(_client->getLastSendReport()));
                ret += r;
            } else {

                // if no length Header use global chunk size
                if (_size <= 0) {
                    _size = size;
                }

                // check if we have write all data out
                if (ret != _size) {
                    return returnError(HTTPC_ERROR_STREAM_WRITE);
                }
                break;
            }

            // read trailing \r\n at the end of the chunk
            char buf[2];
            auto trailing_seq_len = _client()->readBytes((uint8_t*)buf, 2);
            if (trailing_seq_len != 2 || buf[0] != '\r' || buf[1] != '\n') {
                return returnError(HTTPC_ERROR_READ_TIMEOUT);
            }
        }
    } else {
        return returnError(HTTPC_ERROR_ENCODING);
    }

    disconnect(true);
    return ret;
}

/**
    return all payload as String (may need lot of ram or trigger out of memory!)
    @return String
*/
const String& HTTPClient::getString(void) {
    if (_payload) {
        return *_payload;
    }

    _payload.reset(new StreamString());

    if (_size > 0) {
        // try to reserve needed memory
        if (!_payload->reserve((_size + 1))) {
            DEBUG_HTTPCLIENT("[HTTP-Client][getString] not enough memory to reserve a string! need: %d\n", (_size + 1));
            return *_payload;
        }
    }

    writeToStream(_payload.get());
    return *_payload;
}

/**
    converts error code to String
    @param error int
    @return String
*/
String HTTPClient::errorToString(int error) {
    switch (error) {
    case HTTPC_ERROR_CONNECTION_FAILED:
        return F("connection failed");
    case HTTPC_ERROR_SEND_HEADER_FAILED:
        return F("send header failed");
    case HTTPC_ERROR_SEND_PAYLOAD_FAILED:
        return F("send payload failed");
    case HTTPC_ERROR_NOT_CONNECTED:
        return F("not connected");
    case HTTPC_ERROR_CONNECTION_LOST:
        return F("connection lost");
    case HTTPC_ERROR_NO_STREAM:
        return F("no stream");
    case HTTPC_ERROR_NO_HTTP_SERVER:
        return F("no HTTP server");
    case HTTPC_ERROR_TOO_LESS_RAM:
        return F("not enough ram");
    case HTTPC_ERROR_ENCODING:
        return F("Transfer-Encoding not supported");
    case HTTPC_ERROR_STREAM_WRITE:
        return F("Stream write error");
    case HTTPC_ERROR_READ_TIMEOUT:
        return F("read Timeout");
    default:
        return String();
    }
}

/**
    adds Header to the request
    @param name
    @param value
    @param first
*/
void HTTPClient::addHeader(const String& name, const String& value, bool first, bool replace) {
    // not allow set of Header handled by code
    if (!name.equalsIgnoreCase(F("Connection")) &&
            !name.equalsIgnoreCase(F("User-Agent")) &&
            !name.equalsIgnoreCase(F("Host")) &&
            !(name.equalsIgnoreCase(F("Authorization")) && _base64Authorization.length())) {

        String headerLine;
        headerLine.reserve(name.length() + value.length() + 4);
        headerLine += name;
        headerLine += ": ";

        if (replace) {
            int headerStart = _headers.indexOf(headerLine);
            if (headerStart != -1) {
                int headerEnd = _headers.indexOf('\n', headerStart);
                _headers = _headers.substring(0, headerStart) + _headers.substring(headerEnd + 1);
            }
        }

        headerLine += value;
        headerLine += "\r\n";
        if (first) {
            _headers = headerLine + _headers;
        } else {
            _headers += headerLine;
        }
    }
}

void HTTPClient::collectHeaders(const char* headerKeys[], const size_t headerKeysCount) {
    _headerKeysCount = headerKeysCount;
    _currentHeaders = std::make_unique<RequestArgument[]>(_headerKeysCount);
    for (size_t i = 0; i < _headerKeysCount; i++) {
        _currentHeaders[i].key = headerKeys[i];
    }
}

String HTTPClient::header(const char* name) {
    for (size_t i = 0; i < _headerKeysCount; ++i) {
        if (_currentHeaders[i].key.equalsIgnoreCase(name)) {
            return _currentHeaders[i].value;
        }
    }
    return String();
}

String HTTPClient::header(size_t i) {
    if (i < _headerKeysCount) {
        return _currentHeaders[i].value;
    }
    return String();
}

String HTTPClient::headerName(size_t i) {
    if (i < _headerKeysCount) {
        return _currentHeaders[i].key;
    }
    return String();
}

int HTTPClient::headers() {
    return _headerKeysCount;
}

bool HTTPClient::hasHeader(const char* name) {
    for (size_t i = 0; i < _headerKeysCount; ++i) {
        if ((_currentHeaders[i].key.equalsIgnoreCase(name)) && (_currentHeaders[i].value.length() > 0)) {
            return true;
        }
    }
    return false;
}

/**
    init TCP connection and handle ssl verify if needed
    @return true if connection is ok
*/
bool HTTPClient::connect(void) {
    if (_reuse && _canReuse && connected()) {
        DEBUG_HTTPCLIENT("[HTTP-Client] connect: already connected, reusing connection\n");
        // clear _client's output (all of it, no timeout)
        while (_client()->available()) {
            _client()->read();
        }
        return true;
    }

    if (!_client()) {
        DEBUG_HTTPCLIENT("[HTTP-Client] connect: HTTPClient::begin was not called or returned error\n");
        return false;
    }

    _client()->setTimeout(_tcpTimeout);

    if (!_client()->connect(_host.c_str(), _port)) {
        DEBUG_HTTPCLIENT("[HTTP-Client] failed connect to %s:%u\n", _host.c_str(), _port);
        return false;
    }

    DEBUG_HTTPCLIENT("[HTTP-Client] connected to %s:%u\n", _host.c_str(), _port);
    _client()->setNoDelay(true);
    return connected();
}

/**
    sends HTTP request header
    @param type (GET, POST, ...)
    @return status
*/
bool HTTPClient::sendHeader(const char * type) {
    if (!connected()) {
        return false;
    }

    String header;
    // 128: Arbitrarily chosen to have enough buffer space for avoiding internal reallocations
    header.reserve(_headers.length() + _uri.length() +
                   _base64Authorization.length() + _host.length() + _userAgent.length() + 128);
    header += type;
    header += ' ';
    if (_uri.length()) {
        header += _uri;
    } else {
        header += '/';
    }
    header += F(" HTTP/1.");

    if (_useHTTP10) {
        header += '0';
    } else {
        header += '1';
    }

    header += F("\r\nHost: ");
    header += _host;
    if (_port != 80 && _port != 443) {
        header += ':';
        header += String(_port);
    }
    if (_userAgent.length()) {
        header += F("\r\nUser-Agent: ");
        header += _userAgent;
    }

    if (!_useHTTP10) {
        header += F("\r\nAccept-Encoding: identity;q=1,chunked;q=0.1,*;q=0");
    }

    if (_base64Authorization.length()) {
        header += F("\r\nAuthorization: Basic ");
        header += _base64Authorization;
    }

    header += F("\r\nConnection: ");
    header += _reuse ? F("keep-alive") : F("close");
    header += "\r\n";

    header += _headers;
    header += "\r\n";

    DEBUG_HTTPCLIENT("[HTTP-Client] sending request header\n-----\n%s-----\n", header.c_str());

    // transfer all of it, with timeout
    return StreamConstPtr(header).sendAll(_client()) == header.length();
}

/**
    reads the response from the server
    @return int http code
*/
int HTTPClient::handleHeaderResponse() {

    if (!connected()) {
        return HTTPC_ERROR_NOT_CONNECTED;
    }

    clear();

    _canReuse = _reuse;

    String transferEncoding;

    _transferEncoding = HTTPC_TE_IDENTITY;
    unsigned long lastDataTime = millis();
    String date;

    while (connected()) {
        size_t len = _client()->available();
        if (len > 0) {
            int headerSeparator = -1;
            String headerLine = _client()->readStringUntil('\n');

            lastDataTime = millis();

            DEBUG_HTTPCLIENT("[HTTP-Client][handleHeaderResponse] RX: '%s'\n", headerLine.c_str());

            if (headerLine.startsWith(F("HTTP/1."))) {

                constexpr auto httpVersionIdx = sizeof "HTTP/1." - 1;
                _canReuse = _canReuse && (headerLine[httpVersionIdx] != '0');
                _returnCode = headerLine.substring(httpVersionIdx + 2, headerLine.indexOf(' ', httpVersionIdx + 2)).toInt();
                _canReuse = _canReuse && (_returnCode > 0) && (_returnCode < 500);

            } else if ((headerSeparator = headerLine.indexOf(':')) > 0) {
                String headerName = headerLine.substring(0, headerSeparator);
                String headerValue = headerLine.substring(headerSeparator + 1);
                headerValue.trim();

                if (headerName.equalsIgnoreCase(F("Content-Length"))) {
                    _size = headerValue.toInt();
                }

                if (headerName.equalsIgnoreCase("Date")) {
                    date = headerValue;
                }

                if (_canReuse && headerName.equalsIgnoreCase(F("Connection"))) {
                    if (headerValue.indexOf(F("close")) >= 0 &&
                            headerValue.indexOf(F("keep-alive")) < 0) {
                        _canReuse = false;
                    }
                }

                if (headerName.equalsIgnoreCase(F("Transfer-Encoding"))) {
                    transferEncoding = headerValue;
                }

                if (headerName.equalsIgnoreCase(F("Location"))) {
                    _location = headerValue;
                }

                if (headerName.equalsIgnoreCase("Set-Cookie")) {
                    setCookie(date, headerValue);
                }

                for (size_t i = 0; i < _headerKeysCount; i++) {
                    if (_currentHeaders[i].key.equalsIgnoreCase(headerName)) {
                        // Uncomment the following lines if you need to add support for multiple headers with the same key:
                        // if (!_currentHeaders[i].value.isEmpty()) {
                        //     // Existing value, append this one with a comma
                        //     _currentHeaders[i].value += ',';
                        //     _currentHeaders[i].value += headerValue;
                        // } else {
                        _currentHeaders[i].value = headerValue;
                        // }
                        break; // We found a match, stop looking
                    }
                }
                continue;
            }

            headerLine.trim(); // remove \r

            if (headerLine == "") {
                DEBUG_HTTPCLIENT("[HTTP-Client][handleHeaderResponse] code: %d\n", _returnCode);

                if (_size > 0) {
                    DEBUG_HTTPCLIENT("[HTTP-Client][handleHeaderResponse] size: %d\n", _size);
                }

                if (transferEncoding.length() > 0) {
                    DEBUG_HTTPCLIENT("[HTTP-Client][handleHeaderResponse] Transfer-Encoding: %s\n", transferEncoding.c_str());
                    if (transferEncoding.equalsIgnoreCase(F("chunked"))) {
                        _transferEncoding = HTTPC_TE_CHUNKED;
                    } else {
                        _returnCode = HTTPC_ERROR_ENCODING;
                        return _returnCode;
                    }
                } else {
                    _transferEncoding = HTTPC_TE_IDENTITY;
                }

                if (_returnCode <= 0) {
                    DEBUG_HTTPCLIENT("[HTTP-Client][handleHeaderResponse] Remote host is not an HTTP Server!");
                    _returnCode = HTTPC_ERROR_NO_HTTP_SERVER;
                }
                return _returnCode;
            }

        } else {
            if ((millis() - lastDataTime) > _tcpTimeout) {
                return HTTPC_ERROR_READ_TIMEOUT;
            }
        }
    }

    return HTTPC_ERROR_CONNECTION_LOST;
}

/**
    called to handle error return, may disconnect the connection if still exists
    @param error
    @return error
*/
int HTTPClient::returnError(int error) {
    if (error < 0) {
        DEBUG_HTTPCLIENT("[HTTP-Client][returnError] error(%d): %s\n", error, errorToString(error).c_str());
        if (connected()) {
            DEBUG_HTTPCLIENT("[HTTP-Client][returnError] tcp stop\n");
            _client()->stop();
        }
    }
    return error;
}

void HTTPClient::setCookieJar(CookieJar* cookieJar) {
    _cookieJar = cookieJar;
}

void HTTPClient::resetCookieJar() {
    _cookieJar = nullptr;
}

void HTTPClient::clearAllCookies() {
    if (_cookieJar) {
        _cookieJar->clear();
    }
}

void HTTPClient::setCookie(String date, String headerValue) {
    if (!_cookieJar) {
        return;
    }

#define HTTP_TIME_PATTERN "%a, %d %b %Y %H:%M:%S"

    Cookie cookie;
    String value;
    int pos1, pos2;

    struct tm tm;
    strptime(date.c_str(), HTTP_TIME_PATTERN, &tm);
    cookie.date = mktime(&tm);

    pos1 = headerValue.indexOf('=');
    pos2 = headerValue.indexOf(';');

    if (pos1 >= 0 && pos2 > pos1) {
        cookie.name = headerValue.substring(0, pos1);
        cookie.value = headerValue.substring(pos1 + 1, pos2);
    } else {
        return;     // invalid cookie header
    }

    // only Cookie Attributes are case insensitive from this point on
    headerValue.toLowerCase();

    // expires
    if (headerValue.indexOf("expires=") >= 0) {
        pos1 = headerValue.indexOf("expires=") + strlen("expires=");
        pos2 = headerValue.indexOf(';', pos1);

        if (pos2 > pos1) {
            value = headerValue.substring(pos1, pos2);
        } else {
            value = headerValue.substring(pos1);
        }

        strptime(value.c_str(), HTTP_TIME_PATTERN, &tm);
        cookie.expires.date = mktime(&tm);
        cookie.expires.valid = true;
    }

    // max-age
    if (headerValue.indexOf("max-age=") >= 0) {
        pos1 = headerValue.indexOf("max-age=") + strlen("max-age=");
        pos2 = headerValue.indexOf(';', pos1);

        if (pos2 > pos1) {
            value = headerValue.substring(pos1, pos2);
        } else {
            value = headerValue.substring(pos1);
        }

        cookie.max_age.duration = value.toInt();
        cookie.max_age.valid = true;
    }

    // domain
    if (headerValue.indexOf("domain=") >= 0) {
        pos1 = headerValue.indexOf("domain=") + strlen("domain=");
        pos2 = headerValue.indexOf(';', pos1);

        if (pos2 > pos1) {
            value = headerValue.substring(pos1, pos2);
        } else {
            value = headerValue.substring(pos1);
        }

        if (value.startsWith(".")) {
            value.remove(0, 1);
        }

        if (_host.indexOf(value) >= 0) {
            cookie.domain = value;
        } else {
            return;     // server tries to set a cookie on a different domain; ignore it
        }
    } else {
        pos1 = _host.lastIndexOf('.', _host.lastIndexOf('.') - 1);
        if (pos1 >= 0) {
            cookie.domain = _host.substring(pos1 + 1);
        } else {
            cookie.domain = _host;
        }
    }

    // path
    if (headerValue.indexOf("path=") >= 0) {
        pos1 = headerValue.indexOf("path=") + strlen("path=");
        pos2 = headerValue.indexOf(';', pos1);

        if (pos2 > pos1) {
            cookie.path = headerValue.substring(pos1, pos2);
        } else {
            cookie.path = headerValue.substring(pos1);
        }
    }

    // HttpOnly
    cookie.http_only = (headerValue.indexOf("httponly") >= 0);

    // secure
    cookie.secure = (headerValue.indexOf("secure") >= 0);

    // overwrite or delete cookie in/from cookie jar
    time_t now_local = time(NULL);
    time_t now_gmt = mktime(gmtime(&now_local));

    bool found = false;

    for (auto c = _cookieJar->begin(); c != _cookieJar->end(); ++c) {
        if (c->domain == cookie.domain && c->name == cookie.name) {
            // when evaluating, max-age takes precedence over expires if both are defined
            if ((cookie.max_age.valid && ((cookie.date + cookie.max_age.duration) < now_gmt || (cookie.max_age.duration <= 0)))
                    || (!cookie.max_age.valid && cookie.expires.valid && (cookie.expires.date < now_gmt))) {
                _cookieJar->erase(c);
                c--;
            } else {
                *c = cookie;
            }
            found = true;
        }
    }

    // add cookie to jar
    if (!found && !(cookie.max_age.valid && cookie.max_age.duration <= 0)) {
        _cookieJar->push_back(cookie);
    }

}

bool HTTPClient::generateCookieString(String *cookieString) {
    if (!_cookieJar) {
        return false;
    }
    time_t now_local = time(NULL);
    time_t now_gmt = mktime(gmtime(&now_local));

    *cookieString = "";
    bool found = false;

    for (auto c = _cookieJar->begin(); c != _cookieJar->end(); ++c) {
        if ((c->max_age.valid && ((c->date + c->max_age.duration) < now_gmt)) || (!c->max_age.valid && c->expires.valid && (c->expires.date < now_gmt))) {
            _cookieJar->erase(c);
            c--;
        } else if (_host.indexOf(c->domain) >= 0 && (!c->secure || _secure)) {
            if (*cookieString == "") {
                *cookieString = c->name + "=" + c->value;
            } else {
                *cookieString += " ;" + c->name + "=" + c->value;
            }
            found = true;
        }
    }
    return found;
}
