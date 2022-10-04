# Arduino-Pico OTA Bootloader

This directory contains a small "stage 3" bootloader (after the boot ROM and the ``boot2.S`` flash configuration) which implements a power fail safe, generic OTA method.

The bootloader is built here into an ``.ELF``, without ``boot2.S`` (which will come from the main app), configured to copy itself into RAM (so that it can update itself), and included in the main applications.  Exactly ``12KB`` for all sketches is consumed by this OTA bootloader.

It works by mounting the LittleFS file system (the parameters are stored by the main app at 0x3000-16), checking for a specially named command file.  If that file exists, and its contents pass a checksum, the bootloader reads from the filesystem (optionally, automatically decompressing ``GZIP`` compressed files) and writes to application flash.

Every block is checked to see if it identical to the block already in flash, and if so it is skipped.  This allows silently skipping bootloader writes in many cases.

Should a power failure happen, as long as it was not in the middle of writing a new OTA bootloader, it should simply begin copying the same program from scratch.

When the copy is completed, the command file's contents are erased so that on a reboot it won't attempt to write the same firmware over and over.  It then reboots the chip (and re-runs the potentially new bootloader).

If there is no special file, or its contents don't have a proper checksum, the bootloader simply adjusts the ARM internal vector pointers and jumps to the main application.

The files in the LittleFS filesystem can come over ``WiFi``, over an ``Ethernet`` object, or even over a serial port.
