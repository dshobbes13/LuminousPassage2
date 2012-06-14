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

#define NUM_SAMPLES     64

//*****************
// VARIABLES
//*****************

volatile unsigned char mAdData[2][NUM_SAMPLES];
volatile unsigned char mAdSampleArray;
volatile unsigned char mAdSampleNumber;
volatile unsigned char mAdReady;


//*****************
// PRIVATE PROTOTYPES
//*****************


//*****************
// PUBLIC
//*****************

void AdInit( void )
{
    // Configure AD system

    // Configure Timer

    // Initialize variables

    // Start sampling
}

unsigned char AdReady( void )
{
    return mAdReady;
}

void AdData( signed char* data )
{
    memcpy( data, (const void*)&mAdData[0], NUM_SAMPLES );
    for( unsigned char i=0; i<NUM_SAMPLES; i++ )
    {
        data[i] = data[i] - 128;
    }
}

ISR( AD_vect )
{
}

