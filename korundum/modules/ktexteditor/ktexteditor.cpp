/***************************************************************************
                          ktexteditor.cpp  -  KTextEditor ruby extension
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

#include <ktexteditor_smoke.h>

#include <qtruby.h>

#include <iostream>

static VALUE getClassList(VALUE /*self*/)
{
    VALUE classList = rb_ary_new();
    for (int i = 1; i <= ktexteditor_Smoke->numClasses; i++) {
        if (ktexteditor_Smoke->classes[i].className && !ktexteditor_Smoke->classes[i].external)
            rb_ary_push(classList, rb_str_new2(ktexteditor_Smoke->classes[i].className));
    }
    return classList;
}

const char*
resolve_classname_ktexteditor(smokeruby_object * o)
{
    return qtruby_modules[o->smoke].binding->className(o->classId);
}

extern TypeHandler KTextEditor_handlers[];

extern "C" {

VALUE ktexteditor_module;
VALUE ktexteditor_internal_module;

static QtRuby::Binding binding;

Q_DECL_EXPORT void
Init_ktexteditor()
{
    init_ktexteditor_Smoke();

    binding = QtRuby::Binding(ktexteditor_Smoke);

    smokeList << ktexteditor_Smoke;

    QtRubyModule module = { "KTextEditor", resolve_classname_ktexteditor, 0, &binding };
    qtruby_modules[ktexteditor_Smoke] = module;

    install_handlers(KTextEditor_handlers);

    ktexteditor_module = rb_define_module("KTextEditor");
    ktexteditor_internal_module = rb_define_module_under(ktexteditor_module, "Internal");

    rb_define_singleton_method(ktexteditor_internal_module, "getClassList", (VALUE (*) (...)) getClassList, 0);

    rb_require("ktexteditor/ktexteditor.rb");
    rb_funcall(ktexteditor_internal_module, rb_intern("init_all_classes"), 0);
}

}
