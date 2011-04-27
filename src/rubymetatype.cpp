/*
 * Copyright 2003-2011 by Richard Dale <richard.j.dale@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) version 3, or any
 * later version accepted by the membership of KDE e.V. (or its
 * successor approved by the membership of KDE e.V.), which shall
 * act as a proxy defined in Section 6 of version 3 of the license.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "rubymetatype.h"
#include "marshall.h"

#include <QtCore/QHash>
#include <QtCore/QTextCodec>
#include <QtCore/QStringList>
#include <QtCore/qdebug.h>

Q_DECLARE_METATYPE(QObjectList)
Q_DECLARE_METATYPE(QList<int>)

namespace QtRuby {
    
class RubyTypeInfo
{
public:
    RubyTypeInfo() : marshall(0), demarshall(0)
    { }

    Marshall::MarshallFunction marshall;
    Marshall::DemarshallFunction demarshall;
};
    
typedef QHash<int, RubyTypeInfo *> TypeInfosMap;
Q_GLOBAL_STATIC(TypeInfosMap, typeInfos)
static QTextCodec *codec = 0;

#if RUBY_VERSION < 0x10900
static const char * KCODE = 0;

static void
init_codec() {
    VALUE temp = rb_gv_get("$KCODE");
    KCODE = StringValuePtr(temp);
    if (qstrcmp(KCODE, "EUC") == 0) {
        codec = QTextCodec::codecForName("eucJP");
    } else if (qstrcmp(KCODE, "SJIS") == 0) {
        codec = QTextCodec::codecForName("Shift-JIS");
    }
}

QString*
qstringFromRString(VALUE rstring) {
    if (KCODE == 0) {
        init_codec();
    }

    if (qstrcmp(KCODE, "UTF8") == 0)
        return new QString(QString::fromUtf8(StringValuePtr(rstring), RSTRING_LEN(rstring)));
    else if (qstrcmp(KCODE, "EUC") == 0)
        return new QString(codec->toUnicode(StringValuePtr(rstring)));
    else if (qstrcmp(KCODE, "SJIS") == 0)
        return new QString(codec->toUnicode(StringValuePtr(rstring)));
    else if(qstrcmp(KCODE, "NONE") == 0)
        return new QString(QString::fromLatin1(StringValuePtr(rstring)));

    return new QString(QString::fromLocal8Bit(StringValuePtr(rstring), RSTRING_LEN(rstring)));
}

VALUE
rstringFromQString(const QString * s) {
    if (KCODE == 0) {
        init_codec();
    }

    if (qstrcmp(KCODE, "UTF8") == 0)
        return rb_str_new2(s->toUtf8());
    else if (qstrcmp(KCODE, "EUC") == 0)
        return rb_str_new2(codec->fromUnicode(*s));
    else if (qstrcmp(KCODE, "SJIS") == 0)
        return rb_str_new2(codec->fromUnicode(*s));
    else if (qstrcmp(KCODE, "NONE") == 0)
        return rb_str_new2(s->toLatin1());
    else
        return rb_str_new2(s->toLocal8Bit());
}

#else

QString*
qstringFromRString(VALUE rstring) {
    VALUE encoding = rb_funcall(rstring, rb_intern("encoding"), 0);
    encoding = rb_funcall(encoding, rb_intern("to_s"), 0);
    const char * enc_s = RSTRING_PTR(encoding);

    if (qstrcmp(enc_s, "UTF8") == 0) {
        return new QString(QString::fromUtf8(StringValuePtr(rstring), RSTRING_LEN(rstring)));
    } else if (qstrcmp(enc_s, "EUC-JP") == 0) {
        codec = QTextCodec::codecForName("eucJP");
        return new QString(codec->toUnicode(StringValuePtr(rstring)));
    } else if (qstrcmp(enc_s, "Shift-JIS") == 0) {
        codec = QTextCodec::codecForName("Shift-JIS");
        return new QString(codec->toUnicode(StringValuePtr(rstring)));
    } else if(qstrcmp(enc_s, "ISO-8859-1") == 0 || qstrcmp(enc_s, "US-ASCII") == 0) {
        return new QString(QString::fromLatin1(StringValuePtr(rstring)));
    }

    return new QString(QString::fromLocal8Bit(StringValuePtr(rstring), RSTRING_LEN(rstring)));
}

VALUE
rstringFromQString(const QString * s) {
    return rb_str_new2(s->toUtf8());
}
#endif

QTRUBY_EXPORT void
registerCustomType(int type, Marshall::MarshallFunction mf, Marshall::DemarshallFunction df)
{
    RubyTypeInfo *info = typeInfos()->value(type);
    if (!info) {
        info = new RubyTypeInfo();
        typeInfos()->insert(type, info);
    }
    info->marshall = mf;
    info->demarshall = df;
}

QTRUBY_EXPORT VALUE
createRubyValue(int type, const void *ptr)
{
    Q_ASSERT(ptr != 0);
    RubyTypeInfo *info = typeInfos()->value(type);
    if (info && info->marshall) {
        return info->marshall(ptr);
    } else {
        // check if it's one of the types we know
        switch (QMetaType::Type(type)) {
        case QMetaType::Void:
            return Qnil;
        case QMetaType::Bool:
            return *reinterpret_cast<const bool*>(ptr) ? Qtrue : Qfalse;
        case QMetaType::Int:
            return INT2NUM(*reinterpret_cast<const int*>(ptr));
        case QMetaType::UInt:
            return UINT2NUM(*reinterpret_cast<const uint*>(ptr));
        case QMetaType::LongLong:
            return LL2NUM(*reinterpret_cast<const qlonglong*>(ptr));
        case QMetaType::ULongLong:
            return rb_ull2inum(*reinterpret_cast<const qulonglong*>(ptr));
        case QMetaType::Double:
            return rb_float_new(*reinterpret_cast<const double*>(ptr));
        case QMetaType::QString:
        {
            const QString * s = reinterpret_cast<const QString*>(ptr);
            if (s->isNull())
                return Qnil;
            return rstringFromQString(s);
        }
        case QMetaType::Float:
            return rb_float_new((double) *reinterpret_cast<const float*>(ptr));
        case QMetaType::Short:
            return INT2NUM(*reinterpret_cast<const short*>(ptr));
        case QMetaType::UShort:
            return UINT2NUM(*reinterpret_cast<const unsigned short*>(ptr));
        case QMetaType::Char:
            return CHR2FIX(*reinterpret_cast<const char*>(ptr));
        case QMetaType::UChar:
            return CHR2FIX(*reinterpret_cast<const unsigned char*>(ptr));
        case QMetaType::QStringList:
        {
            VALUE result = rb_ary_new();
            Q_FOREACH(QString s, *reinterpret_cast<const QStringList *>(ptr)) {
                rb_ary_push(result, rstringFromQString(&s));
            }
            return result;
        }
        case QMetaType::QVariantList:
//            result = arrayFromVariantList(exec, *reinterpret_cast<const QVariantList *>(ptr));
            break;
        case QMetaType::QVariantMap:
//            result = objectFromVariantMap(exec, *reinterpret_cast<const QVariantMap *>(ptr));
            break;
        case QMetaType::QDateTime:
        case QMetaType::QDate:
        case QMetaType::QRegExp:
        case QMetaType::QObjectStar:
        case QMetaType::QWidgetStar:
        case QMetaType::QVariant:
        {
            QByteArray typeName = QMetaType::typeName(type);
            if (typeName.endsWith('*'))
                typeName.chop(1);;
            Smoke::ModuleIndex classId = Smoke::findClass(typeName);
            if (classId == Smoke::NullModuleIndex) {
                return Qnil;
            } else {
                VALUE value = Global::getRubyValue(ptr);
                if (value == Qnil)
                    return Global::wrapInstance(classId, (void *) ptr);
                else
                    return value;
            }
        }
        default:
            if (type == qMetaTypeId<VALUE>()) {
                return *((VALUE*) ptr);
            }

#ifndef QT_NO_QOBJECT
            // lazy registration of some common list types
            else if (type == qMetaTypeId<QObjectList>()) {
                qRubySmokeRegisterSequenceMetaType<QObjectList>();
                return createRubyValue(type, ptr);
            }
#endif
            else if (type == qMetaTypeId<QList<int> >()) {
                qRubyRegisterSequenceMetaType<QList<int> >();
                return createRubyValue(type, ptr);
            }

            else {
                QByteArray typeName = QMetaType::typeName(type);
                if (typeName.endsWith('*') && !*reinterpret_cast<void* const *>(ptr))
                    return Qnil;
//                else
//                    result = eng->newVariant(QVariant(type, ptr));
            }
        }
    }

    return Qnil;
}

QTRUBY_EXPORT bool
convertFromRubyValue(VALUE value, int type, void *ptr)
{
    RubyTypeInfo *info = typeInfos()->value(type);
    if (info && info->demarshall) {
        info->demarshall(value, ptr);
        return true;
    }

    if (TYPE(value) == T_DATA) {
        QByteArray typeName = QMetaType::typeName(type);
        if (typeName.endsWith('*'))
            typeName.chop(1);
        Smoke::ModuleIndex classId = Smoke::findClass(typeName);
        if (classId == Smoke::NullModuleIndex)
            return false;

        Object::Instance * instance = Object::Instance::get(value);
        if (instance == 0 || instance->value == 0) {
            return false;
        } else {
            ptr = instance->classId.smoke->cast(    instance->value,
                                                    instance->classId,
                                                    classId );
            return true;
        }
    }
    
    // check if it's one of the types we know
    switch (QMetaType::Type(type)) {
    case QMetaType::Bool:
        *reinterpret_cast<bool*>(ptr) = (value == Qtrue);
        return true;
    case QMetaType::Int:
        *reinterpret_cast<int*>(ptr) = NUM2INT(value);
        return true;
    case QMetaType::UInt:
        *reinterpret_cast<uint*>(ptr) = NUM2UINT(value);
        return true;
    case QMetaType::LongLong:
        *reinterpret_cast<qlonglong*>(ptr) = qlonglong(NUM2LL(value));
        return true;
    case QMetaType::ULongLong:
        *reinterpret_cast<qulonglong*>(ptr) = qulonglong(rb_num2ull(value));
        return true;
    case QMetaType::Double:
        *reinterpret_cast<double*>(ptr) = NUM2DBL(value);
        return true;
    case QMetaType::QString:
    {
        if (value == Qnil)
            *reinterpret_cast<QString*>(ptr) = QString();
        else
            *reinterpret_cast<QString*>(ptr) = *qstringFromRString(value);
        return true;
    }
    case QMetaType::Float:
        *reinterpret_cast<float*>(ptr) = (float) NUM2DBL(value);
        return true;
    case QMetaType::Short:
        *reinterpret_cast<short*>(ptr) = short(NUM2INT(value));
        return true;
    case QMetaType::UShort:
        *reinterpret_cast<unsigned short*>(ptr) = ushort(NUM2UINT(value));
        return true;
    case QMetaType::Char:
        *reinterpret_cast<char*>(ptr) = char(NUM2CHR(value));
        return true;
    case QMetaType::UChar:
        *reinterpret_cast<unsigned char*>(ptr) = (unsigned char)(NUM2CHR(value));
        return true;
    case QMetaType::QStringList:
    {
        quint32 len = RARRAY_LEN(value);
        for (quint32 i = 0; i < len; ++i)
            (*reinterpret_cast<QStringList *>(ptr)).append(*qstringFromRString(rb_ary_entry(value, i)));
        return true;
    }
    case QMetaType::QVariantList:
//        if (isArray(value)) {
//            *reinterpret_cast<QVariantList *>(ptr) = variantListFromArray(exec, JSC::asArray(value));
//            return true;
//        }
        break;
    case QMetaType::QVariantMap:
//        if (isObject(value)) {
//            *reinterpret_cast<QVariantMap *>(ptr) = variantMapFromObject(exec, JSC::asObject(value));
//            return true;
//        }
        break;
    default:
    ;
    }

    QByteArray name = QMetaType::typeName(type);
    if (value == Qnil && name.endsWith('*')) {
        *reinterpret_cast<void* *>(ptr) = 0;
        return true;
    } else if (type == qMetaTypeId<VALUE>()) {
        *reinterpret_cast<VALUE*>(ptr) = value;
        return true;
    }

    // lazy registration of some common list types
#ifndef QT_NO_QOBJECT
    if (type == qMetaTypeId<QObjectList>()) {
        qRubySmokeRegisterSequenceMetaType<QObjectList>();
        return convertFromRubyValue(value, type, ptr);
    }
#endif
    else if (type == qMetaTypeId<QList<int> >()) {
        qRubyRegisterSequenceMetaType<QList<int> >();
        return convertFromRubyValue(value, type, ptr);
    }

#if 0
    if (!name.isEmpty()) {
        qWarning("QtRuby::convertFromRubyValue: unable to convert value to type `%s'",
                 name.constData());
    }
#endif
    return false;
}

}
