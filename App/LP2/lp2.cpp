// File: LP2.cpp

//******************
// INCLUDES
//******************

#include "lp2.h"

#include <QCheckBox>
#include <QComboBox>
#include <QDir>
#include <QDoubleSpinBox>
#include <QElapsedTimer>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QProgressBar>
#include <QPushButton>
#include <QSpacerItem>
#include <QSpinBox>
#include <QTimer>
#include <QThread>
#include <QTreeWidget>

#include "SerialDevice.h"
#include "Fft.h"
#include "Lights.h"
#include "PatternThread.h"

#include "audio.h"


//******************
// DEFINITIONS
//******************

#define READ_TIMEOUT        15
#define READ_BUFFER_LENGTH  1024
#define MESSAGE_LENGTH_RX   64

#define MAX_MESSAGE_COUNT   50


//******************
// CLASS
//******************

cLP2::cLP2( QWidget* pParent )
    : QWidget( pParent )
    , mpSerial( NULL )
{

    mpSerialPorts = new QComboBox( this );
    QDir dir( "/dev" );
    QStringList modems = dir.entryList( QStringList() << "tty.usbmodem*", QDir::System );
    mpSerialPorts->addItems( modems );

    mpOpenButton = new QPushButton( "Open", this );
    connect( mpOpenButton, SIGNAL( clicked() ), this, SLOT( Open() ) );
    mpCloseButton = new QPushButton( "Close", this );
    connect( mpCloseButton, SIGNAL( clicked() ), this, SLOT( Close() ) );


    mpMessages = new QListWidget( this );
    mpMessages->setMinimumWidth( 250 );
    mpMessages->setMaximumHeight( 200 );

    mpLabel = new QLabel( "Avg Message Rate - " );

    qint32 maxFreq = 5120 / 2;
    qint32 freqWidth = maxFreq / GLOBAL_NUM_FREQ;
    qint32 freqStart = freqWidth / 2;
    for( qint32 i=0; i<GLOBAL_NUM_FREQ; i++ )
    {
        qint32 freq;
        if( i == 0 )
        {
            freq = freqStart / 2;
        }
        else
        {
            freq = freqStart + ( freqWidth * i ) - ( freqWidth / 2 );
        }
        if( freq < 1000 )
        {
            mFftLabels << QString::number( freq );
        }
        else
        {
            mFftLabels << QString( "%1k" ).arg( (qreal)freq / 1000, 2, 'f', 1 );
        }
    }
    mpFft = new cFft( GLOBAL_NUM_FREQ, 256, mFftLabels, this );
    mpFft->SetHysteresis( 0.1 );

    mpSimulateBucketsCheck = new QCheckBox( "Simulate Buckets", this );
    for( qint32 i=0; i<GLOBAL_NUM_BUCKETS; i++ )
    {
        mpBucketValSliders[i] = new QSlider( Qt::Horizontal, this );
        mpBucketValSliders[i]->setRange( 0, 512 );
        connect( mpBucketValSliders[i], SIGNAL( valueChanged( int ) ), this, SLOT( HandleBucketSliders() ) );
        mpBucketAvgSliders[i] = new QSlider( Qt::Horizontal, this );
        mpBucketAvgSliders[i]->setRange( 0, 512 );
        connect( mpBucketAvgSliders[i], SIGNAL( valueChanged( int ) ), this, SLOT( HandleBucketSliders() ) );
    }

    mpThreshold = new QSpinBox( this );
    mpThreshold->setRange( 0, 10 );
    mpThreshold->setSuffix( " thresh" );
    connect( mpThreshold, SIGNAL( valueChanged( int ) ), this, SLOT( UpdateThreshold() ) );
    mpThreshold->setValue( 8 );

    mpAveraging = new QDoubleSpinBox( this );
    mpAveraging->setMinimum( 0.0 );
    mpAveraging->setMaximum( 1.0 );
    mpAveraging->setDecimals( 2 );
    mpAveraging->setSingleStep( 0.05 );
    mpAveraging->setSuffix( " avg" );
    connect( mpAveraging, SIGNAL( valueChanged( double ) ), this, SLOT( UpdateAveraging( double ) ) );
    mpAveraging->setValue( 0.9 );

    mpHysteresis = new QDoubleSpinBox( this );
    mpHysteresis->setMinimum( 0.0 );
    mpHysteresis->setMaximum( 1.0 );
    mpHysteresis->setDecimals( 2 );
    mpHysteresis->setSingleStep( 0.01 );
    mpHysteresis->setValue( 0.25 );
    mpHysteresis->setSuffix( " hyst" );

    mpSeconds = new QSpinBox( this );
    mpSeconds->setRange( 0, 10 );
    mpSeconds->setValue( 0 );
    mpSeconds->setSuffix( " sec" );

    for( qint32 i=0; i<GLOBAL_NUM_BUCKETS; i++ )
    {
        mpTimeFlags[i] = new QCheckBox( this );
    }

    QStringList bucketLabels;
    bucketLabels << "1" << "2" << "3" << "4" << "5" << "6";
    mpBuckets = new cFft( GLOBAL_NUM_BUCKETS, 512, bucketLabels, this );
    mpBuckets->SetHysteresis( 0.25 );

    connect( mpHysteresis, SIGNAL( valueChanged( double ) ), this, SLOT( UpdateBucketParameters() ) );
    connect( mpSeconds, SIGNAL( valueChanged( int ) ), this, SLOT( UpdateBucketParameters() ) );
    connect( mpTimeFlags[0], SIGNAL( clicked() ), this, SLOT( UpdateBucketParameters() ) );
    connect( mpTimeFlags[1], SIGNAL( clicked() ), this, SLOT( UpdateBucketParameters() ) );
    connect( mpTimeFlags[2], SIGNAL( clicked() ), this, SLOT( UpdateBucketParameters() ) );
    connect( mpTimeFlags[3], SIGNAL( clicked() ), this, SLOT( UpdateBucketParameters() ) );
    connect( mpTimeFlags[4], SIGNAL( clicked() ), this, SLOT( UpdateBucketParameters() ) );
    connect( mpTimeFlags[5], SIGNAL( clicked() ), this, SLOT( UpdateBucketParameters() ) );

    for( qint32 i=0; i<GLOBAL_NUM_BUCKETS; i++ )
    {
        mpLo[i] = new QSpinBox( this );
        mpHi[i] = new QSpinBox( this );
        mpLo[i]->setRange( 0, GLOBAL_NUM_FREQ-1 );
        mpHi[i]->setRange( 0, GLOBAL_NUM_FREQ-1 );
        connect( mpLo[i], SIGNAL( valueChanged( int ) ), this, SLOT( UpdateBuckets() ) );
        connect( mpHi[i], SIGNAL( valueChanged( int ) ), this, SLOT( UpdateBuckets() ) );
    }
    mpLo[0]->setValue( 1 );
    mpHi[0]->setValue( 2 );
    mpLo[1]->setValue( 3 );
    mpHi[1]->setValue( 4 );
    mpLo[2]->setValue( 5 );
    mpHi[2]->setValue( 8 );
    mpLo[3]->setValue( 9 );
    mpHi[3]->setValue( 16 );
    mpLo[4]->setValue( 17 );
    mpHi[4]->setValue( 32 );
    mpLo[5]->setValue( 33 );
    mpHi[5]->setValue( 63 );

    mpLights = new cLights( GLOBAL_NUM_CHANNELS, this );

    mpEffects = new QListWidget( this );
    mpEffects->setSelectionMode( QAbstractItemView::MultiSelection );
    for( qint32 i=0; i<Effect_MAX; i++ )
    {
        QListWidgetItem* pItem = new QListWidgetItem( GetEffectName( (eEffect)i ) );
        pItem->setData( Qt::UserRole, i );
        mpEffects->addItem( pItem );
    }
    connect( mpEffects, SIGNAL( itemClicked( QListWidgetItem* ) ), this, SLOT( HandleEffectClicked( QListWidgetItem* ) ) );

    mpManualValueSlider = new QSlider( Qt::Horizontal, this );
    mpManualValueSlider->setRange( 0, 255 );
    connect( mpManualValueSlider, SIGNAL( valueChanged( int ) ), this, SLOT( HandleManual() ) );

    mpPulseSquareSourceSpin = new QSpinBox( this );
    mpPulseSquareSourceSpin->setRange( 0, GLOBAL_NUM_BUCKETS );
    mpPulseSquareSourceSpin->setSuffix( " source" );
    mpPulseSquareLengthSpin = new QSpinBox( this );
    mpPulseSquareLengthSpin->setRange( 0, 64 );
    mpPulseSquareLengthSpin->setSuffix( " length" );
    mpPulseSquareWidthSpin = new QSpinBox( this );
    mpPulseSquareWidthSpin->setRange( 0, 64 );
    mpPulseSquareWidthSpin->setSuffix( " width" );
    connect( mpPulseSquareSourceSpin, SIGNAL( valueChanged( int ) ), this, SLOT( HandlePulseSquare() ) );
    connect( mpPulseSquareLengthSpin, SIGNAL( valueChanged( int ) ), this, SLOT( HandlePulseSquare() ) );
    connect( mpPulseSquareWidthSpin, SIGNAL( valueChanged( int ) ), this, SLOT( HandlePulseSquare() ) );
    mpPulseSquareSourceSpin->setValue( 1 );
    mpPulseSquareLengthSpin->setValue( 40 );
    mpPulseSquareWidthSpin->setValue( 10 );

    mpPulseSineSourceSpin = new QSpinBox( this );
    mpPulseSineSourceSpin->setRange( 0, GLOBAL_NUM_BUCKETS );
    mpPulseSineSourceSpin->setSuffix( " source" );
    mpPulseSineLengthSpin = new QSpinBox( this );
    mpPulseSineLengthSpin->setRange( 0, 64 );
    mpPulseSineLengthSpin->setSuffix( " length" );
    mpPulseSineWidthSpin = new QSpinBox( this );
    mpPulseSineWidthSpin->setRange( 0, 64 );
    mpPulseSineWidthSpin->setSuffix( " width" );
    connect( mpPulseSineSourceSpin, SIGNAL( valueChanged( int ) ), this, SLOT( HandlePulseSine() ) );
    connect( mpPulseSineLengthSpin, SIGNAL( valueChanged( int ) ), this, SLOT( HandlePulseSine() ) );
    connect( mpPulseSineWidthSpin, SIGNAL( valueChanged( int ) ), this, SLOT( HandlePulseSine() ) );
    mpPulseSineSourceSpin->setValue( 1 );
    mpPulseSineLengthSpin->setValue( 40 );
    mpPulseSineWidthSpin->setValue( 10 );

    mpDistanceSquareSourceSpin = new QSpinBox( this );
    mpDistanceSquareSourceSpin->setRange( 0, GLOBAL_NUM_BUCKETS );
    mpDistanceSquareSourceSpin->setSuffix( " source" );
    mpDistanceSquareStartSpin = new QSpinBox( this );
    mpDistanceSquareStartSpin->setRange( 0, GLOBAL_NUM_CHANNELS );
    mpDistanceSquareStartSpin->setSuffix( " start" );
    mpDistanceSquareStopSpin = new QSpinBox( this );
    mpDistanceSquareStopSpin->setRange( 0, GLOBAL_NUM_CHANNELS );
    mpDistanceSquareStopSpin->setSuffix( " stop" );
    mpDistanceSquareAmpSpin = new QSpinBox( this );
    mpDistanceSquareAmpSpin->setRange( 0, 5 );
    mpDistanceSquareAmpSpin->setSuffix( " amp" );
    connect( mpDistanceSquareSourceSpin, SIGNAL( valueChanged( int ) ), this, SLOT( HandleDistanceSquare() ) );
    connect( mpDistanceSquareStartSpin, SIGNAL( valueChanged( int ) ), this, SLOT( HandleDistanceSquare() ) );
    connect( mpDistanceSquareStopSpin, SIGNAL( valueChanged( int ) ), this, SLOT( HandleDistanceSquare() ) );
    connect( mpDistanceSquareAmpSpin, SIGNAL( valueChanged( int ) ), this, SLOT( HandleDistanceSquare() ) );
    mpDistanceSquareSourceSpin->setValue( 2 );
    mpDistanceSquareStartSpin->setValue( 50 );
    mpDistanceSquareStopSpin->setValue( 59 );
    mpDistanceSquareAmpSpin->setValue( 1 );

    QVBoxLayout* pMainLayout = new QVBoxLayout( this );
    QGridLayout* pGridLayout;
    QHBoxLayout* pHBoxLayout;
    QVBoxLayout* pVBoxLayout;

    pGridLayout = new QGridLayout();
    pGridLayout->addWidget( mpSerialPorts, 0, 0 );
    pGridLayout->addWidget( mpOpenButton,  1, 0 );
    pGridLayout->addWidget( mpCloseButton, 2, 0 );
    pGridLayout->addWidget( mpLabel,       3, 0 );
    pGridLayout->addWidget( mpMessages,    4, 0 );
    pGridLayout->addWidget( mpFft,         0, 1, 5, 6 );
    pMainLayout->addLayout( pGridLayout );

    pGridLayout = new QGridLayout();
    pGridLayout->addWidget( mpSimulateBucketsCheck, 0, 0 );
    pGridLayout->addWidget( mpBucketValSliders[0],  0, 1 );
    pGridLayout->addWidget( mpBucketValSliders[1],  0, 2 );
    pGridLayout->addWidget( mpBucketValSliders[2],  0, 3 );
    pGridLayout->addWidget( mpBucketValSliders[3],  0, 4 );
    pGridLayout->addWidget( mpBucketValSliders[4],  0, 5 );
    pGridLayout->addWidget( mpBucketValSliders[5],  0, 6 );
    pGridLayout->addWidget( mpBucketAvgSliders[0],  1, 1 );
    pGridLayout->addWidget( mpBucketAvgSliders[1],  1, 2 );
    pGridLayout->addWidget( mpBucketAvgSliders[2],  1, 3 );
    pGridLayout->addWidget( mpBucketAvgSliders[3],  1, 4 );
    pGridLayout->addWidget( mpBucketAvgSliders[4],  1, 5 );
    pGridLayout->addWidget( mpBucketAvgSliders[5],  1, 6 );
    pGridLayout->addWidget( mpLo[0],       2, 1 );
    pGridLayout->addWidget( mpHi[0],       3, 1 );
    pGridLayout->addWidget( mpLo[1],       2, 2 );
    pGridLayout->addWidget( mpHi[1],       3, 2 );
    pGridLayout->addWidget( mpLo[2],       2, 3 );
    pGridLayout->addWidget( mpHi[2],       3, 3 );
    pGridLayout->addWidget( mpLo[3],       2, 4 );
    pGridLayout->addWidget( mpHi[3],       3, 4 );
    pGridLayout->addWidget( mpLo[4],       2, 5 );
    pGridLayout->addWidget( mpHi[4],       3, 5 );
    pGridLayout->addWidget( mpLo[5],       2, 6 );
    pGridLayout->addWidget( mpHi[5],       3, 6 );
    pGridLayout->addWidget( mpBuckets,     4, 1, 6, 6 );
    pGridLayout->addWidget( mpThreshold,   4, 0 );
    pGridLayout->addWidget( mpThreshold,   5, 0 );
    pGridLayout->addWidget( mpAveraging,   6, 0 );
    pGridLayout->addWidget( mpHysteresis,  7, 0 );
    pGridLayout->addWidget( mpSeconds,     8, 0 );
    pHBoxLayout = new QHBoxLayout();
    pHBoxLayout->addWidget( mpTimeFlags[0] );
    pHBoxLayout->addWidget( mpTimeFlags[1] );
    pHBoxLayout->addWidget( mpTimeFlags[2] );
    pHBoxLayout->addWidget( mpTimeFlags[3] );
    pHBoxLayout->addWidget( mpTimeFlags[4] );
    pHBoxLayout->addWidget( mpTimeFlags[5] );
    pGridLayout->addLayout( pHBoxLayout,   9, 0 );
    pMainLayout->addLayout( pGridLayout );

    pGridLayout = new QGridLayout();
    pGridLayout->addWidget( mpLights,  0, 0, 1, -1 );
    pGridLayout->addWidget( mpEffects, 1, 0, -1, 1 );
    QLabel* pLabelManual = new QLabel( "Manual:", this );
    pGridLayout->addWidget( pLabelManual, 1, 1 );
    pGridLayout->addWidget( mpManualValueSlider,  1, 2, 1, 4 );
    QLabel* pLabelPulseSquare = new QLabel( "Pulse Square:", this );
    pGridLayout->addWidget( pLabelPulseSquare, 2, 1 );
    pGridLayout->addWidget( mpPulseSquareSourceSpin, 2, 2 );
    pGridLayout->addWidget( mpPulseSquareLengthSpin, 2, 3 );
    pGridLayout->addWidget( mpPulseSquareWidthSpin, 2, 4 );
    QLabel* pLabelPulseSine = new QLabel( "Pulse Sine:", this );
    pGridLayout->addWidget( pLabelPulseSine, 3, 1 );
    pGridLayout->addWidget( mpPulseSineSourceSpin, 3, 2 );
    pGridLayout->addWidget( mpPulseSineLengthSpin, 3, 3 );
    pGridLayout->addWidget( mpPulseSineWidthSpin, 3, 4 );
    QLabel* pLabelDistanceSquare = new QLabel( "Distance Square:", this );
    pGridLayout->addWidget( pLabelDistanceSquare, 4, 1 );
    pGridLayout->addWidget( mpDistanceSquareSourceSpin, 4, 2 );
    pGridLayout->addWidget( mpDistanceSquareStartSpin, 4, 3 );
    pGridLayout->addWidget( mpDistanceSquareStopSpin, 4, 4 );
    pGridLayout->addWidget( mpDistanceSquareAmpSpin, 4, 5 );
    pGridLayout->setColumnStretch( 1, 1 );
    pGridLayout->setColumnStretch( 2, 1 );
    pGridLayout->setColumnStretch( 3, 1 );
    pGridLayout->setColumnStretch( 4, 1 );
    pMainLayout->addLayout( pGridLayout );

    QPushButton* pSaveButton = new QPushButton( "Save", this );
    QPushButton* pLoadButton = new QPushButton( "Load", this );
    pMainLayout->addWidget( pSaveButton );
    pMainLayout->addWidget( pLoadButton );
    connect( pSaveButton, SIGNAL( clicked() ), this, SLOT( Save() ) );
    connect( pLoadButton, SIGNAL( clicked() ), this, SLOT( Load() ) );

    mpPatternThread = new cPatternThread( this );
    connect( mpPatternThread, SIGNAL( UpdatedPattern( quint8* ) ), this, SLOT( HandleUpdatedPattern( quint8* ) ) );
    mpPatternThread->start();

    connect( this, SIGNAL( NewMessage( QByteArray ) ), this, SLOT( HandleNewMessage( QByteArray ) ) );

    QThread::currentThread()->setPriority( QThread::HighPriority );
}

