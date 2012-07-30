// File: LuminousPassage2

//*****************
// INCLUDES
//*****************

#include "ad.h"
#include "comMaster.h"
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

#define DEBUG


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

static unsigned long mLastPatternTime = 0;

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
#if defined( MASTER_SINGLE ) || defined( MASTER )
    AdInit();
    PatternInit();
    ComMasterInit();
#endif

#if !defined( MASTER )
    PwmInit();
#endif

#if !defined( MASTER_SINGLE ) && !defined( MASTER )
    ComSlaveInit();
#endif

    //TestFft();
    //while(1);

    mLastPatternTime = micros();

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
    if( Serial.available() )
    {
        unsigned int command = Serial.read();
        unsigned char byteCommand = (quint8)command;
        if( ( byteCommand >= '0' ) && ( byteCommand <= '9' ) )
        {
            eEffect effect = (eEffect)( byteCommand - '0' );
            if( PatternGetEffect( effect ) )
            {
                PatternSetEffect( effect, false );
            }
            else
            {
                PatternSetEffect( effect, true );
            }
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

            PatternUpdateAd( mAdMean, mAdPeak );
            PatternUpdateFreq( mFftSum );

            //PrintDataCharRaw( mFftSum, GLOBAL_NUM_FREQ );

            for( unsigned char i=0; i<GLOBAL_NUM_FREQ; i ++ )
            {
                mFftSum[i] = 0;
            }

        }
    }

    // Give pattern generator processing time
    if( ( micros() - mLastPatternTime ) >= 10000 )
    {
#ifdef DEBUG
        DebugUp();
#endif
        mLastPatternTime += 10000;
        PatternProcess();
        memcpy( mPwmValues, PatternData(), GLOBAL_NUM_CHANNELS );
        mUpdatePwm = 1;
#ifdef DEBUG
        DebugDown();
#endif
    }

#else   // SLAVE

    // Check for new data over i2c
    if( 0 )//newData )
    {
        // GetData
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
        Serial.print( F(" AdPeak=") );
        Serial.print( mAdPeak );
        Serial.print( F(" AdMean=") );
        Serial.print( mAdMean );
        Serial.print( F(" AdOffset=") );
        Serial.print( mAdOffset );
        Serial.print( F(" FftCounts=") );
        Serial.print( mFftCount );
        */

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

        /*
        PrintDataUint( " FFT:", mFftMag, 11 );
        //PrintDataUint( " FFT4:", mFftFourBuckets, 4 );
        Serial.print( "\n\r" );

        mFftCount = 0;
        */
    }
#endif

}

