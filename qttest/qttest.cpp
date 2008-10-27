/*
    qttest.rb - Ruby bindings for the QtTest library
    Copyright (C) 2008 Rafał Rzepecki

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include <ruby.h>
#include <QtTest/qtestkeyboard.h>
#include <qtruby.h>

static VALUE qwidget_key_click(int argc, VALUE * argv, VALUE self)
{
    if (argc < 1 || argc > 4) rb_raise(rb_eArgError, "Invalid argument list");
    smokeruby_object *o = value_obj_info(self);
    QWidget * widget = static_cast<QWidget *>(o->ptr);

    QTest::keyClick(widget, NUM2CHR(argv[0]));
    return Qnil;
}

extern TypeHandler qttest_handlers[];

extern "C" {

Q_DECL_EXPORT void
Init_qttest()
{
    rb_require("Qt4");

    VALUE widget = rb_define_class_under(qt_module, "Widget", qt_base_class);
    rb_define_method(widget, "key_click", (VALUE (*) (...)) qwidget_key_click, -1);
    rb_define_method(widget, "keyClick", (VALUE (*) (...)) qwidget_key_click, -1);
}

}
