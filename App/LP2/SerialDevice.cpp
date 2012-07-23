/**
 * \file SerialDevice.cpp
 *
 * Copyright (c) 2011, Topcon Medical Systems, Inc.
 * All rights reserved.
 *
 * Implement the cSerialDevice class.
 */

//******************
// INCLUDES
//******************

#include <fcntl.h>
#include <termios.h>
#include <unistd.h>

#include "SerialDevice.h"

//******************
// CLASS
//******************

cSerialDevice::cSerialDevice( const QString& path )
    : iSequentialDevice( path )
{
}

// virtual //
cSerialDevice::~cSerialDevice( void )
{
}

//******************
// PROTECTED
//******************

// virtual //
uint32_t cSerialDevice::DeviceFlags( void ) const
{
    return O_NOCTTY;
}

#include <iostream>
// virtual //
int32_t cSerialDevice::SetupDevice( int fd )
{
    struct termios options;

    tcgetattr( fd, &options );

    options.c_iflag &= ~( IGNBRK | BRKINT | PARMRK | ISTRIP |
                          INLCR | IGNCR | ICRNL | IXON );
    options.c_oflag &= ~OPOST;
    options.c_lflag &= ~( ECHO | ECHONL | ICANON | ISIG | IEXTEN );
    options.c_cflag &= ~( CSIZE | PARENB );
    options.c_cflag |= CS8;

    tcsetattr( fd, TCSANOW, &options );

    cfsetospeed( &options, B115200 );

    return 0;
}
