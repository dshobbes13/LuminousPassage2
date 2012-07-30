// File: Lights.cpp

//******************
// INCLUDES
//******************

#include "Lights.h"

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
    const qint32 DIAMETER = 20;
}

//******************
// CLASS
//******************

cLights::cLights( quint8 number, QWidget* pParent )
    : QWidget( pParent )
    , mNumber( number )
{
    mData.fill( 128, mNumber );
}

cLights::~cLights()
{
}

QSize cLights::sizeHint( void ) const
{
    return QSize( DIAMETER*mNumber, DIAMETER*2 );
}

QSize cLights::minimumSizeHint( void ) const
{
    return QSize( DIAMETER*mNumber, DIAMETER*2 );
}

void cLights::UpdateData( QVector<quint8> newData )
{
    if( newData.size() == mNumber )
    {
        mData = newData;
    }
    update();
}

void cLights::Reset( void )
{
    mData.fill( 128, mNumber );
    update();
}

void cLights::paintEvent( QPaintEvent* pEvent )
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

    for( qint32 i=0; i<mNumber; i++ )
    {
        qreal centerX = ( step * i ) - offset;
        qreal centerY1 = h / 4;
        qreal centerY2 = 3 * ( h / 4 );
        qreal centerY3 = h / 2;
        qreal x = centerX - ( DIAMETER / 2 );
        qreal y1 = centerY1 - ( DIAMETER / 2  );
        qreal y2 = centerY2 - ( DIAMETER / 2  );
        qreal y3 = centerY3 - ( DIAMETER / 2  );
        QColor pink( Qt::magenta );
        QColor blue( Qt::blue );
        QColor green( Qt::green );
        quint8 alpha = mData.at(i);
        pink.setAlpha( alpha );
        blue.setAlpha( alpha );
        green.setAlpha( alpha );
        if( i < 40 )
        {
            painter.setBrush( pink );
            painter.drawEllipse( x, y1, DIAMETER, DIAMETER );
            painter.setBrush( blue );
            painter.drawEllipse( x, y2, DIAMETER, DIAMETER );
        }
        else
        {
            painter.setBrush( green );
            painter.drawEllipse( x, y3, DIAMETER, DIAMETER );
        }
    }

    painter.end();
}
