/***************************************************************************
                          kdevplatform.cpp  -  KDevelop ruby extension
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

#include <smoke/kdevplatform_smoke.h>

#include <qtruby.h>

#include <iostream>

static VALUE getClassList(VALUE /*self*/)
{
    VALUE classList = rb_ary_new();
    for (int i = 1; i < kdevplatform_Smoke->numClasses; i++) {
        if (kdevplatform_Smoke->classes[i].className && !kdevplatform_Smoke->classes[i].external)
            rb_ary_push(classList, rb_str_new2(kdevplatform_Smoke->classes[i].className));
    }
    return classList;
}

const char*
resolve_classname_kdevplatform(smokeruby_object * o)
{
    return qtruby_modules[o->smoke].binding->className(o->classId);
}

extern TypeHandler KDevPlatform_handlers[];

extern "C" {

VALUE kdevelop_module;
VALUE kdevelop_internal_module;

static QtRuby::Binding binding;

Q_DECL_EXPORT void
Init_kdevplatform()
{
    init_kdevplatform_Smoke();

    binding = QtRuby::Binding(kdevplatform_Smoke);

    smokeList << kdevplatform_Smoke;

    QtRubyModule module = { "KDevPlatform", resolve_classname_kdevplatform, 0, &binding };
    qtruby_modules[kdevplatform_Smoke] = module;

    install_handlers(KDevPlatform_handlers);

    kdevelop_module = rb_define_module("KDevelop");
    kdevelop_internal_module = rb_define_module_under(kdevelop_module, "Internal");

    rb_define_singleton_method(kdevelop_internal_module, "getClassList", (VALUE (*) (...)) getClassList, 0);

    rb_require("kdevplatform/kdevplatform.rb");
    rb_funcall(kdevelop_internal_module, rb_intern("init_all_classes"), 0);
}

}
