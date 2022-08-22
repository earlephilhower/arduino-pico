#pragma once

#include "WebServer.h"

class WebServerSecure : public HTTPServer
{
public:
  WebServerSecure(IPAddress addr, int port = 443);
  WebServerSecure(int port = 443);
  virtual ~WebServerSecure();

  virtual void begin();
  virtual void begin(uint16_t port);
  virtual void handleClient();

  virtual void close();
  virtual void stop();
  WiFiServerSecure *getServer() {
      return &_server;
  }

private:
  WiFiServerSecure _server;
  WiFiClientSecure _curClient;
};
