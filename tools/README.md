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

## makever.py
Updates the version info prior to a release in platform.txt, package.json,
and the version header.   Run from root of the repo.

## libpico/make-libpico.sh
Builds the libpico.a file as well as the bootloader stage2 binaries.
Run whenever the pico-sdk is updated.
