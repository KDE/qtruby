/*
 *   Copyright 2003-2011 by Richard Dale <richard.j.dale@gmail.com>

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

#include <QtCore/QString>
#include <QtCore/QVariant>

// #include <QtGui/QPixmap>

#include <object.h>
#include <global.h>
#include <methodcall.h>

#include "rubyqvariant.h"

namespace QtRuby {

VALUE
qvariant_value(VALUE /*self*/, VALUE variant_value_klass, VALUE variant_value)
{
    Object::Instance * instance = Object::Instance::get(variant_value);
    QVariant * variant = reinterpret_cast<QVariant*>(instance->value);
    void * value_ptr = 0;

    if (variant->userType() == QMetaType::type("rObject")) {
        return *reinterpret_cast<VALUE*>(variant->data());
#ifdef QT_QTDBUS
    } else if (variant->userType() == qMetaTypeId<QDBusObjectPath>()) {
        QString s = qVariantValue<QDBusObjectPath>(*variant).path();
        return rb_str_new2(s.toLatin1());
    } else if (variant->userType() == qMetaTypeId<QDBusSignature>()) {
        QString s = qVariantValue<QDBusSignature>(*variant).signature();
        return rb_str_new2(s.toLatin1());
    } else if (variant->userType() == qMetaTypeId<QDBusVariant>()) {
        QVariant *ptr = new QVariant(qVariantValue<QDBusVariant>(*variant).variant());
        return Global::wrapInstance(Global::QVariantClassId, ptr, Object::ScriptOwnership);
#endif
    } else if (variant->type() >= QVariant::UserType) {
        // If the QVariant contains a user type, don't bother to look at the Ruby class argument
        value_ptr = QMetaType::construct(QMetaType::type(variant->typeName()), (void *) variant->constData());
        Smoke::ModuleIndex mi = Smoke::findClass(variant->typeName());
        return Global::wrapInstance(mi, value_ptr, Object::ScriptOwnership);
    }

    const char * classname = rb_class2name(variant_value_klass);
    Smoke::ModuleIndex value_class_id = Global::idFromRubyClass(variant_value_klass);
    if (value_class_id == Smoke::NullModuleIndex) {
        return Qnil;
    }

/*
    if (qstrcmp(classname, "Qt::Pixmap") == 0) {
        QPixmap v = qVariantValue<QPixmap>(*variant);
        value_ptr = (void *) new QPixmap(v);
    } else if (qstrcmp(classname, "Qt::Font") == 0) {
        QFont v = qVariantValue<QFont>(*variant);
        value_ptr = (void *) new QFont(v);
    } else if (qstrcmp(classname, "Qt::Brush") == 0) {
        QBrush v = qVariantValue<QBrush>(*variant);
        value_ptr = (void *) new QBrush(v);
    } else if (qstrcmp(classname, "Qt::Color") == 0) {
        QColor v = qVariantValue<QColor>(*variant);
        value_ptr = (void *) new QColor(v);
    } else if (qstrcmp(classname, "Qt::Palette") == 0) {
        QPalette v = qVariantValue<QPalette>(*variant);
        value_ptr = (void *) new QPalette(v);
    } else if (qstrcmp(classname, "Qt::Icon") == 0) {
        QIcon v = qVariantValue<QIcon>(*variant);
        value_ptr = (void *) new QIcon(v);
    } else if (qstrcmp(classname, "Qt::Image") == 0) {
        QImage v = qVariantValue<QImage>(*variant);
        value_ptr = (void *) new QImage(v);
    } else if (qstrcmp(classname, "Qt::Polygon") == 0) {
        QPolygon v = qVariantValue<QPolygon>(*variant);
        value_ptr = (void *) new QPolygon(v);
    } else if (qstrcmp(classname, "Qt::Region") == 0) {
        QRegion v = qVariantValue<QRegion>(*variant);
        value_ptr = (void *) new QRegion(v);
    } else if (qstrcmp(classname, "Qt::Bitmap") == 0) {
        QBitmap v = qVariantValue<QBitmap>(*variant);
        value_ptr = (void *) new QBitmap(v);
    } else if (qstrcmp(classname, "Qt::Cursor") == 0) {
        QCursor v = qVariantValue<QCursor>(*variant);
        value_ptr = (void *) new QCursor(v);
    } else if (qstrcmp(classname, "Qt::SizePolicy") == 0) {
        QSizePolicy v = qVariantValue<QSizePolicy>(*variant);
        value_ptr = (void *) new QSizePolicy(v);
    } else if (qstrcmp(classname, "Qt::KeySequence") == 0) {
        QKeySequence v = qVariantValue<QKeySequence>(*variant);
        value_ptr = (void *) new QKeySequence(v);
    } else if (qstrcmp(classname, "Qt::Pen") == 0) {
        QPen v = qVariantValue<QPen>(*variant);
        value_ptr = (void *) new QPen(v);
    } else if (qstrcmp(classname, "Qt::TextLength") == 0) {
        QTextLength v = qVariantValue<QTextLength>(*variant);
        value_ptr = (void *) new QTextLength(v);
    } else if (qstrcmp(classname, "Qt::TextFormat") == 0) {
        QTextFormat v = qVariantValue<QTextFormat>(*variant);
        value_ptr = (void *) new QTextFormat(v);
    } else if (qstrcmp(classname, "Qt::Variant") == 0) {
        value_ptr = (void *) new QVariant(*((QVariant *) variant->constData()));
    } else {
        // Assume the value of the Qt::Variant can be obtained
        // with a call such as Qt::Variant.toPoint()
        QByteArray toValueMethodName(classname);
        if (toValueMethodName.startsWith("Qt::")) {
            toValueMethodName.remove(0, strlen("Qt::"));
        }
        toValueMethodName.prepend("to");
        return rb_funcall(variant_value, rb_intern(toValueMethodName), 1, variant_value);
    }
*/
    return Global::wrapInstance(value_class_id, value_ptr, Object::ScriptOwnership);
}

