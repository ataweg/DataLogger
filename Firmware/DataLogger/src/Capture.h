// --------------------------------------------------------------------------
//
// 2020-06-16  AWe   print some statistics to capture file
// 2020-06-03  AWe   initial version
//
// --------------------------------------------------------------------------

// --------------------------------------------------------------------------
//
// MIT License
//
// Copyright (c) 2021 Axel Werner (ataweg)
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
// --------------------------------------------------------------------------

#ifndef __CAPTURE_H__
#define __CAPTURE_H__

#include "SdFat/SdFat.h"       // SdFile

// --------------------------------------------------------------------------
//
// --------------------------------------------------------------------------

// 15 14 13 12 11 10  9  8   7   6  5  4  3  2  1  0
// D7 D6 D5 D4 D3 D2 D1 D0 I2C SIO A5 A4 A3 A2 A1 A0

typedef union
{
   struct
   {
      uint8_t  analog: 6;
      uint8_t  sio: 1;
      uint8_t  i2c: 1;
      uint8_t  digital;
   };
   uint16_t val;         // see also Capture.h
} Source_t;

class Capture
{
private:
   SdFile captureFile;
   Source_t captureSource;
   uint32_t startTime;
   uint32_t sampleTime;
   uint32_t nextSampleTime;
   uint32_t nextSampleSioTime;
   uint32_t nextSampleI2cTime;
   uint32_t sampleCount;

public:
   Capture( void );
   ~Capture( void );

   SdFile *file( void )    { return &captureFile; }
   Source_t source( void ) { return captureSource; }

   bool setup( void );
   bool start( void );
   bool run( void );
   bool stop( void );
};

// --------------------------------------------------------------------------
//
// --------------------------------------------------------------------------
#endif // __CAPTURE_H__
