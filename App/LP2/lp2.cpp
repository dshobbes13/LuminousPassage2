// File: LP2.cpp

//******************
// INCLUDES
//******************

#include "lp2.h"

#include <QComboBox>
#include <QDir>
#include <QDoubleSpinBox>
#include <QElapsedTimer>
#include <QGridLayout>
#include <QLabel>
#include <QListWidget>
#include <QProgressBar>
#include <QPushButton>
#include <QSpinBox>
#include <QTimer>

#include "SerialDevice.h"
#include "Fft.h"
#include "Lights.h"

#include "audio.h"
#include "pattern.h"


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
{

    mpSerialPorts = new QComboBox( this );
    QDir dir( "/dev" );
    QStringList modems = dir.entryList( QStringList() << "tty.usbmodem*", QDir::System );
    mpSerialPorts->addItems( modems );

    mpOpenButton = new QPushButton( "Open", this );
    mpCloseButton = new QPushButton( "Close", this );

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

    QStringList bucketLabels;
    bucketLabels << "1" << "2" << "3" << "4" << "5" << "6";
    mpBuckets = new cFft( GLOBAL_NUM_BUCKETS, 512, bucketLabels, this );

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

    mpThreshold = new QSpinBox( this );
    mpThreshold->setRange( 0, 10 );
    connect( mpThreshold, SIGNAL( valueChanged( int ) ), this, SLOT( UpdateThreshold() ) );
    mpThreshold->setValue( 8 );

    mpAveraging = new QDoubleSpinBox( this );
    mpAveraging->setMinimum( 0.0 );
    mpAveraging->setMaximum( 1.0 );
    mpAveraging->setDecimals( 2 );
    mpAveraging->setSingleStep( 0.05 );
    connect( mpAveraging, SIGNAL( valueChanged( double ) ), this, SLOT( UpdateAveraging( double ) ) );
    mpAveraging->setValue( 0.9 );

    mpMessages = new QListWidget( this );
    mpMessages->setMinimumWidth( 250 );
    mpMessages->setMaximumHeight( 200 );

    mpLabel = new QLabel( "Avg Message Rate - " );

    mpEffects = new QComboBox( this );
    mpEffects->addItem( "MANUAL",     Effect_MANUAL );
    mpEffects->addItem( "CYCLE",      Effect_CYCLE );
    mpEffects->addItem( "BREATH",     Effect_BREATH );
    mpEffects->addItem( "FREQ",       Effect_FREQ );
    mpEffects->addItem( "BUCKETS",    Effect_BUCKETS );
    mpEffects->addItem( "BASS_PULSE", Effect_BASS_PULSE );
    mpToggleEffect = new QPushButton( "Toggle Effect" );
    mpLights = new cLights( GLOBAL_NUM_CHANNELS, this );
    connect( mpToggleEffect, SIGNAL( clicked() ), this, SLOT( HandleEffect() ) );

    QGridLayout* pGridLayout = new QGridLayout( this );
    pGridLayout->addWidget( mpSerialPorts, 0, 0 );
    pGridLayout->addWidget( mpOpenButton,  1, 0 );
    pGridLayout->addWidget( mpCloseButton, 2, 0 );
    pGridLayout->addWidget( mpLabel,       3, 0 );
    pGridLayout->addWidget( mpFft,         0, 1, 4, 6 );
    pGridLayout->addWidget( mpMessages,    4, 0 );
    pGridLayout->addWidget( mpBuckets,     4, 1, 1, 6 );
    pGridLayout->addWidget( mpLo[0],       5, 1 );
    pGridLayout->addWidget( mpHi[0],       6, 1 );
    pGridLayout->addWidget( mpLo[1],       5, 2 );
    pGridLayout->addWidget( mpHi[1],       6, 2 );
    pGridLayout->addWidget( mpLo[2],       5, 3 );
    pGridLayout->addWidget( mpHi[2],       6, 3 );
    pGridLayout->addWidget( mpLo[3],       5, 4 );
    pGridLayout->addWidget( mpHi[3],       6, 4 );
    pGridLayout->addWidget( mpLo[4],       5, 5 );
    pGridLayout->addWidget( mpHi[4],       6, 5 );
    pGridLayout->addWidget( mpLo[5],       5, 6 );
    pGridLayout->addWidget( mpHi[5],       6, 6 );
    pGridLayout->addWidget( mpThreshold,   5, 0 );
    pGridLayout->addWidget( mpAveraging,   6, 0 );
    pGridLayout->addWidget( mpEffects,     7, 0 );
    pGridLayout->addWidget( mpToggleEffect, 7, 1 );
    pGridLayout->addWidget( mpLights,      8, 0, 1, 7 );

    mpSerial = NULL;

    QTimer* pTimer = new QTimer( this );
    connect( pTimer, SIGNAL( timeout() ), this, SLOT( HandleTimeout() ) );
    pTimer->start( 10 );

    connect( mpOpenButton, SIGNAL( clicked() ), this, SLOT( Open() ) );
    connect( mpCloseButton, SIGNAL( clicked() ), this, SLOT( Close() ) );
    connect( this, SIGNAL( NewMessage( QByteArray ) ), this, SLOT( HandleNewMessage( QByteArray ) ) );
}

cLP2::~cLP2()
{
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
    mpBuckets->UpdateData( vBuckets, vBucketAverages );

    PatternUpdateFreq( newFreqs );
    PatternUpdateBuckets( newBuckets, newBucketAverages );

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

void cLP2::HandleTimeout( void )
{
    PatternProcess();
    quint8* newData = PatternData();
    QVector<quint8> vData;
    for( qint32 i=0; i<GLOBAL_NUM_CHANNELS; i++ )
    {
        vData.append( newData[i] );
    }
    mpLights->UpdateData( vData );
}

void cLP2::UpdateThreshold( void )
{
    quint8 threshold = (quint8)( mpThreshold->value() );
    AudioUpdateThreshold( threshold );
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

void cLP2::UpdateAveraging( double averaging )
{
    AudioUpdateAveraging( (float)averaging );
}

void cLP2::HandleEffect( void )
{
    eEffect effect = (eEffect)mpEffects->itemData( mpEffects->currentIndex() ).toInt();
    char command = 0;
    switch( effect )
    {
    case Effect_MANUAL:     command = '1';  break;
    case Effect_CYCLE:      command = '2';  break;
    case Effect_BREATH:     command = '3';  break;
    case Effect_FREQ:       command = '4';  break;
    case Effect_BUCKETS:    command = '5';  break;
    case Effect_BASS_PULSE: command = '6';  break;
    default:                                break;
    }

    const char commandString[1] = { command };
    if( PatternGetEffect( effect ) )
    {
        PatternSetEffect( effect, false );
    }
    else
    {
        PatternSetEffect( effect, true );
    }
    if( mpSerial )
    {
        mpSerial->write( commandString, 1 );
    }
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

