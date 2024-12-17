/*  -
    Copyright (c) 1983, 1992, 1993
 	The Regents of the University of California.  All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions
    are met:
    1. Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    2. Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    4. Neither the name of the University nor the names of its contributors
      may be used to endorse or promote products derived from this software
      without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
    IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
    ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
    FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
    DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
    OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
    HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
    LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
    OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
    SUCH DAMAGE.
*/

// This code is built as a C file because otherwise G++ would add profiling
// code to the preamble of these functions as well, leading to an infinite
// loop in the mcount routine.  Because the Arduino IDE can't (easily)
// apply different compile parameters to different files, we set all C++
// files to "-pg" but leave all C files uninstrumented.

// Original code and organization taken from https://mcuoneclipse.com/2015/08/23/tutorial-using-gnu-profiling-gprof-with-arm-cortex-m/

#include <Arduino.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

// Frequency of sampling PC
#ifndef GMON_HZ
#define GMON_HZ 10000
#endif

// Fraction of text space to allocate for histogram counters here, 1/2
#ifndef HISTFRACTION
#ifdef PICO_RP2350
#define HISTFRACTION    4 // Every 8 bytes of .text
#else
#define HISTFRACTION    8 // Every 16 bytes of .text
#endif
#endif

// Fraction of text space to allocate for from hash buckets.
// The value of HASHFRACTION is based on the minimum number of bytes
// of separation between two subroutine call points in the object code.
// Given MIN_SUBR_SEPARATION bytes of separation the value of
// HASHFRACTION is calculated as:
//
//     HASHFRACTION = MIN_SUBR_SEPARATION / (2 * sizeof(short) - 1);
//
// For example, on the VAX, the shortest two call sequence is:
//
//     calls   $0,(r0)
//     calls   $0,(r0)
//
// which is separated by only three bytes, thus HASHFRACTION is
// calculated as:
//
//     HASHFRACTION = 3 / (2 * 2 - 1) = 1
//
// Note that the division above rounds down, thus if MIN_SUBR_FRACTION
// is less than three, this algorithm will not work!
//
// In practice, however, call instructions are rarely at a minimal
// distance.  Hence, we will define HASHFRACTION to be 2 across all
// architectures.  This saves a reasonable amount of space for
// profiling data structures without (in practice) sacrificing
// any granularity.
#ifndef HASHFRACTION
#define HASHFRACTION    2
#endif

// Percent of text space to allocate for tostructs with a minimum.
#ifndef ARCDENSITY
#define ARCDENSITY      2 // This is in percentage, relative to text size!
#endif
#define MINARCS          50
#define MAXARCS          ((1 << (8 * sizeof(HISTCOUNTER))) - 2)



// Histogram counters are unsigned shorts (according to the kernel)
typedef uint16_t HISTCOUNTER; //#define HISTCOUNTER unsigned short

// In the original profiler code selfpc and count are full 32 bits each
// so the structure actually comes to 12 bytes due to padding (with 2
// bytes wasted per entry).  We don't have that much to spare on the Picos,
// so limit the recorded address to 16MB (which is the flash address
// window, anyway) and the counts to 16M (saturating). This saves 4 bytes
// (33%) per entry at the cost of some logic to expand/pack it.
struct tostruct {
    uint8_t selfpc[3]; // Callee address/program counter. The caller address is in froms[] array which points to tos[] array
    uint8_t count[3];  // How many times it has been called
    uint16_t link;     // Link to next entry in hash table. For tos[0] this points to the last used entry
};


typedef enum { PROFILE_NOT_INIT = 0, PROFILE_ON, PROFILE_OFF } PROFILE_State;
struct profinfo {
    PROFILE_State state;  // Profiling state
    uint16_t *counter;    // Profiling counters
    size_t lowpc, highpc; // Range to be profiled
    uint32_t scale;       // Scale value of bins
};
// Global profinfo for profil() call
static struct profinfo prof = { PROFILE_NOT_INIT, 0, 0, 0, 0 };


// Possible states of profiling
typedef enum { GMON_PROF_ON = 0, GMON_PROF_BUSY, GMON_PROF_ERROR, GMON_PROF_OFF } GMON_State;

