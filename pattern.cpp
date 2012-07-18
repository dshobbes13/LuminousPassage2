// File: pattern.cpp

//*****************
// INCLUDES
//*****************

#include "pattern.h"

#include <Arduino.h>

#include "utility.h"


//*****************
// DEFINITIONS
//*****************

//#define DEBUG

enum eMode
{
    Mode_NULL            = 0,
    Mode_AUTO_SWEEP      = 1,
    Mode_CONTROL_SWEEP   = 2,
    Mode_MUSIC           = 3,
    Mode_DEBUG_FREQ      = 4,
    Mode_DIGITAL_PATTERN = 5,
    Mode_WAVE_FADE       = 6,
    Mode_TWO_PATTERNS    = 7,
    Mode_AD_MEAN         = 8,
    Mode_AD_PEAK         = 9,
    Mode_CYCLE           = 10,
};

//*****************
// VARIABLES
//*****************

static unsigned char mPatternChannels[PATTERN_NUM_CHANNELS];

static unsigned char mChannelMap[PATTERN_NUM_CHANNELS] = {
    4, // 0
    1, // 1
    5, // 2
    0, // 3
    6, // 4
    2, // 5
    7, // 6
    3, // 7
    8, // 8
    9, // 9
};

static unsigned char mNewRaw = 0;
static unsigned char mRawMean = 128;
static unsigned char mRawPeak = 128;

static unsigned char mNewFrequencies = 0;
static unsigned int mPatternFrequencies[PATTERN_NUM_FREQ];

static unsigned long mSweepTime = 0;
static unsigned char mSweepState = 1;
static unsigned char mSweepValue = 0;

static unsigned int mAnalogValue1 = 0x00;
static unsigned int mAnalogState1 = 0x00;
static unsigned int mAnalogThresh1 = 0x10;

static unsigned char mDigitalFreq1 = 0;
static unsigned char mDigitalFreq2 = 0;

static unsigned char mDigitalInput1 = 0;
static unsigned char mDigitalTicks1 = 0;

static unsigned char mDigitalInput2 = 0;
static unsigned char mDigitalTicks2 = 0;

#define NUM_WAVES 2
static unsigned char mDigitalFadeInput = 0;
static unsigned char mDigitalFadeTicks = 0;
static unsigned char mDigitalFadeTicksStart[NUM_WAVES] = {0};

static unsigned long mDigitalPatternTime = 0;

static unsigned char mCycleCount = 0;

//*****************
// PRIVATE PROTOTYPES
//*****************

unsigned char WaveAtTick( unsigned char tick );
unsigned char Abs( unsigned char value );


//*****************
// PUBLIC
//*****************

void PatternInit( void )
{
}

