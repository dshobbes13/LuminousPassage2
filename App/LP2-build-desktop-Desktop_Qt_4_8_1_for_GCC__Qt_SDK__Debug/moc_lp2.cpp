/****************************************************************************
** Meta object code from reading C++ file 'LP2.h'
**
** Created: Wed Aug 8 18:27:40 2012
**      by: The Qt Meta Object Compiler version 63 (Qt 4.8.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../LP2/LP2.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'LP2.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_cLP2[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
      18,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: signature, parameters, type, tag, flags
      14,    6,    5,    5, 0x05,

 // slots: signature, parameters, type, tag, flags
      37,    5,    5,    5, 0x08,
      44,    5,    5,    5, 0x08,
      52,    5,    5,    5, 0x08,
      59,    6,    5,    5, 0x08,
      99,   88,    5,    5, 0x08,
     129,    5,    5,    5, 0x08,
     157,  147,    5,    5, 0x08,
     181,    5,    5,    5, 0x08,
     206,    5,    5,    5, 0x08,
     222,    5,    5,    5, 0x08,
     250,  244,    5,    5, 0x08,
     288,    5,    5,    5, 0x08,
     303,    5,    5,    5, 0x08,
     323,    5,    5,    5, 0x08,
     341,    5,    5,    5, 0x08,
     364,    5,    5,    5, 0x08,
     371,    5,    5,    5, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_cLP2[] = {
    "cLP2\0\0message\0NewMessage(QByteArray)\0"
    "Open()\0Close()\0Read()\0"
    "HandleNewMessage(QByteArray)\0newPattern\0"
    "HandleUpdatedPattern(quint8*)\0"
    "UpdateThreshold()\0averaging\0"
    "UpdateAveraging(double)\0"
    "UpdateBucketParameters()\0UpdateBuckets()\0"
    "HandleBucketSliders()\0pItem\0"
    "HandleEffectClicked(QListWidgetItem*)\0"
    "HandleManual()\0HandlePulseSquare()\0"
    "HandlePulseSine()\0HandleDistanceSquare()\0"
    "Save()\0Load()\0"
};

void cLP2::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        cLP2 *_t = static_cast<cLP2 *>(_o);
        switch (_id) {
        case 0: _t->NewMessage((*reinterpret_cast< QByteArray(*)>(_a[1]))); break;
        case 1: _t->Open(); break;
        case 2: _t->Close(); break;
        case 3: _t->Read(); break;
        case 4: _t->HandleNewMessage((*reinterpret_cast< QByteArray(*)>(_a[1]))); break;
        case 5: _t->HandleUpdatedPattern((*reinterpret_cast< quint8*(*)>(_a[1]))); break;
        case 6: _t->UpdateThreshold(); break;
        case 7: _t->UpdateAveraging((*reinterpret_cast< double(*)>(_a[1]))); break;
        case 8: _t->UpdateBucketParameters(); break;
        case 9: _t->UpdateBuckets(); break;
        case 10: _t->HandleBucketSliders(); break;
        case 11: _t->HandleEffectClicked((*reinterpret_cast< QListWidgetItem*(*)>(_a[1]))); break;
        case 12: _t->HandleManual(); break;
        case 13: _t->HandlePulseSquare(); break;
        case 14: _t->HandlePulseSine(); break;
        case 15: _t->HandleDistanceSquare(); break;
        case 16: _t->Save(); break;
        case 17: _t->Load(); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData cLP2::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject cLP2::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_cLP2,
      qt_meta_data_cLP2, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &cLP2::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *cLP2::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *cLP2::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_cLP2))
        return static_cast<void*>(const_cast< cLP2*>(this));
    return QWidget::qt_metacast(_clname);
}

int cLP2::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 18)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 18;
    }
    return _id;
}

// SIGNAL 0
void cLP2::NewMessage(QByteArray _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
QT_END_MOC_NAMESPACE
