// --------------------------------------------------------------------------
//
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

#ifndef __BUTTON_H__
#define __BUTTON_H__

#include <stdint.h>
#include "PinNames.h"      // PinName

// --------------------------------------------------------------------------
//
// --------------------------------------------------------------------------

#define BTN_DEBOUNCE_TIMEOUT          10 // ms
#define BTN_LONG_PRESSED_TIME       1000 // ms
#define BTN_VERY_LONG_PRESSED_TIME  3000 // ms

// --------------------------------------------------------------------------
//
// --------------------------------------------------------------------------

class Button
{
public:
   typedef enum State : uint8_t
   {
      Idle,                // 0
      Released = Idle,     // 0
      JustPressed,         // 1
      Pressed,             // 2
      JustReleased,        // 3  not used, used below enums
      ShortPressed,        // 4  when button is just release
      LongPressed,         // 5  dto.
      VeryLongPressed,     // 6  dto
   } State;

private:
   PinName pin;
   union
   {
      uint8_t  debounce_start_time;
      uint16_t press_start_time;
   };
   struct
   {
      bool btn_stable: 1;
      bool btn_old: 1;
      bool btn_long_pressed: 1;
      bool btn_very_long_pressed: 1;
   };
   State btn_state;                 // the state of the button returned to caller

public:
   Button( void ) {}                // !!!
   Button( uint8_t pin );
   ~Button( void );

   State get( void );
};

// --------------------------------------------------------------------------
//
// --------------------------------------------------------------------------

class Buttons: public Button
{
public:
   typedef enum Button_Id : uint8_t
   {
      BTN,
   } Button_Id;

   uint8_t num_buttons = 1;

private:
   Button* list;
   uint8_t buttons;

public:
   Buttons( uint8_t num_buttons, ... );
   ~Buttons( void );

   void set( Button_Id key, State state );
   State get( Button_Id key );
   void clear( Button_Id key );
   uint8_t scan( void );

};

// --------------------------------------------------------------------------
//
// --------------------------------------------------------------------------
#endif // __BUTTON_H__
