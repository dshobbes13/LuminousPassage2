
//#include "util.h"
#include "fft.h"
#include "pwm.h"
#include <Wire.h>

#define DEBUG 0

#ifndef cbi
#define cbi( sfr, bit ) ( _SFR_BYTE( sfr ) &= ~_BV( bit ) )
#endif

// Set bit macro for SFRs
#ifndef sbi
#define sbi( sfr, bit ) ( _SFR_BYTE( sfr ) |= _BV( bit ) )
#endif


#define NUM_SAMPLES         64
#define AD_SAMPLE_TIME_US   50
#define AD_INPUT_PIN        0
#define PWM_UPDATE_TIME_US  1250
#define NUM_CHANNELS        128

char mFftDataReal[NUM_SAMPLES];
char mFftDataImag[NUM_SAMPLES];
unsigned int mFftMag[NUM_SAMPLES/2];
unsigned char mDraw = false;


unsigned long mAdTime;
int mAdSample;

unsigned long mPwmTime;

unsigned char mChannelValues[NUM_CHANNELS];

void setup( void )
{

    // Set A/D prescale to 16
    sbi( ADCSRA, ADPS2 );
    cbi( ADCSRA, ADPS1 );
    cbi( ADCSRA, ADPS0 );

    // Set I2C to 400kHz
    Wire.begin();
    cbi( TWSR, TWPS1 );
    cbi( TWSR, TWPS0 );
    TWBR=12;

    // Init serial port for debugging
    Serial.begin( 115200 );
    while( !Serial );

    Serial.print( " TWSR=" );
    Serial.print( TWSR );
    Serial.print( " TWBR=" );
    Serial.print( TWBR );
    Serial.print( "\n\r" );

    // Init variables
    mAdSample = 0;
    mAdTime = micros();
    mPwmTime = micros();
    for( int i=0; i<NUM_CHANNELS; i++ )
    {
        mChannelValues[i] = 128;
    }

    // Init modules
    PwmInit();
}

