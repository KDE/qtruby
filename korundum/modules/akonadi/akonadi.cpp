/***************************************************************************
                          akonadi.cpp  -  Akonadi ruby extension
                             -------------------
    begin                : Thurs May 29 2008
    copyright            : (C) 2008 by Richard Dale
    email                : Richard_Dale@tipitina.demon.co.uk
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

#include <akonadi_smoke.h>

#include <qtruby.h>

#include <iostream>

static VALUE getClassList(VALUE /*self*/)
{
    VALUE classList = rb_ary_new();
    for (int i = 1; i <= akonadi_Smoke->numClasses; i++) {
        if (akonadi_Smoke->classes[i].className && !akonadi_Smoke->classes[i].external)
            rb_ary_push(classList, rb_str_new2(akonadi_Smoke->classes[i].className));
    }
    return classList;
}

const char*
resolve_classname_akonadi(smokeruby_object * o)
{
    return qtruby_modules[o->smoke].binding->className(o->classId);
}

extern TypeHandler Akonadi_handlers[];

extern "C" {

VALUE akonadi_module;
VALUE akonadi_internal_module;

static QtRuby::Binding binding;

Q_DECL_EXPORT void
Init_akonadi()
{
    init_akonadi_Smoke();

    binding = QtRuby::Binding(akonadi_Smoke);

    smokeList << akonadi_Smoke;

    QtRubyModule module = { "Akonadi", resolve_classname_akonadi, 0, &binding };
    qtruby_modules[akonadi_Smoke] = module;

    install_handlers(Akonadi_handlers);

    akonadi_module = rb_define_module("Akonadi");
    akonadi_internal_module = rb_define_module_under(akonadi_module, "Internal");

    rb_define_singleton_method(akonadi_internal_module, "getClassList", (VALUE (*) (...)) getClassList, 0);

    rb_require("akonadi/akonadi.rb");
    rb_funcall(akonadi_internal_module, rb_intern("init_all_classes"), 0);
}

}
