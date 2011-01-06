#include <ruby.h>

#include <QHash>
#include <QList>
#include <QtDebug>

#include <soprano/node.h>
#include <soprano/statement.h>

#include <soprano_smoke.h>

#include <qtruby.h>

#include <iostream>

Q_DECLARE_METATYPE(Soprano::Statement)
Q_DECLARE_METATYPE(Soprano::Node)

static VALUE getClassList(VALUE /*self*/)
{
    VALUE classList = rb_ary_new();
    for (int i = 1; i <= soprano_Smoke->numClasses; i++) {
        if (soprano_Smoke->classes[i].className && !soprano_Smoke->classes[i].external)
            rb_ary_push(classList, rb_str_new2(soprano_Smoke->classes[i].className));
    }
    return classList;
}

const char*
resolve_classname_soprano(smokeruby_object * o)
{
    return qtruby_modules[o->smoke].binding->className(o->classId);
}

extern TypeHandler Soprano_handlers[];

extern "C" {

VALUE soprano_module;
VALUE soprano_internal_module;

static QtRuby::Binding binding;

Q_DECL_EXPORT void
Init_soprano()
{
    init_soprano_Smoke();

    binding = QtRuby::Binding(soprano_Smoke);

    smokeList << soprano_Smoke;

    QtRubyModule module = { "Soprano", resolve_classname_soprano, 0, &binding };
    qtruby_modules[soprano_Smoke] = module;

    install_handlers(Soprano_handlers);

    soprano_module = rb_define_module("Soprano");
    soprano_internal_module = rb_define_module_under(soprano_module, "Internal");

    rb_define_singleton_method(soprano_internal_module, "getClassList", (VALUE (*) (...)) getClassList, 0);

    (void) qRegisterMetaType<Soprano::Statement>();
    (void) qRegisterMetaType<Soprano::Node>();

    rb_require("soprano/soprano.rb");
    rb_funcall(soprano_internal_module, rb_intern("init_all_classes"), 0);
}

}
