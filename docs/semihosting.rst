Semihosting Support
===================

Using special debugger breakpoints and commands, the Pico can read and write to the debugging console as well
as read and write files on a development PC.  The ``Semihosting`` library allows applications to use the
semihosting support as a normal filesystem or serial port.

**NOTE** Semihosting only works when connected to an OpenOCD + GDB debug session.  Running an application
compiled for Semihosting without the debugger will cause a panic and hang the chip.

As of now, only ARM has support for Semihosting.

Running Semihosting on the Development Host
-------------------------------------------

Start OpenOCD normally from inside a directory that you can read and write files within (i.e. do not run from
``C:\\Program Files\\..`` on Windows where general users aren't allowed to write).  The starting
directory will be where the Pico will read and write files using the ``SemiFS`` class.
Be sure to keep the terminal window you ran OpenOCD in open, because all ``SerialSemi`` input and output
will go to **that** terminal and not ``gdb``'s.

Start GDB normally and connect to the OpenOCD debugger and enable semihosting support

.. code::

    (gdb) target extended-remote localhost:3333
    (gdb) monitor arm semihosting enable

At this point load and run your ``ELF`` application as normal.  Again, all ``SerialSemi`` output will go
to the **OpenOCD** window, not GDB.

See the ``hellosemi`` example in the ``Semihosting`` library.

SerialSemi - Serial over Semihosting
------------------------------------

Simply include ``<Semihosting.h>`` in your application and use ``SerialSemi`` as you would any other
``Serial`` port with the following limitations:

* Baud rate, bit width, etc. are all ignored
* Input is limited because ``read`` may hang indefinitely in the host and ``available`` is not part of the spec

``SerialSemi`` can also be selected as the debug output port in the IDE, in which case ``::printf`` will write
to the debugger directly.

SemiFS - Host filesystem access through Semihosting
---------------------------------------------------

Use ``SemiFS`` the same way as any other file system.  Note that only file creation and renaming are supported, with
no provision for iterating over directories or listing files.  In most cases simply opening a ``File`` and writing out
a debug dump is all that's needed:

.. code::

    SemiFS.begin();
    File f = SemiFS.open("debug.dmp", "w");
    f.write(buffer, size);
    f.close();
    SerialSemi.printf("Debug dump now available on host.\n");
