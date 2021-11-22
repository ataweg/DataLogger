// --------------------------------------------------------------------------
//
// Project       - generic -
//
// File          Axel Werner
//
// Author        aweLog.h
//
// --------------------------------------------------------------------------
// Changelog
//
// 2019-08-08  AWe   add #include "aweLog_config.h"
//                     move #define PRINTF_BUFFER_SIZE to there
// 2019-02-01  AWe   redesign aweLog
//                     remove color support
// 2019-01-07  AWe   from ESP projects redesigned for use with AVR chips
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

// from http://logging.apache.org/log4j/1.2/apidocs/org/apache/log4j/Level.html
//
//  +------------+-----------------------------------------------------------+
//  |  ALL       | The ALL has the lowest possible rank and is intended to   |
//  |            | turn on all logging.                                      |
//  +------------+-----------------------------------------------------------+
//  |  DEBUG     | The DEBUG Level designates fine-grained informational     |
//  |            | events that are most useful to debug an application.      |
//  +------------+-----------------------------------------------------------+
//  |  ERROR     | The ERROR level designates error events that might still  |
//  |            | allow the application to continue running.                |
//  +------------+-----------------------------------------------------------+
//  |  FATAL     | The FATAL level designates very severe error events that  |
//  |            | will presumably lead the application to abort.            |
//  +------------+-----------------------------------------------------------+
//  |  INFO      | The INFO level designates informational messages that     |
//  |            | highlight the progress of the application at coarse-      |
//  |            | grained level.                                            |
//  +------------+-----------------------------------------------------------+
//  |  OFF       | The OFF has the highest possible rank and is intended to  |
//  |            | turn off logging.                                         |
//  +------------+-----------------------------------------------------------+
//  |  TRACE     | The TRACE Level designates finer-grained informational    |
//  |            | events than the DEBUG                                     |
//  +------------+-----------------------------------------------------------+
//  |  TRACE_INT | TRACE level integer value.                                |
//  +------------+-----------------------------------------------------------+
//  |  WARN      | The WARN level designates potentially harmful situations. |
//  +------------+-----------------------------------------------------------+


#ifndef __AWELOG_H__
#define __AWELOG_H__

// --------------------------------------------------------------------------
//
// --------------------------------------------------------------------------

#include "aweLog_config.h"

#include "WString.h"          // __FlashStringHelper

// --------------------------------------------------------------------------
//
// --------------------------------------------------------------------------

#ifndef CONFIG_LOG_DEFAULT_LEVEL
   #define CONFIG_LOG_DEFAULT_LEVEL    LOG_NONE
#endif

#ifndef LOG_LOCAL_LEVEL
   #define LOG_LOCAL_LEVEL  CONFIG_LOG_DEFAULT_LEVEL
#endif

#define S( str ) ( str == NULL ? "<null>": str )

// --------------------------------------------------------------------------
//
// --------------------------------------------------------------------------

/**
 * @brief Log level
 *
 */
#define LOG_NONE         0   /*!< No log output */
#define LOG_ERROR        1   /*!< Critical errors, software module can not recover on its own */
#define LOG_WARN         2   /*!< Error conditions from which recovery measures have been taken */
#define LOG_INFO         3   /*!< Information messages which describe normal flow of events */
#define LOG_DEBUG        4   /*!< Extra information which is not necessary for normal use ( values, pointers, sizes, etc ). */
#define LOG_VERBOSE      5   /*!< Bigger chunks of debugging information, or frequent messages which can potentially flood the output. */
#define LOG_ALWAYS      -1   // print log meassge independly from LOG_LEVEL

// --------------------------------------------------------------------------
//
// --------------------------------------------------------------------------

#ifdef __cplusplus
extern "C" {
#endif

int freeMemory( void );

void _log_write( char log_level, const char *tag, uint32_t log_time, uint16_t line, const __FlashStringHelper *format, ... );

uint16_t _log_time2str( char *buffer, uint16_t buffer_size, const int32_t time );
uint32_t _log_timestamp( void );

#ifdef __cplusplus
}
#endif


