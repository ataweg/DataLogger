// --------------------------------------------------------------------------
//
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

#ifndef __CONFIG_H__
#define __CONFIG_H__

#include "Capture.h"

// --------------------------------------------------------------------------
//
// --------------------------------------------------------------------------

enum
{
   FileName,
   FileType,
   FileSize,
   CaptureSource,
   SamplingRate,
   SerialSamplingRate,
   I2cSamplingRate,
   StartSample,
   StopSample,
   SerialBaudrate,
   SerialBits,
   SerialParity,
   SerialStopBits,
   SystemTime
};

enum
{
   Text = 1,
   Number,
   Flag,
   Symbol,
};

typedef struct
{
   const char *name;
   uint8_t     length;
   uint8_t     id;
   uint8_t     type;
}  Param_t;

typedef union
{
   char *text;
   uint32_t val;
} ParamValue_t;

typedef struct
{
   char     ConfigFileName[ 12 ];   // cannot changed
   char     FileName[ 12 ];         // 8.3 filename
   uint8_t  FileType;
   uint32_t FileSize;
   Source_t CaptureSource;
   uint32_t SamplingRate;
   uint32_t SerialSamplingRate;
   uint32_t I2cSamplingRate;
   char     StartSample[ 9 ];
   char     StopSample[ 9 ];
   uint32_t SerialBaudrate;
   uint8_t  SerialBits;
   uint8_t  SerialParity;
   uint8_t  SerialStopBits;
   uint32_t SystemTime;
} Settings;

// --------------------------------------------------------------------------
//
// --------------------------------------------------------------------------

extern Settings settings;

// --------------------------------------------------------------------------
//
// --------------------------------------------------------------------------

bool getConfiguration( void );

// --------------------------------------------------------------------------
//
// --------------------------------------------------------------------------
#endif // __CONFIG_H__