unsigned char PatternProcess( void )
{
    static unsigned char mUpdatedChannels = 0;
    static unsigned char mCommandReceived = 0;
    static unsigned int mCommand = 0;
    static eMode mMode = Mode_NULL;

    // Check for any new commands
    if( Serial.available() )
    {
        mCommandReceived = 1;
        mCommand = Serial.read();
    }

    // Process mode commands
    if( mCommandReceived )
    {
        if( ( ( mCommand >= '0' ) && ( mCommand <= '9' ) ) || ( mCommand == 'z') )
        {
            mCommandReceived = 0;
            switch( mCommand )
            {
            case '0':
                mMode = Mode_NULL;
                break;
            case '1':
                mMode = Mode_AUTO_SWEEP;
                mSweepTime = millis();
                break;
            case '2':
                mMode = Mode_CONTROL_SWEEP;
                mSweepTime = millis();
                break;
            case '3':
                mMode = Mode_MUSIC;
                break;
            case '4':
                mMode = Mode_DEBUG_FREQ;
                break;
            case '5':
                mMode = Mode_DIGITAL_PATTERN;
                mDigitalPatternTime = millis();
                break;
            case '6':
                mMode = Mode_WAVE_FADE;
                mDigitalPatternTime = millis();
                mDigitalFadeTicks = 0;
                break;
            case '7':
                mMode = Mode_TWO_PATTERNS;
                mDigitalPatternTime = millis();
                break;
            case '8':
                mMode = Mode_AD_MEAN;
                break;
            case '9':
                mMode = Mode_AD_PEAK;
                break;
            case 'z':
                mMode = Mode_CYCLE;
                mCycleCount = 0;
                mSweepTime = millis();
                break;
            default:
                break;
            }
        }
    }

    switch( mMode )
    {

    case Mode_AUTO_SWEEP:
        if( millis() - mSweepTime > 20 )
        {
            mUpdatedChannels = 1;
            mSweepTime = millis();
            if( mSweepState )
            {
                mSweepValue += 0x08;
                for( unsigned char i=0; i<PATTERN_NUM_CHANNELS; i++ )
                {
                    mPatternChannels[i] = mSweepValue;
                }
                if( mSweepValue > 0xF7 )
                {
                    mSweepState = 0;
                }
            }
            else
            {
                mSweepValue -= 0x08;
                for( unsigned char i=0; i<PATTERN_NUM_CHANNELS; i++ )
                {
                    mPatternChannels[i] = mSweepValue;
                }
                if( mSweepValue < 0x07 )
                {
                    mSweepState = 1;
                }
            }
        }
        break;

    case Mode_CONTROL_SWEEP:
        if( mCommandReceived )
        {
            mCommandReceived = 0;
            mUpdatedChannels = 1;
            if( mCommand == 'u' )
            {
                mSweepValue += 0x08;
                for( unsigned char i=0; i<PATTERN_NUM_CHANNELS; i++ )
                {
                    mPatternChannels[i] = mSweepValue;
                }
            }
            else if( mCommand == 'd' )
            {
                mSweepValue -= 0x08;
                for( unsigned char i=0; i<PATTERN_NUM_CHANNELS; i++ )
                {
                    mPatternChannels[i] = mSweepValue;
                }
            }
            else
            {
            }
        }
        break;

    case Mode_MUSIC:
        if( mNewFrequencies )
        {
            mUpdatedChannels = 1;
            mNewFrequencies = 0;
    
            // Consolidate into 2 buckets
            static unsigned int lo;
            static unsigned int hi;
            lo = 0;
            for( unsigned char i=1; i<4; i++ )
            {
                lo += mPatternFrequencies[i];
            }
            hi = 0;
            for( unsigned char i=4; i<15; i++ )
            {
                hi += mPatternFrequencies[i];
            }
            mPatternChannels[0] = ( lo <= 0xFF ) ? lo : 0xFF;
            mPatternChannels[1] = ( lo <= 0xFF ) ? lo : 0xFF;
            mPatternChannels[2] = ( lo <= 0xFF ) ? lo : 0xFF;
            mPatternChannels[3] = ( lo <= 0xFF ) ? lo : 0xFF;
            mPatternChannels[4] = ( hi <= 0xFF ) ? hi : 0xFF;
            mPatternChannels[5] = ( hi <= 0xFF ) ? hi : 0xFF;
            mPatternChannels[6] = ( hi <= 0xFF ) ? hi : 0xFF;
            mPatternChannels[7] = ( hi <= 0xFF ) ? hi : 0xFF;
            mPatternChannels[8] = ( hi <= 0xFF ) ? hi : 0xFF;
            mPatternChannels[9] = ( hi <= 0xFF ) ? hi : 0xFF;
        }
        break;

    case Mode_DEBUG_FREQ:
        if( mNewFrequencies )
        {
            mUpdatedChannels = 1;
            mNewFrequencies = 0;
            
            for( unsigned char i=0; i<10; i++ )
            {
                unsigned int raw = mPatternFrequencies[i] * 4;
                mPatternChannels[i] = ( raw <= 0xFF ) ? raw : 0xFF;
            }
        }
        break;

    case Mode_DIGITAL_PATTERN:
        if( mCommandReceived )
        {
            mCommandReceived = 0;
            mDigitalFreq1 = mCommand - 'a';
        }
        if( ( millis() - mDigitalPatternTime > 10 ) && ( mDigitalInput1 || mDigitalInput2 ) )
        {
            mDigitalPatternTime = millis();
            mUpdatedChannels = 1;

            mPatternChannels[4] = 0;
            mPatternChannels[1] = 0;
            mPatternChannels[5] = 0;
            mPatternChannels[0] = 0;
            mPatternChannels[6] = 0;
            mPatternChannels[2] = 0;
            mPatternChannels[7] = 0;
            mPatternChannels[3] = 0;
            mPatternChannels[8] = 0;
            mPatternChannels[9] = 0;

            // 1
            if( mDigitalInput1 )
            {
                mPatternChannels[4] += ( mDigitalTicks1 == 0 || mDigitalTicks1 == 1 ) ? 0x7F : 0x00;
                mPatternChannels[1] += ( mDigitalTicks1 == 1 || mDigitalTicks1 == 2 ) ? 0x7F : 0x00;
                mPatternChannels[5] += ( mDigitalTicks1 == 2 || mDigitalTicks1 == 3 ) ? 0x7F : 0x00;
                mPatternChannels[0] += ( mDigitalTicks1 == 3 || mDigitalTicks1 == 4 ) ? 0x7F : 0x00;
                mPatternChannels[6] += ( mDigitalTicks1 == 4 || mDigitalTicks1 == 5 ) ? 0x7F : 0x00;
                mPatternChannels[2] += ( mDigitalTicks1 == 5 || mDigitalTicks1 == 6 ) ? 0x7F : 0x00;
                mPatternChannels[7] += ( mDigitalTicks1 == 6 || mDigitalTicks1 == 7 ) ? 0x7F : 0x00;
                mPatternChannels[3] += ( mDigitalTicks1 == 7 || mDigitalTicks1 == 8 ) ? 0x7F : 0x00;
                mPatternChannels[8] += ( mDigitalTicks1 == 8 || mDigitalTicks1 == 9 ) ? 0x7F : 0x00;
                mPatternChannels[9] += ( mDigitalTicks1 == 9 || mDigitalTicks1 == 10 ) ? 0x7F : 0x00;
                if( ++mDigitalTicks1 > 11 )
                {
                    mDigitalTicks1 = 0;
                    mDigitalInput1 = 0;
                }
            }

            // 2
            if( mDigitalInput2 )
            {
                mPatternChannels[4] += ( mDigitalTicks2 == 0 || mDigitalTicks2 == 1 ) ? 0x7F : 0x00;
                mPatternChannels[1] += ( mDigitalTicks2 == 1 || mDigitalTicks2 == 2 ) ? 0x7F : 0x00;
                mPatternChannels[5] += ( mDigitalTicks2 == 2 || mDigitalTicks2 == 3 ) ? 0x7F : 0x00;
                mPatternChannels[0] += ( mDigitalTicks2 == 3 || mDigitalTicks2 == 4 ) ? 0x7F : 0x00;
                mPatternChannels[6] += ( mDigitalTicks2 == 4 || mDigitalTicks2 == 5 ) ? 0x7F : 0x00;
                mPatternChannels[2] += ( mDigitalTicks2 == 5 || mDigitalTicks2 == 6 ) ? 0x7F : 0x00;
                mPatternChannels[7] += ( mDigitalTicks2 == 6 || mDigitalTicks2 == 7 ) ? 0x7F : 0x00;
                mPatternChannels[3] += ( mDigitalTicks2 == 7 || mDigitalTicks2 == 8 ) ? 0x7F : 0x00;
                mPatternChannels[8] += ( mDigitalTicks2 == 8 || mDigitalTicks2 == 9 ) ? 0x7F : 0x00;
                mPatternChannels[9] += ( mDigitalTicks2 == 9 || mDigitalTicks2 == 10 ) ? 0x7F : 0x00;
                if( ++mDigitalTicks2 > 11 )
                {
                    mDigitalTicks2 = 0;
                    mDigitalInput2 = 0;
                }
            }
        }
        break;

    case Mode_WAVE_FADE:
        if( mCommandReceived )
        {
            mCommandReceived = 0;
            mDigitalFreq1 = mCommand - 'a';
        }
        if( ( millis() - mDigitalPatternTime > 10 ) && ( mDigitalFadeInput & 0x03 ) )
        {
            mDigitalPatternTime = millis();
            mUpdatedChannels = 1;

            mPatternChannels[4] = 0;
            mPatternChannels[1] = 0;
            mPatternChannels[5] = 0;
            mPatternChannels[0] = 0;
            mPatternChannels[6] = 0;
            mPatternChannels[2] = 0;
            mPatternChannels[7] = 0;
            mPatternChannels[3] = 0;
            mPatternChannels[8] = 0;
            mPatternChannels[9] = 0;

            for( unsigned char i=0; i<NUM_WAVES; i++ )
            {
                if( mDigitalFadeInput & ( 0x01 << i ) )
                {
                    mPatternChannels[4] += WaveAtTick( mDigitalFadeTicks - mDigitalFadeTicksStart[i] - 0 );
                    mPatternChannels[1] += WaveAtTick( mDigitalFadeTicks - mDigitalFadeTicksStart[i] - 1 );
                    mPatternChannels[5] += WaveAtTick( mDigitalFadeTicks - mDigitalFadeTicksStart[i] - 2 );
                    mPatternChannels[0] += WaveAtTick( mDigitalFadeTicks - mDigitalFadeTicksStart[i] - 3 );
                    mPatternChannels[6] += WaveAtTick( mDigitalFadeTicks - mDigitalFadeTicksStart[i] - 4 );
                    mPatternChannels[1] += WaveAtTick( mDigitalFadeTicks - mDigitalFadeTicksStart[i] - 5 );
                    mPatternChannels[7] += WaveAtTick( mDigitalFadeTicks - mDigitalFadeTicksStart[i] - 6 );
                    mPatternChannels[3] += WaveAtTick( mDigitalFadeTicks - mDigitalFadeTicksStart[i] - 7 );
                    mPatternChannels[8] += WaveAtTick( mDigitalFadeTicks - mDigitalFadeTicksStart[i] - 8 );
                    mPatternChannels[9] += WaveAtTick( mDigitalFadeTicks - mDigitalFadeTicksStart[i] - 9 );
                }
            }

            if( ( mDigitalFadeTicks % 6 ) == 0 )
            {
                mDigitalFadeInput = mDigitalFadeInput << 1;
                for( unsigned char i=(NUM_WAVES-1); i>0; i-- )
                {
                    mDigitalFadeTicksStart[i] = mDigitalFadeTicksStart[i-1];
                }
            }

            mDigitalFadeTicks++;
        }
        break;

    case Mode_TWO_PATTERNS:
        if( mCommandReceived )
        {
            mCommandReceived = 0;
            mDigitalFreq1 = mCommand - 'a';
        }

        if( mNewFrequencies )
        {
            mUpdatedChannels = 1;
            mNewFrequencies = 0;

            static unsigned int lo;
            lo = 0;
            for( unsigned char i=1; i<4; i++ )
            {
                lo += mPatternFrequencies[i];
            }
            mPatternChannels[4] = ( lo <= 0xFF ) ? lo : 0xFF;
            mPatternChannels[1] = ( lo <= 0xFF ) ? lo : 0xFF;
            mPatternChannels[5] = ( lo <= 0xFF ) ? lo : 0xFF;
            mPatternChannels[0] = ( lo <= 0xFF ) ? lo : 0xFF;
        }

        if( ( millis() - mDigitalPatternTime > 10 ) && ( mDigitalInput1 || mDigitalInput2 ) )
        {
            mDigitalPatternTime = millis();
            mUpdatedChannels = 1;

            mPatternChannels[6] = 0;
            mPatternChannels[2] = 0;
            mPatternChannels[7] = 0;
            mPatternChannels[3] = 0;
            mPatternChannels[8] = 0;
            mPatternChannels[9] = 0;

            // 1
            if( mDigitalInput1 )
            {
                mPatternChannels[6] += ( mDigitalTicks1 == 0 || mDigitalTicks1 == 1 ) ? 0x7F : 0x00;
                mPatternChannels[2] += ( mDigitalTicks1 == 1 || mDigitalTicks1 == 2 ) ? 0x7F : 0x00;
                mPatternChannels[7] += ( mDigitalTicks1 == 2 || mDigitalTicks1 == 3 ) ? 0x7F : 0x00;
                mPatternChannels[3] += ( mDigitalTicks1 == 3 || mDigitalTicks1 == 4 ) ? 0x7F : 0x00;
                mPatternChannels[8] += ( mDigitalTicks1 == 4 || mDigitalTicks1 == 5 ) ? 0x7F : 0x00;
                mPatternChannels[9] += ( mDigitalTicks1 == 5 || mDigitalTicks1 == 6 ) ? 0x7F : 0x00;
                if( ++mDigitalTicks1 > 7 )
                {
                    mDigitalTicks1 = 0;
                    mDigitalInput1 = 0;
                }
            }

            // 2
            if( mDigitalInput2 )
            {
                mPatternChannels[6] += ( mDigitalTicks2 == 0 || mDigitalTicks2 == 1 ) ? 0x7F : 0x00;
                mPatternChannels[2] += ( mDigitalTicks2 == 1 || mDigitalTicks2 == 2 ) ? 0x7F : 0x00;
                mPatternChannels[7] += ( mDigitalTicks2 == 2 || mDigitalTicks2 == 3 ) ? 0x7F : 0x00;
                mPatternChannels[3] += ( mDigitalTicks2 == 3 || mDigitalTicks2 == 4 ) ? 0x7F : 0x00;
                mPatternChannels[8] += ( mDigitalTicks2 == 4 || mDigitalTicks2 == 5 ) ? 0x7F : 0x00;
                mPatternChannels[9] += ( mDigitalTicks2 == 5 || mDigitalTicks2 == 6 ) ? 0x7F : 0x00;
                if( ++mDigitalTicks2 > 7 )
                {
                    mDigitalTicks2 = 0;
                    mDigitalInput2 = 0;
                }
            }
        }
        break;

    case Mode_AD_MEAN:
        if( mNewRaw )
        {
            mNewRaw = 0;
            mUpdatedChannels = 1;

            unsigned char absMean = Abs( mRawMean );
            mPatternChannels[4] = ( absMean > 0 ) ? 0xFF : 0x00;
            mPatternChannels[1] = ( absMean > 1 ) ? 0xFF : 0x00;
            mPatternChannels[5] = ( absMean > 2 ) ? 0xFF : 0x00;
            mPatternChannels[0] = ( absMean > 3 ) ? 0xFF : 0x00;
            mPatternChannels[6] = ( absMean > 4 ) ? 0xFF : 0x00;
            mPatternChannels[2] = ( absMean > 5 ) ? 0xFF : 0x00;
            mPatternChannels[7] = ( absMean > 6 ) ? 0xFF : 0x00;
            mPatternChannels[3] = ( absMean > 7 ) ? 0xFF : 0x00;
            mPatternChannels[8] = ( absMean > 8 ) ? 0xFF : 0x00;
            mPatternChannels[9] = ( absMean > 9 ) ? 0xFF : 0x00;
        }
        break;

    case Mode_AD_PEAK:
        if( mNewRaw )
        {
            mNewRaw = 0;
            mUpdatedChannels = 1;

            mPatternChannels[4] = ( mRawPeak > 0xF5 ) ? 0xFF : 0x00;
            mPatternChannels[1] = ( mRawPeak > 0xF6 ) ? 0xFF : 0x00;
            mPatternChannels[5] = ( mRawPeak > 0xF7 ) ? 0xFF : 0x00;
            mPatternChannels[0] = ( mRawPeak > 0xF8 ) ? 0xFF : 0x00;
            mPatternChannels[6] = ( mRawPeak > 0xF9 ) ? 0xFF : 0x00;
            mPatternChannels[2] = ( mRawPeak > 0xFA ) ? 0xFF : 0x00;
            mPatternChannels[7] = ( mRawPeak > 0xFB ) ? 0xFF : 0x00;
            mPatternChannels[3] = ( mRawPeak > 0xFC ) ? 0xFF : 0x00;
            mPatternChannels[8] = ( mRawPeak > 0xFD ) ? 0xFF : 0x00;
            mPatternChannels[9] = ( mRawPeak > 0xFE ) ? 0xFF : 0x00;
        }
        break;

    case Mode_CYCLE:
        if( millis() - mSweepTime > 1000 )
        {
            mUpdatedChannels = 1;
            mSweepTime = millis();

            mPatternChannels[4] = ( mCycleCount == 0 ) ? 0xFF : 0x00;
            mPatternChannels[1] = ( mCycleCount == 1 ) ? 0xFF : 0x00;
            mPatternChannels[5] = ( mCycleCount == 2 ) ? 0xFF : 0x00;
            mPatternChannels[0] = ( mCycleCount == 3 ) ? 0xFF : 0x00;
            mPatternChannels[6] = ( mCycleCount == 4 ) ? 0xFF : 0x00;
            mPatternChannels[2] = ( mCycleCount == 5 ) ? 0xFF : 0x00;
            mPatternChannels[7] = ( mCycleCount == 6 ) ? 0xFF : 0x00;
            mPatternChannels[3] = ( mCycleCount == 7 ) ? 0xFF : 0x00;
            mPatternChannels[8] = ( mCycleCount == 8 ) ? 0xFF : 0x00;
            mPatternChannels[9] = ( mCycleCount == 9 ) ? 0xFF : 0x00;

            if( ++mCycleCount >= 10 )
            {
                mCycleCount = 0;
            }
        }
        break;

    default:
        break;
    }

    return mUpdatedChannels;
}

