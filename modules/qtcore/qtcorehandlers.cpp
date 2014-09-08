/*
 *   Copyright 2003-2013 by Richard Dale <richard.j.dale@gmail.com>

 *   Based on the PerlQt marshalling code by Ashley Winters

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


#include <QtCore/QByteArray>
#include <QtCore/QDate>
#include <QtCore/QDateTime>
#include <QtCore/QFileInfo>
#include <QtCore/QLine>
#include <QtCore/QList>
#include <QtCore/QLocale>
#include <QtCore/QModelIndex>
#include <QtCore/QPoint>
#include <QtCore/QPointF>
#include <QtCore/QRect>
#include <QtCore/QRectF>
#include <QtCore/QSize>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QTime>
#include <QtCore/QUrl>
#include <QtCore/QVariant>
#include <QtCore/QVector>
#include <QtCore/QXmlStreamEntityDeclaration>
#include <QtCore/QXmlStreamNamespaceDeclaration>
#include <QtCore/QXmlStreamNotationDeclaration>
#include <QtCore/qdebug.h>

#include <global.h>
#include <marshall.h>
#include <rubymetatype.h>
#include <object.h>

Q_DECLARE_METATYPE2(QHash<int,QByteArray>)
Q_DECLARE_METATYPE2(QHash<QString,QVariant>)
Q_DECLARE_METATYPE2(QList<QPair<int,int> >)
Q_DECLARE_METATYPE2(QList<QPair<QByteArray,QByteArray> >)
Q_DECLARE_METATYPE2(QList<QPair<QString,QChar> >)
Q_DECLARE_METATYPE2(QList<QPair<QString,QString> >)
Q_DECLARE_METATYPE2(QList<QPair<QString,unsigned short> >)
Q_DECLARE_METATYPE2(QMap<int,QVariant>)
Q_DECLARE_METATYPE2(QMap<QString,QVariant>)
Q_DECLARE_METATYPE2(QVector<QPair<double,QVariant> >)
Q_DECLARE_METATYPE(QByteArray)
Q_DECLARE_METATYPE(QDate)
Q_DECLARE_METATYPE(QDateTime)
// Q_DECLARE_METATYPE(QFileInfo)
Q_DECLARE_METATYPE(QList<bool>)
Q_DECLARE_METATYPE(QList<int>)
Q_DECLARE_METATYPE(QList<unsigned int>)
Q_DECLARE_METATYPE(QList<QByteArray>)
Q_DECLARE_METATYPE(QList<QDate>)
Q_DECLARE_METATYPE(QList<QDateTime>)
Q_DECLARE_METATYPE(QList<QFileInfo>)
Q_DECLARE_METATYPE(QList<QLocale::Country>)
Q_DECLARE_METATYPE(QList<QModelIndex>)
Q_DECLARE_METATYPE(QList<QObject*>)
Q_DECLARE_METATYPE(QList<qreal>)
Q_DECLARE_METATYPE(QList<QRectF>)
Q_DECLARE_METATYPE(QList<QRegExp>)
Q_DECLARE_METATYPE(QList<QSize>)
Q_DECLARE_METATYPE(QList<QStringList>)
Q_DECLARE_METATYPE(QList<QTime>)
Q_DECLARE_METATYPE(QList<QUrl>)
Q_DECLARE_METATYPE(QList<QVariant>)
Q_DECLARE_METATYPE(QLocale::Country)
Q_DECLARE_METATYPE(QModelIndex)
Q_DECLARE_METATYPE(QRegExp)
Q_DECLARE_METATYPE(QTime)
Q_DECLARE_METATYPE(QVariant)
Q_DECLARE_METATYPE(QVector<QLine>)
Q_DECLARE_METATYPE(QVector<QLineF>)
Q_DECLARE_METATYPE(QVector<QPoint>)
Q_DECLARE_METATYPE(QVector<QPointF>)
Q_DECLARE_METATYPE(QVector<qreal>)
Q_DECLARE_METATYPE(QVector<QRect>)
Q_DECLARE_METATYPE(QVector<QRectF>)
Q_DECLARE_METATYPE(QVector<QVariant>)
Q_DECLARE_METATYPE(QVector<QXmlStreamEntityDeclaration>)
Q_DECLARE_METATYPE(QVector<QXmlStreamNamespaceDeclaration>)
Q_DECLARE_METATYPE(QVector<QXmlStreamNotationDeclaration>)
Q_DECLARE_METATYPE(QVector<unsigned int>)
Q_DECLARE_METATYPE(QXmlStreamEntityDeclaration)
Q_DECLARE_METATYPE(QXmlStreamNamespaceDeclaration)
Q_DECLARE_METATYPE(QXmlStreamNotationDeclaration)

namespace QtRuby {

static void marshall_QString(Marshall *m) {
    switch(m->action()) {
    case Marshall::FromVALUE:
    {
        QString * s = 0;

        if (*(m->var()) == Qnil) {
            s = new QString();
        } else {
            s = new QString(qrubyvalue_cast<QString>(*(m->var())));
        }

        m->item().s_voidp = s;
        m->next();

        if (!m->type().isConst() && *(m->var()) != Qnil && s != 0 && !s->isNull()) {
            rb_str_resize(*(m->var()), 0);
            VALUE temp = qRubyValueFromValue(*s);
            rb_str_cat2(*(m->var()), StringValuePtr(temp));
        }

        if (s != 0 && m->cleanup()) {
            delete s;
        }

        break;
    }

    case Marshall::ToVALUE:
    {
        if (m->item().s_voidp == 0) {
            *(m->var()) = Qnil;
            return;
        }

        QString * s = reinterpret_cast<QString*>(m->item().s_voidp);
        *(m->var()) = qRubyValueFromValue(*s);

        if (m->cleanup() || m->type().isStack() ) {
            delete s;
        }
        break;
    }

    default:
        m->unsupported();
        break;
    }
}

static void marshall_CString(Marshall *m) {
    switch(m->action()) {
    case Marshall::FromVALUE:
    {
        if (*(m->var()) == Qnil) {
            m->item().s_voidp = 0;
            return;
        }

        QByteArray temp(qrubyvalue_cast<QString>(*m->var()).toLatin1());
        m->item().s_voidp = qstrdup(temp.constData());
        break;
    }

    case Marshall::ToVALUE:
    {
        if (m->item().s_voidp == 0) {
            *(m->var()) = Qnil;
            return;
        }

        *(m->var()) = rb_str_new2(reinterpret_cast<char*>(m->item().s_voidp));
        break;
    }

    default:
        m->unsupported();
        break;
    }
}

static void marshall_CStringArray(Marshall *m) {
    switch(m->action()) {
    case Marshall::FromVALUE:
    {
        VALUE arglist = *(m->var());
        if (    arglist == Qnil
                || TYPE(arglist) != T_ARRAY
                || RARRAY_LEN(arglist) == 0 )
        {
            m->item().s_voidp = 0;
            break;
        }

        char **argv = new char *[RARRAY_LEN(arglist) + 1];
        long i;
        for(i = 0; i < RARRAY_LEN(arglist); i++) {
            VALUE item = rb_ary_entry(arglist, i);
            char *s = StringValuePtr(item);
            argv[i] = new char[strlen(s) + 1];
            strcpy(argv[i], s);
        }

        argv[i] = 0;
        m->item().s_voidp = argv;
        m->next();

        rb_ary_clear(arglist);
        for(i = 0; argv[i]; i++) {
            rb_ary_push(arglist, rb_str_new2(argv[i]));
        }

        break;
    }

    default:
        m->unsupported();
        break;
    }
}

static void marshall_QLongLong(Marshall *m) {
    switch(m->action()) {
    case Marshall::FromVALUE:
    {
        long long * ll = 0;

        if (*(m->var()) == Qnil) {
            ll = new long long();
        } else {
            ll = new long long(qrubyvalue_cast<long long>(*(m->var())));
        }

        m->item().s_voidp = ll;
        m->next();

        if (ll != 0 && m->cleanup()) {
            delete ll;
        }

        break;
    }

    case Marshall::ToVALUE:
    {
        if (m->item().s_voidp == 0) {
            *(m->var()) = Qnil;
            return;
        }

        *(m->var()) = qRubyValueFromValue(*(reinterpret_cast<long long*>(m->item().s_voidp)));
        break;
    }

    default:
        m->unsupported();
        break;
    }
}

static void marshall_QULongLong(Marshall *m) {
    switch(m->action()) {
    case Marshall::FromVALUE:
    {
        unsigned long long * ll = 0;

        if (*(m->var()) == Qnil) {
            ll = new unsigned long long();
        } else {
            ll = new unsigned long long(qrubyvalue_cast<unsigned long long>(*(m->var())));
        }

        m->item().s_voidp = ll;
        m->next();

        if (!m->type().isConst() && ll != 0) {
            // Copy the string back to the QScriptValue instance
        }

        if (ll != 0 && m->cleanup()) {
            delete ll;
        }

        break;
    }

    case Marshall::ToVALUE:
    {
        if (m->item().s_voidp == 0) {
            *(m->var()) = Qnil;
            return;
        }

        *(m->var()) = qRubyValueFromValue(*(reinterpret_cast<unsigned long long*>(m->item().s_voidp)));
        break;
    }

    default:
        m->unsupported();
        break;
    }
}

/*
    Primitive Ruby types are immutable, and so to marshall references to C primitives
    they need to be passed as single element Ruby Arrays to make them mutable.
 */
