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


/*
    This file is taken from Cygwin distribution. Please keep it in sync.
    The differences should be within __MINGW32__ guard.
*/

// Original code and organization taken from https://mcuoneclipse.com/2015/08/23/tutorial-using-gnu-profiling-gprof-with-arm-cortex-m/

#include <Arduino.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>

// Frequency of sampling
#ifndef GMON_HZ
#define GMON_HZ 10000
#endif

/*
    fraction of text space to allocate for histogram counters here, 1/2
*/
#ifndef HISTFRACTION
#define HISTFRACTION    8
#endif

/*
    Fraction of text space to allocate for from hash buckets.
    The value of HASHFRACTION is based on the minimum number of bytes
    of separation between two subroutine call points in the object code.
    Given MIN_SUBR_SEPARATION bytes of separation the value of
    HASHFRACTION is calculated as:

        HASHFRACTION = MIN_SUBR_SEPARATION / (2 * sizeof(short) - 1);

    For example, on the VAX, the shortest two call sequence is:

        calls   $0,(r0)
        calls   $0,(r0)

    which is separated by only three bytes, thus HASHFRACTION is
    calculated as:

        HASHFRACTION = 3 / (2 * 2 - 1) = 1

    Note that the division above rounds down, thus if MIN_SUBR_FRACTION
    is less than three, this algorithm will not work!

    In practice, however, call instructions are rarely at a minimal
    distance.  Hence, we will define HASHFRACTION to be 2 across all
    architectures.  This saves a reasonable amount of space for
    profiling data structures without (in practice) sacrificing
    any granularity.
*/
#ifndef HASHFRACTION
#define HASHFRACTION    2
#endif

/*
    percent of text space to allocate for tostructs with a minimum.
*/
#ifndef ARCDENSITY
#define ARCDENSITY      2 /* this is in percentage, relative to text size! */
#endif
#define MINARCS          50
#define MAXARCS          ((1 << (8 * sizeof(HISTCOUNTER))) - 2)

/*
    Possible states of profiling.
*/
#define GMON_PROF_ON      0
#define GMON_PROF_BUSY  1
#define GMON_PROF_ERROR 2
#define GMON_PROF_OFF     3

//void _mcleanup(void); /* routine to be called to write gmon.out file */



/*
    Structure prepended to gmon.out profiling data file.
*/
struct gmonhdr {
    size_t  lpc;    /* base pc address of sample buffer */
    size_t  hpc;    /* max pc address of sampled buffer */
    int ncnt;       /* size of sample buffer (plus this header) */
    int version;    /* version number */
    int profrate;   /* profiling clock rate */
    int spare[3];   /* reserved */
};
#define GMONVERSION 0x00051879

/*
    histogram counters are unsigned shorts (according to the kernel).
*/
#define HISTCOUNTER unsigned short

void _monInit(void); /* initialization routine */

// In the original profiler code selfpc and count are full 32 bits each
// so the structure actually comes to 12 bytes due to padding (with 2
// bytes wasted per entry).  We don't have that much to spare on the Picos,
// so limit the recorded address to 16MB (which is the flash address
// window, anyway) and the counts to 16M (saturating). This saves 4 bytes
// (33%) per entry at the cost of some logic to expand/pack it.
struct tostruct {
    uint8_t selfpc[3]; /* callee address/program counter. The caller address is in froms[] array which points to tos[] array */
    uint8_t count[3];    /* how many times it has been called */
    u_short link;   /* link to next entry in hash table. For tos[0] this points to the last used entry */
};
#define SETSELFPC(x, d) do { (x)->selfpc[0] = d & 0xff; (x)->selfpc[1] = (d >> 8) & 0xff; (x)->selfpc[2] = (d >> 16) & 0xff; } while (0)
#define SETCOUNT(x, d) do { (x)->count[0] = d & 0xff; (x)->count[1] = (d >> 8) & 0xff; (x)->count[2] = (d >> 16) & 0xff; } while (0)
#define GETSELFPC(x) (0x10000000 | ((uint32_t)(x)->selfpc[0]) | (((uint32_t)(x)->selfpc[1]) << 8) | (((uint32_t)(x)->selfpc[2]) << 16))
#define GETCOUNT(x) (((uint32_t)(x)->count[0]) | (((uint32_t)(x)->count[1]) << 8) | (((uint32_t)(x)->count[2]) << 16))

/*
    a raw arc, with pointers to the calling site and
    the called site and a count.
*/
struct rawarc {
    size_t  raw_frompc;
    size_t  raw_selfpc;
    long  raw_count;
};

