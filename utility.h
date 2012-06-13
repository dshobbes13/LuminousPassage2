#ifndef UTILITY_H
#define UTILITY_H

// Clear bit macro for SFRs
#ifndef cbi
#define cbi( sfr, bit ) ( _SFR_BYTE( sfr ) &= ~_BV( bit ) )
#endif

// Set bit macro for SFRs
#ifndef sbi
#define sbi( sfr, bit ) ( _SFR_BYTE( sfr ) |= _BV( bit ) )
#endif

#endif
