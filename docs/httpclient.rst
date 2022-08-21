HTTPClient Library
==================

A simple HTTP requestor that can handle both HTTP and HTTP requests is
included as the ``HTTPClient`` library.

Check the examples for use under HTTP and HTTPS configurations.  In general,
for HTTP connections (unsecured and very uncommon on the internet today) simply
passing in a URL and performiung a GET is sufficient to transfer data.

.. code:: cpp

    // Error checking is left as an exercise for the reader...
    HTTPClient http;
    if (http.begin("http://my.server/url")) {
        if (http.GET() > 0) {
            String data = http.getString();
        }
        http.end();
    }

For HTTPS connections, simply add the appropriate WiFiClientSecure calls
as needed (i.e. ``setInsecure()``, ``setTrustAnchor``, etc.).  See the
WiFiClientSecure documentation for more details.

.. code:: cpp

    // Error checking is left as an exercise for the reader...
    HTTPClient https;
    https.setInsecure(); // Use certs, but do not check their authenticity
    if (https.begin("https://my.secure.server/url")) {
        if (http.GET() > 0) {
            String data = http.getString();
        }
        http.end();
    }

Unlike the ESP8266 and ESP32 ``HTTPClient`` implementations it is not necessary
to create a ``WiFiClient`` or ``WiFiClientSecure`` to pass in to the ``HTTPClient``
object.
