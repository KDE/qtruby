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

#include "object.h"

#include "utils.h"
#include "methodcall.h"
#include "debug.h"
#include "global.h"

#include <smoke/qtcore_smoke.h>

#include <QtCore/QtDebug>

#include <QtCore/QStringList>
#include <QtCore/QObject>
#include <QtCore/QMetaMethod>
#include <QtCore/QMetaObject>
#include <QtGui/QWidget>
#include <QtCore/QPair>

namespace QtRuby {

void Object::Instance::finalize()
{
    switch (ownership) {
    case QScriptEngine::QtOwnership:
        break;
    case QScriptEngine::ScriptOwnership:
        if (value != 0) {
            dispose();
        }
        break;
    case QScriptEngine::AutoOwnership:
//        if (value != 0 && value->parent() == 0) {
//            dispose();
//        }
        break;
    }
}

void Object::Instance::dispose()
{
    if ((Debug::DoDebug & Debug::GC) != 0) {
        qWarning("Deleting (%s*)%p", className(), value);
    }

    if (value == 0)
        return;

    QByteArray methodName(className());
    methodName.prepend("~");
    Smoke::ModuleIndex nameId = classId.smoke->findMethodName(className(), methodName);
    Smoke::ModuleIndex methodId = classId.smoke->findMethod(classId, nameId);

    if (methodId.index > 0) {
        Smoke::Method &methodRef = methodId.smoke->methods[methodId.smoke->methodMaps[methodId.index].method];
        Smoke::ClassFn fn = methodId.smoke->classes[methodRef.classId].classFn;
        Smoke::StackItem destroyInstanceStack[1];
        (*fn)(methodRef.method, value, destroyInstanceStack);
    }

    value = 0;
}

Object::Instance::~Instance()
{
    // TODO: Don't delete anything for now until custom free() methods have
    // been implemented
    // finalize();
}

Object::Instance * Object::Instance::get(VALUE value)
{
    if (value == Qnil || TYPE(value) != T_DATA)
        return 0;

    Object::Instance * instance = 0;
    Data_Get_Struct(value, Object::Instance, instance);
    return instance;
}

static bool application_terminated = false;

VALUE
set_application_terminated(VALUE /*self*/, VALUE yn)
{
    application_terminated = (yn == Qtrue);
    return Qnil;
}

void Object::mark(void * ptr)
{
    Object::Instance * instance = reinterpret_cast<Object::Instance *>(ptr);
    if (Debug::DoDebug & Debug::GC) {
        qWarning("Checking for mark (%s*)%p", instance->className(), instance->value);
    }
}

void Object::free(void * ptr)
{
    Object::Instance * instance = reinterpret_cast<Object::Instance *>(ptr);

    if (application_terminated)
        return;

    if (Debug::DoDebug & Debug::GC) {
        qWarning("Checking for delete (%s*)%p allocated: %s",
                 instance->className(),
                 instance->value,
                 instance->ownership == Object::ScriptOwnership ? "true" : "false");
    }

    Global::unmapPointer(instance, instance->classId, 0);

    if (Debug::DoDebug & Debug::GC) {
        qWarning("Deleting (%s*)%p", instance->className(), instance->value);
    }

    delete instance;
}

VALUE
dispose(VALUE self)
{
    Object::Instance * instance = Object::Instance::get(self);
    instance->dispose();

    return Qnil;
}

VALUE
is_disposed(VALUE self)
{
    Object::Instance * instance = Object::Instance::get(self);
    return (instance != 0 && !instance->isNull()) ? Qfalse : Qtrue;
}


VALUE
cast_object_to(VALUE /*self*/, VALUE obj, VALUE new_klass)
{
    Object::Instance * instance = Object::Instance::get(obj);
    VALUE new_klassname = rb_funcall(new_klass, rb_intern("name"), 0);
    Smoke::ModuleIndex classId = Global::idFromRubyClass(new_klass);
    if (classId == Smoke::NullModuleIndex) {
        rb_raise(rb_eArgError, "unable to find class \"%s\" to cast to\n", StringValuePtr(new_klassname));
    }

    Object::ValueOwnership ownership = instance->ownership;
    instance->ownership = Object::QtOwnership;
    void * ptr = instance->cast(classId);
    return Global::wrapInstance(classId, ptr, ownership, new_klass);
}

}
