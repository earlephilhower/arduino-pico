Contributing and Porting to the Core
====================================

First of all, thank you for contributing to the project.  It's a lot of work
keeping up with all the different uses of the RP2040, so the more people
working on the code, the better.  Your assistance can help the project
succeed.

Contributing to the Core (Pull Requests)
----------------------------------------

We use the standard GitHub Pull Request model.  If you're unfamiliar with it,
this `guide <https://www.freecodecamp.org/news/how-to-make-your-first-pull-request-on-github-3/>` gives a simple overview of the process.

All pull requests have to pass a set of Continuous Integration (CI) checks
which help make sure the code compiles under different configurations and has
no spelling or style errors.

Tips for a Good Pull Request (PR)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

All code in the core and libraries, except for INO sketches, uses a 4-space
indent with cuddled brackets.  When in doubt, copy your formatting from the
surrounding code.  You should install ``astyle`` and run ``tests/restyle.sh``
on your machine before committing and pushing any pull requests to ensure
the formatting is correct.

Describe the change you're proposing and why it's important in your
``git commit`` message.  If it fixes an open issue, place ``Fixes #xxxx``
(where xxxx is the issue number) in the message to link the two.

Try and only change one thing per pull request.  That makes it easier to
review and prioritize.  Opening up a separate PR per change also helps keep
track of them when release messages are generated.

Porting Libraries and Applications to the Core
----------------------------------------------

We try and follow Arduino standards so, with luck, porting to this core should
be relatively straightforward.  The ``WiFi`` library and associates support
libraries like ``WebServer`` are modeled after the ESP32 and ESP8266 versions
of those libraries, combined with the "standard" Arduino ``WiFi`` one.

Compiler Defines for Porting
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

If you are adding RP2040 support to an existing library and need to isolate
code that only runs on this core, use the following define.

.. code:: cpp

        #if defined(ARDUINO_ARCH_RP2040)
        ~~~ your changes ~~~
        #endif

Library Architectures
~~~~~~~~~~~~~~~~~~~~~

After adding support in the code, libraries need their ``library.properties``
and ``library.json`` files updated to indicate support, or the IDE will
not know your new code is compatible here.

Add ``rp2040`` to ``architectures`` (in ``library.properties``) and
``"rp2040"`` to ``platforms[]`` (in ``library.json``) to let the tools know.
