#ifndef UTILITY_H
#define UTILITY_H

// Clear bit macro for SFRs
#ifndef cbi
#define cbi( sfr, bit ) ( _SFR_BYTE( sfr ) &= ~_BV( bit ) )
#endif

// Set bit macro for SFRs
#ifndef sbi
#define sbi( sfr, bit ) ( _SFR_BYTE( sfr ) |= _BV( bit ) )
#endif

void Magnitude( unsigned int* mag, char* real, char* imag, unsigned char n );

void PrintDataChar( const char* label, char* data, unsigned char n );

void PrintDataUint( const char* label, unsigned int* data, unsigned char n );

char* HexString( unsigned char hex );

char HexChar( unsigned char hex );

#endif
