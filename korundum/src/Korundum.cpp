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

#include <kdeversion.h>
#include <kapplication.h>
#include <kurl.h>
#include <kconfigskeleton.h>
#include <kio/global.h>

#include <ruby.h>

#include <marshall.h>
#include <qtruby.h>
#include <smokeruby.h>
#include <smoke.h>

extern "C" {
extern VALUE qt_internal_module;
extern VALUE kconfigskeleton_class;
extern VALUE kconfigskeleton_itemenum_choice_class;
extern VALUE kio_udsatom_class;
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
extern Q_DECL_EXPORT void Init_korundum4();
extern void Init_qtruby4();
extern void set_new_kde(VALUE (*new_kde) (int, VALUE *, VALUE));
extern void set_kconfigskeletonitem_immutable(VALUE (*kconfigskeletonitem_immutable) (VALUE));
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


static VALUE
kconfigskeletonitem_immutable(VALUE self)
{
	smokeruby_object *o = value_obj_info(self);
	KConfigSkeletonItem * item = (KConfigSkeletonItem *) o->ptr;
	return item->isImmutable() ? Qtrue : Qfalse;
}

static VALUE
config_additem(int argc, VALUE * argv, VALUE self)
{
	smokeruby_object *o = value_obj_info(self);
	KConfigSkeleton * config = (KConfigSkeleton *) o->ptr;
	
	if (argc < 1 || argc > 2) {
		rb_raise(rb_eArgError, "wrong number of arguments(%d for 2)\n", argc);
	}
	
	if (TYPE(argv[0]) != T_DATA) {
		rb_raise(rb_eArgError, "wrong argument type, expected KDE::ConfigSkeletonItem\n", argc);
	}
	
	smokeruby_object *c = value_obj_info(argv[0]);
	KConfigSkeletonItem * item = (KConfigSkeletonItem *) c->ptr;
	
	if (argc == 1) {
		config->addItem(item);
	} else {
		config->addItem(item, QString(StringValuePtr(argv[1])));
	}
	
	return self;
}

static VALUE
choice_name(VALUE self)
{
	smokeruby_object *o = value_obj_info(self);
	KConfigSkeleton::ItemEnum::Choice * choice = (KConfigSkeleton::ItemEnum::Choice *) o->ptr;
	return rb_str_new2(choice->name.latin1());
}

static VALUE
set_choice_name(VALUE self, VALUE name)
{
	smokeruby_object *o = value_obj_info(self);
	KConfigSkeleton::ItemEnum::Choice * choice = (KConfigSkeleton::ItemEnum::Choice *) o->ptr;
	choice->name = StringValuePtr(name);
	return self;
}

static VALUE
choice_label(VALUE self)
{
	smokeruby_object *o = value_obj_info(self);
	KConfigSkeleton::ItemEnum::Choice * choice = (KConfigSkeleton::ItemEnum::Choice *) o->ptr;
	return rb_str_new2(choice->label.latin1());
}

static VALUE
set_choice_label(VALUE self, VALUE label)
{
	smokeruby_object *o = value_obj_info(self);
	KConfigSkeleton::ItemEnum::Choice * choice = (KConfigSkeleton::ItemEnum::Choice *) o->ptr;
	choice->label = StringValuePtr(label);
	return self;
}

static VALUE
choice_whatsthis(VALUE self)
{
	smokeruby_object *o = value_obj_info(self);
	KConfigSkeleton::ItemEnum::Choice * choice = (KConfigSkeleton::ItemEnum::Choice *) o->ptr;
	return rb_str_new2(choice->whatsThis.latin1());
}

static VALUE
set_choice_whatsthis(VALUE self, VALUE whatsthis)
{
	smokeruby_object *o = value_obj_info(self);
	KConfigSkeleton::ItemEnum::Choice * choice = (KConfigSkeleton::ItemEnum::Choice *) o->ptr;
	choice->whatsThis = StringValuePtr(whatsthis);
	return self;
}

/*
static VALUE
udsatom_str(VALUE self)
{
	smokeruby_object *o = value_obj_info(self);
	KIO::UDSAtom * udsatom = (KIO::UDSAtom *) o->ptr;
	if (udsatom->m_str.isNull()) {
		return Qnil;
	} else {
		return rb_str_new2(udsatom->m_str.latin1());
	}
}


static VALUE
udsatom_long(VALUE self)
{
	smokeruby_object *o = value_obj_info(self);
	KIO::UDSAtom * udsatom = (KIO::UDSAtom *) o->ptr;
	return LL2NUM(udsatom->m_long);
}

static VALUE
udsatom_uds(VALUE self)
{
	smokeruby_object *o = value_obj_info(self);
	KIO::UDSAtom * udsatom = (KIO::UDSAtom *) o->ptr;
	return UINT2NUM(udsatom->m_uds);
}

static VALUE
set_udsatom_str(VALUE self, VALUE str)
{
	smokeruby_object *o = value_obj_info(self);
	KIO::UDSAtom * udsatom = (KIO::UDSAtom *) o->ptr;
	udsatom->m_str = QString(StringValuePtr(str));
	return self;
}

static VALUE
set_udsatom_long(VALUE self, VALUE m_long)
{
	smokeruby_object *o = value_obj_info(self);
	KIO::UDSAtom * udsatom = (KIO::UDSAtom *) o->ptr;
	udsatom->m_long = NUM2LL(m_long);
	return self;
}

static VALUE
set_udsatom_uds(VALUE self, VALUE uds)
{
	smokeruby_object *o = value_obj_info(self);
	KIO::UDSAtom * udsatom = (KIO::UDSAtom *) o->ptr;
	udsatom->m_uds = NUM2UINT(uds);
	return self;
}
*/

void
Init_korundum4()
{
	if (qt_internal_module != Qnil) {
		rb_fatal("require 'Korundum' must not follow require 'Qt'\n");
		return;
	}

	set_new_kde(new_kde);
	set_kconfigskeletonitem_immutable(kconfigskeletonitem_immutable);
	set_kde_resolve_classname(kde_resolve_classname);
		
	// The Qt extension is linked against libsmokeqt.so, but Korundum links against
	// libsmokekde.so only. Specifying both a 'require Qt' and a 'require Korundum',
	// would give a link error (see the rb_fatal() error above).
	// So call the Init_qtruby() initialization function explicitely, not via 'require Qt'
	// (Qt.o is linked into libqtruby.so, as well as the Qt.so extension).
	Init_qtruby4();
    install_handlers(KDE_handlers);
	
    kde_internal_module = rb_define_module_under(kde_module, "Internal");
	

	rb_define_method(kconfigskeleton_class, "addItem", (VALUE (*) (...)) config_additem, -1);
	
	rb_define_method(kconfigskeleton_itemenum_choice_class, "name", (VALUE (*) (...)) choice_name, 0);
	rb_define_method(kconfigskeleton_itemenum_choice_class, "label", (VALUE (*) (...)) choice_label, 0);
	rb_define_method(kconfigskeleton_itemenum_choice_class, "whatsThis", (VALUE (*) (...)) choice_whatsthis, 0);
	rb_define_method(kconfigskeleton_itemenum_choice_class, "name=", (VALUE (*) (...)) set_choice_name, 1);
	rb_define_method(kconfigskeleton_itemenum_choice_class, "label=", (VALUE (*) (...)) set_choice_label, 1);
	rb_define_method(kconfigskeleton_itemenum_choice_class, "whatsThis=", (VALUE (*) (...)) set_choice_whatsthis, 1);

/*	
	rb_define_method(kio_udsatom_class, "m_str", (VALUE (*) (...)) udsatom_str, 0);
	rb_define_method(kio_udsatom_class, "m_long", (VALUE (*) (...)) udsatom_long, 0);
	rb_define_method(kio_udsatom_class, "m_uds", (VALUE (*) (...)) udsatom_uds, 0);
	rb_define_method(kio_udsatom_class, "m_str=", (VALUE (*) (...)) set_udsatom_str, 1);
	rb_define_method(kio_udsatom_class, "m_long=", (VALUE (*) (...)) set_udsatom_long, 1);
	rb_define_method(kio_udsatom_class, "m_uds=", (VALUE (*) (...)) set_udsatom_uds, 1);
*/	
	rb_require("KDE/korundum4.rb");
}

};
