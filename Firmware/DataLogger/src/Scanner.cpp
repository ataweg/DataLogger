// --------------------------------------------------------------------------
//
// Project       DataLogger
//
// File          Scanner.cpp
//
// Author        Axel Werner
//
// --------------------------------------------------------------------------
// Changelog
//
//
// 2020.06.04  AWe   takeover from
//                      E:\Axel\Projects\CAD-SW\cpu32\p32asm\p32SCAN.C
//                      E:\Axel\Projects\CAD-SW\p32asm\p32PARSE.C
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

/***************************************************************************\
 *
 *   Copyright ? 1995. Alle Rechte vorbehalten.
 *
 *  PROJECT:     DP32 Assembler
 *  SUBJECT:     Microcode Assembler
 *
 *  DESCRIPTION: scanner
 *
 *  FILE:        p32scan.c
 *
 *  AUTHOR:      Axel Werner
 *
 *  STATUS:      draft
 *
 *  CHANGES:
 *
 *   29.10.1994 0.1 AWe  first draft
 *   17.04.1996 1.0 AWe  modifed for use with dp32 assembler
 *
\***************************************************************************/


// --------------------------------------------------------------------------
// debug support
// --------------------------------------------------------------------------

#define LOG_LOCAL_LEVEL    LOG_NONE
#include "aweLog.h"
static const char TAG[] PROGMEM = tag( "Scanner" );

// --------------------------------------------------------------------------
//
// --------------------------------------------------------------------------

#ifdef ARDUINO
   #include <Arduino.h>             // pinMode(), digitalRead(), INPUT_PULLUP, millis(), ...
#else
   #include "WArduino.h"
#endif

#include "SdFat/SdFat.h"       // SdFile, read(), seekSet()
#include "Scanner.h"

// --------------------------------------------------------------------------
//
// --------------------------------------------------------------------------

SdFile *inFile     = NULL;
bool endOfFile;   // set if EOF found

char *lineBuffer = NULL;
char *lineBufferEnd;
char *lineStart;
char *linePos;

char *token;
int   tokenLen;
char  tokenType;

int lineNum;

// --------------------------------------------------------------------------
// Prototypes for local functions
// --------------------------------------------------------------------------

// --------------------------------------------------------------------------
//
// --------------------------------------------------------------------------

bool startScanner( SdFile *infile )
{
   uint16_t bytesRequest;
   uint16_t bytesGot;

   if( infile == NULL )
   {
      return false;
   }
   inFile = infile;

   if( lineBuffer == NULL )
   {
      if( ( lineBuffer = ( char* ) malloc( LINE_BUFFER_SIZE + 1 ) ) == NULL )
      {
         LOGE( TAG, "Cannot allocate %d byte for input buffer. Abort!\n", LINE_BUFFER_SIZE );
         return false;
      }
      LOGI( TAG, "alloc %d byte @ 0x%04x", LINE_BUFFER_SIZE + 1, lineBuffer );
   }

   lineBufferEnd = lineBuffer + LINE_BUFFER_SIZE;
   lineStart = linePos = lineBuffer;
   lineNum = 1;
   endOfFile = false;   // set if EOF found
   LOGD( TAG, "linePos %d, lineBufferEnd %d", linePos, lineBufferEnd );

   // fill up buffer
   bytesRequest = LINE_BUFFER_SIZE;
   bytesGot = inFile->read( lineBuffer, bytesRequest );
   lineBuffer[ bytesGot ] = 0;
   LOGI( TAG, "Read %d bytes from file. Need %d bytes", bytesGot, bytesRequest );
   // Serial.println( lineBuffer );
   if( bytesGot < bytesRequest )
   {
      lineBuffer[bytesGot] = EOF;
   }

   return true;
}

// --------------------------------------------------------------------------
//
// --------------------------------------------------------------------------

bool restartScanner( void )
{
   if( inFile == NULL )
      return false;

   inFile->seekSet( 0L );

   return startScanner( inFile );
}

// --------------------------------------------------------------------------
//
// --------------------------------------------------------------------------

bool finishScanner( void )
{
   if( lineBuffer )
   {
      free( lineBuffer );
      lineBuffer = NULL;
   }

   return true;
}

// --------------------------------------------------------------------------
//
// --------------------------------------------------------------------------