void loop( void )
{
    static unsigned long totalTime = millis();
    static unsigned long time;
    static unsigned long timeAdCounts = 0;
    static unsigned long timeAdDelays = 0;
    static unsigned long timeAdPeakDelay = 0;
    static unsigned long timeAdBlocks = 0;
    static unsigned long timeFftCounts = 0;
    static unsigned long timeFftBlocks = 0;
    static unsigned long timePwmCounts = 0;
    static unsigned long timePwmDelays = 0;
    static unsigned long timePwmPeakDelay = 0;
    static unsigned long timePwmBlocks = 0;
    static unsigned long adPeak = 0;
    static unsigned long adSum = 0;


    unsigned long currentTime;

    /*
    // Check for A/D sample
    currentTime = micros();
    if( ( currentTime - mAdTime ) > AD_SAMPLE_TIME_US )
    {
        // Debug
        timeAdCounts++;
        unsigned long d = ( currentTime - mAdTime ) - AD_SAMPLE_TIME_US;
        timeAdDelays += d;
        if( d > timeAdPeakDelay )
        {
            timeAdPeakDelay = d;
        }

        // Reset time for next sample
        mAdTime = currentTime;

        // Sample A/D
        time = micros();
        mFftDataReal[mAdSample] = analogRead( AD_INPUT_PIN );
        timeAdBlocks += micros() - time;

        // Check for wrap on buffer
        if( ++mAdSample >= NUM_SAMPLES )
        {
            // Debug
            timeFftCounts++;
            for( int i=0; i<NUM_SAMPLES; i++ )
            {
              if( mFftDataReal[i] > adPeak )
              {
                adPeak = mFftDataReal[i];
              }
              adSum += mFftDataReal[i];
            }

            // Process full buffer
            time = micros();
            fix_fft( mFftDataReal, mFftDataImag, NUM_SAMPLES, 0 );
            timeFftBlocks += micros() - time;

            // Get magnitude
            for( unsigned char i=0; i<(NUM_SAMPLES/2); i++ )
            {
                mFftMag[i] = sqrt( pow( mFftDataReal[i], 2 ) + pow( mFftDataImag[i], 2 ) );
                mDraw = true;
            }

            // Reset for next pass
            mAdSample = 0;
        }
    }
    */

    // Update channel values

    // Check if time to update PWM
    currentTime = micros();
    if( ( currentTime - mPwmTime ) > PWM_UPDATE_TIME_US )
    {
        // Debug
        timePwmCounts++;
        unsigned long d = ( currentTime - mPwmTime ) - PWM_UPDATE_TIME_US;
        timePwmDelays += d;
        if( d > timePwmPeakDelay )
        {
            timePwmPeakDelay = d;
        }

        // Reset time for next update
        mPwmTime = currentTime;

        // Write new values
        time = micros();
        UpdateChannels( mChannelValues );
        timePwmBlocks += micros() - time;

        static int wait = 0;
        if( ++wait >= 32 )
        {
            wait = 0;
            for( int i=0; i<64; i++ )
            {
                mChannelValues[i]++;
                mChannelValues[i+64]++;
                //mChannelValues[i] = 0x20;
                //mChannelValues[i+64] = 0x20;
            }
        }
    }

#if DEBUG
    if( ( millis() - totalTime ) > 5000 )
    {
        totalTime = millis();

        Serial.print( F(" AdCounts=") );
        Serial.print( timeAdCounts );
        Serial.print( F(" AdAvgDelay=") );
        Serial.print( timeAdDelays / timeAdCounts );
        Serial.print( F(" AdPeakDelay=") );
        Serial.print( timeAdPeakDelay );
        Serial.print( F(" AdAvgBlocks=") );
        Serial.print( timeAdBlocks / timeAdCounts );
        Serial.print( F(" FftCounts=") );
        Serial.print( timeFftCounts );
        Serial.print( F(" FftAvgBlocks=") );
        Serial.print( timeFftBlocks / timeFftCounts );
        Serial.print( F(" PwmCounts=") );
        Serial.print( timePwmCounts );
        Serial.print( F(" PwmAvgDelays=") );
        Serial.print( timePwmDelays / timePwmCounts );
        Serial.print( F(" PwmPeakDelay=") );
        Serial.print( timePwmPeakDelay );
        Serial.print( F(" PwmAvgBlocks=") );
        Serial.print( timePwmBlocks / timePwmCounts );
        Serial.print( "\n\r" );

        /*
        Serial.print( F(" AdPeak=") );
        Serial.print( adPeak );
        Serial.print( F(" AdSum=") );
        Serial.print( adSum );
        Serial.print( F(" FftCounts=") );
        Serial.print( timeFftCounts );
        Serial.print( "\n\r" );
        adPeak = 0;
        adSum = 0;
        */

        timeAdCounts = 0;
        timeAdDelays = 0;
        timeAdPeakDelay = 0;
        timeAdBlocks = 0;
        timeFftCounts = 0;
        timeFftBlocks = 0;
        timePwmCounts = 0;
        timePwmDelays = 0;
        timePwmPeakDelay = 0;
        timePwmBlocks = 0;

        /*
        Serial.write( 0x1B );
        Serial.write( '[' );
        Serial.write( '4' );
        Serial.write( 'A' );
        */

        //while( ( millis() - totalTime ) < 100 );
        //totalTime = millis();

        mAdSample = 0;
        mAdTime = micros();
        mPwmTime = micros();
    }
#endif

    /*
    if( mDraw )
    {
        mDraw = false;
        for( int i=0; i<32; i++ )
        {
            Serial.print( mFftMag[i], HEX );
            Serial.print( " :: " );
        }
        Serial.print( "\n\r" );

        Serial.print( 
        Serial.write( 0x1B );
        Serial.write( '[' );
        Serial.write( '4' );
        Serial.write( 'A' );
    }
    */
}

