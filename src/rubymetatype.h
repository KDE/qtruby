/*
 *   Copyright 2003-2013-2010 by Richard Dale <richard.j.dale@gmail.com>

 *   Adapted from the code in src/script/qrubyengine.h in Qt 4.5

 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef QTRUBY_RUBY_METATYPE_H
#define QTRUBY_RUBY_METATYPE_H

#include "marshall.h"
#include "object.h"
#include "utils.h"
#include "global.h"

/*
    Allows a metatype to be declared for a type containing a single comma.
    For example:
        Q_DECLARE_METATYPE2(QList<QPair<QByteArray,QByteArray> >)
 */
#define Q_DECLARE_METATYPE2(TYPE1, TYPE2)                               \
    QT_BEGIN_NAMESPACE                                                  \
    template <>                                                         \
    struct QMetaTypeId< TYPE1,TYPE2 >                                   \
    {                                                                   \
        enum { Defined = 1 };                                           \
        static int qt_metatype_id()                                     \
            {                                                           \
                static QBasicAtomicInt metatype_id = Q_BASIC_ATOMIC_INITIALIZER(0); \
                if (!metatype_id)                                       \
                    metatype_id = qRegisterMetaType< TYPE1,TYPE2 >( #TYPE1 "," #TYPE2, \
                               reinterpret_cast< TYPE1,TYPE2  *>(quintptr(-1))); \
                return metatype_id;                                     \
            }                                                           \
    };                                                                  \
    QT_END_NAMESPACE

namespace QtRuby {
    extern void registerCustomType(int type, Marshall::MarshallFunction mf, Marshall::DemarshallFunction df);
    extern VALUE createRubyValue(int type, const void *ptr);
    extern bool convertFromRubyValue(VALUE value, int type, void *ptr);
}

/*
    These functions are based on the ones in qscriptengine.h,
    but adapted to use Ruby values and Smoke instances
*/

inline bool qrubyvalue_cast_helper(VALUE value, int type, void *ptr)
{
    return QtRuby::convertFromRubyValue(value, type, ptr);
}

template<typename T>
T qrubyvalue_cast(VALUE value
#if !defined qdoc && defined Q_CC_MSVC && _MSC_VER < 1300
, T * = 0
#endif
    )
{
    T t;
    const int id = qMetaTypeId<T>();

    if (qrubyvalue_cast_helper(value, id, &t))
        return t;

    return T();
}

inline VALUE qRubyValueFromValue_helper(int type, const void *ptr)
{
    return QtRuby::createRubyValue(type, ptr);
}

template <typename T>
inline VALUE qRubyValueFromValue(const T &t)
{
    return qRubyValueFromValue_helper(qMetaTypeId<T>(), &t);
}

template <typename T>
inline T qRubyValueToValue(VALUE value)
{
    return qrubyvalue_cast<T>(value);
}

inline void qRubyRegisterMetaType_helper(int type,
                                           QtRuby::Marshall::MarshallFunction mf,
                                           QtRuby::Marshall::DemarshallFunction df)
{
    QtRuby::registerCustomType(type, mf, df);
}

template<typename T>
int qRubyRegisterMetaType(
    VALUE (*toScriptValue)(const T &t),
    void (*fromScriptValue)(VALUE, T &t)
#ifndef qdoc
    , T * /* dummy */ = 0
#endif
)
{
    const int id = qRegisterMetaType<T>(); // make sure it's registered

    qRubyRegisterMetaType_helper(
        id, reinterpret_cast<QtRuby::Marshall::MarshallFunction>(toScriptValue),
        reinterpret_cast<QtRuby::Marshall::DemarshallFunction>(fromScriptValue));

    return id;
}

template <class Container>
VALUE qRubyValueFromSequence(const Container &cont)
{
    VALUE a = rb_ary_new();
    typename Container::const_iterator begin = cont.begin();
    typename Container::const_iterator end = cont.end();
    typename Container::const_iterator it;
    quint32 i;
    for (it = begin, i = 0; it != end; ++it, ++i)
        rb_ary_push(a, qRubyValueFromValue(*it));
    return a;
}

