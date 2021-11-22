// --------------------------------------------------------------------------
//
// Project       DataLogger
//
// File          Config.cpp
//
// Author        Axel Werner
//
// --------------------------------------------------------------------------
// Changelog
//
//
// 2020-06-03  AWe   initial version
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

//     x    setFileName
//     x    setFileType
//     x    setFileSize
//     x    setCaptureSource
//     x    setSamplingRate
//     x    setSerialSamplingRate
//     x    setI2cSamplingRate
//          setStartSample
//          setStopSample
//     x    setSerialBaudrate
//     x    setSerialBits
//     x    setSerialParity
//     x    setSerialStopBits
//          setSystemTime


// --------------------------------------------------------------------------
// debug support
// --------------------------------------------------------------------------

#define LOG_LOCAL_LEVEL    LOG_INFO
#include "aweLog.h"
static const char TAG[] PROGMEM = tag( "Config" );

// --------------------------------------------------------------------------
//
// --------------------------------------------------------------------------

#ifdef ARDUINO
   #include <Arduino.h>             // pinMode(), digitalRead(), INPUT_PULLUP, millis(), ...
   #include "picoOS/picoOS.h"       // yield()
#else
   #include "WArduino.h"
#endif

#include "Config.h"
#include "Scanner.h"

#include "SdFat/SdFat.h"       // SdVolume

// --------------------------------------------------------------------------
//
// --------------------------------------------------------------------------

uint8_t getParameter( void );
uint8_t setParameter( uint8_t param_id, uint8_t param_type );

void setFileName( void );
void setFileType( void );
void setFileSize( void );
void setCaptureSource( void );
void setSamplingRate( void );
void setSerialSamplingRate( void );
void setI2cSamplingRate( void );
void setStartSample( void );
void setStopSample( void );
void setSerialBaudrate( void );
void setSerialBits( void );
void setSerialParity( void );
void setSerialStopBits( void );
void setSystemTime( void );

void DUMP_TOKEN( void );

// --------------------------------------------------------------------------
//
// --------------------------------------------------------------------------

const char header[] PROGMEM = "# DataLoger configuration file";
const char out_comment[] PROGMEM = "# ";

Settings settings =
{
   "config.txt"         // ConfigFileName
};

// --------------------------------------------------------------------------
// the parameter list
// --------------------------------------------------------------------------

#define str_param_0     "FileName"
#define str_param_1     "FileType"
#define str_param_2     "FileSize"
#define str_param_3     "CaptureSource"
#define str_param_4     "SamplingRate"
#define str_param_5     "SerialSamplingRate"
#define str_param_6     "I2cSamplingRate"
#define str_param_7     "StartSample"
#define str_param_8     "StopSample"
#define str_param_9     "SerialBaudrate"
#define str_param_10    "SerialBits"
#define str_param_11    "SerialParity"
#define str_param_12    "SerialStopBits"
#define str_param_13    "SystemTime"

const char param_name_0[]  PROGMEM = str_param_0;
const char param_name_1[]  PROGMEM = str_param_1;
const char param_name_2[]  PROGMEM = str_param_2;
const char param_name_3[]  PROGMEM = str_param_3;
const char param_name_4[]  PROGMEM = str_param_4;
const char param_name_5[]  PROGMEM = str_param_5;
const char param_name_6[]  PROGMEM = str_param_6;
const char param_name_7[]  PROGMEM = str_param_7;
const char param_name_8[]  PROGMEM = str_param_8;
const char param_name_9[]  PROGMEM = str_param_9;
const char param_name_10[] PROGMEM = str_param_10;
const char param_name_11[] PROGMEM = str_param_11;
const char param_name_12[] PROGMEM = str_param_12;
const char param_name_13[] PROGMEM = str_param_13;

