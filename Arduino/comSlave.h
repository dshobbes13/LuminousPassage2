// File: comSlave.h

#ifndef COM_SLAVE_H
#define COM_SLAVE_H

//*****************
// DEFINITIONS
//*****************

//#define COM_SLAVE_TWI_VERSION
//#define COM_SLVAE_BLOCKING_VERSION
#define COM_SLAVE_ISR_VERSION

#define COM_SLAVE_NUM_BYTES    59

//*****************
// PUBLIC FUNCTIONS
//*****************

void ComSlaveInit( void );

void ComSlaveProcess( void );

unsigned char ComSlaveReadReady( void );

void ComSlaveRead( unsigned char* bytes );

#endif