template <class Container>
void qRubyValueToSequence(VALUE value, Container &cont)
{
    quint32 len = RARRAY_LEN(value);
    for (quint32 i = 0; i < len; ++i) {
        VALUE item = rb_ary_entry(value, i);
#if defined Q_CC_MSVC && !defined Q_CC_MSVC_NET
        cont.push_back(qrubyvalue_cast<Container::value_type>(item));
#else
        cont.push_back(qrubyvalue_cast<typename Container::value_type>(item));
#endif
    }
}

template<typename T>
int qRubyRegisterSequenceMetaType(
#ifndef qdoc
    T * /* dummy */ = 0
#endif
)
{
    return qRubyRegisterMetaType<T>(qRubyValueFromSequence,
                                      qRubyValueToSequence);
}

namespace QtRuby {

template <class Container>
void marshall_Container(Marshall *m) {
    switch(m->action()) {
    case Marshall::FromVALUE:
    {
        m->item().s_voidp = new Container(qrubyvalue_cast<Container>(*(m->var())));
        m->next();

        if (!m->type().isConst()) {
            *(m->var()) = qRubyValueFromValue(*(static_cast<Container*>(m->item().s_voidp)));
        }

        if (m->cleanup()) {
            delete static_cast<Container*>(m->item().s_voidp);
        }
        break;
    }

    case Marshall::ToVALUE:
    {
        *(m->var()) = qRubyValueFromValue(*(static_cast<Container*>(m->item().s_voidp)));
        m->next();

        if (!m->type().isConst()) {
            *(static_cast<Container*>(m->item().s_voidp)) = qrubyvalue_cast<Container>(*(m->var()));
        }

        if (m->cleanup()) {
            delete static_cast<Container*>(m->item().s_voidp);
        }
        break;
    }

    default:
        m->unsupported();
        break;
    }
}

}

inline VALUE qRubySmokeValueFromSequence_helper(Smoke::ModuleIndex classId, void * ptr)
{
    VALUE value = QtRuby::Global::getRubyValue(ptr);
    if (value != Qnil) {
        return value;
    }

    return QtRuby::Global::wrapInstance(classId, ptr);
}

template <class Container>
VALUE qRubySmokeValueFromSequence(const Container &cont)
{
    VALUE a = rb_ary_new();
    const char * typeName = QMetaType::typeName(qMetaTypeId<typename Container::value_type>());
    Smoke::ModuleIndex classId = Smoke::findClass(typeName);
    typename Container::const_iterator begin = cont.begin();
    typename Container::const_iterator end = cont.end();
    typename Container::const_iterator it;
    quint32 i;
    for (it = begin, i = 0; it != end; ++it, ++i) {
        rb_ary_push(a, qRubySmokeValueFromSequence_helper(classId, (void *) &(*it)));
    }

    return a;
}

inline void * qRubySmokeValueToSequence_helper(VALUE item, Smoke::ModuleIndex classId)
{
    QtRuby::Object::Instance * instance = QtRuby::Object::Instance::get(item);
    return instance->cast(classId);
}

template <class Container>
void qRubySmokeValueToSequence(VALUE value, Container &cont)
{
    quint32 len = RARRAY_LEN(value);
    const char * typeName = QMetaType::typeName(qMetaTypeId<typename Container::value_type>());
    Smoke::ModuleIndex classId = Smoke::findClass(typeName);
    for (quint32 i = 0; i < len; ++i) {

#if defined Q_CC_MSVC && !defined Q_CC_MSVC_NET
        cont.push_back(*(static_cast<Container::value_type *>(qRubySmokeValueToSequence_helper(rb_ary_entry(value, i), classId))));
#else
        cont.push_back(*(static_cast<typename Container::value_type *>(qRubySmokeValueToSequence_helper(rb_ary_entry(value, i), classId))));
#endif
    }
}

template<typename T>
int qRubySmokeRegisterSequenceMetaType(
#ifndef qdoc
    T * /* dummy */ = 0
#endif
)
{
    return qRubyRegisterMetaType<T>(qRubySmokeValueFromSequence,
                                      qRubySmokeValueToSequence);
}

