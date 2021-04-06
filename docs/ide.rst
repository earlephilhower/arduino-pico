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
