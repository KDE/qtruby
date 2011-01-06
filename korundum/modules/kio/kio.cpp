/***************************************************************************
                          kio.cpp  -  KIO ruby extension
                             -------------------
    begin                : Mon Jan 8 2010
    copyright            : (C) 2010 by Jonathan Schmidt-Dominé
    email                : devel@the-user.org
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

#include <kio_smoke.h>

#include <qtruby.h>

#include <iostream>

static VALUE getClassList(VALUE /*self*/)
{
    VALUE classList = rb_ary_new();
    for (int i = 1; i <= kio_Smoke->numClasses; i++) {
        if (kio_Smoke->classes[i].className && !kio_Smoke->classes[i].external)
            rb_ary_push(classList, rb_str_new2(kio_Smoke->classes[i].className));
    }
    return classList;
}

const char*
resolve_classname_kio(smokeruby_object * o)
{
    return qtruby_modules[o->smoke].binding->className(o->classId);
}

extern TypeHandler KIO_handlers[];

extern "C" {

VALUE kio_module;
VALUE kio_internal_module;

static QtRuby::Binding binding;

Q_DECL_EXPORT void
Init_kio()
{
    init_kio_Smoke();

    binding = QtRuby::Binding(kio_Smoke);

    smokeList << kio_Smoke;

    QtRubyModule module = { "KIO", resolve_classname_kio, 0, &binding };
    qtruby_modules[kio_Smoke] = module;

    install_handlers(KIO_handlers);

    kio_module = rb_define_module("KIO");
    kio_internal_module = rb_define_module_under(kio_module, "Internal");

    rb_define_singleton_method(kio_internal_module, "getClassList", (VALUE (*) (...)) getClassList, 0);

    rb_require("kio/kio.rb");
    rb_funcall(kio_internal_module, rb_intern("init_all_classes"), 0);
}

}
