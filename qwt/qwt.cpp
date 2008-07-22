#include <ruby.h>

#include <QHash>
#include <QList>
#include <QtDebug>

#include <smoke/qwt_smoke.h>

#include <qtruby.h>

#include <iostream>

static VALUE getClassList(VALUE /*self*/)
{
    VALUE classList = rb_ary_new();
    for (int i = 1; i < qwt_Smoke->numClasses; i++) {
        if (qwt_Smoke->classes[i].className && !qwt_Smoke->classes[i].external)
            rb_ary_push(classList, rb_str_new2(qwt_Smoke->classes[i].className));
    }
    return classList;
}

const char*
resolve_classname_qwt(smokeruby_object * o)
{
    return qtruby_modules[o->smoke].binding->className(o->classId);
}

extern TypeHandler Qwt_handlers[];

extern "C" {

VALUE qwt_module;
VALUE qwt_internal_module;

static QtRuby::Binding binding;

Q_DECL_EXPORT void
Init_qwt()
{
    init_qwt_Smoke();

    binding = QtRuby::Binding(qwt_Smoke);

    smokeList << qwt_Smoke;

    QtRubyModule module = { "Qwt", resolve_classname_qwt, 0, &binding };
    qtruby_modules[qwt_Smoke] = module;

    install_handlers(Qwt_handlers);

    qwt_module = rb_define_module("Qwt");
    qwt_internal_module = rb_define_module_under(qwt_module, "Internal");

    rb_define_singleton_method(qwt_internal_module, "getClassList", (VALUE (*) (...)) getClassList, 0);

    rb_require("qwt/qwt.rb");
    rb_funcall(qwt_internal_module, rb_intern("init_all_classes"), 0);
}

}
