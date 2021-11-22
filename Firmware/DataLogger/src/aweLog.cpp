// ------------------------------------------------------------------------------
//
// 2020-06-03  AWe   add debugHelper
// 2020-05-29  AWe   fix issue when __brkval is not set
// 2019-08-08  AWe   add #include "aweLog_config.h" to aweLog.h
//                     move #define PRINTF_BUFFER_SIZE to there
// 2019-02-01  AWe   redesign aweLog, move TAG to flash, optimize memory usage
// 2019-01-31  AWe   remove unused variable tag from _log_write()
// 2019-01-28  AWe   add freememory(), based on
//                      https://learn.adafruit.com/memories-of-an-arduino/measuring-free-memory
// 2019-01-07  AWe   optimized for AVR chips
// 2018-12-21  AWe   take it from https://playground.arduino.cc/main/printf
//
// ------------------------------------------------------------------------------

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

// --------------------------------------------------------------------------
// debug support
// --------------------------------------------------------------------------

#define LOG_LOCAL_LEVEL    LOG_NONE
#include "aweLog.h"
static const char TAG[] PROGMEM = tag( "aweLog" );

// --------------------------------------------------------------------------
//
// --------------------------------------------------------------------------

#include <Arduino.h>          // Serial, millis(), ...
#include <stdio.h>            // vsnprintf_P(), vsnprintf()
#include <stdarg.h>

// #include "aweLog.h"

// --------------------------------------------------------------------------
//
// --------------------------------------------------------------------------
// see also https://stackoverflow.com/questions/54747439/what-is-brkval-for-avr-and-where-is-it-defined

#ifdef __arm__
   // should use uinstd.h to define sbrk but Due causes a conflict
   extern "C" char* sbrk( int incr );
#else  // __ARM__
   extern char *__brkval;
#endif  // __arm__

extern char  __bss_end;

int freeMemory( void )
{
#if 0
   char top;
#ifdef __arm__
   return &top - reinterpret_cast<char*>( sbrk( 0 ) );
#elif defined(CORE_TEENSY) || (ARDUINO > 103 && ARDUINO != 151)
   // for current version of Arduino
   // Serial.print( F( "__brkval: " ) );
   // Serial.print(  (int)__brkval, HEX );
   // Serial.print( F( "   _top: " ) );
   // Serial.println( (int)&top, HEX  );

   LOGD( TAG, "__brkval: 0x%04x  _top: 0x%04x", ( int )__brkval, ( int )&top );

   return &top - __brkval;
#else  // __arm__
   return __brkval ? &top - __brkval : &top - __malloc_heap_start;
#endif  // __arm__

#elif 0
   // from F:\Arduino\libraries\SD\src\utility\SdFatUtil.h
   int free_memory;
   if( reinterpret_cast<int>( __brkval ) == 0 )
   {
      // if no heap use from end of bss section
      free_memory = reinterpret_cast<int>( &free_memory )
                    - reinterpret_cast<int>( &__bss_end );

      // Serial.print( F( "__bss_end: " ) );
      // Serial.print(  (int)&__bss_end, HEX );
      // Serial.print( F( "   free_memory: " ) );
      // Serial.println( (int)&free_memory, HEX  );

      LOGD( TAG, "__bss_end: 0x%04x  free_memory: 0x%04x", ( int )&__bss_end, ( int )&free_memory );
   }
   else
   {
      // use from top of stack to heap
      free_memory = reinterpret_cast<int>( &free_memory )
                    - reinterpret_cast<int>( __brkval );

      // Serial.print( F( "__brkval: " ) );
      // Serial.print(  (int)__brkval, HEX );
      // Serial.print( F( "   free_memory: " ) );
      // Serial.println( (int)&free_memory, HEX  );

      LOGD( TAG, "__brkval: 0x%04x  free_memory: 0x%04x", ( int )__brkval, ( int )&free_memory );
   }
   return free_memory;

#else
   // see F:\Arduino\libraries\SdFat\src\FreeStack.h
   char* sp = reinterpret_cast<char*>( SP );

   if( reinterpret_cast<int>( __brkval ) == 0 )
   {
      // Serial.print( F( "__bss_end: " ) );
      // Serial.print(  (int)&__bss_end, HEX );
      // Serial.print( F( "   sp: " ) );
      // Serial.println( (int)sp, HEX  );

      LOGD( TAG, "__bss_end: 0x%04x  SP: 0x%04x free: %d", ( int )&__bss_end, ( int )sp, sp - ( char* )&__bss_end );
      // LOGD( TAG, "heap_end: 0x%04x  heap_start: 0x%04x free: %d", __malloc_heap_end, __malloc_heap_start, __malloc_heap_end - __malloc_heap_start );

      return sp - ( char* )&__bss_end;
   }
   else
   {
      // Serial.print( F( "__brkval: " ) );
      // Serial.print(  (int)__brkval, HEX );
      // Serial.print( F( "   sp: " ) );
      // Serial.println( (int)sp, HEX  );

      LOGD( TAG, "__brkval: 0x%04x  SP: 0x%04x free: %d", __brkval, sp, sp - __brkval );
      // LOGD( TAG, "_heap_end: 0x%04x  heap_start: 0x%04x free: %d", __malloc_heap_end, __malloc_heap_start, __malloc_heap_end - __malloc_heap_start );

      return sp - __brkval;
   }
#endif
}

