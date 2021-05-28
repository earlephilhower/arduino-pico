/*
  Copyright (c) 2014 Arduino.  All right reserved.

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

#ifdef __cplusplus

#ifndef _RING_BUFFER_
#define _RING_BUFFER_

#include <stdint.h>
#include <string.h>

namespace arduino {

// Define constants and variables for buffering incoming serial data.  We're
// using a ring buffer (I think), in which head is the index of the location
// to which to write the next incoming character and tail is the index of the
// location from which to read.
#define SERIAL_BUFFER_SIZE 64

template <int N>
class RingBufferN
{
  public:
    uint8_t _aucBuffer[N] ;
    volatile int _iHead ;
    volatile int _iTail ;
    volatile int _numElems;

  public:
    RingBufferN( void ) ;
    void store_char( uint8_t c ) ;
    void clear();
    int read_char();
    int available();
    int availableForStore();
    int peek();
    bool isFull();

  private:
    int nextIndex(int index);
    inline bool isEmpty() const { return (_numElems == 0); }
};

typedef RingBufferN<SERIAL_BUFFER_SIZE> RingBuffer;


template <int N>
RingBufferN<N>::RingBufferN( void )
{
    memset( _aucBuffer, 0, N ) ;
    clear();
}

template <int N>
void RingBufferN<N>::store_char( uint8_t c )
{
  // if we should be storing the received character into the location
  // just before the tail (meaning that the head would advance to the
  // current location of the tail), we're about to overflow the buffer
  // and so we don't write the character or advance the head.
  if (!isFull())
  {
    _aucBuffer[_iHead] = c ;
    _iHead = nextIndex(_iHead);
    _numElems++;
  }
}

template <int N>
void RingBufferN<N>::clear()
{
  _iHead = 0;
  _iTail = 0;
  _numElems = 0;
}

template <int N>
int RingBufferN<N>::read_char()
{
  if (isEmpty())
    return -1;

  uint8_t value = _aucBuffer[_iTail];
  _iTail = nextIndex(_iTail);
  _numElems--;

  return value;
}

template <int N>
int RingBufferN<N>::available()
{
  return _numElems;
}

template <int N>
int RingBufferN<N>::availableForStore()
{
  return (N - _numElems);
}

template <int N>
int RingBufferN<N>::peek()
{
  if (isEmpty())
    return -1;

  return _aucBuffer[_iTail];
}

template <int N>
int RingBufferN<N>::nextIndex(int index)
{
  return (uint32_t)(index + 1) % N;
}

template <int N>
bool RingBufferN<N>::isFull()
{
  return (_numElems == N);
}

}

#endif /* _RING_BUFFER_ */
#endif /* __cplusplus */