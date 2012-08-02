// File: LuminousPassage2

//*****************
// INCLUDES
//*****************

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
    ComMasterInit();
    AudioInit();
#endif

#if !defined( MASTER )
    PwmInit();
#endif

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
        case Command_EFFECT:
            {
                quint8 effect = ComGetByte( 0 );
                quint8 flag = ComGetByte( 1 );
                PatternSetEffect( (eEffect)effect, flag );
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
        ComMasterSendBytes( mPwmValues );
    }

#endif

#ifdef DEBUG
    static unsigned long mDebugPrintTime = millis();
    if( ( millis() - mDebugPrintTime ) > 1000 )
    {
        mDebugPrintTime = millis();

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

