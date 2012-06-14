// File: ad.h

#ifndef AD_H
#define AD_H

//*****************
// DEFINITIONS
//*****************


//*****************
// PUBLIC FUNCTIONS
//*****************

void AdInit( void );

unsigned char AdReady( void );

void AdData( signed char* data );

#endif
