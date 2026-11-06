# CompositeUSB: two serial ports, MIDI, and USB audio from a sketch

This example makes a Pico (RP2040) or Pico 2 (RP2350) appear to the PC as one
USB device with:

- two serial (CDC) ports,
- a USB MIDI port,
- USB audio playback and capture (UAC2, 48 kHz, 16-bit, mono).

The extra functions are added from the sketch, with the stock core and the
default **Pico SDK** USB stack. No core files are modified and no libraries are
needed. No extra hardware is needed: PC playback is accepted but discarded, and
the capture device sends silence or a test tone.

For real audio through a WM8960 codec on RP2350, see the
[WM8960USB example](../../../rp2350/examples/WM8960USB/README.md).

## Requirements

- Tools > USB Stack: **Pico SDK** (the default). The sketch stops with a
  compile error if Adafruit TinyUSB is selected.
- Tested with Arduino-Pico 6.3.2 on Windows.

## What appears on the PC

| Interface | Behavior |
| --- | --- |
| Serial port 1 (CDC0) | The normal `Serial` port: command-line interface and status |
| Serial port 2 (CDC1) | Echoes typed text; receives `\text` output |
| USB MIDI | Echoes incoming packets; sends test Note/CC messages |
| Playback device | Accepts audio and discards it |
| Recording device | Silence, or a 1 kHz test tone on request |

The order in which the PC lists the ports isn't guaranteed. Use the interface
names to tell them apart.

## Command-line interface

Open **serial port 1** at 115200 baud. Commands end with Enter (CR, LF, or CRLF
line endings). Type `\?` to list the commands. Turn on local echo in your
terminal if you want to see what you type.

| Command | Result |
| --- | --- |
| `\?` | List commands and what each one does |
| `\text` | Send `Composite USB sample text: hello from Pico!` to **serial port 2** (open a second terminal on it) |
| `\note` | Send MIDI channel 1 Note On, note 60 (middle C), velocity 100; Note Off follows 500 ms later |
| `\cc` | Send MIDI channel 1 CC21, value 64 |
| `\tone` | Send a 1 kHz test tone to the recording device for three seconds |
| `\stop` | Stop the test tone |
| `\status` | Print USB traffic and error counters |

Commands are lowercase, and unknown commands print an error. If the PC isn't
reading the port, text and MIDI sends time out after five seconds.

### Tips

- **Hearing the test tone:** the tone goes to the PC's *recording* device.
  Open it at 48 kHz/16-bit in a recorder or DAW with input monitoring on. On
  Windows, you can instead turn on **Listen to this device** in the recording
  device's properties. Start with the volume low.
- **MIDI:** open the MIDI input in a MIDI monitor before sending `\note` or
  `\cc`. Turn off MIDI thru/echo in the PC application, or the sketch's echo
  can loop. To hear the note, you need a software synthesizer.
- **`\status` counters:** `OUT`/`IN` are USB audio bytes. `CDC2` counts bytes
  echoed on serial port 2. `MIDI` counts echoed MIDI packets. `ALT` counts
  audio stream starts and stops. `ERR` counts USB write failures and should
  stay at 0.
- The 1200-baud reset into BOOTSEL still works on serial port 1.

## How it works

The core normally provides one serial port and no USB audio. This sketch adds
the extra functions using local copies of a few TinyUSB driver files:

1. [composite_config.h](composite_config.h) includes the core's TinyUSB
   configuration, then raises the counts: 2 CDC, 1 MIDI, 2 audio functions. It
   also sets the audio buffer sizes.
2. [overlay_cdc.c](overlay_cdc.c) builds the CDC driver with two ports. Only
   port 1 line-state and baud-rate events reach the core's `Serial`, so opening
   or closing port 2 doesn't affect port 1.
3. [overlay_midi.c](overlay_midi.c) builds the real MIDI driver in place of the
   core's empty stubs.
4. [overlay_audio.c](overlay_audio.c) builds the USB audio driver and registers
   it with the core as an application driver (`usbd_app_driver_get_cb`).
5. [overlay_dcd.c](overlay_dcd.c) builds the RP2040/RP2350 USB hardware driver
   with one fix: an audio stream can be restarted without an "endpoint already
   available" panic. This is the fix merged upstream in
   [hathach/tinyusb#2937](https://github.com/hathach/tinyusb/pull/2937).
6. [composite.cpp](composite.cpp) briefly disconnects USB, allocates extra
   endpoints, registers the audio, serial, and MIDI interfaces with
   `USB.registerInterface`, then reconnects. The audio, CDC, and MIDI
   callbacks and the command-line interface are also here.

The overlay files `#include` the TinyUSB sources that ship with the core, so no
TinyUSB code is copied. The sketch's versions replace the core's prebuilt ones
when the program is linked. Because of this, the method depends on the core
version. After updating the core, rebuild and retest.

The core's USB task still runs TinyUSB. Serial port 2 and MIDI writes from
`loop()` take `USB.mutex` first. Audio callbacks already run inside the USB
task, so they don't take the mutex again.

This example uses mono on both sides to keep the descriptors small. Its
playback sink and capture source run at exactly the USB rate, so it reports a
fixed 48 kHz feedback value. A real codec with its own clock needs measured
feedback; WM8960USB shows how.

## Tests

`test-cli.ps1` compiles the sketch logic on Windows with Visual Studio C++
build tools and checks it with simulated USB calls; no board is needed. It
covers command parsing, serial-port-2 text, exact MIDI bytes, Note Off timing,
timeouts, and the tone output.

## Credits

- USB audio, CDC, and MIDI composite work originated in PA3GSB's
  [RP2350-SDR-CW-Interface](https://github.com/pa3gsb/RP2350-SDR-CW-Interface).
- Arduino-Pico adaptation from
  [K3NG_Keyer_ESP32_BT_Keyboard](https://github.com/K7MDL2/K3NG_Keyer_ESP32_BT_Keyboard).
- Audio stream-restart fix:
  [hathach/tinyusb#2937](https://github.com/hathach/tinyusb/pull/2937).
- TinyUSB sources included by the overlays are MIT-licensed and keep their
  original notices.
