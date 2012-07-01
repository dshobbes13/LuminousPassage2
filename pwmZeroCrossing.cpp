// File: pwmZeroCrossing.cpp

//*****************
// INCLUDES
//*****************

#include "pwmZeroCrossing.h"

#include <Arduino.h>

#include "utility.h"


//*****************
// DEFINITIONS
//*****************

#define DEBUG

#define PWM_NUM_THRESH  32


//*****************
// VARIABLES
//*****************


volatile static unsigned char mChannelValues[PWM_ZERO_CROSSING_NUM_CHANNELS] = {0};

/*
volatile static unsigned char mThresholdMap[PWM_NUM_THRESH] =
{
    0x0F,
    0x1E,
    0x2D,
    0x3C,
    0x4B,
    0x5A,
    0x69,
    0x78,
    0x87,
    0x96,
    0xA5,
    0xB4,
    0xC3,
    0xD2,
    0xE1,
    0xF0,
};
*/

volatile static unsigned char mThresholdMap[PWM_NUM_THRESH] =
{
    0x07,
    0x0F,
    0x17,
    0x1E,
    0x26,
    0x2E,
    0x36,
    0x3C,
    0x45,
    0x4C,
    0x55,
    0x5B,
    0x64,
    0x6B,
    0x74,
    0x7A,
    0x83,
    0x8A,
    0x93,
    0x99,
    0xA2,
    0xA9,
    0xB2,
    0xB8,
    0xC1,
    0xC8,
    0xD1,
    0xD7,
    0xE0,
    0xE7,
    0xF0,
    0xF7,
};

//*****************
// PRIVATE PROTOTYPES
//*****************


//*****************
// PUBLIC
//*****************

void PwmZeroCrossingInit( void )
{

#ifdef DEBUG
    // Debug
    DDRD = DDRD | 0x80;
    PORTD = PORTD & ~0x80;
#endif

    // Configure INT0 pin
    DDRD = DDRD & ~0x04;

    // Configure output port
    DDRB = DDRB |= 0x3F;

    // Configure INT0 interrupt
    EICRA = 0x02;
    EIFR = 0x01;
    EIMSK = 0x01;
}

void PwmZeroCrossingProcess( void )
{
}

void PwmZeroCrossingSetChannels( unsigned char* channelValues )
{
    cli();
    memcpy( (void*)mChannelValues, channelValues, PWM_ZERO_CROSSING_NUM_CHANNELS );
    sei();
}

//*****************
// PRIVATE
//*****************

ISR( INT0_vect )
{
#ifdef DEBUG
    PORTD = PORTD | 0x80;
#endif

    static unsigned char mStepCount = 0;
    static unsigned char mThreshold = 0;
    static unsigned char mValue = 0;

    // Get current threshold
    mThreshold = mThresholdMap[mStepCount];

    // Build port write
    mValue = 0xFF;
    mValue &= ( mChannelValues[0x00] > mThreshold ) ? ~0x01 : 0xFF;
    mValue &= ( mChannelValues[0x01] > mThreshold ) ? ~0x02 : 0xFF;
    mValue &= ( mChannelValues[0x02] > mThreshold ) ? ~0x04 : 0xFF;
    mValue &= ( mChannelValues[0x03] > mThreshold ) ? ~0x08 : 0xFF;
    mValue &= ( mChannelValues[0x04] > mThreshold ) ? ~0x10 : 0xFF;
    mValue &= ( mChannelValues[0x05] > mThreshold ) ? ~0x20 : 0xFF;
    mValue &= ( mChannelValues[0x06] > mThreshold ) ? ~0x40 : 0xFF;
    mValue &= ( mChannelValues[0x07] > mThreshold ) ? ~0x80 : 0xFF;
    PORTB = mValue & 0x3F;

    if( ++mStepCount >= PWM_NUM_THRESH )
    {
        mStepCount = 0;
    }

#ifdef DEBUG
    PORTD = PORTD & ~0x80;
#endif
}

