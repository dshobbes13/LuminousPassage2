#include "fft.h"

#include <QPainter>
#include <QPaintEvent>

cFft::cFft( QWidget* pParent )
    : QLabel( pParent )
{
    setText( "HELP2" );
}

cFft::~cFft()
{
}

void cFft::paintEvent( QPaintEvent* pPaintEvent )
{
    /*
    QPainter painter( this );

    painter.drawRect( 0, 0, 200, 200 );
    painter.drawText( 0, 0, "HELP" );

    painter.end();
    */
}