//                               name           len                     id                  type                                 Example
const Param_t param_0  PROGMEM = { param_name_0,  strlen( str_param_0  ), FileName,           Text   };   // FileName            "capture.txt"
const Param_t param_1  PROGMEM = { param_name_1,  strlen( str_param_1  ), FileType,           Symbol };   // FileType            BIN, TXT
const Param_t param_2  PROGMEM = { param_name_2,  strlen( str_param_2  ), FileSize,           Number };   // FileSize            32GB, 1024MB, 4KB, 4096
const Param_t param_3  PROGMEM = { param_name_3,  strlen( str_param_3  ), CaptureSource,      Symbol };   // CaptureSource       SIO, ADC0
const Param_t param_4  PROGMEM = { param_name_4,  strlen( str_param_4  ), SamplingRate,       Number };   // SamplingRate        MAX, 100us, 1ms, 1s, 1min, 1h, 6h
const Param_t param_5  PROGMEM = { param_name_5,  strlen( str_param_5  ), SerialSamplingRate, Number };   // SerialSamplingRate  MAX, 100us, 1ms, 1s, 1min, 1h, 6h
const Param_t param_6  PROGMEM = { param_name_6,  strlen( str_param_6  ), I2cSamplingRate,    Number };   // I2cSamplingRate     MAX, 100us, 1ms, 1s, 1min, 1h, 6h
const Param_t param_7  PROGMEM = { param_name_7,  strlen( str_param_7  ), StartSample,        Text   };   // StartSample         "pattern"
const Param_t param_8  PROGMEM = { param_name_8,  strlen( str_param_8  ), StopSample,         Text   };   // StopSample          Y, N
const Param_t param_9  PROGMEM = { param_name_9,  strlen( str_param_9  ), SerialBaudrate,     Number };   // SerialBaudrate      115200, 9600, ...
const Param_t param_10 PROGMEM = { param_name_10, strlen( str_param_10 ), SerialBits,         Number };   // SerialBits          5,6,7,8
const Param_t param_11 PROGMEM = { param_name_11, strlen( str_param_11 ), SerialParity,       Number };   // SerialParity        N, E, O
const Param_t param_12 PROGMEM = { param_name_12, strlen( str_param_12 ), SerialStopBits,     Number };   // SerialStopBits      0, 1, 2
const Param_t param_13 PROGMEM = { param_name_13, strlen( str_param_13 ), SystemTime,         Number };   // SystemTime          "2020-02-08 11:15:32"

const Param_t* const PROGMEM params[] PROGMEM =
{
   &param_0,
   &param_1,
   &param_2,
   &param_3,
   &param_4,
   &param_5,
   &param_6,
   &param_7,
   &param_8,
   &param_9,
   &param_10,
   &param_11
};

const uint8_t num_params = sizeof( params ) / sizeof( Param_t* );

// --------------------------------------------------------------------------
//
// --------------------------------------------------------------------------

// open config file, if not exists create an empty config file
// note that only one file can be open at a time,
// so you have to close this one before opening another.

bool getConfiguration( void )
{

   // Open a file in the current working directory.
   // see .\src\SdFat\FatLib\FatFile.h(573)

   SdFile configFile;
   LOGI( TAG, "alloc %d byte @ 0x%04x", sizeof( configFile ), &configFile );

   LOGI( TAG, "configFile.open: <%s>", settings.ConfigFileName );
   if( configFile.open( settings.ConfigFileName, O_READ ) )
   {
      if( startScanner( &configFile ) )
      {
         while( !endOfFile )
         {
            getParameter();
         }
         finishScanner();
      }
      else
      {
         // cannot allocate memory for scanner
         // !!! todo: need some actions here
      }

      LOGD( TAG, "configFile.close" );
      configFile.close();
      return true;
   }
   else
   {
      // cannot find or open a config file
      LOGE( TAG, "Cannot open config file: <%s>", settings.ConfigFileName );
      LOGI( TAG, "Create config file: <%s>", settings.ConfigFileName );

      // so create one and fill it with the default configuration,
      LOGI( TAG, "configFile.open: <%s>", settings.ConfigFileName );
      if( configFile.open( settings.ConfigFileName, ( O_READ | O_WRITE | O_CREAT ) ) )
      {
         configFile.println( header );
         // write default/ current configuration to the file

         for( uint8_t i = 0; i < num_params; i++ )
         {
            configFile.print( out_comment );
            const Param_t* param = ( const Param_t * )pgm_read_ptr( &params[ i ] );
            const char *param_name = pgm_read_ptr( &param->name );
            configFile.println( param_name );
         }

         LOGD( TAG, "configFile.close" );
         configFile.close();
         return true;
      }
      else
      {
         LOGE( TAG, "error opening config file: <%s>", settings.ConfigFileName );
      }
   }
   return false;
}

