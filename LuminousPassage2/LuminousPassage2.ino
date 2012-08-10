// File: LuminousPassage2

//*****************
// INCLUDES
//*****************

#include <EEPROM.h>

#include "ad.h"
#include "audio.h"
#include "com.h"
#include "comMaster.h"
#include "comSlave.h"
#include "config.h"
#include "fft.h"
#include "global.h"
#include "pattern.h"
#include "pwm.h"
#include "testFft.h"
#include "utility.h"

//*****************
// DEFINITIONS
//*****************

//#define DEBUG


//*****************
// VARIABLES
//*****************

static char mFftDataReal[GLOBAL_NUM_SAMPLES] = {0};
static char mFftDataImag[GLOBAL_NUM_SAMPLES] = {0};
static unsigned char mFftMag[GLOBAL_NUM_SAMPLES/2] = {0};
static unsigned char mFftSum[GLOBAL_NUM_SAMPLES/2] = {0};

static unsigned char mPwmValues[GLOBAL_NUM_CHANNELS] = {0};

static unsigned char mAdTemp = 0;
static unsigned char mAdPeak = 0;
static unsigned long mAdSum = 0;
static unsigned char mAdMean = 0;
static unsigned char mAdOffset = 128;

static unsigned long mFftCount = 0;

static unsigned char mNewFft = 0;
static unsigned char mUpdatePwm = 0;


//*****************
// SETUP
//*****************

void setup( void )
{

    // Init modules
    ComInit();

#if defined( MASTER_SINGLE ) || defined( MASTER )
    AdInit();
    PatternInit();
    AudioInit();
#endif

#if defined( MASTER )
    ComMasterInit();
#endif

//#if !defined( MASTER )
    PwmInit();
//#endif

#if !defined( MASTER_SINGLE ) && !defined( MASTER )
    ComSlaveInit();
#endif

    //TestFft();
    //while(1);

#ifdef DEBUG
    DebugInit();
#endif

}

//*****************
// LOOP
//*****************

