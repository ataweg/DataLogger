// --------------------------------------------------------------------------
//
// Project       DriverMonitor
//
// File          str2time.c
//
// Author        Axel Werner
//
// --------------------------------------------------------------------------
// Changelog
//
// 2019-07-30  AWe   initial implementation
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

#include <stdlib.h>     // strtol()
#include "str2time.h"

// --------------------------------------------------------------------------
//
// --------------------------------------------------------------------------

// hh:mm:ss.ms
// return time in milli seconds

// 31 bit, max value 2147483648  (10 digits + 1 for sign)
// 2147483648 => 2.147.483,648
// 596:31:23,648

int32_t str2time( char *str )
{
   int32_t time = 0;
   int32_t value;
   char* endptr;

   while( *str )
   {
      value = strtol( str, &endptr, 10 );
      time += value;

      if( *endptr == ':' )
      {
         time *= 60L;
         endptr++;
      }
      else if( *endptr == '.' )
      {
         time *= 1000L;
         endptr++;
#if 0  // for more than 3 digitgs after the decimal point
         char c = *endptr;
         unsigned char m = 100;

         while( c )
         {
            if( c < '0' || c > '9' )
               break;
            c -= '0';
            time += c * m;
            m /= 10;
            c = *++endptr;
         }
#endif
      }
      str = endptr;
   }
   return time;
}

// --------------------------------------------------------------------------
//
// --------------------------------------------------------------------------
