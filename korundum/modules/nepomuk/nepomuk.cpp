/***************************************************************************
                          nepomuk.cpp  -  Nepomuk ruby extension
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

#include <nepomuk_smoke.h>

#include <qtruby.h>

#include <iostream>

static VALUE getClassList(VALUE /*self*/)
{
    VALUE classList = rb_ary_new();
    for (int i = 1; i <= nepomuk_Smoke->numClasses; i++) {
        if (nepomuk_Smoke->classes[i].className && !nepomuk_Smoke->classes[i].external)
            rb_ary_push(classList, rb_str_new2(nepomuk_Smoke->classes[i].className));
    }
    return classList;
}

const char*
resolve_classname_nepomuk(smokeruby_object * o)
{
    return qtruby_modules[o->smoke].binding->className(o->classId);
}

extern TypeHandler Nepomuk_handlers[];

extern "C" {

VALUE nepomuk_module;
VALUE nepomuk_internal_module;

static QtRuby::Binding binding;

Q_DECL_EXPORT void
Init_nepomuk()
{
    init_nepomuk_Smoke();

    binding = QtRuby::Binding(nepomuk_Smoke);

    smokeList << nepomuk_Smoke;

    QtRubyModule module = { "Nepomuk", resolve_classname_nepomuk, 0, &binding };
    qtruby_modules[nepomuk_Smoke] = module;

    install_handlers(Nepomuk_handlers);

    nepomuk_module = rb_define_module("Nepomuk");
    nepomuk_internal_module = rb_define_module_under(nepomuk_module, "Internal");

    rb_define_singleton_method(nepomuk_internal_module, "getClassList", (VALUE (*) (...)) getClassList, 0);

    rb_require("nepomuk/nepomuk.rb");
    rb_funcall(nepomuk_internal_module, rb_intern("init_all_classes"), 0);
}

}
