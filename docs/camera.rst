Camera (PicoCamera) Library
===========================

The ``PicoCamera`` library adds support for camera sensors with a parallel
(DVP) interface, such as the Omnivision OV2640, OV3660, and OV7670.  While
the RP2040/RP2350 chips do not include a dedicated camera peripheral, the
library uses a PIO (Programmable I/O) state machine together with DMA to
capture frames in the background with almost no CPU involvement.

The API is intentionally shaped after the well-known ``esp32-camera``
library: the camera is configured once at init time through a
``camera_config_t`` structure, frames are captured into pre-allocated
buffers which the application borrows and returns, and the sensor can be
adjusted at runtime through a set of control function pointers.

Both RGB565 raw frames and JPEG frames (from sensors with an on-chip JPEG
encoder, such as the OV2640 and OV3660) are supported.

Include the library header and define the configuration before use:

.. code:: cpp

    #include <PicoCamera.h>

Configuration (camera_config_t)
-------------------------------

Everything the driver needs is supplied once, at init time, through
``camera_config_t``.  There is no board-level hardcoding.  A complete
configuration looks like this:

.. code:: cpp

    camera_config_t config = {
        .pin_pwdn     = -1,
        .pin_reset    = -1,
        .pin_xclk     = 10,
        .pin_sccb_sda = 12,
        .pin_sccb_scl = 13,
        .pin_d0 = 2, .pin_d1 = 3, .pin_d2 = 4, .pin_d3 = 5,
        .pin_d4 = 6, .pin_d5 = 7, .pin_d6 = 8, .pin_d7 = 9,
        .pin_vsync = 11,
        .pin_href  = 14,
        .pin_pclk  = 15,
        .xclk_freq_hz = 10000000,
        .sccb_i2c_port = 0,
        .pixel_format  = PIXFORMAT_JPEG,
        .frame_size    = FRAMESIZE_QVGA,
        .jpeg_quality  = 12,
        .fb_count      = 2,
        .fb_location   = PICO_CAMERA_FB_AUTO,
    };

The most important fields are:

* ``pin_pwdn`` / ``pin_reset`` - camera power-down and reset GPIOs.  Set
  to ``-1`` if not connected.

* ``pin_xclk`` - master clock (XCLK) output GPIO, driven by a PWM slice.

* ``pin_sccb_sda`` / ``pin_sccb_scl`` - SCCB (I2C) data and clock GPIOs,
  used to detect and configure the sensor.  Pins are hard-muxed: SDA must
  be even, SCL odd, both routing to ``sccb_i2c_port``.  Set
  ``pin_sccb_sda = -1`` to reuse an I2C bus already initialized by the
  application (see the shared bus note below); ``pin_sccb_scl`` is then
  ignored.

* ``pin_d0`` ... ``pin_d7`` - the 8 parallel data lines.  These **must be
  8 consecutive GPIOs** (``pin_dN == pin_d0 + N``, a PIO hardware
  constraint; validated at init).

* ``pin_vsync`` / ``pin_href`` / ``pin_pclk`` - frame sync, line sync and
  pixel clock inputs.  Any free GPIO may be used.

* ``xclk_freq_hz`` - XCLK frequency in Hz.  ``0`` selects the default of
  10 MHz; 10-24 MHz is the typical sensor range.

* ``sccb_i2c_port`` - the I2C peripheral used for SCCB, ``0`` or ``1``.
  In shared-bus mode (``pin_sccb_sda = -1``) it selects which already
  initialized bus to reuse.

* ``pixel_format`` - ``PIXFORMAT_RGB565`` or ``PIXFORMAT_JPEG``.  JPEG
  requires a sensor with an on-chip encoder (OV2640/OV3660 yes, OV7670 no).

* ``frame_size`` - a ``FRAMESIZE_*`` value.  Sizes beyond the sensor
  maximum are clamped with a warning instead of failing.  Available sizes
  include 96x96, QQVGA (160x120), QCIF (176x144), HQVGA (240x176),
  240x240, QVGA (320x240), CIF (400x296), HVGA (480x320), VGA (640x480),
  SVGA (800x600), XGA (1024x768), HD (1280x720), SXGA (1280x1024), and
  UXGA (1600x1200).

