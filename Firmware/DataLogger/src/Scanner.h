// --------------------------------------------------------------------------
//
// 2020-06-04  AWe   initial version
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

#ifndef __SCANNER_H__
#define __SCANNER_H__

#include "SdFat/SdFat.h"       // SdFile

// --------------------------------------------------------------------------
//
// --------------------------------------------------------------------------

#define LINE_BUFFER_SIZE  128
#define LOOK_AHEAD_SIZE    16

#define IDENT        (  1 )
#define NUMBER       (  2 )
#define HEXNUM       (  3 )
#define STRING       (  4 )
#define SYMBOL       (  5 )
#define __EOL        ( -2 )
#define __EOF        ( -1 )

// --------------------------------------------------------------------------
//
// --------------------------------------------------------------------------

extern bool endOfFile;   // set if EOF found
extern char *token;
extern int   tokenLen;
extern char  tokenType;

extern int lineNum;

// --------------------------------------------------------------------------
// Prototypes for functions
// --------------------------------------------------------------------------

bool startScanner( SdFile *infile );
bool finishScanner( void );
bool restartScanner( void );

char* skipBlanks( void );
char* advance( int size );
void lineWrap( void );
void lineWrap( void );
void skipLine( void );

char getToken( void );
bool getNumber( uint32_t *num );
bool getDecimalNumber( uint32_t *num );
bool getHexNumber( uint32_t *num );
bool checkStr( char *s1, char *s2, uint16_t len );

// --------------------------------------------------------------------------
//
// --------------------------------------------------------------------------
#endif // __SCANNER_H__
