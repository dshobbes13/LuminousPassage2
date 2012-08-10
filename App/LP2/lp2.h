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
#include "pattern.h"

class QCheckBox;
class QComboBox;
class QDoubleSpinBox;
class QElapsedTimer;
class QLabel;
class QListWidget;
class QListWidgetItem;
class QProgressBar;
class QPushButton;
class QSlider;
class QSpinBox;

class cFft;
class cSerialDevice;
class cLights;
class cPatternThread;

#include "com.h"

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
    void HandleUpdatedPattern( quint8* newPattern );
    void UpdateAudioParameters( void );
    void UpdateBucketParameters( void );
    void HandleBucketSliders( void );
    void HandleEffectClicked( QListWidgetItem* pItem );
    void HandleManual( void );
    void HandlePulseSquare( void );
    void HandlePulseSine( void );
    void HandleDistanceSquare( void );

    void Save( void );
    void Load( void );

private:
    void SendCommand( eCommand, QByteArray data );
    QString GetEffectName( eEffect effect );
    void AddMessage( QString message );

    QComboBox* mpSerialPorts;
    QPushButton* mpOpenButton;
    QPushButton* mpCloseButton;
    QListWidget* mpMessages;
    QLabel* mpLabel;

    QStringList mFftLabels;
    cFft* mpFft;

    QCheckBox* mpSimulateBucketsCheck;
    QSlider* mpBucketValSliders[GLOBAL_NUM_BUCKETS];
    QSlider* mpBucketAvgSliders[GLOBAL_NUM_BUCKETS];

    QSpinBox* mpThreshold;
    QDoubleSpinBox* mpAveraging;
    QDoubleSpinBox* mpHysteresis;
    QSpinBox* mpSeconds;
    QCheckBox* mpTimeFlags[GLOBAL_NUM_BUCKETS];
    cFft* mpBuckets;
    QSpinBox* mpLo[GLOBAL_NUM_BUCKETS];
    QSpinBox* mpHi[GLOBAL_NUM_BUCKETS];

    cLights* mpLights;
    QListWidget* mpEffects;
    QSlider* mpManualValueSlider;
    QSpinBox* mpPulseSquareSourceSpin;
    QSpinBox* mpPulseSquareLengthSpin;
    QSpinBox* mpPulseSquareWidthSpin;
    QSpinBox* mpPulseSineSourceSpin;
    QSpinBox* mpPulseSineLengthSpin;
    QSpinBox* mpPulseSineWidthSpin;
    QSpinBox* mpDistanceSquareSourceSpin;
    QSpinBox* mpDistanceSquareStartSpin;
    QSpinBox* mpDistanceSquareStopSpin;
    QSpinBox* mpDistanceSquareAmpSpin;

    cPatternThread* mpPatternThread;
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