* ``jpeg_quality`` - 0-63, **lower means higher quality**.  JPEG mode only.

* ``fb_count`` - number of frame buffers to allocate.

* ``fb_location`` - where frame buffers live.  ``PICO_CAMERA_FB_AUTO``
  (the default) uses PSRAM when available and SRAM otherwise,
  ``PICO_CAMERA_FB_IN_PSRAM`` forces PSRAM (init fails if unavailable),
  and ``PICO_CAMERA_FB_IN_SRAM`` forces on-chip SRAM.

Shared SCCB (I2C) Bus
---------------------

To put the camera sensor on an I2C bus already used for other devices,
initialize that bus yourself (``Wire.begin()`` or ``Wire1.begin()``) and
then set ``config.pin_sccb_sda = -1`` and ``config.sccb_i2c_port`` to the
matching port.  The library skips all pin and bus setup in this mode and
never deinitializes a shared bus at ``pico_camera_deinit()``.

Memory Considerations
---------------------

Frame buffers are the largest consumer of memory in this library, so it is
worth planning ahead.  The RP2040 has 264 KB of on-chip SRAM, while the
RP2350 has 520 KB; the frame buffers, the rest of your sketch, and the
Arduino core itself all share this space.

For RGB565 frames each buffer needs ``width * height * 2`` bytes:

* QQVGA (160x120) - about 38 KB per buffer.
* QVGA (320x240) - about 150 KB per buffer, near the practical ceiling
  on the RP2040.
* VGA (640x480) - 600 KB per buffer, does not fit in SRAM on either chip.
* SVGA (800x600) - about 940 KB per buffer.

For JPEG frames each buffer is allocated as ``width * height / 4 + 8 KB``,
which covers typical scenes (the actual JPEG data is usually smaller, so
only part of the buffer is used):

* QVGA (320x240) - about 27 KB per buffer.
* VGA (640x480) - about 83 KB per buffer.
* SVGA (800x600) - about 125 KB per buffer.
* UXGA (1600x1200) - about 477 KB per buffer.

Boards with a PSRAM chip (available on many RP2350 boards) have a much
more generous allowance: when the core's PSRAM support is enabled, the
frame buffers can be placed in PSRAM (see ``fb_location`` above), making
large RGB565 frames and multi-megapixel JPEG captures practical.

Initializing the Camera
-----------------------

With the configuration filled in, start the camera by calling
``pico_camera_init()`` once:

.. code:: cpp

    int err = pico_camera_init(&config);
    if (err != PICO_CAMERA_OK) {
        // no sensor found, bad pin layout, out of memory, ...
        // see the Error Codes section below
    }

This probes the sensor over SCCB, validates the pin layout, allocates the
frame buffers, and arms the PIO + DMA capture engine.  Afterwards frames
can be captured with ``pico_camera_fb_get()``.  Call
``pico_camera_deinit()`` before re-initializing with a different
configuration.

The Frame Buffer (camera_fb_t)
------------------------------

Captured frames are delivered in a ``camera_fb_t`` structure:

.. code:: cpp

    typedef struct {
        uint8_t *buf;        // pixel data
        size_t   len;        // used bytes in buf
        size_t   width;      // pixels
        size_t   height;     // pixels
        pixformat_t format;  // PIXFORMAT_RGB565 or PIXFORMAT_JPEG
        struct timeval timestamp;  // capture time since boot
    } camera_fb_t;

For RGB565 frames, ``len == width * height * 2``, one 16-bit pixel per 2
bytes.  For JPEG frames, ``buf`` holds a complete, standalone JPEG file
(starting with ``0xFF 0xD8`` and ending with ``0xFF 0xD9``) and ``len``
varies per frame; it can be written straight to a file or socket.

With ``fb_count > 1`` the DMA engine can fill the next buffer while the
application still processes the previous one.  With ``fb_count == 1`` each
``pico_camera_fb_get()`` waits for a new frame to be captured after the
previous buffer was returned.

Reading and Processing Frames
-----------------------------