template <class T>
void marshall_PrimitiveRef(Marshall *m) {
    switch(m->action()) {
    case Marshall::FromVALUE:
    {
        VALUE array = *(m->var());
        T * value;

        // TODO: This is a memory leak, but methods like QApplication::QApplication(int&, char**)
        // expect the ref to the primitive to stay around for longer than the method call
        if (TYPE(array) == T_ARRAY) {
            value = new T (qrubyvalue_cast<T>(rb_ary_entry(array, 0)));
        } else {
            value = new T (qrubyvalue_cast<T>(array));
        }

        m->item().s_voidp = value;
        m->next();

        if (!m->type().isConst() && TYPE(array) == T_ARRAY) {
            rb_ary_pop(array);
            rb_ary_push(array, qRubyValueFromValue(*(static_cast<T*>(m->item().s_voidp))));
        }
        break;
    }

    case Marshall::ToVALUE:
    {
        VALUE value;

        if (m->type().isConst()) {
            value = qRubyValueFromValue(*(static_cast<T*>(m->item().s_voidp)));
        } else {
            value = rb_ary_new();
            rb_ary_push(value, qRubyValueFromValue(*(static_cast<T*>(m->item().s_voidp))));
        }

        *(m->var()) = value;
        m->next();

        if (!m->type().isConst()) {
            *(static_cast<T*>(m->item().s_voidp)) = qrubyvalue_cast<T>(rb_ary_entry(value, 0));
        }
        break;
    }

    default:
        m->unsupported();
        break;
    }
}
static void marshall_VoidPtrArray(Marshall *m) {
    switch(m->action()) {
    case Marshall::FromVALUE:
    {
        VALUE value = *(m->var());
        if (value != Qnil) {
            Data_Get_Struct(value, void*, m->item().s_voidp);
        } else {
            m->item().s_voidp = 0;
        }
        break;
    }

    case Marshall::ToVALUE:
    {
        VALUE value = Data_Wrap_Struct(rb_cObject, 0, 0, m->item().s_voidp);
        *(m->var()) = value;
        break;
    }

    default:
        m->unsupported();
        break;
    }
}

