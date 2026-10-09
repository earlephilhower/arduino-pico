/*
    LFS-on-FreeRTOS Plumbing
    Copyright (c) 2026 Earle F. Philhower, III <earlephilhower@yahoo.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#pragma once

#ifdef __FREERTOS

// Create the startup mutex before the scheduler starts
void __initLFSMutex();

// Send a request to the task.  Will block unless fromISR==true
extern "C" void __lfs(__lfs_op op, void *req, bool fromISR = false);

// Return true if __real_ops are safe (i.e. this is the queue thread)
extern "C" bool __isLFSThread();

#endif
