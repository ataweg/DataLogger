// --------------------------------------------------------------------------
//
// Project       DataLogger
//
// File          UiTask.cpp
//
// Author        Axel Werner
//
// --------------------------------------------------------------------------
// Changelog
//
//
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

#define LOG_LOCAL_LEVEL    LOG_NONE
#include "aweLog.h"
static const char TAG[] PROGMEM = tag( "UiTask" );

// --------------------------------------------------------------------------
//
// --------------------------------------------------------------------------

#ifdef ARDUINO
   #include <Arduino.h>             // pinMode(), digitalRead(), INPUT_PULLUP, millis(), ...
   #include "picoOS/picoOS.h"       // yield()
#else
   #include "WArduino.h"
#endif

#include "io.h"                        // _digitalWrite(), _digitalRead()
#include "DataLogger_config.h"
#include "DataLogger.h"
#include "SdCardTask.h"
#include "UiTask.h"

#include "Button.h"

// --------------------------------------------------------------------------
//
// --------------------------------------------------------------------------

Buttons buttons( 1, BTN_Pin );

// --------------------------------------------------------------------------
// protoypes for local functions
// --------------------------------------------------------------------------

// =============================================================================
// the ui task staff
// =============================================================================

void UiTask_setup( void )
{
   LOGI( TAG, "setup done!" );

   LOGD( TAG, "posSP: 0x%04x", SP );
   LOGD( TAG, "posStackSize: %d @ SP 0x%04x", posCheckStack(), SP );
}

// --------------------------------------------------------------------------
//
// --------------------------------------------------------------------------

void UiTask_loop( void )
{
   _digitalWrite( OSZI_BLUE_Pin, HIGH );

   // process buttons
   // we have only one button in this project
   uint8_t buttons_state = buttons.scan();
   if( buttons_state != 0 )
   {
      // LOGD( TAG, "any button pressed or released 0x%02x", buttons_state );
      Button::State state = buttons.get( Buttons::BTN );
      // LOGD( TAG, "button state 0x%02x", state );

      if( state >= Button::ShortPressed )
      {
         if( state == Button::ShortPressed )
         {
            LOGD( TAG, "button ShortPressed" );
            // start capture
            posSemaphoreGive( StartStopCapture );

            LOGD( TAG, "posSP: 0x%04x", SP );
            LOGD( TAG, "posStackSize: %d @ SP 0x%04x", posCheckStack(), SP );
         }
         else if( state == Button::LongPressed )
         {
            LOGD( TAG, "button LongPressed" );
            // stop capture
            posSemaphoreGive( RestartCapture );
         }
         else if( state == Button::VeryLongPressed )
         {
            LOGD( TAG, "button VeryLongPressed" );
            posSemaphoreGive( SysReset );

            // force system reset
         }
         buttons.clear( Buttons::BTN );
      }
   }
   _digitalWrite( OSZI_BLUE_Pin, LOW );
}

// --------------------------------------------------------------------------
//
// --------------------------------------------------------------------------

void UiTask( void )
{
   // the task setup
   UiTask_setup();
   yield();

   // the task loop
   do
   {
      UiTask_loop();
      yield();
   }
   while( 1 ); // endless loop
}

// --------------------------------------------------------------------------
//
// --------------------------------------------------------------------------
