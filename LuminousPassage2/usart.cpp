// File: usart.cpp

//*****************
// INCLUDES
//*****************

#include "usart.h"

#include <Arduino.h>

#include "utility.h"


//*****************
// DEFINITIONS
//*****************

//#define DEBUG


//*****************
// VARIABLES
//*****************

volatile static unsigned char mUsartBytesTx[USART_BUFFER_SIZE_TX] = {0};
volatile static unsigned char mUsartBytesRx[USART_BUFFER_SIZE_RX] = {0};
volatile static unsigned char mUsartCurrentByteTx = 0;
volatile static unsigned char mUsartCurrentByteRx = 0;
volatile static unsigned char mUsartReadReady = 0;


//*****************
// PRIVATE PROTOTYPES
//*****************


//*****************
// PUBLIC
//*****************

void UsartInit( void )
{

    sbi( UCSR0A, U2X0 );    // Double-speed
    sbi( UCSR0C, UCSZ01 );  // 8-bit
    sbi( UCSR0C, UCSZ00 );

    UBRR0L = 16;    // 115200

    sbi( UCSR0B, RXEN0 );   // Turn on Rx
    sbi( UCSR0B, TXEN0 );   // Turn on Tx


    sbi( UCSR0B, RXCIE0 );  // Enable Rx interrupts
#ifdef DEBUG
    DebugInit();
#endif

}

unsigned char UsartReadReady( void )
{
    if( mUsartReadReady )
    {
        mUsartReadReady = 0;
        return 1;
    }
    return 0;
}

void UsartRead( unsigned char* bytes )
{
    memcpy( bytes, (void*)mUsartBytesRx, USART_BUFFER_SIZE_RX );
}

void UsartWrite( unsigned char* bytes )
{
    memcpy( (void*)mUsartBytesTx, bytes, USART_BUFFER_SIZE_TX );
    // Start interrupt
    mUsartCurrentByteTx = 0;
    sbi( UCSR0B, UDRIE0 );
}

/*
ISR( USART_RX_vect )
{
#ifdef DEBUG
    DebugUp();
#endif
    mUsartBytesRx[mUsartCurrentByteRx] = UDR0;
    if( ++mUsartCurrentByteRx >= USART_BUFFER_SIZE_RX )
    {
        mUsartReadReady = 1;
    }
#ifdef DEBUG
    DebugDown();
#endif
}

ISR( USART_UDRE_vect )
{
#ifdef DEBUG
    DebugUp();
#endif
    UDR0 = mUsartBytesTx[mUsartCurrentByteTx];
    if( ++mUsartCurrentByteTx >= USART_BUFFER_SIZE_TX )
    {
        mUsartCurrentByteTx = 0;
        cbi( UCSR0B, UDRIE0 );
    }
#ifdef DEBUG
    DebugDown();
#endif
}
*/
