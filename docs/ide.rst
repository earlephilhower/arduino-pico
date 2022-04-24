IDE Menus
=========

Model
-----
Use the boards menu to select your model of RP2040 board.  There will be two
options:  `Boardname` and `Boardname (Picoprobe)`.  If you want to use a
Picoprobe to upload your sketches and not the default automatic UF2 upload,
use the `(Picoprobe)` option, otherwise use the normal name.  No functional
or code changes are done because of this.

There is also a `Generic` board which allows you to individually select
things such as flash size or boot2 flash type.  Use this if your board isn't
yet fully supported and isn't working with the normal `Raspberry Pi Pico`
option.

Flash Size
----------
Arduino-Pico supports onboard filesystems which will set aside some of the
flash on your board for the filesystem, shrinking the maximum code size
allowed.  Use this menu to select the desired ratio of filesystem to sketch.

CPU Speed
---------
While it is unsupported, the Raspberry Pi Pico RP2040 can often run much
faster than the stock 125MHz.  Use the `CPU Speed` menu to select a
desired over or underclock speed.  **If the sketch fails at the higher
speed, hold the BOOTSEL while plugging it in to enter update mode and try
a lower overclock.**

Debug Port and Debug Level
--------------------------
Debug messages from `printf` and the Core can be printed to a Serial port
to allow for easier debugging.  Select the desired port and verbosity.
Selecting a port for debug output does not stop a sketch from using it
for normal operations.

Generic RP2040 Support
----------------------
If your RP2040 board isn't in the menus you can still use it with the
IDE bu using the `Board->Generic RP2040` menu option.  You will need to
then set the flash size (see above) and tell the IDE how to communicate
with the flash chip using the `Tools->Boot Stage 2` menu.

Boot Stage 2 Options for Generic RP2040
---------------------------------------
The Arduino Pico needs to set up its internal flash interface to talk to
whatever flash chip is in the system.  While all flash chips support a
basic (and slow) 1-bit operation using common timings, each different
brand (and sometimes model) of flash chip require custom timings to work
in QSPI (4-bit) mode.  The `Boot Stage 2` menu lets you select from
the supported timings.

The options with `/2` in them divide the system clock by 2 to drive the
bus.  Options with `/4` divide the clock by 4 and so are slower but more
compatible.

If you can't match a chip name in the menu to your flash chip, a simple
test can be run to determine which is correct.  Simpily load the `Blink`
example, select the first option in the `Boot Stage 2` menu, and upload.
If that works, note it and continue.  Iterate through the options and
note which ones work.  If an option doesn't work, unplug the chip and
hold the BOOTSEL button down while re-inserting it to enter the ROM
uploader mode.  (The CPU and flash will not be harmed if the test fails.)

If one of the custom bootloaders (not `Generic SPI /2 or /4`) worked, use
that option to get best performance.  If none worked other than the
`Generic SPI /2 or /4` then use that.  The `/2` options of all models
is preferred as it is faster, but some boards do require `/4` on the
custom chip interfaces.

When in doubt, `Generic SPI /4` should work with any flash chip but is
slow.  
