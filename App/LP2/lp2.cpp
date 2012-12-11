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
#include <QSettings>
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

#define READ_TIMEOUT        25
#define READ_BUFFER_LENGTH  4096
#define MESSAGE_LENGTH_RX   64

#define MAX_MESSAGE_COUNT   50

#define SETTINGS_FILE   "Settings"

#define SETTINGS_HYSTERESIS     "Hysteresis"
#define SETTINGS_THRESHOLD      "Threshold"
#define SETTINGS_AVERAGING      "Averaging"
#define SETTINGS_LO             "Lo"
#define SETTINGS_HI             "Hi"
#define SETTINGS_SECONDS        "Seconds"
#define SETTINGS_TIME_FLAG      "TimeFlag"

#define SETTINGS_EFFECT         "Effect"

#define SETTINGS_MANUAL_VALUE   "ManualValue"

#define SETTINGS_PULSE_SQUARE_SOURCE    "PulseSquareSource"
#define SETTINGS_PULSE_SQUARE_LENGTH    "PulseSquareLength"
#define SETTINGS_PULSE_SQUARE_WIDTH     "PulseSquareWidth"

#define SETTINGS_PULSE_SINE_SOURCE    "PulseSineSource"
#define SETTINGS_PULSE_SINE_LENGTH    "PulseSineLength"
#define SETTINGS_PULSE_SINE_WIDTH     "PulseSineWidth"
#define SETTINGS_PULSE_SINE_SPEED     "PulseSineSpeed"

#define SETTINGS_DISTANCE_SQUARE_SOURCE     "DistanceSquareSource"
#define SETTINGS_DISTANCE_SQUARE_START      "DistanceSquareStart"
#define SETTINGS_DISTANCE_SQUARE_STOP       "DistanceSquareStop"
#define SETTINGS_DISTANCE_SQUARE_AMP        "DistanceSquareAmp"

#define SETTINGS_SWING_SOURCE   "SwingSource"
#define SETTINGS_SWING_START    "SwingStart"
#define SETTINGS_SWING_STOP     "SwingStop"
#define SETTINGS_SWING_PERIOD   "SwingPeriod"

#define SETTINGS_PULSE_CENTER_SOURCE    "PulseCenterSource"
#define SETTINGS_PULSE_CENTER_WIDTH     "PulseCenterWidth"
#define SETTINGS_PULSE_CENTER_SPEED     "PulseCenterSpeed"

#define SETTINGS_DROP_CYCLE_SOURCE    "DropCycleSource"
#define SETTINGS_DROP_CYCLE_SPEED     "DropCycleSpeed"

#define SETTINGS_PULSE_RIGHT_SOURCE    "PulseRightSource"
#define SETTINGS_PULSE_RIGHT_LENGTH    "PulseRightLength"
#define SETTINGS_PULSE_RIGHT_WIDTH     "PulseRightWidth"
#define SETTINGS_PULSE_RIGHT_SPEED     "PulseRightSpeed"

//******************
// CLASS
//******************

