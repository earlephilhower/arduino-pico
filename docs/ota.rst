OTA Updates
===========


Introduction
------------

OTA (Over the Air) update is the process of uploading firmware to a Pico using a Wi-Fi, Ethernet, or other connection rather than a serial port.  This is especially useful for WiFi enabled Picos, like the Pico W, because it lets systems be updated remotely, without needing physical access.

OTA may be done using:

-  `Arduino IDE <#arduino-ide>`__
-  `Web Browser <#web-browser>`__
-  `HTTP Server <#http-server>`__
- Any other method (ZModen receive over a UART port, etc.) by using the ``Updater`` object in your sketch

The Arduino IDE option is intended primarily for the software development phase. The other two options would be more useful after deployment, to provide the module with application updates either manually with a web browser, or automatically using an HTTP server.

In any case, the first firmware upload has to be done over a serial port. If the OTA routines are correctly implemented in the sketch, then all subsequent uploads may be done over the air.

By default, there is no imposed security for the OTA process.  It is up to the developer to ensure that updates are allowed only from legitimate / trusted sources. Once the update is complete, the module restarts, and the new code is executed. The developer should ensure that the application running on the module is shut down and restarted in a safe manner. Chapters below provide additional information regarding security and safety of OTA updates.

OTA Requirements
~~~~~~~~~~~~~~~~

OTA requires a LittleFS partition to store firmware upgrade files.  Make sure that you configure the sketch with a filesystem large enough to handle whatever size firmware binary you expect.  Updates may be compressed, minimizing the total space needed.

Power Fail Safety
~~~~~~~~~~~~~~~~~

The update commands are all stored in flash, so a power cycle during update (except if the OTA bootloader is being changed) should not brick the device because when power is restored the OTA bootloader will begin the process from scratch once again.


Security Disclaimer
~~~~~~~~~~~~~~~~~~~

No guarantees as to the level of security provided for your application by the following methods is implied.  Please refer to the GNU LGPL license associated for this project for full disclaimers.  If you do find security weaknesses, please don't hesitate to contact the maintainers or supply pull requests with fixes.  The MD5 verification and password protection schemes are already known to supply a very weak level of security.

Basic Security
~~~~~~~~~~~~~~

The module has to be exposed wirelessly to get it updated with a new sketch. That poses a risk of the module being violently hacked and programmed with some other code. To reduce the likelihood of being hacked, consider protecting your uploads with a password, selecting certain OTA port, etc.

Check functionality provided with the `ArduinoOTA <https://github.com/earlephilhower/arduino-pico/tree/master/libraries/ArduinoOTA>`__ library that may improve security:

.. code:: cpp

    void setPort(uint16_t port);
    void setHostname(const char* hostname);
    void setPassword(const char* password);

Certain basic protection is already built in and does not require any additional coding by the developer. `ArduinoOTA <https://github.com/earlephilhower/arduino-pico/tree/master/libraries/ArduinoOTA>`__ and espota.py use `Digest-MD5 <https://en.wikipedia.org/wiki/Digest_access_authentication>`__ to authenticate uploads. Integrity of transferred data is verified on the Pico side using `MD5 <https://en.wikipedia.org/wiki/MD5>`__ checksum.

Make your own risk analysis and, depending on the application, decide what library functions to implement. If required, consider implementation of other means of protection from being hacked, like exposing modules for uploads only according to a specific schedule, triggering OTA only when the user presses a dedicated “Update” button wired to the Pico, etc.

Advanced Security - Signed Updates
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

While the above password-based security will dissuade casual hacking attempts, it is not highly secure.  For applications where a higher level of security is needed, cryptographically signed OTA updates can be required.  This uses SHA256 hashing in place of MD5 (which is known to be cryptographically broken) and RSA-2048 bit level public-key encryption to guarantee that only the holder of a cryptographic private key can produce signed updates accepted by the OTA update mechanisms.

Signed updates are updates whose compiled binaries are signed with a private key (held by the developer) and verified with a public key (stored in the application and available for all to see).  The signing process computes a hash of the binary code, encrypts the hash with the developer's private key, and appends this encrypted hash (also called a signature) to the binary that is uploaded (via OTA, web, or HTTP server).  If the code is modified or replaced in any way by anyone except the holder of the developer's private key, the signature will not match and the Pico will reject the upload.