// The profiling data structures are housed in this structure.
struct gmonparam {
    int   state;
    uint16_t   *kcount;   // Histogram PC sample array
    size_t    kcountsize; // Size of kcount[] array in bytes
    uint16_t   *froms;    // Array of hashed 'from' addresses. The 16bit value is an index into the tos[] array
    size_t    fromssize;  // Size of froms[] array in bytes
    struct tostruct *tos; // to struct, contains histogram counter
    size_t    tossize;    // Size of tos[] array in bytes
    long      tolimit;
    size_t    lowpc;      // Low program counter of area
    size_t    highpc;     // High program counter
    size_t    textsize;   // Code size
};
static struct gmonparam _gmonparam = { GMON_PROF_OFF, NULL, 0, NULL, 0, NULL, 0, 0L, 0, 0, 0};


static bool already_setup = false;  // Flag to indicate if we need to init
static bool _perf_in_setup = false; // Are we currently trying to initialize? (avoid infinite recursion)
int __profileMemSize = 0;           // Memory allocated by the profiler to store tables

static int    s_scale = 0;
#define SCALE_1_TO_1 0x10000L


// Convert an addr to an index
static inline __attribute__((always_inline)) size_t profidx(size_t pc, size_t base, size_t scale) {
    size_t i = (pc - base) / 2;
    return (unsigned long long int) i * scale / 65536;
}

// Sample the current program counter periodically
#if defined(__riscv)
// TODO - systick-like handler
#else
static void __no_inline_not_in_flash_func(_SystickHandler)(void) {
    static size_t pc, idx; // Ensure in heap, not on stack
    extern volatile bool __otherCoreIdled;

    if (!__otherCoreIdled && (prof.state == PROFILE_ON)) {
        pc = ((uint32_t*)(__builtin_frame_address(0)))[14];  // Get SP and use it to get the return address from stack
        if ((pc >= prof.lowpc) && (pc < prof.highpc)) {
            idx = profidx(pc, prof.lowpc, prof.scale);
            prof.counter[idx]++;
        }
    }
}
#endif

// Convert an index into an address
static inline __attribute__((always_inline)) size_t profaddr(size_t idx, size_t base, size_t scale) {
    return base + ((((unsigned long long)(idx) << 16) / (unsigned long long)(scale)) << 1);
}

// Start or stop profiling
// Profiling goes into the SAMPLES buffer of size SIZE (which is treated as an array of uint16_ts of size size/2).
// Each bin represents a range of pc addresses from OFFSET.  The number of pc addresses in a bin depends on SCALE.
// (A scale of 65536 maps each bin to two addresses, A scale of 32768 maps each bin to 4 addresses, a scale of
// 1 maps each bin to 128k address).  Scale may be 1 - 65536, or zero to turn off profiling
static int __no_inline_not_in_flash_func(profile_ctl)(char *samples, size_t size, size_t offset, uint32_t scale) {
    size_t maxbin;

    if (scale > 65536) {
        return -1;
    }
    prof.state = PROFILE_OFF;
    if (scale) {
        bzero(samples, size);
        bzero(&prof, sizeof(prof));
        maxbin = size >> 1;
        prof.counter = (uint16_t*)samples;
        prof.lowpc = offset;
        prof.highpc = profaddr(maxbin, offset, scale);
        prof.scale = scale;
        prof.state = PROFILE_ON;
    }
    return 0;
}

// Control profiling. Profiling is what mcount checks to see if all the data structures are ready.
static void __no_inline_not_in_flash_func(moncontrol)(int mode) {
    if (mode) { // Start
        profile_ctl((char *)_gmonparam.kcount, _gmonparam.kcountsize, _gmonparam.lowpc, s_scale);
        _gmonparam.state = GMON_PROF_ON;
    } else { // Stop
        profile_ctl((char *)NULL, 0, 0, 0);
        _gmonparam.state = GMON_PROF_OFF;
    }
}

// General rounding functions
static inline __attribute__((always_inline)) size_t rounddown(size_t x, size_t y) {
    return (x / y) * y;
}

static inline __attribute__((always_inline)) size_t roundup(size_t x, size_t y) {
    return ((x + y - 1) / y) * y;
}

