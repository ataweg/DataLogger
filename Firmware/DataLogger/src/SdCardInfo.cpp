// --------------------------------------------------------------------------
//
// Project       HW_rev3A1
//
// File          SdCardTask.cpp
//
// Author        Axel Werner
//
// --------------------------------------------------------------------------
// Changelog
//
//
// 2020-06-16  AWe   fix issue with dumpDirectory()
// 2020-06-01  AWe   initial version
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
static const char TAG[] PROGMEM = tag( "SdCardInfo" );

// --------------------------------------------------------------------------
//
// --------------------------------------------------------------------------

#ifdef ARDUINO
   #include <Arduino.h>             // pinMode(), digitalRead(), INPUT_PULLUP, millis(), ...
   #include "picoOS/picoOS.h"       // yield()
#else
   #include "WArduino.h"
#endif

#include "SdCardInfo.h"

// --------------------------------------------------------------------------
//
// --------------------------------------------------------------------------

uint32_t cardSize;         // global for card size
uint32_t eraseSize;        // global for card erase size

uint8_t dumpCID( void );
uint8_t dumpCSD( void );
uint8_t dumpMBR( void );
void dumpVolume( void );

extern SdFat sd;

// --------------------------------------------------------------------------
// store error strings in flash
// --------------------------------------------------------------------------

#define sdErrorMsg( msg ) sd.errorPrint( msg )

// --------------------------------------------------------------------------
//
// --------------------------------------------------------------------------

uint8_t dumpCID( void )
{
   cid_t cid;
   if( !sd.card()->readCID( &cid ) )
   {
      sdErrorMsg( F( "readCID failed" ) );
      return false;
   }
   Serial.print( F( "\nManufacturer ID:" ) );
   Serial.println( cid.mid, HEX );

   Serial.print( F( "OEM ID: " ) );
   Serial.print( cid.oid[0] );
   Serial.println( cid.oid[1] );

   Serial.print( F( "Product: " ) );
   for( uint8_t i = 0; i < 5; i++ )
   {
      Serial.print( cid.pnm[i] );
   }
   Serial.println();

   Serial.print( F( "\nVersion: " ) );
   Serial.print( cid.prv_n );
   Serial.print( '.' );
   Serial.println( cid.prv_m );

   Serial.print( F( "Serial number: " ) );
   Serial.println( cid.psn, HEX );

   Serial.print( F( "Manufacturing date: " ) );
   Serial.print( cid.mdt_month );
   Serial.print( '/' );
   Serial.println( 2000 + cid.mdt_year_low + 10 * cid.mdt_year_high );

   return true;
}

// --------------------------------------------------------------------------
//
// --------------------------------------------------------------------------

uint8_t dumpCSD( void )
{
   csd_t csd;
   bool eraseSingleBlock;

   if( !sd.card()->readCSD( &csd ) )
   {
      sdErrorMsg( F( "readCSD failed" ) );
      return false;
   }

   if( csd.v1.csd_ver == 0 )
   {
      eraseSingleBlock = csd.v1.erase_blk_en;
      eraseSize = ( csd.v1.sector_size_high << 1 ) | csd.v1.sector_size_low;
   }
   else if( csd.v2.csd_ver == 1 )
   {
      eraseSingleBlock = csd.v2.erase_blk_en;
      eraseSize = ( csd.v2.sector_size_high << 1 ) | csd.v2.sector_size_low;
   }
   else
   {
      Serial.println( F( "csd version error" ) );
      return false;
   }

   eraseSize++;
   // LOGD( TAG, "cardSize %ld sectors", cardSize );
   Serial.print( F( "cardSize: " ) );
   // Serial.print( 0.000512 * cardSize );
   Serial.print( ( ( cardSize + 500 ) / 1000L * 512L + 500 ) / 1000L );
   Serial.println( F( "MB (MB = 1,000,000 bytes)" ) );

   Serial.print( F( "flashEraseSize: " ) );
   Serial.print( eraseSize );
   Serial.println( F( " blocks" ) );

   Serial.print( F( "eraseSingleBlock: " ) );
   if( eraseSingleBlock )
   {
      Serial.println( F( "true" ) );
   }
   else
   {
      Serial.println( F( "false" ) );
   }

   return true;
}


// --------------------------------------------------------------------------
// print partition table
// --------------------------------------------------------------------------

