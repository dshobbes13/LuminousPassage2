// File: Fft.h

#ifndef FFT_H
#define FFT_H

//******************
// INCLUDES
//******************

#include <QByteArray>
#include <QList>
#include <QStringList>
#include <QWidget>
#include <QVector>

class QLabel;
class QMoveEvent;
class QPaintEvent;
class QResizeEvent;


//******************
// DEFINITIONS
//******************



//******************
// CLASS
//******************

class cFft : public QWidget
{
    Q_OBJECT
    
public:
    cFft( qint32 number, qint32 scale, QStringList labels, QWidget* pParent = 0 );
    ~cFft();

    void SetHysteresis( qreal hysteresis );

    virtual QSize sizeHint( void ) const;
    virtual QSize minimumSizeHint( void ) const;

public slots:
    void UpdateData( QVector<quint32> newData, QVector<quint32> newAverages );
    void Reset( void );

protected:
    virtual void resizeEvent( QResizeEvent* pEvent );
    virtual void paintEvent( QPaintEvent* pEvent );

private:
    qint32 mNumber;
    qint32 mScale;
    QList<QLabel*> mLabelList;
    QVector<quint32> mData;
    QVector<quint32> mAverages;
    QVector<quint32> mPeak;
    qreal mHysteresis;
};

#endif // FFT_H
