

#include "fft.h"
#include "pwm.h"
#include "utility.h"
#include "ad.h"
#include "testFft.h"

#define DEBUG 1


char mFftDataReal[AD_NUM_SAMPLES];
char mFftDataImag[AD_NUM_SAMPLES];
unsigned int mFftMag[AD_NUM_SAMPLES];

unsigned char mChannelValues[PWM_NUM_CHANNELS] = {0};

void setup( void )
{
    delay( 1000 );

    // Init serial port for debugging
    Serial.begin( 115200 );
    while( !Serial );

    // Init modules
    AdInit();
    PwmInit();

    //TestFft();
}

void loop( void )
{
    static unsigned long mDebugMeasureTime = 0;
    
    static unsigned long mDebugAdBlockTime = 0;
    static unsigned long mDebugAdBlockCount = 0;
    
    static unsigned long mDebugFftBlockTime = 0;
    static unsigned long mDebugFftBlockCount = 0;

    static unsigned long mDebugPwmBlockTime = 0;
    static unsigned long mDebugPwmBlockCount = 0;

    static unsigned char mDebugAdPeak = 0;
    static unsigned long mDebugAdSum = 0;
    static unsigned long mDebugAdCount = 0;

    static unsigned int mFftFourBuckets[4] = {0};

    // Check for A/D sample
    AdProcess();
    if( AdReady() )
    {
        // Get ad samples
        mDebugMeasureTime = micros();
        AdData( (unsigned char*)mFftDataReal );
        mDebugAdBlockTime += micros() - mDebugMeasureTime;
        mDebugAdBlockCount++;

        static unsigned char mDebugAdTempSample;
        for( unsigned int i=0; i<AD_NUM_SAMPLES; i++ )
        {
            mDebugAdTempSample = (unsigned char)mFftDataReal[i];
            if( mDebugAdTempSample > mDebugAdPeak )
            {
                mDebugAdPeak = mDebugAdTempSample;
            }
            mDebugAdSum += mDebugAdTempSample;
            mDebugAdCount++;
        }

        // Prep fft data
        mDebugMeasureTime = micros();
        unsigned char offset = 128;
        for( unsigned int i=0; i<AD_NUM_SAMPLES; i++ )
        {
            mFftDataReal[i] = (unsigned char)mFftDataReal[i] - offset;
            mFftDataImag[i] = 0;
        }

        // Do Fft
        fix_fft( mFftDataReal, mFftDataImag, 6, 0 );
        Magnitude( mFftMag, mFftDataReal, mFftDataImag, AD_NUM_SAMPLES/2 );
        mDebugFftBlockTime += micros() - mDebugMeasureTime;
        mDebugFftBlockCount++;
    }

    /*
    // Update channel values
    mFftFourBuckets[0] = 0;
    mFftFourBuckets[1] = 0;
    mFftFourBuckets[2] = 0;
    mFftFourBuckets[3] = 0;
    for( unsigned char i=0; i<AD_NUM_SAMPLES/2; i++ )
    {
        mFftFourBuckets[ (i&0xF8)>>3 ] += mFftMag[i];
    }

    for( unsigned char i=0; i<4; i++ )
    {
        if( mFftFourBuckets[i] > 0x10 )
        {
            mFftFourBuckets[i] = mFftFourBuckets[i] * 4;
        }
        else
        {
            mFftFourBuckets[i] = 0;
        }
        mChannelValues[i] = ( mFftFourBuckets[i] <= 0xFF ) ? mFftFourBuckets[i] : 0xFF;
    }
    */

    static unsigned long mChannelSweepTime = millis();
    static unsigned char mState = 1;
    if( millis() - mChannelSweepTime > 500 )
    {
        mChannelSweepTime = millis();
        if( mState )
        {
            for( unsigned char i=0; i<PWM_NUM_CHANNELS; i++ )
            {
                mChannelValues[i] += 0x08;
            }
            if( mChannelValues[0] > 0xF0 )
            {
                mState = 0;
            }
        }
        else
        {
            for( unsigned char i=0; i<PWM_NUM_CHANNELS; i++ )
            {
                mChannelValues[i] -= 0x08;
            }
            if( mChannelValues[0] < 0x10 )
            {
                mState = 1;
            }
        }
        for( unsigned char i=0; i<PWM_NUM_CHANNELS; i++ )
        {
            mChannelValues[i] = 0x08;
        }
        PwmSetChannels( mChannelValues );
    }
    PwmProcess();

#if DEBUG
    static unsigned long mDebugPrintTime = millis();
    if( ( millis() - mDebugPrintTime ) > 1000 )
    {
        mDebugPrintTime = millis();

        /*
        // Timing
        Serial.print( F(" AdCounts=") );
        Serial.print( timeAdCounts );
        Serial.print( F(" AdAvgDelay=") );
        Serial.print( timeAdDelays / timeAdCounts );
        Serial.print( F(" AdPeakDelay=") );
        Serial.print( timeAdPeakDelay );
        Serial.print( F(" AdAvgBlocks=") );
        Serial.print( timeAdBlocks / timeAdCounts );
        Serial.print( F(" FftCounts=") );
        Serial.print( timeFftCounts );
        Serial.print( F(" FftAvgBlocks=") );
        Serial.print( timeFftBlocks / timeFftCounts );
        Serial.print( F(" PwmCounts=") );
        Serial.print( timePwmCounts );
        Serial.print( F(" PwmAvgDelays=") );
        Serial.print( timePwmDelays / timePwmCounts );
        Serial.print( F(" PwmPeakDelay=") );
        Serial.print( timePwmPeakDelay );
        Serial.print( F(" PwmAvgBlocks=") );
        Serial.print( timePwmBlocks / timePwmCounts );
        Serial.print( "\n\r" );
        */

        /*
        Serial.write( 0x1B );
        Serial.write( '[' );
        Serial.write( '4' );
        Serial.write( 'A' );
        */

        // Analog audio
        Serial.print( F(" AdPeak=") );
        Serial.print( mDebugAdPeak );
        Serial.print( F(" AdAvg=") );
        Serial.print( mDebugAdSum / mDebugAdCount );
        Serial.print( F(" FftCounts=") );
        Serial.print( mDebugFftBlockCount );

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

        //PrintDataUint( " FFT:", mFftMag, AD_NUM_SAMPLES/2 );
        PrintDataUint( " FFT4:", mFftFourBuckets, 4 );
        Serial.print( "\n\r" );

        /*
        Serial.print( 
        Serial.write( 0x1B );
        Serial.write( '[' );
        Serial.write( '4' );
        Serial.write( 'A' );
        */

        mDebugAdBlockTime = 0;
        mDebugAdBlockCount = 0;

        mDebugFftBlockTime = 0;
        mDebugFftBlockCount = 0;

        mDebugPwmBlockTime = 0;
        mDebugPwmBlockCount = 0;

        mDebugAdPeak = 0;
        mDebugAdSum = 0;
        mDebugAdCount = 0;
    }
#endif

}

