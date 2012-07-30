// File: ad.cpp

//*****************
// INCLUDES
//*****************

#include "ad.h"

#include <Arduino.h>

#include "global.h"
#include "utility.h"


//*****************
// DEFINITIONS
//*****************

//#define DEBUG

#define AD_SAMPLE_TIME_US   100
#define AD_INPUT_PIN        0

//*****************
// VARIABLES
//*****************

volatile static unsigned char mAdData[2][GLOBAL_NUM_SAMPLES] = { {0}, {0} };
volatile static unsigned char mAdSampleArray = 0;
volatile static unsigned char mAdSampleNumber = 0;
volatile static unsigned char mAdReady = 0;


//*****************
// PRIVATE PROTOTYPES
//*****************


//*****************
// PUBLIC
//*****************

void AdInit( void )
{
    // Configure AD system
    /*
    cbi( ADMUX, REFS1 );    // Use AREF
    cbi( ADMUX, REFS0 );
    */
    cbi( ADMUX, REFS1 );    // Use Internal AVcc
    sbi( ADMUX, REFS0 );
    sbi( ADMUX, ADLAR );    // Left justified
    sbi( ADCSRA, ADEN );    // Turn AD on
    sbi( ADCSRA, ADPS2 );   // Set pre-scale to 32 (500kHz AD Clock, 200kHz max for max resolution)
    cbi( ADCSRA, ADPS1 );
    sbi( ADCSRA, ADPS0 );
    sbi( ADCSRA, ADIF );    // Clear interrupt flag

    // Select AD input
    sbi( ADMUX, MUX0 );

#ifdef AD_ISR_VERSION
    sbi( ADCSRA, ADATE );   // Put AD in free-running mode
    sbi( ADCSRB, ADTS2 );   // Trigger from Timer1 compare match B
    cbi( ADCSRB, ADTS1 );
    sbi( ADCSRB, ADTS0 );

    // Configure Timer 1
    TCCR1A = 0x00;
    TCCR1B = 0x09;
    TCCR1C = 0x00;
    TCNT1H = 0x00;
    TCNT1L = 0x00;

    /*
    // 20kHz sampling
    OCR1AH = 0x03;
    OCR1AL = 0x20;
    OCR1BH = 0x03;
    OCR1BL = 0x20;
    */

    /*
    // 10kHz sampling
    OCR1AH = 0x06;
    OCR1AL = 0x40;
    OCR1BH = 0x06;
    OCR1BL = 0x40;
    */

    /*
    // 7680 Hz sampling
    OCR1AH = 0x08;
    OCR1AL = 0x23;
    OCR1BH = 0x08;
    OCR1BL = 0x23;
    */

    // 5120 Hz sampling
    OCR1AH = 0x0C;
    OCR1AL = 0x35;
    OCR1BH = 0x0C;
    OCR1BL = 0x35;

    /*
    // 5000 Hz sampling
    OCR1AH = 0x0C;
    OCR1AL = 0x80;
    OCR1BH = 0x0C;
    OCR1BL = 0x80;
    */

    ICR1H = 0x00;
    ICR1L = 0x00;

    // Start sampling
    sbi( ADCSRA, ADSC );
    sbi( ADCSRA, ADIE );
#endif

#ifdef DEBUG
    DebugInit();
#endif

}

void AdProcess( void )
{
#ifdef AD_BLOCKING_VERSION
    static unsigned long mAdTime = micros();
    static unsigned long mCurrentTime = micros();
    static unsigned char mAdSample = 0;

    mCurrentTime = micros();
    if( ( mCurrentTime - mAdTime ) > AD_SAMPLE_TIME_US )
    {
        // Reset time for next sample
        mAdTime = mCurrentTime;

#ifdef DEBUG
        DebugUp();
#endif
        // Sample A/D
        sbi( ADCSRA, ADSC );
        while( !( ADCSRA & ( 0x01 << ADIF ) ) );
#ifdef DEBUG
        DebugDown();
#endif
        mAdSample = ADCH;
        mAdData[0][mAdSampleNumber] = mAdSample;

        // Check for wrap on buffer
        if( ++mAdSampleNumber >= GLOBAL_NUM_SAMPLES )
        {
            // Reset for next pass
            mAdSampleNumber = 0;
            mAdReady = 1;
        }
    }
#endif
}

unsigned char AdReady( void )
{
    return mAdReady;
}

void AdData( unsigned char* data )
{
#ifdef AD_BLOCKING_VERSION
    memcpy( data, (void*)&mAdData[0], GLOBAL_NUM_SAMPLES );
#endif

#ifdef AD_ISR_VERSION
    if( mAdSampleArray )
    {
        memcpy( data, (void*)&mAdData[0], GLOBAL_NUM_SAMPLES );
    }
    else
    {
        memcpy( data, (void*)&mAdData[1], GLOBAL_NUM_SAMPLES );
    }
#endif

    mAdReady = 0;
}

#ifdef AD_ISR_VERSION
ISR( ADC_vect )
{
#ifdef DEBUG
    DebugUp();
#endif
    // Clear Timer1 flag
    sbi( TIFR1, OCF1B );
    // Get AD value
    mAdData[mAdSampleArray][mAdSampleNumber] = ADCH;
    if( ++mAdSampleNumber >= GLOBAL_NUM_SAMPLES )
    {
        mAdSampleNumber = 0;
        mAdSampleArray = mAdSampleArray ? 0 : 1;
        mAdReady = 1;
    }
#ifdef DEBUG
    DebugDown();
#endif
}
#endif

