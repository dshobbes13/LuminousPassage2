// File: PatternThread.cpp

//******************
// INCLUDES
//******************

#include "PatternThread.h"

#include <QElapsedTimer>
#include <QMutexLocker>

#include "global.h"
#include "pattern.h"

//******************
// DEFINITIONS
//******************

namespace
{
}

//******************
// CLASS
//******************

cPatternThread::cPatternThread( QObject* pParent )
    : QThread( pParent )
    , mStopFlag( false )
{
    mpMutex = new QMutex();
}

cPatternThread::~cPatternThread()
{
}

void cPatternThread::Stop( void )
{
    QMutexLocker locker( mpMutex );
    mStopFlag = true;
}

void cPatternThread::run( void )
{
    QElapsedTimer timer;
    timer.start();
    qint64 time = timer.elapsed();
    quint8 newPattern[GLOBAL_NUM_CHANNELS];
    while(1)
    {
        if( ( timer.elapsed() - time ) >= 10 )
        {
            time += 10;
            PatternProcess();
            PatternData( newPattern );
            emit( UpdatedPattern( newPattern ) );
        }
        QMutexLocker locker( mpMutex );
        if( mStopFlag )
        {
            break;
        }
    }
}