Marshall::TypeHandler QtCoreHandlers[] = {
    { "bool*", marshall_PrimitiveRef<bool> },
    { "bool&", marshall_PrimitiveRef<bool> },
    { "char*", marshall_CString },
    { "char**", marshall_CStringArray },
    { "char&", marshall_PrimitiveRef<char> },
    { "double*", marshall_PrimitiveRef<double> },
    { "double&", marshall_PrimitiveRef<double> },
    { "int*", marshall_PrimitiveRef<int> },
    { "int&", marshall_PrimitiveRef<int> },
    { "long*", marshall_PrimitiveRef<long> },
    { "long&", marshall_PrimitiveRef<long> },
    { "QHash<int,QByteArray>", marshall_Container<QHash<int,QByteArray> > },
    { "QHash<int,QByteArray>&", marshall_Container<QHash<int,QByteArray> > },
    { "QHash<QString,QVariant>", marshall_Container<QHash<QString,QVariant> > },
    { "QHash<QString,QVariant>&", marshall_Container<QHash<QString,QVariant> > },
    { "QList<bool>", marshall_Container<QList<bool> > },
    { "QList<bool>&", marshall_Container<QList<bool> > },
    { "QList<double>", marshall_Container<QList<double> > },
    { "QList<double>&", marshall_Container<QList<double> > },
    { "QList<int>", marshall_Container<QList<int> > },
    { "QList<int>*", marshall_Container<QList<int> > },
    { "QList<int>&", marshall_Container<QList<int> > },
    { "QList<QByteArray>", marshall_Container<QList<QByteArray> > },
    { "QList<QByteArray>&", marshall_Container<QList<QByteArray> > },
    { "QList<QDate>", marshall_Container<QList<QDate> > },
    { "QList<QDate>&", marshall_Container<QList<QDate> > },
    { "QList<QDateTime>", marshall_Container<QList<QDateTime> > },
    { "QList<QDateTime>&", marshall_Container<QList<QDateTime> > },
    { "QList<QFileInfo>&", marshall_Container<QList<QFileInfo> > },
    { "QList<QLocale::Country>", marshall_Container<QList<QLocale::Country> > },
    { "QList<QModelIndex>&", marshall_Container<QList<QModelIndex> > },
    { "QList<QModelIndex>&", marshall_Container<QList<QModelIndex> > },
    { "QList<QObject*>", marshall_Container<QList<QObject*> > },
    { "QList<QObject*>&", marshall_Container<QList<QObject*> > },
    { "QList<QPair<int,int>>", marshall_Container<QList<QPair<int,int> > > },
    { "QList<QPair<QByteArray,QByteArray>>", marshall_Container<QList<QPair<QByteArray,QByteArray> > > },
    { "QList<QPair<QByteArray,QByteArray>>&", marshall_Container<QList<QPair<QByteArray,QByteArray> > > },
    { "QList<QPair<QString,QChar>>", marshall_Container<QList<QPair<QString,QChar> > > },
    { "QList<QPair<QString,QString>>", marshall_Container<QList<QPair<QString,QString> > > },
    { "QList<QPair<QString,QString>>&", marshall_Container<QList<QPair<QString,QString> > > },
    { "QList<QPair<QString,unsigned short>>", marshall_Container<QList<QPair<QString,unsigned short> > > },
    { "QList<qreal>", marshall_Container<QList<qreal> > },
    { "QList<qreal>&", marshall_Container<QList<qreal> > },
    { "QList<QRectF>&", marshall_Container<QList<QRectF> > },
    { "QList<QRegExp>&", marshall_Container<QList<QRegExp> > },
    { "QList<QSize>&", marshall_Container<QList<QSize> > },
    { "QList<QStringList>", marshall_Container<QList<QStringList> > },
    { "QList<QStringList>&", marshall_Container<QList<QStringList> > },
    { "QList<QTime>", marshall_Container<QList<QTime> > },
    { "QList<QTime>&", marshall_Container<QList<QTime> > },
    { "QList<QUrl>&", marshall_Container<QList<QUrl> > },
    { "QList<QVariant>", marshall_Container<QList<QVariant> > },
    { "QList<QVariant>&", marshall_Container<QList<QVariant> > },
    { "QList<unsigned int>", marshall_Container<QList<unsigned int> > },
    { "QList<unsigned int>&", marshall_Container<QList<unsigned int> > },
    { "qlonglong", marshall_QLongLong },
    { "qlonglong&", marshall_QLongLong },
    { "QMap<int,QVariant>", marshall_Container<QMap<int,QVariant> > },
    { "QMap<int,QVariant>&", marshall_Container<QMap<int,QVariant> > },
    { "QMap<QString,QVariant>", marshall_Container<QMap<QString,QVariant> > },
    { "QMap<QString,QVariant>&", marshall_Container<QMap<QString,QVariant> > },
    { "QStringList", marshall_Container<QStringList> },
    { "QStringList&", marshall_Container<QStringList> },
    { "QString", marshall_QString },
    { "QString*", marshall_QString },
    { "QString&", marshall_QString },
    { "qulonglong", marshall_QULongLong },
    { "qulonglong&", marshall_QULongLong },
    { "QVector<double>", marshall_Container<QVector<qreal> > },
    { "QVector<double>&", marshall_Container<QVector<qreal> > },
    { "QVector<QLineF>&", marshall_Container<QVector<QLineF> > },
    { "QVector<QLine>&", marshall_Container<QVector<QLine> > },
    { "QVector<QPair<double,QVariant>>", marshall_Container<QVector<QPair<double,QVariant> > > },
    { "QVector<QPair<double,QVariant>>&", marshall_Container<QVector<QPair<double,QVariant> > > },
    { "QVector<QPointF>&", marshall_Container<QVector<QPointF> > },
    { "QVector<QPoint>", marshall_Container<QVector<QPoint> > },
    { "QVector<QPoint>&", marshall_Container<QVector<QPoint> > },
    { "QVector<qreal>", marshall_Container<QVector<qreal> > },
    { "QVector<qreal>&", marshall_Container<QVector<qreal> > },
    { "QVector<QRectF>&", marshall_Container<QVector<QRectF> > },
    { "QVector<QRect>", marshall_Container<QVector<QRect> > },
    { "QVector<QRect>&", marshall_Container<QVector<QRect> > },
    { "QVector<QVariant>&", marshall_Container<QVector<QVariant> > },
    { "QVector<QXmlStreamEntityDeclaration>", marshall_Container<QVector<QXmlStreamEntityDeclaration> > },
    { "QVector<QXmlStreamEntityDeclaration>&", marshall_Container<QVector<QXmlStreamEntityDeclaration> > },
    { "QVector<QXmlStreamNamespaceDeclaration>", marshall_Container<QVector<QXmlStreamNamespaceDeclaration> > },
    { "QVector<QXmlStreamNamespaceDeclaration>&", marshall_Container<QVector<QXmlStreamNamespaceDeclaration> > },
    { "QVector<QXmlStreamNotationDeclaration>", marshall_Container<QVector<QXmlStreamNotationDeclaration> > },
    { "QVector<QXmlStreamNotationDeclaration>&", marshall_Container<QVector<QXmlStreamNotationDeclaration> > },
    { "QVector<unsigned int>", marshall_Container<QVector<unsigned int> > },
    { "QVector<unsigned int>&", marshall_Container<QVector<unsigned int> > },
    { "short*", marshall_PrimitiveRef<short> },
    { "short&", marshall_PrimitiveRef<short> },
    { "signed int&", marshall_PrimitiveRef<int> },
    { "signed long&", marshall_PrimitiveRef<long> },
    { "signed short&", marshall_PrimitiveRef<short> },
    { "unsigned char*", marshall_CString },
    { "unsigned char&", marshall_PrimitiveRef<unsigned char> },
    { "unsigned int&", marshall_PrimitiveRef<unsigned int> },
    { "unsigned long&", marshall_PrimitiveRef<unsigned long> },
    { "void**", marshall_VoidPtrArray },


    { 0, 0 }
};