void loop( void )
{

#if defined( MASTER_SINGLE ) || defined( MASTER )

    // Check for A/D sample
    AdProcess();
    if( AdReady() )
    {
        // Get ad samples
        AdData( (unsigned char*)mFftDataReal );

        mAdPeak = 0;
        mAdSum = 0;
        for( unsigned int i=0; i<GLOBAL_NUM_SAMPLES; i++ )
        {
            mAdTemp = (unsigned char)mFftDataReal[i];
            if( mAdTemp > mAdPeak )
            {
                mAdPeak = mAdTemp;
            }
            mAdSum += mAdTemp;
        }
        mAdMean = mAdSum / GLOBAL_NUM_SAMPLES;
        mAdOffset = ( mAdOffset >> 1 ) + ( mAdMean >> 1 );

        // Prep fft data
        unsigned char offset = 128;
        for( unsigned int i=0; i<GLOBAL_NUM_SAMPLES; i++ )
        {
            mFftDataReal[i] = (unsigned char)mFftDataReal[i] - offset;
            mFftDataImag[i] = 0;
        }

        // Do Fft
        fix_fft( mFftDataReal, mFftDataImag, 7, 0 );
        Magnitude( mFftMag, mFftDataReal, mFftDataImag, GLOBAL_NUM_SAMPLES/2 );

        mNewFft = 1;
    }

    // Check for any new commands
    if( ComProcess() )
    {
        eCommand command = ComGetCommand();
        switch( command )
        {
        case Command_SAVE:
            PatternSave();
            AudioSave();
            break;
        case Command_LOAD:
            PatternLoad();
            AudioLoad();
            break;
        case Command_AUDIO:
            {
                quint8 threshold = ComGetByte( 0 );
                quint8 averagingFixed = ComGetByte( 1 );
                float averaging = ( (float)averagingFixed ) / 255;
                quint8 lo[GLOBAL_NUM_BUCKETS];
                quint8 hi[GLOBAL_NUM_BUCKETS];
                lo[0] = ComGetByte( 2 );
                hi[0] = ComGetByte( 3 );
                lo[1] = ComGetByte( 4 );
                hi[1] = ComGetByte( 5 );
                lo[2] = ComGetByte( 6 );
                hi[2] = ComGetByte( 7 );
                lo[3] = ComGetByte( 8 );
                hi[3] = ComGetByte( 9 );
                lo[4] = ComGetByte( 10 );
                hi[4] = ComGetByte( 11 );
                lo[5] = ComGetByte( 12 );
                hi[5] = ComGetByte( 13 );
                AudioSetParameters( threshold, averaging, lo, hi );
            }
            break;
        case Command_EFFECT:
            {
                quint8 effect = ComGetByte( 0 );
                quint8 flag = ComGetByte( 1 );
                PatternSetEffect( (eEffect)effect, flag );
            }
            break;
        case Command_BUCKETS:
            {
                quint8 bucketHysteresisFixed = ComGetByte( 0 );
                float bucketHysteresis = ( (float)bucketHysteresisFixed ) / 255;
                quint8 bucketTimeDebounceFlags = ComGetByte( 1 );
                quint8 bucketTimeDebounceSeconds = ComGetByte( 2 );
                PatternSetBucketParameters( bucketHysteresis, bucketTimeDebounceFlags, bucketTimeDebounceSeconds );
            }
            break;
        case Command_MANUAL:
            {
                quint8 value = ComGetByte( 0 );
                PatternSetManual( value );
            }
            break;
        case Command_PULSE_SQUARE:
            {
                quint8 source = ComGetByte( 0 );
                quint8 length = ComGetByte( 1 );
                quint8 width = ComGetByte( 2 );
                PatternSetPulseSquare( source, length, width );
            }
            break;
        case Command_PULSE_SINE:
            {
                quint8 source = ComGetByte( 0 );
                quint8 length = ComGetByte( 1 );
                quint8 width = ComGetByte( 2 );
                PatternSetPulseSine( source, length, width );
            }
            break;
        case Command_DISTANCE_SQUARE:
            {
                quint8 source = ComGetByte( 0 );
                quint8 start = ComGetByte( 1 );
                quint8 stop = ComGetByte( 2 );
                quint8 amp = ComGetByte( 3 );
                PatternSetDistanceSquare( source, start, stop, amp );
            }
            break;
        default:
            break;
        }
    }

    // Update pattern with new fft data
    if( mNewFft )
    {
        mNewFft = 0;
        mFftCount++;
        for( unsigned char i=0; i<GLOBAL_NUM_FREQ; i++ )
        {
            unsigned char value = mFftMag[i];
            mFftSum[i] += ( value < 64 ) ? value : 63;
        }
        if( mFftCount >= 4 )
        {
            mFftCount = 0;

            AudioUpdateFreq( mFftSum );

            PatternUpdateFreq( mFftSum );
            PatternUpdateAd( mAdMean, mAdPeak );
            PatternUpdateBuckets( AudioBuckets(), AudioBucketAverages() );

            ComSendFft( mFftSum );

            for( unsigned char i=0; i<GLOBAL_NUM_FREQ; i ++ )
            {
                mFftSum[i] = 0;
            }

        }
    }

    PatternProcess();
    if( PatternReady() )
    {
        PatternData( mPwmValues );
        mUpdatePwm = 1;
    }

#else // SLAVE

    // Check for new data over i2c
    ComSlaveProcess();
    if( ComSlaveReady() )
    {
        ComSlaveData( mPwmValues );
        mUpdatePwm = 1;
    }

#endif

#if !defined( MASTER )

    if( mUpdatePwm )
    {
        mUpdatePwm = 0;
        PwmSetChannels( mPwmValues );
    }
    PwmProcess();

#else

    if( mUpdatePwm )
    {
        mUpdatePwm = 0;
        PwmSetChannels( mPwmValues );
        ComMasterSendBytes( mPwmValues );
    }

#endif

#ifdef DEBUG
    static unsigned long mDebugPrintTime = millis();
    if( ( millis() - mDebugPrintTime ) > 1000 )
    {
        mDebugPrintTime = millis();

        //ComPrint( "HELLO WORLD\n\r" );

        /*
        // Analog audio
        ComPrint( F(" AdPeak=") );
        ComPrintUchar( mAdPeak );
        ComPrint( F(" AdMean=") );
        ComPrintUchar( mAdMean );
        ComPrint( F(" AdOffset=") );
        ComPrintUchar( mAdOffset );
        ComPrint( F(" FftCounts=") );
        ComPrintUchar( mFftCount );
        */

        /*
        ComPrint( F(" AD Samples: ") );
        for( int i=0; i<32; i++ )
        {
            if( mAdSamples[i] < 16 )
            {
                ComPrintUchar( (unsigned char)'0' );
                ComPrintHex( mAdSamples[i] );
            }
            else
            {
                ComPrintHex( mAdSamples[i] );
            }
            ComPrint( "::" );
        }
        ComPrint( "\n\r" );
        */

        /*
        ComPrintDataUint( " FFT:", mFftMag, 11 );
        //ComPrintDataUint( " FFT4:", mFftFourBuckets, 4 );
        ComPrint( "\n\r" );
        */
    }
#endif

}