template <class Container>
VALUE qRubySmokeValueFromPointerSequence(const Container &cont)
{
    VALUE a = rb_ary_new();
    QByteArray typeName(QMetaType::typeName(qMetaTypeId<typename Container::value_type>()));
    // Remove the star from the type name
    typeName.chop(1);
    Smoke::ModuleIndex classId = Smoke::findClass(typeName);
    typename Container::const_iterator begin = cont.begin();
    typename Container::const_iterator end = cont.end();
    typename Container::const_iterator it;
    quint32 i;
    for (it = begin, i = 0; it != end; ++it, ++i) {
        rb_ary_push(a, qRubySmokeValueFromSequence_helper(classId, (void *) *it));
    }

    return a;
}

template <class Container>
void qRubySmokeValueToPointerSequence(VALUE value, Container &cont)
{
    quint32 len = RARRAY_LEN(value);
    QByteArray typeName(QMetaType::typeName(qMetaTypeId<typename Container::value_type>()));
    // Remove the star from the type name
    typeName.chop(1);
    Smoke::ModuleIndex classId = Smoke::findClass(typeName);
    for (quint32 i = 0; i < len; ++i) {

#if defined Q_CC_MSVC && !defined Q_CC_MSVC_NET
        cont.push_back(static_cast<Container::value_type>(qRubySmokeValueToSequence_helper(rb_ary_entry(value, i), classId)));
#else
        cont.push_back(static_cast<typename Container::value_type>(qRubySmokeValueToSequence_helper(rb_ary_entry(value, i), classId)));
#endif
    }
}

template<typename T>
int qRubySmokeRegisterPointerSequenceMetaType(
#ifndef qdoc
    T * /* dummy */ = 0
#endif
)
{
    return qRubyRegisterMetaType<T>(qRubySmokeValueFromPointerSequence,
                                      qRubySmokeValueToPointerSequence);
}


template <class Container>
VALUE qRubySmokeValueFromPairSequence(const Container &cont)
{
    VALUE a = rb_ary_new();
    const char * firstTypeName = QMetaType::typeName(qMetaTypeId<typename Container::value_type::first_type>());
    const char * secondTypeName = QMetaType::typeName(qMetaTypeId<typename Container::value_type::second_type>());
    Smoke::ModuleIndex firstClassId = Smoke::findClass(firstTypeName);
    Smoke::ModuleIndex secondClassId = Smoke::findClass(secondTypeName);
    typename Container::const_iterator begin = cont.begin();
    typename Container::const_iterator end = cont.end();
    typename Container::const_iterator it;
    quint32 i;
    for (it = begin, i = 0; it != end; ++it, ++i) {
        VALUE pair = rb_ary_new();
        if (firstClassId == Smoke::NullModuleIndex) {
            rb_ary_push(pair, qRubyValueFromValue((*it).first));
        } else {
            rb_ary_push(pair, qRubySmokeValueFromSequence_helper(firstClassId, (void *) &((*it).first)));
        }

        if (secondClassId == Smoke::NullModuleIndex) {
            rb_ary_push(pair, qRubyValueFromValue((*it).second));
        } else {
            rb_ary_push(pair, qRubySmokeValueFromSequence_helper(firstClassId, (void *) &((*it).second)));
        }

        rb_ary_push(a, pair);
    }

    return a;
}

template <class Container>
void qRubySmokeValueToPairSequence(VALUE value, Container &container)
{
    quint32 len = RARRAY_LEN(value);
    const char * firstTypeName = QMetaType::typeName(qMetaTypeId<typename Container::value_type::first_type>());
    const char * secondTypeName = QMetaType::typeName(qMetaTypeId<typename Container::value_type::second_type>());
    Smoke::ModuleIndex firstClassId = Smoke::findClass(firstTypeName);
    Smoke::ModuleIndex secondClassId = Smoke::findClass(secondTypeName);
    for (quint32 i = 0; i < len; ++i) {
        VALUE pair = rb_ary_entry(value, i);

        if (firstClassId == Smoke::NullModuleIndex) {
            if (secondClassId == Smoke::NullModuleIndex) {
                container.push_back(QPair<typename Container::value_type::first_type, typename Container::value_type::second_type>(
                    qrubyvalue_cast<typename Container::value_type::first_type>(rb_ary_entry(pair, 0)),
                    qrubyvalue_cast<typename Container::value_type::second_type>(rb_ary_entry(pair, 1)) ) );
            } else {
                container.push_back(QPair<typename Container::value_type::first_type, typename Container::value_type::second_type>(
                    qrubyvalue_cast<typename Container::value_type::first_type>(rb_ary_entry(pair, 0)),
                    *(static_cast<typename Container::value_type::second_type *>(qRubySmokeValueToSequence_helper(rb_ary_entry(pair, 1), secondClassId))) ) );
            }
        } else {
            if (secondClassId == Smoke::NullModuleIndex) {
                container.push_back(QPair<typename Container::value_type::first_type, typename Container::value_type::second_type>(
                    *(static_cast<typename Container::value_type::first_type *>(qRubySmokeValueToSequence_helper(rb_ary_entry(pair, 0), firstClassId))),
                    qrubyvalue_cast<typename Container::value_type::second_type>(rb_ary_entry(pair, 1)) ) );
            } else {
                container.push_back(QPair<typename Container::value_type::first_type, typename Container::value_type::second_type>(
                    *(static_cast<typename Container::value_type::first_type *>(qRubySmokeValueToSequence_helper(rb_ary_entry(pair, 0), firstClassId))),
                    *(static_cast<typename Container::value_type::second_type *>(qRubySmokeValueToSequence_helper(rb_ary_entry(pair, 1), secondClassId))) ) );
            }
        }
    }
}

