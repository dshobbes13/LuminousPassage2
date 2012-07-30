// File: SerialDevice.cpp

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
    return ( O_NOCTTY | O_NDELAY );
}

#include <iostream>
// virtual //
int32_t cSerialDevice::SetupDevice( int fd )
{
    struct termios options;

    tcgetattr( fd, &options );

    options.c_iflag &= ~INPCK;      //INPCK - Enable parity check
    options.c_iflag &= ~IGNPAR;     //IGNPAR - Ignore parity errors
    options.c_iflag &= ~PARMRK;     //PARMRK - Mark parity errors
    options.c_iflag &= ~ISTRIP;     //ISTRIP - Strip parity bits
    options.c_iflag &= ~IXON;       //IXON - Enable software flow control (outgoing)
    options.c_iflag &= ~IXOFF;      //IXOFF - Enable software flow control (incoming)
    options.c_iflag &= ~IXANY;      //IXANY - Allow any character to start flow again
    options.c_iflag &= ~IGNBRK;     //IGNBRK - Ignore break condition
    options.c_iflag &= ~BRKINT;     //BRKINT - Send a SIGINT when a break condition is detected
    options.c_iflag &= ~INLCR;      //INLCR - Map NL to CR
    options.c_iflag &= ~IGNCR;      //IGNCR - Ignore CR
    options.c_iflag &= ~ICRNL;      //ICRNL - Map CR to NL
    //options.c_iflag &= ~IUCLC;      //IUCLC - Map uppercase to lowercase
    options.c_iflag &= ~IMAXBEL;    //IMAXBEL - Echo BEL on input line too long

    options.c_oflag &= ~OPOST;  //OPOST - Postprocess output (not set = raw output)
    //options.c_oflag &= ~OLCUC;   //OLCUC - Map lowercase to uppercase
    options.c_oflag &= ~ONLCR;   //ONLCR - Map NL to CR-NL
    options.c_oflag &= ~OCRNL;   //OCRNL - Map CR to NL
    //options.c_oflag &= ~NOCR;    //NOCR - No CR output at column 0
    options.c_oflag &= ~ONLRET;  //ONLRET - NL performs CR function
    options.c_oflag &= ~OFILL;   //OFILL - Use fill characters for delay
    options.c_oflag &= ~OFDEL;   //OFDEL - Fill character is DEL
    options.c_oflag &= ~NLDLY;   //NLDLY - Mask for delay time needed between lines
    options.c_oflag &= ~NL0;     //NL0 - No delay for NLs
    options.c_oflag &= ~NL1;     //NL1 - Delay further output after newline for 100 milliseconds
    options.c_oflag &= ~CRDLY;   //CRDLY - Mask for delay time needed to return carriage to left column
    options.c_oflag &= ~CR0;     //CR0 - No delay for CRs
    options.c_oflag &= ~CR1;     //CR1 - Delay after CRs depending on current column position
    options.c_oflag &= ~CR2;     //CR2 - Delay 100 milliseconds after sending CRs
    options.c_oflag &= ~CR3;     //CR3 - Delay 150 milliseconds after sending CRs
    options.c_oflag &= ~TABDLY;  //TABDLY - Mask for delay time needed after TABs
    options.c_oflag &= ~TAB0;    //TAB0 - No delay for TABs
    options.c_oflag &= ~TAB1;    //TAB1 - Delay after TABs depending on current column position
    options.c_oflag &= ~TAB2;    //TAB2 - Delay 100 milliseconds after sending TABs
    options.c_oflag &= ~TAB3;    //TAB3 - Expand TAB characters to spaces
    options.c_oflag &= ~BSDLY;   //BSDLY - Mask for delay time needed after BSs
    options.c_oflag &= ~BS0;     //BS0 - No delay for BSs
    options.c_oflag &= ~BS1;     //BS1 - Delay 50 milliseconds after sending BSs
    options.c_oflag &= ~VTDLY;   //VTDLY - Mask for delay time needed after VTs
    options.c_oflag &= ~VT0;     //VT0 - No delay for VTs
    options.c_oflag &= ~VT1;     //VT1 - Delay 2 seconds after sending VTs
    options.c_oflag &= ~FFDLY;   //FFDLY - Mask for delay time needed after FFs
    options.c_oflag &= ~FF0;     //FF0 - No delay for FFs
    options.c_oflag &= ~FF1;     //FF1 - Delay 2 seconds after sending FFs

    options.c_lflag &= ~ISIG;       //ISIG - Enable SIGINTR, SIGSUSP, SIGDSUSP, and SIGQUIT signals
    options.c_lflag &= ~ICANON;     //ICANON - Enable canonical input (else raw)
    //options.c_lflag &= ~XCASE;      //XCASE - Map uppercase \lowercase (obsolete)
    options.c_lflag &= ~ECHO;       //ECHO - Enable echoing of input characters
    options.c_lflag &= ~ECHOE;      //ECHOE - Echo erase character as BS-SP-BS
    options.c_lflag &= ~ECHOK;      //ECHOK - Echo NL after kill character
    options.c_lflag &= ~ECHONL;     //ECHONL - Echo NL
    options.c_lflag |=  NOFLSH;     //NOFLSH - Disable flushing of input buffers after interrupt or quit characters
    options.c_lflag &= ~IEXTEN;     //IEXTEN - Enable extended functions
    options.c_lflag &= ~ECHOCTL;    //ECHOCTL - Echo control characters as ^char and delete as ~?
    options.c_lflag &= ~ECHOPRT;    //ECHOPRT - Echo erased character as character erased
    options.c_lflag &= ~ECHOKE;     //ECHOKE - BS-SP-BS entire line on line kill
    //options.c_lflag &= ~FLUSHO;     //FLUSHO - Output being flushed
    options.c_lflag &= ~PENDIN;     //PENDIN - Retype pending input at next read or input char
    options.c_lflag &= ~TOSTOP;     //TOSTOP - Send SIGTTOU for background output

    options.c_cflag &= ~EXTA;           //EXTA - External rate clock
    options.c_cflag &= ~EXTB;           //EXTB - External rate clock
    options.c_cflag &= ~CSIZE;          //CSIZE - Bit mask for data bits
    //options.c_cflag &= ~CS5;            //CS5 - 5 data bits
    //options.c_cflag &= ~CS6;            //CS6 - 6 data bits
    //options.c_cflag &= ~CS7;            //CS7 - 7 data bits
    options.c_cflag |=  CS8;            //CS8 - 8 data bits
    options.c_cflag &= ~CSTOPB;         //CSTOPB - 2 stop bits (1 otherwise)
    options.c_cflag |=  CREAD;          //CREAD - Enable receiver
    options.c_cflag &= ~PARENB;         //PARENB - Enable parity bit
    options.c_cflag &= ~PARODD;         //PARODD - Use odd parity instead of even
    options.c_cflag &= ~HUPCL;          //HUPCL - Hangup (drop DTR) on last close
    options.c_cflag |=  CLOCAL;         //CLOCAL - Local line - do not change "owner" of port
    //options.c_cflag &= ~LOBLK;          //LOBLK - Block job control output
    //options.c_cflag &= ~CNEW_RTSCTS;    //CNEW_RTSCTS
    options.c_cflag &= ~CRTSCTS;        //CRTSCTS - Enable hardware flow control (not supported on all platforms)

    cfsetospeed( &options, B115200 );
    cfsetispeed( &options, B115200 );

    tcsetattr( fd, TCSANOW, &options );

    return 0;
}
