:orphan:

WiFiClientSecure Class
======================

`BearSSL::WiFiClientSecure` is the object which actually handles TLS encrypted WiFi connections to a remote server or client.  It extends `WiFiClient` and so can be used with minimal changes to code that does unsecured communications.

Validating X509 Certificates (Am I talking to the server I think I'm talking to?)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Prior to connecting to a server, the `BearSSL::WiFiClientSecure` needs to be told how to verify the identity of the other machine.  **By default BearSSL will not validate any connections and will refuse to connect to any server.**

There are multiple modes to tell BearSSL how to verify the identity of the remote server.  See the `BearSSL_Validation` example for real uses of the following methods:

setInsecure()
^^^^^^^^^^^^^

Don't verify any X509 certificates.  There is no guarantee that the server connected to is the one you think it is in this case.

setKnownKey(const BearSSL::PublicKey \*pk)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Assume the server is using the specific public key.  This does not verify the identity of the server or the X509 certificate it sends, it simply assumes that its public key is the one given.  If the server updates its public key at a later point then connections will fail.

setFingerprint(const uint8_t fp[20]) / setFingerprint(const char \*fpStr)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Verify the SHA1 fingerprint of the certificate returned matches this one.  If the server certificate changes, it will fail.  If an array of 20 bytes are sent in, it is assumed they are the binary SHA1 values.  If a `char*` string is passed in, it is parsed as a series of human-readable hex values separated by spaces or colons (e.g. `setFingerprint("00:01:02:03:...:1f");`)

This fingerprint is calculated on the raw X509 certificate served by the server.  In very rare cases, these certificates have certain encodings which should be normalized before taking a fingerprint (but in order to preserve memory BearSSL does not do this normalization since it would need RAM for an entire copy of the cert), and the fingerprint BearSSL calculates will not match the fingerprint OpenSSL calculates.  In this case, you can enable SSL debugging and get a dump of BearSSL's calculated fingerprint and use that one in your code, or use full certificate validation.  See the `original issue and debug here <https://github.com/esp8266/Arduino/issues/6209>`__.

setTrustAnchors(BearSSL::X509List \*ta)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Use the passed-in certificate(s) as a trust anchor, accepting remote certificates signed by any of these.  If you have many trust anchors it may make sense to use a `BearSSL::CertStore` because it will only require RAM for a single trust anchor (while the `setTrustAnchors` call requires memory for all certificates in the list).

setX509Time(time_t now)
^^^^^^^^^^^^^^^^^^^^^^^

For `setTrustAnchors` and `CertStore` , the current time (set via SNTP) is used to verify the certificate against the list, so SNTP must be enabled and functioning before the connection is attempted.  If you cannot use SNTP for some reason, you can manually set the "present time" that BearSSL will use to validate a certificate with this call where `now` is standard UNIX time.

Client Certificates (Proving I'm who I say I am to the server)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

TLS servers can request that a client identify themselves with an X509 certificate signed by a trust anchor it honors (i.e. a global TA or a private CA).  This is commonly done for applications like MQTT.  By default the client doesn't send a certificate, and in cases where a certificate is required the server will disconnect and no connection will be possible.

setClientRSACert / setClientECCert
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Sets a client certificate to send to a TLS server that requests one.  It should be called before `connect()` to add a certificate to the client in case the server requests it.  Note that certificates include both a certificate and a private key.  Both should be provided to you by your certificate generator.  Elliptic Curve (EC) keys require additional information, as shown in the prototype.

MFLN or Maximum Fragment Length Negotiation (Saving RAM)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Because TLS was developed on systems with many megabytes of memory, they require by default a 16KB buffer for receive and transmit.  That's enormous for the ESP8266, which has only around 40KB total heap available.

We can (and do) minimize the transmission buffer down to slightly more than 512 bytes to save memory, since BearSSL can internally ensure transmissions larger than that are broken up into smaller chunks that do fit.  But that still leaves the 16KB receive buffer requirement since we cannot in general guarantee the TLS peer will send in smaller chunks.

TLS 1.2 added MFLN, which lets a client negotiate smaller buffers with a server and reduce the memory requirements on the ESP8266.  Unfortunately, BearSSL needs to know the buffer sizes before it begins connection, so applications that want to use smaller buffers need to check the remote server's support before `connect()` .

probeMaxFragmentLength(host, port, len)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Use one of these calls **before** connection to determine if a specific fragment length is supported (len must be a power of two from 512 to 4096, per the specification).  This does **not** initiate a SSL connection, it simply opens a TCP port and performs a trial handshake to check support.

setBufferSizes(int recv, int xmit)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Once you have verified (or know beforehand) that MFLN is supported you can use this call to set the size of memory buffers allocated by the connection object.  This must be called **before** `connect()` or it will be ignored.

In certain applications where the TLS server does not support MFLN (not many do as of this writing as it is relatively new to OpenSSL), but you control both the ESP8266 and the server to which it is communicating, you may still be able to `setBufferSizes()` smaller if you guarantee no chunk of data will overflow those buffers.

bool getMFLNStatus()
^^^^^^^^^^^^^^^^^^^^

After a successful connection, this method returns whether or not MFLN negotiation succeeded or not.  If it did not succeed, and you reduced the receive buffer with `setBufferSizes` then you may experience reception errors if the server attempts to send messages larger than your receive buffer.

Sessions (Resuming connections fast)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

setSession(BearSSL::Session &sess)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

If you are connecting to a server repeatedly in a fixed time period (usually 30 or 60 minutes, but normally configurable at the server), a TLS session can be used to cache crypto settings and speed up connections significantly.

Errors
~~~~~~

BearSSL can fail in many more unique and interesting ways.  Use these calls to get more information when something fails.  

getLastSSLError(char \*dest = NULL, size_t len = 0)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Returns the last BearSSL error code encountered and optionally set a user-allocated buffer to a human-readable form of the error.  To only get the last error integer code, just call without any parameters (`int errCode = getLastSSLError();`).

Limiting Ciphers (New connections faster)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

There is very rarely reason to use these calls, but they are available.

setCiphers()
^^^^^^^^^^^^

Takes an array (in PROGMEM is valid) or a std::vector of 16-bit BearSSL cipher identifiers and restricts BearSSL to only use them.  If the server requires a different cipher, then connection will fail.  Generally this is not useful except in cases where you want to connect to servers using a specific cipher.  See the BearSSL headers for more information on the supported ciphers.

setCiphersLessSecure()
^^^^^^^^^^^^^^^^^^^^^^

Helper function which essentially limits BearSSL to less secure ciphers than it would natively choose, but they may be helpful and faster if your server depended on specific crypto options.

Limiting TLS(SSL) Versions
~~~~~~~~~~~~~~~~~~~~~~~~~~

By default, BearSSL will connect with TLS 1.0, TLS 1.1, or TLS 1.2 protocols (depending on the request of the remote side).  If you want to limit to a subset, use the following call:

setSSLVersion(uint32_t min, uint32_t max)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Valid values for min and max are `BR_TLS10`, `BR_TLS11`, `BR_TLS12`.  Min and max may be set to the same value if only a single TLS version is desired.


ESP32 Compatibility
===================
Simple ESP32 ``WiFiClientSecure`` compatibility is built-in, allow for some sketches to run without any modification.
The following methods are implemented:

.. code :: cpp

    void setCACert(const char *rootCA);
    void setCertificate(const char *client_ca);
    void setPrivateKey(const char *private_key);
    bool loadCACert(Stream& stream, size_t size);
    bool loadCertificate(Stream& stream, size_t size);
    bool loadPrivateKey(Stream& stream, size_t size);
    int connect(IPAddress ip, uint16_t port, int32_t timeout);
    int connect(const char *host, uint16_t port, int32_t timeout);
    int connect(IPAddress ip, uint16_t port, const char *rootCABuff, const char *cli_cert, const char *cli_key);
    int connect(const char *host, uint16_t port, const char *rootCABuff, const char *cli_cert, const char *cli_key);

Note that the SSL backend is very different between Arduino-Pico and ESP32-Arduino (BearSSL vs. mbedTLS).  This means
that, for instance, the SSL connection will check valid dates of certificates (and hence require system time to be
set on the Pico, which is automatically done in this case).

TLS-Pre Shared Keys (PSK) is not supported by BearSSL, and hence not implemented here.  Neither is ALPN.

For more advanced control, it is recommended to port to the native Pico calls which allows much more flexibility and control.
