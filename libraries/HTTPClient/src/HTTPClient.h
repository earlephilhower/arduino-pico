/**
    HTTPClient.h

    Modified 2022 by Earle F. Philhower, III

    Created on: 02.11.2015

    Copyright (c) 2015 Markus Sattler. All rights reserved.
    This file is part of the ESP8266HTTPClient for Arduino.

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

    Modified by Jeroen Döll, June 2018
*/

#pragma once

#include <Arduino.h>
#include <StreamString.h>
#include <WiFiClient.h>
#include <WiFiClientSecure.h>

#include <memory>
#include <vector>

#ifdef DEBUG_RP2040_CORE
#ifdef DEBUG_RP2040_PORT
#define DEBUG_HTTPCLIENT(fmt, ...) DEBUG_RP2040_PORT.printf_P( (PGM_P)PSTR(fmt), ## __VA_ARGS__ )
#endif
#endif

//#define DEBUG_HTTPCLIENT(fmt, ...) Serial.printf(fmt, ## __VA_ARGS__ )
#ifndef DEBUG_HTTPCLIENT
#define DEBUG_HTTPCLIENT(...) do { (void)0; } while (0)
#endif


#define HTTPCLIENT_DEFAULT_TCP_TIMEOUT (5000)

/// HTTP client errors
#define HTTPC_ERROR_CONNECTION_FAILED   (-1)
#define HTTPC_ERROR_SEND_HEADER_FAILED  (-2)
#define HTTPC_ERROR_SEND_PAYLOAD_FAILED (-3)
#define HTTPC_ERROR_NOT_CONNECTED       (-4)
#define HTTPC_ERROR_CONNECTION_LOST     (-5)
#define HTTPC_ERROR_NO_STREAM           (-6)
#define HTTPC_ERROR_NO_HTTP_SERVER      (-7)
#define HTTPC_ERROR_TOO_LESS_RAM        (-8)
#define HTTPC_ERROR_ENCODING            (-9)
#define HTTPC_ERROR_STREAM_WRITE        (-10)
#define HTTPC_ERROR_READ_TIMEOUT        (-11)

constexpr int HTTPC_ERROR_CONNECTION_REFUSED __attribute__((deprecated)) = HTTPC_ERROR_CONNECTION_FAILED;

/// size for the stream handling
#define HTTP_TCP_BUFFER_SIZE (1460)

/// HTTP codes see RFC7231
typedef enum {
    HTTP_CODE_CONTINUE = 100,
    HTTP_CODE_SWITCHING_PROTOCOLS = 101,
    HTTP_CODE_PROCESSING = 102,
    HTTP_CODE_OK = 200,
    HTTP_CODE_CREATED = 201,
    HTTP_CODE_ACCEPTED = 202,
    HTTP_CODE_NON_AUTHORITATIVE_INFORMATION = 203,
    HTTP_CODE_NO_CONTENT = 204,
    HTTP_CODE_RESET_CONTENT = 205,
    HTTP_CODE_PARTIAL_CONTENT = 206,
    HTTP_CODE_MULTI_STATUS = 207,
    HTTP_CODE_ALREADY_REPORTED = 208,
    HTTP_CODE_IM_USED = 226,
    HTTP_CODE_MULTIPLE_CHOICES = 300,
    HTTP_CODE_MOVED_PERMANENTLY = 301,
    HTTP_CODE_FOUND = 302,
    HTTP_CODE_SEE_OTHER = 303,
    HTTP_CODE_NOT_MODIFIED = 304,
    HTTP_CODE_USE_PROXY = 305,
    HTTP_CODE_TEMPORARY_REDIRECT = 307,
    HTTP_CODE_PERMANENT_REDIRECT = 308,
    HTTP_CODE_BAD_REQUEST = 400,
    HTTP_CODE_UNAUTHORIZED = 401,
    HTTP_CODE_PAYMENT_REQUIRED = 402,
    HTTP_CODE_FORBIDDEN = 403,
    HTTP_CODE_NOT_FOUND = 404,
    HTTP_CODE_METHOD_NOT_ALLOWED = 405,
    HTTP_CODE_NOT_ACCEPTABLE = 406,
    HTTP_CODE_PROXY_AUTHENTICATION_REQUIRED = 407,
    HTTP_CODE_REQUEST_TIMEOUT = 408,
    HTTP_CODE_CONFLICT = 409,
    HTTP_CODE_GONE = 410,
    HTTP_CODE_LENGTH_REQUIRED = 411,
    HTTP_CODE_PRECONDITION_FAILED = 412,
    HTTP_CODE_PAYLOAD_TOO_LARGE = 413,
    HTTP_CODE_URI_TOO_LONG = 414,
    HTTP_CODE_UNSUPPORTED_MEDIA_TYPE = 415,
    HTTP_CODE_RANGE_NOT_SATISFIABLE = 416,
    HTTP_CODE_EXPECTATION_FAILED = 417,
    HTTP_CODE_MISDIRECTED_REQUEST = 421,
    HTTP_CODE_UNPROCESSABLE_ENTITY = 422,
    HTTP_CODE_LOCKED = 423,
    HTTP_CODE_FAILED_DEPENDENCY = 424,
    HTTP_CODE_UPGRADE_REQUIRED = 426,
    HTTP_CODE_PRECONDITION_REQUIRED = 428,
    HTTP_CODE_TOO_MANY_REQUESTS = 429,
    HTTP_CODE_REQUEST_HEADER_FIELDS_TOO_LARGE = 431,
    HTTP_CODE_INTERNAL_SERVER_ERROR = 500,
    HTTP_CODE_NOT_IMPLEMENTED = 501,
    HTTP_CODE_BAD_GATEWAY = 502,
    HTTP_CODE_SERVICE_UNAVAILABLE = 503,
    HTTP_CODE_GATEWAY_TIMEOUT = 504,
    HTTP_CODE_HTTP_VERSION_NOT_SUPPORTED = 505,
    HTTP_CODE_VARIANT_ALSO_NEGOTIATES = 506,
    HTTP_CODE_INSUFFICIENT_STORAGE = 507,
    HTTP_CODE_LOOP_DETECTED = 508,
    HTTP_CODE_NOT_EXTENDED = 510,
    HTTP_CODE_NETWORK_AUTHENTICATION_REQUIRED = 511
} t_http_codes;

