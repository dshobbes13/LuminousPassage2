// File: utility.h

#ifndef UTILITY_H
#define UTILITY_H

//*****************
// INCLUDES
//*****************


//*****************
// DEFINITIONS
//*****************

// Clear bit macro for SFRs
#ifndef cbi
#define cbi( sfr, bit ) ( _SFR_BYTE( sfr ) &= ~_BV( bit ) )
#endif

// Set bit macro for SFRs
#ifndef sbi
#define sbi( sfr, bit ) ( _SFR_BYTE( sfr ) |= _BV( bit ) )
#endif


//*****************
// PUBLIC FUNCTIONS
//*****************

void DebugInit( void );

void DebugUp( void );

void DebugDown( void );

void DebugSet0( unsigned char value );

void DebugSet1( unsigned char value );

void Magnitude( unsigned char* mag, char* real, char* imag, unsigned char n );

char* HexString( unsigned char hex );

char HexChar( unsigned char hex );

#endif
