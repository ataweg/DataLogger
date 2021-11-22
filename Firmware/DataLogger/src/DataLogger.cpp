// --------------------------------------------------------------------------
//
// Project       DataLogger
//
// File          DataLogger.cpp
//
// Author        Axel Werner
//
// --------------------------------------------------------------------------
// Changelog
//
// 2020-06-14  AWe   move  BuildMsg to DataLogger.ino which is always compiled,
//                   so we have the current build date and time
// 2020-06-01  AWe   moved tasks to their own files
// 2020-05-29  AWe   add extended support for sdcard
// 2020-04-16  AWe   initial version taken from
//                      https://www.arduino.cc/en/Tutorial/Datalogger
//                      F:\Arduino\libraries\SD\examples\Datalogger\Datalogger.ino
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
static const char TAG[] PROGMEM = tag( "DataLogger" );

// --------------------------------------------------------------------------
//
// --------------------------------------------------------------------------

#ifdef ARDUINO
   #include <Arduino.h>             // pinMode(), digitalRead(), INPUT_PULLUP, millis(), ...
   #include "picoOS/picoOS.h"       // posCreateTask(), yield()
#else
   #include "WArduino.h"
#endif

#include "io.h"                        // _digitalWrite(), _digitalRead()
#include "DataLogger_config.h"
#include "DataLogger.h"
#include "SdCardTask.h"
#include "UiTask.h"
#include "Led.h"
#include "Switch.h"

// --------------------------------------------------------------------------
// protoypes
// --------------------------------------------------------------------------

ISR( TIMER2_OVF_vect );
void Timer2init( void );

// --------------------------------------------------------------------------
//
// --------------------------------------------------------------------------

sys_flags_t flags = { 0 };

Led Led_R( LED_R_Pin );
Led Led_G( LED_G_Pin );
Led Led_Debug( DEBUG_LED );

Switch CardDetect( CD_Pin );
Switch WriteProtect( WP_Pin );

//------------------------------------------------------------------------------
// Setup
//------------------------------------------------------------------------------

extern uint16_t __data_start;
extern uint8_t* __brkval;

void setup()
{
   // fill free memory with pattern

   uint8_t *start = __brkval;
   uint8_t *tmp = start;
   uint8_t *end = SP;

   while( tmp <  end )
      *tmp++ = 0xC5;

//   pinMode( BTN_Pin,   INPUT_PULLUP );
//   pinMode( SS_Pin,    INPUT_PULLUP );
//   pinMode( CD_Pin,    INPUT_PULLUP );
//   pinMode( WP_Pin,    INPUT_PULLUP );

//   pinMode( LED_R_Pin, OUTPUT );
//   pinMode( LED_G_Pin, OUTPUT );
//   pinMode( DEBUG_LED, OUTPUT );
//
//   _digitalWrite( LED_R_Pin, LOW );
//   _digitalWrite( LED_G_Pin, LOW );
//   _digitalWrite( DEBUG_LED, LOW );

   Led_R.off();
   Led_G.off();
   Led_Debug.off();

#if ( LOG_LOCAL_LEVEL >= LOG_DEBUG )
   pinMode( OSZI_YELLOW_Pin, OUTPUT );
   pinMode( OSZI_BLUE_Pin,   OUTPUT );
   pinMode( OSZI_RED_Pin,    OUTPUT );
   pinMode( OSZI_GREEN_Pin,  OUTPUT );

   _digitalWrite( OSZI_YELLOW_Pin, LOW );
   _digitalWrite( OSZI_BLUE_Pin,   LOW );
   _digitalWrite( OSZI_RED_Pin,    LOW );
   _digitalWrite( OSZI_GREEN_Pin,  LOW );
#endif

#ifdef USE_SERIAL_OUTPUT
   // Open serial communications and wait for port to open:
   Serial.begin( 115200 );
   delay( 2000 );

   while( !Serial )
   {
      ; // wait for serial port to connect. Needed for native USB port only
   }

   for( uint8_t i = 0; i < num_buildmsg_str; i++ )
   {
      const char *ptr_P = ( char * ) pgm_read_ptr( &str_BuildMsg[ i ] );
      Serial.println( ( const __FlashStringHelper * ) ptr_P );
   }

 #ifdef EVAL_BREADBOARD
   Serial.println( F( "Eval Board" ) );
 #else
   // ARDUINO_UNO_REV3
   Serial.println( F( "Arduino Uno Board" ) );
 #endif
#endif   // USE_SERIAL_OUTPUT

   LOGD( TAG, "start: 0x%04x end: 0x%04x", start, end );
   LOGD( TAG, "__data_start: 0x%04x", ( uint8_t * )&__data_start );
   // dump_data_hex( ( const char* )&__data_start, 0x800 );

   posDISABLE_INTERRUPTS();  // disable interrupts
   posInit( 144 ); // put your setup code here, to run once:
   posENABLE_INTERRUPTS();   // enable interrupts

   Timer2init();

   // create the sdcard task
   uint8_t UiTaskID = posCreateTask( SdCardTask, 512 );

   LOGD( TAG, "posCurrentStackEnd: 0x%04x", posGetCurrentStackEnd() );
   LOGD( TAG, "posStackEnd: 0x%04x", posGetStackEnd( UiTaskID ) );

   LOGD( TAG, "posSP: 0x%04x", SP );
   LOGD( TAG, "posStackSize: %d @ SP 0x%04x", posCheckStack(), SP );

   UiTask_setup();

   LOGD( TAG, "posSP: 0x%04x", SP );
   LOGD( TAG, "posStackSize: %d @ SP 0x%04x", posCheckStack(), SP );

   // dump_data_hex( ( const char* )&__data_start, 0x800 );
}

// --------------------------------------------------------------------------
// The loop
// --------------------------------------------------------------------------

void loop()
{
   UiTask_loop();
   yield();
}

// --------------------------------------------------------------------------
// Timer 2 stuff  (clock source)
// --------------------------------------------------------------------------

void Timer2init( void )
{
   // Timer 2
   noInterrupts();             // Disable all interrupts temporarily
   TCCR2A = 0;                 // Configure timer2 in normal mode (pure increment counting, no PWM etc.)
   TCCR2B = 0;

   TCNT2 = TIMER2_RELOAD;      // Preset timer according to above calculation
   TCCR2B |= 7;                // 1024 Specify as Prescale value
   TIMSK2 |= ( 1 << TOIE2 );   // Activate Timer Overflow Interrupt
   interrupts();               // Arm all interrupts

   LOGI( TAG, "timer 2 divider %d", TIMER2_DIVIDER );
}

// --------------------------------------------------------------------------
// Timer Interrupt Service Routine
// --------------------------------------------------------------------------

// 10ms interrupt for led update

ISR( TIMER2_OVF_vect )
{
   TCNT2 = TIMER2_RELOAD;                // Preset counter again

   // process leds
   // leds.update();

   Led_R.update();
   Led_G.update();
   Led_Debug.update();
}

// --------------------------------------------------------------------------
//
// --------------------------------------------------------------------------
