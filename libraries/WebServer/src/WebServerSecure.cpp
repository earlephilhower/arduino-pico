#include "WebServerSecure.h"

WebServerSecure::WebServerSecure(IPAddress addr, int port) : HTTPServer(), _server(addr, port)
{
  log_v("WebServerSecure::WebServerSecure(addr=%s, port=%d)", addr.toString().c_str(), port);
}
WebServerSecure::WebServerSecure(int port) : HTTPServer(), _server(port)
{
  log_v("WebServerSecure::WebserverSecure(port=%d)", port);
}
WebServerSecure::~WebServerSecure() {
  _server.close();
}

void WebServerSecure::begin() {
  close();
  _server.begin();
  _server.setNoDelay(true);
}

void WebServerSecure::begin(uint16_t port) {
  close();
  _server.begin(port);
  _server.setNoDelay(true);
}
void WebServerSecure::handleClient() {
  if (_currentStatus == HC_NONE) {
    if (_currentClient) {
        delete _currentClient;
        _currentClient = nullptr;
    }

    WiFiClientSecure client = _server.available();
    if (!client) {
      if (_nullDelay) {
        delay(1);
      }
      return;
    }

    log_v("New client: client.localIP()=%s", client.localIP().toString().c_str());

    _currentClient = new WiFiClientSecure(client);
    _currentStatus = HC_WAIT_READ;
    _statusChange = millis();
  }
  httpHandleClient();
}
void WebServerSecure::close() {
  _server.close();
  httpClose();
}

void WebServerSecure::stop() {
  close();
}

