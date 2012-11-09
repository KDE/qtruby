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


#include "methodcall.h"
#include "smokebinding.h"
#include "global.h"
#include "debug.h"

#include "smoke/qtcore_smoke.h"

#include <QtCore/QStringList>
#include <QtCore/qdebug.h>

namespace QtRuby {

MethodCall::ArgumentTypeConversion::ArgumentTypeConversion(Smoke::ModuleIndex methodId, Smoke::StackItem& item, VALUE& value) :
    m_methodId(methodId), m_item(item), m_value(value)
{
    m_stack = new Smoke::StackItem[method().numArgs + 1];
    QByteArray methodName(m_methodId.smoke->methodNames[method().name]);

    if (methodName.startsWith("operator ")) {
        m_type.set(m_methodId.smoke, method().ret);
    } else {
        m_type.set(m_methodId.smoke, (m_methodId.smoke->argumentList + method().args)[0]);
    }

    Marshall::HandlerFn handlerFn = getMarshallFn(type());
    (*handlerFn)(this);

    // Note that the method called can be in one of two forms:
    //      1) A constructor 'Foobar::Foobar(value)',
    //      2) A 'value::operator Foobar()' method.
    // In case 1) the argument is in m_stack[1], and the result in m_stack[0]
    // In case 2) the instance to call the method on, is m_stack[1].s_voidp,
    // and the result is in m_stack[0].
    Smoke::ClassFn fn = m_methodId.smoke->classes[method().classId].classFn;
    (*fn)(method().method, m_stack[1].s_voidp, m_stack);
    m_item = m_stack[0];
}

MethodCall::ArgumentTypeConversion::~ArgumentTypeConversion()
{
    QByteArray methodName(m_methodId.smoke->methodNames[method().name]);
    QByteArray className;
    Smoke::ModuleIndex classId;

    if (methodName.startsWith("operator ")) {
        className = methodName.mid(qstrlen("operator "));
        classId = Smoke::findClass(className);
    } else {
        className = m_methodId.smoke->classes[method().classId].className;
        classId = Smoke::ModuleIndex(m_methodId.smoke, method().classId);
    }

    QByteArray destructorName(className);
    destructorName.prepend("~");
    Smoke::ModuleIndex nameId = classId.smoke->findMethodName(className, destructorName);
    Smoke::ModuleIndex methodId = classId.smoke->findMethod(classId, nameId);

    if (methodId != Smoke::NullModuleIndex) {
        Smoke::Method &methodRef = classId.smoke->methods[classId.smoke->methodMaps[methodId.index].method];
        Smoke::ClassFn fn = classId.smoke->classes[methodRef.classId].classFn;
        m_stack[1] = m_stack[0];
        (*fn)(methodRef.method, m_stack[1].s_voidp, m_stack);
    }

    delete[] m_stack;
}

void
MethodCall::ArgumentTypeConversion::unsupported()
{
    rb_raise(   rb_eArgError,
                "Cannot handle '%s' as value type conversion %s::%s",
                type().name(),
                m_methodId.smoke->className(method().classId),
                m_methodId.smoke->methodNames[method().name] );
}

void
MethodCall::ArgumentTypeConversion::next()
{
}

MethodCall::ReturnValue::ReturnValue(Smoke::ModuleIndex methodId, Smoke::Stack stack, VALUE * returnValue) :
    m_methodId(methodId), m_stack(stack), m_returnValue(returnValue)
{
    Marshall::HandlerFn fn = getMarshallFn(type());
    (*fn)(this);
}

void
MethodCall::ReturnValue::unsupported()
{
    QString message;
    if (qstrcmp(m_methodId.smoke->className(method().classId), "QGlobalSpace") == 0) {
        rb_raise(   rb_eArgError,
                    "Cannot handle '%s' as return type of %s",
                    type().name(),
                    m_methodId.smoke->methodNames[method().name] );
    } else {
        rb_raise(   rb_eArgError,
                    "Cannot handle '%s' as return type of %s::%s",
                    type().name(),
                    m_methodId.smoke->className(method().classId),
                    m_methodId.smoke->methodNames[method().name] );
    }
}

void
MethodCall::ReturnValue::next()
{
}

MethodCall::MethodCall( const QVector<Smoke::ModuleIndex>& methodIds,
                        VALUE target,
                        VALUE * args ) :
    m_methodIds(methodIds), m_methodRef(methodIds[0].smoke->methods[methodIds[0].index]),
    m_current(-1), m_target(target), m_instance(0), m_returnValue(Qnil),
    m_valueList(args), m_called(false), m_error(false)
{
    m_methodId = m_methodIds[0];
    m_smoke = m_methodId.smoke;

    if (m_target != Qnil)
        m_instance = QtRuby::Object::Instance::get(m_target);

    m_args = m_smoke->argumentList + m_methodRef.args;
    m_stack = new Smoke::StackItem[m_methodRef.numArgs + 1];
}

MethodCall::~MethodCall()
{
    delete[] m_stack;
}

void MethodCall::unsupported()
{
    m_error = true;

    if (qstrcmp(m_smoke->className(m_methodRef.classId), "QGlobalSpace") == 0) {
        rb_raise(   rb_eArgError,
                    "Cannot handle '%s' as argument to %s",
                    type().name(),
                    m_smoke->methodNames[m_methodRef.name] );
    } else {
        rb_raise(   rb_eArgError,
                    "Cannot handle '%s' as argument to %s::%s",
                    type().name(),
                    m_smoke->className(m_methodRef.classId),
                    m_smoke->methodNames[m_methodRef.name] );
    }
}

void MethodCall::callMethod()
{
    if (m_called || m_error) {
        return;
    }

    m_called = true;

    if (m_target == Qnil && !(m_methodRef.flags & Smoke::mf_static)) {
        rb_raise(rb_eArgError, "%s is not a class method\n", m_smoke->methodNames[m_methodRef.name]);
    }

    Smoke::ClassFn fn = m_smoke->classes[m_methodRef.classId].classFn;
    void *ptr = 0;

    if (m_instance != 0 && !m_instance->isNull()) {
        ptr = m_instance->cast(Smoke::ModuleIndex(m_smoke, m_methodRef.classId));
    }

    (*fn)(m_methodRef.method, ptr, m_stack);

    if ((m_methodRef.flags & Smoke::mf_ctor) != 0) {
        Smoke::StackItem initializeInstanceStack[2];
        initializeInstanceStack[1].s_voidp = Global::modules[m_smoke].binding;
        fn(0, m_stack[0].s_class, initializeInstanceStack);
    }

    ReturnValue result(m_methodId, m_stack, &m_returnValue);

    if ((Debug::DoDebug & Debug::Calls) != 0) {
        QStringList argsString;
        for (int i = 0; i < m_methodRef.numArgs; ++i) {
            argsString << QString::fromLatin1(Debug::to_s(m_valueList[i]));
        }

        qWarning(   "Trace@%s:%d %s.%s(%s) => %s",
                    rb_sourcefile(),
                    rb_sourceline(),
                    Global::rubyClassNameFromId(Smoke::ModuleIndex(m_smoke, m_methodRef.classId)).constData(),
                    (m_methodRef.flags & Smoke::mf_ctor) != 0 ? "new" : m_smoke->methodNames[m_methodRef.name],
                    argsString.join(", ").toLatin1().constData(),
                    Debug::to_s(m_returnValue).constData() );
    }
}

void MethodCall::next()
{
    int previous = m_current;
    m_current++;

    while (!m_called && !m_error && m_current < m_methodRef.numArgs) {
        if (hasTypeConversion()) {
            ArgumentTypeConversion conversion(typeConversion(), item(), *var());
            next();
        } else {
            Marshall::HandlerFn fn = getMarshallFn(type());
            (*fn)(this);
        }

        m_current++;
    }

    callMethod();
    m_current = previous;
}

}

// kate: space-indent on; indent-width 4; replace-tabs on; mixed-indent off;
