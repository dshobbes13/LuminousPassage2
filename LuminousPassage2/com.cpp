// File: com.cpp

//*****************
// INCLUDES
//*****************

#include "com.h"

#include <Arduino.h>

#include "config.h"
#include "global.h"
#include "utility.h"


//*****************
// DEFINITIONS
//*****************

//#define DEBUG

#define BUFFER_LENGTH   16

#define TIMEOUT         10


//*****************
// VARIABLES
//*****************

static unsigned char mBufferRx[BUFFER_LENGTH] = {0};
static unsigned char mCurrentLength = 0;

static unsigned char mMessage[MESSAGE_LENGTH] = {0};

static unsigned long mBytesPendingTime = 0;


//*****************
// PRIVATE PROTOTYPES
//*****************


//*****************
// PUBLIC
//*****************

void ComInit( void )
{
    Serial.begin( 115200 );
    while( !Serial );
    Serial.setTimeout( 0 );

#ifdef DEBUG
    DebugInit();
#endif
}

unsigned char ComProcess( void )
{
    unsigned char newMessage = 0;

    if( mCurrentLength > 0 )
    {
        if( ( millis() - mBytesPendingTime ) >= TIMEOUT )
        {
            // Flush buffer
            mCurrentLength = 0;
        }
    }

    if( Serial.available() )
    {
        unsigned char bytesRead = Serial.readBytes( (char*)(&mBufferRx[mCurrentLength]), BUFFER_LENGTH - mCurrentLength );
        mCurrentLength += bytesRead;
        if( mCurrentLength >= MESSAGE_LENGTH )
        {
            newMessage = 1;
            memcpy( mMessage, mBufferRx, MESSAGE_LENGTH );
            mCurrentLength -= MESSAGE_LENGTH;
            for( unsigned char i=0; i<mCurrentLength; i++ )
            {
                mBufferRx[i] = mBufferRx[i+MESSAGE_LENGTH];
            }
        }
    }

    if( mCurrentLength > 0 )
    {
        mBytesPendingTime = millis();
    }

    return newMessage;
}

void ComSendFft( unsigned char* data )
{
    Serial.write( data, GLOBAL_NUM_FREQ );
}

eCommand ComGetCommand( void )
{
    return (eCommand)mBufferRx[0];
}

unsigned char ComGetByte( unsigned char byteNumber )
{
    return mBufferRx[1+byteNumber];
}

void ComPrint( const char* string )
{
    Serial.print( string );
}

void ComPrintUchar( unsigned char data )
{
    Serial.print( data );
}

void ComPrintHex( unsigned char data )
{
    Serial.print( data, HEX );
}

void ComPrintUintHex( unsigned int data )
{
    Serial.print( (unsigned char)( data >> 8 ), HEX );
    Serial.print( (unsigned char)data, HEX );
}

void ComPrintDataChar( const char* label, char* data, unsigned char n )
{
    Serial.print( label );
    for( unsigned int i=0; i<n; i++ )
    {
        Serial.print( HexString( data[i] ) );
        Serial.print( "::" );
    }
}

void ComPrintDataUchar( const char* label, unsigned char* data, unsigned char n )
{
    Serial.print( label );
    for( unsigned int i=0; i<n; i++ )
    {
        Serial.print( HexString( data[i] ) );
        Serial.print( "::" );
    }
}

void ComPrintDataUint( const char* label, unsigned int* data, unsigned char n )
{
    Serial.print( label );
    for( unsigned int i=0; i<n; i++ )
    {
        unsigned char hi = (unsigned char)( data[i] >> 8 );
        unsigned char lo = (unsigned char)data[i];
        Serial.print( HexString( hi ) );
        Serial.print( HexString( lo ) );
        Serial.print( "::" );
    }
}


//*****************
// PRIVATE
//*****************

