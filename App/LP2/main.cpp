#include <QApplication>
#include "lp2.h"

int main( int argc, char* argv[] )
{
    QApplication a( argc, argv );
    cLP2 w;
    w.show();
    
    return a.exec();
}
