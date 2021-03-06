// --------------------------------------------------------------------------
//
// 2019-07-26  AWe   update to latest commit 6fa2251 on 22 Dec 2018
// 2018-12-21  AWe   taken from https://github.com/tkem/mbino
//
// --------------------------------------------------------------------------

/* mbino - basic mbed APIs for the Arduino platform
 * Copyright (c) 2017, 2018 Thomas Kemmer
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you
 * may not use this file except in compliance with the License.  You
 * may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
 * implied.  See the License for the specific language governing
 * permissions and limitations under the License.
 */
#ifndef MBINO_AVR_PIN_NAMES_H
#define MBINO_AVR_PIN_NAMES_H

#include <pins_arduino.h>

// assume at least 6 digital pins
#ifndef NUM_DIGITAL_PINS
#define NUM_DIGITAL_PINS 6
#endif

typedef enum PinDirection : uint8_t
{
   PIN_INPUT,
   PIN_OUTPUT
} PinDirection;

typedef enum PinMode : uint8_t
{
   PullNone,
   PullUp,
   PullDefault = PullNone
} PinMode;

// *not* an enum type so predefined A0..An can be used as well
typedef uint8_t PinName;

static const PinName NC = -1;

// assume min. 6 digital pins
static const PinName D0 = 0;
static const PinName D1 = 1;
static const PinName D2 = 2;
static const PinName D3 = 3;
static const PinName D4 = 4;
static const PinName D5 = 5;
#if NUM_DIGITAL_PINS > 6  /* standard, leonardo, mega, ethernet */
static const PinName D6 = 6;
static const PinName D7 = 7;
static const PinName D8 = 8;
static const PinName D9 = 9;
static const PinName D10 = 10;
static const PinName D11 = 11;
static const PinName D12 = 12;
static const PinName D13 = 13;
static const PinName D14 = 14;
static const PinName D15 = 15;
static const PinName D16 = 16;
static const PinName D17 = 17;
static const PinName D18 = 18;
static const PinName D19 = 19;
#endif
#if NUM_DIGITAL_PINS > 20  /* leonardo, mega */
static const PinName D20 = 20;
static const PinName D21 = 21;
static const PinName D22 = 22;
static const PinName D23 = 23;
static const PinName D24 = 24;
static const PinName D25 = 25;
static const PinName D26 = 26;
static const PinName D27 = 27;
static const PinName D28 = 28;
static const PinName D29 = 29;
static const PinName D30 = 30;
#endif
#if NUM_DIGITAL_PINS > 31  /* mega */
static const PinName D31 = 31;
static const PinName D32 = 32;
static const PinName D33 = 33;
static const PinName D34 = 34;
static const PinName D35 = 35;
static const PinName D36 = 36;
static const PinName D37 = 37;
static const PinName D38 = 38;
static const PinName D39 = 39;
static const PinName D40 = 40;
static const PinName D41 = 41;
static const PinName D42 = 42;
static const PinName D43 = 43;
static const PinName D44 = 44;
static const PinName D45 = 45;
static const PinName D46 = 46;
static const PinName D47 = 47;
static const PinName D48 = 48;
static const PinName D49 = 49;
static const PinName D50 = 50;
static const PinName D51 = 51;
static const PinName D52 = 52;
static const PinName D53 = 53;
static const PinName D54 = 54;
static const PinName D55 = 55;
static const PinName D56 = 56;
static const PinName D57 = 57;
static const PinName D58 = 58;
static const PinName D59 = 59;
static const PinName D60 = 60;
static const PinName D61 = 61;
static const PinName D62 = 62;
static const PinName D63 = 63;
static const PinName D64 = 64;
static const PinName D65 = 65;
static const PinName D66 = 66;
static const PinName D67 = 67;
static const PinName D68 = 68;
static const PinName D69 = 69;
#endif
#if NUM_DIGITAL_PINS > 70
#warning There are more than 70 digital pins on your board. Some pin names may not be defined.
#endif

#ifdef LED_BUILTIN
static const PinName LED1 = LED_BUILTIN;
#ifdef LED_BUILTIN_RX
static const PinName LED2 = LED_BUILTIN_RX;
#else
static const PinName LED2 = LED_BUILTIN;
#endif
#ifdef LED_BUILTIN_TX
static const PinName LED3 = LED_BUILTIN_TX;
#else
static const PinName LED3 = LED_BUILTIN;
#endif
static const PinName LED4 = LED_BUILTIN;
#endif

#ifdef PIN_WIRE_SCL
static const PinName I2C_SCL = PIN_WIRE_SCL;
#endif
#ifdef PIN_WIRE_SDA
static const PinName I2C_SDA = PIN_WIRE_SDA;
#endif

#ifdef PIN_SPI_MISO
static const PinName SPI_MOSI = PIN_SPI_MOSI;
#endif
#ifdef PIN_SPI_MOSI
static const PinName SPI_MISO = PIN_SPI_MISO;
#endif
#ifdef PIN_SPI_SCK
static const PinName SPI_SCK = PIN_SPI_SCK;
#endif
#ifdef PIN_SPI_SS
static const PinName SPI_SS = PIN_SPI_SS;
#endif

// see https://www.arduino.cc/en/Reference/Serial
#ifdef SERIAL_PORT_HARDWARE
// on Leonardo et al. there is no USART0
#ifdef SERIAL_PORT_USBVIRTUAL
static const PinName UART1_TX = 1;
static const PinName UART1_RX = 0;
#define UART_TX UART1_TX
#define UART_RX UART1_RX
#else
static const PinName UART0_TX = 1;
static const PinName UART0_RX = 0;
#define UART_TX UART0_TX
#define UART_RX UART0_RX
#endif
// commonly used alternative names
#define SERIAL_TX UART_TX
#define SERIAL_RX UART_RX
#define STDIO_UART_TX UART_TX
#define STDIO_UART_RX UART_RX
// on Leonardo et al. there are no "USB serial pins"
#ifndef SERIAL_PORT_USBVIRTUAL
#define USBTX UART_TX
#define USBRX UART_RX
#endif
#endif

#ifdef SERIAL_PORT_HARDWARE1
static const PinName UART1_TX = 18;
static const PinName UART1_RX = 19;
#endif

#ifdef SERIAL_PORT_HARDWARE2
static const PinName UART2_TX = 16;
static const PinName UART2_RX = 17;
#endif

#ifdef SERIAL_PORT_HARDWARE3
static const PinName UART3_TX = 14;
static const PinName UART3_RX = 15;
#endif

#endif
