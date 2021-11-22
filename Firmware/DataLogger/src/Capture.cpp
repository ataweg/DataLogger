// --------------------------------------------------------------------------
//
// Project       DataLogger
//
// File          Capture.cpp
//
// Author        Axel Werner
//
// --------------------------------------------------------------------------
// Changelog
//
//
// 2020-06-16  AWe   rename capture file if exists
//                   print some statistics to capture file
// 2020-06-10  AWe   write captured data to file (SIO, I2C, ADC, DIG)
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

// --------------------------------------------------------------------------
// debug support
// --------------------------------------------------------------------------

#define LOG_LOCAL_LEVEL    LOG_INFO
#include "aweLog.h"
static const char TAG[] PROGMEM = tag( "Capture" );

// --------------------------------------------------------------------------
//
// --------------------------------------------------------------------------

#define USE_TWI

#ifdef ARDUINO
   #include <Arduino.h>             // pinMode(), digitalRead(), INPUT_PULLUP, millis(), ...
   #include "picoOS/picoOS.h"       // yield()
#else
   #include "WArduino.h"
#endif

#ifdef USE_TWI
   #include <Wire.h>
#endif
#include "Config.h"
#include "Capture.h"
#include "DataLogger.h"          // flags
#include "Led.h"

#include "SdFat/SdFat.h"       // SdVolume

// --------------------------------------------------------------------------
//
// --------------------------------------------------------------------------

extern Led Led_R;
extern Led Led_G;
extern Led Led_Debug;

// --------------------------------------------------------------------------
//
// --------------------------------------------------------------------------

void i2cReceiveEvent( int howMany )
{
   posSemaphoreGive( I2C_ReceiveEvent );
}

// --------------------------------------------------------------------------
//
// --------------------------------------------------------------------------

Capture::Capture( void )
{
}

// --------------------------------------------------------------------------
//
// --------------------------------------------------------------------------

Capture::~Capture( void )
{
}

// --------------------------------------------------------------------------
//
// --------------------------------------------------------------------------

bool Capture::setup( void )
{
   LOGI( TAG, "Setup Capture" );

   bool rc = false;

   // setup capture sources
   captureSource = settings.CaptureSource;
   if( captureSource.val )
   {
      if( captureSource.sio )
      {
         // setup serial interface ( boudrate, num bits, parity, stop bits
      }
      if( captureSource.i2c )
      {
         // setup i2c interface
         Wire.begin( 4 );                    // join i2c bus with address #4
         Wire.onReceive( i2cReceiveEvent );  // register event
      }
      if( captureSource.analog )
      {
         // configure pins for analog input
         // analogPin: 0 .. 5
         // configure pin for input with pullup
         uint8_t mask = captureSource.analog;
         for( uint8_t i = 0; i < 6; i++ )
         {
            if( mask & 1 )
            {
               uint8_t pin = A0 + i;
               pinMode( pin, INPUT );
               LOGD( TAG, "%d: set analog pin %d to input", i, pin );
            }
            mask >>= 1;
         }
      }
      if( captureSource.digital )
      {
         // configure pin for input with pullup
         uint8_t mask = captureSource.digital;
         for( uint8_t i = 0; i < 8; i++ )
         {
            if( mask & 1 )
            {
               uint8_t pin = i;
               pinMode( pin, INPUT_PULLUP );
               LOGD( TAG, "%d: set digital pin %d to input", i, pin );
            }
            mask >>= 1;
         }
      }
      rc = true;
   }
   else
   {
      LOGE( TAG, "No capture source defined" );
   }

   LOGD( TAG, "Setup Capture done %d", rc );
   return rc;
}

// --------------------------------------------------------------------------
//
// --------------------------------------------------------------------------

