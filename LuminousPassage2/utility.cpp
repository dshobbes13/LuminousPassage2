// File: utility.cpp

//*****************
// INCLUDES
//*****************

#include "utility.h"

#include <Arduino.h>


//*****************
// DEFINITIONS
//*****************


//*****************
// VARIABLES
//*****************


//*****************
// PRIVATE PROTOTYPES
//*****************


//*****************
// PUBLIC
//*****************

void DebugInit( void )
{
    DDRC = DDRC | 0x08;
    PORTC = PORTC & ~0x08;
}

void DebugUp( void )
{
    PORTC = PORTC | 0x08;
}

void DebugDown( void )
{
    PORTC = PORTC & ~0x08;
}

void Magnitude( unsigned char* mag, char* real, char* imag, unsigned char n )
{
    for( unsigned char i=0; i<n; i++ )
    {
        mag[i] = sqrt( pow( real[i], 2 ) + pow( imag[i], 2 ) );
    }
}

char* HexString( unsigned char hex )
{
    static char string[3] = { 0x00, 0x00, 0x00 };
    string[0] = HexChar( hex >> 4 );
    string[1] = HexChar( hex );
    return string;
}

char HexChar( unsigned char hex )
{
    unsigned char nibble = hex & 0x0F;
    return ( ( nibble < 10 ) ? ( nibble + '0' ) : ( nibble - 10 + 'A' ) );
}


//*****************
// PRIVATE
//*****************

