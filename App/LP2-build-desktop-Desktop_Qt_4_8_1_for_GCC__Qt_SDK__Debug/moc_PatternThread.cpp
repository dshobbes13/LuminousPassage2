/****************************************************************************
** Meta object code from reading C++ file 'PatternThread.h'
**
** Created: Thu Aug 2 07:38:27 2012
**      by: The Qt Meta Object Compiler version 63 (Qt 4.8.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../LP2/PatternThread.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'PatternThread.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_cPatternThread[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       1,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: signature, parameters, type, tag, flags
      27,   16,   15,   15, 0x05,

       0        // eod
};

static const char qt_meta_stringdata_cPatternThread[] = {
    "cPatternThread\0\0newPattern\0"
    "UpdatedPattern(quint8*)\0"
};

void cPatternThread::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        cPatternThread *_t = static_cast<cPatternThread *>(_o);
        switch (_id) {
        case 0: _t->UpdatedPattern((*reinterpret_cast< quint8*(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData cPatternThread::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject cPatternThread::staticMetaObject = {
    { &QThread::staticMetaObject, qt_meta_stringdata_cPatternThread,
      qt_meta_data_cPatternThread, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &cPatternThread::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *cPatternThread::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *cPatternThread::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_cPatternThread))
        return static_cast<void*>(const_cast< cPatternThread*>(this));
    return QThread::qt_metacast(_clname);
}

int cPatternThread::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QThread::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 1)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 1;
    }
    return _id;
}

// SIGNAL 0
void cPatternThread::UpdatedPattern(quint8 * _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
QT_END_MOC_NAMESPACE
