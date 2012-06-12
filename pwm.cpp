
#include "pwm.h"

#include <Arduino.h>
#include <Wire.h>

#define IC1 0x20
#define IC2 0x21
#define IC3 0x22
#define IC4 0x23
#define IC5 0x20
#define IC6 0x21
#define IC7 0x22
#define IC8 0x23

#define REG_IODIRA 0x00
#define REG_IDDIRB 0x01
#define REG_GPIOA  0x12
#define REG_GPIOB  0x13

#define NUM_STEPS  8

void PwmInit( void )
{
    Serial.print( " TWSR=" );
    Serial.print( TWSR );
    Serial.print( " TWBR=" );
    Serial.print( TWBR );
    Serial.print( "\n\r" );

    Wire.beginTransmission( IC1 );
    Wire.write( (byte)REG_IODIRA );
    Wire.write( (byte)0x00 );
    Wire.write( (byte)0x00 );
    Wire.endTransmission();

    Wire.beginTransmission( IC2 );
    Wire.write( (byte)REG_IODIRA );
    Wire.write( (byte)0x00 );
    Wire.write( (byte)0x00 );
    Wire.endTransmission();

    Wire.beginTransmission( IC3 );
    Wire.write( (byte)REG_IODIRA );
    Wire.write( (byte)0x00 );
    Wire.write( (byte)0x00 );
    Wire.endTransmission();

    Wire.beginTransmission( IC4 );
    Wire.write( (byte)REG_IODIRA );
    Wire.write( (byte)0x00 );
    Wire.write( (byte)0x00 );
    Wire.endTransmission();

    Wire.beginTransmission( IC5 );
    Wire.write( (byte)REG_IODIRA );
    Wire.write( (byte)0x00 );
    Wire.write( (byte)0x00 );
    Wire.endTransmission();

    Wire.beginTransmission( IC6 );
    Wire.write( (byte)REG_IODIRA );
    Wire.write( (byte)0x00 );
    Wire.write( (byte)0x00 );
    Wire.endTransmission();

    Wire.beginTransmission( IC7 );
    Wire.write( (byte)REG_IODIRA );
    Wire.write( (byte)0x00 );
    Wire.write( (byte)0x00 );
    Wire.endTransmission();

    Wire.beginTransmission( IC8 );
    Wire.write( (byte)REG_IODIRA );
    Wire.write( (byte)0x00 );
    Wire.write( (byte)0x00 );
    Wire.endTransmission();
}

void UpdateChannels( unsigned char* channelValues )
{
    static byte mStepCount = 0;
    static byte mBytes[16] = {0};
    static byte mThreshold = 0;

    switch( mStepCount )
    {
    case 0x00: mThreshold = 0x0F; break;
    case 0x01: mThreshold = 0x1E; break;
    case 0x02: mThreshold = 0x2D; break;
    case 0x03: mThreshold = 0x3C; break;
    case 0x04: mThreshold = 0x4B; break;
    case 0x05: mThreshold = 0x5A; break;
    case 0x06: mThreshold = 0x69; break;
    case 0x07: mThreshold = 0x78; break;
    case 0x08: mThreshold = 0x87; break;
    case 0x09: mThreshold = 0x96; break;
    case 0x0A: mThreshold = 0xA5; break;
    case 0x0B: mThreshold = 0xB4; break;
    case 0x0C: mThreshold = 0xC3; break;
    case 0x0D: mThreshold = 0xD2; break;
    case 0x0E: mThreshold = 0xE1; break;
    case 0x0F: mThreshold = 0xF0; break;
    default: break;
    }

    for( int i=0; i<16; i++ )
    {
        mBytes[i] = 0xFF;
        mBytes[i] &= ( channelValues[i*8 + 0] > mThreshold ) ? ~0x01 : 0xFF;
        mBytes[i] &= ( channelValues[i*8 + 1] > mThreshold ) ? ~0x02 : 0xFF;
        mBytes[i] &= ( channelValues[i*8 + 2] > mThreshold ) ? ~0x04 : 0xFF;
        mBytes[i] &= ( channelValues[i*8 + 3] > mThreshold ) ? ~0x08 : 0xFF;
        mBytes[i] &= ( channelValues[i*8 + 4] > mThreshold ) ? ~0x10 : 0xFF;
        mBytes[i] &= ( channelValues[i*8 + 5] > mThreshold ) ? ~0x20 : 0xFF;
        mBytes[i] &= ( channelValues[i*8 + 6] > mThreshold ) ? ~0x40 : 0xFF;
        mBytes[i] &= ( channelValues[i*8 + 7] > mThreshold ) ? ~0x80 : 0xFF;
    }

    if( ++mStepCount >= 16 )
    {
        mStepCount = 0;
    }

    byte result;

    Wire.beginTransmission( IC1 );
    Wire.write( (byte)REG_GPIOA );
    Wire.write( &mBytes[0], 2 );
    result = Wire.endTransmission();

    Wire.beginTransmission( IC2 );
    Wire.write( (byte)REG_GPIOA );
    Wire.write( &mBytes[2], 2 );
    result = Wire.endTransmission();

    Wire.beginTransmission( IC3 );
    Wire.write( (byte)REG_GPIOA );
    Wire.write( &mBytes[4], 2 );
    result = Wire.endTransmission();

    Wire.beginTransmission( IC4 );
    Wire.write( (byte)REG_GPIOA );
    Wire.write( &mBytes[6], 2 );
    result = Wire.endTransmission();

    Wire.beginTransmission( IC5 );
    Wire.write( (byte)REG_GPIOA );
    Wire.write( &mBytes[8], 2 );
    result = Wire.endTransmission();

    Wire.beginTransmission( IC6 );
    Wire.write( (byte)REG_GPIOA );
    Wire.write( &mBytes[10], 2 );
    result = Wire.endTransmission();
/*
    Wire.beginTransmission( IC7 );
    Wire.write( (byte)REG_GPIOA );
    Wire.write( &mBytes[12], 2 );
    result = Wire.endTransmission();

    Wire.beginTransmission( IC8 );
    Wire.write( (byte)REG_GPIOA );
    Wire.write( &mBytes[14], 2 );
    result = Wire.endTransmission();
*/
}

