
#include "pwm.h"

#include <Arduino.h>

#include "utility.h"

//#define TWI
#define BLOCK

#ifdef TWI
#include "twi.h"
#endif

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

#define NUM_STEPS  8

#define TWSR_STATUS_START   0x08
#define TWSR_STATUS_RSTART  0x10
#define TWSR_STATUS_SLAW_ACK    0x18
#define TWSR_STATUS_SLAW_NACK   0x20
#define TWSR_STATUS_DATA_ACK    0x28
#define TWSR_STATUS_DATA_NACK   0x30
#define TWSR_STATUS_LOST        0x38

#define TWSR_STATUS     ( TWSR & 0xF8 )

unsigned char buffer[3];

volatile unsigned char mIcStep = 0;
volatile unsigned char mTwiStep = 0;
volatile unsigned char mAddresses[6] = {0};
volatile unsigned char mBytes[16] = {0};

void SendStart( void );
void SendAddress( unsigned char address );
void SendData( unsigned char data );
void SendStop( void );

void PwmInit( void )
{

    // Set I2C to 400kHz
#ifdef TWI
    twi_init();
#endif
    cbi( TWSR, TWPS1 );
    cbi( TWSR, TWPS0 );
    TWBR=12;
    TWCR = _BV( TWEN );

    digitalWrite( SDA, 1 );
    digitalWrite( SCL, 1 );

    buffer[0] = REG_IODIRA;
    buffer[1] = 0x00;
    buffer[2] = 0x00;

#ifdef TWI
    twi_writeTo( IC1, buffer, 3, false, true );
    twi_writeTo( IC2, buffer, 3, false, true );
    twi_writeTo( IC3, buffer, 3, false, true );
    twi_writeTo( IC4, buffer, 3, false, true );
    twi_writeTo( IC5, buffer, 3, false, true );
    twi_writeTo( IC6, buffer, 3, false, true );
#else
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

    mAddresses[0] = IC1 << 1;
    mAddresses[1] = IC2 << 1;
    mAddresses[2] = IC3 << 1;
    mAddresses[3] = IC4 << 1;
    mAddresses[4] = IC5 << 1;
    mAddresses[5] = IC6 << 1;

#ifndef BLOCK
    TWCR = _BV( TWEN ) | _BV( TWIE );
#endif
}

void UpdateChannels( unsigned char* channelValues )
{
    static byte mStepCount = 0;
    static byte mThreshold = 0;

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

    for( int i=0; i<16; i++ )
    {
        mBytes[i] = 0xFF;
        mBytes[i] &= ( channelValues[i*8 + 0] > mThreshold ) ? ~0x01 : 0xFF;
        mBytes[i] &= ( channelValues[i*8 + 1] > mThreshold ) ? ~0x02 : 0xFF;
        mBytes[i] &= ( channelValues[i*8 + 2] > mThreshold ) ? ~0x04 : 0xFF;
        mBytes[i] &= ( channelValues[i*8 + 3] > mThreshold ) ? ~0x08 : 0xFF;
        mBytes[i] &= ( channelValues[i*8 + 4] > mThreshold ) ? ~0x10 : 0xFF;
        mBytes[i] &= ( channelValues[i*8 + 5] > mThreshold ) ? ~0x20 : 0xFF;
        mBytes[i] &= ( channelValues[i*8 + 6] > mThreshold ) ? ~0x40 : 0xFF;
        mBytes[i] &= ( channelValues[i*8 + 7] > mThreshold ) ? ~0x80 : 0xFF;
    }

    if( ++mStepCount >= 16 )
    {
        mStepCount = 0;
    }

#ifdef TWI
    buffer[0] = REG_GPIOA;

    buffer[1] = mBytes[0];
    buffer[2] = mBytes[1];
    twi_writeTo( IC1, buffer, 3, false, true );

    buffer[1] = mBytes[2];
    buffer[2] = mBytes[3];
    twi_writeTo( IC2, buffer, 3, false, true );

    buffer[1] = mBytes[4];
    buffer[2] = mBytes[5];
    twi_writeTo( IC3, buffer, 3, false, true );

    buffer[1] = mBytes[6];
    buffer[2] = mBytes[7];
    twi_writeTo( IC4, buffer, 3, false, true );

    buffer[1] = mBytes[8];
    buffer[2] = mBytes[9];
    twi_writeTo( IC5, buffer, 3, false, true );

    buffer[1] = mBytes[10];
    buffer[2] = mBytes[11];
    twi_writeTo( IC6, buffer, 3, false, true );

#elif defined(BLOCK)

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
#else
    // Start new ISR transfer
    TWCR = _BV( TWINT ) | _BV( TWSTA ) | _BV( TWEN ) | _BV( TWIE );
#endif
}

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

ISR( TWI_vect )
{
    switch( mTwiStep )
    {
    case 0:
        // Send address
        TWDR = mAddresses[mIcStep];
        TWCR = _BV( TWINT ) | _BV( TWEN ) | _BV( TWIE );
        mTwiStep++;
        break;
    case 1:
        // Send 1st byte
        TWDR = mBytes[mIcStep*2];
        TWCR = _BV( TWINT ) | _BV( TWEN ) | _BV( TWIE );
        mTwiStep++;
        break;
    case 2:
        // Send 2nd byte
        TWDR = mBytes[mIcStep*2 + 1];
        TWCR = _BV( TWINT ) | _BV( TWEN ) | _BV( TWIE );
        mTwiStep++;
        break;
    case 3:
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
        }
    }
}

