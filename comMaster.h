// File: pwmCentipede.h

#ifndef PWM_CENTIPEDE_H
#define PWM_CENTIPEDE_H

//*****************
// DEFINITIONS
//*****************

//#define PWM_CENTIPEDE_TWI_VERSION
//#define PWM_CENTIPEDE_BLOCKING_VERSION
#define PWM_CENTIPEDE_ISR_VERSION

#define PWM_CENTIPEDE_NUM_CHANNELS    64

//*****************
// PUBLIC FUNCTIONS
//*****************

void PwmCentipedeInit( void );

void PwmCentipedeProcess( void );

void PwmCentipedeSetChannels( unsigned char* channelValues );

#endif