The capture loop is always the same shape: borrow a frame buffer with
``pico_camera_fb_get()``, do something with the pixels, then give the
buffer back with ``pico_camera_fb_return()``.  The following ``loop()``,
adapted from the ``push_image_to_python`` example shipping with the
library, grabs an RGB565 frame and streams it raw over the USB serial
port; the spot marked in the middle is where your own processing (image
filtering, feature detection, driving a display, and so on) goes:

.. code:: cpp

    void loop() {
        camera_fb_t *fb = pico_camera_fb_get();
        if (fb) {
            // fb->buf holds fb->width * fb->height 16-bit RGB565 pixels.
            // Process the frame here, e.g. walk the pixels:
            //
            //   uint16_t *pixels = (uint16_t *)fb->buf;
            //   for (size_t i = 0; i < fb->width * fb->height; i++) {
            //       uint16_t pixel = pixels[i];
            //       ...
            //   }

            // Stream the raw frame to the PC, framed with start/end markers
            Serial.write("SRGB", 4);
            Serial.write(fb->buf, fb->len);
            Serial.write("ERGB", 4);
            Serial.flush();  // wait until the whole frame left the FIFO

            pico_camera_fb_return(fb);
        }
        delay(10);
    }

A matching Python receiver (``push_image_to_python.py``) that displays the
stream is included next to the example in
``libraries/PicoCamera/examples/push_image_to_python``.

Runtime Sensor Control
----------------------

``pico_camera_sensor_get()`` returns the detected sensor's control
structure.  All controls are function pointers, and **a sensor only
implements a subset of them, so always NULL-check before calling**:

.. code:: cpp

    sensor_t *s = pico_camera_sensor_get();
    if (s && s->set_vflip) {
        s->set_vflip(s, 1);
    }

Available operations include:

* ``set_pixformat(fmt)`` - switch between RGB565 and JPEG at runtime.
* ``set_framesize(size)`` - change resolution at runtime.
* ``set_brightness(level)`` / ``set_contrast(level)`` /
  ``set_saturation(level)`` / ``set_sharpness(level)`` - image tuning,
  small-integer levels (on the OV2640, brightness/contrast/saturation
  accept -2...2).
* ``set_gainceiling(gc)`` - AGC ceiling, ``GAINCEILING_2X`` ...
  ``GAINCEILING_128X``.
* ``set_quality(q)`` - JPEG quality 0-63, lower is better.
* ``set_colorbar(on)`` - enable a test color-bar pattern.
* ``set_whitebal(on)`` / ``set_wb_mode(mode)`` - auto white balance
  switch / white balance preset mode.
* ``set_gain_ctrl(on)`` / ``set_exposure_ctrl(on)`` - AGC / AEC switch.
* ``set_ae_level(level)`` / ``set_aec_value(value)`` - auto-exposure
  target level / manual exposure value.
* ``set_hmirror(on)`` / ``set_vflip(on)`` - horizontal mirror / vertical
  flip.
* ``set_special_effect(effect)`` - effect index (grayscale, negative,
  etc.).
* ``set_reg(reg, mask, value)`` / ``get_reg(reg, mask)`` - raw register
  write/read, where ``mask`` selects which bits to touch.  This is the
  escape hatch for anything not wrapped above.
* ``set_xclk(timer, xclk)`` - change the XCLK frequency at runtime
  (``timer`` is unused).
* ``reset()`` - sensor soft reset.

Not every sensor implements every operation.  The OV2640 implements the
full set; the OV7670 has no JPEG encoder and lacks the image-tuning
controls; the OV3660 falls in between.  Always check the function pointer
for ``NULL`` before calling, as shown above.

Shutting Down the Camera
------------------------

When the camera is no longer needed, release it with
``pico_camera_deinit()``:

.. code:: cpp

    pico_camera_deinit();

This stops the PIO + DMA capture engine, frees all frame buffers, and
releases the SCCB bus.  A shared bus (passed in with
``pin_sccb_sda = -1``) is left running, since the library did not set it
up.  Afterwards the camera can be re-initialized with a fresh
configuration by calling ``pico_camera_init()`` again.

Camera API
----------

