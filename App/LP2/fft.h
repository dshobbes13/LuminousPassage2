#ifndef FFT_H
#define FFT_H

#include <QLabel>

class QPaintEvent;

class cFft : public QLabel
{
    Q_OBJECT
    
public:
    cFft( QWidget* pParent = 0 );
    ~cFft();

private:
    void paintEvent( QPaintEvent* pPaintEvent );
};

#endif // FFT_H
