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
#include "invokeslot.h"
#include "smokebinding.h"
#include "global.h"
#include "utils.h"
#include "debug.h"

#include "smoke/qtcore_smoke.h"

#include <QtCore/QStringList>
#include <QtCore/qdebug.h>


// This is based on the SWIG SWIG_INIT_STACK and SWIG_RELEASE_STACK macros.
// If RUBY_INIT_STACK is only called when an embedded extension such as, a
// Ruby Plasma plugin is loaded, then later the C++ stack can drop below where the
// Ruby runtime thinks the stack should start (ie the stack position when the
// plugin was loaded), and result in sys stackerror exceptions
//
// TODO: While constructing the main class of a plugin when it is being loaded,
// there could be a problem when a custom virtual method is called or a slot is
// invoked, because RUBY_INIT_STACK will have aleady have been called from within
// the krubypluginfactory code, and it shouldn't be called again.

#if defined(RUBY_INIT_STACK)
#  define QTRUBY_INIT_STACK                            \
      if ( qtruby_embedded && nested_callback_count == 0 ) { RUBY_INIT_STACK } \
      nested_callback_count++;
#  define QTRUBY_RELEASE_STACK nested_callback_count--;

static unsigned int nested_callback_count = 0;

#else  /* normal non-embedded extension */

#  define QTRUBY_INIT_STACK
#  define QTRUBY_RELEASE_STACK
#endif  /* RUBY_EMBEDDED */

//
// This function was borrowed from the kross code. It puts out
// an error message and stacktrace on stderr for the current exception.
//
static void
show_exception_message()
{
    VALUE info = rb_gv_get("$!");
    VALUE bt = rb_funcall(info, rb_intern("backtrace"), 0);
    VALUE message = RARRAY_PTR(bt)[0];
    VALUE message2 = rb_obj_as_string(info);

    QString errormessage = QString("%1: %2 (%3)")
                            .arg( StringValueCStr(message) )
                            .arg( StringValueCStr(message2) )
                            .arg( rb_class2name(CLASS_OF(info)) );
    fprintf(stderr, "%s\n", errormessage.toLatin1().data());

    QString tracemessage;
    for(int i = 1; i < RARRAY_LEN(bt); ++i) {
        if( TYPE(RARRAY_PTR(bt)[i]) == T_STRING ) {
            QString s = QString("%1\n").arg( StringValueCStr(RARRAY_PTR(bt)[i]) );
            Q_ASSERT( ! s.isNull() );
            tracemessage += s;
            fprintf(stderr, "\t%s", s.toLatin1().data());
        }
    }
}

static VALUE funcall2_protect_id = Qnil;
static int funcall2_protect_argc = 0;
static VALUE * funcall2_protect_args = 0;

static VALUE
funcall2_protect(VALUE obj)
{
    VALUE result = Qnil;
    result = rb_funcall2(obj, funcall2_protect_id, funcall2_protect_argc, funcall2_protect_args);
    return result;
}

#  define QTRUBY_FUNCALL2(result, obj, id, argc, args) \
      if (qtruby_embedded) { \
          int state = 0; \
          funcall2_protect_id = id; \
          funcall2_protect_argc = argc; \
          funcall2_protect_args = args; \
          result = rb_protect(funcall2_protect, obj, &state); \
          if (state != 0) { \
              show_exception_message(); \
              result = Qnil; \
          } \
      } else { \
          result = rb_funcall2(obj, id, argc, args); \
      }

      
namespace QtRuby {

InvokeSlot::ReturnValue::ReturnValue(VALUE* returnValue, const QMetaMethod& metaMethod, void ** a) :
    m_returnValue(returnValue), m_metaMethod(metaMethod), _a(a)
{
}

void
InvokeSlot::ReturnValue::unsupported()
{
}

void
InvokeSlot::ReturnValue::next()
{
}

InvokeSlot::InvokeSlot(VALUE self, ID methodID, VALUE *valueList, const QMetaMethod& metaMethod, void ** a) :
    m_self(self), m_methodID(methodID), m_argv(valueList), m_metaMethod(metaMethod), _a(a),
    m_current(-1), m_called(false), m_error(false)
{
    Object::Instance * instance = Object::Instance::get(m_self);
    m_smoke = instance->classId.smoke;
    m_argc = m_metaMethod.parameterTypes().count();
    m_smokeTypes = new SmokeType[m_argc + 1];
    QList<QByteArray> types = m_metaMethod.parameterTypes();

    for (int index = 0; index < m_argc; index++) {
        const char* typeName = types[index].constData();
        m_smokeTypes[index] = findSmokeType(typeName, m_smoke);
    }
    
    m_stack = new Smoke::StackItem[m_argc + 1];
}

InvokeSlot::~InvokeSlot()
{
    delete[] m_smokeTypes;
    delete[] m_stack;
}

SmokeType InvokeSlot::type()
{
    return m_smokeTypes[m_current];
}

Smoke::StackItem &InvokeSlot::item()
{
    smokeStackItemFromQt(type(), m_stack[m_current + 1], _a[m_current + 1]);
    return m_stack[m_current + 1];
}

void InvokeSlot::unsupported()
{
    m_error = true;
}

void InvokeSlot::callMethod()
{
    qDebug() << Q_FUNC_INFO;
    if (m_called) {
        return;
    }
    m_called = true;

    VALUE result;
    QTRUBY_INIT_STACK
    QTRUBY_FUNCALL2(result, m_self, m_methodID, m_argc, m_argv)
    QTRUBY_RELEASE_STACK

    if (qstrcmp(m_metaMethod.typeName(), "") != 0) {
        ReturnValue r(&result, m_metaMethod, _a);
    }
}

void InvokeSlot::next()
{
    int previous = m_current;
    m_current++;
    
    while(!m_called && m_current < m_metaMethod.parameterTypes().count()) {
        Marshall::HandlerFn fn = getMarshallFn(type());
        (*fn)(this);
        m_current++;
    }

    callMethod();
    m_current = previous;
}

}

// kate: space-indent on; indent-width 4; replace-tabs on; mixed-indent off;