// --------------------------------------------------------------------------
//
// --------------------------------------------------------------------------

// <name> <parameter><__EOL>

uint8_t getParameter( void )
{
   getToken();
   if( tokenType == __EOL )
      return false;

   DUMP_TOKEN();
   if( tokenType == IDENT )
   {
      // token, tokenLen, tokenType
      for( uint8_t i = 0; i < num_params; i++ )
      {
         const Param_t* param = ( const Param_t * )pgm_read_ptr( &params[ i ] );

         uint8_t param_nameLen = pgm_read_byte( &param->length );
         if( param_nameLen == tokenLen )
         {
            const char *param_name = pgm_read_ptr( &param->name );
            if( strncasecmp_P( token, param_name, tokenLen ) == 0 )
            {
               uint8_t param_id = pgm_read_byte( &param->id );
               uint8_t param_type = pgm_read_byte( &param->type );
               LOGD( TAG, "Param_t %d, id %d param_type %d", i, param_id, param_type );

               // get parameter value
               getToken();
               if( tokenType == __EOL )
                  return false;
               DUMP_TOKEN();

               setParameter( param_id, param_type );
               return true;
            }
         }
      }
      // token not found, ignore line
      LOGE( TAG, "token not found, ignore line" );
   }
   skipLine();
   return false;
}

// --------------------------------------------------------------------------
//
// --------------------------------------------------------------------------

// token points to the first value of the paramters

uint8_t setParameter( uint8_t param_id, uint8_t param_type )
{
   switch( param_id )
   {
      case FileName:             setFileName();             break;
      case FileType:             setFileType();             break;
      case FileSize:             setFileSize();             break;
      case CaptureSource:        setCaptureSource();        break;
      case SamplingRate:         setSamplingRate();         break;
      case SerialSamplingRate:   setSerialSamplingRate();   break;
      case I2cSamplingRate:      setI2cSamplingRate();      break;
      case StartSample:          setStartSample();          break;
      case StopSample:           setStopSample();           break;
      case SerialBaudrate:       setSerialBaudrate();       break;
      case SerialBits:           setSerialBits();           break;
      case SerialParity:         setSerialParity();         break;
      case SerialStopBits:       setSerialStopBits();       break;
      case SystemTime:           setSystemTime();           break;
   }
}

// --------------------------------------------------------------------------
//
// --------------------------------------------------------------------------

void dump_token( void )
{
   char tmp = *( token + tokenLen );
   *( token + tokenLen ) = 0;
   LOG( TAG, "got TOKEN %d, %s 0x%02x", tokenLen, token, *token );
   *( token + tokenLen ) = tmp;
}

void DUMP_TOKEN( void )
{
#if LOG_LOCAL_LEVEL >= LOG_DEBUG
   dump_token();
#endif
}

// --------------------------------------------------------------------------
//
// --------------------------------------------------------------------------

void setFileName( void )
{
   // "filename.ext["]

   char *filename = token;
   uint8_t len = tokenLen;

   if( *filename == '"' )
   {
      filename++;
      len--;
   }
   len--;
   if( filename[ len ] != '"' )
      len++;

   if( len >= sizeof( settings.FileName ) )
      len = sizeof( settings.FileName ) - 1;

   strncpy( settings.FileName, filename, len );
   settings.FileName[ len ] = '\0';
   // SdFile::make83Name( const char* str, uint8_t* name )

   LOGI( TAG, "settings.FileName: <%s>", settings.FileName );
}

// --------------------------------------------------------------------------
//
// --------------------------------------------------------------------------

void setFileType( void )
{
   if( strncasecmp_P( token, PSTR( "txt" ), 3 ) == 0 )
      settings.FileType = 1; // txt
   else
      settings.FileType = 0; // bin

   LOGI( TAG, "settings.FileType: <%s>", settings.FileType ? "TXT" : "BIN" );
}

// --------------------------------------------------------------------------
//
// --------------------------------------------------------------------------

