// File: LP2.h

#ifndef LP2_H
#define LP2_H

//******************
// INCLUDES
//******************

#include <QWidget>

#include <QByteArray>
#include <QElapsedTimer>
#include <QString>
#include <QStringList>

#include "global.h"

class QComboBox;
class QDoubleSpinBox;
class QElapsedTimer;
class QLabel;
class QListWidget;
class QProgressBar;
class QPushButton;
class QSpinBox;

class cFft;
class cSerialDevice;
class cLights;


//******************
// DEFINITIONS
//******************


//******************
// CLASS
//******************

class cLP2 : public QWidget
{
    Q_OBJECT

public:
    cLP2( QWidget* pParent = 0 );
    ~cLP2();

signals:
    void NewMessage( QByteArray message );

private slots:
    void Open( void );
    void Close( void );
    void Read( void );
    void HandleNewMessage( QByteArray message );
    void HandleTimeout( void );
    void UpdateThreshold( void );
    void UpdateBuckets( void );
    void UpdateAveraging( double averaging );
    void HandleEffect( void );

private:
    void AddMessage( QString message );

    QComboBox* mpSerialPorts;
    QPushButton* mpOpenButton;
    QPushButton* mpCloseButton;

    QStringList mFftLabels;
    cFft* mpFft;
    cFft* mpBuckets;

    QComboBox* mpEffects;
    QPushButton* mpToggleEffect;
    cLights* mpLights;

    QSpinBox* mpLo[GLOBAL_NUM_BUCKETS];
    QSpinBox* mpHi[GLOBAL_NUM_BUCKETS];
    QSpinBox* mpThreshold;
    QDoubleSpinBox* mpAveraging;

    QListWidget* mpMessages;
    QLabel* mpLabel;

    cSerialDevice* mpSerial;

    QByteArray mDataRx;
    QElapsedTimer mReadTimer;
    bool mFirstRead;

    qint32 mCountMessages;
    QElapsedTimer mNewMessageTimer;

    quint32 mBytes;
    quint32 mUpdateNumber;

};

#endif // LP2_H
