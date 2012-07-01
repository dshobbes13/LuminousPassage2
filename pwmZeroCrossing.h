// File: pwmZeroCrossing.h

#ifndef PWM_ZERO_CROSSING_H
#define PWM_ZERO_CROSSING_H

//*****************
// DEFINITIONS
//*****************

#define PWM_ZERO_CROSSING_NUM_CHANNELS    8

//*****************
// PUBLIC FUNCTIONS
//*****************

void PwmZeroCrossingInit( void );

void PwmZeroCrossingProcess( void );

void PwmZeroCrossingSetChannels( unsigned char* channelValues );

#endif
