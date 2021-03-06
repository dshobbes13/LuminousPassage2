// File: audio.h

#ifndef AUDIO_H
#define AUDIO_H

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


//*****************
// PUBLIC FUNCTIONS
//*****************

void AudioInit( void );

void AudioSave( void );

void AudioLoad( void );

void AudioSetParameters( quint8 threshold, float averaging, quint8* lo, quint8* hi );

void AudioUpdateFreq( quint8* newFrequencies );

quint16* AudioBuckets( void );

quint8* AudioFreqAverages( void );

quint16* AudioBucketAverages( void );

#endif
