#ifndef __PICOOS_CONFIG_H__
#define __PICOOS_CONFIG_H__

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

// #define USE_POS_MEMOCK_LOCK
// #define USE_POS_TIMER

#define DEFAULT_STACKSIZE     256
#define ROOT_STACKSIZE        256
#define MAX_TASKS             1
#define NUM_SEMAPHORES        8

// define semaphores used in this project

#define SysReset           ( 1 << 0 )
#define StartStopCapture   ( 1 << 1 )
#define RestartCapture     ( 1 << 2 )
#define I2C_ReceiveEvent   ( 1 << 3 )
#define AllSemaphores      ( ( 1 << ( NUM_SEMAPHORES - 1 ) ) - 1 )

#define posTimeout         ( -1  )

// --------------------------------------------------------------------------
//
// --------------------------------------------------------------------------

#endif // __PICOOS_CONFIG_H__