/*
    general rounding functions.
*/
#define ROUNDDOWN(x,y)  (((x)/(y))*(y))
#define ROUNDUP(x,y)    ((((x)+(y)-1)/(y))*(y))

/*
    The profiling data structures are housed in this structure.
*/
struct gmonparam {
    int   state;
    u_short   *kcount;    /* histogram PC sample array */
    size_t    kcountsize; /* size of kcount[] array in bytes */
    u_short   *froms;     /* array of hashed 'from' addresses. The 16bit value is an index into the tos[] array */
    size_t    fromssize;  /* size of froms[] array in bytes */
    struct tostruct *tos; /* to struct, contains histogram counter */
    size_t    tossize;    /* size of tos[] array in bytes */
    long      tolimit;
    size_t    lowpc;      /* low program counter of area */
    size_t    highpc;     /* high program counter */
    size_t    textsize;   /* code size */
};
extern struct gmonparam _gmonparam;


typedef enum {
    PROFILE_NOT_INIT = 0,
    PROFILE_ON,
    PROFILE_OFF
} PROFILE_State;

struct profinfo {
    PROFILE_State state; /* profiling state */
    u_short *counter;     /* profiling counters */
    size_t lowpc, highpc;   /* range to be profiled */
    u_int scale;      /* scale value of bins */
};


/* global profinfo for profil() call */
static struct profinfo prof = {
    PROFILE_NOT_INIT, 0, 0, 0, 0
};
extern volatile bool __otherCoreIdled;

/* convert an addr to an index */
#define PROFIDX(pc, base, scale)  \
  ({                  \
    size_t i = (pc - base) / 2;       \
    if (sizeof (unsigned long long int) > sizeof (size_t))    \
      i = (unsigned long long int) i * scale / 65536;     \
    else                \
      i = i / 65536 * scale + i % 65536 * scale / 65536;    \
    i;                  \
  })

/* convert an index into an address */
#define PROFADDR(idx, base, scale)    \
  ((base)         \
   + ((((unsigned long long)(idx) << 16)  \
       / (unsigned long long)(scale)) << 1))


int profile_ctl(struct profinfo *, char *, size_t, size_t, u_int);
int profil(char *, size_t, size_t, u_int);


/* sample the current program counter */
void __no_inline_not_in_flash_func(_SystickHandler)(void) {
    static size_t pc, idx; // Ensure in heap, not on stack

    if (!__otherCoreIdled && (prof.state == PROFILE_ON)) {
        pc = ((uint32_t*)(__builtin_frame_address(0)))[14]; /* get SP and use it to get the return address from stack */
        if (pc >= prof.lowpc && pc < prof.highpc) {
            idx = PROFIDX(pc, prof.lowpc, prof.scale);
            prof.counter[idx]++;
        }
    }
}

/* Stop profiling to the profiling buffer pointed to by p. */
static int __no_inline_not_in_flash_func(profile_off)(struct profinfo *p) {
    p->state = PROFILE_OFF;
    return 0;
}

/* Create a timer thread and pass it a pointer P to the profiling buffer. */
static int __no_inline_not_in_flash_func(profile_on)(struct profinfo *p) {
    p->state = PROFILE_ON;
    return 0; /* ok */
}

/*
    start or stop profiling

    profiling goes into the SAMPLES buffer of size SIZE (which is treated
    as an array of u_shorts of size size/2)

    each bin represents a range of pc addresses from OFFSET.  The number
    of pc addresses in a bin depends on SCALE.  (A scale of 65536 maps
    each bin to two addresses, A scale of 32768 maps each bin to 4 addresses,
    a scale of 1 maps each bin to 128k address).  Scale may be 1 - 65536,
    or zero to turn off profiling
*/
int __no_inline_not_in_flash_func(profile_ctl)(struct profinfo *p, char *samples, size_t size, size_t offset, u_int scale) {
    size_t maxbin;

    if (scale > 65536) {
        //    errno = EINVAL;
        return -1;
    }
    profile_off(p);
    if (scale) {
        memset(samples, 0, size);
        memset(p, 0, sizeof * p);
        maxbin = size >> 1;
        prof.counter = (u_short*)samples;
        prof.lowpc = offset;
        prof.highpc = PROFADDR(maxbin, offset, scale);
        prof.scale = scale;
        return profile_on(p);
    }
    return 0;
}

/*  Equivalent to unix profil()
    Every SLEEPTIME interval, the user's program counter (PC) is examined:
    offset is subtracted and the result is multiplied by scale.
    The word pointed to by this address is incremented. */
