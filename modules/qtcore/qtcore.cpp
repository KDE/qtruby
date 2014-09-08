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

#include <QDebug>
#include <QCoreApplication>
#include <QTimer>
#include <QStringList>

#include <smoke/qtcore_smoke.h>

#include <global.h>
#include <marshall.h>
#include <rubymetatype.h>
#include <utils.h>
#include <object.h>

#include "rubyqabstractitemmodel.h"
#include "rubyqbytearray.h"
#include "rubyqobject.h"
#include "rubyqvariant.h"
#include "typeresolver.h"

extern bool qRegisterResourceData(int, const unsigned char *, const unsigned char *, const unsigned char *);
extern bool qUnregisterResourceData(int, const unsigned char *, const unsigned char *, const unsigned char *);

namespace QtRuby {
extern Marshall::TypeHandler QtCoreHandlers[];
extern void registerQtCoreTypes();

// The only way to convert a QChar to a QString is to
// pass a QChar to a QString constructor. However,
// QStrings aren't in the QtRuby api, so add this
// convenience method 'Qt::Char.to_s' to get a ruby
// string from a Qt::Char.
static VALUE
qchar_to_s(VALUE self)
{
    Object::Instance * instance = Object::Instance::get(self);
    if (instance == 0 || instance->isNull()) {
        return Qnil;
    }

    QChar * qchar = reinterpret_cast<QChar*>(instance->value);
    QString s(*qchar);
    return qRubyValueFromValue(s);
}

// Returns $qApp.ARGV() - the original ARGV array with Qt command line options removed
static VALUE
qcoreapplication_argv(VALUE /*self*/)
{
    VALUE result = rb_ary_new();
    // Drop argv[0], as it isn't included in the ruby global ARGV
    for (int index = 1; index < QCoreApplication::instance()->arguments().size(); ++index) {
        rb_ary_push(result, rb_str_new2(QCoreApplication::instance()->arguments().at(index).toLatin1().constData()));
    }

    return result;
}

static VALUE
q_register_resource_data(VALUE /*self*/, VALUE version, VALUE tree_value, VALUE name_value, VALUE data_value)
{
    const unsigned char * tree = (const unsigned char *) malloc(RSTRING_LEN(tree_value));
    memcpy((void *) tree, (const void *) RSTRING_PTR(tree_value), RSTRING_LEN(tree_value));

    const unsigned char * name = (const unsigned char *) malloc(RSTRING_LEN(name_value));
    memcpy((void *) name, (const void *) RSTRING_PTR(name_value), RSTRING_LEN(name_value));

    const unsigned char * data = (const unsigned char *) malloc(RSTRING_LEN(data_value));
    memcpy((void *) data, (const void *) RSTRING_PTR(data_value), RSTRING_LEN(data_value));

    return qRegisterResourceData(NUM2INT(version), tree, name, data) ? Qtrue : Qfalse;
}

static VALUE
q_unregister_resource_data(VALUE /*self*/, VALUE version, VALUE tree_value, VALUE name_value, VALUE data_value)
{
    const unsigned char * tree = (const unsigned char *) malloc(RSTRING_LEN(tree_value));
    memcpy((void *) tree, (const void *) RSTRING_PTR(tree_value), RSTRING_LEN(tree_value));

    const unsigned char * name = (const unsigned char *) malloc(RSTRING_LEN(name_value));
    memcpy((void *) name, (const void *) RSTRING_PTR(name_value), RSTRING_LEN(name_value));

    const unsigned char * data = (const unsigned char *) malloc(RSTRING_LEN(data_value));
    memcpy((void *) data, (const void *) RSTRING_PTR(data_value), RSTRING_LEN(data_value));

    return qUnregisterResourceData(NUM2INT(version), tree, name, data) ? Qtrue : Qfalse;
}

static VALUE
qtimer_single_shot(int argc, VALUE * argv, VALUE /*self*/)
{
    if (rb_block_given_p()) {
        if (argc == 2) {
            return rb_funcall(Global::QtInternalModule, rb_intern("single_shot_timer_connect"), 3, argv[0], argv[1], rb_block_proc());
        } else {
            rb_raise(rb_eArgError, "Invalid argument list");
        }
    } else {
        return rb_call_super(argc, argv);
    }
}

// static int rObject_typeId;

// QMetaType helpers
static void delete_ruby_object(void *ptr)
{
    rb_gc_unregister_address((VALUE*) ptr);
    delete (VALUE*) ptr;
}

static void *create_ruby_object(const void *copyFrom)
{
    VALUE *object;

    if (copyFrom) {
        object = new VALUE(*(VALUE*) copyFrom);
    } else {
        object = new VALUE(Qnil);
    }

    rb_gc_register_address(object);
    return object;
}

static void initializeClasses(Smoke * smoke)
{
    Global::QObjectClassId = smoke->idClass("QObject");
    Global::QMetaObjectClassId = smoke->idClass("QMetaObject");
    Global::QDateClassId = smoke->idClass("QDate");
    Global::QDateTimeClassId = smoke->idClass("QDateTime");
    Global::QTimeClassId = smoke->idClass("QTime");
    Global::QEventClassId = smoke->idClass("QEvent");
    Global::QVariantClassId = smoke->idClass("QVariant");

    Global::defineTypeResolver(Global::QEventClassId, qeventTypeResolver);
    Global::defineTypeResolver(Global::QObjectClassId, qobjectTypeResolver );

    Global::defineMethod(Global::QObjectClassId, "inherits", (VALUE (*) (...)) inherits_qobject, -1);
    Global::defineMethod(Global::QObjectClassId, "findChildren", (VALUE (*) (...)) find_qobject_children, -1);
    Global::defineMethod(Global::QObjectClassId, "findChild", (VALUE (*) (...)) find_qobject_child, -1);
    Global::defineMethod(Global::QObjectClassId, "qobject_cast", (VALUE (*) (...)) qobject_qt_metacast, 1);

    rb_define_module_function(Global::QtModule, "qRegisterResourceData", (VALUE (*) (...)) q_register_resource_data, 4);
    rb_define_module_function(Global::QtModule, "qUnregisterResourceData", (VALUE (*) (...)) q_unregister_resource_data, 4);

    QMetaType::registerType("rObject", &delete_ruby_object, &create_ruby_object);

    for (int i = 1; i <= smoke->numClasses; i++) {
        Smoke::ModuleIndex classId(smoke, i);
        QString className = QString::fromLatin1(smoke->classes[i].className);

        if (smoke->classes[i].external || className.contains("Internal")) {
            continue;
        }

        if (className != "Qt" && className.startsWith("Q"))
            className = className.mid(1).prepend("Qt::");

        VALUE klass = Global::initializeClass(classId, className);

        if (className == "Qt::MetaObject") {
            Global::QMetaObjectClass = klass;
        } else if (className == "Qt::Variant") {
            Global::QVariantClass = klass;
            Global::QVariantClassId = classId;
            rb_define_singleton_method(Global::QVariantClass, "fromValue", (VALUE (*) (...)) qvariant_from_value, -1);
            rb_define_singleton_method(Global::QVariantClass, "from_value", (VALUE (*) (...)) qvariant_from_value, -1);
            rb_define_singleton_method(Global::QVariantClass, "new", (VALUE (*) (...)) new_qvariant, -1);
        } else if (className == "Qt::Char") {
            rb_define_method(klass, "to_s", (VALUE (*) (...)) qchar_to_s, 0);
        } else if (className == "QByteArray") {
            rb_define_method(klass, "+", (VALUE (*) (...)) qbytearray_append, 1);
            rb_define_method(klass, "data", (VALUE (*) (...)) qbytearray_data, 0);
            rb_define_method(klass, "constData", (VALUE (*) (...)) qbytearray_data, 0);
            rb_define_method(klass, "const_data", (VALUE (*) (...)) qbytearray_data, 0);
        } else if (className == "Qt::ModelIndex") {
            rb_define_method(klass, "internalPointer", (VALUE (*) (...)) qmodelindex_internalpointer, 0);
            rb_define_method(klass, "internal_pointer", (VALUE (*) (...)) qmodelindex_internalpointer, 0);
        } else if (className == "Qt::CoreApplication") {
            Global::defineMethod(classId, "ARGV", (VALUE (*) (...)) qcoreapplication_argv, 0);
        } else if (className == "Qt::AbstractTableModel") {
            Global::QTableModelClass = rb_define_class_under(Global::QtModule, "TableModel", klass);
            rb_define_method(Global::QTableModelClass, "rowCount", (VALUE (*) (...)) qabstractitemmodel_rowcount, -1);
            rb_define_method(Global::QTableModelClass, "row_count", (VALUE (*) (...)) qabstractitemmodel_rowcount, -1);
            rb_define_method(Global::QTableModelClass, "columnCount", (VALUE (*) (...)) qabstractitemmodel_columncount, -1);
            rb_define_method(Global::QTableModelClass, "column_count", (VALUE (*) (...)) qabstractitemmodel_columncount, -1);
            rb_define_method(Global::QTableModelClass, "data", (VALUE (*) (...)) qabstractitemmodel_data, -1);
            rb_define_method(Global::QTableModelClass, "setData", (VALUE (*) (...)) qabstractitemmodel_setdata, -1);
            rb_define_method(Global::QTableModelClass, "set_data", (VALUE (*) (...)) qabstractitemmodel_setdata, -1);
            rb_define_method(Global::QTableModelClass, "flags", (VALUE (*) (...)) qabstractitemmodel_flags, 1);
            rb_define_method(Global::QTableModelClass, "insertRows", (VALUE (*) (...)) qabstractitemmodel_insertrows, -1);
            rb_define_method(Global::QTableModelClass, "insert_rows", (VALUE (*) (...)) qabstractitemmodel_insertrows, -1);
            rb_define_method(Global::QTableModelClass, "insertColumns", (VALUE (*) (...)) qabstractitemmodel_insertcolumns, -1);
            rb_define_method(Global::QTableModelClass, "insert_columns", (VALUE (*) (...)) qabstractitemmodel_insertcolumns, -1);
            rb_define_method(Global::QTableModelClass, "removeRows", (VALUE (*) (...)) qabstractitemmodel_removerows, -1);
            rb_define_method(Global::QTableModelClass, "remove_rows", (VALUE (*) (...)) qabstractitemmodel_removerows, -1);
            rb_define_method(Global::QTableModelClass, "removeColumns", (VALUE (*) (...)) qabstractitemmodel_removecolumns, -1);
            rb_define_method(Global::QTableModelClass, "remove_columns", (VALUE (*) (...)) qabstractitemmodel_removecolumns, -1);

            Global::QListModelClass = rb_define_class_under(Global::QtModule, "ListModel", klass);
            rb_define_method(Global::QListModelClass, "rowCount", (VALUE (*) (...)) qabstractitemmodel_rowcount, -1);
            rb_define_method(Global::QListModelClass, "row_count", (VALUE (*) (...)) qabstractitemmodel_rowcount, -1);
            rb_define_method(Global::QListModelClass, "columnCount", (VALUE (*) (...)) qabstractitemmodel_columncount, -1);
            rb_define_method(Global::QListModelClass, "column_count", (VALUE (*) (...)) qabstractitemmodel_columncount, -1);
            rb_define_method(Global::QListModelClass, "data", (VALUE (*) (...)) qabstractitemmodel_data, -1);
            rb_define_method(Global::QListModelClass, "setData", (VALUE (*) (...)) qabstractitemmodel_setdata, -1);
            rb_define_method(Global::QListModelClass, "set_data", (VALUE (*) (...)) qabstractitemmodel_setdata, -1);
            rb_define_method(Global::QListModelClass, "flags", (VALUE (*) (...)) qabstractitemmodel_flags, 1);
            rb_define_method(Global::QListModelClass, "insertRows", (VALUE (*) (...)) qabstractitemmodel_insertrows, -1);
            rb_define_method(Global::QListModelClass, "insert_rows", (VALUE (*) (...)) qabstractitemmodel_insertrows, -1);
            rb_define_method(Global::QListModelClass, "insertColumns", (VALUE (*) (...)) qabstractitemmodel_insertcolumns, -1);
            rb_define_method(Global::QListModelClass, "insert_columns", (VALUE (*) (...)) qabstractitemmodel_insertcolumns, -1);
            rb_define_method(Global::QListModelClass, "removeRows", (VALUE (*) (...)) qabstractitemmodel_removerows, -1);
            rb_define_method(Global::QListModelClass, "remove_rows", (VALUE (*) (...)) qabstractitemmodel_removerows, -1);
            rb_define_method(Global::QListModelClass, "removeColumns", (VALUE (*) (...)) qabstractitemmodel_removecolumns, -1);
            rb_define_method(Global::QListModelClass, "remove_columns", (VALUE (*) (...)) qabstractitemmodel_removecolumns, -1);
        } else if (className == "Qt::AbstractItemModel") {
            rb_define_method(klass, "createIndex", (VALUE (*) (...)) qabstractitemmodel_createindex, -1);
            rb_define_method(klass, "create_index", (VALUE (*) (...)) qabstractitemmodel_createindex, -1);
        } else if (className == "Qt::Timer") {
            rb_define_singleton_method(klass, "singleShot", (VALUE (*) (...)) qtimer_single_shot, -1);
            rb_define_singleton_method(klass, "single_shot", (VALUE (*) (...)) qtimer_single_shot, -1);
        }
        // VALUE name = rb_funcall(klass, rb_intern("name"), 0);
        // qDebug() << "name:" << StringValuePtr(name);
    }
}

}

extern "C" {

Q_DECL_EXPORT void
Init_qtcore()
{
    init_qtcore_Smoke();
    QtRuby::Module qtcore_module = { "qtcore", new QtRuby::Binding(qtcore_Smoke) };
    QtRuby::Global::modules[qtcore_Smoke] = qtcore_module;
    QtRuby::registerQtCoreTypes();
    QtRuby::Marshall::installHandlers(QtRuby::QtCoreHandlers);
    QtRuby::Global::initialize();
    QtRuby::initializeClasses(qtcore_Smoke);
    rb_require("qtcore/qtcore.rb");

    return;
}

}