cLP2::~cLP2()
{
    mpPatternThread->Stop();
    mpPatternThread->quit();
    mpPatternThread->wait();
}

void cLP2::Open( void )
{
    QString modem = mpSerialPorts->currentText();
    if( mpSerial )
    {
        delete mpSerial;
        mpSerial = NULL;
    }
    mpSerial = new cSerialDevice( "/dev/" + modem );
    connect( mpSerial, SIGNAL( readyRead() ), this, SLOT( Read() ) );
    bool opened = mpSerial->open( QIODevice::ReadWrite );
    AddMessage( opened ? "SERIAL PORT OPEN SUCCESS" : "SERIAL PORT OPEN FAILED" );
    if( opened )
    {
        mFirstRead = true;
    }
}

void cLP2::Close( void )
{
    if( mpSerial )
    {
        mpSerial->close();
        delete mpSerial;
        mpSerial = NULL;
    }
    AddMessage( "SERIAL PORT CLOSED" );
    mpFft->Reset();
    mpBuckets->Reset();
}

void cLP2::Read( void )
{
    if( mReadTimer.isValid() && ( mReadTimer.elapsed() > READ_TIMEOUT ) )
    {
        mDataRx.clear();
        mReadTimer.invalidate();
    }

    static char mReadBuffer[READ_BUFFER_LENGTH];
    qint64 bytesRead = mpSerial->read( mReadBuffer, READ_BUFFER_LENGTH );
    if( bytesRead >= 0 )
    {
        mDataRx.append( mReadBuffer, bytesRead );
        while( mDataRx.size() >= MESSAGE_LENGTH_RX )
        {
            QByteArray message = mDataRx.left( MESSAGE_LENGTH_RX );
            mDataRx.remove( 0, MESSAGE_LENGTH_RX );
            if( !mFirstRead )
            {
                emit( NewMessage( message ) );
            }
        }
    }

    if( mDataRx.size() > 0 )
    {
        mReadTimer.start();
    }

    mFirstRead = false;
}

