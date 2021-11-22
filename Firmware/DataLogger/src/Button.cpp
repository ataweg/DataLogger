// --------------------------------------------------------------------------
//
// 2020-02-06  AWe   rename some variables for better understanding
// 2019-03-22  AWe   fix button scan function
// 2019-02-04  AWe   initial version
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
static const char TAG[] PROGMEM = tag( "Button" );

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
#include "Button.h"

// --------------------------------------------------------------------------
//
// --------------------------------------------------------------------------

Button::Button( uint8_t _pin )
{
   LOGD( TAG, "Button constructor pin %d", _pin );

   pin = _pin;
   pinMode( pin, INPUT_PULLUP );

   btn_stable = false;
   btn_old = false;
   btn_state = Idle;     // the state of the button returned to caller
}

// --------------------------------------------------------------------------
//
// --------------------------------------------------------------------------

Button::~Button( void )
{
}

// --------------------------------------------------------------------------
//
// --------------------------------------------------------------------------

// see also https://github.com/evert-arias/EasyButton

Button::State Button::get( void )
{
   bool btn_in = !_digitalRead( pin, 1 );  // true: button pressed, false: button not pressed
   uint16_t sample_time = ( uint16_t )millis();
   State rc = btn_state;

   if( btn_in != btn_old )
   {
      // button pin has changed, button can bounce
      debounce_start_time = ( uint8_t )sample_time;
      btn_stable = false;
      btn_old = btn_in;
   }
   else
   {
      // button pin has not changed, so wait for debounce time is over

      if( !btn_stable )
      {
         // calculation is ok, as long as the time between two samples is less than 256 ms
         uint8_t debounce_time = ( uint8_t )sample_time - debounce_start_time;

         if( BTN_DEBOUNCE_TIMEOUT < debounce_time )
         {
            // button debouncing periode is over
            btn_stable = true;
         }
      }
      else
      {
         // button is stable
         if( btn_state == Idle )
         {
            if( btn_in )
            {
               // button is just pressed
               btn_state = Pressed;
               rc = JustPressed;
               LOGI( TAG, "get: pin %d pressed", pin );
               // start press counter
               press_start_time = sample_time;
            }
         }
         else if( btn_state == Pressed )
         {
            if( !btn_in )
            {
               // button is just released
               LOGD( TAG, "get: pin %d released", pin );
               btn_state = Idle;

               uint16_t press_time = sample_time - press_start_time;
               if( press_time > BTN_VERY_LONG_PRESSED_TIME )
               {
                  rc = VeryLongPressed;
                  LOGD( TAG, "get: pin %d very long pressed, %d ms", pin, press_time );
               }
               else if( press_time > BTN_LONG_PRESSED_TIME )
               {
                  rc = LongPressed;
                  LOGD( TAG, "get: pin %d long pressed, %d ms", pin, press_time );
               }
               else
               {
                  rc = ShortPressed;
                  LOGD( TAG, "get: pin %d short pressed, %d ms", pin, press_time );
               }
            }
         }
      }
   }

   return rc;
}

// --------------------------------------------------------------------------
//
// --------------------------------------------------------------------------

Buttons::Buttons( uint8_t _num_buttons, ... )
{
   LOGI( TAG, "init %d button(s)", _num_buttons );

   num_buttons = _num_buttons;
   list = ( Button * )calloc( _num_buttons, sizeof( Button ) );
   // LOGI( TAG, "alloc %d byte @ 0x%04x", _num_buttons * sizeof( Button ), list );

   // configure input pins
   va_list arg;
   va_start( arg, _num_buttons );

   for( int id = 0; id < _num_buttons; id++ )
   {
      PinName pin = ( PinName )va_arg( arg, int );

      list[ id ] = Button( pin );
   }
   va_end( arg );
}

// --------------------------------------------------------------------------
//
// --------------------------------------------------------------------------

Buttons::~Buttons( void )
{
   free( list );
   list = NULL;
}

// --------------------------------------------------------------------------
//
// --------------------------------------------------------------------------

void Buttons::set( Button_Id key, Button::State state )
{
   if( state != 0 )
   {
      clear( key );
      buttons |= state << ( key * 3 );

      // LOGD( TAG, "set: key %d state %d", key, state );
   }
}

// --------------------------------------------------------------------------
//
// --------------------------------------------------------------------------

Button::State Buttons::get( Button_Id key )
{
   Button::State state = ( Button::State )( ( buttons >> ( key * 3 ) ) & 7 );

   // if( state != 0 )
   // {
   //    LOGD( TAG, "get: key %d state %d", key, state );
   // }
   return state;
}

// --------------------------------------------------------------------------
//
// --------------------------------------------------------------------------

void Buttons::clear( Button_Id key )
{
   buttons &= ~( 7 << ( key * 3 ) );
}

// --------------------------------------------------------------------------
//
// --------------------------------------------------------------------------
// bit  7 6 5 4 3 2 1 0
//                BTN
//                0

// scan all buttons and check if any buttion is pressed

uint8_t Buttons::scan( void )
{
   uint8_t scanned = 0;
   for( uint8_t i = num_buttons; i > 0; )
   {
      i--;
      scanned <<= 3;
      Button::State state = list[ i ].get();
      // LOGD( TAG, "%d button state 0x%02x", i, state );
      if( state != Button::Idle )
      {
         scanned |= state;
         set( ( Button_Id )i, state );
      }
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