static VALUE create_qvariant_one_arg(VALUE arg)
{
    return rb_funcall(Global::QVariantClass, rb_intern("new"), 1, arg);
}

VALUE
qvariant_from_value(int argc, VALUE * argv, VALUE self)
{
    if (argc == 2) {
        Smoke::ModuleIndex nameId = Smoke::NullModuleIndex;
        const char *typeName = StringValuePtr(argv[1]);

        if (TYPE(argv[0]) == T_DATA) {
            nameId = qtcore_Smoke->idMethodName("QVariant#");
        } else if (TYPE(argv[0]) == T_ARRAY || qstrcmp(typeName, "long long") == 0 || qstrcmp(typeName, "unsigned long long") == 0) {
            nameId = qtcore_Smoke->idMethodName("QVariant?");
        } else {
            nameId = qtcore_Smoke->idMethodName("QVariant$");
        }

        Smoke::ModuleIndex meth = qtcore_Smoke->findMethod(qtcore_Smoke->idClass("QVariant"), nameId);
        Smoke::Index i = meth.smoke->methodMaps[meth.index].method;
        i = -i;     // turn into ambiguousMethodList index
        while (meth.smoke->ambiguousMethodList[i] != 0) {
            if (    qstrcmp(    meth.smoke->types[meth.smoke->argumentList[meth.smoke->methods[meth.smoke->ambiguousMethodList[i]].args]].name,
                                typeName ) == 0 )
            {
                QVector<Smoke::ModuleIndex> methodIds;
                methodIds << Smoke::ModuleIndex(meth.smoke, meth.smoke->ambiguousMethodList[i]);
                methodIds << Smoke::NullModuleIndex;
                QtRuby::MethodCall c(methodIds, self, argv);
                c.next();
                return *(c.var());
            }

            i++;
        }

        qDebug("No suitable method for signature QVariant::QVariant(%s) found - looking for another suitable constructor\n", StringValuePtr(argv[1]));
    }

    QVariant * v = 0;

    const char * classname = rb_obj_classname(argv[0]);
    Object::Instance * instance = Object::Instance::get(argv[0]);
    int type = 0;

    if (qstrcmp(classname, "Qt::Enum") == 0) {
        return rb_funcall(Global::QVariantClass, rb_intern("new"), 1, rb_funcall(argv[0], rb_intern("to_i"), 0));
    } else if (instance != 0 && !instance->isNull() && (type = QVariant::nameToType(instance->className()))) {
        v = new QVariant(type, instance->value);
    } else {
        int error = 0;
        VALUE result = rb_protect(&create_qvariant_one_arg, argv[0], &error);
        if (!error) {
            return result;
        } else {
            VALUE lasterr = rb_gv_get("$!");
            VALUE klass = rb_class_path(CLASS_OF(lasterr));
            if (qstrcmp(StringValuePtr(klass), "ArgumentError") == 0) {
                // ArgumentError - no suitable constructor found
                // Create a QVariant that contains an rObject
                v = new QVariant(QMetaType::type("rObject"), &argv[0]);
            } else {
                rb_raise(lasterr, "while creating the QVariant");
            }
        }
    }

    return Global::wrapInstance(Global::QVariantClassId, v, Object::ScriptOwnership);
}

VALUE
new_qvariant(int argc, VALUE * argv, VALUE self)
{
static Smoke::Index new_qvariant_qlist = 0;
static Smoke::Index new_qvariant_qmap = 0;

    if (new_qvariant_qlist == 0) {
        Smoke::ModuleIndex nameId = qtcore_Smoke->findMethodName("Qvariant", "QVariant?");
        Smoke::ModuleIndex meth = qtcore_Smoke->findMethod(qtcore_Smoke->findClass("QVariant"), nameId);
        Smoke::Index i = meth.smoke->methodMaps[meth.index].method;
        i = -i;     // turn into ambiguousMethodList index
        while (qtcore_Smoke->ambiguousMethodList[i] != 0) {
            const char * argType = meth.smoke->types[meth.smoke->argumentList[meth.smoke->methods[meth.smoke->ambiguousMethodList[i]].args]].name;

            if (qstrcmp(argType, "const QList<QVariant>&" ) == 0) {
                new_qvariant_qlist = meth.smoke->ambiguousMethodList[i];
            } else if (qstrcmp(argType, "const QMap<QString,QVariant>&" ) == 0) {
                new_qvariant_qmap = meth.smoke->ambiguousMethodList[i];
            }

            i++;
        }
    }

    if (argc == 1 && TYPE(argv[0]) == T_HASH) {
        QVector<Smoke::ModuleIndex> methodIds;
        methodIds << Smoke::ModuleIndex(qtcore_Smoke, new_qvariant_qmap);
        methodIds << Smoke::NullModuleIndex;
        QtRuby::MethodCall c(methodIds, self, argv);
        c.next();
        return *(c.var());
    } else if ( argc == 1
                && TYPE(argv[0]) == T_ARRAY
                && RARRAY_LEN(argv[0]) > 0
                && TYPE(rb_ary_entry(argv[0], 0)) != T_STRING )
    {
        QVector<Smoke::ModuleIndex> methodIds;
        methodIds << Smoke::ModuleIndex(qtcore_Smoke, new_qvariant_qlist);
        methodIds << Smoke::NullModuleIndex;
        QtRuby::MethodCall c(methodIds, self, argv);
        c.next();
        return *(c.var());
    }

    return rb_call_super(argc, argv);
}

}