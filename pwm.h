// File: pwm.h

#ifndef PWM_H
#define PWM_H

//*****************
// DEFINITIONS
//*****************

//#define PWM_TWI_VERSION
//#define PWM_BLOCKING_VERSION
#define PWM_ISR_VERSION

#define PWM_NUM_CHANNELS    96

//*****************
// PUBLIC FUNCTIONS
//*****************

void PwmInit( void );

void PwmProcess( void );

void PwmSetChannels( unsigned char* channelValues );

#endif
