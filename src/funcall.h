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

#ifndef QTRUBY_FUNCCALL_H
#define QTRUBY_FUNCCALL_H

#include <ruby.h>

#include "qtruby_export.h"

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


#else  /* normal non-embedded extension */

#  define QTRUBY_INIT_STACK
#  define QTRUBY_RELEASE_STACK
#endif  /* RUBY_EMBEDDED */


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
    extern unsigned int nested_callback_count;
    extern bool qtruby_embedded;
    extern void show_exception_message();
    extern VALUE funcall2_protect_id;
    extern int funcall2_protect_argc;
    extern VALUE * funcall2_protect_args;
    extern VALUE funcall2_protect(VALUE obj);
    extern VALUE set_qtruby_embedded_wrapped(VALUE /*self*/, VALUE yn);
}

#endif // QTRUBY_METHODMISSING_H

// kate: space-indent on; indent-width 4; replace-tabs on; mixed-indent off;