bool Capture::start( void )
{
   bool rc = false;
   bool createNewCaptureFile = false;

   sampleCount = 0;
   startTime = millis();
   sampleTime = 0;

   // check if capture file exists
   if( captureFile.open( settings.FileName, O_READ ) )
   {
      // capture file exists, so rename it
      char newFileName[ 12 + 1 ];
      LOGD( TAG, "alloc %d byte @ 0x%04x", sizeof( newFileName ), newFileName );

      char *tmp = settings.FileName;
      uint8_t i;
      for( i = 0; i < 8; i++ )
      {
         if( *tmp == '\0' || *tmp == '.' )
            break;

         newFileName[ i ] = *tmp++;
      }
      tmp = &newFileName[ i ];

      uint16_t index = 0;
      for( index = 0; index < 1000; index++ )
      {
         SdFile tmpFile;
         LOGI( TAG, "alloc %d byte @ 0x%04x", sizeof( tmpFile ), &tmpFile );

         snprintf_P( tmp, 5, PSTR( ".%03u" ), index );
         if( !tmpFile.open( newFileName ) )
         {
            createNewCaptureFile = true;
            break;
         }
         tmpFile.close();
      }

      if( createNewCaptureFile )
      {
         LOGI( TAG, "rename <%s> to <%s>", settings.FileName, newFileName );
         captureFile.rename( newFileName );
      }
      else
      {
         LOGI( TAG, "Cannot rename <%s> to <%s>", settings.FileName, newFileName );
      }

      LOGD( TAG, "captureFile.close" );
      captureFile.close();
   }
   else
   {
      // capture file doesn't exists
      createNewCaptureFile = true;
   }

   if( createNewCaptureFile )
   {
      LOGD( TAG, "captureFile.open: <%s>", settings.FileName );
      if( !captureFile.open( settings.FileName, ( O_READ | O_WRITE | O_CREAT | O_TRUNC ) ) )
      {
         LOGE( TAG, "Can't open capture file: <%s>", settings.FileName );
         flags.sdcard_error = true;
      }
      else
      {
         // start capture sources
         if( captureSource.val )
         {
            if( captureSource.sio )
            {
            }
            if( captureSource.i2c )
            {
            }
            if( captureSource.analog )
            {
            }
            if( captureSource.digital )
            {
            }
         }
         rc = true;
      }
   }

   LOGI( TAG, "Start Capture %d", rc );
   return rc;
}

// --------------------------------------------------------------------------
//
// --------------------------------------------------------------------------

#ifndef PRINTF_BUFFER_SIZE
   #define PRINTF_BUFFER_SIZE          64          // resulting string limited to 64 chars
#endif
#ifndef TIME2STR_LEN
   #define TIME2STR_LEN                16 + 1
#endif

bool Capture::run( void )
{
   // get on set of data from capture sources and write them to the file

   bool rc = false;

   // make a buffer for assembling the data to log:
   char print_buf[ PRINTF_BUFFER_SIZE ];
   // LOGI( TAG, "alloc %d byte @ 0x%04x", sizeof( print_buf ), print_buf );

   uint8_t buflen = sizeof( print_buf ) - 1;
   char* buf = print_buf;               // save begin of buffer

   sampleTime = millis();

   // capture sources
   if( captureSource.val )
   {
      bool have_sampled_data = false;
      bool sample_time_written = false;

      int16_t written = _log_time2str( buf, buflen, sampleTime );
      buf[ written ] = '\0';

      if( captureSource.sio && sampleTime >= nextSampleSioTime )
      {
         int available = Serial.available();
         if( available > 0 )
         {
            if( !sample_time_written )
            {
               captureFile.print( print_buf );
               sample_time_written = true;
            }

            buf = print_buf;
            buflen = sizeof( print_buf ) - 1;
            written = snprintf_P( buf, buflen, PSTR( " SIO \"" ) );
            buf[ written ] = '\0';
            buf += written;
            buflen -= written;

            int num_bytes_read = available;
            if( num_bytes_read > buflen - 2 )
               num_bytes_read > buflen - 2 ;

            written = Serial.readBytes( buf, num_bytes_read );
            buf += written;
            buflen -= written;
            written = snprintf_P( buf, buflen, PSTR( "\"" ) );

            captureFile.print( print_buf );
            have_sampled_data = true;
         }
          nextSampleTime = sampleTime + settings.SerialSamplingRate;
      }

      if( captureSource.i2c  && sampleTime >= nextSampleI2cTime )
      {
#ifdef USE_TWI
         if( posSemaphoreTake( I2C_ReceiveEvent ) )
         {
            if( !sample_time_written )
            {
               captureFile.print( print_buf );
               sample_time_written = true;
            }

            buf = print_buf;
            buflen = sizeof( print_buf ) - 1;
            written = snprintf_P( buf, buflen, PSTR( " I2C" ) );
            buf[ written ] = '\0';
            buf += written;
            buflen -= written;

            while( Wire.available() ) // loop through all but the last
            {
               uint16_t val = Wire.read(); // receive byte as a integer

               written = snprintf_P( buf, buflen, PSTR( " 0x%02x" ), val );
               buf[ written ] = '\0';
               buf += written;
               buflen -= written;
               if( buflen < 5 )
               {
                  captureFile.print( print_buf );
                  buf = print_buf;
                  buflen = sizeof( print_buf ) - 1;
               }
            }
            captureFile.print( print_buf );
            have_sampled_data = true;
         }
#endif
          nextSampleTime = sampleTime + settings.I2cSamplingRate;
      }

      if( sampleTime >= nextSampleTime )
      {
         if( captureSource.analog )
         {
            // On ATmega based boards (UNO, Nano, Mini, Mega), it takes about
            // 100 microseconds (0.0001 s) to read an analog input, so the maximum
            // reading rate is about 10,000 times a second.

            if( !sample_time_written )
            {
               captureFile.print( print_buf );
               sample_time_written = true;
            }

            buf = print_buf;
            buflen = sizeof( print_buf ) - 1;
            written = snprintf_P( buf, buflen, PSTR( " ADC" ) );
            buf[ written ] = '\0';
            buf += written;
            buflen -= written;

            // see also C:\Program Files (x86)\Arduino\hardware\arduino\avr\cores\arduino\wiring_analog.c
            uint8_t mask = captureSource.analog;
            for( uint8_t i = 0; i < 6; i++ )
            {
               if( mask & 1 )
               {
                  uint8_t pin = A0 + i;
                  int sensor = analogRead( pin );
                  LOGD( TAG, "%d: analog pin %d get %d", i, pin, sensor );

                  written = snprintf_P( buf, buflen, PSTR( " %d" ), sensor );
                  buf[ written ] = '\0';
                  buf += written;
                  buflen -= written;
               }
               mask >>= 1;
            }
            captureFile.print( print_buf );
            have_sampled_data = true;
         }
         if( captureSource.digital )
         {
            // read from port C
            uint8_t port_c = PORTC;
            // read from port D
            uint8_t port_d = PORTD;
            // combine both ports and maskout not selected pins
            uint8_t digital = ( port_c & 0x3f | port_d & 0xc0 ) & captureSource.digital;

            LOGD( TAG, "digital pins 0x%02x get 0x%02x", captureSource.digital, digital );

            if( !sample_time_written )
            {
               captureFile.print( print_buf );
               sample_time_written = true;
            }

            buf = print_buf;
            buflen = sizeof( print_buf ) - 1;
            written = snprintf_P( buf, buflen, PSTR( " DIG 0x%02x" ), digital );
            buf[ written ] = '\0';
            captureFile.print( print_buf );
            have_sampled_data = true;
         }

         nextSampleTime = sampleTime + settings.SamplingRate;
      }

      if( have_sampled_data )
      {
         Led_Debug.oneshot();
         sampleCount++;

         captureFile.println();
         if( captureFile.getError() )
         {
            flags.sdcard_error = true;
         }
         else
            rc = true;
      }
   }

   return rc;
}

