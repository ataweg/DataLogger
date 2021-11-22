// --------------------------------------------------------------------------
//
// Project       io
//
// File          led.h
//
// Author        Axel Werner
//
// --------------------------------------------------------------------------
// Changelog
//
//    2018-02-15  AWe   adept for use with ESP32 and ESP-IDF
//    2017-11-18  AWe
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

#ifndef __LED_H__
#define __LED_H__

#include "PinNames.h"      // PinName

// --------------------------------------------------------------------------
//
// --------------------------------------------------------------------------

class Led
{
public:
   typedef enum Mode : uint8_t
   {
      Off,
      Flash,
      Blink,
      On,
      None = 0xff,
   } Mode;

private:
   union
   {
      PinName pin;
      uint8_t id;
   };
   struct
   {
      uint8_t mode;
      uint8_t counter;    // counts down number of repeats
      uint8_t on_time;
      uint8_t off_time;
   } state;
   uint8_t    timer;
   union
   {
      struct
      {
         unsigned no_update: 1;
         unsigned led_on: 1;
         unsigned flash_phase: 1;
         unsigned busy: 1;
      };
      uint8_t val;
   } flags;

   struct          // used for led configuration
   {
      uint8_t mode;        // new mode for the led, NONE = no new mode
      uint8_t repeat;      // copied to counter
      uint8_t on_time;
      uint8_t off_time;
   } config;

public:
   Led( uint8_t pin );
   ~Led( void );

   uint8_t get( void );
   void set( uint8_t mode, uint8_t on_time, uint8_t off_time, uint8_t repeat );
   bool busy( void );
   void update( void );

   void on( void )                { set( On,      0,  0, 0 ); }
   void off( void )               { set( Off,     0,  0, 0 ); }
   void flash_fast( void )        { set( Blink,   3, 10, 0 ); }      // 300ms on,    1s off
   void flash( void )             { set( Blink,   3, 20, 0 ); }      // 300ms on,    2s off
   void flash_slow( void )        { set( Blink,   3, 30, 0 ); }      // 300ms on,    3s off
   void blink_fast( void )        { set( Blink,   3,  3, 0 ); }      // 300ms on, 300ms off
   void blink( void )             { set( Blink,   8,  8, 0 ); }      // 800ms on, 800ms off
   void blink_slow( void )        { set( Blink,  15, 15, 0 ); }      //  1,5s on,  1,5s off
   void short_oneshot( void )     { set( Flash,   3,  0, 0 ); }      // 300ms on, then alway off
   void oneshot( void )           { set( Flash,   8,  0, 0 ); }      // 800ms on, then alway off
   void long_oneshot( void )      { set( Flash,  15,  0, 0 ); }      //  1,5s on, then alway off
};

#if 0
// --------------------------------------------------------------------------
//
// --------------------------------------------------------------------------

extern int SYS_LED;
extern int INFO_LED;

// --------------------------------------------------------------------------
//
// --------------------------------------------------------------------------

class Leds: public Led
{
public:

   void init( int _num_leds, ... );
   void update( void );
   void deinit( void );

public:
   SysLed_On()                 { SysLed.set( LED_ON,  0, 0, 0 )  } ;
   SysLed_Off()                { SysLed.set( LED_OFF, 0, 0, 0 )  } ;
   InfoLed_On()                { InfoLed.set( LED_ON,  0, 0, 0 ) } ;
   InfoLed_Off()               { InfoLed.set( LED_OFF, 0, 0, 0 ) } ;

   setSysLed( onoff )          { SysLed.set( onoff, 0, 0, 0 )    } ;
   setInfoLed( onoff )         { InfoLed.set( onoff, 0, 0, 0 )   } ;
};
#endif

// --------------------------------------------------------------------------
//
// --------------------------------------------------------------------------
#endif // __LED_H__