void cLP2::HandleNewMessage( QByteArray message )
{
    QVector<quint32> vFreqs;
    quint8 newFreqs[GLOBAL_NUM_FREQ];
    for( qint32 i=0; i<GLOBAL_NUM_FREQ; i++ )
    {
        vFreqs << (quint8)message.at(i);
        newFreqs[i] = (quint8)message.at(i);
    }

    AudioUpdateFreq( newFreqs );
    quint8* newFreqAverages = AudioFreqAverages();
    quint16* newBuckets = AudioBuckets();
    quint16* newBucketAverages = AudioBucketAverages();

    QVector<quint32> vFreqAverages;
    QVector<quint32> vBuckets;
    QVector<quint32> vBucketAverages;
    for( qint32 i=0; i<GLOBAL_NUM_FREQ; i++ )
    {
        vFreqAverages.append( newFreqAverages[i] );
    }
    for( qint32 i=0; i<GLOBAL_NUM_BUCKETS; i++ )
    {
        vBuckets.append( newBuckets[i] );
        vBucketAverages.append( newBucketAverages[i] );
    }

    mpFft->UpdateData( vFreqs, vFreqAverages );
    PatternUpdateFreq( newFreqs );

    if( !mpSimulateBucketsCheck->isChecked() )
    {
        mpBuckets->UpdateData( vBuckets, vBucketAverages );
        PatternUpdateBuckets( newBuckets, newBucketAverages );
    }

    mCountMessages++;
    if( mCountMessages >= 10 )
    {
        qreal seconds = (qreal)mNewMessageTimer.elapsed() / 1000;
        qreal rate = 10 / seconds;
        mNewMessageTimer.restart();
        mpLabel->setText( QString( "Avg Message Rate - %2").arg( rate, 0, 'f', 2 ) );
        mCountMessages = 0;
    }
}

