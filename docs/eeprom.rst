EEPROM Library
==============

While the Raspberry Pi Pico RP2040 does not come with an EEPROM onboard, we
simulate one by using a single 4K chunk of flash at the end of flash space.

**Note that this is a simulated EEPROM and will only support the number of
writes as the onboard flash chip, not the 100,000 or so of a real EEPROM.**
Therefore, do not frequently update the EEPROM or you may prematurely wear
out the flash.

EEPROM Class API
----------------

EEPROM.begin(size=256...4096)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Call before the first use of the EEPROM data for read or write.  It makes a
copy of the emulated EEPROM sector in RAM to allow random update and access.

EEPROM.read(addr), EEPROM[addr]
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Returns the data at a specific offset in the EEPROM. See `EEPROM.get` later
for a more 

EEPROM.write(addr, data), EEPROM[addr] = data
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Writes a byte of data at the offset specified.  Not persisted to flash until
``EEPROM.commit()`` is called.

EEPROM.commit()
~~~~~~~~~~~~~~~
Writes the updated data to flash, so next reboot it will be readable.

EEPROM.end()
~~~~~~~~~~~~
``EEPROM.commit()`` and frees all memory used.  Need to call `EEPROM.begin()`
before the EEPROM can be used again.

EEPROM.get(addr, val)
~~~~~~~~~~~~~~~~~~~~~
Copies the (potentially multi-byte) data in EEPROM at the specific byte
offset into the returned value.  Useful for reading structures from EEPROM.

EEPROM.put(addr, val)
~~~~~~~~~~~~~~~~~~~~~
Copies the (potentially multi-byte) value into EEPROM a the byte offset
supplied.  Useful for storing ``struct`` in EEPROM.  Note that any pointers
inside a written structure will not be valid, and that most C++ objects
like ``String`` cannot be written to EEPROM this way because of it.

EEPROM.length()
~~~~~~~~~~~~~~~
Returns the length of the EEPROM (i.e. the value specified in
``EEPROM.begin()`` ).

EEPROM Examples
---------------
Three EEPROM `examples <https://github.com/earlephilhower/arduino-pico/tree/master/libraries/EEPROM>`_ are included.
