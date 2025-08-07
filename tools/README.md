# Tools directory for the RP2040 Arduino-Pico

## get.py
Downloads and installs the toolchain into a GIT clone of the repo.  Run
once after the `git clone` and any time the toolchain JSON file updates.
The `dist` directory caches downloaded toolchain files.

## discovery.py
Run in the background by the IDE to scan for UF2 drives to show in the
menus.  Normally not run manually by the user.

## uf2conv.py
Manages the upload of the UF2 formatted file to the board.  Called as part
of the IDE upload process.  Will optionally send the serial reset signal
to get the board into update mode (1200bps connection).

## simplesub.py
Very dumb `sed`-like tool used by the platform scripts to generate the
linker `.ld` file (replacing the EEPROM location, FS sizes, etc.).
Because we run on Windows, Mac, and Linux, need to provide this and not
rely on existence of `sed` command.

## pyserial
`git clone` of the PySerial Python3 library to be used by the IDE.

## makeboards.py
Generates `boards.txt` programmatically.  Never edit the `boards.txt` file
manually, use `python3 tools/makeboards.py`.  Change the script
as necessary to add any add'l fields or menus required.  Used because the
`boards.txt` file is very repetitive and it's safer to generate with code
than by hand.

## makepacer.cpp
Generates ``libraries/PWMAudio/src/PWMAudioPrecalc.h` which contains the
precalculated DMA pacer settings for common audio sample rates and CPU
frequencies.  Makes setting the frequency for PWMAudio instantaneous.

## makepio.py
Rebuilds all the ``*.pio`` files in the core and libraries using the
currently installed pioasm.  Use when a new PIOASM is available.

## makever.py
Updates the version info prior to a release in platform.txt, package.json,
and the version header.   Run from root of the repo.

## libpico/make-libpico.sh
Builds the libpico.a file as well as the bootloader stage2 binaries.
Run whenever the pico-sdk is updated.

## format-tzdata.py
Rebuilds cores/rp2040/TZ.h file using latest tzdata file.
