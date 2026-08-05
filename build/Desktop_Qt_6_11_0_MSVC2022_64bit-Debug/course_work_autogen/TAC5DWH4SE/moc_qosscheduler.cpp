/****************************************************************************
** Meta object code from reading C++ file 'qosscheduler.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.11.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../core/qosscheduler.h"
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'qosscheduler.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 69
#error "This file was generated using the moc from 6.11.0. It"
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
struct qt_meta_tag_ZN12QosSchedulerE_t {};
} // unnamed namespace

template <> constexpr inline auto QosScheduler::qt_create_metaobjectdata<qt_meta_tag_ZN12QosSchedulerE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "QosScheduler",
        "hourlyAggregationNeeded",
        "",
        "priorityTrafficNeeded",
        "processName",
        "onHighLoss",
        "lossPercent",
        "onHourlyTimer"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'hourlyAggregationNeeded'
        QtMocHelpers::SignalData<void()>(1, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'priorityTrafficNeeded'
        QtMocHelpers::SignalData<void(const QString &)>(3, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 4 },
        }}),
        // Slot 'onHighLoss'
        QtMocHelpers::SlotData<void(double)>(5, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Double, 6 },
        }}),
        // Slot 'onHourlyTimer'
        QtMocHelpers::SlotData<void()>(7, 2, QMC::AccessPrivate, QMetaType::Void),
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<QosScheduler, qt_meta_tag_ZN12QosSchedulerE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject QosScheduler::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN12QosSchedulerE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN12QosSchedulerE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN12QosSchedulerE_t>.metaTypes,
    nullptr
} };

void QosScheduler::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<QosScheduler *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->hourlyAggregationNeeded(); break;
        case 1: _t->priorityTrafficNeeded((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 2: _t->onHighLoss((*reinterpret_cast<std::add_pointer_t<double>>(_a[1]))); break;
        case 3: _t->onHourlyTimer(); break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (QosScheduler::*)()>(_a, &QosScheduler::hourlyAggregationNeeded, 0))
            return;
        if (QtMocHelpers::indexOfMethod<void (QosScheduler::*)(const QString & )>(_a, &QosScheduler::priorityTrafficNeeded, 1))
            return;
    }
}

const QMetaObject *QosScheduler::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *QosScheduler::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN12QosSchedulerE_t>.strings))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int QosScheduler::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 4)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 4;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 4)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 4;
    }
    return _id;
}

// SIGNAL 0
void QosScheduler::hourlyAggregationNeeded()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}

// SIGNAL 1
void QosScheduler::priorityTrafficNeeded(const QString & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 1, nullptr, _t1);
}
QT_WARNING_POP
