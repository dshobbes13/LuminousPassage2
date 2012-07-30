// File: pattern.cpp

//*****************
// INCLUDES
//*****************

#include "pattern.h"

#ifndef SOFTWARE

#include <Arduino.h>

#include "utility.h"

#else


#endif

#include "global.h"

//*****************
// DEFINITIONS
//*****************

//#define DEBUG


//*****************
// VARIABLES
//*****************

static quint16 mPatternChannels[GLOBAL_NUM_CHANNELS];
static quint8 mPatternChannelsNormalized[GLOBAL_NUM_CHANNELS];

static quint8 mNewFrequencies = 0;
static quint8 mPatternFrequencies[GLOBAL_NUM_FREQ] = {0};

static quint8 mNewBuckets = 0;
static quint16 mPatternBuckets[GLOBAL_NUM_BUCKETS] = {0};
static quint16 mPatternBucketAverages[GLOBAL_NUM_BUCKETS] = {0};

static quint8 mNewRaw = 0;
static quint8 mRawMean = 128;
static quint8 mRawPeak = 128;

static quint8 mEffects[Effect_MAX] = {0};

static quint16 mTick = 0;
static quint16 mLastTick[Effect_MAX] = {0};

// Effect_CYCLE
static quint8 mCycleCount = 0;

// Effect_BREATH
static quint8 mSweepState = 1;
static quint8 mSweepValue = 0;

// Effect_BUCKETS
static quint8 mBucketState[GLOBAL_NUM_BUCKETS];
static quint16 mBucketThreshold = 0x10;

static quint8 mDigitalInput1 = 0;
static quint8 mDigitalTicks1 = 0;

static quint8 mDigitalInput2 = 0;
static quint8 mDigitalTicks2 = 0;


//*****************
// PRIVATE PROTOTYPES
//*****************

quint8 WaveAtTick( quint8 tick );
quint8 Abs( quint8 value );


//*****************
// PUBLIC
//*****************

void PatternInit( void )
{
    for( quint8 i=0; i<GLOBAL_NUM_CHANNELS; i++ )
    {
        mPatternChannels[i] = 0x0A;
    }
}

void PatternSetEffect( eEffect effect, bool on )
{
    mEffects[effect] = on;
}

quint8 PatternGetEffect( eEffect effect )
{
    return mEffects[effect];
}

void PatternProcess( void )
{
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

        for( quint8 i=0; i<40; i++ )
        {
            mPatternChannels[i] += one;
        }
        for( quint8 i=40; i<59; i++ )
        {
            mPatternChannels[i] += two;
        }
    }

    if( mEffects[Effect_BASS_PULSE] )
    {
        if( ( mTick - mLastTick[Effect_BASS_PULSE] >= 1 ) && ( mDigitalInput1 || mDigitalInput2 ) )
        {
            mLastTick[Effect_BASS_PULSE] = mTick;
            if( mDigitalInput1 )
            {
                if( ++mDigitalTicks1 > 11 )
                {
                    mDigitalTicks1 = 0;
                    mDigitalInput1 = 0;
                }
            }
            if( mDigitalInput2 )
            {
                if( ++mDigitalTicks2 > 11 )
                {
                    mDigitalTicks2 = 0;
                    mDigitalInput2 = 0;
                }
            }
        }

        if( mDigitalInput1 )
        {
            mPatternChannels[0] += ( mDigitalTicks1 == 0 || mDigitalTicks1 == 1 ) ? 0x7F : 0x00;
            mPatternChannels[1] += ( mDigitalTicks1 == 1 || mDigitalTicks1 == 2 ) ? 0x7F : 0x00;
            mPatternChannels[2] += ( mDigitalTicks1 == 2 || mDigitalTicks1 == 3 ) ? 0x7F : 0x00;
            mPatternChannels[3] += ( mDigitalTicks1 == 3 || mDigitalTicks1 == 4 ) ? 0x7F : 0x00;
            mPatternChannels[4] += ( mDigitalTicks1 == 4 || mDigitalTicks1 == 5 ) ? 0x7F : 0x00;
            mPatternChannels[5] += ( mDigitalTicks1 == 5 || mDigitalTicks1 == 6 ) ? 0x7F : 0x00;
            mPatternChannels[6] += ( mDigitalTicks1 == 6 || mDigitalTicks1 == 7 ) ? 0x7F : 0x00;
            mPatternChannels[7] += ( mDigitalTicks1 == 7 || mDigitalTicks1 == 8 ) ? 0x7F : 0x00;
            mPatternChannels[8] += ( mDigitalTicks1 == 8 || mDigitalTicks1 == 9 ) ? 0x7F : 0x00;
            mPatternChannels[9] += ( mDigitalTicks1 == 9 || mDigitalTicks1 == 10 ) ? 0x7F : 0x00;
        }

        if( mDigitalInput2 )
        {
            mPatternChannels[0] += ( mDigitalTicks2 == 0 || mDigitalTicks2 == 1 ) ? 0x7F : 0x00;
            mPatternChannels[1] += ( mDigitalTicks2 == 1 || mDigitalTicks2 == 2 ) ? 0x7F : 0x00;
            mPatternChannels[2] += ( mDigitalTicks2 == 2 || mDigitalTicks2 == 3 ) ? 0x7F : 0x00;
            mPatternChannels[3] += ( mDigitalTicks2 == 3 || mDigitalTicks2 == 4 ) ? 0x7F : 0x00;
            mPatternChannels[4] += ( mDigitalTicks2 == 4 || mDigitalTicks2 == 5 ) ? 0x7F : 0x00;
            mPatternChannels[5] += ( mDigitalTicks2 == 5 || mDigitalTicks2 == 6 ) ? 0x7F : 0x00;
            mPatternChannels[6] += ( mDigitalTicks2 == 6 || mDigitalTicks2 == 7 ) ? 0x7F : 0x00;
            mPatternChannels[7] += ( mDigitalTicks2 == 7 || mDigitalTicks2 == 8 ) ? 0x7F : 0x00;
            mPatternChannels[8] += ( mDigitalTicks2 == 8 || mDigitalTicks2 == 9 ) ? 0x7F : 0x00;
            mPatternChannels[9] += ( mDigitalTicks2 == 9 || mDigitalTicks2 == 10 ) ? 0x7F : 0x00;
        }
    }
}

