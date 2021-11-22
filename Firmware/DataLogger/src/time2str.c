// --------------------------------------------------------------------------
//
// Project       DriverMonitor
//
// File          time2str.c
//
// Author        Axel Werner
//
// --------------------------------------------------------------------------
// Changelog
//
// 2019-11-06  AWe   suppress the printing of zero, for 0h0m, 0hm or 0m
// 2019-10-31  AWe   fix issue with negative time value
// 2019-09-30  AWe   for esp32 use itoa() instead of ltoa(), because we have
//                     only 32 bit value
// 2019-01-30  AWe   support for negative time values
//                   remove eresulution parameter
// 2018-12-28  AWe   adapted to use in DriverMonitor project
//                     correct sprint parameter to handle long values
// 2018-06-26  AWe   remove us, to be compatible with esp-idf
// 2018-06-26  AWe   initial implementation, used in esp_log.h
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

#include <stdlib.h>     // ltoa()
#include <string.h>     // strlen()
#include "time2str.h"

// --------------------------------------------------------------------------
//
// --------------------------------------------------------------------------

// time in milli seconds, can be negative
// 31 bit, max value 2147483648  (10 digits + 1 for sign)
// 2147483648 => 2.147.483,648
// hh:mm:ss.ms
// 596:31:23,648

// valid/used format strings are
//   "hms.d"
//   "s.dd"
//   "0h0ms.d"

static void div_mod( uint32_t divident, uint32_t divisor, uint16_t *result, uint32_t *remainder )
{
   *result = divident / divisor;
   *remainder = divident % divisor;
}

char* time2str( char *buffer, uint16_t buffer_size, const int32_t time, const char *format )
{
   char *ptr = buffer;        // pointer to put in the next char
   char *buf = buffer;        // return the begin of the buffer
   int   print_zero = 0;      // print zero value
   uint32_t rest;
   uint16_t hh, mm, ss, ms;

   if( time < 0 )
   {
      rest = -time;
      *ptr++ = '-';
      buffer++;      // has printed negative sign
   }
   else
      rest = time;

   if( *format == '0' )
      format++;            // don't print zero value
   else
      print_zero = 1;

   if( *format == 'h' )
   {
      format++;
      div_mod( rest, ( 60UL * 60 * 1000 ), &hh, &rest );
      if( hh || print_zero )
      {
         ltoa( hh, ptr, 10 );
         ptr += strlen( ptr );
         print_zero = 1;  // print the following zero values
      }

      if( *format == '0' )
         format++;
      else
         print_zero = 1;
   }

   if( *format == 'm' )
   {
      format++;
      div_mod( rest, ( 60UL * 1000 ), &mm, &rest );
      if( ptr != buffer )     // has printed something, but not the minus sign
      {
         *ptr++ = ':';
         if( mm < 10 )
            *ptr++ = '0';
      }

      if( mm || print_zero )
      {
         ltoa( mm, ptr, 10 );
         ptr += strlen( ptr );
      }
   }

   if( *format == 's' )
   {
      format++;
      div_mod( rest, 1000, &ss, &rest );
      if( ptr != buffer )
      {
         *ptr++ = ':';
         if( ss < 10 )
            *ptr++ = '0';
      }
      ltoa( ss, ptr, 10 );
      ptr += strlen( ptr );
   }

   if( *format == '.' )
   {
      format++;
      uint16_t resolution = 1000;

      *ptr++ = '.';
      while( *format == 'd' )
      {
         format++;
         resolution = resolution / 10;
         uint8_t a = rest / resolution;
         rest -= a * resolution;
         *ptr++ = '0' + a;
      }
   }

   *ptr = 0;
   return buf;
}

// --------------------------------------------------------------------------
//
// --------------------------------------------------------------------------
