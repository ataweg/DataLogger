#ifndef __POSPORT_AVRGCC_H__
#define __POSPORT_AVRGCC_H__
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
//    08.11.2012  AWe   start implementation
//
// --------------------------------------------------------------------------

#if defined(__AVR__)
   #include <avr/io.h>

   #ifdef SPH
      // we have a 16 bit stack pointer
      typedef unsigned short stackPtrType;
      typedef unsigned short stackSizeType;
   #else
      // we have a 8 bit stack pointer
      typedef unsigned char stackPtrType;
      typedef unsigned char stackSizeType;
   #endif

// --------------------------------------------------------------------------
//
// --------------------------------------------------------------------------

// direction in which the stack growths, -1 means stack grows downwards
#define posSTACK_GROWTH            ( -1 )

// Critical section management.
#define posENTER_CRITICAL()                                    \
            asm volatile ( "in   __tmp_reg__, __SREG__" ::);   \
            asm volatile ( "cli" ::);                          \
            asm volatile ( "push __tmp_reg__" ::)

#define posEXIT_CRITICAL()                                     \
            asm volatile ( "pop  __tmp_reg__" ::);             \
            asm volatile ( "out  __SREG__, __tmp_reg__" ::)

#define posDISABLE_INTERRUPTS()    asm volatile ( "cli" :: );
#define posENABLE_INTERRUPTS()     asm volatile ( "sei" :: );

// --------------------------------------------------------------------------
//
// --------------------------------------------------------------------------

#if defined(__AVR_ATmega2560__) || defined(__AVR_ATmega2561__)
   #error "curently not implemented for ATmega2560, ATmega2561"

#else
#define posSAVE_CONTEXT() \
   asm volatile ( \
      "push r0                \n\t" /* R0   */  \
      "in   r0, __SREG__      \n\t" /*      */  \
      "cli                    \n\t" /*      */  \
      "push r0                \n\t" /* SREG */  \
      "push r1                \n\t" /* R1   */  \
      "clr  r1                \n\t" /* R1   */  \
      "push r2                \n\t" /* R2   */  \
      "push r3                \n\t" /* R3   */  \
      "push r4                \n\t" /* R4   */  \
      "push r5                \n\t" /* R5   */  \
      "push r6                \n\t" /* R6   */  \
      "push r7                \n\t" /* R7   */  \
      "push r8                \n\t" /* R8   */  \
      "push r9                \n\t" /* R9   */  \
      "push r10               \n\t" /* R10  */  \
      "push r11               \n\t" /* R11  */  \
      "push r12               \n\t" /* R12  */  \
      "push r13               \n\t" /* R13  */  \
      "push r14               \n\t" /* R14  */  \
      "push r15               \n\t" /* R15  */  \
      "push r16               \n\t" /* R16  */  \
      "push r17               \n\t" /* R17  */  \
      "push r18               \n\t" /* R18  */  \
      "push r19               \n\t" /* R19  */  \
      "push r20               \n\t" /* R20  */  \
      "push r21               \n\t" /* R21  */  \
      "push r22               \n\t" /* R22  */  \
      "push r23               \n\t" /* R23  */  \
      "push r24               \n\t" /* R24  */  \
      "push r25               \n\t" /* R25  */  \
      "push r26               \n\t" /* R26 X */ \
      "push r27               \n\t" /* R27   */ \
      "push r28               \n\t" /* R28 Y */ \
      "push r29               \n\t" /* R29   */ \
      "push r30               \n\t" /* R30 Z */ \
      "push r31               \n\t" /* R31   */ \
/*    "lds  r26, pxCurrentTCB       \n\t" */ \
/*    "lds  r27, pxCurrentTCB + 1   \n\t" */ \
/*    "in   r0, __SP_L__            \n\t" */ \
/*    "st   x+, r0                  \n\t" */ \
/*    "in   r0, __SP_H__            \n\t" */ \
/*    "st   x+, r0                  \n\t" */ \
   );
#endif

// --------------------------------------------------------------------------
//
// --------------------------------------------------------------------------

/*
 * Opposite to posSAVE_CONTEXT().  Interrupts will have been disabled during
 * the context save so we can write to the stack pointer.
 */

#if defined(__AVR_ATmega2560__) || defined(__AVR_ATmega2561__)
   #error "curently not implemented for ATmega2560, ATmega2561"

#else
#define posRESTORE_CONTEXT() \
   asm volatile ( \
/*    "lds  r26, pxCurrentTCB       \n\t" */ \
/*    "lds  r27, pxCurrentTCB + 1   \n\t" */ \
/*    "ld   r28, x+                 \n\t" */ \
/*    "out  __SP_L__, r28           \n\t" */ \
/*    "ld      r29, x+              \n\t" */ \
/*    "out  __SP_H__, r29           \n\t" */ \
      "pop  r31               \n\t" \
      "pop  r30               \n\t" \
      "pop  r29               \n\t" \
      "pop  r28               \n\t" \
      "pop  r27               \n\t" \
      "pop  r26               \n\t" \
      "pop  r25               \n\t" \
      "pop  r24               \n\t" \
      "pop  r23               \n\t" \
      "pop  r22               \n\t" \
      "pop  r21               \n\t" \
      "pop  r20               \n\t" \
      "pop  r19               \n\t" \
      "pop  r18               \n\t" \
      "pop  r17               \n\t" \
      "pop  r16               \n\t" \
      "pop  r15               \n\t" \
      "pop  r14               \n\t" \
      "pop  r13               \n\t" \
      "pop  r12               \n\t" \
      "pop  r11               \n\t" \
      "pop  r10               \n\t" \
      "pop  r9                \n\t" \
      "pop  r8                \n\t" \
      "pop  r7                \n\t" \
      "pop  r6                \n\t" \
      "pop  r5                \n\t" \
      "pop  r4                \n\t" \
      "pop  r3                \n\t" \
      "pop  r2                \n\t" \
      "pop  r1                \n\t" \
      "pop  r0                \n\t" \
      "out  __SREG__, r0      \n\t" \
      "pop  r0                \n\t" \
   );
#endif

// --------------------------------------------------------------------------
//
// --------------------------------------------------------------------------

// push value to stack
#define posPushShort( value) \
      asm volatile ( "push %A0 \n\t" \
                     "push %B0 \n\t" \
                     : "+r" (value) \
                   );

// get value from stack
#define posPopShort( value) \
      asm volatile ( "pop %B0 \n\t" \
                     "pop %A0 \n\t" \
                     : "+r" (waitForReadyFunc) \
                   );

// --------------------------------------------------------------------------
//
// --------------------------------------------------------------------------

#else
   #error "wrong compiler, require gnu avr compiler"
#endif

#endif // __POSPORT_AVRGCC_H__
