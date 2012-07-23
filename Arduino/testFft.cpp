// File: testFft.cpp

//*****************
// INCLUDES
//*****************

#include "testFft.h"

#include <Arduino.h>

#include "fft.h"
#include "utility.h"


//*****************
// DEFINITIONS
//*****************

#define NUM_SAMPLES     64

//*****************
// VARIABLES
//*****************

static char mImagData[NUM_SAMPLES];
static unsigned int mMagnitude[NUM_SAMPLES];

static char TEST_DC[NUM_SAMPLES] = {
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };

static char TEST_156[NUM_SAMPLES] = {
0x00, 0x06, 0x0C, 0x13, 0x19, 0x1F, 0x25, 0x2B, 0x31, 0x36, 0x3C, 0x41, 0x47, 0x4C, 0x51, 0x55,
0x5A, 0x5E, 0x62, 0x66, 0x6A, 0x6D, 0x70, 0x73, 0x75, 0x78, 0x7A, 0x7B, 0x7D, 0x7E, 0x7E, 0x7F,
0x7F, 0x7F, 0x7E, 0x7E, 0x7D, 0x7B, 0x7A, 0x78, 0x75, 0x73, 0x70, 0x6D, 0x6A, 0x66, 0x62, 0x5E,
0x5A, 0x55, 0x51, 0x4C, 0x47, 0x41, 0x3C, 0x36, 0x31, 0x2B, 0x25, 0x1F, 0x19, 0x13, 0x0C, 0x06 };

static char TEST_5156[NUM_SAMPLES] = {
0x00, 0x7F, 0xF4, 0x82, 0x19, 0x7B, 0xDB, 0x88, 0x31, 0x73, 0xC4, 0x93, 0x46, 0x66, 0xB0, 0xA2,
0x5A, 0x55, 0x9E, 0xB4, 0x69, 0x41, 0x90, 0xC9, 0x75, 0x2B, 0x87, 0xE1, 0x7D, 0x13, 0x82, 0xF9,
0x7F, 0xFA, 0x82, 0x12, 0x7D, 0xE2, 0x86, 0x2A, 0x75, 0xCA, 0x90, 0x41, 0x6A, 0xB5, 0x9E, 0x55,
0x5A, 0xA2, 0xAF, 0x66, 0x47, 0x93, 0xC4, 0x73, 0x31, 0x89, 0xDB, 0x7B, 0x19, 0x82, 0xF3, 0x7F };

static char TEST_9844[NUM_SAMPLES] = {
0x00, 0x06, 0xF4, 0x13, 0xE7, 0x1F, 0xDB, 0x2B, 0xCF, 0x36, 0xC4, 0x41, 0xBA, 0x4C, 0xB0, 0x55,
0xA6, 0x5E, 0x9E, 0x66, 0x97, 0x6D, 0x90, 0x73, 0x8B, 0x77, 0x87, 0x7B, 0x83, 0x7E, 0x82, 0x7F,
0x81, 0x7F, 0x82, 0x7E, 0x83, 0x7B, 0x86, 0x78, 0x8B, 0x73, 0x90, 0x6D, 0x96, 0x66, 0x9E, 0x5E,
0xA6, 0x56, 0xAF, 0x4C, 0xB9, 0x42, 0xC4, 0x37, 0xCF, 0x2B, 0xDB, 0x1F, 0xE7, 0x13, 0xF3, 0x07 };

static char TEST_14844[NUM_SAMPLES] = {
0x00, 0x81, 0x0C, 0x7E, 0xE7, 0x85, 0x25, 0x78, 0xCF, 0x8D, 0x3C, 0x6D, 0xBA, 0x9A, 0x50, 0x5E,
0xA6, 0xAB, 0x62, 0x4C, 0x97, 0xBF, 0x70, 0x36, 0x8B, 0xD5, 0x79, 0x1F, 0x83, 0xED, 0x7E, 0x07,
0x81, 0x06, 0x7E, 0xEE, 0x83, 0x1F, 0x7A, 0xD6, 0x8B, 0x36, 0x70, 0xBF, 0x96, 0x4B, 0x62, 0xAB,
0xA6, 0x5E, 0x51, 0x9A, 0xB9, 0x6D, 0x3C, 0x8D, 0xCF, 0x77, 0x25, 0x85, 0xE7, 0x7E, 0x0D, 0x81 };

