/*
    HTTPServer.h - Dead simple web-server.
    Supports only one simultaneous client, knows how to handle GET and POST.

    Copyright (c) 2014 Ivan Grokhotkov. All rights reserved.

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

#include <functional>
#include <memory>
#include <WiFi.h>
#include "HTTP_Method.h"
#include "Uri.h"

enum HTTPUploadStatus { UPLOAD_FILE_START, UPLOAD_FILE_WRITE, UPLOAD_FILE_END,
                        UPLOAD_FILE_ABORTED
                      };
enum HTTPRawStatus { RAW_START, RAW_WRITE, RAW_END, RAW_ABORTED };
enum HTTPClientStatus { HC_NONE, HC_WAIT_READ, HC_WAIT_CLOSE };
enum HTTPAuthMethod { BASIC_AUTH, DIGEST_AUTH };

#define HTTP_DOWNLOAD_UNIT_SIZE 1436

#ifndef HTTP_UPLOAD_BUFLEN
#define HTTP_UPLOAD_BUFLEN 1436
#endif

#ifndef HTTP_RAW_BUFLEN
#define HTTP_RAW_BUFLEN 1436
#endif

#define HTTP_MAX_DATA_WAIT 5000 //ms to wait for the client to send the request
#define HTTP_MAX_DATA_AVAILABLE_WAIT 30 //ms to wait for the client to send the request when there is another client with data available
#define HTTP_MAX_POST_WAIT 5000 //ms to wait for POST data to arrive
#define HTTP_MAX_SEND_WAIT 5000 //ms to wait for data chunk to be ACKed
#define HTTP_MAX_CLOSE_WAIT 2000 //ms to wait for the client to close the connection

#define CONTENT_LENGTH_UNKNOWN ((size_t) -1)
#define CONTENT_LENGTH_NOT_SET ((size_t) -2)

#define WEBSERVER_HAS_HOOK 1

class HTTPServer;

typedef struct {
    HTTPUploadStatus status;
    String  filename;
    String  name;
    String  type;
    size_t  totalSize;    // file size
    size_t  currentSize;  // size of data currently in buf
    uint8_t buf[HTTP_UPLOAD_BUFLEN];
} HTTPUpload;


typedef struct {
    HTTPRawStatus status;
    size_t  totalSize;   // content size
    size_t  currentSize; // size of data currently in buf
    uint8_t buf[HTTP_UPLOAD_BUFLEN];
    void    *data;       // additional data
} HTTPRaw;


#include "detail/RequestHandler.h"

namespace fs {
class FS;
}

class HTTPServer {
public:
    HTTPServer();
    virtual ~HTTPServer();

    virtual void httpClose();

    bool authenticate(const char * username, const char * password);
    void requestAuthentication(HTTPAuthMethod mode = BASIC_AUTH, const char* realm = nullptr, const String& authFailMsg = String(""));

    typedef std::function<void(void)> THandlerFunction;
    void on(const Uri &uri, THandlerFunction fn);
    void on(const Uri &uri, HTTPMethod method, THandlerFunction fn);
    void on(const Uri &uri, HTTPMethod method, THandlerFunction fn, THandlerFunction ufn); //ufn handles file uploads
    void addHandler(RequestHandler* handler);
    void serveStatic(const char* uri, fs::FS& fs, const char* path, const char* cache_header = nullptr);
    void onNotFound(THandlerFunction fn);  //called when handler is not assigned
    void onFileUpload(THandlerFunction ufn); //handle file uploads

    String uri() {
        return _currentUri;
    }
    HTTPMethod method() {
        return _currentMethod;
    }
    HTTPUpload& upload() {
        return *_currentUpload;
    }
    HTTPRaw& raw() {
        return *_currentRaw;
    }

    String pathArg(unsigned int i); // get request path argument by number
    String arg(String name);        // get request argument value by name
    String arg(int i);              // get request argument value by number
    String argName(int i);          // get request argument name by number
    int args();                     // get arguments count
    bool hasArg(String name);       // check if argument exists
    void collectHeaders(const char* headerKeys[], const size_t headerKeysCount); // set the request headers to collect
    template<typename... Args>
    void collectHeaders(const Args&... args) { // set the request headers to collect (variadic template version)
        if (_currentHeaders) {
            delete[] _currentHeaders;
        }
        _headerKeysCount = sizeof...(args) + 1;
        _currentHeaders = new RequestArgument[_headerKeysCount] {
            { .key = "Authorization", .value = "" },
            { .key = String(args), .value = "" } ...
        };
    }

    String header(String name);     // get request header value by name
    String header(int i);           // get request header value by number
    String headerName(int i);       // get request header name by number
    int headers();                  // get header count
    bool hasHeader(String name);    // check if header exists

    int clientContentLength() {
        return _clientContentLength;    // return "content-length" of incoming HTTP header from "_currentClient"
    }

    String hostHeader();            // get request host header if available or empty String if not

    // send response to the client
    // code - HTTP response code, can be 200 or 404
    // content_type - HTTP content type, like "text/plain" or "image/png"
    // content - actual content body
    void send(int code, const char* content_type = nullptr, const String& content = String(""));
    void send(int code, char* content_type, const String& content);
    void send(int code, const String& content_type, const String& content);
    void send(int code, const char* content_type, const char* content);
    void send(int code, const char* content_type, const char* content, size_t contentLength);

    void send_P(int code, PGM_P content_type, PGM_P content);
    void send_P(int code, PGM_P content_type, PGM_P content, size_t contentLength);
    template<typename TypeName>
    void send(int code, PGM_P content_type, TypeName content, size_t contentLength) {
        send(code, content_type, (const char *)content, contentLength);
    }

    void enableDelay(bool value);
    void enableCORS(bool value = true);
    void enableCrossOrigin(bool value = true);

    void setContentLength(const size_t contentLength);
    void sendHeader(const String& name, const String& value, bool first = false);
    void sendContent(const String& content);
    void sendContent(const char* content, size_t contentLength);
    void sendContent_P(PGM_P content);
    void sendContent_P(PGM_P content, size_t size);

    bool chunkedResponseModeStart_P(int code, PGM_P content_type) {
        if (_currentVersion == 0)
            // no chunk mode in HTTP/1.0
        {
            return false;
        }
        setContentLength(CONTENT_LENGTH_UNKNOWN);
        send(code, content_type, "");
        return true;
    }
    bool chunkedResponseModeStart(int code, const char* content_type) {
        return chunkedResponseModeStart_P(code, content_type);
    }
    bool chunkedResponseModeStart(int code, const String& content_type) {
        return chunkedResponseModeStart_P(code, content_type.c_str());
    }
    void chunkedResponseFinalize() {
        sendContent("");
    }

    static String urlDecode(const String& text);

    template<typename T>
    size_t streamFile(T &file, const String& contentType, const int code = 200) {
        _streamFileCore(file.size(), file.name(), contentType, code);
        return _currentClient->write(file);
    }

    // Hook
    enum ClientFuture { CLIENT_REQUEST_CAN_CONTINUE, CLIENT_REQUEST_IS_HANDLED, CLIENT_MUST_STOP, CLIENT_IS_GIVEN };
    typedef String(*ContentTypeFunction)(const String&);
    using HookFunction = std::function<ClientFuture(const String& method, const String& url, WiFiClient* client, ContentTypeFunction contentType)>;
    void addHook(HookFunction hook) {
        if (_hook) {
            auto previousHook = _hook;
            _hook = [previousHook, hook](const String & method, const String & url, WiFiClient * client, ContentTypeFunction contentType) {
                auto whatNow = previousHook(method, url, client, contentType);
                if (whatNow == CLIENT_REQUEST_CAN_CONTINUE) {
                    return hook(method, url, client, contentType);
                }
                return whatNow;
            };
        } else {
            _hook = hook;
        }
    }

protected:
    virtual size_t _currentClientWrite(const char* b, size_t l) {
        return _currentClient->write(b, l);
    }
    virtual size_t _currentClientWrite_P(PGM_P b, size_t l) {
        return _currentClient->write(b, l);
    }
    void _addRequestHandler(RequestHandler* handler);
    void _handleRequest();
    void _finalizeResponse();
    ClientFuture _parseRequest(WiFiClient* client);
    void _parseArguments(String data);
    static String _responseCodeToString(int code);
    bool _parseForm(WiFiClient* client, String boundary, uint32_t len);
    bool _parseFormUploadAborted();
    void _uploadWriteByte(uint8_t b);
    int _uploadReadByte(WiFiClient* client);
    void _prepareHeader(String& response, int code, const char* content_type, size_t contentLength);
    bool _collectHeader(const char* headerName, const char* headerValue);

    void _streamFileCore(const size_t fileSize, const String & fileName, const String & contentType, const int code = 200);

    String _getRandomHexString();
    // for extracting Auth parameters
    String _extractParam(String& authReq, const String& param, const char delimit = '"');

    struct RequestArgument {
        String key;
        String value;
    };

    bool        _corsEnabled;

    WiFiClient  *_currentClient;
    HTTPMethod  _currentMethod;
    String      _currentUri;
    uint8_t     _currentVersion;
    HTTPClientStatus _currentStatus;
    unsigned long _statusChange;
    bool        _nullDelay;

    RequestHandler*  _currentHandler;
    RequestHandler*  _firstHandler;
    RequestHandler*  _lastHandler;
    THandlerFunction _notFoundHandler;
    THandlerFunction _fileUploadHandler;

    int              _currentArgCount;
    RequestArgument* _currentArgs;
    int              _postArgsLen;
    RequestArgument* _postArgs;

    std::unique_ptr<HTTPUpload> _currentUpload;
    std::unique_ptr<HTTPRaw>    _currentRaw;

    int              _headerKeysCount;
    RequestArgument* _currentHeaders;
    size_t           _contentLength;
    int              _clientContentLength;	// "Content-Length" from header of incoming POST or GET request
    String           _responseHeaders;

    String           _hostHeader;
    bool             _chunked;

    String           _snonce;  // Store noance and opaque for future comparison
    String           _sopaque;
    String           _srealm;  // Store the Auth realm between Calls

    HookFunction     _hook;
};
