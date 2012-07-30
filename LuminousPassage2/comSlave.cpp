// File: comSlave.cpp

//*****************
// INCLUDES
//*****************

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
volatile static unsigned char mBytes[GLOBAL_NUM_SINGLE_CH] = {0};

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
    twi_setAddress( 0x00 );
    twi_attachSlaveRxEvent( TwiRead );
    sbi( TWAR, TWGCE );
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

    // Enable sending acks
    sbi( TWCR, TWEA );

    // Enable TWI and setup for interrupts
    sbi( TWCR, TWEN );
    sbi( TWCR, TWIE );
#endif

#if defined( COM_SLAVE_ISR_VERSION ) && defined( DEBUG )
    DebugInit();
#endif
}

void ComSlaveProcess( void )
{
}

unsigned char ComSlaveReadReady( void )
{
    return mReadReady;
}

void ComSlaveRead( unsigned char* bytes )
{
    memcpy( bytes, (void*)mBytes, GLOBAL_NUM_SINGLE_CH );
    mReadReady = 0;
    mCurrentByte = 0;
}

//*****************
// PRIVATE
//*****************

#ifdef COM_SLAVE_TWI_VERSION
void TwiRead( unsigned char* data, int length )
{
    if( ( GLOBAL_NUM_SINGLE_CH - mCurrentByte ) > length )
    {
        memcpy( &mBytes[mCurrentByte], data, length );
    }
    mCurrentByte += length;
    if( mCurrentByte >= GLOBAL_NUM_SINGLE_CH )
    {
        mReadReady = 1;
    }
}
#endif

#ifdef COM_SLAVE_TWI_VERSION
ISR( TWI_vect )
{
#ifdef DEBUG
    DebugUp();
#endif

    switch( TWSR_STATUS )
    {
    case TWSR_STATUS_SL_ADDR         0x60
    case TWSR_STATUS_SL_GCALL        0x70
    case TWSR_STATUS_SL_ADDR_DATA    0x80
    case TWSR_STATUS_SL_GCALL_DATA   0X90
    case TWSR_STATUS_SL_STOP         0xA0
    }

#ifdef DEBUG
    DebugDown();
#endif
}
#endif