int pico_camera_init(const camera_config_t \*config)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Probes the sensor over SCCB, validates the pin layout, allocates the frame
buffers, and arms the PIO + DMA capture engine.  Returns
``PICO_CAMERA_OK`` on success or one of the error codes listed below.
Call it once; call ``pico_camera_deinit()`` before re-initializing with a
different configuration.

camera_fb_t \*pico_camera_fb_get()
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Blocks until one full frame has been captured by DMA into a buffer and
returns that buffer.  Returns ``NULL`` on error or when no free buffer is
available.

void pico_camera_fb_return(camera_fb_t \*fb)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Returns a frame buffer to the driver for reuse.  Always call this when
done with a frame; buffers are recycled, and not returning them starves
the driver once all ``fb_count`` buffers are checked out.

int pico_camera_deinit()
~~~~~~~~~~~~~~~~~~~~~~~~
Stops the PIO/DMA engine, releases the SCCB bus (unless shared), and frees
all frame buffers.

sensor_t \*pico_camera_sensor_get()
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Returns the detected sensor's runtime control structure, or ``NULL``
before init.  See the runtime sensor control section above.

const sensor_info_t \*pico_camera_sensor_info_get()
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Returns static information about the detected sensor model: ``name``,
``sccb_addr``, ``pid``, ``max_size`` (largest ``framesize_t`` supported),
and ``support_jpeg``.  Useful for adapting the configuration at runtime.

Error Codes
-----------

``pico_camera_init()`` and ``pico_camera_deinit()`` return one of the
following:

* ``PICO_CAMERA_OK`` (0) - success.
* ``PICO_CAMERA_ERR_NOT_DETECTED`` - no sensor answered on SCCB.
* ``PICO_CAMERA_ERR_NOT_SUPPORTED`` - e.g. JPEG requested on an OV7670.
* ``PICO_CAMERA_ERR_INVALID_ARG`` - bad configuration, such as
  non-consecutive data pins.
* ``PICO_CAMERA_ERR_INVALID_STATE`` - init called twice, or deinit
  without init.
* ``PICO_CAMERA_ERR_NO_MEM`` - frame buffer allocation failed.
* ``PICO_CAMERA_ERR_TIMEOUT`` - capture timed out.
* ``PICO_CAMERA_ERR_FAILED_TO_SET_FRAME_SIZE`` /
  ``PICO_CAMERA_ERR_FAILED_TO_SET_OUT_FORMAT`` - the sensor rejected the
  requested format or size.

Example Sketch
--------------

The following sketch captures JPEG frames from an OV2640 and prints the
frame size over the serial port:

.. code:: cpp

    #include <PicoCamera.h>

    camera_config_t config = {
        .pin_pwdn     = -1,
        .pin_reset    = -1,
        .pin_xclk     = 10,
        .pin_sccb_sda = 12,
        .pin_sccb_scl = 13,
        .pin_d0 = 2, .pin_d1 = 3, .pin_d2 = 4, .pin_d3 = 5,
        .pin_d4 = 6, .pin_d5 = 7, .pin_d6 = 8, .pin_d7 = 9,
        .pin_vsync = 11,
        .pin_href  = 14,
        .pin_pclk  = 15,
        .xclk_freq_hz = 10000000,
        .sccb_i2c_port = 0,
        .pixel_format  = PIXFORMAT_JPEG,
        .frame_size    = FRAMESIZE_QVGA,
        .jpeg_quality  = 12,
        .fb_count      = 2,
        .fb_location   = PICO_CAMERA_FB_AUTO,
    };

    void setup() {
        Serial.begin(115200);
        int err = pico_camera_init(&config);
        if (err != PICO_CAMERA_OK) {
            Serial.printf("Camera init failed, error %d\n", err);
            while (true) { /* halt */ }
        }
    }

    void loop() {
        camera_fb_t *fb = pico_camera_fb_get();
        if (fb) {
            Serial.printf("Frame: %dx%d, %d bytes\n",
                          (int)fb->width, (int)fb->height, (int)fb->len);
            pico_camera_fb_return(fb);
        }
    }

Additional examples, including rendering frames to a TFT display and
streaming images to a host PC, ship with the library under
``libraries/PicoCamera/examples``.

For more information, see the PicoCamera project home page at
https://github.com/umeiko/PicoCamera
