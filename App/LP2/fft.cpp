// File: Fft.cpp

//******************
// INCLUDES
//******************

#include "fft.h"

#include <QDebug>
#include <QFont>
#include <QFontMetrics>
#include <QLabel>
#include <QMoveEvent>
#include <QPainter>
#include <QPaintEvent>
#include <QPoint>
#include <QResizeEvent>
#include <QSize>
#include <QStringList>


//******************
// DEFINITIONS
//******************

namespace
{
    const qint32 WIDTH = 20;
    const qint32 HEIGHT = 256;

    const qreal BAR_PERCENT_W = 0.8;
    const qreal BAR_PERCENT_H = 0.9;
}

//******************
// CLASS
//******************

cFft::cFft( qint32 number, qint32 scale, QStringList labels, QWidget* pParent )
    : QWidget( pParent )
    , mNumber( number )
    , mScale( scale )
{
    for( qint32 i=0; i<mNumber; i++ )
    {
        QString text = ( labels.size() > i ) ? labels.at(i) : "";
        QLabel* pLabel = new QLabel( text, this );
        pLabel->setAlignment( Qt::AlignHCenter | Qt::AlignVCenter );
        mLabelList.append( pLabel );
    }

    setMinimumWidth( WIDTH * mNumber );
    setMinimumHeight( HEIGHT );
    setSizePolicy( QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding );

    QFont f = font();
    f.setPointSize( 10 );
    setFont( f );

    mData.fill( 0, mNumber );
    mAverages.fill( 0, mNumber );
    mPeak.fill( 0, mNumber );
}

cFft::~cFft()
{
}

QSize cFft::sizeHint( void ) const
{
    return QSize( WIDTH*mNumber, HEIGHT );
}

QSize cFft::minimumSizeHint( void ) const
{
    return QSize( WIDTH*mNumber, HEIGHT );
}

void cFft::UpdateData( QVector<quint32> newData, QVector<quint32> newAverages )
{
    if( newData.size() == mNumber )
    {
        mData = newData;
    }
    if( newAverages.size() == mNumber )
    {
        mAverages = newAverages;
    }
    for( qint32 i=0; i<mData.size(); i++ )
    {
        quint32 newData = mData.at(i);
        quint32 lastPeak = mPeak.at(i);
        if( newData > lastPeak )
        {
            mPeak.replace( i, newData );
        }
    }
    update();
}

void cFft::Reset( void )
{
    mData.fill( 0, mNumber );
    mAverages.fill( 0, mNumber );
    mPeak.fill( 0, mNumber );
    update();
}

void cFft::resizeEvent( QResizeEvent* pEvent )
{
    QSize size = pEvent->size();

    qreal w = size.width();
    qreal h = size.height();

    qreal step = ( w / mNumber );
    qreal offset = step / 2;

    qreal textHeight = h * ( 1 - BAR_PERCENT_H ) / 2;
    qreal textRow1 = h * BAR_PERCENT_H;
    qreal textRow2 = textRow1 + textHeight;

    for( qint32 i=0; i<mNumber; i++ )
    {
        qint32 textX = step * i;
        if( i & 0x01 )
        {
            mLabelList.at(i)->setGeometry( textX, textRow2, step, textHeight );
        }
        else
        {
            mLabelList.at(i)->setGeometry( textX, textRow1, step, textHeight );
        }
    }
}

void cFft::paintEvent( QPaintEvent* pEvent )
{
    QPainter painter( this );

    painter.setRenderHint( QPainter::Antialiasing );

    qreal w = width();
    qreal h = height();
    painter.drawLine( 0, 0, w, 0 );
    painter.drawLine( w, 0, w, h );
    painter.drawLine( w, h, 0, h );
    painter.drawLine( 0, h, 0, 0 );

    qreal step = ( w / mNumber );
    qreal offset = step / 2;

    qreal barWidth = step * BAR_PERCENT_W;
    qreal barHeight = h * BAR_PERCENT_H;
    qreal barOffset = offset - ( barWidth / 2 );

    for( qint32 i=0; i<mNumber; i++ )
    {
        qreal x = step * i;
        /*
        painter.setPen( QPen( Qt::black ) );
        painter.drawLine( x + offset, 0, x + offset, h );
        painter.setPen( QPen( Qt::red ) );
        painter.drawLine( x + barOffset, 0, x + barOffset, h );
        painter.setPen( QPen( Qt::black ) );
        painter.drawLine( x + barOffset, h/2, x + barOffset + barWidth, h/2 );
        */

        // Draw bar outline
        painter.setPen( QPen( Qt::black ) );
        QRect barRect( x + barOffset, 0, barWidth, barHeight );
        painter.drawRect( barRect );

        // Draw bar fill
        quint32 value = mData.at(i);
        qreal percent = (qreal)value / mScale;
        qreal startY = barHeight * ( 1 - percent );
        qreal fillH = barHeight - startY;
        painter.setBrush( QBrush( Qt::blue ) );
        painter.drawRect( QRect( x + barOffset, startY, barWidth, fillH ) );

        // Draw average
        quint32 avgValue = mAverages.at(i);
        qreal avgPercent = (qreal)avgValue / mScale;
        qreal avgY = barHeight * ( 1 - avgPercent );
        painter.setBrush( QBrush() );
        painter.setPen( QPen( QBrush( Qt::black ), 5 ) );
        painter.drawLine( x + barOffset, avgY, x + barOffset + barWidth, avgY );

        QRect textRect1( x + barOffset,  0, barWidth, 15 );
        QRect textRect2( x + barOffset, 15, barWidth, 15 );
        painter.setPen( QPen( QBrush( Qt::black ), 3 ) );
        painter.drawText( textRect1, Qt::AlignHCenter | Qt::AlignVCenter, QString::number( mPeak.at(i) ) );
        painter.drawText( textRect2, Qt::AlignHCenter | Qt::AlignVCenter, QString::number( avgValue ) );
    }

    painter.end();
}
