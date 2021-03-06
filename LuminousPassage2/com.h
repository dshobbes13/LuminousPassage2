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
    Command_NULL            = 0,
    Command_SAVE            = 1,    // { }
    Command_LOAD            = 2,    // { }
    Command_AUDIO           = 3,    // { Treshold, Averaging, Lo0, Hi0, Lo1, Hi1, Lo2, Hi2, Lo3, Hi3, Lo4, Hi4, Lo5, Hi5 }
    Command_EFFECT          = 4,    // { Byte1 = Effect, Byte2 = On/Off }
    Command_BUCKETS         = 5,    // { Byte1 = Hysteresis, Byte2 = SecondsFlags, Byte3 = Seconds }
    Command_MANUAL          = 6,    // { Byte1 = Value }
    Command_PULSE_SQUARE    = 7,    // { Byte1 = Source, Byte2 = Length, Byte3 = Width }
    Command_PULSE_SINE      = 8,    // { Byte1 = Source, Byte2 = Length, Byte3 = Width, Byte4 = Speed }
    Command_DISTANCE_SQUARE = 9,    // { Byte1 = Source, Byte2 = Start, Byte3 = Stop, Byte4 = Amp }
    Command_SWING           = 10,   // { Byte1 = Source, Byte2 = Start, Byte3 = Stop, Byte4 = PeriodHi, Byte5 = PeriodLo }
    Command_PULSE_CENTER    = 11,   // { Byte1 = Source, Byte2 = Width, Byte3 = Speed }
    Command_DROP_CYCLE      = 12,   // { Byte1 = Source, Byte2 = Speed }
    Command_PULSE_RIGHT     = 13,   // { Byte1 = Source, Byte2 = Length, Byte3 = Width, Byte4 = Speed }
};

#define MESSAGE_LENGTH  16

//*****************
// PUBLIC FUNCTIONS
//*****************

#ifndef SOFTWARE

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

#endif
