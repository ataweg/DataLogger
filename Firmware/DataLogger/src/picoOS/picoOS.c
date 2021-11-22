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
// Author        picoOS.c
//
// --------------------------------------------------------------------------
// Changelog
//
//    2020-06-02  AWe   add support for stack overflow checking
//    2020-06-02  AWe   add support for posSemaphores
//    2019-01-11  AWe   adapted for use with Arduino
//                       see also https://www.arduino.cc/en/Reference/Scheduler
//    14.08.2014  AWe   adapt for macrochip PIC controller
//    08.11.2012  AWe   start implemtation
//
// --------------------------------------------------------------------------

#include "picoOS.h"

// --------------------------------------------------------------------------
//
// --------------------------------------------------------------------------

extern unsigned long posMillis( void );

unsigned char currentTask;
unsigned char numTasks;

stackPtrType taskStackPtrList[ 1 + MAX_TASKS ]; // main program counts as task zero
stackPtrType taskMaxStackPtrList[ 1 + MAX_TASKS ];
stackPtrType stackEnd;

posSemaphores_t posSemaphores = 0;

// --------------------------------------------------------------------------
//
// --------------------------------------------------------------------------

// SP defined in C:\Program Files (x86)\Arduino\hardware\tools\avr\avr\include\avr\common.h(89):

void posInit( stackSizeType root_stack_size )
{
   currentTask = 0;
   numTasks = 1;                     // the first task is the root task
   stackEnd = SP + posSTACK_GROWTH * root_stack_size;
   taskMaxStackPtrList[ currentTask ] = stackEnd;
}

//----------------------------------------------------------------------
// Start Task or change to next task
//----------------------------------------------------------------------
//
// execute the task with the specified taskID
//
// on entry return address of caller is saved on its task stack
// on exit return to address popped from stack of the new task
//
// for atmel gcc-avr: used registers
//    r18, r24, r25, r30, r31

void posTaskSwitch( signed char taskID )
{
   if( taskID < ( signed char )numTasks )
   {
      stackPtrType stackPtrReg;

      // save the current processor status
      posSAVE_CONTEXT();

      // save the stack pointer of the current task
      stackPtrReg = SP;
      taskStackPtrList[ currentTask ] = stackPtrReg;

      if( taskID == NEXT_TASK )
      {
         // switch to next task, round robbin priority
         currentTask++;                            // select next task
         if( !( currentTask < numTasks ) )
         {
            currentTask = ROOT_TASK;               // wrap around
         }
      }
      else
      {
         // start the task matching the taskID
         currentTask = taskID;
      }

      // get stack pointer from the taskStackPtrList
      stackPtrReg = taskStackPtrList[ currentTask ];

      // load stack pointer register
      SP = stackPtrReg;

      // load the processor status for the new task
      // and return to the next address of task
      posRESTORE_CONTEXT();
   }
}

//----------------------------------------------------------------------
// Wait Task
//----------------------------------------------------------------------
// posWait waits for called function until it returns with not zero

// for PIC
//  on function entry the return address is stored in the hardware stack
//  so we have to exchange this return address with the one for the next task
//  can have return values in W

void posWait( unsigned char( *waitForReadyFunc )( void ) )
{
   while( ( *waitForReadyFunc )() == 0 )
   {
      // save address of the wait function to stack
      posPushShort( waitForReadyFunc );

      // save the current context, change the task, and restore its context
      posNextTask();

      // get address of the wait function from stack
      posPopShort( waitForReadyFunc );
   }
}

// --------------------------------------------------------------------------
// create task
// --------------------------------------------------------------------------
// set task stack pointer with address to memory
// save TaskAddress to task stack

unsigned char posCreateTask( taskFunctionType task, stackSizeType stack_size )
{
   unsigned char rc = 0; // means could not create a new task

   if( numTasks < 1 + MAX_TASKS )
   {
      stackPtrType  newStackPtr;
      unsigned char newTaskId = numTasks;

      numTasks++;
      newStackPtr = stackEnd;

      // make room for the stack
      stackEnd += posSTACK_GROWTH * stack_size;

      // setup context area on the stack of the newly created task
      newStackPtr = posInitialiseStack( newStackPtr, task );
      taskStackPtrList[ newTaskId ] = newStackPtr;
      taskMaxStackPtrList[ newTaskId ] = stackEnd;

      rc = newTaskId;
   }

   return rc;
}

// --------------------------------------------------------------------------
//
// --------------------------------------------------------------------------

void posDelay( unsigned long timeout )
{
   volatile unsigned long t0 = posMillis();

   while( posMillis() - t0 < timeout )
   {
      posNextTask();
   }
}

// --------------------------------------------------------------------------
// Semaphores
// --------------------------------------------------------------------------

void posSemaphoreGive( posSemaphores_t semaphores )
{
   posSemaphores |= semaphores;
}

// --------------------------------------------------------------------------
//
// --------------------------------------------------------------------------

posSemaphores_t posSemaphoreTake( posSemaphores_t semaphores )
{
   posSemaphores_t semaphores_current = posSemaphores & semaphores;
   if( semaphores_current )
   {
      // clear sema
      posSemaphores &= ~semaphores;
      return semaphores_current;
   }
   else
      return 0;
}

// --------------------------------------------------------------------------
//
// --------------------------------------------------------------------------

posSemaphores_t posSemaphoreGet( posSemaphores_t semaphores )
{
   posSemaphores_t semaphores_current = posSemaphores & semaphores;
   return semaphores_current;
}

// --------------------------------------------------------------------------
//
// --------------------------------------------------------------------------

posSemaphores_t posSemaphoreWait( posSemaphores_t semaphores, unsigned long timeout )
{
   unsigned long t0 = posMillis();

   do
   {
      posSemaphores_t semaphores_current = posSemaphores & semaphores;
      if( semaphores_current )
      {
         // clear sema
         posSemaphores &= ~semaphores;
         return semaphores_current;
      }
      posNextTask();
   }
   while( posMillis() - t0 < timeout );

   return posTimeout;
}

// --------------------------------------------------------------------------
//
// --------------------------------------------------------------------------

short posCheckStack( void )
{
   return ( ( short )taskMaxStackPtrList[ currentTask ] - ( short )SP - sizeof( stackPtrType ) ) * posSTACK_GROWTH;
}

stackPtrType posGetStackEnd( signed char task_num )
{
   return taskMaxStackPtrList[ task_num ];
}

stackPtrType posGetCurrentStackEnd( void )
{
   return taskMaxStackPtrList[ currentTask ];
}

// --------------------------------------------------------------------------
//
// --------------------------------------------------------------------------

void yield( void )
{
   posNextTask();
}

// --------------------------------------------------------------------------
//
// --------------------------------------------------------------------------

#ifdef USE_POS_MEMORY_LOCK
static posSemaphores_t posMemoryLockCounter = 0;
static unsigned char   posMemoryLockTask = 0;

posSemaphores_t posMemoryLock( void )
{
   // do the current task set the lock
   while( ( posMemoryLockCounter != 0 ) && ( currentTask != posMemoryLockTask ) )
   {
      // othen we have to wait
      posNextTask();
   }

   posMemoryLockTask = currentTask;
   posMemoryLockCounter++;

   return posMemoryLockCounter;
}

posSemaphores_t posMemoryUnlock( void )
{
   // the task who set the lock, can also remove it
   if( ( posMemoryLockCounter != 0 ) && ( currentTask == posMemoryLockTask ) )
   {
      posMemoryLockCounter--;
   }
   return posMemoryLockCounter;
}
#endif
// --------------------------------------------------------------------------
//
// --------------------------------------------------------------------------

