// Use to create a callback thunk from C to C++
// #define CCALLBACKNAME to a unique per-file name and #include this file
// To make a CB use a define of the form:
/*
    #define PACKETHANDLERCB(class, cbFcn) \
    (CCALLBACKNAME<void(uint8_t, uint16_t, uint8_t*, uint16_t), __COUNTER__>::func = std::bind(&class::cbFcn, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4), \
    static_cast<btstack_packet_handler_t>(<CCALLBACKNAMEvoid(uint8_t, uint16_t, uint8_t*, uint16_t), __COUNTER__ - 1>::callback))
*/

#include <functional>

// Thank you to https://stackoverflow.com/questions/66474621/multiple-non-static-callbacks-of-c-member-functions for the following beautiful hack

#ifndef CCALLBACKNAME
#define CCALLBACKNAME _CCallback
#endif

template <typename T, int tag>
struct CCALLBACKNAME;

template <typename Ret, typename... Params, int tag>
struct CCALLBACKNAME<Ret(Params...), tag> {
    template <typename... Args>
    static Ret callback(Args... args) {
        return func(args...);
    }
    int _tag = tag;
    static std::function<Ret(Params...)> func;
};

template <typename Ret, typename... Params, int tag>
std::function<Ret(Params...)> CCALLBACKNAME<Ret(Params...), tag>::func;