// Allocate memory and set boundaries before any sampling is performed
void __no_inline_not_in_flash_func(monstartup)(size_t lowpc, size_t highpc) {
    register size_t o;
    char *cp;
    struct gmonparam *p = &_gmonparam;

    // Round lowpc and highpc to multiples of the density we're using so the rest of the scaling (here and in gprof) stays in ints.
    p->lowpc = rounddown(lowpc, HISTFRACTION * sizeof(HISTCOUNTER));
    p->highpc = roundup(highpc, HISTFRACTION * sizeof(HISTCOUNTER));
    p->textsize = p->highpc - p->lowpc;
    p->kcountsize = p->textsize / HISTFRACTION;
    p->fromssize = p->textsize / HASHFRACTION;
    p->tolimit = p->textsize * ARCDENSITY / 100;
    if (p->tolimit < MINARCS) {
        p->tolimit = MINARCS;
    } else if (p->tolimit > MAXARCS) {
        p->tolimit = MAXARCS;
    }
    p->tossize = p->tolimit * sizeof(struct tostruct);
    __profileMemSize = p->kcountsize + p->fromssize + p->tossize;
#ifdef RP2350_PSRAM_CS
    cp = pmalloc(__profileMemSize);
#else
    cp = malloc(__profileMemSize);
#endif
    if (cp == NULL) {
        // OOM
        already_setup = false;
        return;
    }

    // Zero out cp as value will be added there
    bzero(cp, p->kcountsize + p->fromssize + p->tossize);

    p->tos = (struct tostruct *)cp;
    cp += p->tossize;
    p->kcount = (uint16_t *)cp;
    cp += p->kcountsize;
    p->froms = (uint16_t *)cp;

    p->tos[0].link = 0;

    o = p->highpc - p->lowpc;
    if (p->kcountsize < o) {
        s_scale = ((float)p->kcountsize / o) * SCALE_1_TO_1;
    } else {
        s_scale = SCALE_1_TO_1;
    }
    moncontrol(1); // Start
}

// Accessors for the selfpc and count fields
static inline __attribute__((always_inline)) void setselfpc(struct tostruct *x, size_t d) {
    x->selfpc[0] = d & 0xff;
    x->selfpc[1] = (d >> 8) & 0xff;
    x->selfpc[2] = (d >> 16) & 0xff;
}

static inline __attribute__((always_inline))void setcount(struct tostruct *x, size_t d) {
    x->count[0] = d & 0xff;
    x->count[1] = (d >> 8) & 0xff;
    x->count[2] = (d >> 16) & 0xff;
}

static inline __attribute__((always_inline)) uint32_t getselfpc(const struct tostruct *x) {
    return 0x10000000 | ((uint32_t)x->selfpc[0]) | (((uint32_t)x->selfpc[1]) << 8) | (((uint32_t)x->selfpc[2]) << 16);
}

static inline __attribute__((always_inline)) uint32_t getcount(const struct tostruct *x) {
    return ((uint32_t)x->count[0]) | (((uint32_t)x->count[1]) << 8) | (((uint32_t)x->count[2]) << 16);
}

// Called by the GCC function shim (gprof_shim.S) on function entry to record an arc hit
void __no_inline_not_in_flash_func(_mcount_internal)(uint32_t *frompcindex, uint32_t *selfpc) {
    register struct tostruct *top;
    register struct tostruct *prevtop;
    register long            toindex;
    struct gmonparam *p = &_gmonparam;

    if (_perf_in_setup) {
        // Avoid infinite recursion
        return;
    }

    if (!already_setup) {
        extern char __flash_binary_start; // Start of flash
        extern char __etext;              // End of .text
        already_setup = true;
        _perf_in_setup = true;
        monstartup((uint32_t)&__flash_binary_start, (uint32_t)&__etext);
        _perf_in_setup = false;
    }
    // Check that we are profiling and that we aren't recursively invoked.
    if (p->state != GMON_PROF_ON) {
        return;
    }
    p->state++;
    // Check that frompcindex is a reasonable pc value.
    frompcindex = (uint32_t*)((long)frompcindex - (long)p->lowpc);
    if ((unsigned long)frompcindex > p->textsize) {
        goto done;
    }
    frompcindex = (uint32_t*)&p->froms[((long)frompcindex) / (HASHFRACTION * sizeof(*p->froms))];
    toindex = *((uint16_t*)frompcindex); // Get froms[] value
    if (toindex == 0) {
        // First time traversing this arc
        toindex = ++p->tos[0].link;  // The link of tos[0] points to the last used record in the array
        if (toindex >= p->tolimit) { // More tos[] entries than we can handle!
            goto overflow;
        }
        *((uint16_t*)frompcindex) = (uint16_t)toindex; // Store new 'to' value into froms[]
        top = &p->tos[toindex];
        setselfpc(top, (uint32_t)selfpc);
        setcount(top, 1);
        top->link = 0;
        goto done;
    }
    top = &p->tos[toindex];
    if (getselfpc(top) == (size_t)selfpc) {
        // Arc at front of chain; usual case.
        uint32_t cnt = getcount(top) + 1;
        if (cnt >= 1 << 24) {
            cnt = (1 << 24) - 1;
        }
        setcount(top, cnt);
        goto done;
    }
    // Have to go looking down chain for it. top points to what we are looking at, prevtop points to previous top. We know it is not at the head of the chain.
    for (; /* goto done */;) {
        if (top->link == 0) {
            // top is end of the chain and none of the chain had top->selfpc == selfpc, so we allocate a new tostruct and link it to the head of the chain.
            toindex = ++p->tos[0].link;
            if (toindex >= p->tolimit) {
                goto overflow;
            }
            top = &p->tos[toindex];
            setselfpc(top, (uint32_t)selfpc);
            setcount(top, 1);
            top->link = *((uint16_t*)frompcindex);
            *(uint16_t*)frompcindex = (uint16_t)toindex;
            goto done;
        }
        // Otherwise, check the next arc on the chain.
        prevtop = top;
        top = &p->tos[top->link];
        if (getselfpc(top) == (size_t)selfpc) {
            // Increment its count, move it to the head of the chain.
            uint32_t cnt = getcount(top) + 1;
            if (cnt >= 1 << 24) {
                cnt = (1 << 24) - 1;
            }
            setcount(top, cnt);
            toindex = prevtop->link;
            prevtop->link = top->link;
            top->link = *((uint16_t*)frompcindex);
            *((uint16_t*)frompcindex) = (uint16_t)toindex;
            goto done;
        }
    }
done:
    p->state--;
    return;

overflow:
    p->state++; // Halt further profiling
    return;
}


