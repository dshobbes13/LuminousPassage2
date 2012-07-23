// File: pwm.h

#ifndef PWM_H
#define PWM_H

//*****************
// DEFINITIONS
//*****************

#define PWM_NUM_CHANNELS    10

//*****************
// PUBLIC FUNCTIONS
//*****************

void PwmInit( void );

void PwmProcess( void );

void PwmSetChannels( unsigned char* channelValues );

#endif