// File: pattern.h

#ifndef PATTERN_H
#define PATTERN_H

//*****************
// INCLUDES
//*****************

#ifndef SOFTWARE
#include "typedefs.h"
#else
#include <QtGlobal>
#endif


//*****************
// DEFINITIONS
//*****************

//#define PATTERN_BLOCKING_VERSION
#define PATTERN_ISR_VERSION


enum eEffect
{
    Effect_NULL             = 0,
    Effect_MANUAL           = 1,
    Effect_CYCLE            = 2,
    Effect_BREATH           = 3,
    Effect_FREQ             = 4,
    Effect_BUCKETS          = 5,
    Effect_PULSE_SQUARE     = 6,
    Effect_PULSE_SINE       = 7,
    Effect_DISTANCE_SQUARE  = 8,
    Effect_MAX              = 9,
};


//*****************
// PUBLIC FUNCTIONS
//*****************

void PatternInit( void );

void PatternSave( void );

void PatternLoad( void );

void PatternSetEffect( eEffect effect, quint8 on );

void PatternSetBucketParameters( float bucketHysteresis, quint8 bucketTimeDebounceFlags, quint8 bucketTimeDebounceSeconds );

void PatternSetManual( quint8 value );

void PatternSetPulseSquare( quint8 source, quint8 length, quint8 width );

void PatternSetPulseSine( quint8 source, quint8 length, quint8 width );

void PatternSetDistanceSquare( quint8 source, quint8 start, quint8 stop, quint8 amp );

quint8 PatternReady( void );

void PatternData( quint8* data );

void PatternProcess( void );

void PatternUpdateFreq( quint8* newFrequencies );

void PatternUpdateBuckets( quint16* newBuckets, quint16* newBucketAverages );

void PatternUpdateAd( quint8 mean, quint8 peak );

#endif
