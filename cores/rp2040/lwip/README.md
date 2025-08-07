# Overview

This directory #include points to the SDK's LWIP implementation.  We remove
any prebuilt LWIP files at the makelibpico and allow the IDE to semect
custom definitions for the build at app compile time.  This removes the
need for multiple libraries to be built and distributed for things like IPV6
or large buffers.

# Usage

Run ./update.sh when the SDK's LWIP version changes to ensure any
new or renamed files are added.  Then `git add` any new ones and
`git commit`.

