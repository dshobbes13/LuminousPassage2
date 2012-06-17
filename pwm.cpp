// File: pwm.cpp

//*****************
// INCLUDES
//*****************

#include "pwm.h"

#include <Arduino.h>

#include "utility.h"

#ifdef PWM_TWI_VERSION
#include "twi.h"
#endif


//*****************
// DEFINITIONS
//*****************

#define DEBUG

#define PWM_UPDATE_TIME_US  1250

#define IC1 0x20
#define IC2 0x21
#define IC3 0x22
#define IC4 0x23
#define IC5 0x20
#define IC6 0x21
#define IC7 0x22
#define IC8 0x23

#define REG_IODIRA 0x00
#define REG_IDDIRB 0x01
#define REG_GPIOA  0x12
#define REG_GPIOB  0x13

#define TWSR_STATUS_START   0x08
#define TWSR_STATUS_RSTART  0x10
#define TWSR_STATUS_SLAW_ACK    0x18
#define TWSR_STATUS_SLAW_NACK   0x20
#define TWSR_STATUS_DATA_ACK    0x28
#define TWSR_STATUS_DATA_NACK   0x30
#define TWSR_STATUS_LOST        0x38

#define TWSR_STATUS     ( TWSR & 0xF8 )


//*****************
// VARIABLES
//*****************

#ifdef PWM_TWI_VERSION
unsigned static char mBuffer[3];
#endif

#ifdef PWM_ISR_VERSION
volatile static unsigned char mIcStep = 0;
volatile static unsigned char mTwiStep = 0;
volatile static unsigned char mAddresses[6] = {0};
volatile static unsigned char mBusy = 0;
volatile static unsigned char mStatus = 0;
#endif

volatile static unsigned char mChannelValues[PWM_NUM_CHANNELS] = {0};
volatile static unsigned char mBytes[PWM_NUM_CHANNELS/8] = {0};


//*****************
// PRIVATE PROTOTYPES
//*****************

#ifdef PWM_BLOCKING_VERSION
void SendStart( void );
void SendAddress( unsigned char address );
void SendData( unsigned char data );
void SendStop( void );
#endif

#if defined( PWM_TWI_VERSION ) || defined( PWM_BLOCKING_VERSION )
void PwmUpdateChannels( void );
#endif

#ifdef PWM_ISR_VERSION
void InitiateTransfer( void );
#endif

void PwmUpdateBytes( void );

//*****************
// PUBLIC
//*****************

void PwmInit( void )
{

#ifdef PWM_TWI_VERSION
    twi_init();
#endif

    // Set I2C to 400kHz
    cbi( TWSR, TWPS1 );
    cbi( TWSR, TWPS0 );
    TWBR=12;

#if defined( PWM_ISR_VERSION ) && defined( DEBUG )
    // Debug
    DDRD = DDRD | 0x04;
    PORTD = PORTD & ~0x04;
#endif

    digitalWrite( SDA, 1 );
    digitalWrite( SCL, 1 );

#ifdef PWM_TWI_VERSION
    mBuffer[0] = REG_IODIRA;
    mBuffer[1] = 0x00;
    mBuffer[2] = 0x00;

    twi_writeTo( IC1, mBuffer, 3, false, true );
    twi_writeTo( IC2, mBuffer, 3, false, true );
    twi_writeTo( IC3, mBuffer, 3, false, true );
    twi_writeTo( IC4, mBuffer, 3, false, true );
    twi_writeTo( IC5, mBuffer, 3, false, true );
    twi_writeTo( IC6, mBuffer, 3, false, true );
#endif

#ifdef PWM_BLOCKING_VERSION
    SendStart();
    SendAddress( IC1 );
    SendData( REG_IODIRA );
    SendData( 0x00 );
    SendData( 0x00 );
    SendStop();

    SendStart();
    SendAddress( IC2 );
    SendData( REG_IODIRA );
    SendData( 0x00 );
    SendData( 0x00 );
    SendStop();

    SendStart();
    SendAddress( IC3 );
    SendData( REG_IODIRA );
    SendData( 0x00 );
    SendData( 0x00 );
    SendStop();

    SendStart();
    SendAddress( IC4 );
    SendData( REG_IODIRA );
    SendData( 0x00 );
    SendData( 0x00 );
    SendStop();

    SendStart();
    SendAddress( IC5 );
    SendData( REG_IODIRA );
    SendData( 0x00 );
    SendData( 0x00 );
    SendStop();

    SendStart();
    SendAddress( IC6 );
    SendData( REG_IODIRA );
    SendData( 0x00 );
    SendData( 0x00 );
    SendStop();
#endif

#ifdef PWM_ISR_VERSION
    // Init variables
    mAddresses[0] = IC1 << 1;
    mAddresses[1] = IC2 << 1;
    mAddresses[2] = IC3 << 1;
    mAddresses[3] = IC4 << 1;
    mAddresses[4] = IC5 << 1;
    mAddresses[5] = IC6 << 1;

    // Enable TWI and setup for interrupts
    TWCR = _BV( TWEN ) | _BV( TWIE );

    // Initialize for output pins
    for( unsigned char i=0; i<16; i++ )
    {
        mBytes[i] = 0x00;
    }
    InitiateTransfer();
    while( mBusy );

    // Start TIMER2 for interrupting every 1.25ms
    TCCR2A = 0x02;
    TCCR2B = 0x05;
    TCNT2 = 0x00;
    OCR2A = 156;
    TIMSK2 = 0x02;
#endif
}

