// File: SequentialDevice.cpp

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
    const int BUFFER_SIZE = 4096;

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

    fcntl( mFileDescriptor, F_SETFL, 0 );

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
        mDataBuffer.append( incomingData.left( amountRead ) );

        if( bytesAvailable() > 0 )
        {
            emit readyRead();
        }
    }
}
