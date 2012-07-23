#ifndef LP2_H
#define LP2_H

#include <QDialog>

class QProgressBar;
class QPushButton;

class cFft;
class cSerialDevice;

#define NUM_FFT     64

class cLP2 : public QWidget
{
    Q_OBJECT
    
public:
    cLP2( QWidget* pParent = 0 );
    ~cLP2();

private slots:
    void Read( void );

private:
    QPushButton* mpButton;
    QProgressBar* mpProgress[NUM_FFT];

    cSerialDevice* mpSerial;

    cFft* mpFft;

    quint8 mData[NUM_FFT];
    quint32 mBytes;

};

#endif // LP2_H
