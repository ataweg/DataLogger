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
//
// Project       MidiMerge
//
// File          Axel Werner
//
// Author        picoOS.h
//
// --------------------------------------------------------------------------
// Changelog
//
//    2019-01-11  AWe   adapted for use with Arduino
//    08.11.2012  AWe   start implemtation
//
// --------------------------------------------------------------------------

#if defined(__GNUC__)
   #include "posPort_AVRGCC.h"

#include "picoOS.h"

// --------------------------------------------------------------------------
//
// --------------------------------------------------------------------------

//      for( i = posCONTEXT_SIZE; i != 0 ; i--)
//      {
//         *(unsigned char*)newStackPtr = 0xC5;
//         newStackPtr += posSTACK_GROWTH;
//     }

stackPtrType posInitialiseStack( stackPtrType _newStackPtr, taskFunctionType task )
{
   uint16_t usAddress;
   unsigned char *newStackPtr = ( unsigned char *)_newStackPtr;

   /* Place a few bytes of known values on the bottom of the stack.
   This is just useful for debugging. */

   *newStackPtr-- = 0x11;
   *newStackPtr-- = 0x22;
   *newStackPtr-- = 0x33;

   /* Simulate how the stack would look after a call to vPortYield() generated
   by the compiler. */

   /* The start of the task code will be popped off the stack last, so place
   it on first. */

#if defined(__AVR_ATmega2560__) || defined(__AVR_ATmega2561__)
   /* The AVR ATmega2560/ATmega2561 have 256KBytes of program memory and a 17-bit
    * program counter.  When a code address is stored on the stack, it takes 3 bytes
    * instead of 2 for the other ATmega* chips.
    *
    * Store 0 as the top byte since we force all task routines to the bottom 128K
    * of flash. We do this by using the .lowtext label in the linker script.
    *
    * In order to do this properly, we would need to get a full 3-byte pointer to
    * pxCode.  That requires a change to GCC.  Not likely to happen any time soon.
    */
   usAddress = ( uint16_t ) task;
   *newStackPtr-- = ( usAddress & ( uint16_t ) 0x00ff );

   usAddress >>= 8;
   *newStackPtr-- = ( usAddress & ( uint16_t ) 0x00ff );
   *newStackPtr-- = 0;
#else
   usAddress = ( uint16_t ) task;
   *newStackPtr-- = ( usAddress & ( uint16_t ) 0x00ff );

   usAddress >>= 8;
   *newStackPtr-- = ( usAddress & ( uint16_t ) 0x00ff );
#endif

   /* Next simulate the stack as if after a call to posSAVE_CONTEXT().
   posSAVE_CONTEXT places the flags on the stack immediately after r0
   to ensure the interrupts get disabled as soon as possible, and so ensuring
   the stack use is minimal should a context switch interrupt occur. */
   *newStackPtr-- = 0x00;  /* R0 */
   *newStackPtr-- = 0x80;  //  set interrupt enable

#if defined(__AVR_ATmega2560__) || defined(__AVR_ATmega2561__)
   /* If we have an ATmega256x, we are also saving the RAMPZ and EIND registers.
    * We should default those to 0.
    */
   *newStackPtr-- = 0x00;  /* EIND */
   *newStackPtr-- = 0x00;  /* RAMPZ */
#endif

   /* Now the remaining registers.   The compiler expects R1 to be 0. */
   *newStackPtr-- = 0x00;  /* R1  */
   *newStackPtr-- = 0x02;  /* R2  */
   *newStackPtr-- = 0x03;  /* R3  */
   *newStackPtr-- = 0x04;  /* R4  */
   *newStackPtr-- = 0x05;  /* R5  */
   *newStackPtr-- = 0x06;  /* R6  */
   *newStackPtr-- = 0x07;  /* R7  */
   *newStackPtr-- = 0x08;  /* R8  */
   *newStackPtr-- = 0x09;  /* R9  */
   *newStackPtr-- = 0x10;  /* R10 */
   *newStackPtr-- = 0x11;  /* R11 */
   *newStackPtr-- = 0x12;  /* R12 */
   *newStackPtr-- = 0x13;  /* R13 */
   *newStackPtr-- = 0x14;  /* R14 */
   *newStackPtr-- = 0x15;  /* R15 */
   *newStackPtr-- = 0x16;  /* R16 */
   *newStackPtr-- = 0x17;  /* R17 */
   *newStackPtr-- = 0x18;  /* R18 */
   *newStackPtr-- = 0x19;  /* R19 */
   *newStackPtr-- = 0x20;  /* R20 */
   *newStackPtr-- = 0x21;  /* R21 */
   *newStackPtr-- = 0x22;  /* R22 */
   *newStackPtr-- = 0x23;  /* R23 */

#ifdef USE_PARAMETERS
   /* Place the parameter on the stack in the expected location. */
   usAddress = ( uint16_t ) pvParameters;
   *newStackPtr-- = ( usAddress & ( uint16_t ) 0x00ff );

   usAddress >>= 8;
   *newStackPtr-- = ( usAddress & ( uint16_t ) 0x00ff );
#else
   *newStackPtr-- = 0x24;  /* R24 */
   *newStackPtr-- = 0x25;  /* R25 */
#endif
   *newStackPtr-- = 0x26;  /* R26 X */
   *newStackPtr-- = 0x27;  /* R27 */
   *newStackPtr-- = 0x28;  /* R28 Y */
   *newStackPtr-- = 0x29;  /* R29 */
   *newStackPtr-- = 0x30;  /* R30 Z */
   *newStackPtr-- = 0x31;  /* R31 */

   return (stackPtrType)newStackPtr;
}

// --------------------------------------------------------------------------
//
// --------------------------------------------------------------------------
#else
   #error "wrong compiler, require gnu avr compiler"
#endif
