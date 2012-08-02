// File: PatternThread.h

#ifndef PATTERN_THREAD_H
#define PATTERN_THREAD_H

//******************
// INCLUDES
//******************

#include <QThread>

class QMutex;

//******************
// DEFINITIONS
//******************



//******************
// CLASS
//******************

class cPatternThread : public QThread
{
    Q_OBJECT
    
public:
    cPatternThread( QObject* pParent = 0 );
    ~cPatternThread();

    void Stop( void );

signals:
    void UpdatedPattern( quint8* newPattern );

protected:
    virtual void run( void );

    bool mStopFlag;
    QMutex* mpMutex;
};

#endif // PATTERN_THREAD_H
