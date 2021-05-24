/*
    stdlib_noniso.h - nonstandard (but useful) conversion functions

    Copyright (c) 2014 Ivan Grokhotkov. All rights reserved.
    Copyright (c) 2021 David Gauchard. All rights reserved.
    This file is part of the esp8266 core for Arduino environment.

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
*/

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <math.h>
#include <limits>
#include "stdlib_noniso.h"

// ulltoa() is slower than std::to_char() (1.6 times)
// but is smaller by ~800B/flash and ~250B/rodata

// ulltoa fills str backwards and can return a pointer different from str
extern "C" char* ulltoa(unsigned long long val, char* str, int slen, unsigned int radix) {
    str += --slen;
    *str = 0;
    do {
        auto mod = val % radix;
        val /= radix;
        *--str = mod + ((mod > 9) ? ('a' - 10) : '0');
    } while (--slen && val);
    return val ? nullptr : str;
}

// lltoa fills str backwards and can return a pointer different from str
extern "C" char* lltoa(long long val, char* str, int slen, unsigned int radix) {
    bool neg;
    if (val < 0) {
        val = -val;
        neg = true;
    } else {
        neg = false;
    }
    char* ret = ulltoa(val, str, slen, radix);
    if (neg) {
        if (ret == str || ret == nullptr) {
            return nullptr;
        }
        *--ret = '-';
    }
    return ret;
}

extern "C" char* ltoa(long value, char* result, int base) {
    return itoa((int)value, result, base);
}

extern "C" char* ultoa(unsigned long value, char* result, int base) {
    return utoa((unsigned int)value, result, base);
}

extern "C" char * dtostrf(double number, signed char width, unsigned char prec, char *s) {
    bool negative = false;

    if (isnan(number)) {
        strcpy(s, "nan");
        return s;
    }
    if (isinf(number)) {
        strcpy(s, "inf");
        return s;
    }

    char* out = s;

    int fillme = width; // how many cells to fill for the integer part
    if (prec > 0) {
        fillme -= (prec + 1);
    }

    // Handle negative numbers
    if (number < 0.0) {
        negative = true;
        fillme--;
        number = -number;
    }

    // Round correctly so that print(1.999, 2) prints as "2.00"
    // I optimized out most of the divisions
    double rounding = 2.0;
    for (uint8_t i = 0; i < prec; ++i) {
        rounding *= 10.0;
    }
    rounding = 1.0 / rounding;

    number += rounding;

    // Figure out how big our number really is
    double tenpow = 1.0;
    int digitcount = 1;
    double nextpow;
    while (number >= (nextpow = (10.0 * tenpow))) {
        tenpow = nextpow;
        digitcount++;
    }

    // minimal compensation for possible lack of precision (#7087 addition)
    number *= 1 + std::numeric_limits<decltype(number)>::epsilon();

    number /= tenpow;
    fillme -= digitcount;

    // Pad unused cells with spaces
    while (fillme-- > 0) {
        *out++ = ' ';
    }

    // Handle negative sign
    if (negative) {
        *out++ = '-';
    }

    // Print the digits, and if necessary, the decimal point
    digitcount += prec;
    int8_t digit = 0;
    while (digitcount-- > 0) {
        digit = (int8_t)number;
        if (digit > 9) {
            digit = 9;    // insurance
        }
        *out++ = (char)('0' | digit);
        if ((digitcount == prec) && (prec > 0)) {
            *out++ = '.';
        }
        number -= digit;
        number *= 10.0;
    }

    // make sure the string is terminated
    *out = 0;
    return s;
}

/*
    strrstr (static)

    Backwards search for p_pcPattern in p_pcString
    Based on: https://stackoverflow.com/a/1634398/2778898

*/
extern "C" const char* strrstr(const char*__restrict p_pcString,
                               const char*__restrict p_pcPattern) {
    const char* pcResult = 0;

    size_t      stStringLength = (p_pcString ? strlen(p_pcString) : 0);
    size_t      stPatternLength = (p_pcPattern ? strlen(p_pcPattern) : 0);

    if ((stStringLength) &&
            (stPatternLength) &&
            (stPatternLength <= stStringLength)) {
        // Pattern is shorter or has the same length than the string
        for (const char* s = (p_pcString + stStringLength - stPatternLength); s >= p_pcString; --s) {
            if (0 == strncmp(s, p_pcPattern, stPatternLength)) {
                pcResult = s;
                break;
            }
        }
    }
    return pcResult;
}
