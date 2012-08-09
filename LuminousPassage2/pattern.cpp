// File: pattern.cpp

//*****************
// INCLUDES
//*****************

#include "pattern.h"

#ifndef SOFTWARE

#include <Arduino.h>
#include <EEPROM.h>

#include "com.h"
#include "config.h"
#include "utility.h"
#include "typedefs.h"

#else

#endif

#include "global.h"
#include "trig.h"

//*****************
// DEFINITIONS
//*****************

#ifdef FIRMWARE
//#define DEBUG
#endif

#define MAX_SIMULTANEOUS        4

#define MIN_BUCKET_THRESHOLD    50


#ifdef SOFTWARE
void cli( void );
void sei( void );
#endif

//*****************
// VARIABLES
//*****************

volatile static qint16 mPatternChannels[GLOBAL_NUM_CHANNELS] = {0};

volatile static quint8 mInterruptCount = 0;

volatile static quint8 mPatternReady = 0;
volatile static quint32 mLastPatternTime = 0;

volatile static quint8 mPatternFrequencies[GLOBAL_NUM_FREQ] = {0};

volatile static quint16 mPatternBuckets[GLOBAL_NUM_BUCKETS] = {0};
volatile static quint16 mPatternBucketAverages[GLOBAL_NUM_BUCKETS] = {0};
volatile static quint8 mPatternBucketSeconds[GLOBAL_NUM_BUCKETS] = {0};

volatile static quint8 mRawMean = 128;
volatile static quint8 mRawPeak = 128;

volatile static quint8 mEffects[Effect_MAX] = {0};

volatile static quint16 mTick = 0;
volatile static quint16 mLastTick[Effect_MAX] = {0};

// Effect_MANUAL
volatile static quint8 mManualValue = 0;

// Effect_CYCLE
volatile static quint8 mCycleCount = 0;

// Effect_BREATH
volatile static quint8 mBreathState = 1;
volatile static quint8 mBreathValue = 0;

// Effect_PULSE_SQUARE
volatile static quint8 mPulseSquareSource = 1;
volatile static quint8 mPulseSquareLength = 40;
volatile static quint8 mPulseSquareWidth = 10;
volatile static quint8 mPulseSquareInput[MAX_SIMULTANEOUS] = {0};
volatile static quint8 mPulseSquareTicks[MAX_SIMULTANEOUS] = {0};

// Effect_PULSE_SINE
volatile static quint8 mPulseSineSource = 1;
volatile static quint8 mPulseSineLength = 40;
volatile static quint8 mPulseSineWidth = 10;
volatile static quint8 mPulseSineInput[MAX_SIMULTANEOUS] = {0};
volatile static quint8 mPulseSineTicks[MAX_SIMULTANEOUS] = {0};

// Effect_DISTANCE
volatile static quint8 mDistanceSquareSource = 2;
volatile static quint8 mDistanceSquareStart = 50;
volatile static quint8 mDistanceSquareStop = 59;
volatile static quint8 mDistanceSquareAmplification = 1;

// Buckets to digital inputs
volatile static quint8 mBucketState[GLOBAL_NUM_BUCKETS] = {0};
volatile static float mBucketHysteresis = 0.25;
volatile static quint8 mBucketTimeDebounceFlags = 0;
volatile static quint8 mBucketTimeDebounceSeconds = 0;

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

#if defined( PATTERN_BLOCKING_VERSION ) && !defined ( SOFTWARE )
    mLastPatternTime = millis();
#endif

#ifndef SOFTWARE
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

void PatternSave( void )
{
#ifdef FIRMWARE
    cli();
    for( quint8 i=0; i<Effect_MAX; i++ )
    {
        EEPROM.write( EEPROM_PATTERN_EFFECT_START + i, mEffects[i] );
    }

    EEPROM.write( EEPROM_PATTERN_BUCKET_HYSTERESIS, (quint8)( 255 * mBucketHysteresis ) );
    EEPROM.write( EEPROM_PATTERN_BUCKET_TIME_FLAGS, mBucketTimeDebounceFlags );
    EEPROM.write( EEPROM_PATTERN_BUCKET_TIME_SECS,  mBucketTimeDebounceSeconds );

    EEPROM.write( EEPROM_PATTERN_PULSE_SQUARE_SOURCE, mPulseSquareSource);
    EEPROM.write( EEPROM_PATTERN_PULSE_SQUARE_LENGTH, mPulseSquareLength);
    EEPROM.write( EEPROM_PATTERN_PULSE_SQUARE_WIDTH, mPulseSquareWidth);

    EEPROM.write( EEPROM_PATTERN_PULSE_SINE_SOURCE, mPulseSineSource );
    EEPROM.write( EEPROM_PATTERN_PULSE_SINE_LENGTH, mPulseSineLength );
    EEPROM.write( EEPROM_PATTERN_PULSE_SINE_WIDTH, mPulseSineWidth );

    EEPROM.write( EEPROM_PATTERN_DISTANCE_SQUARE_SOURCE, mDistanceSquareSource );
    EEPROM.write( EEPROM_PATTERN_DISTANCE_SQUARE_START, mDistanceSquareStart );
    EEPROM.write( EEPROM_PATTERN_DISTANCE_SQUARE_STOP, mDistanceSquareStop );
    EEPROM.write( EEPROM_PATTERN_DISTANCE_SQUARE_AMP, mDistanceSquareAmplification );
    sei();
#endif
}

