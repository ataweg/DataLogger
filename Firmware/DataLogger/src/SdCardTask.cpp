// --------------------------------------------------------------------------
//
// Project       DataLogger
//
// File          SdCardTask.cpp
//
// Author        Axel Werner
//
// --------------------------------------------------------------------------
// Changelog
//
//
// 2020-06-16  AWe   dump sdcard info and list files
// 2020-06-01  AWe   initial version
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
static const char TAG[] PROGMEM = tag( "SdCardTask" );

// --------------------------------------------------------------------------
//
// --------------------------------------------------------------------------

#ifdef ARDUINO
   #include <Arduino.h>             // pinMode(), digitalRead(), INPUT_PULLUP, millis(), ...
   #include "picoOS/picoOS.h"       // yield()
#else
   #include "WArduino.h"
#endif

#include <avr/wdt.h>
#include <SPI.h>
#include "io.h"                        // _digitalWrite(), _digitalRead()
#include "DataLogger_config.h"
#include "DataLogger.h"
#include "SdCardTask.h"
#include "SdCardInfo.h"
#include "UiTask.h"
#include "Config.h"
#include "Capture.h"
#include "Led.h"
#include "Switch.h"

#include "SdFat/SdFat.h"       // SdVolume

// --------------------------------------------------------------------------
//
// --------------------------------------------------------------------------

extern Led Led_R;
extern Led Led_G;
extern Led Led_Debug;

#if CD_Pin != NC
Switch CardDetect( CD_Pin );
#endif
#if WP_Pin != NC
Switch WriteProtect( WP_Pin );
#endif

const int chipSelect = SS_Pin;    // normally SS is pin 10

// set up variables using the SD utility library functions:
SdFat sd;
Capture capture;

enum : uint8_t
{
   PowerOn,                // 0
   SdCardReady,            // 1
   ReadyForCapture,        // 2
   Capture,                // 3
   FatalError,             // 4
   FatalErrorWait,         // 5
   FatalErrorLeave,        // 6
   Undef = 0xFF

} state = PowerOn;

// --------------------------------------------------------------------------
// prototypes for local functions
// --------------------------------------------------------------------------

bool isSdCardReady( void );
bool setupFileSystem( void );
bool isSdCardRemoved( void );

// --------------------------------------------------------------------------
//
// --------------------------------------------------------------------------

// open the volume and open the root directory

bool setupFileSystem( void )
{
   return true;
}

// --------------------------------------------------------------------------
//
// --------------------------------------------------------------------------

bool isSdCardReady( void )
{
   static bool print_once = true;

   if( !flags.sdcard_ready )
   {
      // see if the card is present and can be initialized:
      uint32_t t = millis();

      // Initialize at the highest speed supported by the board that is
      // not over 50 MHz. Try a lower speed if SPI errors occur.
      // see .\src\SdCard\SdFat.h(329)
      //     .\src\SdCard\SdCard\SdSpiCard.cpp(237)
      if( sd.begin( chipSelect, SD_SCK_MHZ( 50 ) ) )
      {
         // sdcard is available
         t = millis() - t;
         LOGI( TAG, "Wiring is correct and a card is present." );
         LOGI( TAG, "Need %d ms to initialize", t );

         flags.sdcard_ready = true;
         flags.sdcard_error = false;
         print_once = true;
      }
      else
      {
         // sdcard is not present
         if( print_once )
         {
            LOGE( TAG, "initialization failed. Things to check:" );
            LOGI( TAG, "* is a card inserted?" );
            LOGI( TAG, "* is your wiring correct?" );
            LOGI( TAG, "* did you change the chipSelect pin to match your shield?" );
            print_once = false;
         }
      }
   }
   return flags.sdcard_ready;
}

// --------------------------------------------------------------------------
//
// --------------------------------------------------------------------------

