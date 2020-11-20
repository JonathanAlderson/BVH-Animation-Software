/****************************************************************************
** Meta object code from reading C++ file 'MasterWidget.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.9.5)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "MasterWidget.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'MasterWidget.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.9.5. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_MasterWidget_t {
    QByteArrayData data[22];
    char stringdata0[219];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_MasterWidget_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_MasterWidget_t qt_meta_stringdata_MasterWidget = {
    {
QT_MOC_LITERAL(0, 0, 12), // "MasterWidget"
QT_MOC_LITERAL(1, 13, 6), // "rewind"
QT_MOC_LITERAL(2, 20, 0), // ""
QT_MOC_LITERAL(3, 21, 11), // "fastForward"
QT_MOC_LITERAL(4, 33, 5), // "pause"
QT_MOC_LITERAL(5, 39, 4), // "play"
QT_MOC_LITERAL(6, 44, 4), // "stop"
QT_MOC_LITERAL(7, 49, 9), // "playPause"
QT_MOC_LITERAL(8, 59, 10), // "updateText"
QT_MOC_LITERAL(9, 70, 7), // "frameNo"
QT_MOC_LITERAL(10, 78, 13), // "playbackSpeed"
QT_MOC_LITERAL(11, 92, 11), // "addKeyframe"
QT_MOC_LITERAL(12, 104, 11), // "setKeyframe"
QT_MOC_LITERAL(13, 116, 12), // "lerpKeyframe"
QT_MOC_LITERAL(14, 129, 8), // "toggleIK"
QT_MOC_LITERAL(15, 138, 15), // "toggleDampening"
QT_MOC_LITERAL(16, 154, 13), // "toggleControl"
QT_MOC_LITERAL(17, 168, 12), // "lambdaUpdate"
QT_MOC_LITERAL(18, 181, 1), // "i"
QT_MOC_LITERAL(19, 183, 11), // "xGainUpdate"
QT_MOC_LITERAL(20, 195, 11), // "yGainUpdate"
QT_MOC_LITERAL(21, 207, 11) // "zGainUpdate"

    },
    "MasterWidget\0rewind\0\0fastForward\0pause\0"
    "play\0stop\0playPause\0updateText\0frameNo\0"
    "playbackSpeed\0addKeyframe\0setKeyframe\0"
    "lerpKeyframe\0toggleIK\0toggleDampening\0"
    "toggleControl\0lambdaUpdate\0i\0xGainUpdate\0"
    "yGainUpdate\0zGainUpdate"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_MasterWidget[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
      17,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    0,   99,    2, 0x0a /* Public */,
       3,    0,  100,    2, 0x0a /* Public */,
       4,    0,  101,    2, 0x0a /* Public */,
       5,    0,  102,    2, 0x0a /* Public */,
       6,    0,  103,    2, 0x0a /* Public */,
       7,    0,  104,    2, 0x0a /* Public */,
       8,    2,  105,    2, 0x0a /* Public */,
      11,    0,  110,    2, 0x0a /* Public */,
      12,    0,  111,    2, 0x0a /* Public */,
      13,    0,  112,    2, 0x0a /* Public */,
      14,    0,  113,    2, 0x0a /* Public */,
      15,    0,  114,    2, 0x0a /* Public */,
      16,    0,  115,    2, 0x0a /* Public */,
      17,    1,  116,    2, 0x0a /* Public */,
      19,    1,  119,    2, 0x0a /* Public */,
      20,    1,  122,    2, 0x0a /* Public */,
      21,    1,  125,    2, 0x0a /* Public */,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int, QMetaType::Float,    9,   10,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int,   18,
    QMetaType::Void, QMetaType::Int,   18,
    QMetaType::Void, QMetaType::Int,   18,
    QMetaType::Void, QMetaType::Int,   18,

       0        // eod
};

void MasterWidget::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        MasterWidget *_t = static_cast<MasterWidget *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->rewind(); break;
        case 1: _t->fastForward(); break;
        case 2: _t->pause(); break;
        case 3: _t->play(); break;
        case 4: _t->stop(); break;
        case 5: _t->playPause(); break;
        case 6: _t->updateText((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< float(*)>(_a[2]))); break;
        case 7: _t->addKeyframe(); break;
        case 8: _t->setKeyframe(); break;
        case 9: _t->lerpKeyframe(); break;
        case 10: _t->toggleIK(); break;
        case 11: _t->toggleDampening(); break;
        case 12: _t->toggleControl(); break;
        case 13: _t->lambdaUpdate((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 14: _t->xGainUpdate((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 15: _t->yGainUpdate((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 16: _t->zGainUpdate((*reinterpret_cast< int(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObject MasterWidget::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_MasterWidget.data,
      qt_meta_data_MasterWidget,  qt_static_metacall, nullptr, nullptr}
};


const QMetaObject *MasterWidget::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *MasterWidget::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_MasterWidget.stringdata0))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int MasterWidget::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 17)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 17;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 17)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 17;
    }
    return _id;
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
