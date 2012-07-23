#include "lp2.h"

#include <QProgressBar>
#include <QPushButton>
#include <QGridLayout>

#include "SerialDevice.h"
#include "fft.h"

cLP2::cLP2( QWidget* pParent )
    : QWidget( pParent )
{

    for( qint32 i=0; i<NUM_FFT; i++ )
    {
        mpProgress[i] = new QProgressBar( this );
        mpProgress[i]->setOrientation( Qt::Vertical );
        mpProgress[i]->setRange( 0, 255 );
        mpProgress[i]->setValue( 128 );
    }

    mpButton = new QPushButton( "Help" );
    mpSerial = new cSerialDevice( "/dev/tty.usbmodemfd131" );

    QGridLayout* pGridLayout = new QGridLayout( this );
    pGridLayout->addWidget( mpButton, 0, 0, 1, -1 );
    for( qint32 i=0; i<NUM_FFT; i++ )
    {
        pGridLayout->addWidget( mpProgress[i], 1, i );
    }

    mpFft = new cFft( this );

    connect( mpSerial, SIGNAL( readyRead() ), this, SLOT( Read() ) );

    mBytes = 0;
}

cLP2::~cLP2()
{
}

void cLP2::Read( void )
{
    if( mpSerial->bytesAvailable() >= NUM_FFT )
    {
        mpSerial->read( (char*)mData, NUM_FFT );
        for( qint32 i=0; i<NUM_FFT; i++ )
        {
            mpProgress[i]->setValue( mData[i] );
        }
    }
}