template<typename T>
int qRubySmokeRegisterPairSequenceMetaType(
#ifndef qdoc
    T * /* dummy */ = 0
#endif
)
{
    return qRubyRegisterMetaType<T>(qRubySmokeValueFromPairSequence,
                                      qRubySmokeValueToPairSequence);
}

template <class Container>
VALUE qRubySmokeValueFromHash(const Container &container)
{
    VALUE value = rb_hash_new();
    const char * keyTypeName = QMetaType::typeName(qMetaTypeId<typename Container::key_type>());
    const char * mappedTypeName = QMetaType::typeName(qMetaTypeId<typename Container::mapped_type>());
    Smoke::ModuleIndex keyClassId = Smoke::findClass(keyTypeName);
    Smoke::ModuleIndex mappedClassId = Smoke::findClass(mappedTypeName);
    typename Container::const_iterator begin = container.begin();
    typename Container::const_iterator end = container.end();
    typename Container::const_iterator it;
    for (it = begin; it != end; ++it) {
        VALUE key;

        if (keyClassId == Smoke::NullModuleIndex) {
            key = qRubyValueFromValue(it.key());
        } else {
            key = qRubySmokeValueFromSequence_helper(keyClassId, (void *) &(it.key()));
        }

        if (mappedClassId == Smoke::NullModuleIndex) {
            rb_hash_aset(value, key, qRubyValueFromValue(it.value()));
        } else {
            rb_hash_aset(value, key, qRubySmokeValueFromSequence_helper(mappedClassId, (void *) &(it.value())));
        }
    }

    return value;
}

template <class Container>
void qRubySmokeValueToHash(VALUE value, Container &container)
{
    const char * mappedTypeName = QMetaType::typeName(qMetaTypeId<typename Container::mapped_type>());
    Smoke::ModuleIndex mappedClassId = Smoke::findClass(mappedTypeName);
    // Convert the Hash to an Array of key/value pairs
    VALUE a = rb_funcall(value, rb_intern("to_a"), 0);
    quint32 len = RARRAY_LEN(a);
    for (quint32 i = 0; i < len; ++i) {
        VALUE pair = rb_ary_entry(a, i);
        if (mappedClassId == Smoke::NullModuleIndex) {
            container[qrubyvalue_cast<typename Container::key_type>(rb_ary_entry(pair, 0))] =
                qrubyvalue_cast<typename Container::mapped_type>(rb_ary_entry(pair, 1));
        } else {
            container[qrubyvalue_cast<typename Container::key_type>(rb_ary_entry(pair, 0))] =
                *(static_cast<typename Container::mapped_type *>(qRubySmokeValueToSequence_helper(rb_ary_entry(pair, 1), mappedClassId)));
        }
    }
}

template<typename T>
int qRubySmokeRegisterHashMetaType(
#ifndef qdoc
    T * /* dummy */ = 0
#endif
)
{
    return qRubyRegisterMetaType<T>(qRubySmokeValueFromHash,
                                      qRubySmokeValueToHash);
}

#endif // QTRUBY_RUBY_METATYPE_H
