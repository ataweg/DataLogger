// --------------------------------------------------------------------------
//
// 2020-07-16  AWe   initial version
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
static const char TAG[] PROGMEM = tag( "Switch" );

// --------------------------------------------------------------------------
//
// --------------------------------------------------------------------------

#ifdef ARDUINO
   #include <Arduino.h>       // pinMode(), digitalRead(), INPUT_PULLUP, millis(), ...
   #include "picoOS/picoOS.h"
#else
   #include "WArduino.h"
#endif

#include <stdarg.h>

#include "io.h"                     // _digitalWrite(), _digitalRead()
#include "Switch.h"

// --------------------------------------------------------------------------
//
// --------------------------------------------------------------------------

Switch::Switch( uint8_t _pin, bool _inverted )
{
   LOGD( TAG, "Switch constructor pin %d", _pin );

   pin = _pin;
   pinMode( pin, INPUT_PULLUP );
   inverted = _inverted;

   sw_stable = false;
   sw_old = false;
   sw_state = Off;     // the state of the switch returned to caller
}

// --------------------------------------------------------------------------
//
// --------------------------------------------------------------------------

Switch::~Switch( void )
{
}

// --------------------------------------------------------------------------
//
// --------------------------------------------------------------------------

// see also https://github.com/evert-arias/EasySwitch

Switch::State Switch::get( void )
{
   bool sw_in = _digitalRead( pin, 1 );
   uint16_t sample_time = ( uint16_t )millis();

   if( sw_in != sw_old )
   {
      // switch pin has changed, switch can bounce
      debounce_start_time = ( uint8_t )sample_time;
      sw_stable = false;
      sw_old = sw_in;
   }
   else
   {
      // switch pin has not changed, so wait for debounce time is over

      if( !sw_stable )
      {
         // calculation is ok, as long as the time between two samples is less than 256 ms
         uint8_t debounce_time = ( uint8_t )sample_time - debounce_start_time;

         if( SW_DEBOUNCE_TIMEOUT < debounce_time )
         {
            // switch debouncing periode is over
            // switch is stable
            sw_stable = true;
            sw_state = ( Switch::State )( inverted ^ sw_in );
         }
      }
   }

   return sw_state;
}

// --------------------------------------------------------------------------
//
// --------------------------------------------------------------------------

Switches::Switches( uint8_t _num_switches, ... )
{
   LOGI( TAG, "init %d switch(s)", _num_switches );

   num_switches = _num_switches;
   list = ( Switch * )calloc( _num_switches, sizeof( Switch ) );
   // LOGI( TAG, "alloc %d byte @ 0x%04x", _num_switches * sizeof( Switch ), list );

   // configure input pins
   va_list arg;
   va_start( arg, _num_switches );

   for( int id = 0; id < _num_switches; id++ )
   {
      PinName pin = ( PinName )va_arg( arg, int );

      list[ id ] = Switch( pin );
   }
   va_end( arg );
}

// --------------------------------------------------------------------------
//
// --------------------------------------------------------------------------

Switches::~Switches( void )
{
   free( list );
   list = NULL;
}

// --------------------------------------------------------------------------
//
// --------------------------------------------------------------------------

void Switches::set( Switch_Id id, Switch::State state )
{
   clear( id );
   switches |= ( state << id );

   // LOGD( TAG, "set: id %d state %d", id, state );
}

// --------------------------------------------------------------------------
//
// --------------------------------------------------------------------------

Switch::State Switches::get( Switch_Id id )
{
   Switch::State state = ( Switch::State )( ( switches >> id ) & 1 );

   // if( state != 0 )
   // {
   //    LOGD( TAG, "get: id %d state %d", id, state );
   // }
   return state;
}

// --------------------------------------------------------------------------
//
// --------------------------------------------------------------------------

void Switches::clear( Switch_Id id )
{
   switches &= ~( 1 << id );
}

// --------------------------------------------------------------------------
//
// --------------------------------------------------------------------------
// bit  7 6 5 4 3 2 1 0
//                BTN
//                0

// scan all switches and check if any buttion is pressed

uint8_t Switches::scan( void )
{
   uint8_t scanned = 0;
   for( uint8_t i = num_switches; i > 0; )
   {
      i--;
      scanned <<= 1;
      Switch::State state = list[ i ].get();
      // LOGD( TAG, "%d switch state 0x%02x", i, state );
      set( ( Switch_Id )i, state );
   }

   // if( scanned )
   // {
   //    LOGD( TAG, "scanned 0x%02x", scanned );
   // }

   return scanned;
}

// --------------------------------------------------------------------------
//
// --------------------------------------------------------------------------
