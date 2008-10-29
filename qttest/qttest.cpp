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

#include <marshall_types.h>

template <typename T>
static T value2enum(VALUE v)
{
    long res;
    if (v == Qnil) {
        res = 0;
    } else if (TYPE(v) == T_OBJECT) {
        // Both Qt::Enum and Qt::Integer have a value() method, so 'get_qinteger()' can be called ok
        VALUE temp = rb_funcall(qt_internal_module, rb_intern("get_qinteger"), 1, v);
        res = (long) NUM2LONG(temp);
    } else {
        res = (long) NUM2LONG(v);
    }

    return static_cast<T>(res);
}

static VALUE qwidget_key_click(int argc, VALUE * argv, VALUE self)
{
    smokeruby_object *o = value_obj_info(self);
    QWidget * widget = qobject_cast<QWidget *>(static_cast<QObject *>(o->ptr));
    if (!widget)
        rb_call_super(argc, argv);

    if (argc < 1 || argc > 3) rb_raise(rb_eArgError, "Invalid argument list");
    
    char chKey;
    Qt::Key enKey;
    bool isChar = false;
    switch(TYPE(argv[0])) {
	case T_STRING:
	case T_FIXNUM:
	    chKey = NUM2CHR(argv[0]);
	    isChar = true;
	    break;
	default:
	    enKey = value2enum<Qt::Key>(argv[0]);
    }

    Qt::KeyboardModifiers modifier = Qt::NoModifier;
    if (argc > 1)
        modifier = value2enum<Qt::KeyboardModifiers>(argv[1]);
    int delay = -1;
    if (argc > 2)
        delay = NUM2INT(argv[2]);

    if (isChar)
	QTest::keyClick(widget, chKey, modifier, delay);
    else
	QTest::keyClick(widget, enKey, modifier, delay);
    
    return Qnil;
}

static VALUE qwidget_key_clicks(int argc, VALUE * argv, VALUE self)
{
    smokeruby_object *o = value_obj_info(self);
    QWidget * widget = qobject_cast<QWidget *>(static_cast<QObject *>(o->ptr));
    if (!widget)
        rb_call_super(argc, argv);

    if (argc < 1 || argc > 3) rb_raise(rb_eArgError, "Invalid argument list");
    char * sequence = StringValueCStr(argv[0]);

    Qt::KeyboardModifiers modifier = Qt::NoModifier;
    if (argc > 1)
        modifier = value2enum<Qt::KeyboardModifiers>(argv[1]);
    int delay = -1;
    if (argc > 2)
        delay = NUM2INT(argv[2]);

    QTest::keyClicks(widget, sequence, modifier, delay);

    return Qnil;
}

static VALUE qtest_qwait(VALUE /*self*/, VALUE ms_)
{
    int ms = NUM2INT(ms_);

    QTest::qWait(ms);
    return Qnil;
}

extern TypeHandler qttest_handlers[];

extern "C" {

Q_DECL_EXPORT void
Init_qttest()
{
    rb_require("Qt4");

    rb_define_method(qt_base_class, "key_click", (VALUE (*) (...)) qwidget_key_click, -1);
    rb_define_method(qt_base_class, "keyClick", (VALUE (*) (...)) qwidget_key_click, -1);
    rb_define_method(qt_base_class, "key_clicks", (VALUE (*) (...)) qwidget_key_clicks, -1);
    rb_define_method(qt_base_class, "keyClicks", (VALUE (*) (...)) qwidget_key_clicks, -1);

    VALUE test_class = rb_define_class_under(qt_module, "Test", rb_cObject);
    rb_define_singleton_method(test_class, "qWait", (VALUE (*) (...)) qtest_qwait, 1);
    rb_define_singleton_method(test_class, "wait", (VALUE (*) (...)) qtest_qwait, 1);
}

}

// kate: indent-width 4;
