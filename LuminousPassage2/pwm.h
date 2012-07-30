// File: pwm.h

#ifndef PWM_H
#define PWM_H

//*****************
// DEFINITIONS
//*****************


//*****************
// PUBLIC FUNCTIONS
//*****************

void PwmInit( void );

void PwmProcess( void );

void PwmSetChannels( unsigned char* channelValues );

#endif