typedef enum {
    HTTPC_TE_IDENTITY,
    HTTPC_TE_CHUNKED
} transferEncoding_t;

/**
    redirection follow mode.
    + `HTTPC_DISABLE_FOLLOW_REDIRECTS` - no redirection will be followed.
    + `HTTPC_STRICT_FOLLOW_REDIRECTS` - strict RFC2616, only requests using
        GET or HEAD methods will be redirected (using the same method),
        since the RFC requires end-user confirmation in other cases.
    + `HTTPC_FORCE_FOLLOW_REDIRECTS` - all redirections will be followed,
        regardless of a used method. New request will use the same method,
        and they will include the same body data and the same headers.
        In the sense of the RFC, it's just like every redirection is confirmed.
*/
typedef enum {
    HTTPC_DISABLE_FOLLOW_REDIRECTS,
    HTTPC_STRICT_FOLLOW_REDIRECTS,
    HTTPC_FORCE_FOLLOW_REDIRECTS
} followRedirects_t;

class TransportTraits;
typedef std::unique_ptr<TransportTraits> TransportTraitsPtr;


// cookie jar support
typedef struct  {
    String host;       // host which tries to set the cookie
    time_t date;       // timestamp of the response that set the cookie
    String name;
    String value;
    String domain;
    String path = "";
    struct {
        time_t date = 0;
        bool valid = false;
    } expires;
    struct {
        time_t duration = 0;
        bool valid = false;
    } max_age;
    bool http_only = false;
    bool secure = false;
} Cookie;
typedef std::vector<Cookie> CookieJar;

class HTTPStream : public WiFiClient {
public:
    HTTPStream() {
        _conn = nullptr;
        _chunked = false;
        _chunkLen = 0;
        _state = READ_HEX;
        _partial = 0;
        _eof = false;
    }

    HTTPStream(const HTTPStream&);
    HTTPStream& operator=(const HTTPStream&);

    void reset(WiFiClient *connection, bool chunked) {
        DEBUG_HTTPCLIENT("[HTTPStream] reset(%p, %d)\n", connection, chunked);
        _conn = connection;
        _partial = 0;
        _state = READ_HEX;
        _chunked = chunked;
        _eof = false;
    }

    virtual size_t write(uint8_t c) override {
        return _conn->write(c);
    }

    virtual size_t write(const uint8_t *buf, size_t size) override {
        return _conn->write(buf, size);
    }

    virtual uint8_t connected() override {
        return _conn->connected();
    }

    virtual int available() override {
        if (!_chunked) {
            return _conn->available();
        }
        // We're chunked at this point
        if (_eof) {
            return 0;
        }
        if (!_chunkLen) {
            tryReadChunkLen(1);
        }
        return std::min(_chunkLen, _conn->available());
    }