int __no_inline_not_in_flash_func(profil)(char *samples, size_t size, size_t offset, u_int scale) {
    return profile_ctl(&prof, samples, size, offset, scale);
}



#define PICO_RUNTIME_INIT_PROFILING "11011" // Towards the end, after PSRAM
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


#define MINUS_ONE_P (-1)
#define ERR(s) write(2, s, sizeof(s))

struct gmonparam _gmonparam = { GMON_PROF_OFF, NULL, 0, NULL, 0, NULL, 0, 0L, 0, 0, 0};
static char already_setup = 0; /* flag to indicate if we need to init */
static int	s_scale;
/* see profil(2) where this is described (incorrectly) */
#define		SCALE_1_TO_1	0x10000L

static void moncontrol(int mode);

int __profileMemSize = 0;

void __no_inline_not_in_flash_func(monstartup)(size_t lowpc, size_t highpc) {
    register size_t o;
    char *cp;
    struct gmonparam *p = &_gmonparam;

    /*
        round lowpc and highpc to multiples of the density we're using
        so the rest of the scaling (here and in gprof) stays in ints.
    */
    p->lowpc = ROUNDDOWN(lowpc, HISTFRACTION * sizeof(HISTCOUNTER));
    p->highpc = ROUNDUP(highpc, HISTFRACTION * sizeof(HISTCOUNTER));
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
    if (cp == (char *)0) {
        ERR("monstartup: out of memory\n");
        already_setup = 0;
        return;
    }
#else
    cp = malloc(__profileMemSize);
    if (cp == (char *)0) {
        ERR("monstartup: out of memory\n");
        return;
    }
#endif

    /* zero out cp as value will be added there */
    bzero(cp, p->kcountsize + p->fromssize + p->tossize);

    p->tos = (struct tostruct *)cp;
    cp += p->tossize;
    p->kcount = (u_short *)cp;
    cp += p->kcountsize;
    p->froms = (u_short *)cp;

    p->tos[0].link = 0;

    o = p->highpc - p->lowpc;
    if (p->kcountsize < o) {
#ifndef notdef
        s_scale = ((float)p->kcountsize / o) * SCALE_1_TO_1;
#else /* avoid floating point */
        int quot = o / p->kcountsize;

        if (quot >= 0x10000) {
            s_scale = 1;
        } else if (quot >= 0x100) {
            s_scale = 0x10000 / quot;
        } else if (o >= 0x800000) {
            s_scale = 0x1000000 / (o / (p->kcountsize >> 8));
        } else {
            s_scale = 0x1000000 / ((o << 8) / p->kcountsize);
        }
#endif
    } else {
        s_scale = SCALE_1_TO_1;
    }
    moncontrol(1); /* start */
}

#ifndef O_BINARY
#define O_BINARY 0
#endif

// Ensure matches definition in rp2040support
typedef int (*profileWriteCB)(const void *data, int len);
void _writeProfile(profileWriteCB writeCB) {
    int fromindex;
    int endfrom;
    size_t frompc;
    int toindex;
    struct rawarc rawarc;
    const int BS = 64;
    struct rawarc rawarcbuff[BS];
    int rawarcbuffptr = 0;
    struct gmonparam *p = &_gmonparam;
    struct gmonhdr gmonhdr, *hdr;

    moncontrol(0); /* stop */

    hdr = (struct gmonhdr *)&gmonhdr;
    hdr->lpc = p->lowpc;
    hdr->hpc = p->highpc;
    hdr->ncnt = p->kcountsize + sizeof(gmonhdr);
    hdr->version = GMONVERSION;
    hdr->profrate = GMON_HZ;
    writeCB((void *)hdr, sizeof * hdr);
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
            rawarc.raw_selfpc = GETSELFPC(&p->tos[toindex]);
            rawarc.raw_count = GETCOUNT(&p->tos[toindex]);
            // Buffer up writes because Semihosting is really slow per write call
            rawarcbuff[rawarcbuffptr++] = rawarc;
            if (rawarcbuffptr == BS) {
                writeCB((void *)rawarcbuff, BS * sizeof(struct rawarc));
                rawarcbuffptr = 0;
            }
        }
    }
    if (rawarcbuffptr) {
        writeCB((void *)rawarcbuff, rawarcbuffptr * sizeof(struct rawarc));
    }
}

