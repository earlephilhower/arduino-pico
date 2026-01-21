#pragma once
#include <stdint.h>
#define CCALLBACKNAME _BLEGATTCB
#include <ctocppcallback.h>

#define GATTPACKETHANDLERCB(class, cbFcn) \
  (_BLEGATTCB<void(uint8_t, uint16_t, uint8_t *, uint16_t), __COUNTER__>::func = std::bind(&class ::cbFcn, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4), \
   static_cast<btstack_packet_handler_t>(_BLEGATTCB<void(uint8_t, uint16_t, uint8_t *, uint16_t), __COUNTER__ - 1>::callback))

#undef CCALLBACKNAME
