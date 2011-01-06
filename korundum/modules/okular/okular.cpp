/***************************************************************************
                          okular.cpp  -  Okular ruby extension
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

#include <okular_smoke.h>

#include <qtruby.h>

#include <iostream>

static VALUE getClassList(VALUE /*self*/)
{
    VALUE classList = rb_ary_new();
    for (int i = 1; i <= okular_Smoke->numClasses; i++) {
        if (okular_Smoke->classes[i].className && !okular_Smoke->classes[i].external)
            rb_ary_push(classList, rb_str_new2(okular_Smoke->classes[i].className));
    }
    return classList;
}

const char*
resolve_classname_okular(smokeruby_object * o)
{
    return qtruby_modules[o->smoke].binding->className(o->classId);
}

extern TypeHandler Okular_handlers[];

extern "C" {

VALUE okular_module;
VALUE okular_internal_module;

static QtRuby::Binding binding;

Q_DECL_EXPORT void
Init_okular()
{
    init_okular_Smoke();

    binding = QtRuby::Binding(okular_Smoke);

    smokeList << okular_Smoke;

    QtRubyModule module = { "Okular", resolve_classname_okular, 0, &binding };
    qtruby_modules[okular_Smoke] = module;

    install_handlers(Okular_handlers);

    okular_module = rb_define_module("Okular");
    okular_internal_module = rb_define_module_under(okular_module, "Internal");

    rb_define_singleton_method(okular_internal_module, "getClassList", (VALUE (*) (...)) getClassList, 0);

    rb_require("okular/okular.rb");
    rb_funcall(okular_internal_module, rb_intern("init_all_classes"), 0);
}

}