/*
    Control profiling
 	profiling is what mcount checks to see if
 	all the data structures are ready.
*/
static void __no_inline_not_in_flash_func(moncontrol)(int mode) {
    struct gmonparam *p = &_gmonparam;

    if (mode) {
        /* start */
        profil((char *)p->kcount, p->kcountsize, p->lowpc, s_scale);
        p->state = GMON_PROF_ON;
    } else {
        /* stop */
        profil((char *)0, 0, 0, 0);
        p->state = GMON_PROF_OFF;
    }
}
bool _perf_in_setup = false;
void __no_inline_not_in_flash_func(_mcount_internal)(uint32_t *frompcindex, uint32_t *selfpc) {
    register struct tostruct	*top;
    register struct tostruct	*prevtop;
    register long			toindex;
    struct gmonparam *p = &_gmonparam;
    if (_perf_in_setup) {
        return;
    }

    if (!already_setup) {
        extern char __etext; /* end of text/code symbol, defined by linker */
        already_setup = 1;
        _perf_in_setup = true;
        monstartup(0x10000000, (uint32_t)&__etext);
        _perf_in_setup = false;
    }
    /*
     	check that we are profiling
     	and that we aren't recursively invoked.
    */
    if (p->state != GMON_PROF_ON) {
        goto out;
    }
    p->state++;
    /*
     	check that frompcindex is a reasonable pc value.
     	for example:	signal catchers get called from the stack,
     			not from text space.  too bad.
    */
    //goto out;
    frompcindex = (uint32_t*)((long)frompcindex - (long)p->lowpc);
    if ((unsigned long)frompcindex > p->textsize) {
        goto done;
    }
    frompcindex = (uint32_t*)&p->froms[((long)frompcindex) / (HASHFRACTION * sizeof(*p->froms))];
    toindex = *((u_short*)frompcindex); /* get froms[] value */
    if (toindex == 0) {
        /*
        	first time traversing this arc
        */
        toindex = ++p->tos[0].link; /* the link of tos[0] points to the last used record in the array */
        if (toindex >= p->tolimit) { /* more tos[] entries than we can handle! */
            goto overflow;
        }
        *((u_short*)frompcindex) = (u_short)toindex; /* store new 'to' value into froms[] */
        top = &p->tos[toindex];
        SETSELFPC(top, (uint32_t)selfpc); //top->selfpc = (size_t)selfpc;
        SETCOUNT(top, 1); //top->count = 1;
        top->link = 0;
        goto done;
    }
    top = &p->tos[toindex];
    if (GETSELFPC(top)/*->selfpc*/ == (size_t)selfpc) {
        /*
         	arc at front of chain; usual case.
        */
        uint32_t cnt = GETCOUNT(top) + 1;//->count++;
        if (cnt >= 1 << 24) {
            cnt = 1 << 24;
        }
        SETCOUNT(top, cnt);//top->count++;
        goto done;
    }
    /*
     	have to go looking down chain for it.
     	top points to what we are looking at,
     	prevtop points to previous top.
     	we know it is not at the head of the chain.
    */
    for (; /* goto done */;) {
        if (top->link == 0) {
            /*
             	top is end of the chain and none of the chain
             	had top->selfpc == selfpc.
             	so we allocate a new tostruct
             	and link it to the head of the chain.
            */
            toindex = ++p->tos[0].link;
            if (toindex >= p->tolimit) {
                goto overflow;
            }
            top = &p->tos[toindex];
            SETSELFPC(top, (uint32_t)selfpc);
            SETCOUNT(top, 1); //->count = 1;
            top->link = *((u_short*)frompcindex);
            *(u_short*)frompcindex = (u_short)toindex;
            goto done;
        }
        /*
         	otherwise, check the next arc on the chain.
        */
        prevtop = top;
        top = &p->tos[top->link];
        if (GETSELFPC(top)/*->selfpc*/ == (size_t)selfpc) {
            /*
             	there it is.
             	increment its count
             	move it to the head of the chain.
            */
            uint32_t cnt = GETCOUNT(top) + 1;//->count++;
            if (cnt >= 1 << 24) {
                cnt = 1 << 24;
            }
            SETCOUNT(top, cnt);
            toindex = prevtop->link;
            prevtop->link = top->link;
            top->link = *((u_short*)frompcindex);
            *((u_short*)frompcindex) = (u_short)toindex;
            goto done;
        }
    }
done:
    p->state--;
    /* and fall through */
out:
    return;		/* normal return restores saved registers */
overflow:
    p->state++; /* halt further profiling */
#define	TOLIMIT	"mcount: tos overflow\n"
    write(2, TOLIMIT, sizeof(TOLIMIT));
    goto out;
}

void __no_inline_not_in_flash_func(_monInit)(void) {
    _gmonparam.state = GMON_PROF_OFF;
    already_setup = 0;
}
