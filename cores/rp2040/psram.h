/**
    @file sfe_psram.c

    @brief This file contains a function that is used to detect and initialize PSRAM on
    SparkFun rp2350 boards.
*/

/*
    The MIT License (MIT)

    Copyright (c) 2024 SparkFun Electronics

    Permission is hereby granted, free of charge, to any person obtaining a
    copy of this software and associated documentation files (the "Software"),
    to deal in the Software without restriction, including without limitation
    the rights to use, copy, modify, merge, publish, distribute, sublicense,
    and/or sell copies of the Software, and to permit persons to whom the
    Software is furnished to do so, subject to the following conditions: The
    above copyright notice and this permission notice shall be included in all
    copies or substantial portions of the Software. THE SOFTWARE IS PROVIDED
    "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT
    NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
    PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
    HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
    ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
    CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

*/

#pragma once

#include <Arduino.h>

void psram_reinit_timing(uint32_t hz = 0);
void *__psram_malloc(size_t size);
void __psram_free(void *ptr);
void *__psram_realloc(void *ptr, size_t size);
void *__psram_calloc(size_t num, size_t size);
size_t __psram_largest_free_block();
size_t __psram_total_space();
size_t __psram_total_used();