    virtual int availableForWrite() override {
        return _conn->availableForWrite();
    }

    virtual void flush() override {
        _conn->flush();
    }

    virtual void stop() override {
        _conn->stop();
    }

    virtual int read() override {
        if (!_chunked) {
            return _conn->read();
        }
        // We're chunked at this point
        if (_eof) {
            DEBUG_HTTPCLIENT("[HTTPStream] ::read() after EOF\n");
            return -1;
        }
        if (!_chunkLen) {
            tryReadChunkLen(_timeout);
        }
        if (!_chunkLen) {
            DEBUG_HTTPCLIENT("[HTTPStream] ::read no chunkLen\n");
            return -1;
        }
        uint32_t start = millis();
        while (((millis() - start) < _timeout) && !_conn->available() && _conn->connected()) {
            delay(1);
        }
        if (_conn->available()) {
            _chunkLen--;
            return _conn->read();
        } else {
            DEBUG_HTTPCLIENT("[HTTPStream] ::read wrapped stream failure. avail = %d, conn = %d\n", _conn->available(), _conn->connected());
            return -1;
        }
    }

    virtual int read(uint8_t *buf, size_t size) override {
        if (!_chunked) {
            return _conn->read(buf, size);
        }
        for (int i = 0; i < (int)size; i++) {
            int c = read();
            if (c < 0) {
                return i;
            }
            *(buf++) = (uint8_t)c;
        }
        return size;
    }

    virtual int peek() override {
        if (!_chunked) {
            return _conn->peek();
        }
        // We're chunked at this point
        if (_eof) {
            DEBUG_HTTPCLIENT("[HTTPStream] ::peek after EOF\n");
            return -1;
        }
        if (!_chunkLen) {
            tryReadChunkLen(_timeout);
        }
        if (!_chunkLen) {
            DEBUG_HTTPCLIENT("[HTTPStream] ::peek no chunkLen\n");
            return -1;
        }
        uint32_t start = millis();
        while (((millis() - start) < _timeout) && !_conn->available() && _conn->connected()) {
            delay(1);
        }
        if (_conn->available()) {
            return _conn->peek();
        } else {
            DEBUG_HTTPCLIENT("[HTTPStream] ::peek wrapped stream failure. avail = %d, conn = %d\n", _conn->available(), _conn->connected());
            return -1;
        }
    }

    void setTimeout(unsigned long timeout) {
        _timeout = timeout;
        _conn->setTimeout(timeout);
    }

private:
    void tryReadChunkLen(uint32_t to) {
        if (_state == ERROR) {
            return;
        }
        uint32_t start = millis();
        while ((millis() - start) <= to) {
            if (_conn->available()) {
                int recv = _conn->read();
                if (recv < 0) {
                    DEBUG_HTTPCLIENT("[HTTPStream] Read of available data failed\n");
                    _state = ERROR;
                    return;
                }
                switch (_state) {
                case READ_HEX:
                    if (recv == '\r') {
                        DEBUG_HTTPCLIENT("[HTTPStream] Saw \\r of chunk len\r\n");
                        _state = READ_LF;
                        break;
                    }
                    if (recv >= '0' && recv <= '9') {
                        DEBUG_HTTPCLIENT("[HTTPStream] Read %c of chunk size\n", recv);
                        _partial <<= 4;
                        _partial |= recv - '0';
                    } else if (tolower(recv) >= 'a' && tolower(recv) <= 'f') {
                        DEBUG_HTTPCLIENT("[HTTPStream] Read %c of chunk size\n", recv);
                        _partial <<= 4;
                        _partial |= tolower(recv) - 'a' + 10;
                    } else {
                        DEBUG_HTTPCLIENT("[HTTPStream] READ_HEX error '%c'\n", recv);
                        _state = ERROR;
                        return;
                    }
                    break;
                case READ_LF:
                    if (recv != '\n') {
                        _state = ERROR;
                        DEBUG_HTTPCLIENT("[HTTPStream] READ_LF error '%02x'\n", recv);
                        return;
                    }
                    DEBUG_HTTPCLIENT("[HTTPStream] Chunk len = %d\n", _partial);
                    _chunkLen = _partial;
                    _partial = 0;
                    _state = TAIL_CR;
                    if (_chunkLen == 0) {
                        // 0-sized chunk is EOF special case
                        _eof = true;
                    }
                    return;
                case TAIL_CR:
                    if (recv == '\r') {
                        DEBUG_HTTPCLIENT("[HTTPStream] Saw \\r of chunk end\n");
                        _state = TAIL_LF;
                        break;
                    }
                    DEBUG_HTTPCLIENT("[HTTPStream] TAIL_CR error '%c'\n", recv);
                    _state = ERROR;
                    return;
                case TAIL_LF:
                    if (recv == '\n') {
                        DEBUG_HTTPCLIENT("[HTTPStream] Saw \\n of chunk end\n");
                        _state = READ_HEX;
                        break;
                    }
                    DEBUG_HTTPCLIENT("[HTTPStream] TAIL_LF error '%c'\n", recv);
                    _state = ERROR;
                    return;
                case ERROR:
                    return;
                }
            }
        }
        DEBUG_HTTPCLIENT("[HTTPStream] Timeout waiting for chunk\n");
    }

