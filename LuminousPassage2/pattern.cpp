// File: pattern.cpp

//*****************
// INCLUDES
//*****************

#include "pattern.h"

#ifndef SOFTWARE

#include <Arduino.h>

#include "com.h"
#include "config.h"
#include "utility.h"

#else

#endif

#include "global.h"

//*****************
// DEFINITIONS
//*****************

#ifdef FIRMWARE
#define DEBUG
#endif

#define MAX_SIMULTANEOUS        4

#define MIN_BUCKET_THRESHOLD    50

#define BASS_PULSE_NUM          10

//*****************
// VARIABLES
//*****************

volatile static quint16 mPatternChannels[GLOBAL_NUM_CHANNELS] = {0};

volatile static quint8 mInterruptCount = 0;

volatile static quint8 mPatternReady = 0;
volatile static quint32 mLastPatternTime = 0;

volatile static quint8 mPatternFrequencies[GLOBAL_NUM_FREQ] = {0};

volatile static quint16 mPatternBuckets[GLOBAL_NUM_BUCKETS] = {0};
volatile static quint16 mPatternBucketAverages[GLOBAL_NUM_BUCKETS] = {0};

volatile static quint8 mRawMean = 128;
volatile static quint8 mRawPeak = 128;

volatile static quint8 mEffects[Effect_MAX] = {0};

volatile static quint16 mTick = 0;
volatile static quint16 mLastTick[Effect_MAX] = {0};

// Effect_CYCLE
volatile static quint8 mCycleCount = 0;

// Effect_BREATH
volatile static quint8 mSweepState = 1;
volatile static quint8 mSweepValue = 0;

// Buckets to digital inputs
volatile static quint8 mBucketState[GLOBAL_NUM_BUCKETS] = {0};
volatile static float mBucketThreshold = 0.20;
volatile static quint8 mDigitalBucketInput[GLOBAL_NUM_BUCKETS][MAX_SIMULTANEOUS];
volatile static quint8 mDigitalBucketTicks[GLOBAL_NUM_BUCKETS][MAX_SIMULTANEOUS];

//*****************
// PRIVATE PROTOTYPES
//*****************

void PatternProcessInternal( void );


//*****************
// PUBLIC
//*****************

void PatternInit( void )
{
    for( quint8 i=0; i<GLOBAL_NUM_CHANNELS; i++ )
    {
        mPatternChannels[i] = 0x0A;
    }

    for( quint8 i=0; i<GLOBAL_NUM_BUCKETS; i++ )
    {
        for( quint8 j=0; j<MAX_SIMULTANEOUS; j++ )
        {
            mDigitalBucketInput[i][j] = 0;
            mDigitalBucketTicks[i][j] = 0;
        }
    }

#if defined( PATTERN_BLOCKING_VERSION ) && defined ( FIRMWARE )
    mLastPatternTime = millis();
#endif

#if defined( PATTERN_ISR_VERSION ) && defined( FIRMWARE )
    // Start TIMER2 for interrupting at 100Hz (10ms), tick time
    TCCR2A = 0x02;
    TCCR2B = 0x06;
    TCNT2 = 0x00;
    OCR2A = 125;
    TIMSK2 = 0x02;
#endif

#ifdef DEBUG
    DebugInit();
#endif
}

void PatternSetEffect( eEffect effect, quint8 on )
{
#if defined( PATTERN_ISR_VERSION ) && defined( FIRMWARE )
    cli();
#endif
    mEffects[effect] = on;
    switch( effect )
    {
    case Effect_CYCLE:
        mCycleCount = 0;
        break;
    case Effect_BREATH:
        mSweepState = 0;
        mSweepValue = 0;
        break;
    default:
        break;
    }
#if defined( PATTERN_ISR_VERSION ) && defined( FIRMWARE )
    sei();
#endif
}

quint8 PatternReady( void )
{
    return mPatternReady;
}

void PatternData( quint8* data )
{
#if defined( PATTERN_ISR_VERSION ) && defined( FIRMWARE )
    cli();
#endif
    for( quint8 i=0; i<GLOBAL_NUM_CHANNELS; i++ )
    {
        quint16 value = mPatternChannels[i];
        data[i] = ( value < 0xFF ) ? value : 0xFF;
    }
#if defined( PATTERN_ISR_VERSION ) && defined( FIRMWARE )
    sei();
#endif

    mPatternReady = 0;
}

void PatternProcess( void )
{
#if defined( PATTERN_BLOCKING_VERSION ) && defined( FIRMWARE )
    if( ( micros() - mLastPatternTime ) >= 10000 )
    {
        mLastPatternTime += 10000;
        PatternProcessInternal();
    }
#endif

#if defined( SOFTWARE )
    PatternProcessInternal();
#endif
}

void PatternUpdateFreq( quint8* newFrequencies )
{
#if defined( PATTERN_ISR_VERSION ) && defined( FIRMWARE )
    cli();
#endif

    // Filter for noise
    for( quint8 i=0; i<GLOBAL_NUM_FREQ; i++ )
    {
        quint8 value = newFrequencies[i];
        mPatternFrequencies[i] = ( value > 0x02 ) ? ( value - 0x02 ) : 0x00;
    }

#if defined( PATTERN_ISR_VERSION ) && defined( FIRMWARE )
    sei();
#endif
}