void registerQtCoreTypes()
{
    qRubyRegisterSequenceMetaType<QList<QStringList> >();
    qRubyRegisterSequenceMetaType<QList<bool> >();
    qRubyRegisterSequenceMetaType<QList<double> >();
    qRubyRegisterSequenceMetaType<QList<int> >();
    qRubyRegisterSequenceMetaType<QList<unsigned int> >();
    qRubyRegisterSequenceMetaType<QList<QLocale::Country> >();
    qRubyRegisterSequenceMetaType<QList<qreal> >();
    qRubyRegisterSequenceMetaType<QVector<qreal> >();
    qRubyRegisterSequenceMetaType<QVector<unsigned int> >();
    qRubySmokeRegisterHashMetaType<QHash<int,QByteArray> >();
    qRubySmokeRegisterHashMetaType<QHash<QString,QVariant> >();
    qRubySmokeRegisterHashMetaType<QMap<int,QVariant> >();
    qRubySmokeRegisterHashMetaType<QMap<QString,QVariant> >();
    qRubySmokeRegisterPairSequenceMetaType<QList<QPair<int,int> > >();
    qRubySmokeRegisterPairSequenceMetaType<QList<QPair<QByteArray,QByteArray> > >();
    qRubySmokeRegisterPairSequenceMetaType<QList<QPair<QString,unsigned short> > >();
    qRubySmokeRegisterPairSequenceMetaType<QList<QPair<QString,QChar> > >();
    qRubySmokeRegisterPairSequenceMetaType<QList<QPair<QString,QString> > >();
    qRubySmokeRegisterPairSequenceMetaType<QVector<QPair<double,QVariant> > >();
    qRubySmokeRegisterPointerSequenceMetaType<QList<QObject*> >();
    qRubySmokeRegisterSequenceMetaType<QList<QByteArray> >();
    qRubySmokeRegisterSequenceMetaType<QList<QDate> >();
    qRubySmokeRegisterSequenceMetaType<QList<QDateTime> >();
    qRubySmokeRegisterSequenceMetaType<QList<QFileInfo> >();
    qRubySmokeRegisterSequenceMetaType<QList<QModelIndex> >();
    qRubySmokeRegisterSequenceMetaType<QList<QRectF> >();
    qRubySmokeRegisterSequenceMetaType<QList<QRegExp> >();
    qRubySmokeRegisterSequenceMetaType<QList<QSize> >();
    qRubySmokeRegisterSequenceMetaType<QList<QTime> >();
    qRubySmokeRegisterSequenceMetaType<QList<QUrl> >();
    qRubySmokeRegisterSequenceMetaType<QList<QVariant> >();
    qRubySmokeRegisterSequenceMetaType<QVector<QLine> >();
    qRubySmokeRegisterSequenceMetaType<QVector<QLineF> >();
    qRubySmokeRegisterSequenceMetaType<QVector<QPoint> >();
    qRubySmokeRegisterSequenceMetaType<QVector<QPointF> >();
    qRubySmokeRegisterSequenceMetaType<QVector<QRect> >();
    qRubySmokeRegisterSequenceMetaType<QVector<QRectF> >();
    qRubySmokeRegisterSequenceMetaType<QVector<QXmlStreamEntityDeclaration> >();
    qRubySmokeRegisterSequenceMetaType<QVector<QXmlStreamNamespaceDeclaration> >();
    qRubySmokeRegisterSequenceMetaType<QVector<QXmlStreamNotationDeclaration> >();

    return;
}

}

// kate: space-indent on; indent-width 4; replace-tabs on; mixed-indent off;