void PwmProcess( void )
{
#if defined( PWM_TWI_VERSION ) || defined( PWM_BLOCKING_VERSION )
    static unsigned long mPwmTime = micros();
    static unsigned long mCurrentTime = micros();

    mCurrentTime = micros();
    if( ( mCurrentTime - mPwmTime ) > PWM_UPDATE_TIME_US )
    {
        // Reset time for next update
        mPwmTime = mCurrentTime;
        // Update channel values
        PwmUpdateChannels();
    }
#endif
}

void PwmSetChannels( unsigned char* channelValues )
{
    cli();
    memcpy( (void*)mChannelValues, channelValues, PWM_NUM_CHANNELS );
    sei();
}

//*****************
// PRIVATE
//*****************

#ifdef PWM_BLOCKING_VERSION
void SendStart( void )
{
    TWCR = _BV( TWINT ) | _BV( TWSTA ) | _BV( TWEN );
    while( !( TWCR & _BV( TWINT ) ) );
}

void SendAddress( unsigned char address )
{
    TWDR = ( address << 1 );
    TWCR = _BV( TWINT ) | _BV( TWEN );
    while( !( TWCR & _BV( TWINT ) ) );
}

void SendData( unsigned char data )
{
    TWDR = data;
    TWCR = _BV( TWINT ) | _BV( TWEN );
    while( !( TWCR & _BV( TWINT ) ) );
}

void SendStop( void )
{
    TWCR = _BV( TWINT ) | _BV( TWSTO ) | _BV( TWEN );
    while( TWCR & _BV( TWSTO ) );
}
#endif

void PwmUpdateBytes( void )
{
    static unsigned char mStepCount = 0;
    static unsigned char mThreshold = 0;

    switch( mStepCount )
    {
    case 0x00: mThreshold = 0x0F; break;
    case 0x01: mThreshold = 0x1E; break;
    case 0x02: mThreshold = 0x2D; break;
    case 0x03: mThreshold = 0x3C; break;
    case 0x04: mThreshold = 0x4B; break;
    case 0x05: mThreshold = 0x5A; break;
    case 0x06: mThreshold = 0x69; break;
    case 0x07: mThreshold = 0x78; break;
    case 0x08: mThreshold = 0x87; break;
    case 0x09: mThreshold = 0x96; break;
    case 0x0A: mThreshold = 0xA5; break;
    case 0x0B: mThreshold = 0xB4; break;
    case 0x0C: mThreshold = 0xC3; break;
    case 0x0D: mThreshold = 0xD2; break;
    case 0x0E: mThreshold = 0xE1; break;
    case 0x0F: mThreshold = 0xF0; break;
    default: break;
    }

    for( unsigned char i=0; i<(PWM_NUM_CHANNELS/8); i++ )
    {
        mBytes[i] = 0xFF;
        mBytes[i] &= ( mChannelValues[i*8 + 0] > mThreshold ) ? ~0x01 : 0xFF;
        mBytes[i] &= ( mChannelValues[i*8 + 1] > mThreshold ) ? ~0x02 : 0xFF;
        mBytes[i] &= ( mChannelValues[i*8 + 2] > mThreshold ) ? ~0x04 : 0xFF;
        mBytes[i] &= ( mChannelValues[i*8 + 3] > mThreshold ) ? ~0x08 : 0xFF;
        mBytes[i] &= ( mChannelValues[i*8 + 4] > mThreshold ) ? ~0x10 : 0xFF;
        mBytes[i] &= ( mChannelValues[i*8 + 5] > mThreshold ) ? ~0x20 : 0xFF;
        mBytes[i] &= ( mChannelValues[i*8 + 6] > mThreshold ) ? ~0x40 : 0xFF;
        mBytes[i] &= ( mChannelValues[i*8 + 7] > mThreshold ) ? ~0x80 : 0xFF;
    }

    if( ++mStepCount >= 16 )
    {
        mStepCount = 0;
    }
}

