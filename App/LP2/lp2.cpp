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
#include <QLabel>
#include <QListWidget>
#include <QProgressBar>
#include <QPushButton>
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

    mpSimulateBucketsCheck = new QCheckBox( "Simulate Buckets", this );
    for( qint32 i=0; i<GLOBAL_NUM_BUCKETS; i++ )
    {
        mpBucketSliders[i] = new QSlider( Qt::Horizontal, this );
        mpBucketSliders[i]->setRange( 0, 512 );
        connect( mpBucketSliders[i], SIGNAL( valueChanged( int ) ), this, SLOT( HandleBucketSliders() ) );
    }

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

    mpEffects = new QListWidget( this );
    mpEffects->setSelectionMode( QAbstractItemView::MultiSelection );
    for( qint32 i=1; i<Effect_MAX; i++ )
    {
        QListWidgetItem* pItem = new QListWidgetItem( GetEffectName( (eEffect)i ) );
        pItem->setData( Qt::UserRole, i );
        mpEffects->addItem( pItem );
    }
    mpLights = new cLights( GLOBAL_NUM_CHANNELS, this );
    connect( mpEffects, SIGNAL( itemClicked( QListWidgetItem* ) ), this, SLOT( HandleEffectClicked( QListWidgetItem* ) ) );

    QGridLayout* pGridLayout = new QGridLayout( this );
    pGridLayout->addWidget( mpSerialPorts, 0, 0 );
    pGridLayout->addWidget( mpOpenButton,  1, 0 );
    pGridLayout->addWidget( mpCloseButton, 2, 0 );
    pGridLayout->addWidget( mpLabel,       3, 0 );
    pGridLayout->addWidget( mpMessages,    4, 0 );
    pGridLayout->addWidget( mpFft,         0, 1, 5, 6 );
    pGridLayout->addWidget( mpSimulateBucketsCheck, 5, 0 );
    pGridLayout->addWidget( mpBucketSliders[0],     5, 1 );
    pGridLayout->addWidget( mpBucketSliders[1],     5, 2 );
    pGridLayout->addWidget( mpBucketSliders[2],     5, 3 );
    pGridLayout->addWidget( mpBucketSliders[3],     5, 4 );
    pGridLayout->addWidget( mpBucketSliders[4],     5, 5 );
    pGridLayout->addWidget( mpBucketSliders[5],     5, 6 );
    pGridLayout->addWidget( mpEffects,     6, 0 );
    pGridLayout->addWidget( mpBuckets,     6, 1, 1, 6 );
    pGridLayout->addWidget( mpLo[0],       7, 1 );
    pGridLayout->addWidget( mpHi[0],       8, 1 );
    pGridLayout->addWidget( mpLo[1],       7, 2 );
    pGridLayout->addWidget( mpHi[1],       8, 2 );
    pGridLayout->addWidget( mpLo[2],       7, 3 );
    pGridLayout->addWidget( mpHi[2],       8, 3 );
    pGridLayout->addWidget( mpLo[3],       7, 4 );
    pGridLayout->addWidget( mpHi[3],       8, 4 );
    pGridLayout->addWidget( mpLo[4],       7, 5 );
    pGridLayout->addWidget( mpHi[4],       8, 5 );
    pGridLayout->addWidget( mpLo[5],       7, 6 );
    pGridLayout->addWidget( mpHi[5],       8, 6 );
    pGridLayout->addWidget( mpThreshold,   7, 0 );
    pGridLayout->addWidget( mpAveraging,   8, 0 );
    pGridLayout->addWidget( mpLights,      9, 0, 1, 7 );

    mpSerial = NULL;

    /*
    QTimer* pTimer = new QTimer( this );
    connect( pTimer, SIGNAL( timeout() ), this, SLOT( HandleTimeout() ) );
    pTimer->start( 10 );
    */
    mpPatternThread = new cPatternThread( this );
    connect( mpPatternThread, SIGNAL( UpdatedPattern( quint8* ) ), this, SLOT( HandleUpdatedPattern( quint8* ) ) );
    mpPatternThread->start();

    connect( mpOpenButton, SIGNAL( clicked() ), this, SLOT( Open() ) );
    connect( mpCloseButton, SIGNAL( clicked() ), this, SLOT( Close() ) );
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
            newBuckets[i] = (quint16)mpBucketSliders[i]->value();
            newBucketAverages[i] = 64;
            vBuckets.append( mpBucketSliders[i]->value() );
            vBucketAverages.append( 64 );
        }

        mpBuckets->UpdateData( vBuckets, vBucketAverages );
        PatternUpdateBuckets( newBuckets, newBucketAverages );
    }
}

void cLP2::HandleTimeout( void )
{
    /*
    PatternProcess();
    quint8 newData[GLOBAL_NUM_CHANNELS];
    PatternData( newData );
    QVector<quint8> vData;
    for( qint32 i=0; i<GLOBAL_NUM_CHANNELS; i++ )
    {
        vData.append( newData[i] );
    }
    mpLights->UpdateData( vData );
    */
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

void cLP2::HandleEffectClicked( QListWidgetItem* pItem )
{
    eEffect effect = (eEffect)pItem->data( Qt::UserRole ).toUInt();
    char message[8] = {0};
    message[0] = 0x01;
    message[1] = (quint8)effect;
    if( pItem->isSelected() )
    {
        message[2] = 0x01;
        PatternSetEffect( effect, true );
    }
    else
    {
        message[2] = 0x00;
        PatternSetEffect( effect, false );
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
    case Effect_NULL:       name = "NULL";          break;
    case Effect_MANUAL:     name = "MANUAL";        break;
    case Effect_CYCLE:      name = "CYCLE";         break;
    case Effect_BREATH:     name = "BREATH";        break;
    case Effect_FREQ:       name = "FREQ";          break;
    case Effect_BUCKETS:    name = "BUCKETS";       break;
    case Effect_BASS_PULSE: name = "BASS_PULSE";    break;
    default:                                        break;
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