void cLP2::HandleBucketSliders( void )
{
    if( mpSimulateBucketsCheck->isChecked() )
    {
        quint16 newBuckets[GLOBAL_NUM_BUCKETS];
        quint16 newBucketAverages[GLOBAL_NUM_BUCKETS];

        QVector<quint32> vBuckets;
        QVector<quint32> vBucketAverages;
        for( qint32 i=0; i<GLOBAL_NUM_BUCKETS; i++ )
        {
            newBuckets[i] = (quint16)mpBucketValSliders[i]->value();
            newBucketAverages[i] = (quint16)mpBucketAvgSliders[i]->value();
            vBuckets.append( mpBucketValSliders[i]->value() );
            vBucketAverages.append( mpBucketAvgSliders[i]->value() );
        }

        mpBuckets->UpdateData( vBuckets, vBucketAverages );
        PatternUpdateBuckets( newBuckets, newBucketAverages );
    }
}

void cLP2::HandleUpdatedPattern( quint8* newPattern )
{
    QVector<quint8> vData;
    for( qint32 i=0; i<GLOBAL_NUM_CHANNELS; i++ )
    {
        vData.append( newPattern[i] );
    }
    mpLights->UpdateData( vData );
}

void cLP2::UpdateThreshold( void )
{
    quint8 threshold = (quint8)( mpThreshold->value() );
    AudioUpdateThreshold( threshold );
}