// --------------------------------------------------------------------------
//
// --------------------------------------------------------------------------

// tag_fmt is the format string "%c (%s) " tag "(%d): " for log_code, log_time, TAG, __LINE__
// when log_time iz zero, don't print a log time

#define TIME2STR_LEN       16 + 1
static char print_buf[ PRINTF_BUFFER_SIZE ];

void _log_write( char log_level, const char *tag_fmt, uint32_t log_time, uint16_t line, const __FlashStringHelper *format, ... )
{
#ifdef USE_SERIAL_OUTPUT
   uint16_t buflen = sizeof( print_buf ) - 1;      // reserve one byte for the terminating zero
   char* buf = print_buf;
   int16_t written;

   // prepare the string repressenting the time
   char *time2str_buf = &buf[ PRINTF_BUFFER_SIZE - TIME2STR_LEN ];

   written = _log_time2str( time2str_buf, TIME2STR_LEN, log_time );
   // time2str_buf[ written ] = '\0';

#ifdef __AVR__
   written = snprintf_P( buf, buflen - TIME2STR_LEN, tag_fmt, log_level, time2str_buf, line );
#else
   written = snprintf( buf, buflen - TIME2STR_LEN, tag_fmt, time2str_buf, line );
#endif
   buf[ written ] = '\0';
   // buf += written;
   // buflen -= written;

   Serial.print( print_buf );

   va_list args;
   va_start( args, format );

   buf = print_buf;
   buflen = sizeof( print_buf ) - 1;
 #ifdef __AVR__
   written = vsnprintf_P( buf, buflen, ( const char * )format, args ); // progmem for AVR
#else
   written = vsnprintf( buf, buflen, ( const char * )format, args );   // for the rest of the world
#endif
   va_end( args );

   buf[ written ] = '\0';
   Serial.println( print_buf );
#endif // USE_SERIAL_OUTPUT
}

// --------------------------------------------------------------------------
//
// --------------------------------------------------------------------------

// convert time in milli seconds to string "h:mm:ss.ddd"

static void div_mod( uint32_t divident, uint32_t divisor, uint16_t *result, uint32_t *remainder )
{
   *result = ( uint16_t )( divident / divisor );
   *remainder = divident % divisor;
}

