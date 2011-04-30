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
#include <QtCore/QAbstractItemModel>

#include <object.h>
#include <global.h>

#include "rubyqabstractitemmodel.h"

namespace QtRuby {

VALUE
qabstract_item_model_rowcount(int argc, VALUE * argv, VALUE self)
{
    Object::Instance * instance = Object::Instance::get(self);
    QAbstractItemModel * model  = reinterpret_cast<QAbstractItemModel*>(instance->value);
    if (argc == 0) {
        return INT2NUM(model->rowCount());
    }

    if (argc == 1) {
        Object::Instance * mi = Object::Instance::get(argv[0]);
        QModelIndex * modelIndex = reinterpret_cast<QModelIndex *>(mi->value);
        return INT2NUM(model->rowCount(*modelIndex));
    }

    rb_raise(rb_eArgError, "Invalid argument list");
}

VALUE
qabstract_item_model_columncount(int argc, VALUE * argv, VALUE self)
{
    Object::Instance * instance = Object::Instance::get(self);
    QAbstractItemModel * model  = reinterpret_cast<QAbstractItemModel*>(instance->value);
    if (argc == 0) {
        return INT2NUM(model->columnCount());
    }

    if (argc == 1) {
        Object::Instance * mi = Object::Instance::get(argv[0]);
        QModelIndex * modelIndex = reinterpret_cast<QModelIndex *>(mi->value);
        return INT2NUM(model->columnCount(*modelIndex));
    }

    rb_raise(rb_eArgError, "Invalid argument list");
}

VALUE
qabstract_item_model_data(int argc, VALUE * argv, VALUE self)
{
    Object::Instance * instance = Object::Instance::get(self);
    QAbstractItemModel * model  = reinterpret_cast<QAbstractItemModel*>(instance->value);
    Object::Instance * mi = Object::Instance::get(argv[0]);
    QModelIndex * modelIndex = reinterpret_cast<QModelIndex *>(mi->value);
    QVariant value;
    if (argc == 1) {
        value = model->data(*modelIndex);
    } else if (argc == 2) {
        value = model->data(*modelIndex, NUM2INT(rb_funcall(argv[1], rb_intern("to_i"), 0)));
    } else {
        rb_raise(rb_eArgError, "Invalid argument list");
    }

    return Global::wrapInstance(Global::QVariantClassId, new QVariant(value), Object::ScriptOwnership);
}

VALUE
qabstract_item_model_setdata(int argc, VALUE * argv, VALUE self)
{
    Object::Instance * instance = Object::Instance::get(self);
    QAbstractItemModel * model  = reinterpret_cast<QAbstractItemModel*>(instance->value);
    Object::Instance * mi = Object::Instance::get(argv[0]);
    QModelIndex * modelIndex = reinterpret_cast<QModelIndex *>(mi->value);
    Object::Instance * v = Object::Instance::get(argv[1]);
    QVariant * variant = reinterpret_cast<QVariant *>(v->value);

    if (argc == 2) {
        return (model->setData(*modelIndex, *variant) ? Qtrue : Qfalse);
    }

    if (argc == 3) {
        return (model->setData( *modelIndex,
                                *variant,
                                NUM2INT(rb_funcall(argv[2], rb_intern("to_i"), 0)) ) ? Qtrue : Qfalse);
    }

    rb_raise(rb_eArgError, "Invalid argument list");
}

VALUE
qabstract_item_model_flags(VALUE self, VALUE model_index)
{
    Object::Instance * instance = Object::Instance::get(self);
    QAbstractItemModel * model  = reinterpret_cast<QAbstractItemModel*>(instance->value);
    Object::Instance * mi = Object::Instance::get(model_index);
    const QModelIndex * modelIndex = reinterpret_cast<const QModelIndex *>(mi->value);
    return INT2NUM((int) model->flags(*modelIndex));
}

VALUE
qabstract_item_model_insertrows(int argc, VALUE * argv, VALUE self)
{
    Object::Instance * instance = Object::Instance::get(self);
    QAbstractItemModel * model  = reinterpret_cast<QAbstractItemModel*>(instance->value);

    if (argc == 2) {
        return (model->insertRows(NUM2INT(argv[0]), NUM2INT(argv[1])) ? Qtrue : Qfalse);
    }

    if (argc == 3) {
        Object::Instance * mi = Object::Instance::get(argv[2]);
        const QModelIndex * modelIndex = reinterpret_cast<const QModelIndex *>(mi->value);
        return (model->insertRows(NUM2INT(argv[0]), NUM2INT(argv[1]), *modelIndex) ? Qtrue : Qfalse);
    }

    rb_raise(rb_eArgError, "Invalid argument list");
}

VALUE
qabstract_item_model_insertcolumns(int argc, VALUE * argv, VALUE self)
{
    Object::Instance * instance = Object::Instance::get(self);
    QAbstractItemModel * model  = reinterpret_cast<QAbstractItemModel*>(instance->value);

    if (argc == 2) {
        return (model->insertColumns(NUM2INT(argv[0]), NUM2INT(argv[1])) ? Qtrue : Qfalse);
    }

    if (argc == 3) {
        Object::Instance * mi = Object::Instance::get(argv[2]);
        const QModelIndex * modelIndex = reinterpret_cast<const QModelIndex *>(mi->value);
        return (model->insertColumns(NUM2INT(argv[0]), NUM2INT(argv[1]), *modelIndex) ? Qtrue : Qfalse);
    }

    rb_raise(rb_eArgError, "Invalid argument list");
}

VALUE
qabstract_item_model_removerows(int argc, VALUE * argv, VALUE self)
{
    Object::Instance * instance = Object::Instance::get(self);
    QAbstractItemModel * model  = reinterpret_cast<QAbstractItemModel*>(instance->value);

    if (argc == 2) {
        return (model->removeRows(NUM2INT(argv[0]), NUM2INT(argv[1])) ? Qtrue : Qfalse);
    }

    if (argc == 3) {
        Object::Instance * mi = Object::Instance::get(argv[2]);
        const QModelIndex * modelIndex = reinterpret_cast<const QModelIndex *>(mi->value);
        return (model->removeRows(NUM2INT(argv[0]), NUM2INT(argv[1]), *modelIndex) ? Qtrue : Qfalse);
    }

    rb_raise(rb_eArgError, "Invalid argument list");
}

VALUE
qabstract_item_model_removecolumns(int argc, VALUE * argv, VALUE self)
{
    Object::Instance * instance = Object::Instance::get(self);
    QAbstractItemModel * model  = reinterpret_cast<QAbstractItemModel*>(instance->value);

    if (argc == 2) {
        return (model->removeColumns(NUM2INT(argv[0]), NUM2INT(argv[1])) ? Qtrue : Qfalse);
    }

    if (argc == 3) {
        Object::Instance * mi = Object::Instance::get(argv[2]);
        const QModelIndex * modelIndex = reinterpret_cast<const QModelIndex *>(mi->value);
        return (model->removeRows(NUM2INT(argv[0]), NUM2INT(argv[1]), *modelIndex) ? Qtrue : Qfalse);
    }

    rb_raise(rb_eArgError, "Invalid argument list");
}

}