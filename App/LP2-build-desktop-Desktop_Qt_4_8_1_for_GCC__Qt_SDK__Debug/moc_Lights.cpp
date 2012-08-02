/****************************************************************************
** Meta object code from reading C++ file 'Lights.h'
**
** Created: Wed Aug 1 22:07:59 2012
**      by: The Qt Meta Object Compiler version 63 (Qt 4.8.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../LP2/Lights.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'Lights.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_cLights[] = {

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
      17,    9,    8,    8, 0x0a,
      45,    8,    8,    8, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_cLights[] = {
    "cLights\0\0newData\0UpdateData(QVector<quint8>)\0"
    "Reset()\0"
};

void cLights::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        cLights *_t = static_cast<cLights *>(_o);
        switch (_id) {
        case 0: _t->UpdateData((*reinterpret_cast< QVector<quint8>(*)>(_a[1]))); break;
        case 1: _t->Reset(); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData cLights::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject cLights::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_cLights,
      qt_meta_data_cLights, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &cLights::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *cLights::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *cLights::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_cLights))
        return static_cast<void*>(const_cast< cLights*>(this));
    return QWidget::qt_metacast(_clname);
}

int cLights::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
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
