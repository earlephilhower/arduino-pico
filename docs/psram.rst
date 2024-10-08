RP2350 PSRAM Support
====================

The RP2350 chip in the Raspberry Pi Pico 2, and other RP2350 boards,
supports an external interface to PSRAM.  When a PSRAM chip is attached
to the processor (please note that there is none on the Pico 2 board, but
iLabs and SparkFun boards, among others, do have it), up to 16 megabytes
of additional memory can be used by the chip.

While this external RAM is slower than the built-in SRAM, it is still
able to be used in any place where normal RAM would be used (other than
for memory-mapped functions and statically initialized variables).

When present, PSRAM can be used in two ways: for specific instantiated
variables, or through a ``malloc``-like access method.  Both can be used
in any single application.

Using PSRAM for regular variables
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Similar to ``PROGMEM`` in the original Arduino AVR devices, the variable
decorator ``PSRAM`` can be added to map a variable into PSRAM.  Simply add
``PSRAM`` to an array and it will be mapped into PSRAM:

.. code:: cpp

    ...
    float weights[4000] PSRAM;  // Place an array of 4000 floats in PSRAM
    char samplefile[1'000'000] PSRAM; // Allocate 1M for WAV samples in PSRAM
    ...

These variables can be used just like normal ones, no special handling is
required.  For example:

.. code:: cpp

    char buff[4 *1024 * 1024];   // 4MB array

    void initBuff() {
        bzero(buff, sizeof(buff));
        for (int i = 0; i < 4 *1024 * 1024; i += 4096) {
            buff[i] = rand();
        }
    }

The only restriction is that these variables may not be initialized statically.
The following example will **NOT** work:

.. code:: cpp

    char buff[] = "This is illegal and will not function";

Using PSRAM for dynamic allocations
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

PSRAM can also be used as a heap for dynamic allocations using ``pmalloc`` and
``pcalloc``.  These calls function exactly like normal ``malloc`` and ``calloc``
except they allocate space from PSRAM.

Simply replace a ``malloc`` or ``calloc`` with ``pmalloc`` or ``pcalloc`` to use
the PSRAM heap.  Other calls, such as ``free`` and ``realloc`` "just work" and do
not need to be modified (they check where the passed-in pointer resides and
do the right thing automatically).

For example, to create and modify large buffer in PSRAM:

.. code:: cpp

    void ex() {
        int *buff;
        // Ignoring OOM error conditions in example for brevity
        buff = (int *)pmalloc(10000 * sizeof(*buff));
        // Something happened and we need more space, so...
        buff = (int *)realloc(buff, 20000 * sizeof(*buff)); // buff now has 20K elements
        for (int i = 0; i < 20000; i++) {
            buff[i] = i;
        }
        // Do some work, now we're done
        free(buff);
    }

C++ objects can be allocated in PSRAM using "placement new" constructors.  Note that
this will only place immediate object data in PSRAM: if the object creates any other
objects via ``new`` *those* objects will be placed in normal RAM unless the object
also uses placement new constructors.



Checking on PSRAM space
~~~~~~~~~~~~~~~~~~~~~~~

The ``rp2040`` helper object has the following calls to return the state of the
PSRAM heap with the following calls, similar to the normal RAM heap:

int rp2040.getPSRAMSize()
-------------------------

Return the total size of the attached PSRAM chip.  This is the **RAW** space and
does not take into account any allocations for static variables or dynamic
allocations.  (i.e. it will return 1, 2, 4, 8, or 16MV depending on the chip).

int rp2040.getTotalPSRAMHeap()
------------------------------

Returns the total PSRAM heap (free and used) available or used for ``pmalloc``
allocations.

int rp2040.getUsedPSRAMHeap()
-----------------------------

Returns the total used bytes (including any overhead) of the PSRAM heap.

int getFreePSRAMHeap()
----------------------

Returns the total free bytes in the PSRAM heap.  (Note that this may include
multiple non-contiguous chunks, so this is not always the maximum block size
that can be allocated.)
