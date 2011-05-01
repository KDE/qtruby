/*
 *   Copyright 2009 by Richard Dale <richard.j.dale@gmail.com>

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

#include <QtDBus/QDBusArgument>

#include <global.h>
#include <marshall.h>

#include <smoke/qtdbus_smoke.h>

namespace QtRuby {
extern Marshall::TypeHandler QtDBusHandlers[];
extern void registerQtDBusTypes();

static VALUE
qdbusargument_endarraywrite(VALUE self)
{
    Object::Instance * instance = Object::Instance::get(self);
    QDBusArgument * arg = reinterpret_cast<QDBusArgument *>(instance->value);
    arg->endArray();
    return self;
}

static VALUE
qdbusargument_endmapwrite(VALUE self)
{
    Object::Instance * instance = Object::Instance::get(self);
    QDBusArgument * arg = reinterpret_cast<QDBusArgument *>(instance->value);
    arg->endMap();
    return self;
}

static VALUE
qdbusargument_endmapentrywrite(VALUE self)
{
    Object::Instance * instance = Object::Instance::get(self);
    QDBusArgument * arg = reinterpret_cast<QDBusArgument *>(instance->value);
    arg->endMapEntry();
    return self;
}

static VALUE
qdbusargument_endstructurewrite(VALUE self)
{
    Object::Instance * instance = Object::Instance::get(self);
    QDBusArgument * arg = reinterpret_cast<QDBusArgument *>(instance->value);
    arg->endStructure();
    return self;
}

static void initializeClasses(Smoke * smoke)
{
    for (int i = 1; i <= smoke->numClasses; i++) {
        Smoke::ModuleIndex classId(smoke, i);
        QString className = QString::fromLatin1(smoke->classes[i].className);

        if (    smoke->classes[i].external
                || className.contains("Internal")
                || className == "Qt" )
        {
            continue;
        }

        if (className.startsWith("Q"))
            className = className.mid(1).prepend("Qt::");

        VALUE klass = Global::initializeClass(classId, className);

        if (className == "Qt::DBusArgument") {
            rb_define_method(klass, "endArrayWrite", (VALUE (*) (...)) qdbusargument_endarraywrite, 0);
            rb_define_method(klass, "end_array_write", (VALUE (*) (...)) qdbusargument_endarraywrite, 0);
            rb_define_method(klass, "endMapEntryWrite", (VALUE (*) (...)) qdbusargument_endmapentrywrite, 0);
            rb_define_method(klass, "end_map_entry_write", (VALUE (*) (...)) qdbusargument_endmapentrywrite, 0);
            rb_define_method(klass, "endMapWrite", (VALUE (*) (...)) qdbusargument_endmapwrite, 0);
            rb_define_method(klass, "end_map_write", (VALUE (*) (...)) qdbusargument_endmapwrite, 0);
            rb_define_method(klass, "endStructureWrite", (VALUE (*) (...)) qdbusargument_endstructurewrite, 0);
            rb_define_method(klass, "end_structure_write", (VALUE (*) (...)) qdbusargument_endstructurewrite, 0);
        }
    }
}

}

extern "C" {

Q_DECL_EXPORT void
Init_qtdbus()
{
    init_qtdbus_Smoke();
    QtRuby::Module qtdbus_module = { "qtdbus", new QtRuby::Binding(qtdbus_Smoke) };
    QtRuby::Global::modules[qtdbus_Smoke] = qtdbus_module;
    QtRuby::registerQtDBusTypes();
    QtRuby::Marshall::installHandlers(QtRuby::QtDBusHandlers);
    QtRuby::initializeClasses(qtdbus_Smoke);
    rb_require("qtdbus/qtdbus.rb");

    return;
}

}