    WiFiClient *_conn;
    bool _chunked;
    int _chunkLen;
    enum { READ_HEX, READ_LF, TAIL_CR, TAIL_LF, ERROR } _state;
    int _partial;
    bool _eof;
};


class HTTPClient {
public:
    HTTPClient() = default;
    ~HTTPClient() {
        if (_clientMade) {
            delete _clientMade;
        }
    }
    HTTPClient(HTTPClient&&) = default;
    HTTPClient& operator=(HTTPClient&&) = default;

    // The easier way
    bool begin(String url);
    bool begin(String host, uint16_t port, String uri = "/", bool https = false);
    bool begin(String url, const uint8_t httpsFingerprint[20]) {
        setFingerprint(httpsFingerprint);
        return begin(url);
    }
    bool begin(String host, uint16_t port, String uri, const uint8_t httpsFingerprint[20]) {
        setFingerprint(httpsFingerprint);
        return begin(host, port, uri);
    }

    // Let's do it the hard way, too
    bool begin(WiFiClient &client, const String& url);
    bool begin(WiFiClient &client, const String& host, uint16_t port, const String& uri = "/", bool https = false);


    void end(void);

    bool connected(void);

    void setReuse(bool reuse); /// keep-alive
    void setUserAgent(const String& userAgent);
    void setAuthorization(const char * user, const char * password);
    void setAuthorization(const char * auth);
    void setAuthorization(String auth);
    void setTimeout(uint16_t timeout);

    // Redirections
    void setFollowRedirects(followRedirects_t follow);
    void setRedirectLimit(uint16_t limit); // max redirects to follow for a single request

    bool setURL(const String& url); // handy for handling redirects
    void useHTTP10(bool usehttp10 = true);

    /// request handling
    int GET();
    int DELETE();
    int POST(const uint8_t* payload, size_t size);
    int POST(const String& payload);
    int PUT(const uint8_t* payload, size_t size);
    int PUT(const String& payload);
    int PATCH(const uint8_t* payload, size_t size);
    int PATCH(const String& payload);
    int sendRequest(const char* type, const String& payload);
    int sendRequest(const char* type, const uint8_t* payload = nullptr, size_t size = 0);
    int sendRequest(const char* type, Stream * stream, size_t size = 0);

    void addHeader(const String& name, const String& value, bool first = false, bool replace = true);

    /// Response handling
    void collectHeaders(const char* headerKeys[], const size_t headerKeysCount);
    String header(const char* name);   // get request header value by name
    String header(size_t i);              // get request header value by number
    String headerName(size_t i);          // get request header name by number
    int headers();                     // get header count
    bool hasHeader(const char* name);  // check if header exists


    int getSize(void);
    const String& getLocation(void); // Location header from redirect if 3XX

    WiFiClient& getStream(void);
    WiFiClient* getStreamPtr(void);
    int writeToPrint(Print* print);
    int writeToStream(Stream* stream);
    const String& getString(void);
    static String errorToString(int error);

    // Cookie jar support
    void setCookieJar(CookieJar* cookieJar);
    void resetCookieJar();
    void clearAllCookies();

