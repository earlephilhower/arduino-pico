#!/bin/bash
for i in pbuf raw tcp udp netif dns dhcp igmp mld6; do
    ../../../tools/makewrapper.py  -s ../../../pico-sdk/lib/lwip/src/include/lwip/$i.h -p wrap_$i
done
for i in mdns sntp; do
    ../../../tools/makewrapper.py  -s ../../../pico-sdk/lib/lwip/src/include/lwip/apps/$i.h -p wrap_$i
done
../../../tools/makewrapper.py -s ../../../pico-sdk/lib/lwip/src/include/lwip/timeouts.h -p wrap_timeouts
../../../tools/makewrapper.py -s ../../../pico-sdk/lib/lwip/src/include/netif/ethernet.h -p wrap_ethernet
