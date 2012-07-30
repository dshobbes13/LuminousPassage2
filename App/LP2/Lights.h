// File: Lights.h

#ifndef LIGHTS_H
#define LIGHTS_H

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

class cLights : public QWidget
{
    Q_OBJECT
    
public:
    cLights( quint8 number, QWidget* pParent = 0 );
    ~cLights();

    virtual QSize sizeHint( void ) const;
    virtual QSize minimumSizeHint( void ) const;

public slots:
    void UpdateData( QVector<quint8> newData );
    void Reset( void );

protected:
    virtual void paintEvent( QPaintEvent* pEvent );

private:
    quint8 mNumber;
    QVector<quint8> mData;
};

#endif // LIGHTS_H
