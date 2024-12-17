Bluetooth Audio (A2DP Source and Sink)
======================================

The PicoW can be used as a Bluetooth Audio sink or source with the ``BluetoothAudio`` class.
Operation is generally handled "automatically" in the background so while the audio is
playing or streaming the main application can perform other operations (like displaying
playback info, polling buttons for controls, etc.)

.. code :: cpp

    #include <BluetoothAudio.h>
    ...

**Note about CPU usage:**  Bluetooth SBC audio is a compressed format.  That means
that it takes non-trivial amounts of CPU to compress on send, or decompress on receive.
Transmitting precompressed audio from, say, MP3 or AAC, requires first decompressing
the source file into raw PCM and then re-compressing them in the SBC format.  You may
want to consider overclocking in this case to avoid underflow.

A2DPSink
--------

This class implements slave sink-mode operation with player control (play, pause, etc.) and
can play the received and decoded SBC audio to ``PWMAudio``, ``I2S``, or a user-created
`BluetoothAudioConsumer`` class.

The ``A2DPSink.ino`` example demonstrates turning a PicoW into a Bluetooth headset with
``PWMAudio``.

A2DPSource
-----------

This class implements a master source-mode SBC Bluetooth A2DP audio connection which
transmits audio using the standard ``Stream`` interface (like ``I2S`` or ``PWMAudio``.
The main application connects to a Bluetooth speaker and then writes samples into a buffer
that's automatically transmitted behind the scenes.

The ``A2DPSource.ino`` example shows how to connect to a Bluetooth speaker, transmit
data, and respond to commands from the speaker.
