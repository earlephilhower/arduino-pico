:orphan:

BearSSL WiFi Classes
--------------------

Methods and properties described in this section are specific to the Raspberry Pi Pico W and the ESP8266. They are not covered in `Arduino WiFi library <https://www.arduino.cc/en/Reference/WiFi>`__ documentation. Before they are fully documented please refer to information below.

The `BearSSL <https://bearssl.org>`__ library (with modifications for ESP8266 compatibility and to use ROM tables whenever possible) is used to perform all cryptography and TLS operations.  The main ported repo is available `on GitHub <https://github.com/earlephilhower/bearssl-esp8266>`__.

CPU Requirements
~~~~~~~~~~~~~~~~

SSL operations take significant CPU cycles to run, so it will connect significantly slower than unprotected connections on the Pico, but the actual data transfer rates once connected are similar.

See the section on `sessions <#sessions-resuming-connections-fast>`__ and `limiting cryptographic negotiation <#limiting-ciphers-new-connections-faster>`__ for ways of ensuring faster modes are used.

Memory Requirements
~~~~~~~~~~~~~~~~~~~
BearSSL doesn't perform memory allocations at runtime, but it does require allocation of memory at the beginning of a connection.  There are two memory chunks required:
. A per-application secondary stack
. A per-connection TLS receive/transmit buffer plus overhead

The per-application secondary stack is approximately 7KB in size and is used for temporary variables during BearSSL processing.  Only one stack is required, and it will be allocated whenever any `BearSSL::WiFiClientSecure` or `BearSSL::WiFiServerSecure` are instantiated.  So, in the case of a global client or server, the memory will be allocated before `setup()` is called.

The per-connection buffers are approximately 22KB in size, but in certain circumstances it can be reduced dramatically by using MFLN or limiting message sizes.  See the `MLFN section <#mfln-or-maximum-fragment-length-negotiation-saving-ram>`__ below for more information.

Object Lifetimes
~~~~~~~~~~~~~~~~

There are many configuration options that require passing in a pointer to an object (i.e. a pointer to a private key, or a certificate list).  In order to preserve memory, BearSSL does NOT copy the objects passed in via these pointers and as such any pointer passed in to BearSSL needs to be preserved for the life of the client object.  For example, the following code is **in error**:

.. code:: cpp

    BearSSL::WiFiClientSecure client;
    const char x509CA PROGMEM = ".......";
    void setup() {
        BearSSL::X509List x509(x509CA);
        client.setTrustAnchor(&x509);
    }
    void loop() {
        client.connect("192.168.1.1", 443);
    }

Because the pointer to the local object `x509` no longer is valid after setup(), expect to crash in the main `loop()` where it is accessed by the `client` object.

As a rule, either keep your objects global, use `new` to create them, or ensure that all objects needed live inside the same scope as the client.

TLS and HTTPS Basics
~~~~~~~~~~~~~~~~~~~~

The following discussion is only intended to give a rough idea of TLS/HTTPS(which is just HTTP over a TLS connection) and the components an application needs to manage to make a TLS connection.  For more detailed information, please check the relevant `RFC 5246 <https://tools.ietf.org/search/rfc5246>`__ and others.

TLS can be broken into two stages: verifying the identities of server (and potentially client), and then encrypting blocks of data bidirectionally.  Verifying the identity of the other partner is handled via keys encoded in X509 certificates, optionally signed by a series of other entities.


Public and Private Keys
~~~~~~~~~~~~~~~~~~~~~~~

Cryptographic keys are required for many of the BearSSL functions.  Both public and private keys are supported, with either Elliptic Curve or RSA key support.

To generate a public or private key from an existing PEM (ASCII format) or DER (binary format), the simplest method is to use the constructor:

.. code:: cpp

    BearSSL::PublicKey(const char *pemString)
    ... or ...
    BearSSL::PublicKey(const uint8_t *derArray, size_t derLen)

Note that `PROGMEM` strings and arrays are natively supported by these constructors and no special `*_P` modes are required.  There are additional functions to identify the key type and access the underlying BearSSL proprietary types, but they are not needed by user applications.

TLS Sessions
~~~~~~~~~~~~

TLS supports the notion of a session (completely independent and different from HTTP sessions) which allow clients to reconnect to a server without having to renegotiate encryption settings or validate X509 certificates.  This can save significant time (3-4 seconds in the case of EC keys) and can help save power by allowing the ESP8266 to sleep for a long time, reconnect and transmit some samples using the SSL session, and then jump back to sleep quicker.

`BearSSL::Session` is an opaque class.  Use the `BearSSL::WiFiClientSecure.setSession(&BearSSLSession)` method to apply it before the first `BearSSL::WiFiClientSecure.connect()` and it will be updated with session parameters during the operation of the connection.  After the connection has had `.close()` called on it, serialize the `BearSSL::Session` object to stable storage (EEPROM, RTC RAM, etc.) and restore it before trying to reconnect.  See the `BearSSL_Sessions` example for a detailed example.

`Sessions <#sessions-resuming-connections-fast>`__ contains additional information on the sessions API.

X.509 Certificate(s)
~~~~~~~~~~~~~~~~~~~~

X509 certificates are used to identify peers in TLS connections.  Normally only the server identifies itself, but the client can also supply an X509 certificate if desired (this is often done in MQTT applications).  The certificate contains many fields, but the most interesting in our applications are the name, the public key, and potentially a chain of signing that leads back to a trusted authority (like a global internet CA or a company-wide private certificate authority).

Any call that takes an X509 certificate can also take a list of X509 certificates, so there is no special `X509` class, simply `BearSSL::X509List` (which may only contain a single certificate).

Generating a certificate to be used to validate using the constructor

.. code:: cpp

    BearSSL::X509List(const char *pemX509);
    ...or...
    BearSSL::X509List(const uint8_t *derCert, size_t derLen);

If you need to add additional certificates (unlikely in normal operation), the `::append()` operation can be used.


Certificate Stores
~~~~~~~~~~~~~~~~~~

The web browser you're using to read this document keeps a list of 100s of certification authorities (CAs) worldwide that it trusts to attest to the identity of websites.

In many cases your application will know the specific CA it needs to validate web or MQTT servers against (often just a single, self-signing CA private to your institution).  Simply load your private CA in a `BearSSL::X509List` and use that as your trust anchor.

However, there are cases where you will not know beforehand which CA you will need (i.e. a user enters a website through a keypad), and you need to keep the list of CAs just like your web browser.  In those cases, you need to generate a certificate bundle on the PC while compiling your application, upload the `certs.ar` bundle to LittleFS or SD when uploading your application binary, and pass it to a `BearSSL::CertStore()` in order to validate TLS peers.

See the `BearSSL_CertStore` example for full details.

Supported Crypto
~~~~~~~~~~~~~~~~

Please see the `BearSSL website <https://bearssl.org>`__ for detailed cryptographic information.  In general, TLS 1.2, TLS 1.1, and TLS 1.0 are supported with RSA and Elliptic Curve keys and a very rich set of hashing and symmetric encryption codes.  Please note that Elliptic Curve (EC) key operations take a significant amount of time.

