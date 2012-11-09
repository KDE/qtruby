/*
 *   Copyright 2003-2011 by Richard Dale <richard.j.dale@gmail.com>

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

#include "virtualmethodcall.h"
#include "funcall.h"

namespace QtRuby {

VirtualMethodCall::ReturnValue::ReturnValue(Smoke::ModuleIndex methodId, Smoke::Stack stack, VALUE returnValue) :
    m_methodId(methodId), m_stack(stack), m_returnValue(returnValue)
{
    m_type.set(m_methodId.smoke, method().ret);
    Marshall::HandlerFn fn = getMarshallFn(type());
    (*fn)(this);
}

void
VirtualMethodCall::ReturnValue::unsupported()
{
    rb_raise(   rb_eArgError,
                "Cannot handle '%s' as return type of virtual method %s::%s",
                type().name(),
                m_methodId.smoke->className(method().classId),
                m_methodId.smoke->methodNames[method().name] );
}

void
VirtualMethodCall::ReturnValue::next()
{
}

VirtualMethodCall::VirtualMethodCall(Smoke::ModuleIndex methodId, Smoke::Stack stack, VALUE obj, VALUE * args) :
    m_methodId(methodId), m_stack(stack), m_obj(obj), m_args(args),
    m_current(-1), m_called(false),
    m_methodRef(methodId.smoke->methods[methodId.index])
{
}

VirtualMethodCall::~VirtualMethodCall()
{
}

void
VirtualMethodCall::unsupported()
{
    m_called = true;
    rb_raise(   rb_eArgError,
                "Cannot handle '%s' as argument of virtual method %s::%s",
                type().name(),
                m_methodId.smoke->className(m_methodRef.classId),
                m_methodId.smoke->methodNames[m_methodRef.name] );

}

void
VirtualMethodCall::callMethod()
{
    if (m_called) {
        return;
    }

    m_called = true;
    VALUE returnValue = Qnil;
    QTRUBY_INIT_STACK
    QTRUBY_FUNCALL2(returnValue, m_obj, rb_intern(m_methodId.smoke->methodNames[m_methodRef.name]), m_methodRef.numArgs, m_args)
    QTRUBY_RELEASE_STACK

    ReturnValue result(m_methodId, m_stack, returnValue);
}

void
VirtualMethodCall::next()
{
    int previous = m_current;
    m_current++;

    while (!m_called && m_current < m_methodRef.numArgs) {
        Marshall::HandlerFn fn = getMarshallFn(type());
        (*fn)(this);
        m_current++;
    }

    callMethod();
    m_current = previous;
}

}

// kate: space-indent on; indent-width 4; replace-tabs on; mixed-indent off;
