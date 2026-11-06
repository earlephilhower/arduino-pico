# WM8960USB: Pico 2 USB audio through a WM8960 codec

This sketch turns a Pico 2 / Pico 2 W and a WM8960 codec module into a USB
sound card that also provides two serial ports and USB MIDI:

- PC audio plays on the WM8960 headphone/speaker output.
- The WM8960 microphone input is sent to the PC as a recording device.
- It has the same serial, MIDI, and command interface as
  [CompositeUSB](../../../rp2040/examples/CompositeUSB/README.md).

Audio is 48 kHz, 16-bit, mono in each direction. Mono playback is sent to both
the left and right codec outputs.

## Hardware

- **RP2350 board:** Pico 2, Pico 2 W, or similar. On RP2040 the sketch only
  prints a message, because it uses RP2350-only DMA features.
- **WM8960 module with its own MCLK oscillator:** 24 MHz by default; 12.288 MHz
  can be selected. The Pico doesn't generate MCLK.
- **Arduino-Pico 6.3.2**, with USB Stack set to **Pico SDK** (the default).

Default wiring (change it in [codec.h](codec.h)):

| Pico GPIO | Module signal | Direction |
| --- | --- | --- |
| 18 | BCLK | Pico -> codec |
| 19 | LRCLK/WS | Pico -> codec |
| 6 | DAC data (DIN) | Pico -> codec |
| 7 | ADC data (DOUT) | Codec -> Pico |
| 20 | SDA | I2C0 |
| 21 | SCL | I2C0 |

These are GPIO numbers, not header pin numbers. Connect ground, and power the
module as its maker specifies. Start with the headphone/speaker volume low.

Wiring rules if you change pins:

- LRCLK must be the GPIO directly after BCLK (BCLK + 1).
- All four I2S pins must sit within one 32-GPIO block: GPIO0–31, or GPIO16–47 on
  RP2350B.
- SDA/SCL must be a valid I2C0 pin pair. The codec address is `0x1A`.

The Pico is the I2S clock master. It drives BCLK at 3.072 MHz and LRCLK at
48 kHz. The codec runs as an I2S slave and uses its PLL to make its internal
clock from MCLK. The codec setup turns on the headphone/speaker outputs and the
microphone input with a fixed gain. If the codec doesn't answer on I2C, the
sketch stops with an error message instead of running without audio.

## Build and flash

```powershell
arduino-cli compile --fqbn rp2040:rp2040:rpipico2w WM8960USB
```

Or open `WM8960USB.ino` in Arduino IDE, select your Pico 2 board, and upload.
You don't need extra libraries.

## Using it

On the PC, the board shows up as **WM8960USB sample** with these devices:

- **Speakers (Composite test playback):** choose this as the output to hear PC
  audio on the codec.
- **Microphone (WM8960 ADC capture):** choose this as the input to record the
  codec microphone.
- Two serial ports and one MIDI port.

Open serial port 1 at 115200 baud and type `\?` for commands:

| Command | Result |
| --- | --- |
| `\text` | Sample text appears on serial port 2 |
| `\note` / `\cc` | Send MIDI channel 1 Note 60 / CC21 test messages |
| `\tone` | 3-second 1 kHz tone at about -30 dBFS, heard on the codec output and sent to the PC recording device |
| `\stop` | Stop the tone; playback and microphone continue |
| `\status` | Audio and USB counters (see below) |

If you hear the microphone on the PC with monitoring on, don't also send that
audio back to the board's speaker device. Doing both can cause acoustic
feedback.

### Reading `\status`

```text
USB OUT=15083198 IN=14963546 CDC2=22 MIDI=0 ALT=20 ERR=0 TONE=OFF
DMA TX=7650766 RX=7491023 lead=238 underruns=1 gap=501526 us
PCM queued=505 drop=0 capture-starve=0 FB=3145895 (16.16 frames/ms)
Timer runs=159317; DMA timer is independent of USB mutex
```

| Field | Meaning | Healthy value |
| --- | --- | --- |
| `OUT` / `IN` | USB audio bytes received from / sent to the PC | `OUT` increasing while the PC plays |
| `lead` | Frames already queued ahead of the DMA output | About 200–260 |
| `underruns` | Times the output DMA ran out of data since boot | Stays fixed; one at startup is normal |
| `gap` | Longest interval seen between audio service passes since boot | Peak value, not a recurring delay |
| `PCM queued` | Playback frames buffered in software | Steady, a few hundred |
| `drop` / `capture-starve` | Playback frames dropped / microphone data unavailable | 0 |
| `FB` | Rate the board asks the PC for, in 16.16 format. 3145728 is exactly 48.000 frames/ms | Within a few hundred of 3145728 |
| `Timer runs` | 1 ms audio timer ticks | Increasing |
| `ERR` | USB write failures | 0 |

