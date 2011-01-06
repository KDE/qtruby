/***************************************************************************
                          solid.cpp  -  Solid ruby extension
                             -------------------
    begin                : 08-06-2008
    copyright            : (C) 2008 by Richard Dale
    email                : richard.j.dale@gmail.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <ruby.h>

#include <QHash>
#include <QList>
#include <QtDebug>

#include <solid_smoke.h>

#include <qtruby.h>

#include <iostream>

static VALUE getClassList(VALUE /*self*/)
{
    VALUE classList = rb_ary_new();
    for (int i = 1; i <= solid_Smoke->numClasses; i++) {
        if (solid_Smoke->classes[i].className && !solid_Smoke->classes[i].external)
            rb_ary_push(classList, rb_str_new2(solid_Smoke->classes[i].className));
    }
    return classList;
}

const char*
resolve_classname_solid(smokeruby_object * o)
{
    return qtruby_modules[o->smoke].binding->className(o->classId);
}

extern TypeHandler Solid_handlers[];

extern "C" {

VALUE solid_module;
VALUE solid_internal_module;

static QtRuby::Binding binding;

Q_DECL_EXPORT void
Init_solid()
{
    init_solid_Smoke();

    binding = QtRuby::Binding(solid_Smoke);

    smokeList << solid_Smoke;

    QtRubyModule module = { "Solid", resolve_classname_solid, 0, &binding };
    qtruby_modules[solid_Smoke] = module;

    install_handlers(Solid_handlers);

    solid_module = rb_define_module("Solid");
    solid_internal_module = rb_define_module_under(solid_module, "Internal");

    rb_define_singleton_method(solid_internal_module, "getClassList", (VALUE (*) (...)) getClassList, 0);

    rb_require("solid/solid.rb");
    rb_funcall(solid_internal_module, rb_intern("init_all_classes"), 0);
}

}
