#include "fft.h"

short dataReal[64];
short dataImag[64];

void setup( void )
{
  fix_fft( dataReal, dataImag, 64 );
}

void loop( void )
{
}
