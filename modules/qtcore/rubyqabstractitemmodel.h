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

#ifndef QTRUBY_RUBYQABSTRACTITEMMODEL_H
#define QTRUBY_RUBYQABSTRACTITEMMODEL_H

#include <ruby.h>

#include "qtruby_export.h"

namespace QtRuby {
    extern VALUE qmodelindex_internalpointer(VALUE self);
    extern VALUE qabstractitemmodel_rowcount(int argc, VALUE * argv, VALUE self);
    extern VALUE qabstractitemmodel_insertrows(int argc, VALUE * argv, VALUE self);
    extern VALUE qabstractitemmodel_columncount(int argc, VALUE * argv, VALUE self);
    extern VALUE qabstractitemmodel_data(int argc, VALUE * argv, VALUE self);
    extern VALUE qabstractitemmodel_setdata(int argc, VALUE * argv, VALUE self);
    extern VALUE qabstractitemmodel_flags(VALUE self, VALUE model_index);
    extern VALUE qabstractitemmodel_insertcolumns(int argc, VALUE * argv, VALUE self);
    extern VALUE qabstractitemmodel_removerows(int argc, VALUE * argv, VALUE self);
    extern VALUE qabstractitemmodel_removecolumns(int argc, VALUE * argv, VALUE self);
    extern VALUE qabstractitemmodel_createindex(int argc, VALUE * argv, VALUE self);
}

#endif // QTRUBY_RUBYQABSTRACTITEMMODEL_H