void cLP2::UpdateAveraging( double averaging )
{
    AudioUpdateAveraging( (float)averaging );
}

void cLP2::UpdateBucketParameters( void )
{
    qreal hysteresis = mpHysteresis->value();
    quint8 seconds = mpSeconds->value();
    quint8 flags;
    flags |= ( mpTimeFlags[0]->isChecked() ? 0x01 : 0x00 );
    flags |= ( mpTimeFlags[1]->isChecked() ? 0x02 : 0x00 );
    flags |= ( mpTimeFlags[2]->isChecked() ? 0x04 : 0x00 );
    flags |= ( mpTimeFlags[3]->isChecked() ? 0x08 : 0x00 );
    flags |= ( mpTimeFlags[4]->isChecked() ? 0x10 : 0x00 );
    flags |= ( mpTimeFlags[5]->isChecked() ? 0x20 : 0x00 );
    mpBuckets->SetHysteresis( hysteresis );

    PatternSetBucketParameters( (float)hysteresis, flags, seconds );

    quint8 hysteresisFixed = 255 * hysteresis;
    QByteArray data( MESSAGE_LENGTH-1, 0 );

    data.insert( 0, hysteresisFixed );
    data.insert( 1, flags );
    data.insert( 2, seconds );

    SendCommand( Command_BUCKETS, data );
}