Cryptographic signing only protects against tampering with binaries delivered via OTA.  If someone has physical access, they will always be able to flash the device over the serial port.  Signing also does not encrypt anything but the hash (so that it can't be modified), so this does not protect code inside the device: if a user has physical access they can read out your program.

**Securing your private key is paramount.  The same private/public key pair that was used with the original upload must also be used to sign later binaries.  Loss of the private key associated with a binary means that you will not be able to OTA-update any of your devices in the field.  Alternatively, if someone else copies the private key, then they will be able to use it to sign binaries which will be accepted by the Pico.**

Signed Binary Format
^^^^^^^^^^^^^^^^^^^^

The format of a signed binary is compatible with the standard binary format, and can be uploaded to a non-signed Pico via serial or OTA without any conditions.  Note, however, that once an unsigned OTA app is overwritten by this signed version, further updates will require signing.

As shown below, the signed hash is appended to the unsigned binary, followed by the total length of the signed hash (i.e., if the signed hash was 64 bytes, then this uint32 data segment will contain 64).  This format allows for extensibility (such as adding a CA-based validation scheme allowing multiple signing keys all based on a trust anchor). Pull requests are always welcome. (currently it uses SHA256 with RSASSA-PKCS1-V1_5-SIGN signature scheme from RSA PKCS #1 v1.5)

.. code:: bash

    NORMAL-BINARY <SIGNATURE> <uint32 LENGTH-OF-SIGNATURE>

Signed Binary Prerequisites
^^^^^^^^^^^^^^^^^^^^^^^^^^^

OpenSSL is required to run the standard signing steps, and should be available on any UNIX-like or Windows system.  As usual, the latest stable version of OpenSSL is recommended.

Signing requires the generation of an RSA-2048 key (other bit lengths are supported as well, but 2048 is a good selection today) using any appropriate tool.  The following shell commands will generate a new public/private key pair.  Run them in the sketch directory:

.. code:: bash

    openssl genrsa -out private.key 2048
    openssl rsa -in private.key -outform PEM -pubout -out public.key

Automatic Signing
^^^^^^^^^^^^^^^^^

The simplest way of implementing signing is to use the automatic mode, which presently is only possible on Linux and Mac due to some of the tools not being available for Windows.  This mode uses the IDE to configure the source code to enable sigining verification with a given public key, and signs binaries as part of the standard build process using a given public key.

To enable this mode, just include `private.key` and `public.key` in the sketch `.ino` directory.  The IDE will call a helper script (`tools/signing.py`) before the build begins to create a header to enable key validation using the given public key, and to actually do the signing after the build process, generating a `sketch.bin.signed` file.  When OTA is enabled (ArduinoOTA, Web, or HTTP), the binary will automatically only accept signed updates.

When the signing process starts, the message:

.. code:: bash

    Enabling binary signing

will appear in the IDE window before a compile is launched. At the completion of the build, the signed binary file well be displayed in the IDE build window as:

.. code:: bash

    Signed binary: /full/path/to/sketch.bin.signed

If you receive either of the following messages in the IDE window, the signing was not completed and you will need to verify the `public.key` and `private.key`:

.. code:: bash

    Not enabling binary signing
    ... or ...
    Not signing the generated binary

Manual Signing of Binaries
^^^^^^^^^^^^^^^^^^^^^^^^^^

Users may also manually sign executables and require the OTA process to verify their signature.  In the main code, before enabling any update methods, add the following declarations and function call:

.. code:: cpp

    <in globals>
    BearSSL::PublicKey signPubKey( ... key contents ... );
    BearSSL::HashSHA256 hash;
    BearSSL::SigningVerifier sign( &signPubKey );
    ...
    <in setup()>
    Update.installSignature( &hash, &sign );

The above snippet creates a BearSSL public key and a SHA256 hash verifier, and tells the Update object to use them to validate any updates it receives from any method.

Compile the sketch normally and, once a `.bin` file is available, sign it using the signer script:

.. code:: bash

    <PicoArduinoPath>/tools/signing.py --mode sign --privatekey <path-to-private.key> --bin <path-to-unsigned-bin> --out <path-to-signed-binary>

Compression
-----------

The eboot bootloader incorporates a GZIP decompressor, built for very low code requirements.  For applications, this optional decompression is completely transparent.  For uploading compressed filesystems, the application must be built with `ATOMIC_FS_UPDATE` defined because, otherwise, eboot will not be involved in writing the filesystem.

No changes to the application are required.  The `Updater` class and `eboot` bootloader (which performs actual application overwriting on update) automatically search for the `gzip` header in the uploaded binary, and if found, handle it.

Compress an application `.bin` file or filesystem package using any `gzip` available, at any desired compression level (`gzip -9` is recommended because it provides the maximum compression and uncompresses as fast as any other compressino level).  For example:

.. code:: bash

    gzip -9 sketch.bin  # Maximum compression, output sketch.bin.gz
    <Upload the resultant sketch.bin.gz>

If signing is desired, sign the gzip compressed file *after* compression.

.. code:: bash

    gzip -9 sketch.bin
    <PicoPath>/tools/signing.py --mode sign --privatekey <path-to-private.key> --bin sketch.bin.gz --out sketch.bin.gz.signed

Safety
~~~~~~

The OTA process consumes some of the Pico’s resources and bandwidth during upload. Then, the module is restarted and a new sketch executed. Analyse and test how this affects the functionality of the existing and new sketches.

If the Pico is in a remote location and controlling some equipment, you should devote additional attention to what happens if operation of this equipment is suddenly interrupted by the update process. Therefore, decide how to put this equipment into a safe state before starting the update. For instance, your module may be controlling a garden watering system in a sequence. If this sequence is not properly shut down and a water valve is left open, the garden may be flooded.

The following functions are provided with the `ArduinoOTA <https://github.com/earlephilhower/arduino-pico/tree/master/libraries/ArduinoOTA>`__ library and intended to handle functionality of your application during specific stages of OTA, or on an OTA error:

.. code:: cpp

    void onStart(OTA_CALLBACK(fn));
    void onEnd(OTA_CALLBACK(fn));
    void onProgress(OTA_CALLBACK_PROGRESS(fn));
    void onError(OTA_CALLBACK_ERROR (fn));

Uploading from the Arduino IDE
------------------------------

Uploading modules wirelessly from Arduino IDE is intended for the following typical scenarios:

-  During firmware development as a quicker alternative to loading over a serial port,

-  For updating a small number of modules,

-  Only if modules are accessible on the same network as the computer with the Arduino IDE.

-  For all IDE uploads,m the Pico W and the computer must be connected to the same network.

To upload wirelessly from the IDE:

1. Build a sketch starts ``WiFi`` and includes the appropriare calls to ``ArduinoOTA`` (see the examples for reference).  These include the ``ArduinoOTA.begin()`` call in ``setup()`` and periodically calling ``ArduinoOTA.handle();`` from the ``loop()``

2. Upload using standard USB connection the first time.

3. The ``Tools->Port`` should now list ``pico-######`` under the ``Network Ports``.  Select it (you won't be able to use the serial monitor, of course).

4. Try another upload.  It should display the OTA process in place of the serial port upload.

Password Protection
-------------------

Protecting your OTA uploads with password is really straightforward. All you need to do, is to include the following statement in your code:

.. code:: cpp

    ArduinoOTA.setPassword((const char *)"123");

Where ``123`` is a sample password that you should replace with your own.

Before implementing it in your sketch, it is a good idea to check how it works using *BasicOTA.ino* sketch available under *File > Examples > ArduinoOTA*. Go ahead, open *BasicOTA.ino*, uncomment the above statement that is already there, and upload the sketch. To make troubleshooting easier, do not modify example sketch besides what is absolutely required. This is including original simple ``123`` OTA password. Then attempt to upload sketch again (using OTA). After compilation is complete, once upload is about to begin, you should see prompt for password.

Enter the password and upload should be initiated as usual with the only difference being ``Authenticating...OK`` message visible in upload log.

You will not be prompted for a reentering the same password next time. Arduino IDE will remember it for you. You will see prompt for password only after reopening IDE, or if you change it in your sketch, upload the sketch and then try to upload it again.

Please note, it is possible to reveal password entered previously in Arduino IDE, if IDE has not been closed since last upload. This can be done by enabling *Show verbose output during: upload* in *File > Preferences* and attempting to upload the module.




Web Browser
-----------

Updates described in this chapter are done with a web browser that can be useful in the following typical scenarios:

-  after application deployment if loading directly from Arduino IDE is inconvenient or not possible,
-  after deployment if user is unable to expose module for OTA from external update server,
-  to provide updates after deployment to small quantity of modules when setting an update server is not practicable.

Requirements
~~~~~~~~~~~~

-  The Pico and the computer must be connected to the same network, or the IP of the Pico should be known if on a different network.

Implementation Overview
~~~~~~~~~~~~~~~~~~~~~~~

Updates with a web browser are implemented using ``HTTPUpdateServer`` class together with ``WebServer`` and ``LEAmDNS`` classes. The following code is required to get it work:

setup()

.. code:: cpp

        MDNS.begin(host);

        httpUpdater.setup(&httpServer);
        httpServer.begin();

        MDNS.addService("http", "tcp", 80);

loop()

.. code:: cpp

        httpServer.handleClient();

In case OTA update fails dead after entering modifications in your sketch, you can always recover module by loading it over a serial port. Then diagnose the issue with sketch using Serial Monitor. Once the issue is fixed try OTA again.


HTTP Server
-----------

``HTTPUpdate`` class can check for updates and download a binary file from HTTP web server. It is possible to download updates from every IP or domain address on the network or Internet.

Note that by default this class closes all other connections except the one used by the update, this is because the update method blocks. This means that if there's another application receiving data then TCP packets will build up in the buffer leading to out of memory errors causing the OTA update to fail. There's also a limited number of receive buffers available and all may be used up by other applications.

There are some cases where you know that you won't be receiving any data but would still like to send progress updates.
It's possible to disable the default behaviour (and keep connections open) by calling closeConnectionsOnUpdate(false).

Requirements
~~~~~~~~~~~~

-  web server

Arduino code
~~~~~~~~~~~~

Simple updater
^^^^^^^^^^^^^^

Simple updater downloads the file every time the function is called.

.. code:: cpp

    WiFiClient client;
    HTTPUpdate.update(client, "192.168.0.2", 80, "/arduino.bin");

Advanced updater
^^^^^^^^^^^^^^^^

Its possible to point the update function to a script on the server. If a version string argument is given, it will be sent to the server. The server side script can use this string to check whether an update should be performed.

The server-side script can respond as follows: - response code 200, and send the firmware image, - or response code 304 to notify Pico that no update is required.

.. code:: cpp

    WiFiClient client;
    t_httpUpdate_return ret = HTTPUpdate.update(client, "192.168.0.2", 80, "/pico/update/arduino.php", "optional current version string here");
    switch(ret) {
        case HTTP_UPDATE_FAILED:
            Serial.println("[update] Update failed.");
            break;
        case HTTP_UPDATE_NO_UPDATES:
            Serial.println("[update] Update no Update.");
            break;
        case HTTP_UPDATE_OK:
            Serial.println("[update] Update ok."); // may not be called since we reboot the RP2040
            break;
    }

TLS updater
^^^^^^^^^^^

Please read and try the examples provided with the library.

Server request handling
~~~~~~~~~~~~~~~~~~~~~~~

Simple updater
^^^^^^^^^^^^^^

For the simple updater the server only needs to deliver the binary file for update.

Advanced updater
^^^^^^^^^^^^^^^^

For advanced update management a script (such as a PHP script) can run on the server side.  It will receive the following headers which it may use to choose a specific firmware file to serve:

::
        [User-Agent] => Pico-HTTP-Update
        [x-Pico-STA-MAC] => 18:FE:AA:AA:AA:AA
        [x-Pico-AP-MAC] => 1A:FE:AA:AA:AA:AA
        [x-Pico-Version] => DOOR-7-g14f53a19
        [x-Pico-Mode] => sketch


Stream Interface
----------------

The Stream Interface is the base for all other update modes like OTA, HTTP Server / client. Given a Stream-class variable `streamVar` providing `byteCount` bytes of firmware, it can store the firmware as follows:

.. code:: cpp

    Update.begin(firmwareLengthInBytes);
    Update.writeStream(streamVar);
    Update.end();

OTA Bootloader and Memory Map
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

A firmware file is uploaded via any method (Ethernet, WiFi, serial ZModem, etc.) and stored on the LittleFS filesystem as a normal file.  The Updater class (or the underlying PicoOTA) will make a special "OTA command" file on the filesystem, which will be read by the OTA bootloader.  On a reboot, this OTA bootloader will check for an upgrade file, verify its contents, and then perform the requested update and reboot.  If no upgrade file is present, the OTA bootloader simply jumps to the main sketch.

The ROM layout consists of:

.. code:: cpp

    [boot2.S] [OTA Bootloader] [0-pad] [OTA partition table] [Main sketch] [LittleFS filesystem] [EEPROM]

