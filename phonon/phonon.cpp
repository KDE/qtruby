/***************************************************************************
                          phonon.cpp  -  Phonon ruby extension
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

#include <smoke/phonon_smoke.h>

#include <qtruby.h>

#include <iostream>

static VALUE getClassList(VALUE /*self*/)
{
    VALUE classList = rb_ary_new();
    for (int i = 1; i < phonon_Smoke->numClasses; i++) {
        if (phonon_Smoke->classes[i].className && !phonon_Smoke->classes[i].external)
            rb_ary_push(classList, rb_str_new2(phonon_Smoke->classes[i].className));
    }
    return classList;
}

const char*
resolve_classname_phonon(smokeruby_object * o)
{
    return qtruby_modules[o->smoke].binding->className(o->classId);
}

extern TypeHandler Phonon_handlers[];

extern "C" {

VALUE phonon_module;
VALUE phonon_internal_module;

static QtRuby::Binding binding;

Q_DECL_EXPORT void
Init_phonon()
{
    init_phonon_Smoke();

    binding = QtRuby::Binding(phonon_Smoke);

    smokeList << phonon_Smoke;

    QtRubyModule module = { "Phonon", resolve_classname_phonon, 0, &binding };
    qtruby_modules[phonon_Smoke] = module;

    install_handlers(Phonon_handlers);

    phonon_module = rb_define_module("Phonon");
    phonon_internal_module = rb_define_module_under(phonon_module, "Internal");

    rb_define_singleton_method(phonon_internal_module, "getClassList", (VALUE (*) (...)) getClassList, 0);

    rb_require("phonon/phonon.rb");
    rb_funcall(phonon_internal_module, rb_intern("init_all_classes"), 0);
}

}