void cLP2::UpdateBuckets( void )
{
    quint8 mLo[GLOBAL_NUM_BUCKETS];
    quint8 mHi[GLOBAL_NUM_BUCKETS];
    for( qint32 i=0; i<GLOBAL_NUM_BUCKETS; i++ )
    {
        mpLo[i]->setSuffix( QString( " - " + mFftLabels.at( mpLo[i]->value() ) ) );
        mpHi[i]->setSuffix( QString( " - " + mFftLabels.at( mpHi[i]->value() ) ) );
        mLo[i] = (quint8)( mpLo[i]->value() );
        mHi[i] = (quint8)( mpHi[i]->value() );
    }
    AudioUpdateBuckets( mLo, mHi );
}

void cLP2::HandleEffectClicked( QListWidgetItem* pItem )
{
    QByteArray data( MESSAGE_LENGTH-1, 0 );

    eEffect effect = (eEffect)pItem->data( Qt::UserRole ).toUInt();
    data.insert( 0, (char)effect );
    if( pItem->isSelected() )
    {
        data.insert( 1, (char)0x01 );
        PatternSetEffect( effect, true );
    }
    else
    {
        data.insert( 1, (char)0x00 );
        PatternSetEffect( effect, false );
    }

    SendCommand( Command_EFFECT, data );
}

