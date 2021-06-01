#ifndef __COMPAT_H__
#define __COMPAT_H__

namespace arduino {

inline void pinMode(pin_size_t pinNumber, int mode) {
	pinMode(pinNumber, (PinMode)mode);
};

inline void digitalWrite(pin_size_t pinNumber, int status) {
	digitalWrite(pinNumber, (PinStatus)status);
};

}

#endif