void PatternLoad( void )
{
#ifdef FIRMWARE
    cli();
    for( quint8 i=0; i<Effect_MAX; i++ )
    {
        mEffects[i] = EEPROM.read( EEPROM_PATTERN_EFFECT_START + i );
    }

    mBucketHysteresis = ( (float)EEPROM.read( EEPROM_PATTERN_BUCKET_HYSTERESIS ) ) / 255;
    mBucketTimeDebounceFlags = EEPROM.read( EEPROM_PATTERN_BUCKET_TIME_FLAGS );
    mBucketTimeDebounceSeconds = EEPROM.read( EEPROM_PATTERN_BUCKET_TIME_SECS );

    mPulseSquareSource = EEPROM.read( EEPROM_PATTERN_PULSE_SQUARE_SOURCE );
    mPulseSquareLength = EEPROM.read( EEPROM_PATTERN_PULSE_SQUARE_LENGTH );
    mPulseSquareWidth = EEPROM.read( EEPROM_PATTERN_PULSE_SQUARE_WIDTH );

    mPulseSineSource = EEPROM.read( EEPROM_PATTERN_PULSE_SINE_SOURCE );
    mPulseSineLength = EEPROM.read( EEPROM_PATTERN_PULSE_SINE_LENGTH );
    mPulseSineWidth = EEPROM.read( EEPROM_PATTERN_PULSE_SINE_WIDTH );

    mDistanceSquareSource = EEPROM.read( EEPROM_PATTERN_DISTANCE_SQUARE_SOURCE );
    mDistanceSquareStart = EEPROM.read( EEPROM_PATTERN_DISTANCE_SQUARE_START );
    mDistanceSquareStop = EEPROM.read( EEPROM_PATTERN_DISTANCE_SQUARE_STOP );
    mDistanceSquareAmplification = EEPROM.read( EEPROM_PATTERN_DISTANCE_SQUARE_AMP );
    sei();
#endif
}

void PatternSetEffect( eEffect effect, quint8 on )
{
    cli();
    mEffects[effect] = on;
    switch( effect )
    {
    case Effect_CYCLE:
        mCycleCount = 0;
        break;
    case Effect_BREATH:
        mBreathState = 0;
        mBreathValue = 0;
        break;
    case Effect_PULSE_SQUARE:
        for( quint8 i=0; i<MAX_SIMULTANEOUS; i++ )
        {
            mPulseSquareInput[i] = 0;
            mPulseSquareTicks[i] = 0;
        }
        break;
    case Effect_PULSE_SINE:
        for( quint8 i=0; i<MAX_SIMULTANEOUS; i++ )
        {
            mPulseSineInput[i] = 0;
            mPulseSineTicks[i] = 0;
        }
        break;
    case Effect_DISTANCE_SQUARE:
        break;
    default:
        break;
    }
    sei();
}

void PatternSetBucketParameters( float bucketHysteresis, quint8 bucketTimeDebounceFlags, quint8 bucketTimeDebounceSeconds )
{
    mBucketHysteresis = bucketHysteresis;
    mBucketTimeDebounceFlags = bucketTimeDebounceFlags;
    mBucketTimeDebounceSeconds = bucketTimeDebounceSeconds;
}

void PatternSetManual( quint8 value )
{
    cli();
    mManualValue = value;
    sei();
}

void PatternSetPulseSquare( quint8 source, quint8 length, quint8 width )
{
    cli();
    mPulseSquareSource = source;
    mPulseSquareLength = length;
    mPulseSquareWidth = width;
    sei();
}

void PatternSetPulseSine( quint8 source, quint8 length, quint8 width )
{
    cli();
    mPulseSineSource = source;
    mPulseSineLength = length;
    mPulseSineWidth = width;
    sei();
}

void PatternSetDistanceSquare( quint8 source, quint8 start, quint8 stop, quint8 amp )
{
    mDistanceSquareSource = source;
    mDistanceSquareStart = start;
    mDistanceSquareStop = stop;
    mDistanceSquareAmplification = amp;
}

quint8 PatternReady( void )
{
    return mPatternReady;
}