void cLP2::HandleManual( void )
{
    QByteArray data( MESSAGE_LENGTH-1, 0 );

    char value = mpManualValueSlider->value();
    data.insert( 0, value );
    PatternSetManual( (quint8)value );

    SendCommand( Command_MANUAL, data );
}

void cLP2::HandlePulseSquare( void )
{
    QByteArray data( MESSAGE_LENGTH-1, 0 );

    char source = mpPulseSquareSourceSpin->value();
    char length = mpPulseSquareLengthSpin->value();
    char width = mpPulseSquareWidthSpin->value();
    data.insert( 0, source );
    data.insert( 1, length );
    data.insert( 2, width );
    PatternSetPulseSquare( (quint8)source, (quint8)length, (quint8)width );

    SendCommand( Command_PULSE_SQUARE, data );
}

void cLP2::HandlePulseSine( void )
{
    QByteArray data( MESSAGE_LENGTH-1, 0 );

    char source = mpPulseSineSourceSpin->value();
    char length = mpPulseSineLengthSpin->value();
    char width = mpPulseSineWidthSpin->value();
    data.insert( 0, source );
    data.insert( 1, length );
    data.insert( 2, width );
    PatternSetPulseSine( (quint8)source, (quint8)length, (quint8)width );

    SendCommand( Command_PULSE_SINE, data );
}