### Troubleshooting

- **No sound from PC playback, and `OUT=0`:** Windows isn't sending audio to
  the board.
  - Check that **Composite test playback** is the selected output.
  - If Windows shows names from other firmware previously run on the board,
    it is reusing old saved device settings. Open Device
    Manager, choose View > Show hidden devices, uninstall the old audio
    device, then replug the board. The sketch uses its own product name and
    serial number to help prevent this.
- **The sketch stops at startup with a WM8960 error:** check SDA/SCL wiring,
  module power, and that the module's MCLK oscillator is running.
- **Crackling:** check that `underruns` isn't increasing and `lead` stays
  above about 100.

## How it works

### Playback

```text
PC -> USB -> TinyUSB buffer -> 1024-frame playback buffer -> DMA -> PIO I2S -> WM8960 DAC
```

- When a USB playback packet arrives, the sketch copies it from TinyUSB into
  its own playback buffer.
- A 1 ms timer moves audio from that buffer to the output DMA ring. The DMA
  ring is kept about 256 frames (5 ms) ahead of the hardware.
- Playback starts after 512 frames are buffered. If the PC stops sending, the
  output switches to silence and refills before restarting, instead of
  repeating old audio.

The PC's and the codec's sample clocks are never exactly equal. USB *feedback*
corrects this:

- After each packet, the sketch counts every playback frame waiting in the
  TinyUSB buffer, its own buffer, and the DMA ring.
- It compares the total to a 768-frame (16 ms) target.
  - Below the target, it asks the PC for slightly more audio.
  - Above the target, it asks for slightly less.
- The correction is smoothed, and limited to ±0.25 frames per millisecond.

### Capture

```text
WM8960 ADC -> PIO I2S -> DMA ring -> USB capture callback -> PC
```

- The USB capture callback is the only reader of the input DMA ring. Nothing
  else should read that ring.
- It waits for 192 frames before it starts sending.
- If it runs out of microphone data, it sends silence and waits for the ring to
  refill.
- It keeps the TinyUSB buffer about half full. TinyUSB then sends 47, 48, or 49
  frames per USB packet to follow the codec clock.

### Scheduling

- The 1 ms timer keeps the output DMA filled even when `loop()` or USB is busy.
  The timer is protected by its own short spin lock and never waits on the
  core's `USB.mutex`.
- USB buffer reads happen only in USB callbacks, which already run inside the
  core's USB task.
- Work in `loop()` (commands, serial port 2, MIDI) is short and doesn't block
  audio.

### I2S/DMA

[wm8960_i2s.cpp](wm8960_i2s.cpp) and the `.pio` programs come from the WM8960
driver in [K3NG_Keyer_ESP32_BT_Keyboard](https://github.com/K7MDL2/K3NG_Keyer_ESP32_BT_Keyboard).
They provide:

- Continuous, hardware-paced RP2350 DMA for transmit and receive.
- 4096-frame rings (about 85 ms).
- Underrun detection and progress counters for `\status`.

## Tests

Run `.\test-wm8960.ps1` on Windows. It needs Visual Studio C++ build tools.
It checks feedback direction and limits, playback output and mute, tone level
and fades, overflow handling, timer rendering while USB is busy, and microphone
start-up and starvation handling.

## Status

Tested on Windows with a Pico 2 W:

- PC playback on the codec is clean, with feedback within 0.005% of 48 kHz.
- The microphone is heard on the PC.
- The test tone is clean.

Linux and macOS haven't been tested.

## Credits

- USB and I2S work originated in PA3GSB's
  [RP2350-SDR-CW-Interface](https://github.com/pa3gsb/RP2350-SDR-CW-Interface).
- Arduino-Pico adaptation from
  [K3NG_Keyer_ESP32_BT_Keyboard](https://github.com/K7MDL2/K3NG_Keyer_ESP32_BT_Keyboard).
- Audio stream-restart fix:
  [hathach/tinyusb#2937](https://github.com/hathach/tinyusb/pull/2937).
- TinyUSB sources included by the overlays are MIT-licensed and keep their
  original notices.
