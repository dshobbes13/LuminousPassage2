// File: audio.cpp

//*****************
// INCLUDES
//*****************

#include "audio.h"

#ifndef SOFTWARE

#include <Arduino.h>

#include "typedefs.h"

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

static quint16 mBuckets[GLOBAL_NUM_BUCKETS] = {0};
static quint8 mLo[GLOBAL_NUM_BUCKETS] = {0};
static quint8 mHi[GLOBAL_NUM_BUCKETS] = {0};

static quint8 mFreqAverages[GLOBAL_NUM_FREQ] = {0};
static quint16 mBucketAverages[GLOBAL_NUM_BUCKETS] = {0};

static quint8 mThreshold = 8;
static float mAveraging = 0.9;


//*****************
// PRIVATE PROTOTYPES
//*****************


//*****************
// PUBLIC
//*****************

void AudioInit( void )
{
}

void AudioUpdateThreshold( quint8 threshold )
{
    mThreshold = threshold;
}

void AudioUpdateAveraging( float averaging )
{
    mAveraging = averaging;
}

void AudioUpdateBuckets( quint8* lo, quint8* hi )
{
    memcpy( mLo, lo, GLOBAL_NUM_BUCKETS );
    memcpy( mHi, hi, GLOBAL_NUM_BUCKETS );
}

void AudioUpdateFreq( quint8* newFrequencies )
{
    for( quint8 i=0; i<GLOBAL_NUM_BUCKETS; i++ )
    {
        mBuckets[i] = 0;
        for( quint8 j=0; j<GLOBAL_NUM_FREQ; j++ )
        {
            if( ( j >= mLo[i] ) && ( j <= mHi[i] ) )
            {
                quint8 value = newFrequencies[j];
                mBuckets[i] += ( value > mThreshold ) ? value : 0;
            }
        }
    }

    for( quint8 i=0; i<GLOBAL_NUM_FREQ; i++ )
    {
        float oldValue = (float)(mFreqAverages[i]) * mAveraging;
        float newValue = (float)(newFrequencies[i]) * ( 1.0 - mAveraging );
        mFreqAverages[i] = (quint8)( oldValue + newValue );
    }

    for( quint8 i=0; i<GLOBAL_NUM_BUCKETS; i++ )
    {
        float oldValue = (float)(mBucketAverages[i]) * mAveraging;
        float newValue = (float)(mBuckets[i]) * ( 1.0 - mAveraging );
        mBucketAverages[i] = (quint16)( oldValue + newValue );
    }

}

quint16* AudioBuckets( void )
{
    return mBuckets;
}

quint8* AudioFreqAverages( void )
{
    return mFreqAverages;
}

quint16* AudioBucketAverages( void )
{
    return mBucketAverages;
}

