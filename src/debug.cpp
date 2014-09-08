/*
 * Copyright 2003-2013 by Richard Dale <richard.j.dale@gmail.com>
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

#include <QDebug>

#include "debug.h"

namespace QtRuby {
    namespace Debug {
        uint DoDebug = 0;


QString
methodToString(const Smoke::ModuleIndex& methodId)
{
    QString result;
    Smoke * smoke = methodId.smoke;
    Smoke::Method& methodRef = smoke->methods[methodId.index];

    if ((methodRef.flags & Smoke::mf_signal) != 0) {
        result.append("signal ");
    }

    if ((methodRef.flags & Smoke::mf_slot) != 0) {
        result.append("slot ");
    }

    const char * typeName = smoke->types[methodRef.ret].name;

    if ((methodRef.flags & Smoke::mf_enum) != 0) {
        result.append(QString("enum %1::%2")
                            .arg(smoke->classes[methodRef.classId].className)
                            .arg(smoke->methodNames[methodRef.name]) );
        return result;
    }

    if ((methodRef.flags & Smoke::mf_virtual) != 0) {
        result.append("virtual ");
    }

    if ((methodRef.flags & Smoke::mf_static) != 0) {
        result.append("static ");
    }

    if ((methodRef.flags & Smoke::mf_ctor) == 0) {
        result.append((typeName != 0 ? typeName : "void"));
        result.append(" ");
    }

    result.append(  QString("%1::%2(")
                        .arg(smoke->classes[methodRef.classId].className)
                        .arg(smoke->methodNames[methodRef.name]) );

    for (int i = 0; i < methodRef.numArgs; i++) {
        if (i > 0) {
            result.append(", ");
        }

        typeName = smoke->types[smoke->argumentList[methodRef.args+i]].name;
        result.append((typeName != 0 ? typeName : "void"));
    }

    result.append(")");

    if ((methodRef.flags & Smoke::mf_const) != 0) {
        result.append(" const");
    }

    if ((methodRef.flags & Smoke::mf_purevirtual) != 0) {
        result.append(" = 0");
    }

    return result;
}

QByteArray to_s(VALUE value)
{
    if (value == Qnil) {
        return "nil";
    } else if (value == Qtrue) {
        return "true";
    } else if (value == Qfalse) {
        return "false";
    } else {
        VALUE str = rb_funcall(value, rb_intern("to_s"), 0, 0);
        return StringValuePtr(str);
    }
}

VALUE
qdebug(VALUE klass, VALUE msg)
{
    qDebug("%s", StringValuePtr(msg));
    return klass;
}

VALUE
qfatal(VALUE klass, VALUE msg)
{
    qFatal("%s", StringValuePtr(msg));
    return klass;
}

VALUE
qwarning(VALUE klass, VALUE msg)
{
    qWarning("%s", StringValuePtr(msg));
    return klass;
}


VALUE
setDebug(VALUE self, VALUE on_value)
{
    int on = NUM2INT(on_value);
    DoDebug = on;
    return self;
}

VALUE
debugging(VALUE /*self*/)
{
    return INT2NUM(DoDebug);
}

    }
}

// kate: space-indent on; indent-width 4; replace-tabs on; mixed-indent off;
