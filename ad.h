// File: ad.h

#ifndef AD_H
#define AD_H

//*****************
// DEFINITIONS
//*****************

#define AD_NUM_SAMPLES      64


//*****************
// PUBLIC FUNCTIONS
//*****************

void AdInit( void );

void AdProcess( void );

unsigned char AdReady( void );

void AdData( unsigned char* data );

#endif
