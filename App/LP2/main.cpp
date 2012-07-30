// File: main.cpp

//******************
// INCLUDES
//******************

#include <QApplication>

#include "LP2.h"
#include "Fft.h"


//******************
// MAIN
//******************

int main( int argc, char* argv[] )
{
    QApplication a( argc, argv );
    cLP2 w;
    //cFft w( 64 );
    w.show();

    return a.exec();
}
