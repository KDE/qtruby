/*
 * Copyright 2003-2011 Ian Monroe <imonroe@kde.org>
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

#include "smokebinding.h"

#include <QtCore/QtDebug>

#include "debug.h"
#include "global.h"
#include "virtualmethodcall.h"
#include "utils.h"

namespace QtRuby {

Binding::Binding(Smoke* s)
    : SmokeBinding(s) { }

char* Binding::className(Smoke::Index classId)
{
    qDebug() << "QtRuby::Binding::className " << smoke->className(classId);
    // Convert '::' to '.' here
    return (char *) smoke->className(classId);
}

//!method called when a virtual method of a smoke-owned object is called. eg QWidget::mousePressEvent
bool Binding::callMethod(Smoke::Index method, void* ptr, Smoke::Stack args, bool /*isAbstract*/)
{
    QByteArray methodName(smoke->methodNames[smoke->methods[method].name]);

    VALUE obj = Global::getRubyValue(ptr);
    if (obj == Qnil) {
        if ((Debug::DoDebug & Debug::Virtual) != 0) {
            Smoke::ModuleIndex methodId(smoke, method);
            qWarning(   "module: %s Cannot find object for virtual method %p->%s::%s -> 0x%8.8x",
                        smoke->moduleName(),
                        ptr,
                        smoke->classes[smoke->methods[method].classId].className,
                        Debug::methodToString(methodId).toLatin1().constData(),
                        (uint) obj );
        }

        return false;
    }


    if ((Debug::DoDebug & Debug::Virtual) != 0) {
        Smoke::ModuleIndex methodId(smoke, method);
        qWarning(   "module: %s %p->%s called",
                    smoke->moduleName(),
                    ptr,
                    Debug::methodToString(methodId).toLatin1().constData() );
    }

    Object::Instance * instance = Object::Instance::get(obj);
    if (instance == 0) {
        if ((Debug::DoDebug & Debug::Virtual) != 0) {
            Smoke::ModuleIndex methodId(smoke, method);
            qWarning(   "module: %s Cannot find instance for virtual method %p->%s -> 0x%8.8x",
                        smoke->moduleName(),
                        ptr,
                        Debug::methodToString(methodId).toLatin1().constData(),
                        (uint) obj );
        }

        return false;
    }

    // If the virtual method hasn't been overriden, just call the C++ one.
    // During GC, avoid checking for override and just call the C++ version
    if (rb_during_gc() || rb_respond_to(obj, rb_intern(methodName)) == 0) {
        return false;
    }

    if ((Debug::DoDebug & Debug::Virtual) != 0) {
        qWarning("Method '%s' overriden", methodName.constData());
    }

    VirtualMethodCall methodCall(Smoke::ModuleIndex(smoke, method), args, obj, ALLOCA_N(VALUE, smoke->methods[method].numArgs));
    methodCall.next();
    return true;
}

void Binding::deleted(Smoke::Index classId, void* ptr)
{
    VALUE obj = Global::getRubyValue(ptr);
    Object::Instance * instance = Object::Instance::get(obj);

    if ((Debug::DoDebug & Debug::GC) != 0) {
        qWarning("%p->~%s()", ptr, smoke->className(classId));
    }

    if (instance == 0 || instance->isNull()) {
        return;
    }

    Global::unmapPointer(instance, instance->classId);
    instance->value = 0;

    return;
}

}