cLP2::cLP2( QWidget* pParent )
    : QWidget( pParent )
    , mpSerial( NULL )
{

    QSettings settings( SETTINGS_FILE, QSettings::NativeFormat );

    // Serial port, open/close
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

    // FFT display
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

    // Bucket display
    QStringList bucketLabels;
    bucketLabels << "1" << "2" << "3" << "4" << "5" << "6";
    mpBuckets = new cFft( GLOBAL_NUM_BUCKETS, 512, bucketLabels, this );
    mpBuckets->SetHysteresis( settings.value( SETTINGS_HYSTERESIS, 0.25 ).toDouble() );

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
    mpThreshold->setValue( settings.value( SETTINGS_THRESHOLD, 8 ).toUInt() );
    connect( mpThreshold, SIGNAL( valueChanged( int ) ), this, SLOT( UpdateAudioParameters() ) );

    mpAveraging = new QDoubleSpinBox( this );
    mpAveraging->setMinimum( 0.0 );
    mpAveraging->setMaximum( 1.0 );
    mpAveraging->setDecimals( 2 );
    mpAveraging->setSingleStep( 0.05 );
    mpAveraging->setSuffix( " avg" );
    mpAveraging->setValue( settings.value( SETTINGS_AVERAGING, 0.9 ).toDouble() );
    connect( mpAveraging, SIGNAL( valueChanged( double ) ), this, SLOT( UpdateAudioParameters() ) );

    for( qint32 i=0; i<GLOBAL_NUM_BUCKETS; i++ )
    {
        mpLo[i] = new QSpinBox( this );
        mpHi[i] = new QSpinBox( this );
        mpLo[i]->setRange( 1, GLOBAL_NUM_FREQ-1 );
        mpHi[i]->setRange( 1, GLOBAL_NUM_FREQ-1 );
        mpLo[i]->setValue( settings.value( SETTINGS_LO + QString::number(i), 0 ).toUInt() );
        mpHi[i]->setValue( settings.value( SETTINGS_HI + QString::number(i), 0 ).toUInt() );
        connect( mpLo[i], SIGNAL( valueChanged( int ) ), this, SLOT( UpdateAudioParameters() ) );
        connect( mpHi[i], SIGNAL( valueChanged( int ) ), this, SLOT( UpdateAudioParameters() ) );
    }

    mpHysteresis = new QDoubleSpinBox( this );
    mpHysteresis->setMinimum( 0.0 );
    mpHysteresis->setMaximum( 1.0 );
    mpHysteresis->setDecimals( 2 );
    mpHysteresis->setSingleStep( 0.01 );
    mpHysteresis->setSuffix( " hyst" );
    mpHysteresis->setValue( settings.value( SETTINGS_HYSTERESIS, 0.25 ).toDouble() );
    connect( mpHysteresis, SIGNAL( valueChanged( double ) ), this, SLOT( UpdateBucketParameters() ) );

    mpSeconds = new QSpinBox( this );
    mpSeconds->setRange( 0, 10 );
    mpSeconds->setSuffix( " sec" );
    mpSeconds->setValue( settings.value( SETTINGS_SECONDS, 0 ).toUInt() );
    connect( mpSeconds, SIGNAL( valueChanged( int ) ), this, SLOT( UpdateBucketParameters() ) );

    for( qint32 i=0; i<GLOBAL_NUM_BUCKETS; i++ )
    {
        mpTimeFlags[i] = new QCheckBox( this );
        mpTimeFlags[i]->setChecked( settings.value( SETTINGS_TIME_FLAG + QString::number(i), false ).toBool() );
        connect( mpTimeFlags[i], SIGNAL( clicked() ), this, SLOT( UpdateBucketParameters() ) );
    }

    // Lights display
    mpLights = new cLights( GLOBAL_NUM_CHANNELS, this );

    mpEffects = new QListWidget( this );
    mpEffects->setSelectionMode( QAbstractItemView::MultiSelection );
    for( qint32 i=0; i<Effect_MAX; i++ )
    {
        QListWidgetItem* pItem = new QListWidgetItem( GetEffectName( (eEffect)i ) );
        pItem->setData( Qt::UserRole, i );
        mpEffects->addItem( pItem );
        pItem->setSelected( settings.value( SETTINGS_EFFECT + QString::number(i), false ).toBool() );
    }
    connect( mpEffects, SIGNAL( itemClicked( QListWidgetItem* ) ), this, SLOT( HandleEffectClicked( QListWidgetItem* ) ) );

    // Effect MANUAL
    mpManualValueSlider = new QSlider( Qt::Horizontal, this );
    mpManualValueSlider->setRange( 0, 255 );
    mpManualValueSlider->setValue( settings.value( SETTINGS_MANUAL_VALUE, 128 ).toUInt() );
    connect( mpManualValueSlider, SIGNAL( valueChanged( int ) ), this, SLOT( HandleManual() ) );

    // Effect PULSE SQUARE
    mpPulseSquareSourceSpin = new QSpinBox( this );
    mpPulseSquareSourceSpin->setRange( 0, GLOBAL_NUM_BUCKETS );
    mpPulseSquareSourceSpin->setSuffix( " source" );
    mpPulseSquareLengthSpin = new QSpinBox( this );
    mpPulseSquareLengthSpin->setRange( 0, 64 );
    mpPulseSquareLengthSpin->setSuffix( " length" );
    mpPulseSquareWidthSpin = new QSpinBox( this );
    mpPulseSquareWidthSpin->setRange( 0, 64 );
    mpPulseSquareWidthSpin->setSuffix( " width" );
    mpPulseSquareSourceSpin->setValue( settings.value( SETTINGS_PULSE_SQUARE_SOURCE, 1 ).toUInt() );
    mpPulseSquareLengthSpin->setValue( settings.value( SETTINGS_PULSE_SQUARE_LENGTH, 40 ).toUInt() );
    mpPulseSquareWidthSpin->setValue( settings.value( SETTINGS_PULSE_SQUARE_WIDTH, 10 ).toUInt() );
    connect( mpPulseSquareSourceSpin, SIGNAL( valueChanged( int ) ), this, SLOT( HandlePulseSquare() ) );
    connect( mpPulseSquareLengthSpin, SIGNAL( valueChanged( int ) ), this, SLOT( HandlePulseSquare() ) );
    connect( mpPulseSquareWidthSpin, SIGNAL( valueChanged( int ) ), this, SLOT( HandlePulseSquare() ) );

    // Effect PULSE SINE
    mpPulseSineSourceSpin = new QSpinBox( this );
    mpPulseSineSourceSpin->setRange( 0, GLOBAL_NUM_BUCKETS );
    mpPulseSineSourceSpin->setSuffix( " source" );
    mpPulseSineLengthSpin = new QSpinBox( this );
    mpPulseSineLengthSpin->setRange( 0, 64 );
    mpPulseSineLengthSpin->setSuffix( " length" );
    mpPulseSineWidthSpin = new QSpinBox( this );
    mpPulseSineWidthSpin->setRange( 0, 64 );
    mpPulseSineWidthSpin->setSuffix( " width" );
    mpPulseSineSpeedSpin = new QSpinBox( this );
    mpPulseSineSpeedSpin->setRange( 0, 10 );
    mpPulseSineSpeedSpin->setSuffix( " speed" );
    mpPulseSineSourceSpin->setValue( settings.value( SETTINGS_PULSE_SINE_SOURCE, 1 ).toUInt() );
    mpPulseSineLengthSpin->setValue( settings.value( SETTINGS_PULSE_SINE_LENGTH, 40 ).toUInt() );
    mpPulseSineWidthSpin->setValue( settings.value( SETTINGS_PULSE_SINE_WIDTH, 10 ).toUInt() );
    mpPulseSineSpeedSpin->setValue( settings.value( SETTINGS_PULSE_SINE_SPEED, 5 ).toUInt() );
    connect( mpPulseSineSourceSpin, SIGNAL( valueChanged( int ) ), this, SLOT( HandlePulseSine() ) );
    connect( mpPulseSineLengthSpin, SIGNAL( valueChanged( int ) ), this, SLOT( HandlePulseSine() ) );
    connect( mpPulseSineWidthSpin, SIGNAL( valueChanged( int ) ), this, SLOT( HandlePulseSine() ) );
    connect( mpPulseSineSpeedSpin, SIGNAL( valueChanged( int ) ), this, SLOT( HandlePulseSine() ) );

    // Effect DISTANCE SQUARE
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
    mpDistanceSquareSourceSpin->setValue( settings.value( SETTINGS_DISTANCE_SQUARE_SOURCE, 2 ).toUInt() );
    mpDistanceSquareStartSpin->setValue( settings.value( SETTINGS_DISTANCE_SQUARE_START, 50 ).toUInt() );
    mpDistanceSquareStopSpin->setValue( settings.value( SETTINGS_DISTANCE_SQUARE_STOP, 59 ).toUInt() );
    mpDistanceSquareAmpSpin->setValue( settings.value( SETTINGS_DISTANCE_SQUARE_AMP, 1 ).toUInt() );
    connect( mpDistanceSquareSourceSpin, SIGNAL( valueChanged( int ) ), this, SLOT( HandleDistanceSquare() ) );
    connect( mpDistanceSquareStartSpin, SIGNAL( valueChanged( int ) ), this, SLOT( HandleDistanceSquare() ) );
    connect( mpDistanceSquareStopSpin, SIGNAL( valueChanged( int ) ), this, SLOT( HandleDistanceSquare() ) );
    connect( mpDistanceSquareAmpSpin, SIGNAL( valueChanged( int ) ), this, SLOT( HandleDistanceSquare() ) );

    // Effect_SWING
    mpSwingSourceSpin = new QSpinBox( this );
    mpSwingSourceSpin->setRange( 0, 5 );
    mpSwingSourceSpin->setSuffix( " source" );
    mpSwingStartSpin = new QSpinBox( this );
    mpSwingStartSpin->setRange( 0, GLOBAL_NUM_CHANNELS );
    mpSwingStartSpin->setSuffix( " start" );
    mpSwingStopSpin = new QSpinBox( this );
    mpSwingStopSpin->setRange( 0, GLOBAL_NUM_CHANNELS );
    mpSwingStopSpin->setSuffix( " stop" );
    mpSwingPeriodSpin = new QSpinBox( this );
    mpSwingPeriodSpin->setRange( 0, 500 );
    mpSwingPeriodSpin->setSuffix( " period (1/100)s" );
    mpSwingSourceSpin->setValue( settings.value( SETTINGS_SWING_SOURCE, 5 ).toUInt() );
    mpSwingStartSpin->setValue( settings.value( SETTINGS_SWING_START, 0 ).toUInt() );
    mpSwingStopSpin->setValue( settings.value( SETTINGS_SWING_STOP, 49 ).toUInt() );
    mpSwingPeriodSpin->setValue( settings.value( SETTINGS_SWING_PERIOD, 200 ).toUInt() );
    connect( mpSwingSourceSpin, SIGNAL( valueChanged( int ) ), this, SLOT( HandleSwing() ) );
    connect( mpSwingStartSpin, SIGNAL( valueChanged( int ) ), this, SLOT( HandleSwing() ) );
    connect( mpSwingStopSpin, SIGNAL( valueChanged( int ) ), this, SLOT( HandleSwing() ) );
    connect( mpSwingPeriodSpin, SIGNAL( valueChanged( int ) ), this, SLOT( HandleSwing() ) );

    // Effect PULSE CENTER
    mpPulseCenterSourceSpin = new QSpinBox( this );
    mpPulseCenterSourceSpin->setRange( 0, GLOBAL_NUM_BUCKETS );
    mpPulseCenterSourceSpin->setSuffix( " source" );
    mpPulseCenterWidthSpin = new QSpinBox( this );
    mpPulseCenterWidthSpin->setRange( 0, 64 );
    mpPulseCenterWidthSpin->setSuffix( " width" );
    mpPulseCenterSpeedSpin = new QSpinBox( this );
    mpPulseCenterSpeedSpin->setRange( 0, 10 );
    mpPulseCenterSpeedSpin->setSuffix( " speed" );
    mpPulseCenterSourceSpin->setValue( settings.value( SETTINGS_PULSE_CENTER_SOURCE, 1 ).toUInt() );
    mpPulseCenterWidthSpin->setValue( settings.value( SETTINGS_PULSE_CENTER_WIDTH, 8 ).toUInt() );
    mpPulseCenterSpeedSpin->setValue( settings.value( SETTINGS_PULSE_CENTER_SPEED, 5 ).toUInt() );
    connect( mpPulseCenterSourceSpin, SIGNAL( valueChanged( int ) ), this, SLOT( HandlePulseCenter() ) );
    connect( mpPulseCenterWidthSpin, SIGNAL( valueChanged( int ) ), this, SLOT( HandlePulseCenter() ) );
    connect( mpPulseCenterSpeedSpin, SIGNAL( valueChanged( int ) ), this, SLOT( HandlePulseCenter() ) );

    // Effect DROP CYCLE
    mpDropCycleSourceSpin = new QSpinBox( this );
    mpDropCycleSourceSpin->setRange( 0, GLOBAL_NUM_BUCKETS );
    mpDropCycleSourceSpin->setSuffix( " source" );
    mpDropCycleSpeedSpin = new QSpinBox( this );
    mpDropCycleSpeedSpin->setRange( 0, 64 );
    mpDropCycleSpeedSpin->setSuffix( " speed" );
    mpDropCycleSourceSpin->setValue( settings.value( SETTINGS_DROP_CYCLE_SOURCE, 1 ).toUInt() );
    mpDropCycleSpeedSpin->setValue( settings.value( SETTINGS_DROP_CYCLE_SPEED, 5 ).toUInt() );
    connect( mpDropCycleSourceSpin, SIGNAL( valueChanged( int ) ), this, SLOT( HandleDropCycle() ) );
    connect( mpDropCycleSpeedSpin, SIGNAL( valueChanged( int ) ), this, SLOT( HandleDropCycle() ) );

    // Effect PULSE RIGHT
    mpPulseRightSourceSpin = new QSpinBox( this );
    mpPulseRightSourceSpin->setRange( 0, GLOBAL_NUM_BUCKETS );
    mpPulseRightSourceSpin->setSuffix( " source" );
    mpPulseRightLengthSpin = new QSpinBox( this );
    mpPulseRightLengthSpin->setRange( 0, 64 );
    mpPulseRightLengthSpin->setSuffix( " length" );
    mpPulseRightWidthSpin = new QSpinBox( this );
    mpPulseRightWidthSpin->setRange( 0, 64 );
    mpPulseRightWidthSpin->setSuffix( " width" );
    mpPulseRightSpeedSpin = new QSpinBox( this );
    mpPulseRightSpeedSpin->setRange( 0, 10 );
    mpPulseRightSpeedSpin->setSuffix( " speed" );
    mpPulseRightSourceSpin->setValue( settings.value( SETTINGS_PULSE_RIGHT_SOURCE, 1 ).toUInt() );
    mpPulseRightLengthSpin->setValue( settings.value( SETTINGS_PULSE_RIGHT_LENGTH, 40 ).toUInt() );
    mpPulseRightWidthSpin->setValue( settings.value( SETTINGS_PULSE_RIGHT_WIDTH, 10 ).toUInt() );
    mpPulseRightSpeedSpin->setValue( settings.value( SETTINGS_PULSE_RIGHT_SPEED, 5 ).toUInt() );
    connect( mpPulseRightSourceSpin, SIGNAL( valueChanged( int ) ), this, SLOT( HandlePulseRight() ) );
    connect( mpPulseRightLengthSpin, SIGNAL( valueChanged( int ) ), this, SLOT( HandlePulseRight() ) );
    connect( mpPulseRightWidthSpin, SIGNAL( valueChanged( int ) ), this, SLOT( HandlePulseRight() ) );
    connect( mpPulseRightSpeedSpin, SIGNAL( valueChanged( int ) ), this, SLOT( HandlePulseRight() ) );

    // Layout
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
    pGridLayout->addWidget( mpThreshold,   2, 0 );
    pGridLayout->addWidget( mpAveraging,   3, 0 );
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
    pGridLayout->addWidget( mpHysteresis,  4, 0 );
    pGridLayout->addWidget( mpSeconds,     5, 0 );
    pHBoxLayout = new QHBoxLayout();
    pHBoxLayout->addWidget( mpTimeFlags[0] );
    pHBoxLayout->addWidget( mpTimeFlags[1] );
    pHBoxLayout->addWidget( mpTimeFlags[2] );
    pHBoxLayout->addWidget( mpTimeFlags[3] );
    pHBoxLayout->addWidget( mpTimeFlags[4] );
    pHBoxLayout->addWidget( mpTimeFlags[5] );
    pGridLayout->addLayout( pHBoxLayout,   6, 0 );
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
    pGridLayout->addWidget( mpPulseSineSpeedSpin, 3, 5 );
    QLabel* pLabelDistanceSquare = new QLabel( "Distance Square:", this );
    pGridLayout->addWidget( pLabelDistanceSquare, 4, 1 );
    pGridLayout->addWidget( mpDistanceSquareSourceSpin, 4, 2 );
    pGridLayout->addWidget( mpDistanceSquareStartSpin, 4, 3 );
    pGridLayout->addWidget( mpDistanceSquareStopSpin, 4, 4 );
    pGridLayout->addWidget( mpDistanceSquareAmpSpin, 4, 5 );
    QLabel* pLabelSwing = new QLabel( "Swing:", this );
    pGridLayout->addWidget( pLabelSwing, 5, 1 );
    pGridLayout->addWidget( mpSwingSourceSpin, 5, 2 );
    pGridLayout->addWidget( mpSwingStartSpin, 5, 3 );
    pGridLayout->addWidget( mpSwingStopSpin, 5, 4 );
    pGridLayout->addWidget( mpSwingPeriodSpin, 5, 5 );
    QLabel* pLabelPulseCenter = new QLabel( "Pulse Center:", this );
    pGridLayout->addWidget( pLabelPulseCenter, 6, 1 );
    pGridLayout->addWidget( mpPulseCenterSourceSpin, 6, 2 );
    pGridLayout->addWidget( mpPulseCenterWidthSpin, 6, 3 );
    pGridLayout->addWidget( mpPulseCenterSpeedSpin, 6, 4 );
    QLabel* pLabelDropCycle = new QLabel( "Drop Cycle:", this );
    pGridLayout->addWidget( pLabelDropCycle, 7, 1 );
    pGridLayout->addWidget( mpDropCycleSourceSpin, 7, 2 );
    pGridLayout->addWidget( mpDropCycleSpeedSpin, 7, 3 );
    QLabel* pLabelPulseRight = new QLabel( "Pulse Right:", this );
    pGridLayout->addWidget( pLabelPulseRight, 8, 1 );
    pGridLayout->addWidget( mpPulseRightSourceSpin, 8, 2 );
    pGridLayout->addWidget( mpPulseRightLengthSpin, 8, 3 );
    pGridLayout->addWidget( mpPulseRightWidthSpin, 8, 4 );
    pGridLayout->addWidget( mpPulseRightSpeedSpin, 8, 5 );
    pGridLayout->setColumnStretch( 1, 1 );
    pGridLayout->setColumnStretch( 2, 1 );
    pGridLayout->setColumnStretch( 3, 1 );
    pGridLayout->setColumnStretch( 4, 1 );
    pMainLayout->addLayout( pGridLayout );

    // Load/Save
    QPushButton* pSaveButton = new QPushButton( "Save", this );
    QPushButton* pLoadButton = new QPushButton( "Load", this );
    pMainLayout->addWidget( pSaveButton );
    pMainLayout->addWidget( pLoadButton );
    connect( pSaveButton, SIGNAL( clicked() ), this, SLOT( Save() ) );
    connect( pLoadButton, SIGNAL( clicked() ), this, SLOT( Load() ) );

    // Pattern updating
    mpPatternThread = new cPatternThread( this );
    connect( mpPatternThread, SIGNAL( UpdatedPattern( quint8* ) ), this, SLOT( HandleUpdatedPattern( quint8* ) ) );
    mpPatternThread->start();

    // Message handling
    connect( this, SIGNAL( NewMessage( QByteArray ) ), this, SLOT( HandleNewMessage( QByteArray ) ) );

    QThread::currentThread()->setPriority( QThread::HighPriority );

    UpdateAudioParameters();
    UpdateBucketParameters();

    HandleManual();
    HandlePulseSquare();
    HandlePulseSine();
    HandleDistanceSquare();
    HandleSwing();
    HandlePulseCenter();
    HandleDropCycle();
    HandlePulseRight();
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
        qint64 bytesRead = 0;
        do
        {
            char buffer[READ_BUFFER_LENGTH];
            bytesRead = mpSerial->read( buffer, READ_BUFFER_LENGTH );
        } while( bytesRead > 0 );
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
    static char mReadBuffer[READ_BUFFER_LENGTH];
    qint64 bytesRead = 0;
    do
    {
        bytesRead = mpSerial->read( mReadBuffer, READ_BUFFER_LENGTH );
        if( bytesRead > 0 )
        {
            mDataRx.append( mReadBuffer, bytesRead );
            // Look for start of frame
            bool foundSof = false;
            for( qint32 i=0; i<mDataRx.size(); i++ )
            {
                if( mDataRx.at(i) == (char)0xFF )
                {
                    foundSof = true;
                    if( i > 0 )
                    {
                        mDataRx.remove( 0, i );
                    }
                    break;
                }
            }
            if( foundSof )
            {
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
        }
    } while( bytesRead > 0 );
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

void cLP2::UpdateAudioParameters( void )
{
    quint8 threshold = mpThreshold->value();
    qreal averaging = mpAveraging->value();
    quint8 mLo[GLOBAL_NUM_BUCKETS];
    quint8 mHi[GLOBAL_NUM_BUCKETS];
    for( qint32 i=0; i<GLOBAL_NUM_BUCKETS; i++ )
    {
        mpLo[i]->setSuffix( QString( " - " + mFftLabels.at( mpLo[i]->value() ) ) );
        mpHi[i]->setSuffix( QString( " - " + mFftLabels.at( mpHi[i]->value() ) ) );
        mLo[i] = mpLo[i]->value();
        mHi[i] = mpHi[i]->value();
    }

    AudioSetParameters( threshold, (float)averaging, mLo, mHi );

    quint8 averagingFixed = 255 * averaging;
    QByteArray data( MESSAGE_LENGTH-1, 0 );

    data.insert( 0, threshold );
    data.insert( 1, averagingFixed );
    data.insert( 2, mLo[0] );
    data.insert( 3, mHi[0] );
    data.insert( 4, mLo[1] );
    data.insert( 5, mHi[1] );
    data.insert( 6, mLo[2] );
    data.insert( 7, mHi[2] );
    data.insert( 8, mLo[3] );
    data.insert( 9, mHi[3] );
    data.insert( 10, mLo[4] );
    data.insert( 11, mHi[4] );
    data.insert( 12, mLo[5] );
    data.insert( 13, mHi[5] );

    SendCommand( Command_AUDIO, data );
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
    char speed = mpPulseSineSpeedSpin->value();
    data.insert( 0, source );
    data.insert( 1, length );
    data.insert( 2, width );
    data.insert( 3, speed );
    PatternSetPulseSine( (quint8)source, (quint8)length, (quint8)width, (quint8)speed );

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

void cLP2::HandleSwing( void )
{
    QByteArray data( MESSAGE_LENGTH-1, 0 );

    char source = mpSwingSourceSpin->value();
    char start = mpSwingStartSpin->value();
    char stop = mpSwingStopSpin->value();
    quint16 period = mpSwingPeriodSpin->value();
    char periodHi = (char)( (quint8)( period >> 8 ) );
    char periodLo = (char)( (quint8)period );
    data.insert( 0, source );
    data.insert( 1, start );
    data.insert( 2, stop );
    data.insert( 3, periodHi );
    data.insert( 4, periodLo );
    PatternSetSwing( (quint8)source, (quint8)start, (quint8)stop, (quint16)period );

    SendCommand( Command_SWING, data );
}

void cLP2::HandlePulseCenter( void )
{
    QByteArray data( MESSAGE_LENGTH-1, 0 );

    char source = mpPulseCenterSourceSpin->value();
    char width = mpPulseCenterWidthSpin->value();
    char speed = mpPulseCenterSpeedSpin->value();
    data.insert( 0, source );
    data.insert( 1, width );
    data.insert( 2, speed );
    PatternSetPulseCenter( (quint8)source, (quint8)width, (quint8)speed );

    SendCommand( Command_PULSE_CENTER, data );
}

void cLP2::HandleDropCycle( void )
{
    QByteArray data( MESSAGE_LENGTH-1, 0 );

    char source = mpDropCycleSourceSpin->value();
    char speed = mpDropCycleSpeedSpin->value();
    data.insert( 0, source );
    data.insert( 1, speed );
    PatternSetDropCycle( (quint8)source, (quint8)speed );

    SendCommand( Command_DROP_CYCLE, data );
}

void cLP2::HandlePulseRight( void )
{
    QByteArray data( MESSAGE_LENGTH-1, 0 );

    char source = mpPulseRightSourceSpin->value();
    char length = mpPulseRightLengthSpin->value();
    char width = mpPulseRightWidthSpin->value();
    char speed = mpPulseRightSpeedSpin->value();
    data.insert( 0, source );
    data.insert( 1, length );
    data.insert( 2, width );
    data.insert( 3, speed );
    PatternSetPulseRight( (quint8)source, (quint8)length, (quint8)width, (quint8)speed );

    SendCommand( Command_PULSE_RIGHT, data );
}

void cLP2::Save( void )
{
    QByteArray data( MESSAGE_LENGTH-1, 0 );
    SendCommand( Command_SAVE, data );

    QSettings settings( SETTINGS_FILE, QSettings::NativeFormat );

    settings.setValue( SETTINGS_HYSTERESIS, mpHysteresis->value() );
    settings.setValue( SETTINGS_THRESHOLD, mpThreshold->value() );
    settings.setValue( SETTINGS_AVERAGING, mpAveraging->value() );
    for( qint32 i=0; i<GLOBAL_NUM_BUCKETS; i++ )
    {
        settings.setValue( SETTINGS_LO + QString::number(i), mpLo[i]->value() );
        settings.setValue( SETTINGS_HI + QString::number(i), mpHi[i]->value() );
    }
    settings.setValue( SETTINGS_SECONDS, mpSeconds->value() );
    for( qint32 i=0; i<GLOBAL_NUM_BUCKETS; i++ )
    {
        settings.setValue( SETTINGS_TIME_FLAG + QString::number(i), mpTimeFlags[i]->isChecked() );
    }

    for( qint32 i=0; i<Effect_MAX; i++ )
    {
        settings.setValue( SETTINGS_EFFECT + QString::number(i), false );
    }
    foreach( QListWidgetItem* pItem, mpEffects->selectedItems() )
    {
        quint32 effect = pItem->data( Qt::UserRole ).toUInt();
        settings.setValue( SETTINGS_EFFECT + QString::number(effect), pItem->isSelected() );
    }

    settings.setValue( SETTINGS_MANUAL_VALUE, mpManualValueSlider->value() );

    settings.setValue( SETTINGS_PULSE_SQUARE_SOURCE, mpPulseSquareSourceSpin->value() );
    settings.setValue( SETTINGS_PULSE_SQUARE_LENGTH, mpPulseSquareLengthSpin->value() );
    settings.setValue( SETTINGS_PULSE_SQUARE_WIDTH, mpPulseSquareWidthSpin->value() );

    settings.setValue( SETTINGS_PULSE_SINE_SOURCE, mpPulseSineSourceSpin->value() );
    settings.setValue( SETTINGS_PULSE_SINE_LENGTH, mpPulseSineLengthSpin->value() );
    settings.setValue( SETTINGS_PULSE_SINE_WIDTH, mpPulseSineWidthSpin->value() );
    settings.setValue( SETTINGS_PULSE_SINE_SPEED, mpPulseSineSpeedSpin->value() );

    settings.setValue( SETTINGS_DISTANCE_SQUARE_SOURCE, mpDistanceSquareSourceSpin->value() );
    settings.setValue( SETTINGS_DISTANCE_SQUARE_START, mpDistanceSquareStartSpin->value() );
    settings.setValue( SETTINGS_DISTANCE_SQUARE_STOP, mpDistanceSquareStopSpin->value() );
    settings.setValue( SETTINGS_DISTANCE_SQUARE_AMP, mpDistanceSquareAmpSpin->value() );

    settings.setValue( SETTINGS_SWING_SOURCE, mpSwingSourceSpin->value() );
    settings.setValue( SETTINGS_SWING_START, mpSwingStartSpin->value() );
    settings.setValue( SETTINGS_SWING_STOP, mpSwingStopSpin->value() );
    settings.setValue( SETTINGS_SWING_PERIOD, mpSwingPeriodSpin->value() );

    settings.setValue( SETTINGS_PULSE_CENTER_SOURCE, mpPulseCenterSourceSpin->value() );
    settings.setValue( SETTINGS_PULSE_CENTER_WIDTH, mpPulseCenterWidthSpin->value() );
    settings.setValue( SETTINGS_PULSE_CENTER_SPEED, mpPulseCenterSpeedSpin->value() );

    settings.setValue( SETTINGS_DROP_CYCLE_SOURCE, mpDropCycleSourceSpin->value() );
    settings.setValue( SETTINGS_DROP_CYCLE_SPEED, mpDropCycleSpeedSpin->value() );

    settings.setValue( SETTINGS_PULSE_RIGHT_SOURCE, mpPulseRightSourceSpin->value() );
    settings.setValue( SETTINGS_PULSE_RIGHT_LENGTH, mpPulseRightLengthSpin->value() );
    settings.setValue( SETTINGS_PULSE_RIGHT_WIDTH, mpPulseRightWidthSpin->value() );
    settings.setValue( SETTINGS_PULSE_RIGHT_SPEED, mpPulseRightSpeedSpin->value() );
}

void cLP2::Load( void )
{
    QByteArray data( MESSAGE_LENGTH-1, 0 );
    SendCommand( Command_LOAD, data );
}

void cLP2::SendCommand( eCommand command, QByteArray data )
{
    char message[MESSAGE_LENGTH] = {0};
    message[0] = (char)command;
    for( qint32 i=1; i<MESSAGE_LENGTH; i++ )
    {
        message[i] = data.at(i-1);
    }

    if( mpSerial )
    {
        mpSerial->write( message, MESSAGE_LENGTH );
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
    case Effect_SWING:           name = "SWING";            break;
    case Effect_PULSE_CENTER:    name = "PULSE_CENTER";     break;
    case Effect_DROP_CYCLE:      name = "DROP_CYCLE";       break;
    case Effect_PULSE_RIGHT:     name = "PULSE_RIGHT";      break;
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

