// File: comMaster.h

#ifndef COM_MASTER_H
#define COM_MASTER_H

//*****************
// DEFINITIONS
//*****************

//#define COM_MASTER_TWI_VERSION
//#define COM_MASTER_BLOCKING_VERSION
#define COM_MASTER_ISR_VERSION

#define COM_MASTER_NUM_BYTES    59

//*****************
// PUBLIC FUNCTIONS
//*****************

void ComMasterInit( void );

void ComMasterProcess( void );

void ComMasterSendBytes( unsigned char* bytes );

#endif