void cLP2::HandleDistanceSquare( void )
{
    QByteArray data( MESSAGE_LENGTH-1, 0 );

    char source = mpDistanceSquareSourceSpin->value();
    char start = mpDistanceSquareStartSpin->value();
    char stop = mpDistanceSquareStopSpin->value();
    char amp = mpDistanceSquareAmpSpin->value();
    data.insert( 0, source );
    data.insert( 1, start );
    data.insert( 2, stop );
    data.insert( 3, amp );
    PatternSetDistanceSquare( (quint8)source, (quint8)start, (quint8)stop, (quint8)amp );

    SendCommand( Command_DISTANCE_SQUARE, data );
}

void cLP2::Save( void )
{
    QByteArray data( MESSAGE_LENGTH-1, 0 );
    SendCommand( Command_SAVE, data );
}

void cLP2::Load( void )
{
    QByteArray data( MESSAGE_LENGTH-1, 0 );
    SendCommand( Command_LOAD, data );
}

void cLP2::SendCommand( eCommand command, QByteArray data )
{
    char message[8] = {0};
    message[0] = (char)command;
    for( qint32 i=1; i<8; i++ )
    {
        message[i] = data.at(i-1);
    }

    if( mpSerial )
    {
        mpSerial->write( message, 8 );
    }
}

QString cLP2::GetEffectName( eEffect effect )
{
    QString name;
    switch( effect )
    {
    case Effect_NULL:            name = "NULL";             break;
    case Effect_MANUAL:          name = "MANUAL";           break;
    case Effect_CYCLE:           name = "CYCLE";            break;
    case Effect_BREATH:          name = "BREATH";           break;
    case Effect_FREQ:            name = "FREQ";             break;
    case Effect_BUCKETS:         name = "BUCKETS";          break;
    case Effect_PULSE_SQUARE:    name = "PULSE_SQUARE";     break;
    case Effect_PULSE_SINE:      name = "PULSE_SINE";       break;
    case Effect_DISTANCE_SQUARE: name = "DISTANCE_SQUARE";  break;
    default:                                                break;
    }
    return name;
}

void cLP2::AddMessage( QString message )
{
    mpMessages->addItem( message );
    if( mpMessages->count() >= MAX_MESSAGE_COUNT )
    {
        delete mpMessages->takeItem( 0 );
    }
    mpMessages->scrollToBottom();
}

