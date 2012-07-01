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


void Magnitude( unsigned int* mag, char* real, char* imag, unsigned char n )
{
    for( unsigned int i=0; i<n; i++ )
    {
        mag[i] = sqrt( pow( real[i], 2 ) + pow( imag[i], 2 ) );
    }
}

void PrintDataChar( const char* label, char* data, unsigned char n )
{
    Serial.print( label );
    for( unsigned int i=0; i<n; i++ )
    {
        Serial.print( HexString( data[i] ) );
        Serial.print( "::" );
    }
}

void PrintDataUint( const char* label, unsigned int* data, unsigned char n )
{
    Serial.print( label );
    for( unsigned int i=0; i<n; i++ )
    {
        unsigned char hi = (unsigned char)( data[i] >> 8 );
        unsigned char lo = (unsigned char)data[i];
        Serial.print( HexString( hi ) );
        Serial.print( HexString( lo ) );
        Serial.print( "::" );
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

