#include <ruby.h>

#include <QHash>
#include <QList>
#include <QtDebug>

#include <kate_smoke.h>

#include <qtruby.h>

#include <iostream>

static VALUE getClassList(VALUE /*self*/)
{
    VALUE classList = rb_ary_new();
    for (int i = 1; i <= kate_Smoke->numClasses; i++) {
        if (kate_Smoke->classes[i].className && !kate_Smoke->classes[i].external)
            rb_ary_push(classList, rb_str_new2(kate_Smoke->classes[i].className));
    }
    return classList;
}

const char*
resolve_classname_kate(smokeruby_object * o)
{
    return qtruby_modules[o->smoke].binding->className(o->classId);
}

extern TypeHandler Kate_handlers[];

extern "C" {

VALUE kate_module;
VALUE kate_internal_module;

static VALUE kate_module_method_missing(int argc, VALUE * argv, VALUE klass)
{
    return class_method_missing(argc, argv, klass);
}

static QtRuby::Binding binding;

Q_DECL_EXPORT void
Init_kate()
{
    rb_require("korundum4");    // need to initialize the core runtime first
    rb_require("ktexteditor");
    init_kate_Smoke();
    set_qtruby_embedded(true);

    binding = QtRuby::Binding(kate_Smoke);

    smokeList << kate_Smoke;

    QtRubyModule module = { "Kate", resolve_classname_kate, 0, &binding };
    qtruby_modules[kate_Smoke] = module;

    install_handlers(Kate_handlers);

    kate_module = rb_define_module("Kate");
    kate_internal_module = rb_define_module_under(kate_module, "Internal");

    rb_define_singleton_method(kate_module, "method_missing", (VALUE (*) (...)) kate_module_method_missing, -1);
    rb_define_singleton_method(kate_module, "const_missing", (VALUE (*) (...)) kate_module_method_missing, -1);

    rb_define_singleton_method(kate_internal_module, "getClassList", (VALUE (*) (...)) getClassList, 0);

    rb_require("KDE/kate.rb");
    rb_funcall(kate_internal_module, rb_intern("init_all_classes"), 0);
}

}
