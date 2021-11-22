// --------------------------------------------------------------------------
//
// 2020-05-25  AWe   adapted for use in DataLogger project
// 2019-02-13  AWe   Pin D10 cannot used as input
//                   see C:\Program Files (x86)\Arduino\hardware\arduino\avr\libraries\SPI\src\SPI.cpp(47):
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

#ifndef __DATALOGGER_CONFIG_H__
#define __DATALOGGER_CONFIG_H__

#include "PinNames.h"      // PinName

// --------------------------------------------------------------------------
//
// --------------------------------------------------------------------------

// #define ARDUINO_UNO_REV3
#define EVAL_BREADBOARD

// serial output requires 1,816 kB flash memory
#define USE_SERIAL_OUTPUT

// --------------------------------------------------------------------------

// LED_BUILDIN defineid in ...\avr\variants\standard\pins_arduino.h

#if defined( ARDUINO_UNO_REV3 )
   // Arduino UNO rev3
   #define DEBUG_LED             LED_BUILTIN       // normally pin D13 (SCK)
#elif defined( EVAL_BREADBOARD )
   // arduino uno rev3 with breadborad
   #define DEBUG_LED             D9
#else
   // atmega318p breadboard
   #define DEBUG_LED             D4
#endif

// --------------------------------------------------------------------------

#if defined( ARDUINO_UNO_REV3 )

   #define BTN_Pin               NC       // n.c.

   #define LED_R_Pin             NC       // n.c.
   #define LED_G_Pin             NC       // n.c.

   #define OSZI_YELLOW_Pin       NC       // n.c.
   #define OSZI_BLUE_Pin         NC       // n.c.
   #define OSZI_RED_Pin          NC       // n.c.
   #define OSZI_GREEN_Pin        NC       // n.c.

   // pins for sdcard connector
   #define SS_Pin                SS       // pin 10
   #define CD_Pin                NC       // Card Detect
   #define WP_Pin                NC       // Write Protect

#elif defined( EVAL_BREADBOARD )

   #define BTN_Pin               D4

   #define LED_R_Pin             D2
   #define LED_G_Pin             D3

 #if ( LOG_LOCAL_LEVEL >= LOG_DEBUG )
   #define OSZI_YELLOW_Pin       A0
   #define OSZI_BLUE_Pin         A1
   #define OSZI_RED_Pin          A2
   #define OSZI_GREEN_Pin        A3
 #else
   #define OSZI_YELLOW_Pin       NC       // n.c.
   #define OSZI_BLUE_Pin         NC       // n.c.
   #define OSZI_RED_Pin          NC       // n.c.
   #define OSZI_GREEN_Pin        NC       // n.c.
 #endif

   // pins for sdcard connector
   #define SS_Pin                D10
   #define CD_Pin                D8       // Card Detect
   #define WP_Pin                D5       // Write Protect
#endif

// --------------------------------------------------------------------------
// Timer 2 configuration
// --------------------------------------------------------------------------

#define CPU_FREQUENCY      16000000L      // 16MHz
#define TIMER2_PRESCALER   1024
#define LEDS_UPDATE_RATE   100            // 100HZ - 10ms
#define TIMER2_DIVIDER     ( F_CPU/TIMER2_PRESCALER/LEDS_UPDATE_RATE + 1)

#define TIMER2_RELOAD      ( 256 - TIMER2_DIVIDER )

// --------------------------------------------------------------------------
//
// --------------------------------------------------------------------------
#endif // __DATALOGGER_CONFIG_H__
