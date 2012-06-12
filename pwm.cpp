
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
  Wire.begin();

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
    case 0: mThreshold = 0x20; break;
    case 1: mThreshold = 0x40; break;
    case 2: mThreshold = 0x60; break;
    case 3: mThreshold = 0x80; break;
    case 4: mThreshold = 0xA0; break;
    case 5: mThreshold = 0xC0; break;
    case 6: mThreshold = 0xE0; break;
    case 7: mThreshold = 0xFF; break;
    default: break;
  }

  for( int i=0; i<16; i++ )
  {
    mBytes[i] = 0x00;
    mBytes[i] |= ( channelValues[i*8 + 0] < mThreshold ) ? 0x01 : 0x00;
    mBytes[i] |= ( channelValues[i*8 + 1] < mThreshold ) ? 0x02 : 0x00;
    mBytes[i] |= ( channelValues[i*8 + 2] < mThreshold ) ? 0x04 : 0x00;
    mBytes[i] |= ( channelValues[i*8 + 3] < mThreshold ) ? 0x08 : 0x00;
    mBytes[i] |= ( channelValues[i*8 + 4] < mThreshold ) ? 0x10 : 0x00;
    mBytes[i] |= ( channelValues[i*8 + 5] < mThreshold ) ? 0x20 : 0x00;
    mBytes[i] |= ( channelValues[i*8 + 6] < mThreshold ) ? 0x40 : 0x00;
    mBytes[i] |= ( channelValues[i*8 + 7] < mThreshold ) ? 0x80 : 0x00;
  }
  
  if( ++mStepCount >= 8 )
  {
    mStepCount = 0;
  }
  
  Serial.print( " CH1=" );
  Serial.print( channelValues[0] );
  Serial.print( " CH2=" );
  Serial.print( channelValues[1] );
  Serial.print( " BYTE1=" );
  Serial.print( mBytes[0] );
  Serial.print( "\n\r" );

  byte result;
  Wire.begin();

  Wire.beginTransmission( IC1 );
  Wire.write( (byte)REG_GPIOA );
  Wire.write( &mBytes[0], 2 );
  result = Wire.endTransmission();
  /*
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
/*
  Wire.beginTransmission( IC5 );
  Wire.write( (byte)REG_GPIOA );
  Wire.write( &mBytes[8], 2 );
  result = Wire.endTransmission();
  
  Wire.beginTransmission( IC6 );
  Wire.write( (byte)REG_GPIOA );
  Wire.write( &mBytes[10], 2 );
  result = Wire.endTransmission();

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