quint8* PatternData( void )
{
    for( quint8 i=0; i<GLOBAL_NUM_CHANNELS; i++ )
    {
        quint16 value = mPatternChannels[i];
        mPatternChannelsNormalized[i] = ( value <= 0xFF ) ? value : 0xFF;
    }
    return mPatternChannelsNormalized;
}

void PatternUpdateFreq( quint8* newFrequencies )
{
    mNewFrequencies = 1;

    // Filter for noise
    for( quint8 i=0; i<GLOBAL_NUM_FREQ; i++ )
    {
        quint8 value = newFrequencies[i];
        mPatternFrequencies[i] = ( value > 0x02 ) ? ( value - 0x02 ) : 0x00;
    }
}

void PatternUpdateBuckets( quint16* newBuckets, quint16* newBucketAverages )
{
    mNewBuckets = 1;

    for( quint8 i=0; i<GLOBAL_NUM_BUCKETS; i++ )
    {
        mPatternBuckets[i] = newBuckets[i];
        mPatternBucketAverages[i] = newBucketAverages[i];
    }

    // Filter for any digital inputs
    quint16 analog1 = mPatternBuckets[2];
    quint16 thresh1 = mPatternBucketAverages[2];
    if( mBucketState[0] )
    {
        if( analog1 < thresh1 )
        {
            mBucketState[0] = 0;
        }
    }
    else
    {
        if( analog1 > thresh1 )
        {
            // New digital input
            mBucketState[0] = 1;

            if( !mDigitalInput1 )
            {
                mDigitalInput1 = 1;
                mDigitalTicks1 = 0;
            }
            else if( !mDigitalInput2 )
            {
                mDigitalInput2 = 1;
                mDigitalTicks2 = 0;
            }
            else
            {
            }
        }
    }
}

void PatternUpdateAd( quint8 mean, quint8 peak )
{
    mNewRaw = 1;

    mRawMean = mean;
    mRawPeak = peak;
}

quint8 WaveAtTick( quint8 tick )
{
    quint8 value = 0;
    switch( tick )
    {
    case 0:
        value = 0x3F;
        break;
    case 1:
        value = 0x7F;
        break;
    case 2:
        value = 0x3F;
        break;
    default:
        break;
    }
    return value;
}

quint8 Abs( quint8 value )
{
    if( value >= 128 )
    {
        return value - 128;
    }
    else
    {
        return 128 - value;
    }
}

