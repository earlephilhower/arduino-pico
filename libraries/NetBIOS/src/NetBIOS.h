//

#pragma once

#include <Arduino.h>
#include <AsyncUDP.h>

class NetBIOS {
protected:
    AsyncUDP _udp;
    String _name;
    void _onPacket(AsyncUDPPacket &packet);

public:
    NetBIOS();
    ~NetBIOS();
    bool begin(const char *name);
    void end();
};

extern NetBIOS NBNS;
