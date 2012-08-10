// File: usart.h

#ifndef USART_H
#define USART_H

//*****************
// DEFINITIONS
//*****************

#define USART_BUFFER_SIZE_TX   64
#define USART_BUFFER_SIZE_RX   8


//*****************
// PUBLIC FUNCTIONS
//*****************

void UsartInit( void );

unsigned char UsartReadReady( void );

void UsartRead( unsigned char* bytes );

void UsartWrite( unsigned char* bytes );

#endif
