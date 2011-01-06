#include <ruby.h>

#include <QHash>
#include <QList>
#include <QtDebug>

#include <plasma_smoke.h>

#include <qtruby.h>

#include <iostream>

static VALUE getClassList(VALUE /*self*/)
{
    VALUE classList = rb_ary_new();
    for (int i = 1; i <= plasma_Smoke->numClasses; i++) {
        if (plasma_Smoke->classes[i].className && !plasma_Smoke->classes[i].external)
            rb_ary_push(classList, rb_str_new2(plasma_Smoke->classes[i].className));
    }
    return classList;
}

const char*
resolve_classname_plasma(smokeruby_object * o)
{
    return qtruby_modules[o->smoke].binding->className(o->classId);
}

extern TypeHandler Plasma_handlers[];

extern "C" {

VALUE plasma_module;
VALUE plasma_internal_module;

static VALUE plasma_module_method_missing(int argc, VALUE * argv, VALUE klass)
{
    return class_method_missing(argc, argv, klass);
}

static QtRuby::Binding binding;

Q_DECL_EXPORT void
Init_plasma_applet()
{
    rb_require("korundum4");    // need to initialize the core runtime first
    init_plasma_Smoke();
    set_qtruby_embedded(true);

    binding = QtRuby::Binding(plasma_Smoke);

    smokeList << plasma_Smoke;

    QtRubyModule module = { "Plasma", resolve_classname_plasma, 0, &binding };
    qtruby_modules[plasma_Smoke] = module;

    install_handlers(Plasma_handlers);

    plasma_module = rb_define_module("Plasma");
    plasma_internal_module = rb_define_module_under(plasma_module, "Internal");

    rb_define_singleton_method(plasma_module, "method_missing", (VALUE (*) (...)) plasma_module_method_missing, -1);
    rb_define_singleton_method(plasma_module, "const_missing", (VALUE (*) (...)) plasma_module_method_missing, -1);

    rb_define_singleton_method(plasma_internal_module, "getClassList", (VALUE (*) (...)) getClassList, 0);

    rb_require("KDE/plasma.rb");
    rb_funcall(plasma_internal_module, rb_intern("init_all_classes"), 0);
}

}
