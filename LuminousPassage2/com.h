// File: com.h

#ifndef COM_H
#define COM_H

//*****************
// INCLUDES
//*****************


//*****************
// DEFINITIONS
//*****************

enum eCommand
{
    Command_NULL    = 0,
    Command_EFFECT  = 1,    // { Byte1 = Effect, Byte2 = On/Off }
};

//*****************
// PUBLIC FUNCTIONS
//*****************

void ComInit( void );

unsigned char ComProcess( void );

void ComSendFft( unsigned char* data );

eCommand ComGetCommand( void );

unsigned char ComGetByte( unsigned char byteNumber );

void ComPrint( const char* string );

void ComPrintUchar( unsigned char data );

void ComPrintHex( unsigned char data );

void ComPrintUintHex( unsigned int data );

void ComPrintDataChar( const char* label, char* data, unsigned char n );

void ComPrintDataUchar( const char* label, unsigned char* data, unsigned char n );

void ComPrintDataUint( const char* label, unsigned int* data, unsigned char n );


#endif
