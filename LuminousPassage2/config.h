// File: config.h

#ifndef CONFIG_H
#define CONFIG_H

//*****************
// DEFINITIONS
//*****************

#define FIRMWARE

#define MAPPING

//#define MASTER_SINGLE
#define MASTER
//#define SLAVE_0
//#define SLAVE_1
//#define SLAVE_2
//#define SLAVE_3
//#define SLAVE_4
//#define SLAVE_5


#define EEPROM_AUDIO_THRESHOLD      0x000
#define EEPROM_AUDIO_AVERAGING      0x001
#define EEPROM_AUDIO_BUCKETS_LO_0   0x002
#define EEPROM_AUDIO_BUCKETS_HI_0   0x003
#define EEPROM_AUDIO_BUCKETS_LO_1   0x004
#define EEPROM_AUDIO_BUCKETS_HI_1   0x005
#define EEPROM_AUDIO_BUCKETS_LO_2   0x006
#define EEPROM_AUDIO_BUCKETS_HI_2   0x007
#define EEPROM_AUDIO_BUCKETS_LO_3   0x008
#define EEPROM_AUDIO_BUCKETS_HI_3   0x009
#define EEPROM_AUDIO_BUCKETS_LO_4   0x00A
#define EEPROM_AUDIO_BUCKETS_HI_4   0x00B
#define EEPROM_AUDIO_BUCKETS_LO_5   0x00C
#define EEPROM_AUDIO_BUCKETS_HI_5   0x00D

#define EEPROM_PATTERN_EFFECT_START 0x100
#define EEPROM_PATTERN_EFFECT_0     0x100
#define EEPROM_PATTERN_EFFECT_1     0x101
#define EEPROM_PATTERN_EFFECT_2     0x102
#define EEPROM_PATTERN_EFFECT_3     0x103
#define EEPROM_PATTERN_EFFECT_4     0x104
#define EEPROM_PATTERN_EFFECT_5     0x105
#define EEPROM_PATTERN_EFFECT_6     0x106
#define EEPROM_PATTERN_EFFECT_7     0x107
#define EEPROM_PATTERN_EFFECT_8     0x108

#define EEPROM_PATTERN_MANUAL_VALUE     0x200

#define EEPROM_PATTERN_BUCKET_HYSTERESIS    0x210
#define EEPROM_PATTERN_BUCKET_TIME_FLAGS    0x211
#define EEPROM_PATTERN_BUCKET_TIME_SECS     0x212

#define EEPROM_PATTERN_PULSE_SQUARE_SOURCE  0x220
#define EEPROM_PATTERN_PULSE_SQUARE_LENGTH  0x221
#define EEPROM_PATTERN_PULSE_SQUARE_WIDTH   0x222

#define EEPROM_PATTERN_PULSE_SINE_SOURCE    0x230
#define EEPROM_PATTERN_PULSE_SINE_LENGTH    0x231
#define EEPROM_PATTERN_PULSE_SINE_WIDTH     0x232
#define EEPROM_PATTERN_PULSE_SINE_SPEED     0x233

#define EEPROM_PATTERN_DISTANCE_SQUARE_SOURCE   0x240
#define EEPROM_PATTERN_DISTANCE_SQUARE_START    0x241
#define EEPROM_PATTERN_DISTANCE_SQUARE_STOP     0x242
#define EEPROM_PATTERN_DISTANCE_SQUARE_AMP      0x243

#define EEPROM_PATTERN_SWING_SOURCE         0x250
#define EEPROM_PATTERN_SWING_START          0x251
#define EEPROM_PATTERN_SWING_STOP           0x252
#define EEPROM_PATTERN_SWING_PERIOD_HI      0x253
#define EEPROM_PATTERN_SWING_PERIOD_LO      0x254

#define EEPROM_PATTERN_PULSE_CENTER_SOURCE    0x260
#define EEPROM_PATTERN_PULSE_CENTER_WIDTH     0x261
#define EEPROM_PATTERN_PULSE_CENTER_SPEED     0x262

#define EEPROM_PATTERN_DROP_CYCLE_SOURCE    0x270
#define EEPROM_PATTERN_DROP_CYCLE_SPEED     0x271

#define EEPROM_PATTERN_PULSE_RIGHT_SOURCE    0x280
#define EEPROM_PATTERN_PULSE_RIGHT_LENGTH    0x281
#define EEPROM_PATTERN_PULSE_RIGHT_WIDTH     0x282
#define EEPROM_PATTERN_PULSE_RIGHT_SPEED     0x283