void setFileSize( void )
{
   uint32_t value = ( uint32_t )( -1 );
   uint32_t file_size = ( uint32_t )( -1 );

   DUMP_TOKEN();
   // get the number
   if( tokenType = NUMBER )
   {
      value = atol( token );

      // get the unit of measurement
      // (none), G, M, K
      getToken();
      if( tokenType = IDENT && tokenLen == 1 )
      {
         if( *token == 'G' )
         {
            file_size = value * 1024UL * 1024UL * 1024UL;
         }
         else if( *token == 'M' )
         {
            file_size = value * 1024UL * 1024UL;
         }
         else if( *token == 'K' )
         {
            file_size = value * 1024UL;
         }
      }
      else
      {
         file_size = value;
      }

      settings.FileSize = file_size;
   }

   LOGI( TAG, "settings.FileSize %d", settings.FileSize );
}

// --------------------------------------------------------------------------
//
// --------------------------------------------------------------------------

// 15 14 13 12 11 10  9  8   7   6  5  4  3  2  1  0
// D7 D6 D5 D4 D3 D2 D1 D0 I2C SIO A5 A4 A3 A2 A1 A0

void setCaptureSource( void )
{
   Source_t source = { 0 };

   do
   {
      DUMP_TOKEN();

      if( tokenLen == 3 && strncasecmp_P( token, PSTR( "SIO" ), 3 ) == 0 )
      {
         source.sio = 1;
      }
      else if( tokenLen == 3 && strncasecmp_P( token, PSTR( "I2C" ), 3 ) == 0 )
      {
         // I2C use pins A5 and A4
         source.i2c = 1;
      }
      else if( tokenLen == 2 && ( *token == 'A' || *token == 'a' ) )
      {
         uint8_t index = token[1] - '0';
         if( index >= 0 && index <= 5 )
         {
            source.analog |= ( 1 << index );
         }
         else
         {
            LOGE( TAG, "index out of range" );
         }
      }
      else if( tokenLen == 2 && ( *token == 'D' || *token == 'd' ) )
      {
         uint8_t index = token[1] - '0';
         if( index >= 0 && index <= 7 )
         {
            source.digital |= ( 1 << index );
         }
         else
         {
            LOGE( TAG, "index out of range %c%c", token[0], token[1] );
         }
      }
      else
      {
         LOGE( TAG, "illegal source name" );
      }

      // get next parameter value
      getToken();
      // skip comma, only needed for better reading
      if( *token == ',' )
      {
         getToken();
      }
   }
   while( tokenType != __EOL );

   // check pin conflicts, prefer analog sources
   if( source.digital ^ ~source.analog )
   {
      // disable digital pins which are also used as analoge input
      source.digital &= ~source.analog;
   }

   // when I2C is selected, deselect the pins A5 and A4
   if( source.i2c )
   {
      source.analog  &= ~0x30;
      source.digital &= ~0x30;
   }

   settings.CaptureSource.val = source.val;

   LOGI( TAG, "settings.CaptureSource 0x%04x", settings.CaptureSource.val );
}

// --------------------------------------------------------------------------
//
// --------------------------------------------------------------------------

bool get_sampling_rate( uint32_t *sample_rate_ms )
{
   *sample_rate_ms = ( uint32_t )( -1 );
   bool rc = false;

   // get the number
   if( tokenType = NUMBER )
   {
      uint32_t value = atol( token );

      // get the unit of measurement
      // ms, s, min, h, d, Hz, mHz
      getToken();
      if( tokenType = IDENT )
      {
         if( tokenLen == 1 )
         {
            if( strncasecmp_P( token, PSTR( "s" ), 1 ) == 0 )
            {
               *sample_rate_ms = value * 1000;
            }
            else if( strncasecmp_P( token, PSTR( "h" ), 1 ) == 0 )
            {
               *sample_rate_ms = value * 1000 * 60 * 60;
            }
            else if( strncasecmp_P( token, PSTR( "d" ), 1 ) == 0 )
            {
               *sample_rate_ms = value * 1000 * 60 * 60 * 24;
            }
         }
         else if( tokenLen == 2 )
         {
            if( strncasecmp_P( token, PSTR( "ms" ), 2 ) == 0 )
            {
               *sample_rate_ms = value;
            }
            else if( strncasecmp_P( token, PSTR( "Hz" ), 2 ) == 0 )
            {
               *sample_rate_ms = ( ( 1000UL * 1000UL ) / value ) / 1000;
            }
         }
         else if( tokenLen == 3 )
         {
            if( strncasecmp_P( token, PSTR( "min" ), 3 ) == 0 )
            {
               *sample_rate_ms = value * 1000 * 60;
            }
            else if( strncasecmp_P( token, PSTR( "mHz" ), 3 ) == 0 )
            {
               *sample_rate_ms = ( ( 1000UL * 1000UL * 1000UL ) / value ) / 1000;
            }
         }
      }

      if( *sample_rate_ms != ( uint32_t )( -1 ) )
         rc = true;
   }
   else if( tokenType == IDENT && tokenLen == 3 && strncasecmp_P( token, PSTR( "MAX" ), 3 ) == 0 )
   {
      *sample_rate_ms = 0;
      rc = true;
   }
   else
   {
      // not a valid setting
   }

   return rc;
}

