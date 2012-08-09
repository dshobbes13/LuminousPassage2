// File: audio.cpp

//*****************
// INCLUDES
//*****************

#include "audio.h"

#ifndef SOFTWARE

#include <Arduino.h>
#include <EEPROM.h>

#include "config.h"
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

static quint8 mThreshold= 0;
static float mAveraging = 0;


//*****************
// PRIVATE PROTOTYPES
//*****************


//*****************
// PUBLIC
//*****************

void AudioInit( void )
{
    mThreshold = 8;
    mAveraging = 0.9;
    mLo[0] = 1;
    mHi[0] = 2;
    mLo[1] = 3;
    mHi[1] = 4;
    mLo[2] = 5;
    mHi[2] = 8;
    mLo[3] = 9;
    mHi[3] = 16;
    mLo[4] = 17;
    mHi[4] = 32;
    mLo[5] = 33;
    mHi[5] = 63;
}

void AudioSave( void )
{
#ifdef FIRMWARE
    cli();
    EEPROM.write( EEPROM_AUDIO_THRESHOLD, mThreshold );
    EEPROM.write( EEPROM_AUDIO_AVERAGING, mAveraging );
    EEPROM.write( EEPROM_AUDIO_BUCKETS_LO_0, mLo[0] );
    EEPROM.write( EEPROM_AUDIO_BUCKETS_HI_0, mHi[0] );
    EEPROM.write( EEPROM_AUDIO_BUCKETS_LO_1, mLo[1] );
    EEPROM.write( EEPROM_AUDIO_BUCKETS_HI_1, mHi[1] );
    EEPROM.write( EEPROM_AUDIO_BUCKETS_LO_2, mLo[2] );
    EEPROM.write( EEPROM_AUDIO_BUCKETS_HI_2, mHi[2] );
    EEPROM.write( EEPROM_AUDIO_BUCKETS_LO_3, mLo[3] );
    EEPROM.write( EEPROM_AUDIO_BUCKETS_HI_3, mHi[3] );
    EEPROM.write( EEPROM_AUDIO_BUCKETS_LO_4, mLo[4] );
    EEPROM.write( EEPROM_AUDIO_BUCKETS_HI_4, mHi[4] );
    EEPROM.write( EEPROM_AUDIO_BUCKETS_LO_5, mLo[5] );
    EEPROM.write( EEPROM_AUDIO_BUCKETS_HI_5, mHi[5] );
    sei();
#endif
}

void AudioLoad( void )
{
#ifdef FIRMWARE
    cli();
    mThreshold = EEPROM.read( EEPROM_AUDIO_THRESHOLD );
    mAveraging = EEPROM.read( EEPROM_AUDIO_AVERAGING );
    mLo[0] = EEPROM.read( EEPROM_AUDIO_BUCKETS_LO_0 );
    mHi[0] = EEPROM.read( EEPROM_AUDIO_BUCKETS_HI_0 );
    mLo[1] = EEPROM.read( EEPROM_AUDIO_BUCKETS_LO_1 );
    mHi[1] = EEPROM.read( EEPROM_AUDIO_BUCKETS_HI_1 );
    mLo[2] = EEPROM.read( EEPROM_AUDIO_BUCKETS_LO_2 );
    mHi[2] = EEPROM.read( EEPROM_AUDIO_BUCKETS_HI_2 );
    mLo[3] = EEPROM.read( EEPROM_AUDIO_BUCKETS_LO_3 );
    mHi[3] = EEPROM.read( EEPROM_AUDIO_BUCKETS_HI_3 );
    mLo[4] = EEPROM.read( EEPROM_AUDIO_BUCKETS_LO_4 );
    mHi[4] = EEPROM.read( EEPROM_AUDIO_BUCKETS_HI_4 );
    mLo[5] = EEPROM.read( EEPROM_AUDIO_BUCKETS_LO_5 );
    mHi[5] = EEPROM.read( EEPROM_AUDIO_BUCKETS_HI_5 );
    sei();
#endif
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

