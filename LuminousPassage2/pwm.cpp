// File: pwm.cpp

//*****************
// INCLUDES
//*****************

#include "pwm.h"

#include <Arduino.h>

#include "config.h"
#include "global.h"
#include "utility.h"


//*****************
// DEFINITIONS
//*****************

//#define DEBUG

//#define THRESH_16
#define THRESH_24
//#define THRESH_32
//#define THRESH_48

#if defined( THRESH_16 )
    #define PWM_NUM_THRESH  16
#elif defined( THRESH_24 )
    #define PWM_NUM_THRESH  24
#elif defined( THRESH_32 )
    #define PWM_NUM_THRESH  32
#elif defined( THRESH_48 )
    #define PWM_NUM_THRESH  48
#else
    #define PWM_NUM_THRESH  1
#endif


//*****************
// VARIABLES
//*****************


volatile static unsigned char mChannelValues[GLOBAL_NUM_SINGLE_CH] = {0};


#if defined( THRESH_16 )
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
#elif defined( THRESH_24 )
volatile static unsigned char mThresholdMap[PWM_NUM_THRESH] =
{
    0x0A,
    0x14,
    0x1F,
    0x29,
    0x33,
    0x3D,
    0x48,
    0x52,
    0x5C,
    0x66,
    0x71,
    0x7B,
    0x85,
    0x8F,
    0x9A,
    0xA4,
    0xAE,
    0xB8,
    0xC3,
    0xCD,
    0xD7,
    0xE1,
    0xEC,
    0xF6,
};
#elif defined( THRESH_32 )
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
#elif defined( THRESH_48 )
volatile static unsigned char mThresholdMap[PWM_NUM_THRESH] =
{
    0x05,
    0x0A,
    0x10,
    0x15,
    0x1A,
    0x1F,
    0x24,
    0x2A,
    0x2F,
    0x34,
    0x39,
    0x3E,
    0x44,
    0x49,
    0x4E,
    0x53,
    0x58,
    0x5E,
    0x63,
    0x68,
    0x6D,
    0x72,
    0x78,
    0x7D,
    0x82,
    0x87,
    0x8D,
    0x92,
    0x97,
    0x9C,
    0xA1,
    0xA7,
    0xAC,
    0xB1,
    0xB6,
    0xBB,
    0xC1,
    0xC6,
    0xCB,
    0xD0,
    0xD5,
    0xDB,
    0xE0,
    0xE5,
    0xEA,
    0xEF,
    0xF5,
    0xFA,
};
#else
volatile static unsigned char mThresholdMap[PWM_NUM_THRESH] =
{
    0x80,
};
#endif

//*****************
// PRIVATE PROTOTYPES
//*****************


//*****************
// PUBLIC
//*****************

void PwmInit( void )
{

#ifdef DEBUG
    DebugInit();
#endif

    // Configure INT0 pin
    DDRD = DDRD & ~0x04;

    // Configure output port
    DDRD = DDRD |= 0xF0;
    DDRB = DDRB |= 0x3F;

    // Configure INT0 interrupt
    //EICRA = 0x01;  // On toggle
    //EICRA = 0x02;  // On falling
    EICRA = 0x03;  // On rising
    EIFR = 0x01;
    EIMSK = 0x01;
}

void PwmProcess( void )
{
}

void PwmSetChannels( unsigned char* channelValues )
{
    cli();
    mChannelValues[0] = channelValues[CH0];
    mChannelValues[1] = channelValues[CH1];
    mChannelValues[2] = channelValues[CH2];
    mChannelValues[3] = channelValues[CH3];
    mChannelValues[4] = channelValues[CH4];
    mChannelValues[5] = channelValues[CH5];
    mChannelValues[6] = channelValues[CH6];
    mChannelValues[7] = channelValues[CH7];
    mChannelValues[8] = channelValues[CH8];
    mChannelValues[9] = channelValues[CH9];
    sei();
}

//*****************
// PRIVATE
//*****************

ISR( INT0_vect )
{
    static unsigned long lastTime = micros();
    // Debounce for noise
    if( ( micros() - lastTime ) < 500 )
    {
        return;
    }
    lastTime = micros();

#ifdef DEBUG
    DebugUp();
#endif

    static unsigned char mStepCount = 0;
    static unsigned char mThreshold = 0;
    static unsigned char mValueD = 0;
    static unsigned char mValueB = 0;

    // Get current threshold
    mThreshold = mThresholdMap[mStepCount];

    // Build port write
    mValueD = 0xFF;
    mValueD &= ( mChannelValues[0x00] >= mThreshold ) ? ~0x10 : 0xFF;
    mValueD &= ( mChannelValues[0x01] >= mThreshold ) ? ~0x20 : 0xFF;
    mValueD &= ( mChannelValues[0x02] >= mThreshold ) ? ~0x40 : 0xFF;
    mValueD &= ( mChannelValues[0x03] >= mThreshold ) ? ~0x80 : 0xFF;
    
    mValueB = 0xFF;
    mValueB &= ( mChannelValues[0x04] >= mThreshold ) ? ~0x01 : 0xFF;
    mValueB &= ( mChannelValues[0x05] >= mThreshold ) ? ~0x02 : 0xFF;
    mValueB &= ( mChannelValues[0x06] >= mThreshold ) ? ~0x04 : 0xFF;
    mValueB &= ( mChannelValues[0x07] >= mThreshold ) ? ~0x08 : 0xFF;
    mValueB &= ( mChannelValues[0x08] >= mThreshold ) ? ~0x10 : 0xFF;
    mValueB &= ( mChannelValues[0x09] >= mThreshold ) ? ~0x20 : 0xFF;

    PORTD = mValueD & 0xF0; 
    PORTB = mValueB & 0x3F;

    if( ++mStepCount >= PWM_NUM_THRESH )
    {
        mStepCount = 0;
    }

#ifdef DEBUG
    DebugDown();
#endif
}