    // ----------------------------------------------------------------------------------------------
    // HTTPS support, mirrors the WiFiClientSecure interface
    // Could possibly use a virtual interface class between the two, but for now it is more
    // straightforward to simply feed calls through manually here.
    void setSession(Session *session) {
        _tls()->setSession(session);
    }
    void setInsecure() {
        _tls()->setInsecure();
    }
    void setKnownKey(const PublicKey *pk, unsigned usages = BR_KEYTYPE_KEYX | BR_KEYTYPE_SIGN) {
        _tls()->setKnownKey(pk, usages);
    }
    bool setFingerprint(const uint8_t fingerprint[20]) {
        return _tls()->setFingerprint(fingerprint);
    }
    bool setFingerprint(const char *fpStr) {
        return _tls()->setFingerprint(fpStr);
    }
    void allowSelfSignedCerts() {
        _tls()->allowSelfSignedCerts();
    }
    void setTrustAnchors(const X509List *ta) {
        _tls()->setTrustAnchors(ta);
    }
    void setX509Time(time_t now) {
        _tls()->setX509Time(now);
    }
    void setClientRSACert(const X509List *cert, const PrivateKey *sk) {
        _tls()->setClientRSACert(cert, sk);
    }
    void setClientECCert(const X509List *cert, const PrivateKey *sk, unsigned allowed_usages, unsigned cert_issuer_key_type) {
        _tls()->setClientECCert(cert, sk, allowed_usages, cert_issuer_key_type);
    }
    void setBufferSizes(int recv, int xmit) {
        _tls()->setBufferSizes(recv, xmit);
    }
    void setCertStore(CertStoreBase *certStore) {
        _tls()->setCertStore(certStore);
    }
    bool setCiphers(const uint16_t *cipherAry, int cipherCount) {
        return _tls()->setCiphers(cipherAry, cipherCount);
    }
    bool setCiphers(const std::vector<uint16_t>& list) {
        return _tls()->setCiphers(list);
    }
    bool setCiphersLessSecure() {
        return _tls()->setCiphersLessSecure();
    }
    bool setSSLVersion(uint32_t min = BR_TLS10, uint32_t max = BR_TLS12) {
        return _tls()->setSSLVersion(min, max);
    }
    void setCACert(const char *rootCA) {
        _tls()->setCACert(rootCA);
    }
    void setCertificate(const char *client_ca) {
        _tls()->setCertificate(client_ca);
    }
    void setPrivateKey(const char *private_key) {
        _tls()->setPrivateKey(private_key);
    }
    bool loadCACert(Stream& stream, size_t size) {
        return _tls()->loadCACert(stream, size);
    }
    bool loadCertificate(Stream& stream, size_t size) {
        return _tls()->loadCertificate(stream, size);
    }
    bool loadPrivateKey(Stream& stream, size_t size) {
        return _tls()->loadPrivateKey(stream, size);
    }



protected:
    // HTTPS helpers
    WiFiClientSecure *_tls() {
        if (!_clientMade) {
            _clientMade = new WiFiClientSecure();
            _clientGiven = false;
        }
        _clientTLS = true;
        return (WiFiClientSecure*)_clientMade;
    }

    struct RequestArgument {
        String key;
        String value;
    };

    bool beginInternal(const String& url, const char* expectedProtocol);
    void disconnect(bool preserveClient = false);
    void clear();
    int returnError(int error);
    bool connect(void);
    bool sendHeader(const char * type);
    int handleHeaderResponse();
    int writeToStreamDataBlock(Stream * stream, int len);

    // Cookie jar support
    void setCookie(String date, String headerValue);
    bool generateCookieString(String *cookieString);

    WiFiClient *_clientMade = nullptr;
    bool _clientTLS = false;

    std::unique_ptr<WiFiClient> _clientIn;
    bool _clientGiven = false;

    WiFiClient *_client() {
        if (_clientGiven) {
            return _clientIn.get();
        } else {
            return _clientMade;
        }
    }

    /// request handling
    String _host;
    uint16_t _port = 0;
    bool _reuse = true;
    uint16_t _tcpTimeout = HTTPCLIENT_DEFAULT_TCP_TIMEOUT;
    bool _useHTTP10 = false;

    String _uri;
    String _protocol;
    bool _secure = false;
    String _headers;
    String _base64Authorization;

    static const String defaultUserAgent;
    String _userAgent = defaultUserAgent;

    /// Response handling
    std::unique_ptr<RequestArgument[]> _currentHeaders;
    size_t _headerKeysCount = 0;

    int _returnCode = 0;
    int _size = -1;
    bool _canReuse = false;
    followRedirects_t _followRedirects = HTTPC_DISABLE_FOLLOW_REDIRECTS;
    uint16_t _redirectLimit = 10;
    String _location;
    transferEncoding_t _transferEncoding = HTTPC_TE_IDENTITY;
    std::unique_ptr<StreamString> _payload;
    // Cookie jar support
    CookieJar *_cookieJar = nullptr;

    HTTPStream _stream;
};