#define tag( str )                "%c (%s) " str "(%d): "

#define LOG_FORMAT( format )     F( format )

#ifndef NDEBUG
   #define LOGE( tag_fmt, format, ... ) LOG_LEVEL_LOCAL( LOG_ERROR,   tag_fmt, format, ##__VA_ARGS__ )
   #define LOGW( tag_fmt, format, ... ) LOG_LEVEL_LOCAL( LOG_WARN,    tag_fmt, format, ##__VA_ARGS__ )
   #define LOGI( tag_fmt, format, ... ) LOG_LEVEL_LOCAL( LOG_INFO,    tag_fmt, format, ##__VA_ARGS__ )
   #define LOGD( tag_fmt, format, ... ) LOG_LEVEL_LOCAL( LOG_DEBUG,   tag_fmt, format, ##__VA_ARGS__ )
   #define LOGV( tag_fmt, format, ... ) LOG_LEVEL_LOCAL( LOG_VERBOSE, tag_fmt, format, ##__VA_ARGS__ )
#else
   #define LOGE( tag_fmt, format, ... )        {}
   #define LOGW( tag_fmt, format, ... )        {}
   #define LOGI( tag_fmt, format, ... )        {}
   #define LOGD( tag_fmt, format, ... )        {}
   #define LOGV( tag_fmt, format, ... )        {}
#endif

#define LOG_LEVEL( level, tag_fmt, format, ... ) do {                     \
     if( level==LOG_ERROR )          { _log_write( 'E', tag_fmt, _log_timestamp(), __LINE__, LOG_FORMAT( format ), ##__VA_ARGS__ ); } \
     else if( level==LOG_WARN )      { _log_write( 'W', tag_fmt, _log_timestamp(), __LINE__, LOG_FORMAT( format ), ##__VA_ARGS__ ); } \
     else if( level==LOG_INFO )      { _log_write( 'I', tag_fmt, _log_timestamp(), __LINE__, LOG_FORMAT( format ), ##__VA_ARGS__ ); } \
     else if( level==LOG_DEBUG )     { _log_write( 'D', tag_fmt, _log_timestamp(), __LINE__, LOG_FORMAT( format ), ##__VA_ARGS__ ); } \
     else if( level==LOG_VERBOSE )   { _log_write( 'V', tag_fmt, _log_timestamp(), __LINE__, LOG_FORMAT( format ), ##__VA_ARGS__ ); } \
     else                            { _log_write( '*', tag_fmt, _log_timestamp(), __LINE__, LOG_FORMAT( format ), ##__VA_ARGS__ ); } \
} while( 0 )

#define LOG( tag_fmt, format, ... )      { _log_write( ' ', tag_fmt, _log_timestamp(), __LINE__, LOG_FORMAT( format ), ##__VA_ARGS__ ); }


/** runtime macro to output logs at a specified level. Also check the level with ``LOG_LOCAL_LEVEL``.
 *
 * @see ``printf``, ``LOG_LEVEL``
 */
#define LOG_LEVEL_LOCAL( level, tag_fmt, format, ... ) do {               \
        if( LOG_LOCAL_LEVEL >= level ) LOG_LEVEL( level, tag_fmt, format, ##__VA_ARGS__ ); \
    } while( 0 )

// --------------------------------------------------------------------------
// debg helper
// --------------------------------------------------------------------------

#ifdef __cplusplus
extern "C"
{
#endif

int dump_data( const char* data, unsigned short len );
int dump_text( const char* data, unsigned short len );
int dump_data_hex( const char* data, unsigned short len );

#ifdef __cplusplus
}
#endif

// --------------------------------------------------------------------------
//
// --------------------------------------------------------------------------
#endif // __AWELOG_H__