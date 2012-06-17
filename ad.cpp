// File: ad.cpp

//*****************
// INCLUDES
//*****************

#include "ad.h"

#include <Arduino.h>

#include "utility.h"


//*****************
// DEFINITIONS
//*****************

#define AD_SAMPLE_TIME_US   50
#define AD_INPUT_PIN        0

//*****************
// VARIABLES
//*****************

volatile static unsigned char mAdData[2][AD_NUM_SAMPLES] = {0};
volatile static unsigned char mAdSampleArray = 0;
volatile static unsigned char mAdSampleNumber = 0;
volatile static unsigned char mAdReady = 0;


//*****************
// PRIVATE PROTOTYPES
//*****************


//*****************
// PUBLIC
//*****************

void AdInit( void )
{
    // Configure AD system
    analogReference( DEFAULT );

    // Configure Timer

    // Initialize variables

    // Start sampling
}

void AdProcess( void )
{
    static unsigned long mAdTime = micros();
    static unsigned long mCurrentTime = micros();
    static unsigned char mAdSample = 0;

    mCurrentTime = micros();
    if( ( mCurrentTime - mAdTime ) > AD_SAMPLE_TIME_US )
    {
        // Reset time for next sample
        mAdTime = mCurrentTime;

        // Sample A/D
        mAdSample = (unsigned char)( analogRead( AD_INPUT_PIN ) >> 2 );
        mAdData[0][mAdSampleNumber] = mAdSample;

        // Check for wrap on buffer
        if( ++mAdSampleNumber >= AD_NUM_SAMPLES )
        {
            // Reset for next pass
            mAdSampleNumber = 0;
            mAdReady = 1;
        }
    }
}

unsigned char AdReady( void )
{
    if( mAdReady )
    {
        mAdReady = 0;
        return 1;
    }
    return 0;
}

void AdData( unsigned char* data )
{
    memcpy( data, (const void*)&mAdData[0], AD_NUM_SAMPLES );
}

ISR( AD_vect )
{
}