#if defined( MAPPING )

#if defined( MASTER_SINGLE )

#define CH0 0
#define CH1 1
#define CH2 2
#define CH3 3
#define CH4 4
#define CH5 5
#define CH6 6
#define CH7 7
#define CH8 8
#define CH9 9

#elif defined( MASTER )

#define CH0 0
#define CH1 1
#define CH2 2
#define CH3 3
#define CH4 4
#define CH5 5
#define CH6 6
#define CH7 7
#define CH8 8
#define CH9 9

#elif defined( SLAVE_0 )

#define CH0 58
#define CH1 5
#define CH2 11
#define CH3 57
#define CH4 12
#define CH5 34
#define CH6 45
#define CH7 27
#define CH8 20
#define CH9 41

#elif defined( SLAVE_1 )

#define CH0 56
#define CH1 4
#define CH2 10
#define CH3 55
#define CH4 13
#define CH5 28
#define CH6 26
#define CH7 40
#define CH8 35
#define CH9 22

#elif defined( SLAVE_2 )

#define CH0 54
#define CH1 3
#define CH2 9
#define CH3 53
#define CH4 14
#define CH5 36
#define CH6 32
#define CH7 25
#define CH8 19
#define CH9 43

#elif defined( SLAVE_3 )

#define CH0 52
#define CH1 2
#define CH2 8
#define CH3 51
#define CH4 15
#define CH5 37
#define CH6 33
#define CH7 24
#define CH8 18
#define CH9 42

#elif defined( SLAVE_4 )

#define CH0 50
#define CH1 1
#define CH2 7
#define CH3 46
#define CH4 16
#define CH5 38
#define CH6 47
#define CH7 23
#define CH8 30
#define CH9 31

#elif defined( SLAVE_5 )

#define CH0 48
#define CH1 0
#define CH2 6
#define CH3 49
#define CH4 17
#define CH5 39
#define CH6 44
#define CH7 29
#define CH8 21
#define CH9 21

#else

#define CH0 0
#define CH1 0
#define CH2 0
#define CH3 0
#define CH4 0
#define CH5 0
#define CH6 0
#define CH7 0
#define CH8 0
#define CH9 0

#endif

#else // MAPPING

#if defined( MASTER_SINGLE )

#define CH0 0
#define CH1 1
#define CH2 2
#define CH3 3
#define CH4 4
#define CH5 5
#define CH6 6
#define CH7 7
#define CH8 8
#define CH9 9

#elif defined( MASTER )

#define CH0 0
#define CH1 1
#define CH2 2
#define CH3 3
#define CH4 4
#define CH5 5
#define CH6 6
#define CH7 7
#define CH8 8
#define CH9 9

#elif defined( SLAVE_0 )

#define CH0 0
#define CH1 1
#define CH2 2
#define CH3 3
#define CH4 4
#define CH5 5
#define CH6 6
#define CH7 7
#define CH8 8
#define CH9 9

#elif defined( SLAVE_1 )

#define CH0 10
#define CH1 11
#define CH2 12
#define CH3 13
#define CH4 14
#define CH5 15
#define CH6 16
#define CH7 17
#define CH8 18
#define CH9 19

#elif defined( SLAVE_2 )

#define CH0 20
#define CH1 21
#define CH2 22
#define CH3 23
#define CH4 24
#define CH5 25
#define CH6 26
#define CH7 27
#define CH8 28
#define CH9 29

#elif defined( SLAVE_3 )

#define CH0 30
#define CH1 31
#define CH2 32
#define CH3 33
#define CH4 34
#define CH5 35
#define CH6 36
#define CH7 37
#define CH8 38
#define CH9 39

#elif defined( SLAVE_4 )

#define CH0 40
#define CH1 41
#define CH2 42
#define CH3 43
#define CH4 44
#define CH5 45
#define CH6 46
#define CH7 47
#define CH8 48
#define CH9 49

#elif defined( SLAVE_5 )

#define CH0 50
#define CH1 51
#define CH2 52
#define CH3 53
#define CH4 54
#define CH5 55
#define CH6 56
#define CH7 57
#define CH8 58
#define CH9 58

#else

#define CH0 0
#define CH1 0
#define CH2 0
#define CH3 0
#define CH4 0
#define CH5 0
#define CH6 0
#define CH7 0
#define CH8 0
#define CH9 0

#endif

#endif

#endif  // CONFIG_H
