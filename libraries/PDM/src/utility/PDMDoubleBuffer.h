/*
  Copyright (c) 2016 Arduino LLC.  All right reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the GNU Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef _PDM_DOUBLE_BUFFER_H_INCLUDED
#define _PDM_DOUBLE_BUFFER_H_INCLUDED

#include <stddef.h>
#include <stdint.h>

#define DEFAULT_PDM_BUFFER_SIZE 512

class PDMDoubleBuffer
{
public:
  PDMDoubleBuffer();
  virtual ~PDMDoubleBuffer();

  void setSize(int size);
  size_t getSize();

  void reset();

  size_t availableForWrite();
  size_t write(const void *buffer, size_t size);
  size_t read(void *buffer, size_t size);
  size_t peek(void *buffer, size_t size);
  void* data();
  size_t available();
  void swap(int length = 0);

private:
  uint8_t* _buffer[2] __attribute__((aligned (16)));
  int _size;
  volatile int _length[2];
  volatile int _readOffset[2];
  volatile int _index; 
};

#endif
