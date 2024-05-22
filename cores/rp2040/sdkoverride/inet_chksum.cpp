// Taken from LWIP's inet_chksum.c just to set the -O2 flag
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