void PatternData( quint8* data )
{
    cli();
    for( quint8 i=0; i<GLOBAL_NUM_CHANNELS; i++ )
    {
        qint16 value = mPatternChannels[i];
        if( value > 0xFF )
        {
            data[i] = 0xFF;
        }
        else if( value < 0x00 )
        {
            data[i] = 0x00;
        }
        else
        {
            data[i] = (quint8)value;
        }
    }
    sei();

    mPatternReady = 0;
}

void PatternProcess( void )
{
#if defined( PATTERN_BLOCKING_VERSION ) && !defined( SOFTWARE )
    if( ( micros() - mLastPatternTime ) >= 10000 )
    {
        mLastPatternTime += 10000;
        PatternProcessInternal();
    }
#endif

#ifdef SOFTWARE
    PatternProcessInternal();
#endif
}

void PatternUpdateFreq( quint8* newFrequencies )
{
    cli();
    // Filter for noise
    for( quint8 i=0; i<GLOBAL_NUM_FREQ; i++ )
    {
        quint8 value = newFrequencies[i];
        mPatternFrequencies[i] = ( value > 0x02 ) ? ( value - 0x02 ) : 0x00;
    }
    sei();
}

void PatternUpdateBuckets( quint16* newBuckets, quint16* newBucketAverages )
{
    cli();

    for( quint8 i=0; i<GLOBAL_NUM_BUCKETS; i++ )
    {
        mPatternBuckets[i] = newBuckets[i];
        mPatternBucketAverages[i] = newBucketAverages[i];
    }

    // Filter for any digital inputs
    for( quint8 i=0; i<GLOBAL_NUM_BUCKETS; i++ )
    {
        quint16 avg = mPatternBucketAverages[i];
        quint16 value = mPatternBuckets[i];
        quint16 hi = avg * ( 1.0 + mBucketHysteresis );
        quint16 lo = avg * ( 1.0 - mBucketHysteresis );
        quint8 flag = ( ( 0x01 << i ) & mBucketTimeDebounceFlags );
        quint8 seconds = mPatternBucketSeconds[i];
        if( mBucketState[i] )
        {
            if( ( value < lo ) || ( avg < MIN_BUCKET_THRESHOLD ) )
            {
                mBucketState[i] = 0;
            }
        }
        else
        {
            if( ( avg > MIN_BUCKET_THRESHOLD ) &&                           // AVerage value must be least some minimum
                ( value > hi ) &&                                           // Current value must be higher than threshold
                ( !flag || ( seconds >= mBucketTimeDebounceSeconds ) ) )    // Either no time debounce or debounce expired
            {
                mBucketState[i] = 1;
                mPatternBucketSeconds[i] = 0;

                // Find open pulse square input
                if( mPulseSquareSource == i )
                {
                    for( quint8 j=0; j<MAX_SIMULTANEOUS; j++ )
                    {
                        if( !mPulseSquareInput[j] )
                        {
                            mPulseSquareInput[j] = 1;
                            mPulseSquareTicks[j] = 0;
                            break;
                        }
                    }
                }

                // Find open pulse sine input
                if( mPulseSineSource == i )
                {
                    for( quint8 j=0; j<MAX_SIMULTANEOUS; j++ )
                    {
                        if( !mPulseSineInput[j] )
                        {
                            mPulseSineInput[j] = 1;
                            mPulseSineTicks[j] = 0;
                            break;
                        }
                    }
                }

            }
        }
    }

    sei();
}

void PatternUpdateAd( quint8 mean, quint8 peak )
{
    cli();
    mRawMean = mean;
    mRawPeak = peak;
    sei();
}

