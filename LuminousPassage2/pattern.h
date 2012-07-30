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

enum eEffect
{
    Effect_NULL       = 0,
    Effect_MANUAL     = 1,
    Effect_CYCLE      = 2,
    Effect_BREATH     = 3,
    Effect_FREQ       = 4,
    Effect_BUCKETS    = 5,
    Effect_BASS_PULSE = 6,
    Effect_MAX        = 7,
};


//*****************
// PUBLIC FUNCTIONS
//*****************

void PatternInit( void );

void PatternSetEffect( eEffect effect, bool on = true );

quint8 PatternGetEffect( eEffect effect );

void PatternProcess( void );

quint8* PatternData( void );

void PatternUpdateFreq( quint8* newFrequencies );

void PatternUpdateBuckets( quint16* newBuckets, quint16* newBucketAverages );

void PatternUpdateAd( quint8 mean, quint8 peak );

#endif
