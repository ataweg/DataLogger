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

#ifndef __SWITCH_H__
#define __SWITCH_H__

#include <stdint.h>
#include "PinNames.h"      // PinName

// --------------------------------------------------------------------------
//
// --------------------------------------------------------------------------

#define SW_DEBOUNCE_TIMEOUT          10 // ms

// --------------------------------------------------------------------------
//
// --------------------------------------------------------------------------

class Switch
{
public:
   typedef enum State : bool
   {
      Off = false,         // 0
      On  = true,          // 1
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
      bool sw_stable: 1;
      bool sw_old: 1;
      bool inverted: 1;
   };
   State sw_state;                 // the state of the switch returned to caller

public:
   Switch( void ) {}                // !!!
   Switch( uint8_t pin, bool inverted = true );  // active low switch
   ~Switch( void );

   State get( void );
};

// --------------------------------------------------------------------------
//
// --------------------------------------------------------------------------

class Switches: public Switch
{
public:
   typedef enum Switch_Id : uint8_t
   {
      SW,
   } Switch_Id;

   uint8_t num_switches = 1;

private:
   Switch* list;
   uint8_t switches;

public:
   Switches( uint8_t num_switches, ... );
   ~Switches( void );

   void set( Switch_Id key, State state );
   State get( Switch_Id key );
   void clear( Switch_Id key );
   uint8_t scan( void );

};

// --------------------------------------------------------------------------
//
// --------------------------------------------------------------------------
#endif // __SWITCH_H__