void PatternProcessInternal( void )
{
    mPatternReady = 1;
    mTick++;

    // Increment seconds on each bucket
    if( ( mTick % 100 ) == 0 )
    {
        for( quint8 i=0; i<GLOBAL_NUM_BUCKETS; i++ )
        {
            if( mPatternBucketSeconds[i] < 0xFF )
            {
                mPatternBucketSeconds[i]++;
            }
        }
    }

    // Reset channels
    for( quint8 i=0; i<GLOBAL_NUM_CHANNELS; i++ )
    {
        mPatternChannels[i] = 0;
    }

    // Effect_NULL
    if( mEffects[Effect_NULL] )
    {
        for( quint8 i=0; i<GLOBAL_NUM_CHANNELS; i++ )
        {
            mPatternChannels[i] += 0x80;
        }
    }

    // Effect_MANUAL
    if( mEffects[Effect_MANUAL] )
    {
        for( quint8 i=0; i<GLOBAL_NUM_CHANNELS; i++ )
        {
            mPatternChannels[i] += mManualValue;
        }
    }

    // Effect_CYCLE
    if( mEffects[Effect_CYCLE] )
    {
        quint16 diff = mTick - mLastTick[Effect_CYCLE];
        if( diff >= (quint16)10 )
        {
            mLastTick[Effect_CYCLE] = mTick;
            if( ++mCycleCount >= GLOBAL_NUM_CHANNELS )
            {
                mCycleCount = 0;
            }
        }
        mPatternChannels[mCycleCount] += 0xFF;
    }

    // Effect_BREATH
    if( mEffects[Effect_BREATH] )
    {
        quint16 diff = mTick - mLastTick[Effect_BREATH];
        if( diff >= (quint16)5 )
        {
            mLastTick[Effect_BREATH] = mTick;
            if( mBreathState )
            {
                mBreathValue += 0x08;
                mBreathState = ( mBreathValue > 0xF7 ) ? 0 : 1;
            }
            else
            {
                mBreathValue -= 0x08;
                mBreathState = ( mBreathValue < 0x07 ) ? 1 : 0;
            }
        }
        for( quint8 i=0; i<GLOBAL_NUM_CHANNELS; i++ )
        {
            mPatternChannels[i] += mBreathValue;
        }
    }

    // Effect_FREQ
    if( mEffects[Effect_FREQ] )
    {
        for( quint8 i=0; i<GLOBAL_NUM_CHANNELS; i++ )
        {
            quint16 raw = mPatternFrequencies[i] * 4;
            mPatternChannels[i] += raw;
        }
    }

    // Effect_BUCKETS
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

    // Effect_PULSE_SQUARE
    if( mEffects[Effect_PULSE_SQUARE] )
    {
        quint16 diff = mTick - mLastTick[Effect_PULSE_SQUARE];
        if( diff >= (quint16)5 )
        {
            mLastTick[Effect_PULSE_SQUARE] = mTick;
            for( quint8 i=0; i<MAX_SIMULTANEOUS; i++ )
            {
                if( mPulseSquareInput[i] )
                {
                    if( ++mPulseSquareTicks[i] > ( mPulseSquareLength + mPulseSquareWidth ) )
                    {
                        mPulseSquareTicks[i] = 0;
                        mPulseSquareInput[i] = 0;
                    }
                }
            }
        }

        for( quint8 i=0; i<MAX_SIMULTANEOUS; i++ )
        {
            if( mPulseSquareInput[i] )
            {
                quint8 tick = mPulseSquareTicks[i];
                for( quint8 j=0; j<mPulseSquareLength; j++ )
                {
                    qint8 distance = tick - j;
                    if( ( distance >= 0 ) && ( distance < mPulseSquareWidth ) )
                    {
                        mPatternChannels[j] += 0x7F;
                    }
                }
            }
        }
    }

    // Effect_PULSE_SINE
    if( mEffects[Effect_PULSE_SINE] )
    {
        quint16 diff = mTick - mLastTick[Effect_PULSE_SINE];
        if( diff >= (quint16)5 )
        {
            mLastTick[Effect_PULSE_SINE] = mTick;
            for( quint8 i=0; i<MAX_SIMULTANEOUS; i++ )
            {
                if( mPulseSineInput[i] )
                {
                    if( ++mPulseSineTicks[i] > ( mPulseSineLength + mPulseSineWidth ) )
                    {
                        mPulseSineTicks[i] = 0;
                        mPulseSineInput[i] = 0;
                    }
                }
            }
        }

        for( quint8 i=0; i<MAX_SIMULTANEOUS; i++ )
        {
            if( mPulseSineInput[i] )
            {
                quint8 tick = mPulseSineTicks[i];
                for( quint8 j=0; j<mPulseSineLength; j++ )
                {
                    qint8 distance = tick - j;
                    if( ( distance >= 0 ) && ( distance <= mPulseSineWidth ) )
                    {
                        float percent = (float)distance / mPulseSineWidth;
                        quint8 sinIndex = 128 * percent;
                        quint8 sinValue = Sin( sinIndex );
                        mPatternChannels[j] += sinValue;
                    }
                }
            }
        }
    }

    // Effect_DISTANCE_SQUARE
    if( mEffects[Effect_DISTANCE_SQUARE] )
    {
        qint16 raw = mPatternBuckets[mDistanceSquareSource];
        qint16 avg = mPatternBucketAverages[mDistanceSquareSource];
        qint16 value = ( raw - avg ) * (qint8)mDistanceSquareAmplification;
        for( quint8 i=mDistanceSquareStart; i<mDistanceSquareStop; i++ )
        {
            mPatternChannels[i] += value;
        }
    }
}

#if defined( PATTERN_ISR_VERSION ) && !defined( SOFTWARE )
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

#ifdef SOFTWARE
void cli( void )
{
}
void sei( void )
{
}
#endif