// examples for sample rates
// 50ms, 1 h, 3600 s, 50Hz. MAX

void setSamplingRate( void )
{
   uint32_t sample_rate_ms;

   if( get_sampling_rate( &sample_rate_ms ) )
      settings.SamplingRate = sample_rate_ms;

   LOGI( TAG, "settings.SamplingRate %ld", settings.SamplingRate );
}

void setSerialSamplingRate( void )
{
   uint32_t sample_rate_ms;

   if( get_sampling_rate( &sample_rate_ms ) )
      settings.SerialSamplingRate = sample_rate_ms;

   LOGI( TAG, "settings.SerialSamplingRate %ld", settings.SerialSamplingRate );
}

void setI2cSamplingRate( void )
{
   uint32_t sample_rate_ms;

   if( get_sampling_rate( &sample_rate_ms ) )
      settings.I2cSamplingRate = sample_rate_ms;

   LOGI( TAG, "settings.I2cSamplingRate %ld", settings.I2cSamplingRate );
}

// --------------------------------------------------------------------------
//
// --------------------------------------------------------------------------

void setStartSample( void )
{
   LOGI( TAG, "settings.StartSample: <%s>", settings.StartSample );
}

// --------------------------------------------------------------------------
//
// --------------------------------------------------------------------------

void setStopSample( void )
{
   LOGI( TAG, "settings.StopSample: <%s>", settings.StopSample );
}

// --------------------------------------------------------------------------
//
// --------------------------------------------------------------------------

void setSerialBaudrate( void )
{
   // get the number
   if( tokenType = NUMBER )
   {
      settings.SerialBaudrate = atol( token );
   }

   LOGI( TAG, "settings.SerialBaudrate %d", settings.SerialBaudrate );
}

// --------------------------------------------------------------------------
//
// --------------------------------------------------------------------------

void setSerialBits( void )
{
   // get the number
   if( tokenType = NUMBER )
   {
      uint32_t serial_bits = atol( token );
      if( serial_bits <= 8 )
         settings.SerialBits = serial_bits;
   }

   LOGI( TAG, "settings.SerialBits %d", settings.SerialBits );
}

// --------------------------------------------------------------------------
//
// --------------------------------------------------------------------------

void setSerialParity( void )
{
   if( tokenType = IDENT && tokenLen == 1 )
   {
      if( *token == 'N' )
      {
         settings.SerialParity = 0; // none
      }
      else if( *token == 'O' )
      {
         settings.SerialParity = 1; // odd
      }
      else if( *token == 'E' )
      {
         settings.SerialParity = 2; // even
      }
   }

   LOGI( TAG, "settings.SerialParity %d", settings.SerialParity );
}

// --------------------------------------------------------------------------
//
// --------------------------------------------------------------------------

void setSerialStopBits( void )
{
   // get the number
   if( tokenType = NUMBER )
   {
      uint32_t stop_bits = atol( token );

      if( stop_bits <= 2 )
         settings.SerialStopBits = stop_bits;
   }

   LOGI( TAG, "settings.SerialStopBits %d", settings.SerialStopBits );
}

// --------------------------------------------------------------------------
//
// --------------------------------------------------------------------------

void setSystemTime( void )
{
   LOGI( TAG, "settings.SystemTime %d", settings.SystemTime );
}

// --------------------------------------------------------------------------
//
// --------------------------------------------------------------------------