bool isSdCardRemoved( void )
{
   static struct
   {
      bool msg_0: 1;
      bool msg_1: 1;
      bool msg_2: 1;
   } print_once = { 0 };

   bool rc = false;

#if CD_Pin != NC
   {
      if( CardDetect.get() )
      {
         // sdcard insert
         if( !print_once.msg_2 )
         {
            LOGI( TAG, "SdCard is inserted" );
            print_once.msg_2 = true;
         }
         print_once.msg_1 = false;
      }
      else
      {
         // sdcard not insert
         if( !print_once.msg_1 )
         {
            LOGI( TAG, "SdCard is removed" );
            print_once.msg_1 = true;
            rc = true;
         }
         flags.sdcard_ready = false;
         print_once.msg_2 = false;
      }
   }
#else
   {
      // we don't have a card decet pin
      // so we try to access the configuration file?
      SdFile configFile;

      // LOGI( TAG, "alloc %d byte @ 0x%04x", sizeof( configFile ), &configFile );

      // LOGD( TAG, "configFile.open: <%s>", settings.ConfigFileName );
      if( configFile.isOpen() )
      {
         if( !print_once.msg_0 )
         {
            LOGE( TAG, "configFile if already open" );
            print_once.msg_0 = true;
         }
         configFile.close();
      }

      if( !configFile.open( settings.ConfigFileName, O_READ ) )
      {
         if( !print_once.msg_1 )
         {
            LOGI( TAG, "SdCard is removed" );
            rc = true;
            print_once.msg_1 = true;
         }
         flags.sdcard_ready = false;
         print_once.msg_2 = false;
         print_once.msg_0 = false;
      }
      else
      {
         char buf[ 1 ];
         // LOGD( TAG, "configFile.close" );
         if( 0 > configFile.read( buf, 1 ) )
         {
            LOGI( TAG, "Can't read file. SdCard is removed" );
            flags.sdcard_ready = false;
            print_once.msg_1 = true;
            rc = true;
         }
         else
         {
            if( !print_once.msg_2 )
            {
               LOGI( TAG, "SdCard is inserted" );
               print_once.msg_2 = true;
            }
            print_once.msg_1 = false;
            print_once.msg_0 = false;
         }
         configFile.close();
      }

      uint8_t ec = configFile.getError();
      if( ec )
      {
         // for error codes see .\src\SdCard\SdCard\SdInfo.h(41)
         // WRITE_ERROR = 0X1;
         // READ_ERROR  = 0X2;
         LOGD( TAG, "SdCard error: 0x%02x", ec );
         configFile.clearError();
      }
   }
#endif

   return rc;
}

// --------------------------------------------------------------------------
//
// --------------------------------------------------------------------------

void SdCardTask_setup( void )
{
   LOGD( TAG, "posSP: 0x%04x", SP );
   LOGD( TAG, "posStackSize: %d @ SP 0x%04x", posCheckStack(), SP );

   extern uint16_t __data_start;
   // dump_data_hex( ( const char* )&__data_start, 0x800 );
}

// --------------------------------------------------------------------------
//
// --------------------------------------------------------------------------

