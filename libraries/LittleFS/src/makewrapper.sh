#!/bin/bash
../../../tools/makewrapper.py  -s ../lib/littlefs/lfs.h -p wrap_lfs -t __isLFSThread -m LFSMutex -q __lfs
exho Make sure to manually fix 'traverse' implementations
