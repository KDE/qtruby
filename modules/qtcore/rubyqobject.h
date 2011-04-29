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

#ifndef QTRUBY_RUBYQOBJECT_H
#define QTRUBY_RUBYQOBJECT_H

#include <ruby.h>

#include "qtruby_export.h"

namespace QtRuby {
    extern VALUE qobject_qt_metacast(VALUE self, VALUE klass);
    extern VALUE inherits_qobject(int argc, VALUE * argv, VALUE /*self*/);
    extern VALUE find_qobject_children(int argc, VALUE *argv, VALUE self);
    extern VALUE find_qobject_child(int argc, VALUE *argv, VALUE self);
}

#endif // QTRUBY_RUBYQOBJECT_H