uint16_t _log_time2str( char *buffer, uint16_t buffer_size, const int32_t time )
{
   uint32_t rest;
   uint16_t hh, mm, ss, ms;

   div_mod( time, ( 60UL * 60 * 1000 ), &hh, &rest );        // 0..1193 hours
   div_mod( rest, ( 60UL * 1000 ),      &mm, &rest );
   div_mod( rest, ( 1000UL ),           &ss, &rest );
   ms = ( uint16_t )rest;

   return snprintf_P( buffer, buffer_size, PSTR( "%d:%02d:%02d.%03d" ), hh, mm, ss, ms );
}

// --------------------------------------------------------------------------
//
// --------------------------------------------------------------------------

uint32_t _log_timestamp( void )
{
   return millis();
}

// --------------------------------------------------------------------------
// debug helper
// --------------------------------------------------------------------------

int dump_data( const char* data, unsigned short len )
{
   uint16_t buflen = sizeof( print_buf ) - 1;      // reserve one byte for the terminating zero
   char* buf = print_buf;
   int16_t written;

   int i;

   if( len >= sizeof( buf ) - 1 )
      len = sizeof( buf ) - 1;

   for( i = 0; i < len; i++ )
   {
      char c = data[i];
      if( ( c < 0x20 || c > 0x7e ) && c != '\r' && c != '\n' )
         c = '.';
      buf[i] = c;
   }
   buf[i] = 0;

   Serial.println( print_buf );
   return i;
}

// --------------------------------------------------------------------------
//
// --------------------------------------------------------------------------

int dump_text( const char* data, unsigned short len )
{
   uint16_t buflen = sizeof( print_buf ) - 1;      // reserve one byte for the terminating zero
   char* buf = print_buf;
   int16_t written;

   int cnt = 0;

   for( int i = 0; i < len; i++ )
   {
      char c = data[ i ];
      if( ( c >= 0x20 && c < 0x80 ) )
      {
         cnt += Serial.print( c );
      }
      else
      {
         cnt += Serial.print( c, HEX );
      }
   }
   cnt += Serial.println();
   return cnt;
}

// --------------------------------------------------------------------------
//
// --------------------------------------------------------------------------

int dump_data_hex( const char* data, unsigned short len )
{
   char print_buf[ PRINTF_BUFFER_SIZE / 2 ];
   LOGI( TAG, "alloc %d byte @ 0x%04x", sizeof( print_buf ), print_buf );

   uint16_t buflen = sizeof( print_buf ) - 1;      // reserve one byte for the terminating zero
   char* buf = print_buf;
   int16_t written;
   uint16_t i, j;

   LOGD( TAG, "print_buf  0x%04x buf 0x%04x buflen %d len %d", print_buf, buf, buflen, len );

   for( i = 0; i < len; )
   {
      written = snprintf_P( buf, buflen, PSTR( "0x%04x " ), data );
      buf += written;
      buflen -= written;

      for( j = 0; j < 16 && i < len; j++ )
      {
         if( buflen < 5 )
         {
            *buf++ = '\0';
            // LOGD( TAG, "%d %d print_buf <%s> %d", i, j, print_buf, buflen );
            Serial.print( print_buf );
            buflen = sizeof( print_buf ) - 1;      // reserve one byte for the terminating zero
            buf = print_buf;
         }
         uint8_t c = *data++;
         i++;
         uint8_t hi = ( c >> 4 ) & 0x0F;
         uint8_t lo = ( c >> 0 ) & 0x0F;

         *buf++ = hi + 0x30 + ( hi >= 10 ? 7 : 0 );
         *buf++ = lo + 0x30 + ( lo >= 10 ? 7 : 0 );
         *buf++ = ' ';
         buflen -= 3;
      }
      *buf++ = '\n';
      buflen--;
      *buf++ = '\0';
      // LOGD( TAG, "%d %d print_buf <%s> %d", i, j, print_buf, buflen );
      Serial.print( print_buf );
      buflen = sizeof( print_buf ) - 1;      // reserve one byte for the terminating zero
      buf = print_buf;
   }
   return i;
}

// --------------------------------------------------------------------------
//
// --------------------------------------------------------------------------

