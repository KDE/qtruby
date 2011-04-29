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

#include <QtCore/QString>
#include <QtCore/QMetaObject>

#include <object.h>
#include <global.h>

#include "rubyqobject.h"

namespace QtRuby {

VALUE
qobject_qt_metacast(VALUE self, VALUE klass)
{
    Object::Instance * instance = Object::Instance::get(self);
    if (instance == 0 || instance->value == 0) {
        return Qnil;
    }

    Smoke::ModuleIndex classId = Global::idFromRubyClass(klass);
    if (classId == Smoke::NullModuleIndex) {
        return Qnil;
    }

    QObject * qobj = reinterpret_cast<QObject*>(instance->cast(Global::QObjectClassId));
    if (qobj == 0) {
        return Qnil;
    }

    void* ret = qobj->qt_metacast(classId.smoke->classes[classId.index].className);

    if (ret == 0) {
        return Qnil;
    }

    VALUE obj = Global::wrapInstance(classId, ret, instance->ownership);
    instance->ownership = Object::QtOwnership;
    return obj;
}

// Allow classnames in both 'Qt::Widget' and 'QWidget' formats to be
// used as an argument to Qt::Object.inherits()
VALUE
inherits_qobject(int argc, VALUE * argv, VALUE /*self*/)
{
    if (argc != 1) {
        return rb_call_super(argc, argv);
    }

    Smoke::ModuleIndex classId = Smoke::findClass(StringValuePtr(argv[0]));

    if (classId == Smoke::NullModuleIndex) {
        return rb_call_super(argc, argv);
    } else {
        VALUE super_class = rb_str_new2(classId.smoke->classes[classId.index].className);
        return rb_call_super(argc, &super_class);
    }
}

/* Adapted from the internal function qt_qFindChildren() in qobject.cpp */
static void
rb_qFindChildren_helper(VALUE parent, const QString &name, VALUE re,
                         const QMetaObject &mo, VALUE list)
{
    if (parent == Qnil || list == Qnil)
        return;
    VALUE children = rb_funcall(parent, rb_intern("children"), 0);
    VALUE rv = Qnil;
    for (int i = 0; i < RARRAY_LEN(children); ++i) {
        rv = RARRAY_PTR(children)[i];
        Object::Instance * instance = Object::Instance::get(rv);

        QObject * qobj = reinterpret_cast<QObject*>(instance->cast(Global::QObjectClassId));

        // The original code had 'if (mo.cast(obj))' as a test, but it doesn't work here
        if (qobj->qt_metacast(mo.className()) != 0) {
            if (re != Qnil) {
                VALUE re_test = rb_funcall(re, rb_intern("=~"), 1, rb_funcall(rv, rb_intern("objectName"), 0));
                if (re_test != Qnil && re_test != Qfalse) {
                    rb_ary_push(list, rv);
                }
            } else {
                if (name.isNull() || qobj->objectName() == name) {
                    rb_ary_push(list, rv);
                }
            }
        }
        rb_qFindChildren_helper(rv, name, re, mo, list);
    }
    return;
}

/* Should mimic Qt4's QObject::findChildren method with this syntax:
     obj.findChildren(Qt::Widget, "Optional Widget Name")
*/
VALUE
find_qobject_children(int argc, VALUE *argv, VALUE self)
{
    if (argc < 1 || argc > 2) rb_raise(rb_eArgError, "Invalid argument list");
    Check_Type(argv[0], T_CLASS);

    QString name;
    VALUE re = Qnil;
    if (argc == 2) {
        // If the second arg isn't a String, assume it's a regular expression
        if (TYPE(argv[1]) == T_STRING) {
            name = QString::fromLatin1(StringValuePtr(argv[1]));
        } else {
            re = argv[1];
        }
    }

    VALUE metaObject = rb_funcall(argv[0], rb_intern("staticMetaObject"), 0);
    Object::Instance * instance = Object::Instance::get(metaObject);
    QMetaObject * mo = reinterpret_cast<QMetaObject*>(instance->value);
    VALUE result = rb_ary_new();
    rb_qFindChildren_helper(self, name, re, *mo, result);
    return result;
}

/* Adapted from the internal function qt_qFindChild() in qobject.cpp */
static VALUE
rb_qFindChild_helper(VALUE parent, const QString &name, const QMetaObject &mo)
{
    if (parent == Qnil)
        return Qnil;
    VALUE children = rb_funcall(parent, rb_intern("children"), 0);
    VALUE rv;
    int i;
    for (i = 0; i < RARRAY_LEN(children); ++i) {
        rv = RARRAY_PTR(children)[i];
        Object::Instance * instance = Object::Instance::get(rv);
        QObject * qobj = reinterpret_cast<QObject*>(instance->cast(Global::QObjectClassId));

        if (qobj->qt_metacast(mo.className()) != 0 && (name.isNull() || qobj->objectName() == name))
            return rv;
    }
    for (i = 0; i < RARRAY_LEN(children); ++i) {
        rv = rb_qFindChild_helper(RARRAY_PTR(children)[i], name, mo);
        if (rv != Qnil)
            return rv;
    }
    return Qnil;
}

VALUE
find_qobject_child(int argc, VALUE *argv, VALUE self)
{
    if (argc < 1 || argc > 2) rb_raise(rb_eArgError, "Invalid argument list");
    Check_Type(argv[0], T_CLASS);

    QString name;
    if (argc == 2) {
        name = QString::fromLatin1(StringValuePtr(argv[1]));
    }

    VALUE metaObject = rb_funcall(argv[0], rb_intern("staticMetaObject"), 0);
    Object::Instance * instance = Object::Instance::get(metaObject);
    QMetaObject * mo = reinterpret_cast<QMetaObject*>(instance->value);
    return rb_qFindChild_helper(self, name, *mo);
}

}