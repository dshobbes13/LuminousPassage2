
//#include "util.h"
#include "fft.h"
#include "pwm.h"

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
#define PWM_UPDATE_TIME_US  2500
#define NUM_CHANNELS        128

char mFftDataReal[NUM_SAMPLES];
char mFftDataImag[NUM_SAMPLES];

unsigned long mAdTime;
int mAdSample;

unsigned long mPwmTime;

char mChannelValues[NUM_CHANNELS];

void setup( void )
{

  // Set A/D prescale to 16
  sbi( ADCSRA, ADPS2 );
  cbi( ADCSRA, ADPS1 );
  cbi( ADCSRA, ADPS0 );
  
  // Set I2C to 400kHz
  cbi( TWSR, TWPS1 );
  cbi( TWSR, TWPS0 );
  TWBR=25;

  // Init serial port for debugging
  Serial.begin( 115200 );
  while( !Serial );
  
  // Init variables
  mAdSample = 0;
  mAdTime = micros();
  mPwmTime = micros();
  for( int i=0; i<NUM_SAMPLES; i++ )
  {
    mChannelValues[i] = 128;
  }

}

void loop( void )
{
  unsigned long time;
  
  // Check for A/D sample
  if( ( micros() - mAdTime ) > AD_SAMPLE_TIME_US )
  {
    // Reset time for next sample
    mAdTime = micros();
    
    // Sample A/D
    time = micros();
    mFftDataReal[mAdSample] = analogRead( AD_INPUT_PIN );
    Serial.print( micros() - time );
    Serial.print( " " );

    // Check for wrap on buffer
    if( ++mAdSample >= NUM_SAMPLES )
    {
      // Process full buffer
      time = micros();
      fix_fft( mFftDataReal, mFftDataImag, NUM_SAMPLES, 0 );
      Serial.print( "FFT - " );
      Serial.println( micros() - time );

      // Reset for next pass
      mAdSample = 0;
    }
  }
  
  // Update channel values
  
  // Check if time to update PWM
  if( ( micros() - mPwmTime ) > PWM_UPDATE_TIME_US )
  {
    // Reset time for next update
    mPwmTime = micros();
    
    // Write new values
    WriteChannels( mChannelValues );
  }
}