char *advance( int size )
{
   uint16_t bytesMoved;    // number of bytes moved to begin of line buffer
   uint16_t bytesRequest;   // number of bytes we can read to fill up the line buffer
   uint16_t bytesGot;
   uint16_t distance;      // number of bytes from buffer begin to line start
   char *readPos;

   if( ( linePos + size ) > ( lineBufferEnd - LOOK_AHEAD_SIZE ) ) // out of buffer
   {
      LOGD( TAG, "linePos %d, size %d, lineBufferEnd %d, LOOK_AHEAD_SIZE %d", linePos, size, lineBufferEnd, LOOK_AHEAD_SIZE );
      LOGD( TAG, "linePos + size %d, lineBufferEnd - LOOK_AHEAD_SIZE %d", linePos + size, lineBufferEnd - LOOK_AHEAD_SIZE );

      // move lineStart -> lineBuffer
      bytesMoved = ( uint16_t )( lineBufferEnd - lineStart );
      distance   = ( uint16_t )( lineStart - lineBuffer );
      LOGD( TAG, "lineBuffer %d, lineStart %d, bytesMoved %d, distance %d", lineBuffer, lineStart, bytesMoved, distance );
      memmove( lineBuffer, lineStart, bytesMoved );
      lineStart -= distance;
      linePos   -= distance;
      token     -= distance;
      LOGD( TAG, "lineStart %d, linePos %d token %d", lineStart, linePos, token );

      // fill up buffer
      bytesRequest = LINE_BUFFER_SIZE - bytesMoved;
      readPos = lineBufferEnd - bytesRequest;
      bytesGot = inFile->read( readPos, bytesRequest );
      lineBuffer[ bytesMoved + bytesGot ] = 0;
      LOGI( TAG, "Read %d bytes from file. Request %d bytes", bytesGot, bytesRequest );
      // Serial.println( lineBuffer );
      if( bytesGot < bytesRequest )
      {
         readPos[bytesGot] = EOF;
      }
   }

   linePos += size;
   return linePos;
}

// --------------------------------------------------------------------------
//
// --------------------------------------------------------------------------

char *skipBlanks( void )
{
   LOGD( TAG, "skipBlanks %d %c", linePos, *linePos );

   char ch;

   while( true )
   {
      ch = *linePos;    // is incremented by advance()
      switch( ch )
      {

         case '\r':     // 0x0d
         case '\t':     // 0x08
         case ' ' :
            advance( 1 );
            break;

         case '#' :           // start comment
            skipLine();       // linePos points to the end of the current line
            break;

         case '/' :           // start c style comment
         {
            // we have at least LOOK_AHEAD_SIZE byte for looking ahead
            ch = linePos[1];  // look one character ahead
            if( ch == '/' )   // cpp style line comment
            {
               skipLine();    // linePos points to the end of the current line
               break;
            }
            else if( ch == '*' )       // block comment
            {
               // comment_start = linePos;
               // we need here an other advance() funnction, which removes the
               // unused byte from the begin of the comment to its end
               advance( 2 );           // "/*"

               while( true )
               {
                  ch = *linePos;
                  switch( ch )
                  {
                     case EOF:
                        endOfFile = true;
                        goto commentEnd;

                     case '*':
                        ch = linePos[1];     // look one character ahead
                        if( ch == '/' )      // end of block comment
                        {
                           advance( 2 );     // point to first character after comment
                           goto commentEnd;
                        }
                        advance( 1 );
                        break;

                     case '\n':              // 0x0a in comment
                        advance( 1 );
                        lineWrap();
                        break;

                     default:
                        advance( 1 );

                  }  // end switch
               }  // end while
commentEnd:
               break;
            } // end else if
         }   // end case '/': comment

         case EOF:
            endOfFile = true;

         default:
            LOGD( TAG, "Linepos %d", linePos );
            return linePos;

      } // end switch
   } // end while
}

// --------------------------------------------------------------------------
//
// --------------------------------------------------------------------------

void lineWrap( void )
{
   lineNum++;
   lineStart = linePos;
   LOGD( TAG, "LineNum: %d, LineStart %d", lineNum, lineStart );
}

// --------------------------------------------------------------------------
//
// --------------------------------------------------------------------------

void skipLine( void )
{
   while( true )
   {
      char ch = *linePos;

      if( ch == '\n' || endOfFile )
         return;

      advance( 1 );
   }  // end while
}

// --------------------------------------------------------------------------
//
// --------------------------------------------------------------------------

// returns IDENT, NUMBER, HEXNUM, SYMBOL, __EOF