void SdCardTask_loop( void )
{
   _digitalWrite( OSZI_YELLOW_Pin, HIGH );

   // check if sdcard is inserted
   // read configuration file and setup datalogger

   static uint8_t old_state = Undef;

   if( old_state != state )
   {
      LOGD( TAG, "state %d -> %d", old_state, state );
      old_state = state;

      LOGD( TAG, "posSP: 0x%04x", SP );
      LOGD( TAG, "posStackSize: %d @ SP 0x%04x", posCheckStack(), SP );
   }

   switch( state )
   {
      case PowerOn:              // 0
      {
         if( isSdCardReady() )
         {
            LOGD( TAG, "SdCard is ready" );
            Led_R.on();
            Led_G.on();
            state = SdCardReady;
         }
      }
      break;

      case SdCardReady:          // 1
      {
         // a sdcard is detected so setup the file system
         if( setupFileSystem() )
         {
            // requires 3352 bytes
#ifdef HAVE_SPACE
            dumpSdCardInfo();
            dumpDirectory( &sd );
#endif
            // extern uint16_t __data_start;
            // dump_data_hex( ( const char* )&__data_start, 0x800 );
            // get the configuration or create a default configursation file
            if( getConfiguration() )
            {
               LOGD( TAG, "Ready for capture" );

               // setup capture hardware
               if( !capture.setup() )
               {
                  LOGE( TAG, "No capture source defined" );
                  // goto to error state and let led flashing
                  state = FatalError;
               }
               else
               {
                  // extern uint16_t __data_start;
                  // dump_data_hex( ( const char* )&__data_start, 0x800 );
                  // clear StartStopCapture sempahore, so capture samples will not start
                  posSemaphoreTake( StartStopCapture );
                  Led_R.off();
                  Led_G.off();
                  state = ReadyForCapture;
               }
            }
            else
            {
               // file system is available but cannot read file or create config file
               LOGE( TAG, "error opening config file: <%s>", settings.ConfigFileName );
               flags.sdcard_ready = false;
               state = FatalError;
            }
         }
         else
         {
            // file system reports an error
            LOGE( TAG, "Cannot setup file system" );
            flags.sdcard_ready = false;
            state = FatalError;
         }
      }
      break;

      case ReadyForCapture:      // 2
      {
         // sdcard is prepared to get captured data
         // control capture process

         // wait for button to start capture process
         if( posSemaphoreTake( StartStopCapture ) )
         {
            LOGD( TAG, "Start capture" );

            // start capture process
            // create and open a capture file for write
            // start/stop capture samples
            if( capture.start() )
            {
               // capture file is open to get samples
               Led_R.on();
               state = Capture;
            }
            else
            {
               LOGE( TAG, "Can't open capture file: <%s>", settings.FileName );
               LOGE( TAG, "Or no source is defined" );
               flags.sdcard_ready = false;
               state = FatalError;
            }
         }

         // start button not pressed
         // check for card removed
         if( isSdCardRemoved() )
         {
            LOGD( TAG, "SdCard is removed" );
            flags.sdcard_ready = false;
            // sd.vwd()->close();
            // it's not a fatal error,
            // because we can insert the sdcard again
            state = PowerOn;
         }
      }
      break;

      case Capture:              // 3
      {
         // do capturing
         // capture some date and save them to file
         capture.run();

         // take all semaphore, so it will not return to this state
         posSemaphores_t sema = posSemaphoreTake( AllSemaphores );
         if( sema & StartStopCapture )
         {
            LOGD( TAG, "Stop capture" );
            capture.stop();
            Led_R.off();
            state = ReadyForCapture;

            extern uint16_t __data_start;
            // dump_data_hex( ( const char* )&__data_start, 0x800 );
         }

         // check for errros or card removed
         if( flags.sdcard_error )
         {
            // for error codes see .\src\SdCard\SdCard\SdInfo.h(41)
            // WRITE_ERROR = 0X1;
            // READ_ERROR  = 0X2;

            uint8_t ec = capture.file()->getError();
            LOGD( TAG, "Stop capture. Sdcard error: 0x%02x", ec );

            Led_R.flash_slow();
            capture.stop();
            flags.sdcard_ready = false;
            flags.sdcard_error = false;     // clear the flag

            // it's not a fatal error, because we can insert
            // the sdcard again, and start a new capture job
            // !!! todo: not for all reasons, some are fatal
            state = PowerOn;
         }
      }
      break;

      case FatalError:           // 4
      {
         // Could not read/write the sdcard, because it ghas an issue
         // maybe the filesystem ist corrupt, or the card is full.
         // Also no capture source is defined
         // user should remove the sdcard and fix it

         LOGE( TAG, "Fatal error. Remove and fix sdcard" );
         LOGE( TAG, "Wait for button long pressed" );

         Led_R.flash_fast();
         Led_G.flash_fast();
         state = FatalErrorWait;
      }
      break;

      case FatalErrorWait:       // 5
      {
         // wait for button long pressed
         if( posSemaphoreTake( RestartCapture ) )
         {
            LOGE( TAG, "Wait for sdcard removed" );
            Led_G.off();
            state = FatalErrorLeave;
         }
      }
      break;

      case FatalErrorLeave:      // 6
      {
         // wait for sdcard removed
         if( isSdCardRemoved() )
         {
            // stop led flashing
            Led_R.off();

            LOGE( TAG, "Reset system" );
            // do a reset via watchdog
            wdt_enable( WDTO_15MS );
         }
      }
      break;

   }  // switch

   _digitalWrite( OSZI_YELLOW_Pin, LOW );
}

// --------------------------------------------------------------------------
//
// --------------------------------------------------------------------------

void SdCardTask( void )
{
   // the task setup
   LOGD( TAG, "posSP: 0x%04x", SP );
   LOGD( TAG, "posStackSize: %d @ SP 0x%04x", posCheckStack(), SP );

   SdCardTask_setup();

   LOGD( TAG, "posSP: 0x%04x", SP );
   LOGD( TAG, "posStackSize: %d @ SP 0x%04x", posCheckStack(), SP );

   LOGI( TAG, "--------------------------------------------------------------------------------" );
   yield();

   // the task loop
   do
   {
      SdCardTask_loop();
      yield();
   }
   while( 1 ); // endless loop
}

// --------------------------------------------------------------------------
//
// --------------------------------------------------------------------------