void PatternUpdateBuckets( quint16* newBuckets, quint16* newBucketAverages )
{
#if defined( PATTERN_ISR_VERSION ) && defined( FIRMWARE )
    cli();
#endif

    for( quint8 i=0; i<GLOBAL_NUM_BUCKETS; i++ )
    {
        mPatternBuckets[i] = newBuckets[i];
        mPatternBucketAverages[i] = newBucketAverages[i];
    }

    // Filter for any digital inputs
    for( quint8 i=0; i<GLOBAL_NUM_BUCKETS; i++ )
    {
        quint16 value = mPatternBuckets[i];
        quint16 avg = mPatternBucketAverages[i];
        quint16 hi = avg * ( 1.0 + mBucketThreshold );
        quint16 lo = avg * ( 1.0 - mBucketThreshold );
        if( mBucketState[i] )
        {
            if( ( value < lo ) || ( avg < MIN_BUCKET_THRESHOLD ) )
            {
                mBucketState[i] = 0;
            }
        }
        else
        {
            if( ( value > hi ) && ( avg > MIN_BUCKET_THRESHOLD ) )
            {
                mBucketState[i] = 1;

                // Find open digital input
                for( quint8 j=0; j<MAX_SIMULTANEOUS; j++ )
                {
                    if( !mDigitalBucketInput[i][j] )
                    {
                        mDigitalBucketInput[i][j] = 1;
                        mDigitalBucketTicks[i][j] = 0;
                        break;
                    }
                }
            }
        }
    }

#if defined( PATTERN_ISR_VERSION ) && defined( FIRMWARE )
    sei();
#endif
}

void PatternUpdateAd( quint8 mean, quint8 peak )
{
#if defined( PATTERN_ISR_VERSION ) && defined( FIRMWARE )
    cli();
#endif

    mRawMean = mean;
    mRawPeak = peak;

#if defined( PATTERN_ISR_VERSION ) && defined( FIRMWARE )
    sei();
#endif
}

void PatternProcessInternal( void )
{
    mPatternReady = 1;
    mTick++;

    for( quint8 i=0; i<GLOBAL_NUM_CHANNELS; i++ )
    {
        mPatternChannels[i] = 0;
    }

    if( mEffects[Effect_MANUAL] )
    {
        // if( mCommandReceived )
        // {
        //     if( mCommand == 'u' )
        //     {
        //         mSweepValue += 0x08;
        //         for( quint8 i=0; i<GLOBAL_NUM_CHANNELS; i++ )
        //         {
        //             mPatternChannels[i] += mSweepValue;
        //         }
        //     }
        //     else if( mCommand == 'd' )
        //     {
        //         mSweepValue -= 0x08;
        //         for( quint8 i=0; i<GLOBAL_NUM_CHANNELS; i++ )
        //         {
        //             mPatternChannels[i] += mSweepValue;
        //         }
        //     }
        //     else
        //     {
        //     }
        // }
    }

    if( mEffects[Effect_CYCLE] )
    {
        if( mTick - mLastTick[Effect_CYCLE] >= 10 )
        {
            mLastTick[Effect_CYCLE] = mTick;
            if( ++mCycleCount >= GLOBAL_NUM_CHANNELS )
            {
                mCycleCount = 0;
            }
        }
        mPatternChannels[mCycleCount] += 0xFF;
    }

    if( mEffects[Effect_BREATH] )
    {
        if( mTick - mLastTick[Effect_BREATH] >= 5 )
        {
            mLastTick[Effect_BREATH] = mTick;
            if( mSweepState )
            {
                mSweepValue += 0x08;
                mSweepState = ( mSweepValue > 0xF7 ) ? 0 : 1;
            }
            else
            {
                mSweepValue -= 0x08;
                mSweepState = ( mSweepValue < 0x07 ) ? 1 : 0;
            }
        }
        for( quint8 i=0; i<GLOBAL_NUM_CHANNELS; i++ )
        {
            mPatternChannels[i] += mSweepValue;
        }
    }

    if( mEffects[Effect_FREQ] )
    {
        for( quint8 i=0; i<GLOBAL_NUM_CHANNELS; i++ )
        {
            quint16 raw = mPatternFrequencies[i] * 4;
            mPatternChannels[i] += raw;
        }
    }

    if( mEffects[Effect_BUCKETS] )
    {
        quint16 one = mPatternBuckets[0];
        quint16 two = mPatternBuckets[1];

        for( quint8 i=0; i<10; i++ )
        {
            mPatternChannels[i] += one;
        }
        for( quint8 i=10; i<20; i++ )
        {
            mPatternChannels[i] += two;
        }
    }

    if( mEffects[Effect_BASS_PULSE] )
    {
        if( mTick - mLastTick[Effect_BASS_PULSE] >= 5 )
        {
            mLastTick[Effect_BASS_PULSE] = mTick;
            for( quint8 i=0; i<MAX_SIMULTANEOUS; i++ )
            {
                if( mDigitalBucketInput[1][i] )
                {
                    if( ++mDigitalBucketTicks[1][i] > ( BASS_PULSE_NUM + 1 ) )
                    {
                        mDigitalBucketTicks[1][i] = 0;
                        mDigitalBucketInput[1][i] = 0;
                    }
                }
            }
        }

        for( quint8 i=0; i<MAX_SIMULTANEOUS; i++ )
        {
            if( mDigitalBucketInput[1][i] )
            {
                for( quint8 j=0; j<BASS_PULSE_NUM; j++ )
                {
                    mPatternChannels[j] += ( mDigitalBucketTicks[1][i] == j || mDigitalBucketTicks[1][i] == (j+1) ) ? 0x7F : 0x00;
                }
            }
        }
    }

}

#if defined( PATTERN_ISR_VERSION ) && defined( FIRMWARE )
ISR( TIMER2_COMPA_vect )
{
    if( ++mInterruptCount >= 5 )
    {
#ifdef DEBUG
    DebugUp();
#endif
        PatternProcessInternal();
        mInterruptCount = 0;
#ifdef DEBUG
    DebugDown();
#endif
    }
}
#endif