unsigned char* PatternData( void )
{
    return mPatternChannels;
}

void PatternUpdateFreq( unsigned int* newFrequencies )
{
    mNewFrequencies = 1;
    
    // Filter for noise
    mPatternFrequencies[0]  = ( newFrequencies[0]  > 0x10 ) ? ( newFrequencies[0]  - 0x10 ) * 0x01 : 0x00;
    mPatternFrequencies[1]  = ( newFrequencies[1]  > 0x08 ) ? ( newFrequencies[1]  - 0x08 ) * 0x01 : 0x00;
    mPatternFrequencies[2]  = ( newFrequencies[2]  > 0x04 ) ? ( newFrequencies[2]  - 0x04 ) * 0x02 : 0x00;
    mPatternFrequencies[3]  = ( newFrequencies[3]  > 0x02 ) ? ( newFrequencies[3]  - 0x02 ) * 0x04 : 0x00;
    mPatternFrequencies[4]  = ( newFrequencies[4]  > 0x02 ) ? ( newFrequencies[4]  - 0x02 ) * 0x06 : 0x00;
    mPatternFrequencies[5]  = ( newFrequencies[5]  > 0x02 ) ? ( newFrequencies[5]  - 0x02 ) * 0x06 : 0x00;
    mPatternFrequencies[6]  = ( newFrequencies[6]  > 0x01 ) ? ( newFrequencies[6]  - 0x01 ) * 0x06 : 0x00;
    mPatternFrequencies[7]  = ( newFrequencies[7]  > 0x01 ) ? ( newFrequencies[7]  - 0x01 ) * 0x06 : 0x00;
    mPatternFrequencies[8]  = ( newFrequencies[8]  > 0x01 ) ? ( newFrequencies[8]  - 0x01 ) * 0x08 : 0x00;
    mPatternFrequencies[9]  = ( newFrequencies[9]  > 0x01 ) ? ( newFrequencies[9]  - 0x01 ) * 0x08 : 0x00;
    mPatternFrequencies[10] = ( newFrequencies[10] > 0x01 ) ? ( newFrequencies[10] - 0x01 ) * 0x08 : 0x00;
    mPatternFrequencies[11] = ( newFrequencies[11] > 0x01 ) ? ( newFrequencies[11] - 0x01 ) * 0x08 : 0x00;
    mPatternFrequencies[12] = ( newFrequencies[12] > 0x01 ) ? ( newFrequencies[12] - 0x01 ) * 0x08 : 0x00;
    mPatternFrequencies[13] = ( newFrequencies[13] > 0x01 ) ? ( newFrequencies[13] - 0x01 ) * 0x08 : 0x00;
    mPatternFrequencies[14] = ( newFrequencies[14] > 0x01 ) ? ( newFrequencies[14] - 0x01 ) * 0x08 : 0x00;
    mPatternFrequencies[15] = ( newFrequencies[15] > 0x01 ) ? ( newFrequencies[15] - 0x01 ) * 0x08 : 0x00;

    // Filter for any digital inputs
    mAnalogValue1 = mPatternFrequencies[mDigitalFreq1];
    if( mAnalogState1 )
    {
        if( mAnalogValue1 < mAnalogThresh1 )
        {
            mAnalogState1 = 0;
        }
    }
    else
    {
        if( mAnalogValue1 > mAnalogThresh1 )
        {
            // New digital input
            mAnalogState1 = 1;

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

            if( !( mDigitalFadeInput & 0x01 ) )
            {
                mDigitalFadeInput |= 0x01;
                mDigitalFadeTicksStart[0] = mDigitalFadeTicks;
            }
        }
    }
}

void PatternUpdateRaw( unsigned char mean, unsigned char peak )
{
    mNewRaw = 1;

    mRawMean = mean;
    mRawPeak = peak;
}

unsigned char WaveAtTick( unsigned char tick )
{
    unsigned char value = 0;
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

unsigned char Abs( unsigned char value )
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

