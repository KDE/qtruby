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
qmodelindex_internalpointer(VALUE self)
{
    Object::Instance * instance = Object::Instance::get(self);
    QModelIndex * index = reinterpret_cast<QModelIndex *>(instance->value);
    void * ptr = index->internalPointer();
    return ptr != 0 ? (VALUE) ptr : Qnil;
}

VALUE
qabstractitemmodel_rowcount(int argc, VALUE * argv, VALUE self)
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
qabstractitemmodel_columncount(int argc, VALUE * argv, VALUE self)
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
qabstractitemmodel_data(int argc, VALUE * argv, VALUE self)
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
qabstractitemmodel_setdata(int argc, VALUE * argv, VALUE self)
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
qabstractitemmodel_flags(VALUE self, VALUE model_index)
{
    Object::Instance * instance = Object::Instance::get(self);
    QAbstractItemModel * model  = reinterpret_cast<QAbstractItemModel*>(instance->value);
    Object::Instance * mi = Object::Instance::get(model_index);
    const QModelIndex * modelIndex = reinterpret_cast<const QModelIndex *>(mi->value);
    return INT2NUM((int) model->flags(*modelIndex));
}

VALUE
qabstractitemmodel_insertrows(int argc, VALUE * argv, VALUE self)
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
qabstractitemmodel_insertcolumns(int argc, VALUE * argv, VALUE self)
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
qabstractitemmodel_removerows(int argc, VALUE * argv, VALUE self)
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
qabstractitemmodel_removecolumns(int argc, VALUE * argv, VALUE self)
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

VALUE
qabstractitemmodel_createindex(int argc, VALUE * argv, VALUE self)
{
    if (argc == 2 || argc == 3) {
        Object::Instance * instance = Object::Instance::get(self);
        Smoke * smoke = instance->classId.smoke;
        Smoke::ModuleIndex nameId = smoke->idMethodName("createIndex$$$");
        Smoke::ModuleIndex meth = smoke->findMethod(Smoke::findClass("QAbstractItemModel"), nameId);
        Smoke::Index i = meth.smoke->methodMaps[meth.index].method;
        i = -i;     // turn into ambiguousMethodList index
        while (smoke->ambiguousMethodList[i] != 0) {
            if (    qstrcmp(    smoke->types[smoke->argumentList[smoke->methods[smoke->ambiguousMethodList[i]].args + 2]].name,
                            "void*" ) == 0 )
            {
                const Smoke::Method &m = smoke->methods[smoke->ambiguousMethodList[i]];
                Smoke::ClassFn fn = smoke->classes[m.classId].classFn;
                Smoke::StackItem stack[4];
                stack[1].s_int = NUM2INT(argv[0]);
                stack[2].s_int = NUM2INT(argv[1]);

                if (argc == 2) {
                    stack[3].s_voidp = (void*) Qnil;
                } else {
                    stack[3].s_voidp = (void*) argv[2];
                }

                (*fn)(m.method, instance->value, stack);
                return Global::wrapInstance(Smoke::findClass("QModelIndex"), stack[0].s_voidp, Object::ScriptOwnership);
            }

            i++;
        }
    }

    return rb_call_super(argc, argv);
}

}