uint8_t dumpMBR( void )
{
   LOGD( TAG, "dumpMBR" );

   csd_t csd;
   MbrSector_t mbr;     // requires 465 bytes
   bool valid = true;

   if( !sd.card()->readSector( 0, ( uint8_t* )&mbr ) )
   {
      LOGE( TAG, "read MBR failed" );
      sdErrorMsg( F( "read MBR failed" ) );
      return false;
   }

   for( uint8_t ip = 1; ip < 5; ip++ )
   {
      MbrPart_t *pt = &mbr.part[ip - 1];
      if( ( pt->boot != 0 && pt->boot != 0X80 ) ||
          getLe32( pt->relativeSectors ) > sdCardCapacity(&csd))
      {
         valid = false;
      }
   }

   Serial.println( F( "\nSD Partition Table" ) );
   Serial.println( F( "part,boot,type,start,length" ) );

   for( uint8_t ip = 1; ip < 5; ip++ )
   {
      MbrPart_t *pt = &mbr.part[ip - 1];
      Serial.print( pt->boot, HEX );
      Serial.print( F( ", " ) );
      Serial.print( pt->type, HEX );
      Serial.print( F( ", " ) );
      Serial.print( getLe32( pt->relativeSectors ) );
      Serial.print( F( ", " ) );
      Serial.println( getLe32( pt->totalSectors ) );
   }

   if (!valid)
   {
      Serial.println( F( "\nNo MBR. Assuming Super Floppy format." ) );
   }
   return true;
}

// --------------------------------------------------------------------------
//
// --------------------------------------------------------------------------

void dumpVolume( void )
{
   uint32_t freeClusterCount = sd.freeClusterCount();
   if( sd.fatType() <= 32 )
   {
      Serial.print( F( "\nVolume is FAT" ) );
      Serial.println( sd.fatType() );
   }
   else
   {
      Serial.print( F( "\nVolume is exFAT" ) );
   }

   Serial.print( F( "sectorsPerCluster: " ) );
   Serial.println( sd.sectorsPerCluster() );

   Serial.print( F( "clusterCount: " ) );
   Serial.println( sd.clusterCount() );

   uint32_t volFree = sd.freeClusterCount();
   Serial.print( F( "freeClusters: " ) );
   Serial.println( volFree );

   Serial.print( F( "fatStartSector: " ) );
   Serial.println( sd.fatStartSector() );

   Serial.print( F( "dataStartSector: " ) );
   Serial.println( sd.dataStartSector() );

   if( sd.vol()->dataStartSector() % eraseSize )
   {
      Serial.println( F( "Data area is not aligned on flash erase boundaries!" ) );
      Serial.println( F( "Download and use formatter from www.sdcard.org!" ) );
   }
}

// --------------------------------------------------------------------------
//
// --------------------------------------------------------------------------

void printCardType( void )
{
   Serial.print( F( "Card type: " ) );
   switch( sd.card()->type() )
   {
      case SD_CARD_TYPE_SD1:
         Serial.println( F( "SD1" ) );
         break;

      case SD_CARD_TYPE_SD2:
         Serial.println( F( "SD2" ) );
         break;

      case SD_CARD_TYPE_SDHC:
         if( cardSize < 70000000 )
         {
            Serial.println( F( "SDHC" ) );
         }
         else
         {
            Serial.println( F( "SDXC" ) );
         }
         break;

      default:
         Serial.println( F( "Unknown" ) );
   }
}

// --------------------------------------------------------------------------
//
// --------------------------------------------------------------------------

void dumpSdCardInfo( void )
{
   Serial.println( F( "\nInitializing SD card..." ) );

   Serial.print( F( "SdFat version: " ) );
   Serial.println( SD_FAT_VERSION );

   cardSize = sd.card()->sectorCount() * 512;
   if( cardSize == 0 )
   {
      LOGE( TAG, "cardSize failed " );
      sdErrorMsg( F( "cardSize failed" ) );
      return;
   }

   printCardType();

   if( !dumpCID() )
   {
      LOGE( TAG, "dumpCID failed " );
      return;
   }

   if( !dumpCSD() )
   {
      LOGE( TAG, "dumpCSD failed " );
      return;
   }

   uint32_t ocr;
   if( !sd.card()->readOCR( &ocr ) )
   {
      LOGE( TAG, "readOCR failed " );
      sdErrorMsg( F( "\nreadOCR failed" ) );
      return;
   }

   Serial.print( F( "OCR: " ) );
   Serial.println( ocr, HEX );

#ifdef HAVE_SPACE
   // becasue of low memory space we cannot dump the partition information
   if( !dumpMBR() )
   {
      LOGE( TAG, "dumpMBR failed " );
      return;
   }

   if( !sd.volumeBegin() )
   {
      sdErrorMsg( F( "\nFile System initialization failed." ) );
      LOGE( TAG, "File System initialization failed. failed " );
      return;
   }

   // it takes 10s to get the free  cluster size
   dumpMBR();
#endif
}

// --------------------------------------------------------------------------
//
// --------------------------------------------------------------------------

void dumpDirectory( SdFat *dir )
{
   Serial.println( F( "\nFiles found on the card (name, date and size in bytes): " ) );

   // list all files in the card with date and size
   Serial.println( F( "list all files in the card with date and size" ) );
   dir->ls( LS_R | LS_DATE | LS_SIZE );
}

// --------------------------------------------------------------------------
//
// --------------------------------------------------------------------------
