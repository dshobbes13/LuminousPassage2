/****************************************************************************
** Meta object code from reading C++ file 'Fft.h'
**
** Created: Wed Aug 1 21:27:30 2012
**      by: The Qt Meta Object Compiler version 63 (Qt 4.8.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../LP2/Fft.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'Fft.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_cFft[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       2,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      26,    6,    5,    5, 0x0a,
      72,    5,    5,    5, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_cFft[] = {
    "cFft\0\0newData,newAverages\0"
    "UpdateData(QVector<quint32>,QVector<quint32>)\0"
    "Reset()\0"
};

void cFft::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        cFft *_t = static_cast<cFft *>(_o);
        switch (_id) {
        case 0: _t->UpdateData((*reinterpret_cast< QVector<quint32>(*)>(_a[1])),(*reinterpret_cast< QVector<quint32>(*)>(_a[2]))); break;
        case 1: _t->Reset(); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData cFft::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject cFft::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_cFft,
      qt_meta_data_cFft, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &cFft::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *cFft::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *cFft::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_cFft))
        return static_cast<void*>(const_cast< cFft*>(this));
    return QWidget::qt_metacast(_clname);
}

int cFft::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 2)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 2;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
