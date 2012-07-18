// File: LuminousPassage2

//*****************
// INCLUDES
//*****************

#include "ad.h"
#include "fft.h"
#include "pattern.h"
#include "pwm.h"
#include "comMaster.h"
#include "testFft.h"
#include "utility.h"

//*****************
// DEFINITIONS
//*****************

#define DEBUG


//*****************
// VARIABLES
//*****************

static char mFftDataReal[AD_NUM_SAMPLES];
static char mFftDataImag[AD_NUM_SAMPLES];
static unsigned int mFftMag[AD_NUM_SAMPLES];

static unsigned char mPwmValues[PWM_NUM_CHANNELS] = {0};

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
    // Init serial port for debugging
    Serial.begin( 115200 );
    while( !Serial );

    // Init modules
    AdInit();
    PatternInit();
    PwmInit();
    ComMasterInit();

    //TestFft();
}

//*****************
// LOOP
//*****************

void loop( void )
{
    // Check for A/D sample
    AdProcess();
    if( AdReady() )
    {
        // Get ad samples
        AdData( (unsigned char*)mFftDataReal );

        mAdPeak = 0;
        mAdSum = 0;
        for( unsigned int i=0; i<AD_NUM_SAMPLES; i++ )
        {
            mAdTemp = (unsigned char)mFftDataReal[i];
            if( mAdTemp > mAdPeak )
            {
                mAdPeak = mAdTemp;
            }
            mAdSum += mAdTemp;
        }
        mAdMean = mAdSum / AD_NUM_SAMPLES;
        mAdOffset = ( mAdOffset >> 1 ) + ( mAdMean >> 1 );

        // Prep fft data
        unsigned char offset = 128;
        for( unsigned int i=0; i<AD_NUM_SAMPLES; i++ )
        {
            mFftDataReal[i] = (unsigned char)mFftDataReal[i] - offset;
            mFftDataImag[i] = 0;
        }

        // Do Fft
        fix_fft( mFftDataReal, mFftDataImag, 7, 0 );
        Magnitude( mFftMag, mFftDataReal, mFftDataImag, AD_NUM_SAMPLES/2 );

        mFftCount++;
        mNewFft = 1;
    }

    if( mNewFft )
    {
        PatternUpdateRaw( mAdMean, mAdPeak );
        PatternUpdateFreq( mFftMag );
    }
    if( PatternProcess() )
    {
        memcpy( mPwmValues, PatternData(), PWM_NUM_CHANNELS );
        mUpdatePwm = 1;
    }

    if( mUpdatePwm )
    {
        mUpdatePwm = 0;
        PwmSetChannels( mPwmValues );
    }
    PwmProcess();

#ifdef DEBUG
    static unsigned long mDebugPrintTime = millis();
    if( ( millis() - mDebugPrintTime ) > 1000 )
    {
        mDebugPrintTime = millis();

        // Analog audio
        Serial.print( F(" AdPeak=") );
        Serial.print( mAdPeak );
        Serial.print( F(" AdMean=") );
        Serial.print( mAdMean );
        Serial.print( F(" AdOffset=") );
        Serial.print( mAdOffset );
        Serial.print( F(" FftCounts=") );
        Serial.print( mFftCount );

        /*
        Serial.print( F(" AD Samples: ") );
        for( int i=0; i<32; i++ )
        {
            if( mAdSamples[i] < 16 )
            {
                Serial.write( '0' );
                Serial.print( mAdSamples[i], HEX );
            }
            else
            {
                Serial.print( mAdSamples[i], HEX );
            }
            Serial.print( "::" );
        }
        Serial.print( "\n\r" );
        */

        PrintDataUint( " FFT:", mFftMag, 11 );
        //PrintDataUint( " FFT4:", mFftFourBuckets, 4 );
        Serial.print( "\n\r" );

        mFftCount = 0;
    }
#endif

}

