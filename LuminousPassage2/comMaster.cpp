// File: comMaster.cpp

//*****************
// INCLUDES
//*****************

#include "config.h"
#if defined( MASTER )

#include "comMaster.h"

#include <Arduino.h>

#include "global.h"
#include "utility.h"

#ifdef COM_MASTER_TWI_VERSION
#include "twi.h"
#endif


//*****************
// DEFINITIONS
//*****************

//#define DEBUG

#define TWSR_STATUS_START       0x08
#define TWSR_STATUS_RSTART      0x10
#define TWSR_STATUS_SLAW_ACK    0x18
#define TWSR_STATUS_SLAW_NACK   0x20
#define TWSR_STATUS_DATA_ACK    0x28
#define TWSR_STATUS_DATA_NACK   0x30
#define TWSR_STATUS_LOST        0x38

#define TWSR_STATUS     ( TWSR & 0xF8 )


//*****************
// VARIABLES
//*****************

#ifdef COM_MASTER_ISR_VERSION
volatile static unsigned char mTwiStep = 0;
volatile static unsigned char mCurrentByte = 0;
volatile static unsigned char mBusy = 0;
#endif

volatile static unsigned char mBytes[GLOBAL_NUM_CHANNELS] = {0};

//*****************
// PRIVATE PROTOTYPES
//*****************

#ifdef COM_MASTER_BLOCKING_VERSION
void SendStart( void );
void SendAddress( unsigned char address );
void SendData( unsigned char data );
void SendStop( void );
#endif

#ifdef COM_MASTER_ISR_VERSION
void InitiateTransfer( void );
#endif


//*****************
// PUBLIC
//*****************

void ComMasterInit( void )
{

#ifdef COM_MASTER_TWI_VERSION
    twi_init();
#endif

    // Set I2C to 400kHz
    cbi( TWSR, TWPS1 );
    cbi( TWSR, TWPS0 );
    TWBR=12;

#if defined( COM_MASTER_ISR_VERSION ) && defined( DEBUG )
    DebugInit();
#endif

    digitalWrite( SDA, 1 );
    digitalWrite( SCL, 1 );

#ifdef COM_MASTER_ISR_VERSION
    // Enable TWI and setup for interrupts
    TWCR = _BV( TWEN ) | _BV( TWIE );
#endif
}

void ComMasterProcess( void )
{
}

void ComMasterSendBytes( unsigned char* bytes )
{

#ifdef COM_MASTER_ISR_VERSION
    if( mBusy )
    {
        return;
    }
#endif

    cli();
    memcpy( (void*)mBytes, bytes, GLOBAL_NUM_CHANNELS );
    sei();

#ifdef COM_MASTER_TWI_VERSION
    twi_writeTo( 0x00, mBytes, GLOBAL_NUM_CHANNELS, false, true );
#endif

#ifdef COM_MASTER_BLOCKING_VERSION
    SendStart();
    SendAddress( 0x00 );
    for( unsigned char i=0; i<GLOBAL_NUM_CHANNELS; i++ )
    {
        SendData( mBytes[i] );
    }
    SendStop();
#endif

#ifdef COM_MASTER_ISR_VERSION
    InitiateTransfer();
#endif
}

//*****************
// PRIVATE
//*****************

#ifdef COM_MASTER_BLOCKING_VERSION
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

#if defined( COM_MASTER_ISR_VERSION )
void InitiateTransfer( void )
{
    mTwiStep = 0;
    mCurrentByte = 0;
    mBusy = 1;
    TWCR = _BV( TWINT ) | _BV( TWSTA ) | _BV( TWEN ) | _BV( TWIE );
}
#endif

#if defined( COM_MASTER_ISR_VERSION )
ISR( TWI_vect )
{
#ifdef DEBUG
    DebugUp();
#endif

    switch( mTwiStep )
    {
    case 0:
        if( TWSR_STATUS == TWSR_STATUS_START )
        {
            // Send address
            TWDR = (unsigned char)0x00;
            TWCR = _BV( TWINT ) | _BV( TWEN ) | _BV( TWIE );
            mTwiStep++;
        }
        else
        {
            // Failed to start
            TWCR = ~_BV( TWEN ) & ~_BV( TWIE );
            mTwiStep = 0;
            mCurrentByte = 0;
            mBusy = 0;
        }
        break;
    case 1:
        if( ( TWSR_STATUS == TWSR_STATUS_SLAW_ACK ) || ( TWSR_STATUS == TWSR_STATUS_DATA_ACK ) )
        {
            // Send data bytes
            TWDR = (unsigned char)mBytes[mCurrentByte];
            TWCR = _BV( TWINT ) | _BV( TWEN ) | _BV( TWIE );
            if( ++mCurrentByte >= GLOBAL_NUM_CHANNELS )
            {
                mTwiStep++;
            }
        }
        else
        {
            // Failed to send address/data
            TWCR = ~_BV( TWEN ) & ~_BV( TWIE );
            mTwiStep = 0;
            mCurrentByte = 0;
            mBusy = 0;
        }
        break;
    case 2:
        if( TWSR_STATUS == TWSR_STATUS_DATA_ACK )
        {
            // Send stop byte and wait
            TWCR = _BV( TWINT ) | _BV( TWSTO ) | _BV( TWEN ) | _BV( TWIE );
            while( TWCR & _BV( TWSTO ) );
            mTwiStep = 0;
            mCurrentByte = 0;
            mBusy = 0;
        }
        else
        {
            // Failed to send last data byte
            TWCR = ~_BV( TWEN ) & ~_BV( TWIE );
            mTwiStep = 0;
            mCurrentByte = 0;
            mBusy = 0;
        }
        break;
    default:
        // Bad, reset to start
        TWCR = ~_BV( TWEN ) & ~_BV( TWIE );
        mTwiStep = 0;
        mCurrentByte = 0;
        mBusy = 0;
        break;
    }
#ifdef DEBUG
    DebugDown();
#endif
}
#endif

#endif // MASTER
