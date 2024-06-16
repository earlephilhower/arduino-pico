#pragma once

#include <vector>
#include <assert.h>

class RequestHandler {
public:
    virtual ~RequestHandler() { }

    /*
        note: old handler API for backward compatibility
    */

    virtual bool canHandle(HTTPMethod method, String uri) {
        (void)method;
        (void)uri;
        return false;
    }
    virtual bool canUpload(String uri) {
        (void)uri;
        return false;
    }
    virtual bool canRaw(String uri) {
        (void)uri;
        return false;
    }

    /*
        note: new handler API with support for filters etc.
    */

    virtual bool canHandle(HTTPServer &server, HTTPMethod method, String uri) {
        (void)server;
        (void)method;
        (void)uri;
        return false;
    }
    virtual bool canUpload(HTTPServer &server, String uri) {
        (void)server;
        (void)uri;
        return false;
    }
    virtual bool canRaw(HTTPServer &server, String uri) {
        (void)server;
        (void)uri;
        return false;
    }
    virtual bool handle(HTTPServer& server, HTTPMethod requestMethod, String requestUri) {
        (void) server;
        (void) requestMethod;
        (void) requestUri;
        return false;
    }
    virtual void upload(HTTPServer& server, String requestUri, HTTPUpload& upload) {
        (void) server;
        (void) requestUri;
        (void) upload;
    }
    virtual void raw(HTTPServer& server, String requestUri, HTTPRaw& raw) {
        (void) server;
        (void) requestUri;
        (void) raw;
    }

    RequestHandler* next() {
        return _next;
    }
    void next(RequestHandler* r) {
        _next = r;
    }

    virtual RequestHandler& setFilter(std::function<bool(HTTPServer&)> filter) {
        (void)filter;
        return *this;
    }

private:
    RequestHandler* _next = nullptr;

protected:
    std::vector<String> pathArgs;

public:
    const String& pathArg(unsigned int i) {
        assert(i < pathArgs.size());
        return pathArgs[i];
    }
};
