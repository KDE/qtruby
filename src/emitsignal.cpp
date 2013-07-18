/*
 *   Copyright 2003-2013 by Richard Dale <richard.j.dale@gmail.com>

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

#include "funcall.h"
#include "emitsignal.h"
#include "smokebinding.h"
#include "global.h"
#include "utils.h"
#include "debug.h"

#include "smoke/qtcore_smoke.h"

#include <QtCore/QStringList>
#include <QtCore/qdebug.h>

namespace QtRuby {

EmitSignal::ReturnValue::ReturnValue(VALUE* returnValue, const QMetaMethod& metaMethod, void ** a) :
    m_returnValue(returnValue), m_metaMethod(metaMethod), _a(a)
{
}

void
EmitSignal::ReturnValue::unsupported()
{
}

void
EmitSignal::ReturnValue::next()
{
}

EmitSignal::EmitSignal(QObject * qobject, const QMetaMethod& metaMethod, int id, int argc, VALUE * argv, VALUE self, VALUE * result) :
    m_qobject(qobject),
    m_self(self),
    m_metaMethod(metaMethod),
    m_id(id),
    m_argc(argc),
    m_argv(argv),
    m_current(-1), m_called(false), m_error(false)
{
    Object::Instance * instance = Object::Instance::get(m_self);
    m_smoke = instance->classId.smoke;
    _a = new void*[argc + 1];
    _a[0] = 0;
    m_smokeTypes = new SmokeType[argc + 1];

    for (int index = 0; index < argc; index++) {
        m_smokeTypes[index] = findSmokeType(m_metaMethod.parameterTypes().at(index), m_smoke);
    }
    
    m_stack = new Smoke::StackItem[m_argc + 1];
}

EmitSignal::~EmitSignal()
{
    delete[] _a;
    delete[] m_smokeTypes;
    delete[] m_stack;
}

SmokeType EmitSignal::type()
{
    return m_smokeTypes[m_current];
}

Smoke::StackItem &EmitSignal::item()
{
    smokeStackItemToQt( type(),
                        m_metaMethod.parameterTypes().at(m_current),  
                        m_stack[m_current + 1],
                        &(_a[m_current + 1]) );
    return m_stack[m_current + 1];
}

void EmitSignal::unsupported()
{
    m_error = true;
}

void EmitSignal::callMethod()
{

}

void EmitSignal::next()
{
    int previous = m_current;
    m_current++;
    
    while(!m_called && m_current < m_metaMethod.parameterTypes().count()) {
        Marshall::HandlerFn fn = getMarshallFn(type());
        (*fn)(this);
        m_current++;
    }

    // qDebug() << Q_FUNC_INFO << "_a[0]" << _a[0] << "_a[1]" << _a[1] << "*(_a[1])" << *((int*)_a[1]);

    m_qobject->metaObject()->activate(m_qobject, m_id, _a);
    m_current = previous;
}

}

// kate: space-indent on; indent-width 4; replace-tabs on; mixed-indent off;
