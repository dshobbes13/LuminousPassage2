// File: utility.cpp

//*****************
// INCLUDES
//*****************

#include "utility.h"

#include <Arduino.h>


//*****************
// DEFINITIONS
//*****************

const unsigned char SINEWAVE[256] PROGMEM = {
0x00, 0x03, 0x06, 0x09, 0x0C, 0x10, 0x13, 0x16, 0x19, 0x1C, 0x1F, 0x22, 0x25, 0x28, 0x2B, 0x2E,
0x31, 0x33, 0x36, 0x39, 0x3C, 0x3F, 0x41, 0x44, 0x47, 0x49, 0x4C, 0x4E, 0x51, 0x53, 0x55, 0x58,
0x5A, 0x5C, 0x5E, 0x60, 0x62, 0x64, 0x66, 0x68, 0x6A, 0x6B, 0x6D, 0x6F, 0x70, 0x71, 0x73, 0x74,
0x75, 0x76, 0x78, 0x79, 0x7A, 0x7A, 0x7B, 0x7C, 0x7D, 0x7D, 0x7E, 0x7E, 0x7E, 0x7F, 0x7F, 0x7F,
0x7F, 0x7F, 0x7F, 0x7F, 0x7E, 0x7E, 0x7E, 0x7D, 0x7D, 0x7C, 0x7B, 0x7A, 0x7A, 0x79, 0x78, 0x76,
0x75, 0x74, 0x73, 0x71, 0x70, 0x6F, 0x6D, 0x6B, 0x6A, 0x68, 0x66, 0x64, 0x62, 0x60, 0x5E, 0x5C,
0x5A, 0x58, 0x55, 0x53, 0x51, 0x4E, 0x4C, 0x49, 0x47, 0x44, 0x41, 0x3F, 0x3C, 0x39, 0x36, 0x33,
0x31, 0x2E, 0x2B, 0x28, 0x25, 0x22, 0x1F, 0x1C, 0x19, 0x16, 0x13, 0x10, 0x0C, 0x09, 0x06, 0x03,
0x00, 0xFD, 0xFA, 0xF7, 0xF4, 0xF0, 0xED, 0xEA, 0xE7, 0xE4, 0xE1, 0xDE, 0xDB, 0xD8, 0xD5, 0xD2,
0xCF, 0xCD, 0xCA, 0xC7, 0xC4, 0xC1, 0xBF, 0xBC, 0xB9, 0xB7, 0xB4, 0xB2, 0xAF, 0xAD, 0xAB, 0xA8,
0xA6, 0xA4, 0xA2, 0xA0, 0x9E, 0x9C, 0x9A, 0x98, 0x96, 0x95, 0x93, 0x91, 0x90, 0x8F, 0x8D, 0x8C,
0x8B, 0x8A, 0x88, 0x87, 0x86, 0x86, 0x85, 0x84, 0x83, 0x83, 0x82, 0x82, 0x82, 0x81, 0x81, 0x81,
0x81, 0x81, 0x81, 0x81, 0x82, 0x82, 0x82, 0x83, 0x83, 0x84, 0x85, 0x86, 0x86, 0x87, 0x88, 0x8A,
0x8B, 0x8C, 0x8D, 0x8F, 0x90, 0x91, 0x93, 0x95, 0x96, 0x98, 0x9A, 0x9C, 0x9E, 0xA0, 0xA2, 0xA4,
0xA6, 0xA8, 0xAB, 0xAD, 0xAF, 0xB2, 0xB4, 0xB7, 0xB9, 0xBC, 0xBF, 0xC1, 0xC4, 0xC7, 0xCA, 0xCD,
0xCF, 0xD2, 0xD5, 0xD8, 0xDB, 0xDE, 0xE1, 0xE4, 0xE7, 0xEA, 0xED, 0xF0, 0xF4, 0xF7, 0xFA, 0xFD,
};


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

void Magnitude( unsigned int* mag, char* real, char* imag, unsigned char n )
{
    for( unsigned int i=0; i<n; i++ )
    {
        mag[i] = sqrt( pow( real[i], 2 ) + pow( imag[i], 2 ) );
    }
}

unsigned char Sin( unsigned char index )
{
    return pgm_read_byte_near( SINEWAVE + index );
}

unsigned char Cos( unsigned char index )
{
    return pgm_read_byte_near( SINEWAVE + index + 64 );
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

