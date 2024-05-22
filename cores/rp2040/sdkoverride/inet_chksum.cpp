// Taken from LWIP's inet_chksum.c just to set the -O2 flag

/*
    Copyright (c) 2001-2004 Swedish Institute of Computer Science.
    All rights reserved.

    Redistribution and use in source and binary forms, with or without modification,
    are permitted provided that the following conditions are met:

    1. Redistributions of source code must retain the above copyright notice,
      this list of conditions and the following disclaimer.
    2. Redistributions in binary form must reproduce the above copyright notice,
      this list of conditions and the following disclaimer in the documentation
      and/or other materials provided with the distribution.
    3. The name of the author may not be used to endorse or promote products
      derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
    WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
    MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
    SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
    EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
    OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
    INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
    CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
    IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
    OF SUCH DAMAGE.

    This file is part of the lwIP TCP/IP stack.

    Author: Adam Dunkels <adam@sics.se>

*/



#include "lwip/opt.h"

#include "lwip/inet_chksum.h"
#include "lwip/def.h"
#include "lwip/ip_addr.h"

#pragma GCC optimize ("O2")
/**
    An optimized checksum routine. Basically, it uses loop-unrolling on
    the checksum loop, treating the head and tail bytes specially, whereas
    the inner loop acts on 8 bytes at a time.

    @arg start of buffer to be checksummed. May be an odd byte address.
    @len number of bytes in the buffer to be checksummed.
    @return host order (!) lwip checksum (non-inverted Internet sum)

    by Curt McDowell, Broadcom Corp. December 8th, 2005
*/
extern "C" u16_t lwip_standard_chksum(const void *dataptr, int len) {
    const u8_t *pb = (const u8_t *)dataptr;
    const u16_t *ps;
    u16_t t = 0;
    const u32_t *pl;
    u32_t sum = 0, tmp;
    /* starts at odd byte address? */
    int odd = ((mem_ptr_t)pb & 1);

    if (odd && len > 0) {
        ((u8_t *)&t)[1] = *pb++;
        len--;
    }

    ps = (const u16_t *)(const void *)pb;

    if (((mem_ptr_t)ps & 3) && len > 1) {
        sum += *ps++;
        len -= 2;
    }

    pl = (const u32_t *)(const void *)ps;

    while (len > 7)  {
        tmp = sum + *pl++;          /* ping */
        if (tmp < sum) {
            tmp++;                    /* add back carry */
        }

        sum = tmp + *pl++;          /* pong */
        if (sum < tmp) {
            sum++;                    /* add back carry */
        }

        len -= 8;
    }

    /* make room in upper bits */
    sum = FOLD_U32T(sum);

    ps = (const u16_t *)pl;

    /* 16-bit aligned word remaining? */
    while (len > 1) {
        sum += *ps++;
        len -= 2;
    }

    /* dangling tail byte remaining? */
    if (len > 0) {                /* include odd byte */
        ((u8_t *)&t)[0] = *(const u8_t *)ps;
    }

    sum += t;                     /* add end bytes */

    /*  Fold 32-bit sum to 16 bits
        calling this twice is probably faster than if statements... */
    sum = FOLD_U32T(sum);
    sum = FOLD_U32T(sum);

    if (odd) {
        sum = SWAP_BYTES_IN_WORD(sum);
    }

    return (u16_t)sum;
}

