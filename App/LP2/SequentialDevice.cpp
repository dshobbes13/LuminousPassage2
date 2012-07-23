/**
 * \file SequentialDevice.cpp
 *
 * Copyright (c) 2011, Topcon Medical Systems, Inc.
 * All rights reserved.
 *
 * Implement the iSequentialDevice abstract base class.
 */

//******************
// INCLUDES
//******************

#include "SequentialDevice.h"

#include <cassert>
#include <cerrno>
#include <cstring>
#include <iomanip>
#include <sstream>

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <QDebug>
#include <QSocketNotifier>


//******************
// DEFINITIONS
//******************

namespace
{
    // Use a buffer large enough to receive any full message
    const int BUFFER_SIZE = 1032;

    int ModeFlags( QIODevice::OpenMode mode )
    {
        int flags = 0;

        switch( mode )
        {
        case QIODevice::ReadOnly:  flags = O_RDONLY; break;
        case QIODevice::WriteOnly: flags = O_WRONLY; break;
        case QIODevice::ReadWrite: flags = O_RDWR;   break;
        default:                   flags = 0;        break;
        }

        return flags;
    }
}

//******************
// CLASS
//******************

iSequentialDevice::iSequentialDevice( const QString& path )
    : mFilePath( path )
    , mFileDescriptor( -1 )
    , mpNotifier( 0 )
{
}

// virtual //
iSequentialDevice::~iSequentialDevice( void )
{
    close();
}

//******************
// PUBLIC
//******************

// virtual //
qint64 iSequentialDevice::bytesAvailable( void ) const
{
    return mDataBuffer.length() + QIODevice::bytesAvailable();
}

// virtual //
qint64 iSequentialDevice::bytesToWrite( void ) const
{
    return 0;
}

// virtual //
void iSequentialDevice::close( void )
{
    if( mFileDescriptor >= 0 )
    {
        delete mpNotifier;
        mpNotifier = 0;

        ::close( mFileDescriptor );
        mFileDescriptor = -1;
    }
}

// virtual //
bool iSequentialDevice::isSequential( void ) const
{
    return true;
}

// virtual //
bool iSequentialDevice::open( OpenMode mode )
{
    close();

    mFileDescriptor = ::open( mFilePath.toAscii().data(), ModeFlags( mode ) | DeviceFlags() );
    if( mFileDescriptor < 0 )
    {
        qDebug() << "Could not open" << mFilePath << strerror( errno );
        setOpenMode( NotOpen );
        return false;
    }

    if( SetupDevice( mFileDescriptor ) != 0 )
    {
        ::close( mFileDescriptor );
        setOpenMode( NotOpen );
        return false;
    }

    setOpenMode( mode );

    if( mpNotifier )
    {
        disconnect( mpNotifier, SIGNAL( activated( int ) ), this, SLOT( FileActivated() ) );
        delete mpNotifier;
    }

    mpNotifier = new QSocketNotifier( mFileDescriptor, QSocketNotifier::Read );
    connect( mpNotifier, SIGNAL( activated( int ) ), this, SLOT( FileActivated() ) );

    return true;
}

//******************
// PROTECTED
//******************

// virtual //
qint64 iSequentialDevice::readData( char* data, qint64 maxSize )
{
    qint64 length = qMin( maxSize, static_cast<qint64>( mDataBuffer.length() ) );

    memcpy( data, mDataBuffer.data(), length );
    mDataBuffer.remove( 0, length );

    return length;
}

// virtual //
qint64 iSequentialDevice::writeData( const char* data, qint64 maxSize )
{
    qint64 amountWritten = -1;

    if( mFileDescriptor >= 0 )
    {
        amountWritten = ::write( mFileDescriptor, data, maxSize );

        std::stringstream sstr;
        for( int i = 0; i < amountWritten; i++ )
        {
            sstr << std::hex << std::setw( 3 ) << uint16_t( uint8_t( *( data + i ) ) );
            if( ( i + 1 ) % 16 == 0 )
            {
                sstr.str( "" );
            }
        }

    }

    return amountWritten;
}

// virtual //
uint32_t iSequentialDevice::DeviceFlags( void ) const
{
    return 0;
}

// virtual //
int32_t iSequentialDevice::SetupDevice( int )
{
    return 0;
}

//******************
// PRIVATE SLOTS
//******************

void iSequentialDevice::FileActivated( void )
{
    QByteArray incomingData;
    incomingData.resize( BUFFER_SIZE );

    ssize_t amountRead = ::read( mFileDescriptor, incomingData.data(), incomingData.size() );
    if( amountRead >= 0 )
    {
        std::stringstream sstr;
        for( int i = 0; i < amountRead; i++ )
        {
            sstr << std::hex << std::setw( 3 ) << uint16_t( uint8_t( *( incomingData.data() + i ) ) );
            if( ( i + 1 ) % 16 == 0 )
            {
                sstr.str( "" );
            }
        }

        mDataBuffer.append( incomingData.left( amountRead ) );

        if( bytesAvailable() > 0 )
        {
            emit readyRead();
        }
    }
}