// Write out the GMON.OUT file using internal state
void _writeProfile(int (*writeCB)(const void *data, int len)) {
    struct gmonhdr {  // GMON.OUT header
        size_t lpc;   // base pc address of sample buffer
        size_t hpc;   // max pc address of sampled buffer
        int ncnt;     // size of sample buffer (plus this header)
        int version;  // version number
        int profrate; // profiling clock rate
        int spare[3]; // reserved
    };
    const unsigned int GMONVERSION = 0x00051879;
    struct rawarc {     // Per-arc on-disk data format
        size_t raw_frompc;
        size_t raw_selfpc;
        long   raw_count;
    };
    int fromindex;
    int endfrom;
    size_t frompc;
    int toindex;
    struct rawarc rawarc;
    const int BS = 64;
    struct rawarc rawarcbuff[BS];
    int rawarcbuffptr = 0;
    struct gmonparam *p = &_gmonparam;
    struct gmonhdr hdr;

    moncontrol(0); // Stop

    hdr.lpc = p->lowpc;
    hdr.hpc = p->highpc;
    hdr.ncnt = p->kcountsize + sizeof(hdr);
    hdr.version = GMONVERSION;
    hdr.profrate = GMON_HZ;
    writeCB((void *)&hdr, sizeof(hdr));
    writeCB((void *)p->kcount, p->kcountsize);
    endfrom = p->fromssize / sizeof(*p->froms);
    for (fromindex = 0; fromindex < endfrom; fromindex++) {
        if (p->froms[fromindex] == 0) {
            continue;
        }
        frompc = p->lowpc;
        frompc += fromindex * HASHFRACTION * sizeof(*p->froms);
        for (toindex = p->froms[fromindex]; toindex != 0; toindex = p->tos[toindex].link) {
            rawarc.raw_frompc = frompc;
            rawarc.raw_selfpc = getselfpc(&p->tos[toindex]);
            rawarc.raw_count = getcount(&p->tos[toindex]);
            // Buffer up writes because Semihosting is really slow per write call
            rawarcbuff[rawarcbuffptr++] = rawarc;
            if (rawarcbuffptr == BS) {
                writeCB((void *)rawarcbuff, BS * sizeof(struct rawarc));
                rawarcbuffptr = 0;
            }
        }
    }
    // Write any remaining bits
    if (rawarcbuffptr) {
        writeCB((void *)rawarcbuff, rawarcbuffptr * sizeof(struct rawarc));
    }
}


// These are referenced by RP2040Support.cpp and called by the runtime init SDK
// Install a periodic PC sampler at the specified frequency
#if defined(__riscv)
void runtime_init_setup_profiling() {
    // TODO - is there an equivalent?  Or do we need to build a timer IRQ here?
}
#else
#include <hardware/exception.h>
#include <hardware/structs/systick.h>
void runtime_init_setup_profiling() {
    exception_set_exclusive_handler(SYSTICK_EXCEPTION, _SystickHandler);
    systick_hw->csr = 0x7;
    systick_hw->rvr = (F_CPU / GMON_HZ) - 1;
}
#endif
