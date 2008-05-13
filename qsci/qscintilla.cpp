#include <ruby.h>

#include <QHash>
#include <QList>
#include <QtDebug>

#include <qsci/qsci_smoke.h>

#include <qtruby.h>

#include <iostream>

static VALUE getClassList(VALUE /*self*/)
{
    VALUE classList = rb_ary_new();
    for (int i = 1; i < qsci_Smoke->numClasses; i++) {
        if (qsci_Smoke->classes[i].className && !qsci_Smoke->classes[i].external)
            rb_ary_push(classList, rb_str_new2(qsci_Smoke->classes[i].className));
    }
    return classList;
}

const char*
resolve_classname_qsci(Smoke* smoke, int classId, void* /*ptr*/)
{
    return smoke->binding->className(classId);
}

extern "C" {

VALUE qscintilla_module;
VALUE qscintilla_internal_module;

Q_DECL_EXPORT void
Init_qsciruby()
{
    rb_require("Qt");    // need to initialize the core runtime first
    init_qsci_Smoke();

    qsci_Smoke->binding = new QtRubySmokeBinding(qsci_Smoke);

    smokeList << qsci_Smoke;

    QtRubyModule module = { "Qsci", resolve_classname_qsci, 0 };
    modules[qsci_Smoke] = module;

    qscintilla_module = rb_define_module("Qsci");
    qscintilla_internal_module = rb_define_module_under(qscintilla_module, "Internal");

    rb_define_singleton_method(qscintilla_internal_module, "getClassList", (VALUE (*) (...)) getClassList, 0);

    rb_require("Qsci/qscintilla.rb");
    rb_funcall(qscintilla_internal_module, rb_intern("init_all_classes"), 0);
}

}