#if defined( PWM_TWI_VERSION ) || defined( PWM_BLOCKING_VERSION )
void PwmUpdateChannels( void )
{
    PwmUpdateBytes();

#ifdef PWM_TWI_VERSION
    mBuffer[0] = REG_GPIOA;

    mBuffer[1] = mBytes[0];
    mBuffer[2] = mBytes[1];
    twi_writeTo( IC1, mBuffer, 3, false, true );

    mBuffer[1] = mBytes[2];
    mBuffer[2] = mBytes[3];
    twi_writeTo( IC2, mBuffer, 3, false, true );

    mBuffer[1] = mBytes[4];
    mBuffer[2] = mBytes[5];
    twi_writeTo( IC3, mBuffer, 3, false, true );

    mBuffer[1] = mBytes[6];
    mBuffer[2] = mBytes[7];
    twi_writeTo( IC4, mBuffer, 3, false, true );

    mBuffer[1] = mBytes[8];
    mBuffer[2] = mBytes[9];
    twi_writeTo( IC5, mBuffer, 3, false, true );

    mBuffer[1] = mBytes[10];
    mBuffer[2] = mBytes[11];
    twi_writeTo( IC6, mBuffer, 3, false, true );
#endif

#ifdef PWM_BLOCKING_VERSION
    SendStart();
    SendAddress( IC1 );
    SendData( REG_GPIOA );
    SendData( mBytes[0] );
    SendData( mBytes[1] );
    SendStop();

    SendStart();
    SendAddress( IC2 );
    SendData( REG_GPIOA );
    SendData( mBytes[2] );
    SendData( mBytes[3] );
    SendStop();

    SendStart();
    SendAddress( IC3 );
    SendData( REG_GPIOA );
    SendData( mBytes[4] );
    SendData( mBytes[5] );
    SendStop();

    SendStart();
    SendAddress( IC4 );
    SendData( REG_GPIOA );
    SendData( mBytes[6] );
    SendData( mBytes[7] );
    SendStop();

    SendStart();
    SendAddress( IC5 );
    SendData( REG_GPIOA );
    SendData( mBytes[8] );
    SendData( mBytes[9] );
    SendStop();

    SendStart();
    SendAddress( IC6 );
    SendData( REG_GPIOA );
    SendData( mBytes[10] );
    SendData( mBytes[11] );
    SendStop();
#endif
}
#endif

#ifdef PWM_ISR_VERSION
void InitiateTransfer( void )
{
    mBusy = 1;
    TWCR = _BV( TWINT ) | _BV( TWSTA ) | _BV( TWEN ) | _BV( TWIE );
}

ISR( TIMER2_COMPA_vect )
{
    PORTD = PORTD | 0x04;
    PwmUpdateBytes();
    InitiateTransfer();
    PORTD = PORTD & ~0x04;
}

ISR( TWI_vect )
{
    PORTD = PORTD | 0x04;
    switch( mTwiStep )
    {
    case 0:
        // Send address
        TWDR = (unsigned char)mAddresses[mIcStep];
        TWCR = _BV( TWINT ) | _BV( TWEN ) | _BV( TWIE );
        mTwiStep++;
        break;
    case 1:
        // Send register
        TWDR = (unsigned char)REG_GPIOA;
        TWCR = _BV( TWINT ) | _BV( TWEN ) | _BV( TWIE );
        mTwiStep++;
        break;
    case 2:
        // Send 1st byte
        TWDR = (unsigned char)mBytes[mIcStep*2];
        TWCR = _BV( TWINT ) | _BV( TWEN ) | _BV( TWIE );
        mTwiStep++;
        break;
    case 3:
        // Send 2nd byte
        TWDR = (unsigned char)mBytes[mIcStep*2 + 1];
        TWCR = _BV( TWINT ) | _BV( TWEN ) | _BV( TWIE );
        mTwiStep++;
        break;
    case 4:
        // Send stop byte and wait
        TWCR = _BV( TWINT ) | _BV( TWSTO ) | _BV( TWEN ) | _BV( TWIE );
        while( TWCR & _BV( TWSTO ) );
        mTwiStep = 0;
        // Check if next ic
        if( ++mIcStep < 6 )
        {
            // Send next start
            TWCR = _BV( TWINT ) | _BV( TWSTA ) | _BV( TWEN ) | _BV( TWIE );
        }
        else
        {
            mIcStep = 0;
            mBusy = 0;
        }
        break;
    default:
        // Bad, reset to start
        TWCR = ~_BV( TWEN ) & ~_BV( TWIE );
        break;
    }
    PORTD = PORTD & ~0x04;
}
#endif

