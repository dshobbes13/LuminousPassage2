// File: ad.h

#ifndef AD_H
#define AD_H

//*****************
// DEFINITIONS
//*****************

#define AD_ISR_VERSION
//#define AD_BLOCKING_VERSION

#define AD_NUM_SAMPLES    128


//*****************
// PUBLIC FUNCTIONS
//*****************

void AdInit( void );

void AdProcess( void );

unsigned char AdReady( void );

void AdData( unsigned char* data );

#endif
