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

#include "funcall.h"

#include <QtCore/QString>

extern "C" {

QTRUBY_EXPORT void
set_qtruby_embedded(bool yn)
{
#if !defined(RUBY_INIT_STACK)
    if (yn) {
        qWarning("ERROR: set_qtruby_embedded(true) called but RUBY_INIT_STACK is undefined");
        qWarning("       Upgrade to Ruby 1.8.6 or greater");
    }
#endif
    QtRuby::qtruby_embedded = yn;
}

}

namespace QtRuby {

bool qtruby_embedded = false;

//
// This function was borrowed from the kross code. It puts out
// an error message and stacktrace on stderr for the current exception.
//
void
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

unsigned int nested_callback_count;
VALUE funcall2_protect_id = Qnil;
int funcall2_protect_argc = 0;
VALUE * funcall2_protect_args = 0;

VALUE
funcall2_protect(VALUE obj)
{
    VALUE result = Qnil;
    result = rb_funcall2(obj, funcall2_protect_id, funcall2_protect_argc, funcall2_protect_args);
    return result;
}

VALUE
set_qtruby_embedded_wrapped(VALUE /*self*/, VALUE yn)
{
  set_qtruby_embedded( yn == Qtrue );
  return Qnil;
}

}

// kate: space-indent on; indent-width 4; replace-tabs on; mixed-indent off;
