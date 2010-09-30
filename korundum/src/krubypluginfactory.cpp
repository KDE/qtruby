/***************************************************************************
 *   Copyright (C) 2005,2006,2007 by Siraj Razick <siraj@kdemail.net>      *
 *   Copyright (C) 2007 by Riccardo Iaconelli <ruphy@fsfe.org>             *
 *   Copyright (C) 2007 by Matthias Kretz <kretz@kde.org>                  *
 *   Copyright (C) 2008 by Richard Dale <richard.j.dale@gmail.com>         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

#include <ruby.h>

#include <QString>
#include <QDir>
#include <QFileInfo>

#include <QWidget>

#include <KStandardDirs>
#include <klibloader.h>
#include <kdebug.h>

#include <kio/slavebase.h>

#include <qtruby.h>

extern "C" {
extern VALUE rb_load_path;
extern VALUE qt_internal_module;
}

//
// This function was borrowed from the kross code. It puts out
// an error message and stacktrace on stderr for the current exception.
//
static void
show_exception_message()
{
    VALUE info = rb_gv_get("$!");
    VALUE bt = rb_funcall(info, rb_intern("backtrace"), 0);
    VALUE message = RARRAY_PTR(bt)[0];

    VALUE obj_string = rb_obj_as_string(info);
    QString errormessage = QString("%1: %2 (%3)")
                            .arg( QString::fromLatin1(StringValuePtr(message)) )
                            .arg( QString::fromLatin1(StringValuePtr(obj_string)) )
                            .arg( QString::fromLatin1(rb_class2name(CLASS_OF(info))) );
    fprintf(stderr, "%s\n", errormessage.toLatin1().constData());

    QString tracemessage;
    for(int i = 1; i < RARRAY_LEN(bt); ++i) {
        if( TYPE(RARRAY_PTR(bt)[i]) == T_STRING ) {
            QString s = QString("%1\n").arg( QString::fromLatin1(StringValuePtr(RARRAY_PTR(bt)[i])) );
            Q_ASSERT( ! s.isNull() );
            tracemessage += s;
            fprintf(stderr, "\t%s", s.toLatin1().constData());
        }
    }
}

static VALUE plugin_class = Qnil;

static VALUE
create_plugin_instance3(VALUE av)
{
    VALUE pv = rb_ary_pop(av);
    VALUE pw = rb_ary_pop(av);
    return rb_funcall(plugin_class, rb_intern("new"), 3, pv, pw, av);
}

static VALUE
create_plugin_instance2(VALUE av)
{
    VALUE pv = rb_ary_pop(av);
    return rb_funcall(plugin_class, rb_intern("new"), 2, pv, av);
}

class KRubyPluginFactory : public KPluginFactory
{
    public:
        KRubyPluginFactory();

    protected:
        virtual QObject *create(const char *iface, QWidget *parentWidget, QObject *parent, const QVariantList &args, const QString &keyword);

    public:
        static QByteArray camelize(QByteArray name);
};
K_EXPORT_PLUGIN(KRubyPluginFactory)

KRubyPluginFactory::KRubyPluginFactory()
    : KPluginFactory() // no useful KComponentData object for now
{
}

QByteArray KRubyPluginFactory::camelize(QByteArray name)
{
    // Convert foo_bar_baz to FooBarBaz
    QByteArray camelCaseName = name.left(1).toUpper();
    for (int i = 1; i < name.size(); i++) {
        if (name[i] == '_' || name[i] == '-') {
            i++;
            if (i < name.size()) {
                 camelCaseName += name.mid(i, 1).toUpper();
            }
        } else {
             camelCaseName += name[i];
        }
    }

    return camelCaseName;
}

QObject *KRubyPluginFactory::create(const char *iface, QWidget *parentWidget, QObject *parent, const QVariantList &args, const QString &keyword)
{
    Q_UNUSED(iface);
    Q_UNUSED(parentWidget);

    if (keyword.isEmpty()) {
        kWarning() << "\"keyword\" is empty. It's either missing in the .desktop file or the app doesn't pass it to the pluginfactory.";
        return 0;
    }

    QString path = KStandardDirs::locate("data", keyword);

    if (path.isEmpty()) {
        kWarning() << "Ruby script" << keyword << "missing";
        return 0;
    }

    QFileInfo program(path);

#ifdef RUBY_INIT_STACK
    RUBY_INIT_STACK
#endif

#if RUBY_VERSION < 0x10900
    bool firstTime = (rb_load_path == 0);
#else
    bool firstTime = true;
#endif

    ruby_init();
    ruby_script(QFile::encodeName(program.fileName()));

    // If ruby_init_loadpath() is called more than once, it keeps
    // adding the same standard directories to it.
    if (firstTime) {
        ruby_init_loadpath();
    }

    ruby_incpush(QFile::encodeName(program.path()));

    int state = 0;
    const QByteArray encodedFilePath = QFile::encodeName(program.filePath());
    rb_load_protect(rb_str_new2(encodedFilePath), 0, &state);
    if (state != 0) {
        show_exception_message();
        kWarning() << "Failed to load" << encodedFilePath << keyword << path;
        return 0;
    }

    // A path of my_app/foo_bar.rb is turned into module/class 'MyApp::FooBar'
    const QByteArray moduleName = KRubyPluginFactory::camelize(QFile::encodeName(program.dir().dirName()));
    const QByteArray className = KRubyPluginFactory::camelize(program.baseName().toLatin1());

    VALUE plugin_module = rb_const_get(rb_cObject, rb_intern(moduleName));
    if (plugin_module == Qnil) {
        kWarning() << "no " << moduleName << " module found";
        return 0;
    }

    plugin_class = rb_const_get(plugin_module, rb_intern(className));
    if (plugin_class == Qnil) {
        kWarning() << "no " << moduleName << "::" << className << " class found";
        return 0;
    }

    VALUE pw, po;
    #define MKPARENT(cvar, rvar)\
    if(cvar == 0) \
        rvar = Qnil; \
    else \
    { \
        Smoke::ModuleIndex mi = smokeList[0]->findClass("QObject"); \
        smokeruby_object *o = alloc_smokeruby_object(false, mi.smoke, mi.smoke->idClass("QObject").index, cvar); \
        const char *class_name = resolve_classname(o); \
        rvar = set_obj_info(class_name, o); \
    }
    MKPARENT(parentWidget, pw)
    MKPARENT(parent, po)
    #undef MKPARENT

    // Assume the args list only contains strings, ints and booleans
    VALUE av = rb_ary_new();
    for (int i = 0; i < args.size(); ++i) {
        if (args.at(i).type() == QVariant::String) {
            rb_ary_push(av, rb_str_new2(args.at(i).toByteArray()));
        } else if (args.at(i).type() == QVariant::Int) {
            rb_ary_push(av, INT2NUM(args.at(i).toInt()));
        } else if (args.at(i).type() == QVariant::Bool) {
            rb_ary_push(av, args.at(i).toBool() ? Qtrue : Qfalse);
        }
    }
    rb_ary_push(av, pw);
    rb_ary_push(av, po);

    VALUE plugin_value = rb_protect(create_plugin_instance3, av, &state);
    if (state != 0 || plugin_value == Qnil) {
        VALUE lasterr = rb_gv_get("$!");
        VALUE klass = rb_class_path(CLASS_OF(lasterr));    
        if (qstrcmp(StringValuePtr(klass), "ArgumentError") != 0) {
            show_exception_message();
        }
        
        rb_ary_push(av, po);
        plugin_value = rb_protect(create_plugin_instance2, av, &state);
        if (state != 0 || plugin_value == Qnil) {
            show_exception_message();
            kWarning() << "failed to create instance of plugin class";
            return 0;
        }
    }

    // Set an instance variable '@componentData' that contains the plugin specific
    // component data.
    Smoke::ModuleIndex mi = Smoke::findClass("KComponentData");
    smokeruby_object *comp_o = alloc_smokeruby_object(true, mi.smoke, mi.index, new KComponentData(moduleName, moduleName));
    const char *class_name = resolve_classname(comp_o);
    VALUE rb_componentData = set_obj_info(class_name, comp_o);
    rb_iv_set(plugin_value, "@componentData", rb_componentData);

    // Set a global variable '$my_app_foo_bar + <numeric id>' to the value of the new 
    // instance of MyApp::FooBar to prevent it being GC'd. Note that it would be
    // better to be able to come up with a way to discover all the plugin instances,
    // and call rb_gc_mark() on them, in the mark phase of GC.
    QByteArray variableBaseName("$");
    variableBaseName += QFile::encodeName(program.dir().dirName());
    variableBaseName += "_";
    variableBaseName += program.baseName().toLatin1();

    // Handle multiple instances of the same class, and look for an unused global
    // variable
    QByteArray variableName;
    VALUE variable = Qnil;
    int id = 0;
    do {
        id++;
        variableName = variableBaseName + QByteArray::number(id);
        variable = rb_gv_get(variableName);
    } while (variable != Qnil);
    rb_gv_set(variableName, plugin_value);

    smokeruby_object *o = 0;
    Data_Get_Struct(plugin_value, smokeruby_object, o);
    QObject * createdInstance = reinterpret_cast<QObject *>(o->ptr);
    if(createdInstance->parent() == 0)
        createdInstance->setParent(parent);
    return createdInstance;
}

VALUE slave_class = Qnil;

static VALUE create_slave_instance(VALUE av)
{
    VALUE a = rb_ary_pop(av);
    VALUE b = rb_ary_pop(av);
    VALUE c = rb_ary_pop(av);

    return rb_funcall(slave_class, rb_intern("new"), 3, c, b, a);
}

extern "C" {

Q_DECL_EXPORT
int kdemain(int argc, char **argv)
{
    if (argc != 4) {
        printf("USAGE: krubypluginfactory protocol pool_sock app_sock");
        return -1;
    }

    KComponentData("krubypluginfactory");

    KRubyPluginFactory factory;

    QByteArray protocol(argv[1]);
    QByteArray pool_sock(argv[2]);
    QByteArray app_sock(argv[3]);

    // find the script
    QString keyword("kio_");
    keyword.append(protocol);
    keyword.append("/main.rb");
    QString path = KStandardDirs::locate("data", keyword);

    if (path.isEmpty()) {
        kWarning() << "Couldn't find" << keyword;
        return -1;
    }

    QFileInfo program(path);

#ifdef RUBY_INIT_STACK
    RUBY_INIT_STACK
#endif

#if RUBY_VERSION < 0x10900
    bool firstTime = (rb_load_path == 0);
#else
    bool firstTime = true;
#endif

    ruby_init();
    ruby_script(QFile::encodeName(program.fileName()));

    // If ruby_init_loadpath() is called more than once, it keeps
    // adding the same standard directories to it.
    if (firstTime) {
        ruby_init_loadpath();
    }

    ruby_incpush(QFile::encodeName(program.path()));

    int state = 0;
    const QByteArray encodedFilePath = QFile::encodeName(program.filePath());
    rb_load_protect(rb_str_new2(encodedFilePath), 0, &state);
    if (state != 0) {
        show_exception_message();
        kWarning() << "Failed to load " << encodedFilePath << keyword << path;
        return -1;
    }

    QByteArray module_name = KRubyPluginFactory::camelize(protocol);
    VALUE slave_module = rb_const_get(rb_cObject, rb_intern(module_name));
    if (slave_module == Qnil) {
        kWarning() << "Constant " << module_name << " not found.";
        return -1;
    }

    slave_class = rb_const_get(slave_module, rb_intern("Main"));
    if (slave_class == Qnil) {
        kWarning() << "Constant Main in module " << module_name << " not found.";
        return -1;
    }

#define WRAP_BYTEARRAY(cvar, rvar) \
    { \
        smokeruby_object *o = alloc_smokeruby_object(true, byteArrayIndex.smoke, byteArrayIndex.index, cvar); \
        const char *class_name = resolve_classname(o); \
        rb_ary_push(rvar, set_obj_info(class_name, o)); \
    }

    VALUE av = rb_ary_new();
    Smoke::ModuleIndex byteArrayIndex = Smoke::findClass("QByteArray");
    WRAP_BYTEARRAY(new QByteArray(protocol), av)
    WRAP_BYTEARRAY(new QByteArray(pool_sock), av)
    WRAP_BYTEARRAY(new QByteArray(app_sock), av)

#undef WRAP_BYTEARRAY

    VALUE slave_value = rb_protect(create_slave_instance, av, &state);
    if (state != 0) {
        show_exception_message();
        kWarning() << "Failed to create instance of slave instance.";
        return -1;
    }

    // Set a global variable '$my_app_foo_bar + <numeric id>' to the value of the new
    // instance of MyApp::FooBar to prevent it being GC'd. Note that it would be
    // better to be able to come up with a way to discover all the plugin instances,
    // and call rb_gc_mark() on them, in the mark phase of GC.
    QByteArray variableBaseName("$");
    variableBaseName += protocol;

    // Handle multiple instances of the same class, and look for an unused global
    // variable
    QByteArray variableName;
    VALUE variable = Qnil;
    int id = 0;
    do {
        id++;
        variableName = variableBaseName + QByteArray::number(id);
        variable = rb_gv_get(variableName);
    } while (variable != Qnil);
    rb_gv_set(variableName, slave_value);

    smokeruby_object *o = 0;
    Data_Get_Struct(slave_value, smokeruby_object, o);
    KIO::SlaveBase *slave = reinterpret_cast<KIO::SlaveBase *>(o->ptr);
    slave->dispatchLoop();
    delete slave;
    return 0;
}

}
