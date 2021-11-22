// --------------------------------------------------------------------------
//
// Project       DataLogger
//
// File          DataLogger.ino
//
// Author        Axel Werner
//
// Comment       using "Arduino UNO" board and Greiman SdFat library
//
// --------------------------------------------------------------------------
// Changelog
//
// 2020-06-14  AWe   move all to DataLogger.cpp, but BuildMsg, because this file is
//                   always compile, so we have the current build date and time
// 2020-06-01  AWe   moved tasks to their own files
// 2020-05-29  AWe   add extended support for sdcard
// 2020-04-16  AWe   initial version taken from
//                      https://www.arduino.cc/en/Tutorial/Datalogger
//                      F:\Arduino\libraries\SD\examples\Datalogger\Datalogger.ino
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

#include <avr/pgmspace.h>
#include "src/DataLogger.h"

// --------------------------------------------------------------------------
//
// --------------------------------------------------------------------------

const char str_BuildMsg_0[] PROGMEM = "DataLogger (c) AWe 2021" ;
const char str_BuildMsg_1[] PROGMEM = "Made " __DATE__ " " __TIME__;
const char str_BuildMsg_2[] PROGMEM = "libc version " __AVR_LIBC_VERSION_STRING__;
const char str_BuildMsg_3[] PROGMEM = "gcc version  " __VERSION__;

const char * const PROGMEM str_BuildMsg[] PROGMEM =
{
   str_BuildMsg_0,
   str_BuildMsg_1,
   str_BuildMsg_2,
   str_BuildMsg_3,
};

const uint8_t num_buildmsg_str = sizeof( str_BuildMsg ) / sizeof( char * );

// --------------------------------------------------------------------------
//
// --------------------------------------------------------------------------
