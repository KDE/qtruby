/***************************************************************************
                          khtml.cpp  -  KHTML ruby extension
                             -------------------
    begin                : Sat Jun 28 2008
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

#include <khtml_smoke.h>

#include <qtruby.h>

#include <iostream>

static VALUE getClassList(VALUE /*self*/)
{
    VALUE classList = rb_ary_new();
    for (int i = 1; i <= khtml_Smoke->numClasses; i++) {
        if (khtml_Smoke->classes[i].className && !khtml_Smoke->classes[i].external)
            rb_ary_push(classList, rb_str_new2(khtml_Smoke->classes[i].className));
    }
    return classList;
}

const char*
resolve_classname_khtml(smokeruby_object * o)
{
    return qtruby_modules[o->smoke].binding->className(o->classId);
}

extern TypeHandler KHTML_handlers[];

extern "C" {

VALUE dom_module;
VALUE khtml_internal_module;

static QtRuby::Binding binding;

Q_DECL_EXPORT void
Init_khtml()
{
    init_khtml_Smoke();

    binding = QtRuby::Binding(khtml_Smoke);

    smokeList << khtml_Smoke;

    QtRubyModule module = { "KHTML", resolve_classname_khtml, 0, &binding };
    qtruby_modules[khtml_Smoke] = module;

    install_handlers(KHTML_handlers);

    dom_module = rb_define_module("DOM");
    khtml_internal_module = rb_define_module_under(dom_module, "Internal");

    rb_define_singleton_method(khtml_internal_module, "getClassList", (VALUE (*) (...)) getClassList, 0);

    rb_require("khtml/khtml.rb");
    rb_funcall(khtml_internal_module, rb_intern("init_all_classes"), 0);
}

}
