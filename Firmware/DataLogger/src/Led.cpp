// --------------------------------------------------------------------------
//
// Project       io
//
// File          leds.c
//
// Author        Axel Werner
//
// --------------------------------------------------------------------------
// Changelog
//
//    2018-02-15  AWe   redesign leds_init()
//    2018-02-15  AWe   adept for use with ESP32 and ESP-IDF
//    2017-11-18  AWe   initial version
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
static const char TAG[] PROGMEM = tag( "Leds" );

// --------------------------------------------------------------------------
//
// --------------------------------------------------------------------------

#ifdef ARDUINO
   #include <Arduino.h>       // pinMode(), digitalRead(), INPUT_PULLUP, millis(), ...
   #include "picoOS/picoOS.h"
#else
   #include "WArduino.h"
#endif

#include "io.h"                     // _digitalWrite(), _digitalRead()
#include "Led.h"

// --------------------------------------------------------------------------
// variables
// --------------------------------------------------------------------------

#if 0
led_t *leds;
int num_leds = 0;
int SYS_LED  = 0;
int INFO_LED = 1;
#endif

// --------------------------------------------------------------------------
//
// --------------------------------------------------------------------------

Led::Led( uint8_t _pin )
{
   // LOGD( TAG, "Led constructor pin %d", _pin );

   pin = _pin;
   pinMode( pin, OUTPUT );

   config.mode = None;
}

// --------------------------------------------------------------------------
//
// --------------------------------------------------------------------------

Led::~Led( void )
{
}


// --------------------------------------------------------------------------
//
// --------------------------------------------------------------------------

void Led::set( uint8_t mode, uint8_t on_time, uint8_t off_time, uint8_t repeat )
{
   LOGD( TAG, "led %d set 0x%02x %d %d %d", id, mode, on_time, off_time, repeat );

   config.mode = mode;
   config.on_time = on_time;
   config.off_time = off_time;
   config.repeat = repeat;
}

// --------------------------------------------------------------------------
//
// --------------------------------------------------------------------------

uint8_t Led::get( void )
{
   return state.mode;  // return off, on, flash, blink or none
}

// --------------------------------------------------------------------------
//
// --------------------------------------------------------------------------

// led is busy when the led is flashing or blinking and the number of repeats in not zero
// in all other cases (on, off) the led is not bust

bool Led::busy( void )
{
   LOGD( TAG, "led %d is busy %d || %d", id, config.mode != None, flags.busy != 0 );

   return ( config.mode != None || flags.busy != 0 ) ? true : false;
}

// --------------------------------------------------------------------------
// update led
// --------------------------------------------------------------------------

// update leds every 100ms in systemTimer100msTask (user_main.c)

void Led::update( void )
{
   // Is there a new configuration and update is allowed?
   if( config.mode != None && !flags.no_update )
   {
      LOGD( TAG, "led %d setup mode 0x%02x", id, config.mode );

      memcpy( &state, &config, sizeof( config ) );
      config.mode = None;      //  we have take over the configuration

      if( state.mode == On || state.mode == Off )
      {
         flags.led_on = state.mode == On ? 1 : 0;
         timer = 0;
         flags.flash_phase = 0;
         flags.no_update = 0;
         flags.busy = 0;
      }
      else
      {
         // setup timer for flashing or blinking
         flags.led_on = 1;                  // switch led on
         timer = state.on_time;   // time for on phase
         flags.flash_phase = 1;
         // allow update after one cycle of the inactive phase
         flags.no_update = 1;
         flags.busy = 1;
      }
   }
   else
   {
      // no new configuration or update not allowed
      if( state.mode == Blink || state.mode == Flash )
      {
         // do the flashing for the selected led
         timer--;

         // allow update again in the after the first cycle
         if( flags.flash_phase == 0 )
            flags.no_update = 0;

         if( 0 == timer )
         {
            if( flags.flash_phase )
            {
               // end of active phase
               flags.led_on ^= 1;                 // toggle led -> off
               flags.flash_phase = 0;
               timer = state.off_time;  // time for off phase
               if( timer == 0 )
                  timer = 1;
            }
            else
            {
               // end of inactive phase
               if( state.counter != 0 )
               {
                  state.counter--;
                  if( state.counter == 0 )
                  {
                     state.mode = Off;        // this stops binking
                     flags.busy = 0;
                  }
               }

               if( state.off_time == 0 )
               {
                  state. mode = Off;           // this stops flashing
                  flags.busy = 0;
               }
               else
               {
                  flags.led_on ^= 1;                 // toggle led -> on
                  flags.flash_phase = 1;
                  timer = state.on_time;   // time for on phase
               }
            }
         }
      }

      if( state.mode == On )
         flags.led_on = 1;
      else if( state.mode == Off )
         flags.led_on = 0;
   }

#if 1 // !!! configurable?
   // leds are connected to GND
   _digitalWrite( pin, flags.led_on );
#else
   // leds are connected to VCC, so invert pin value
   _digitalWrite( pin, flags.led_on ^ 1 );
#endif
}

#if 0
// --------------------------------------------------------------------------
// update leds
// --------------------------------------------------------------------------

// update leds every 100ms in systemTimer100msTask (user_main.c)

void Leds::update( void )
{
   uint8_t id;

   for( id = 0; id < num_leds; id++ )
   {
      leds[ id ].update();
   }
}

// --------------------------------------------------------------------------
//
// --------------------------------------------------------------------------

void Leds::init( int _num_leds, ... )
{
   LOGI( TAG, "init %d leds", _num_leds );
   num_leds = _num_leds;
   leds = ( led_t * )calloc( num_leds, sizeof( led_t ) ); // all leds are off

   // configure output pins
   va_list arg;
   va_start( arg, num_leds );

   for( int id = 0; id < num_leds; id++ )
   {
      gpio_num_t pin = ( gpio_num_t )va_arg( arg, int );

      leds[ id ].pin = pin;
      if( pin != NC )
      {
         LOGI( TAG, "setup led %d, use pin %d", id, pin );
         gpio_pad_select_gpio( pin );
         /* Set the GPIO as a push/pull output */
         gpio_set_direction( pin, GPIO_MODE_OUTPUT );
         gpio_set_level( pin, 0 );  // led off
      }
   }
   va_end( arg );

   // the last two leds in the list are
   SYS_LED  = num_leds - 2;
   INFO_LED = num_leds - 1;

   LOGI( TAG, "init %d leds done", num_leds );
}

// --------------------------------------------------------------------------
//
// --------------------------------------------------------------------------

void Leds::deinit( void )
{
   if( leds != NULL )
   {
      free( leds );
      leds = NULL;
   }
}

// --------------------------------------------------------------------------
//
// --------------------------------------------------------------------------
#endif