// --------------------------------------------------------------------------
//
// --------------------------------------------------------------------------

bool Capture::stop( void )
{
   // stop the capture soucre writing to file

   bool rc = false;

   // stop sources
   if( captureSource.val )
   {
      if( captureSource.sio )
      {
      }
      if( captureSource.i2c )
      {
      }
      if( captureSource.analog )
      {
      }
      if( captureSource.digital )
      {
      }
      rc = true;
   }

   // print some statistics
   // sampleCount
   // startTime
   // sampleTime

   char print_buf[ PRINTF_BUFFER_SIZE ];
   LOGI( TAG, "alloc %d byte @ 0x%04x", sizeof( print_buf ), print_buf );
   uint8_t buflen = sizeof( print_buf ) - 1;
   char time2str_buf[TIME2STR_LEN ];
   LOGI( TAG, "alloc %d byte @ 0x%04x", sizeof( time2str_buf ), time2str_buf );

   _log_time2str( time2str_buf, TIME2STR_LEN, startTime );
   snprintf_P( print_buf, buflen, PSTR( "Capture start time: %s" ), time2str_buf );
   LOG( TAG, "%s", print_buf );
   captureFile.println( print_buf );

   _log_time2str( time2str_buf, TIME2STR_LEN, sampleTime );
   snprintf_P( print_buf, buflen, PSTR( "Capture end time:   %s" ), time2str_buf );
   LOG( TAG, "%s", print_buf );
   captureFile.println( print_buf );

   _log_time2str( time2str_buf, TIME2STR_LEN, sampleTime - startTime );
   snprintf_P( print_buf, buflen, PSTR( "Capture run time:   %s" ), time2str_buf );
   LOG( TAG, "%s", print_buf );
   captureFile.println( print_buf );

   snprintf_P( print_buf, buflen, PSTR( "Captured %ld samples" ), sampleCount );
   LOG( TAG, "%s", print_buf );
   captureFile.println( print_buf );

   LOGD( TAG, "captureFile.close" );
   captureFile.close();
   LOGI( TAG, "Stopped Capture %d", rc );
   return rc;
}

// --------------------------------------------------------------------------
//
// --------------------------------------------------------------------------
