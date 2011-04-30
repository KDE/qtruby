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

#include <QtCore/qdebug.h>
#include <QtCore/QTimer>

#include <smoke/qtcore_smoke.h>

#include <global.h>
#include <marshall.h>
#include <rubymetatype.h>
#include <utils.h>
#include <object.h>

#include "rubyqabstractitemmodel.h"
#include "rubyqobject.h"
#include "typeresolver.h"

extern bool qRegisterResourceData(int, const unsigned char *, const unsigned char *, const unsigned char *);
extern bool qUnregisterResourceData(int, const unsigned char *, const unsigned char *, const unsigned char *);

namespace QtRuby {
extern Marshall::TypeHandler QtCoreHandlers[];
extern void registerQtCoreTypes();

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

}

extern "C" {

Q_DECL_EXPORT void
Init_qtcore()
{
    init_qtcore_Smoke();
    QtRuby::Module qtcore_module = { "qtcore", new QtRuby::Binding(qtcore_Smoke) };
    QtRuby::Global::modules[qtcore_Smoke] = qtcore_module;

    QtRuby::Global::QObjectClassId = qtcore_Smoke->idClass("QObject");
    QtRuby::Global::QMetaObjectClassId = qtcore_Smoke->idClass("QMetaObject");
    QtRuby::Global::QDateClassId = qtcore_Smoke->idClass("QDate");
    QtRuby::Global::QDateTimeClassId = qtcore_Smoke->idClass("QDateTime");
    QtRuby::Global::QTimeClassId = qtcore_Smoke->idClass("QTime");
    QtRuby::Global::QEventClassId = qtcore_Smoke->idClass("QEvent");
    QtRuby::Global::QVariantClassId = qtcore_Smoke->idClass("QVariant");

    QtRuby::Marshall::installHandlers(QtRuby::QtCoreHandlers);

    QtRuby::Global::initialize();

    QtRuby::Global::defineTypeResolver(QtRuby::Global::QEventClassId, QtRuby::qeventTypeResolver);
    QtRuby::Global::defineTypeResolver(QtRuby::Global::QObjectClassId, QtRuby::qobjectTypeResolver );

    QtRuby::Global::defineMethod(   QtRuby::Global::QObjectClassId,
                                    "inherits",
                                    (VALUE (*) (...)) QtRuby::inherits_qobject,
                                    -1 );
    QtRuby::Global::defineMethod(   QtRuby::Global::QObjectClassId,
                                    "findChildren",
                                    (VALUE (*) (...)) QtRuby::find_qobject_children,
                                    -1);
    QtRuby::Global::defineMethod(   QtRuby::Global::QObjectClassId,
                                    "findChild",
                                    (VALUE (*) (...)) QtRuby::find_qobject_child,
                                    -1 );
    QtRuby::Global::defineMethod(   QtRuby::Global::QObjectClassId,
                                    "qobject_cast",
                                    (VALUE (*) (...)) QtRuby::qobject_qt_metacast,
                                    1 );

    rb_define_module_function(QtRuby::Global::QtModule, "qRegisterResourceData", (VALUE (*) (...)) QtRuby::q_register_resource_data, 4);
    rb_define_module_function(QtRuby::Global::QtModule, "qUnregisterResourceData", (VALUE (*) (...)) QtRuby::q_unregister_resource_data, 4);

    rb_require("qtcore/qtcore.rb");

    Smoke * smoke = qtcore_Smoke;
    for (int i = 1; i <= smoke->numClasses; i++) {
        Smoke::ModuleIndex classId(smoke, i);
        QString className = QString::fromLatin1(smoke->classes[i].className);
        
        if (    smoke->classes[i].external
                || className.contains("Internal")
                || className == "Qt"
                || className == "QGlobalSpace") {
            continue;
        }

        if (className.startsWith("Q"))
            className = className.mid(1).prepend("Qt::");

        VALUE klass = QtRuby::Global::initializeClass(classId, className);

        if (className == "QAbstractTableModel") {
            QtRuby::Global::QTableModelClass = rb_define_class_under(QtRuby::Global::QtModule, "TableModel", klass);
            rb_define_method(QtRuby::Global::QTableModelClass, "rowCount", (VALUE (*) (...)) QtRuby::qabstract_item_model_rowcount, -1);
            rb_define_method(QtRuby::Global::QTableModelClass, "row_count", (VALUE (*) (...)) QtRuby::qabstract_item_model_rowcount, -1);
            rb_define_method(QtRuby::Global::QTableModelClass, "columnCount", (VALUE (*) (...)) QtRuby::qabstract_item_model_columncount, -1);
            rb_define_method(QtRuby::Global::QTableModelClass, "column_count", (VALUE (*) (...)) QtRuby::qabstract_item_model_columncount, -1);
            rb_define_method(QtRuby::Global::QTableModelClass, "data", (VALUE (*) (...)) QtRuby::qabstract_item_model_data, -1);
            rb_define_method(QtRuby::Global::QTableModelClass, "setData", (VALUE (*) (...)) QtRuby::qabstract_item_model_setdata, -1);
            rb_define_method(QtRuby::Global::QTableModelClass, "set_data", (VALUE (*) (...)) QtRuby::qabstract_item_model_setdata, -1);
            rb_define_method(QtRuby::Global::QTableModelClass, "flags", (VALUE (*) (...)) QtRuby::qabstract_item_model_flags, 1);
            rb_define_method(QtRuby::Global::QTableModelClass, "insertRows", (VALUE (*) (...)) QtRuby::qabstract_item_model_insertrows, -1);
            rb_define_method(QtRuby::Global::QTableModelClass, "insert_rows", (VALUE (*) (...)) QtRuby::qabstract_item_model_insertrows, -1);
            rb_define_method(QtRuby::Global::QTableModelClass, "insertColumns", (VALUE (*) (...)) QtRuby::qabstract_item_model_insertcolumns, -1);
            rb_define_method(QtRuby::Global::QTableModelClass, "insert_columns", (VALUE (*) (...)) QtRuby::qabstract_item_model_insertcolumns, -1);
            rb_define_method(QtRuby::Global::QTableModelClass, "removeRows", (VALUE (*) (...)) QtRuby::qabstract_item_model_removerows, -1);
            rb_define_method(QtRuby::Global::QTableModelClass, "remove_rows", (VALUE (*) (...)) QtRuby::qabstract_item_model_removerows, -1);
            rb_define_method(QtRuby::Global::QTableModelClass, "removeColumns", (VALUE (*) (...)) QtRuby::qabstract_item_model_removecolumns, -1);
            rb_define_method(QtRuby::Global::QTableModelClass, "remove_columns", (VALUE (*) (...)) QtRuby::qabstract_item_model_removecolumns, -1);

            QtRuby::Global::QListModelClass = rb_define_class_under(QtRuby::Global::QtModule, "ListModel", klass);
            rb_define_method(QtRuby::Global::QListModelClass, "rowCount", (VALUE (*) (...)) QtRuby::qabstract_item_model_rowcount, -1);
            rb_define_method(QtRuby::Global::QListModelClass, "row_count", (VALUE (*) (...)) QtRuby::qabstract_item_model_rowcount, -1);
            rb_define_method(QtRuby::Global::QListModelClass, "columnCount", (VALUE (*) (...)) QtRuby::qabstract_item_model_columncount, -1);
            rb_define_method(QtRuby::Global::QListModelClass, "column_count", (VALUE (*) (...)) QtRuby::qabstract_item_model_columncount, -1);
            rb_define_method(QtRuby::Global::QListModelClass, "data", (VALUE (*) (...)) QtRuby::qabstract_item_model_data, -1);
            rb_define_method(QtRuby::Global::QListModelClass, "setData", (VALUE (*) (...)) QtRuby::qabstract_item_model_setdata, -1);
            rb_define_method(QtRuby::Global::QListModelClass, "set_data", (VALUE (*) (...)) QtRuby::qabstract_item_model_setdata, -1);
            rb_define_method(QtRuby::Global::QListModelClass, "flags", (VALUE (*) (...)) QtRuby::qabstract_item_model_flags, 1);
            rb_define_method(QtRuby::Global::QListModelClass, "insertRows", (VALUE (*) (...)) QtRuby::qabstract_item_model_insertrows, -1);
            rb_define_method(QtRuby::Global::QListModelClass, "insert_rows", (VALUE (*) (...)) QtRuby::qabstract_item_model_insertrows, -1);
            rb_define_method(QtRuby::Global::QListModelClass, "insertColumns", (VALUE (*) (...)) QtRuby::qabstract_item_model_insertcolumns, -1);
            rb_define_method(QtRuby::Global::QListModelClass, "insert_columns", (VALUE (*) (...)) QtRuby::qabstract_item_model_insertcolumns, -1);
            rb_define_method(QtRuby::Global::QListModelClass, "removeRows", (VALUE (*) (...)) QtRuby::qabstract_item_model_removerows, -1);
            rb_define_method(QtRuby::Global::QListModelClass, "remove_rows", (VALUE (*) (...)) QtRuby::qabstract_item_model_removerows, -1);
            rb_define_method(QtRuby::Global::QListModelClass, "removeColumns", (VALUE (*) (...)) QtRuby::qabstract_item_model_removecolumns, -1);
            rb_define_method(QtRuby::Global::QListModelClass, "remove_columns", (VALUE (*) (...)) QtRuby::qabstract_item_model_removecolumns, -1);
        } else if (className == "QAbstractItemModel") {
            rb_define_method(klass, "createIndex", (VALUE (*) (...)) QtRuby::qabstract_item_model_createindex, -1);
            rb_define_method(klass, "create_index", (VALUE (*) (...)) QtRuby::qabstract_item_model_createindex, -1);
        } else if (className == "QtTimer") {
            rb_define_singleton_method(klass, "singleShot", (VALUE (*) (...)) QtRuby::qtimer_single_shot, -1);
            rb_define_singleton_method(klass, "single_shot", (VALUE (*) (...)) QtRuby::qtimer_single_shot, -1);
        }
        // VALUE name = rb_funcall(klass, rb_intern("name"), 0);
        // qDebug() << "name:" << StringValuePtr(name);
    }
    
   //  QtRuby::Global::initializeClasses(qtcore_Smoke);

    QtRuby::registerQtCoreTypes();
        
    return;
}

}
