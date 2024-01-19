/****************************************************************************
** Meta object code from reading C++ file 'decoder.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.5.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../ffmpegDemo/decoder.h"
#include <QtCore/qmetatype.h>

#if __has_include(<QtCore/qtmochelpers.h>)
#include <QtCore/qtmochelpers.h>
#else
QT_BEGIN_MOC_NAMESPACE
#endif


#include <memory>

#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'decoder.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 68
#error "This file was generated using the moc from 6.5.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

#ifndef Q_CONSTINIT
#define Q_CONSTINIT
#endif

QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
QT_WARNING_DISABLE_GCC("-Wuseless-cast")
namespace {

#ifdef QT_MOC_HAS_STRINGDATA
struct qt_meta_stringdata_CLASSDecoderENDCLASS_t {};
static constexpr auto qt_meta_stringdata_CLASSDecoderENDCLASS = QtMocHelpers::stringData(
    "Decoder",
    "frameInfoUpdateSig",
    "",
    "width",
    "height",
    "format",
    "frameDataUpdateSig",
    "audioDataUpdateSig",
    "const char*",
    "data",
    "len",
    "startDecode",
    "moveThread",
    "url",
    "quitThread",
    "stopRecord"
);
#else  // !QT_MOC_HAS_STRING_DATA
struct qt_meta_stringdata_CLASSDecoderENDCLASS_t {
    uint offsetsAndSizes[32];
    char stringdata0[8];
    char stringdata1[19];
    char stringdata2[1];
    char stringdata3[6];
    char stringdata4[7];
    char stringdata5[7];
    char stringdata6[19];
    char stringdata7[19];
    char stringdata8[12];
    char stringdata9[5];
    char stringdata10[4];
    char stringdata11[12];
    char stringdata12[11];
    char stringdata13[4];
    char stringdata14[11];
    char stringdata15[11];
};
#define QT_MOC_LITERAL(ofs, len) \
    uint(sizeof(qt_meta_stringdata_CLASSDecoderENDCLASS_t::offsetsAndSizes) + ofs), len 
Q_CONSTINIT static const qt_meta_stringdata_CLASSDecoderENDCLASS_t qt_meta_stringdata_CLASSDecoderENDCLASS = {
    {
        QT_MOC_LITERAL(0, 7),  // "Decoder"
        QT_MOC_LITERAL(8, 18),  // "frameInfoUpdateSig"
        QT_MOC_LITERAL(27, 0),  // ""
        QT_MOC_LITERAL(28, 5),  // "width"
        QT_MOC_LITERAL(34, 6),  // "height"
        QT_MOC_LITERAL(41, 6),  // "format"
        QT_MOC_LITERAL(48, 18),  // "frameDataUpdateSig"
        QT_MOC_LITERAL(67, 18),  // "audioDataUpdateSig"
        QT_MOC_LITERAL(86, 11),  // "const char*"
        QT_MOC_LITERAL(98, 4),  // "data"
        QT_MOC_LITERAL(103, 3),  // "len"
        QT_MOC_LITERAL(107, 11),  // "startDecode"
        QT_MOC_LITERAL(119, 10),  // "moveThread"
        QT_MOC_LITERAL(130, 3),  // "url"
        QT_MOC_LITERAL(134, 10),  // "quitThread"
        QT_MOC_LITERAL(145, 10)   // "stopRecord"
    },
    "Decoder",
    "frameInfoUpdateSig",
    "",
    "width",
    "height",
    "format",
    "frameDataUpdateSig",
    "audioDataUpdateSig",
    "const char*",
    "data",
    "len",
    "startDecode",
    "moveThread",
    "url",
    "quitThread",
    "stopRecord"
};
#undef QT_MOC_LITERAL
#endif // !QT_MOC_HAS_STRING_DATA
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_CLASSDecoderENDCLASS[] = {

 // content:
      11,       // revision
       0,       // classname
       0,    0, // classinfo
       7,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       3,       // signalCount

 // signals: name, argc, parameters, tag, flags, initial metatype offsets
       1,    3,   56,    2, 0x06,    1 /* Public */,
       6,    0,   63,    2, 0x06,    5 /* Public */,
       7,    2,   64,    2, 0x06,    6 /* Public */,

 // slots: name, argc, parameters, tag, flags, initial metatype offsets
      11,    0,   69,    2, 0x0a,    9 /* Public */,
      12,    1,   70,    2, 0x0a,   10 /* Public */,
      14,    0,   73,    2, 0x0a,   12 /* Public */,
      15,    0,   74,    2, 0x0a,   13 /* Public */,

 // signals: parameters
    QMetaType::Void, QMetaType::Int, QMetaType::Int, QMetaType::Int,    3,    4,    5,
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 8, QMetaType::LongLong,    9,   10,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString,   13,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

Q_CONSTINIT const QMetaObject Decoder::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_meta_stringdata_CLASSDecoderENDCLASS.offsetsAndSizes,
    qt_meta_data_CLASSDecoderENDCLASS,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_stringdata_CLASSDecoderENDCLASS_t,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<Decoder, std::true_type>,
        // method 'frameInfoUpdateSig'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'frameDataUpdateSig'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'audioDataUpdateSig'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const char *, std::false_type>,
        QtPrivate::TypeAndForceComplete<qint64, std::false_type>,
        // method 'startDecode'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'moveThread'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<QString, std::false_type>,
        // method 'quitThread'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'stopRecord'
        QtPrivate::TypeAndForceComplete<void, std::false_type>
    >,
    nullptr
} };

void Decoder::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<Decoder *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->frameInfoUpdateSig((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[3]))); break;
        case 1: _t->frameDataUpdateSig(); break;
        case 2: _t->audioDataUpdateSig((*reinterpret_cast< std::add_pointer_t<const char*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<qint64>>(_a[2]))); break;
        case 3: _t->startDecode(); break;
        case 4: _t->moveThread((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 5: _t->quitThread(); break;
        case 6: _t->stopRecord(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (Decoder::*)(int , int , int );
            if (_t _q_method = &Decoder::frameInfoUpdateSig; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (Decoder::*)();
            if (_t _q_method = &Decoder::frameDataUpdateSig; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (Decoder::*)(const char * , qint64 );
            if (_t _q_method = &Decoder::audioDataUpdateSig; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 2;
                return;
            }
        }
    }
}

const QMetaObject *Decoder::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *Decoder::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_CLASSDecoderENDCLASS.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int Decoder::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 7)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 7;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 7)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 7;
    }
    return _id;
}

// SIGNAL 0
void Decoder::frameInfoUpdateSig(int _t1, int _t2, int _t3)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t3))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void Decoder::frameDataUpdateSig()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}

// SIGNAL 2
void Decoder::audioDataUpdateSig(const char * _t1, qint64 _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}
QT_WARNING_POP
