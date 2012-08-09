// File: comSlave.cpp

//*****************
// INCLUDES
//*****************

#include "config.h"
#if !defined( MASTER )

#include "comSlave.h"

#include <Arduino.h>

#include "global.h"
#include "utility.h"

#ifdef COM_SLAVE_TWI_VERSION
#include "twi.h"
#endif


//*****************
// DEFINITIONS
//*****************

//#define DEBUG

#define TWSR_STATUS_SL_ADDR         0x60
#define TWSR_STATUS_SL_GCALL        0x70
#define TWSR_STATUS_SL_ADDR_DATA    0x80
#define TWSR_STATUS_SL_GCALL_DATA   0X90
#define TWSR_STATUS_SL_STOP         0xA0

#define TWSR_STATUS     ( TWSR & 0xF8 )


//*****************
// VARIABLES
//*****************

volatile static unsigned char mReadReady = 0;
volatile static unsigned char mCurrentByte = 0;
volatile static unsigned char mBytes[GLOBAL_NUM_CHANNELS] = {0};


//*****************
// PRIVATE PROTOTYPES
//*****************

#ifdef COM_SLAVE_TWI_VERSION
void TwiRead( unsigned char* data, int length );
#endif

#ifdef COM_SLAVE_BLOCKING_VERSION
#endif

#ifdef COM_SLAVE_ISR_VERSION
#endif


//*****************
// PUBLIC
//*****************

void ComSlaveInit( void )
{

#ifdef COM_SLAVE_TWI_VERSION
    twi_init();
    twi_setAddress( 0x01 );
    twi_attachSlaveRxEvent( TwiRead );
    sbi( TWAR, TWGCE );
#endif

#ifdef COM_SLAVE_BLOCKING_VERSION
#endif

    // Set I2C to 400kHz
    cbi( TWSR, TWPS1 );
    cbi( TWSR, TWPS0 );
    TWBR=12;

    digitalWrite( SDA, 1 );
    digitalWrite( SCL, 1 );

#ifdef COM_SLAVE_ISR_VERSION
    // Set broadcast enabled on rx
    TWAR = 0x00;
    sbi( TWAR, TWGCE );

    // Setup TWI for slave receive
    TWCR = _BV( TWINT ) | _BV( TWEA ) | _BV( TWEN ) | _BV( TWIE );
#endif

#ifdef  DEBUG
    DebugInit();
#endif
}

void ComSlaveProcess( void )
{
}

unsigned char ComSlaveReady( void )
{
    return mReadReady;
}

void ComSlaveData( unsigned char* data )
{
    cli();

    memcpy( data, (void*)mBytes, GLOBAL_NUM_CHANNELS );
    mReadReady = 0;

    sei();
}

//*****************
// PRIVATE
//*****************

#ifdef COM_SLAVE_TWI_VERSION
void TwiRead( unsigned char* data, int length )
{
#ifdef DEBUG
    DebugUp();
#endif
    if( ( GLOBAL_NUM_CHANNELS - mCurrentByte ) > length )
    {
        memcpy( (void*)&mBytes[mCurrentByte], data, length );
    }
    mCurrentByte += length;
    if( mCurrentByte >= GLOBAL_NUM_CHANNELS )
    {
        mReadReady = 1;
    }
#ifdef DEBUG
    DebugDown();
#endif
}
#endif

#if defined( COM_SLAVE_ISR_VERSION )
ISR( TWI_vect )
{
#ifdef DEBUG
    DebugUp();
#endif

    switch( TWSR_STATUS )
    {

    case TWSR_STATUS_SL_ADDR:
    case TWSR_STATUS_SL_GCALL:
        // Being address, get ready to read bytes
        mCurrentByte = 0;
        mReadReady = 0;
        TWCR = _BV( TWINT ) | _BV( TWEA ) | _BV( TWEN ) | _BV( TWIE );
        break;

    case TWSR_STATUS_SL_ADDR_DATA:
    case TWSR_STATUS_SL_GCALL_DATA:
        mBytes[mCurrentByte++] = TWDR;
        TWCR = _BV( TWINT ) | _BV( TWEA ) | _BV( TWEN ) | _BV( TWIE );
        break;

    case TWSR_STATUS_SL_STOP:
        if( mCurrentByte >= GLOBAL_NUM_CHANNELS )
        {
            mReadReady = 1;
        }
        mCurrentByte = 0;
        TWCR = _BV( TWINT ) | _BV( TWEA ) | _BV( TWEN ) | _BV( TWIE );
        break;

    default:
        // All other cases, error
        mCurrentByte = 0;
        mReadReady = 0;
        TWCR = _BV( TWINT ) | _BV( TWEA ) | _BV( TWEN ) | _BV( TWIE );
        break;
    }

#ifdef DEBUG
    DebugDown();
#endif
}
#endif

#endif // !MASTER && !MASTER_SINGLE
