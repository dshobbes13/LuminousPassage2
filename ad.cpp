// File: ad.cpp

//*****************
// INCLUDES
//*****************

#include "ad.h"

#include <Arduino.h>

#include "utility.h"


//*****************
// DEFINITIONS
//*****************

#define DEBUG

#define AD_SAMPLE_TIME_US   50
#define AD_INPUT_PIN        0

//*****************
// VARIABLES
//*****************

volatile static unsigned char mAdData[2][AD_NUM_SAMPLES] = {0};
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
    sbi( ADMUX, ADLAR );    // Left justified
    sbi( ADCSRA, ADEN );    // Turn AD on
    sbi( ADCSRA, ADPS2 );   // Set pre-scale to 16
    cbi( ADCSRA, ADPS1 );
    cbi( ADCSRA, ADPS0 );
    sbi( ADCSRA, ADIF );    // Clear interrupt flag

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
    OCR1AH = 0x03;
    OCR1AL = 0x20;
    OCR1BH = 0x03;
    OCR1BL = 0x20;
    ICR1H = 0x00;
    ICR1L = 0x00;

    Serial.print( "\n\r" );
    // Initialize variables

    // Start sampling
    sbi( ADCSRA, ADSC );
    sbi( ADCSRA, ADIE );
#endif

#if defined( AD_ISR_VERSION ) && defined( DEBUG )
    // Debug
    DDRD = DDRD | 0x04;
    PORTD = PORTD & ~0x04;
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

        // Sample A/D
        sbi( ADCSRA, ADSC );
        while( !( ADCSRA & ( 0x01 << ADIF ) ) );
        mAdSample = ADCH;
        mAdData[0][mAdSampleNumber] = mAdSample;

        // Check for wrap on buffer
        if( ++mAdSampleNumber >= AD_NUM_SAMPLES )
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
    if( mAdReady )
    {
        mAdReady = 0;
        return 1;
    }
    return 0;
}

void AdData( unsigned char* data )
{
#ifdef AD_BLOCKING_VERSION
    memcpy( data, (void*)&mAdData[0], AD_NUM_SAMPLES );
#endif

#ifdef AD_ISR_VERSION
    if( mAdSampleArray )
    {
        memcpy( data, (void*)&mAdData[0], AD_NUM_SAMPLES );
    }
    else
    {
        memcpy( data, (void*)&mAdData[1], AD_NUM_SAMPLES );
    }
#endif
}

#ifdef AD_ISR_VERSION
ISR( ADC_vect )
{
    PORTD = PORTD | 0x04;
    // Clear Timer1 flag
    sbi( TIFR1, OCF1B );
    // Get AD value
    mAdData[mAdSampleArray][mAdSampleNumber] = ADCH;
    if( ++mAdSampleNumber >= AD_NUM_SAMPLES )
    {
        mAdSampleNumber = 0;
        mAdSampleArray = mAdSampleArray ? 0 : 1;
        mAdReady = 1;
    }
    PORTD = PORTD & ~0x04;
}
#endif