char getToken( void )
{
   LOGD( TAG, "getToken" );

   char  ch;

   // token points to the begin of the current token
   token = skipBlanks();     // skip white spaces and comments
   tokenLen = 0;
   tokenType = __EOF;

   if( endOfFile )
      return tokenType;

   ch = *linePos;
   if( ch == '\n' )     // 0x0a
   {
      LOGD( TAG, "found __EOL" );
      advance( 1 );
      tokenLen++;
      lineWrap();
      tokenType = __EOL;
   }
   else if( ch >= 'A' && ch <= 'Z' || ch >= 'a' && ch <= 'z' || ch == '_' || ch == '$' )
   {
      // legal identifier names start with a letter or $ or _
      LOGD( TAG, "found identifier" );
      tokenType = IDENT;

      while( true )
      {
         advance( 1 );
         tokenLen++;

         ch = *linePos;
         if( ch >= 'A' && ch <= 'Z' || ch >= 'a' && ch <= 'z' || ch >= '0' && ch <= '9' ||
               ch == '_' || ch == '$' )
         {
            continue;
         } // end if
         LOGD( TAG, "getToken %d: %d %d %d %s ", tokenLen, token, token + tokenLen, linePos, token );
         break;

      } // end while
   }
   else if( ch >= '0' && ch <= '9' )
   {
      // numbers start with a digit, also hex numbers
      LOGD( TAG, "found digit" );
      tokenType = NUMBER;

      while( true )
      {
         advance( 1 );
         tokenLen++;

         ch = *linePos;
         if( ch >= '0' && ch <= '9' )
         {
            continue;
         } // end if
         else if( ch >= 'A' && ch <= 'F' || ch >= 'a' && ch <= 'f' )
         {
            tokenType = HEXNUM;
            continue;
         }
         LOGD( TAG, "getToken Number %d: %d %s", tokenLen, token, token );
         break;
      } // end while
   }
   else if( ch == '"' )
   {
      // a string starts with '"'
      LOGD( TAG, "found string" );
      tokenType = STRING;

      while( true )
      {
         tokenLen++;
         advance( 1 );
         ch = *linePos;
         if( ch == '"' )
         {
            tokenLen++;
            advance( 1 );
            break;
         }
         else if( ch == '\n' || ch == EOF )
         {
            // missing '"' at the end of the string
            break;
         }
      }
   }
   else
   {
      tokenType = SYMBOL;  // token is not an identifier
      tokenLen++;
      advance( 1 );
      LOGD( TAG, "getToken Identifier %d: %d 0x%02x", tokenLen, token, *token );
   }

   LOGD( TAG, "return %d", tokenType );
   return tokenType;
}

// --------------------------------------------------------------------------
//
// --------------------------------------------------------------------------

// hex    dec   hex   hex    dec
// 0e7fH, 123d, 123h, 123dh, 123

bool getNumber( uint32_t *num )
{
   char ch;

   ch = token[ tokenLen ];
   if( ch == 'h' || ch == 'H' )
   {
      if( getHexNumber( num ) )
      {
         advance( 1 );
         return true;
      }
      goto error;
   }

   // since 'D' is a valid hex digit, its part of the token
   ch = token[ tokenLen - 1 ];
   if( ch == 'D' || ch == 'd' )
   {
      if( getDecimalNumber( num ) )
      {
         advance( 1 );
         return true;
      }
      goto error;
   }

   else if( getDecimalNumber( num ) )
   {
      // advance( tokenLen );
      return true;
   }

error:
   LOGE( TAG, "ERROR: number expected\n" );
   return false;
}

// --------------------------------------------------------------------------
//
// --------------------------------------------------------------------------

bool getHexNumber( uint32_t *num )
{
   LOGD( TAG, "getHexNumber" );

   char  ch;
   bool isNum = false;

   // token points to the begin of the current token
   token = skipBlanks();     // skip white spaces and comments
   tokenLen = 0;

   *num = 0L;

   while( true )
   {
      ch = *linePos;
      if( ch >= 'A' && ch <= 'F' || ch >= 'a' && ch <= 'f' ||  ch >= '0' && ch <= '9' )
      {
         advance( 1 );
         tokenLen++;

         ch = *linePos;
         *num = ( *num << 4 ) + ( ch >= 'a' ? ch - 'a' + 10 : ( ch >= 'A' ? ch - 'A' + 10 : ch - '0' ) );
         isNum = true;
         continue;
      } // end if

      return isNum;
   } // end while
}

// --------------------------------------------------------------------------
//
// --------------------------------------------------------------------------

// next token is a decimal number

bool getDecimalNumber( uint32_t *num )
{
   LOGD( TAG, "getDecimalNumber" );

   char ch;
   bool isNum = false;

   // token points to the begin of the current token
   token = skipBlanks();     // skip white spaces and comments
   tokenLen = 0;

   *num = 0L;

   while( true )
   {
      ch = *linePos;
      if( ch >= '0' && ch <= '9' )
      {
         advance( 1 );
         tokenLen++;

         ch = *linePos;
         *num = *num * 10  + ( ch - '0' );
         isNum = true;
         continue;
      } // end if

      return isNum;
   } // end while
}

// --------------------------------------------------------------------------
//
// --------------------------------------------------------------------------

#define upper(x)  ((x>='a' && x<='z') ? x-0x20 : x )

bool checkStr( char *s1, char *s2, uint16_t len )
{
   if( len )
   {
      while( len-- )
      {
         if( upper( *s2 ) != upper( *s1 ) ) return false;
         s1++;
         s2++;
      }
   }
   return true;
}

// --------------------------------------------------------------------------
//
// --------------------------------------------------------------------------
