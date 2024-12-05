Profiling Applications with GPROF
=================================

Applications running on the Pico can be profiled using GNU GPROF to show where the CPU is using its time
on the device and how often certain functions are called.  It does this by recompiling the application
and adding a small preamble to each function built to identify what functions call what others (and
how frequently).  It also uses the ``SYSTICK`` exception timer to sample and record the PC 10,000 times
per second.  When an application is complete, the recorded date can be dumped to the host PC as a
``gmon.,out`` file which can be processed by ``arm-none-eabi-gprof`` into useful date.

s histogram of PCs and tally of function caller/callees can take a significant amount of RAM, from 100KB
to 10000KB depending on the size of the application.  As such, while the RP2040 **may** be able to
profile small applications, this is only really recommended on the RP2350 with external PSRAM.  The
profiler will automatically use PSRAM when available.  Call ``rp2040.getProfileMemoryUsage()`` to get the
memory allocated at runtime.


Profiling also adds processing overhead in terms of the periodic sampling and the function preambles.
In most cases there is no reason to enable (and many reasons to disable) profiling when an application
is deployed to the field.

To transfer the ``GMON.OUT`` data from the Pico to the host HP can be done by having the application
write it out to an SD card or a LittleFS filesystem which is then manually dumped, but for ease of use
semihosting can be used to allow the Pico (under the control of OpenOCD and GDB) to write the
``gmon.out`` file directly on the host PC, ready for use.

**NOTE** Semihosting only works when connected to an OpenOCD + GDB debug session.  Running an application
compiled for Semihosting without the debugger will cause a panic and hang the chip.

As of now, only ARM has support for Semihosting or GPROF.


Enabling Profiling in an Application
------------------------------------

The ``Tools->Profiling->Enabled`` menu needs to be selected to enable profiling support in GCC.  This will
add the necessary preamble to every function compiled (**Note** that the ``libpico`` and ``libc`` will not
be instrumented because they are pre-built so calls from them will not be fully instrumented.  However,
PC data will still be grabbed and decoded from them at runtime.)

The application will automatically start collecting profiling data even before ``setup`` starts in this
mode.  It will continue collecting data until you stop and write out the profiling data using
``rp2040.writeProfiling()`` to dump to the host, a file, serial port, etc.

For example, an application which does all its processing in ``setup()`` might look like:

.. code:: cpp

    #include <SemiFS.h>
    void setup() {
        SerialSemi.printf("BEGIN\n");
        do_some_work_that_takes_a_long_time_with_many_function_calls();
        // Do lots of other work...
        // Now all done...
        SerialSemi.printf("Writing GMON.OUT\n");
        SemiFS.begin();
        File gmon = SemiFS.open("gmon.out", "w");
        rp2040.writeProfiling(&gmon);
        gmon.close();
        SerialSemi.printf("END\n");
    }
    void loop() {}


Collecting and Analyzing Profile Data
-------------------------------------

Running this application under `semihosting <semihosting>`_ GDB and OpenOCD generates a ``gmon.out`` file
in the OpenOCD current working directory.  This file, combined with the ``ELF`` binary build in the
IDE and loaded through GDB, can produce profiler output using

.. code::

    $ /path/to/arm-none-eabi/bin/arm-none-eabi-gprof /path/to/sketch.ino.elf /path/to/gmon.out

See the ``rp2040/Profiling.ino`` example for more details.