static char TEST_19844[NUM_SAMPLES] = {
0x00, 0xFA, 0xF4, 0xED, 0xE7, 0xE1, 0xDB, 0xD5, 0xCF, 0xCA, 0xC4, 0xBF, 0xBA, 0xB4, 0xB0, 0xAB,
0xA6, 0xA2, 0x9E, 0x9A, 0x97, 0x93, 0x90, 0x8D, 0x8B, 0x89, 0x87, 0x85, 0x83, 0x82, 0x82, 0x81,
0x81, 0x81, 0x82, 0x82, 0x83, 0x85, 0x86, 0x88, 0x8B, 0x8D, 0x90, 0x93, 0x96, 0x9A, 0x9E, 0xA2,
0xA6, 0xAA, 0xAF, 0xB4, 0xB9, 0xBE, 0xC4, 0xC9, 0xCF, 0xD5, 0xDB, 0xE1, 0xE7, 0xED, 0xF3, 0xF9 };

static char TEST_500HZ[NUM_SAMPLES] = {
0x00, 0x14, 0x27, 0x3A, 0x4B, 0x5A, 0x67, 0x71, 0x79, 0x7D, 0x7F, 0x7D, 0x79, 0x71, 0x67, 0x5A,
0x4B, 0x3A, 0x27, 0x14, 0x00, 0xEC, 0xD9, 0xC6, 0xB5, 0xA6, 0x99, 0x8F, 0x87, 0x83, 0x81, 0x83,
0x87, 0x8F, 0x99, 0xA6, 0xB5, 0xC6, 0xD9, 0xEC, 0x00, 0x14, 0x27, 0x3A, 0x4B, 0x5A, 0x67, 0x71,
0x79, 0x7D, 0x7F, 0x7D, 0x79, 0x71, 0x67, 0x5A, 0x4B, 0x3A, 0x27, 0x14, 0x00, 0xEC, 0xD9, 0xC6 };

static char TEST_1KHZ[NUM_SAMPLES] = {
0x00, 0x27, 0x4B, 0x67, 0x79, 0x7F, 0x79, 0x67, 0x4B, 0x27, 0x00, 0xD9, 0xB5, 0x99, 0x87, 0x81,
0x87, 0x99, 0xB5, 0xD9, 0x00, 0x27, 0x4B, 0x67, 0x79, 0x7F, 0x79, 0x67, 0x4B, 0x27, 0x00, 0xD9,
0xB5, 0x99, 0x87, 0x81, 0x87, 0x99, 0xB5, 0xD9, 0x00, 0x27, 0x4B, 0x67, 0x79, 0x7F, 0x79, 0x67,
0x4B, 0x27, 0x00, 0xD9, 0xB5, 0x99, 0x87, 0x81, 0x87, 0x99, 0xB5, 0xD9, 0x00, 0x27, 0x4B, 0x67 };

static char TEST_5KHZ[NUM_SAMPLES] = {
0x00, 0x7F, 0x00, 0x81, 0x00, 0x7F, 0x00, 0x81, 0x00, 0x7F, 0x00, 0x81, 0x00, 0x7F, 0x00, 0x81,
0x00, 0x7F, 0x00, 0x81, 0x00, 0x7F, 0x00, 0x81, 0x00, 0x7F, 0x00, 0x81, 0x00, 0x7F, 0x00, 0x81,
0x00, 0x7F, 0x00, 0x81, 0x00, 0x7F, 0x00, 0x81, 0x00, 0x7F, 0x00, 0x81, 0x00, 0x7F, 0x00, 0x81,
0x00, 0x7F, 0x00, 0x81, 0x00, 0x7F, 0x00, 0x81, 0x00, 0x7F, 0x00, 0x81, 0x00, 0x7F, 0x00, 0x81 };

//*****************
// PRIVATE PROTOTYPES
//*****************

void test( const char* label, char* data );

//*****************
// PUBLIC
//*****************

void TestFft( void )
{
    test( "FFT     DC: ", TEST_DC );
    delay( 500 );
    test( "FFT    156: ", TEST_156 );
    delay( 500 );
    test( "FFT   5156: ", TEST_5156 );
    delay( 500 );
    test( "FFT  9,844: ", TEST_9844 );
    delay( 500 );
    test( "FFT 14,844: ", TEST_14844 );
    delay( 500 );
    test( "FFT 19,844: ", TEST_19844 );
    delay( 500 );
    test( "FFT    500: ", TEST_500HZ );
    delay( 500 );
    test( "FFT  1,000: ", TEST_1KHZ );
    delay( 500 );
    test( "FFT  5,000: ", TEST_5KHZ );
    delay( 500 );
}

//*****************
// PRIVATE
//*****************

void test( const char* label, char* data )
{
    for( int i=0; i<NUM_SAMPLES; i++ )
    {
        mImagData[i] = 0;
    }
    fix_fft( data, mImagData, 6, 0 );
    Magnitude( mMagnitude, data, mImagData, NUM_SAMPLES/2 );
    PrintDataUint( label, mMagnitude, NUM_SAMPLES/2 );
}