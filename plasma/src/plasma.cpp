/***************************************************************************
                          Korundum.cpp  -  Runtime for KDE services, DCOP etc
                             -------------------
    begin                : Sun Sep 28 2003
    copyright            : (C) 2003-2004 by Richard Dale
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

#include <qobject.h>
#include <qstringlist.h>
#include <qmap.h>
#include <qdatastream.h>

// #include <kdeversion.h>
// #include <kapplication.h>
// #include <kurl.h>
// #include <kconfigskeleton.h>
// #include <kio/global.h>
#include <plasma/applet.h>

#include <ruby.h>

#include <marshall.h>
#include <qtruby.h>
#include <smokeruby.h>
#include <smoke.h>

extern "C" {
extern VALUE qt_internal_module;
extern VALUE set_obj_info(const char * className, smokeruby_object * o);
extern void set_kde_resolve_classname(const char * (*kde_resolve_classname) (Smoke*, int, void *));
extern const char * kde_resolve_classname(Smoke* smoke, int classId, void * ptr);
};

extern TypeHandler KDE_handlers[];
extern void install_handlers(TypeHandler *);
extern Smoke *qt_Smoke;

static VALUE kde_internal_module;
Marshall::HandlerFn getMarshallFn(const SmokeType &type);

extern "C" {
extern Q_DECL_EXPORT void Init_plasma_applet();
extern void Init_qtruby4();
extern Q_DECL_EXPORT KLibFactory * rb_plasma_applet_factory(const char * factory);
extern void set_new_kde(VALUE (*new_kde) (int, VALUE *, VALUE));
extern void set_kde_resolve_classname(const char * (*kde_resolve_classname) (Smoke*, int, void *));
extern const char * kde_resolve_classname(Smoke* smoke, int classId, void * ptr);
extern VALUE new_qt(int argc, VALUE * argv, VALUE klass);
extern VALUE new_qt(int argc, VALUE * argv, VALUE klass);
extern VALUE qt_module;
extern VALUE qt_internal_module;
extern VALUE qt_base_class;
extern VALUE kde_module;
extern VALUE kio_module;
extern VALUE kparts_module;
extern VALUE khtml_module;

static VALUE
new_kde(int argc, VALUE * argv, VALUE klass)
{
	// Note this should really call only new_qt if the instance is a QObject,
	// and otherwise call new_qt().
	VALUE instance = new_qt(argc, argv, klass);	
	return instance;
}

KLibFactory *
rb_plasma_applet_factory(const char * factory)
{
printf("ENTER rb_plasma_applet_factory %s\n", factory);
	VALUE factory_klass = rb_funcall(rb_cObject, rb_intern("const_get"), 1, rb_intern(factory));
	if (factory_klass == Qnil) {
printf("rb_plasma_applet_factory factory class nil\n");
		return 0;
	}

	VALUE factory_value = rb_funcall(factory_klass, rb_intern("new"), 0);
	if (factory_value == Qnil) {
printf("rb_plasma_applet_factory factory value nil\n");
		return 0;
	}

    smokeruby_object * o = value_obj_info(factory_value);
	return (KLibFactory *) o->ptr;
}

void
Init_plasma_applet()
{
	if (qt_internal_module != Qnil) {
		rb_fatal("require 'plasma' must not follow require 'Qt'\n");
		return;
	}

	set_new_kde(new_kde);
	set_kde_resolve_classname(kde_resolve_classname);
		
	// The Qt extension is linked against libsmokeqt.so, but Korundum links against
	// libsmokekde.so only. Specifying both a 'require Qt' and a 'require Korundum',
	// would give a link error (see the rb_fatal() error above).
	// So call the Init_qtruby() initialization function explicitely, not via 'require Qt'
	// (Qt.o is linked into libqtruby.so, as well as the Qt.so extension).
	Init_qtruby4();
    install_handlers(KDE_handlers);
	
    kde_internal_module = rb_define_module_under(kde_module, "Internal");

//	rb_require("KDE/plasma.rb");
}

};
