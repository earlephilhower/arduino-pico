// Can't place library in the src/ directory, Arduino will attempt to build the tests/etc.
// Just have a stub here that redirects to the actual source file

#include "./lfs_local_config.h"

#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#include "../lib/littlefs/lfs.c"
