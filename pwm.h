// File: pwm.h

#ifndef PWM_H
#define PWM_H

//*****************
// DEFINITIONS
//*****************

//#define PWM_TWI_VERSION
//#define PWM_BLOCKING_VERSION
#define PWM_ISR_VERSION

//*****************
// PUBLIC FUNCTIONS
//*****************

void PwmInit( void );

void PwmUpdateChannels( unsigned char* channelValues );

#endif
