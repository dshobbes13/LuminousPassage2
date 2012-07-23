// File: pattern.h

#ifndef PATTERN_H
#define PATTERN_H

//*****************
// DEFINITIONS
//*****************

#define PATTERN_NUM_FREQ        32
#define PATTERN_NUM_CHANNELS    10

//*****************
// PUBLIC FUNCTIONS
//*****************

void PatternInit( void );

unsigned char PatternProcess( void );

unsigned char* PatternData( void );

void PatternUpdateFreq( unsigned int* newFrequencies );

void PatternUpdateRaw( unsigned char mean, unsigned char peak );

#endif
