Network Time Protocol (NTP)
===========================

NTP allows the Pico to set its internal clock using the internet, and is
required for secure connections because the certificates used have valid
date stamps.

After ``WiFi.begin()`` use ``NTP.begin(s1)`` or ``NTP,begin(s1, s2)``  to
use one or two NTP servers (common ones are ``pool.ntp.org`` and
``time.nist.gov``) .

.. code :: cpp

    WiFi.begin("ssid", "pass");
    NTP.begin("pool.ntp.org", "time.nist.gov");

Either names or ``IPAddress`` may be used to identify the NTP server to
use.

It may take seconds to minutes for the system time to be updated by NTP,
depending on the server.  It is often useful to check that ``time(NULL)``
returns a sane value before continuing a sketch:

.. code :: cpp

    void setClock() {
      NTP.begin("pool.ntp.org", "time.nist.gov");

      Serial.print("Waiting for NTP time sync: ");
      time_t now = time(nullptr);
      while (now < 8 * 3600 * 2) {
        delay(500);
        Serial.print(".");
        now = time(nullptr);
      }
      Serial.println("");
      struct tm timeinfo;
      gmtime_r(&now, &timeinfo);
      Serial.print("Current time: ");
      Serial.print(asctime(&timeinfo));
    }

bool NTP.waitSet(uint32_t timeout)
----------------------------------
This call will wait up to timeout milliseconds for the time to be set, and returns
success or failure.  It will also begin NTP with a default "pool.ntp.org" server if
it is not already running.  Using this method, the above code becomes:

.. code :: cpp

    void setClock() {
      NTP.begin("pool.ntp.org", "time.nist.gov");
      NTP.waitSet();
      time_t now = time(nullptr);
      struct tm timeinfo;
      gmtime_r(&now, &timeinfo);
      Serial.print("Current time: ");
      Serial.print(asctime(&timeinfo));
    }

bool NTP.waitSet(void (\*cb)(), uint32_t timeout)
-------------------------------------------------
Allows for a callback that will be called every 1/10th of a second while waiting for
NTP sync.  For example, using lambdas you can simply print "."s:"

.. code :: cpp

    void setClock() {
      NTP.begin("pool.ntp.org", "time.nist.gov");
      NTP.waitSet([]() { Serial.print("."); });
      time_t now = time(nullptr);
      struct tm timeinfo;
      gmtime_r(&now, &timeinfo);
      Serial.print("Current time: ");
      Serial.print(asctime(&timeinfo));
    }
