/***************************************************************************
                          Korundum.cpp  -  Runtime for KDE services, DCOP etc
                             -------------------
    begin                : Sun Sep 28 2003
    copyright            : (C) 2003-2004 by Richard Dale
                           (C) 2008 by Arno Rehn
    email                : Richard_Dale@tipitina.demon.co.uk
                           arno@arnorehn.de
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

#include <QtCore/qdebug.h>
#include <QtCore/qtextcodec.h>
#include <QtCore/qtextstream.h>
#include <QtDBus/qdbusmetatype.h>

#include <soprano/node.h>
#include <soprano/statement.h>
#include <soprano/bindingset.h>

#include <kdeversion.h>
#include <kapplication.h>
#include <kconfigskeleton.h>
#include <kcoreconfigskeleton.h>
#include <kurl.h>
#include <kio/global.h>
#include <kconfigskeleton.h>

#include <smoke.h>

#include <qt/qt_smoke.h>
#include <kde/kde_smoke.h>

#include <qtruby.h>

#include <iostream>

const char* resolve_classname_kde(Smoke* smoke, int classId, void* ptr);

static VALUE getClassList(VALUE /*self*/)
{
    VALUE classList = rb_ary_new();
    for (int i = 1; i < kde_Smoke->numClasses; i++) {
        if (kde_Smoke->classes[i].className && !kde_Smoke->classes[i].external) {
            rb_ary_push(classList, rb_str_new2(kde_Smoke->classes[i].className));
        }
    }
    return classList;
}

/* 
 * These QDBusArgument operators are copied from kdesupport/soprano/server/dbus/dbusoperators.cpp
 */
Q_DECLARE_METATYPE(Soprano::Statement)
Q_DECLARE_METATYPE(Soprano::Node)
Q_DECLARE_METATYPE(Soprano::BindingSet)

QDBusArgument& operator<<( QDBusArgument& arg, const Soprano::Node& node )
{
    arg.beginStructure();
    arg << ( int )node.type() << node.toString() << node.language() << node.dataType().toString();
    arg.endStructure();
    return arg;
}

const QDBusArgument& operator>>( const QDBusArgument& arg, Soprano::Node& node )
{
    arg.beginStructure();
    int type;
    QString value, language, dataTypeUri;
    arg >> type >> value >> language >> dataTypeUri;
    if ( type == Soprano::Node::LiteralNode ) {
        node = Soprano::Node( Soprano::LiteralValue::fromString( value, dataTypeUri ), language );
    }
    else if ( type == Soprano::Node::ResourceNode ) {
        node = Soprano::Node( QUrl( value ) );
    }
    else if ( type == Soprano::Node::BlankNode ) {
        node = Soprano::Node( value );
    }
    else {
        node = Soprano::Node();
    }
    arg.endStructure();
    return arg;
}

QDBusArgument& operator<<( QDBusArgument& arg, const Soprano::Statement& statement )
{
    arg.beginStructure();
    arg << statement.subject() << statement.predicate() << statement.object() << statement.context();
    arg.endStructure();
    return arg;
}

const QDBusArgument& operator>>( const QDBusArgument& arg, Soprano::Statement& statement )
{
    arg.beginStructure();
    Soprano::Node subject, predicate, object, context;
    arg >> subject >> predicate >> object >> context;
    statement = Soprano::Statement( subject, predicate, object, context );
    arg.endStructure();
    return arg;
}

QDBusArgument& operator<<( QDBusArgument& arg, const Soprano::BindingSet& set )
{
    arg.beginStructure();
    arg.beginMap( QVariant::String, qMetaTypeId<Soprano::Node>() );
    QStringList names = set.bindingNames();
    for ( int i = 0; i < names.count(); ++i ) {
        arg.beginMapEntry();
        arg << names[i] << set[ names[i] ];
        arg.endMapEntry();
    }
    arg.endMap();
    arg.endStructure();
    return arg;
}

const QDBusArgument& operator>>( const QDBusArgument& arg, Soprano::BindingSet& set )
{
    arg.beginStructure();
    arg.beginMap();
    while ( !arg.atEnd() ) {
        QString name;
        Soprano::Node val;
        arg.beginMapEntry();
        arg >> name >> val;
        arg.endMapEntry();
        set.insert( name, val );
    }

    arg.endMap();
    arg.endStructure();
    return arg;
}

extern "C" {
VALUE kconfigskeleton_class;
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

static void classCreated(const char* package, VALUE /*module*/, VALUE klass)
{
	QString packageName(package);
	if (packageName == "KDE::ConfigSkeleton") {
		kconfigskeleton_class = klass;
		rb_define_method(klass, "addItem", (VALUE (*) (...)) config_additem, -1);
	}
}

typedef VALUE (*NewConfigSkeletonItemFn)(int, VALUE *, VALUE);

/*
static VALUE
new_kconfigskeleton_itemintlist(int argc, VALUE * argv, VALUE self)
{
}

static VALUE
new_kconfigskeleton_itemlonglong(int argc, VALUE * argv, VALUE self)
{
}

static VALUE
new_kconfigskeleton_itempathlist(int argc, VALUE * argv, VALUE self)
{
}
*/

template <class SkeletonItem, const char *SkeletonItemSTR >
static VALUE new_kconfigskeleton_string_item(int argc, VALUE * argv, VALUE self)
{
	if (TYPE(self) == T_DATA) {
		// If a ruby block was passed then run that now
		if (rb_block_given_p()) {
			rb_funcall(qt_internal_module, rb_intern("run_initializer_block"), 2, self, rb_block_proc());
		}

		return self;
	}

	QString * reference = new QString(QString::fromLatin1(StringValuePtr(argv[2])));
	SkeletonItem * skeletonItem = 0;

	if (argc == 3) {
		skeletonItem = new SkeletonItem(	QString::fromLatin1(StringValuePtr(argv[0])), 
											QString::fromLatin1(StringValuePtr(argv[1])),
											*reference );
	} else if (argc == 4) {
		skeletonItem = new SkeletonItem(	QString::fromLatin1(StringValuePtr(argv[0])), 
											QString::fromLatin1(StringValuePtr(argv[1])),
											*reference,
											QString::fromLatin1(StringValuePtr(argv[3])) );
	} else {
		return rb_call_super(argc, argv);
	}

	Smoke::ModuleIndex mi = qt_Smoke->findClass(SkeletonItemSTR);
	smokeruby_object  * o = alloc_smokeruby_object(	true, 
													mi.smoke, 
													mi.index, 
													skeletonItem );

	VALUE klass = rb_funcall(self, rb_intern("class"), 0);
	VALUE result = Data_Wrap_Struct(klass, smokeruby_mark, smokeruby_free, o);
	mapObject(result, result);
	rb_throw("newqt", result);
	/*NOTREACHED*/
	return self;
}

#define DEF_SKELETON_STRING_ITEM_CONSTRUCTOR(Method, SkeletonItem) namespace { char Method##STR[] = #SkeletonItem; }  \
        static NewConfigSkeletonItemFn new_kconfigskeleton_##Method = new_kconfigskeleton_string_item<SkeletonItem, Method##STR >;

DEF_SKELETON_STRING_ITEM_CONSTRUCTOR( itemstring, KCoreConfigSkeleton::ItemString )
DEF_SKELETON_STRING_ITEM_CONSTRUCTOR( itempassword, KCoreConfigSkeleton::ItemPassword )
DEF_SKELETON_STRING_ITEM_CONSTRUCTOR( itempath, KCoreConfigSkeleton::ItemPath )

template <class SkeletonItem, const char *SkeletonItemSTR >
static VALUE new_kconfigskeleton_stringlist_item(int argc, VALUE * argv, VALUE self)
{
	if (TYPE(self) == T_DATA) {
		// If a ruby block was passed then run that now
		if (rb_block_given_p()) {
			rb_funcall(qt_internal_module, rb_intern("run_initializer_block"), 2, self, rb_block_proc());
		}

		return self;
	}

	QStringList * reference = new QStringList();
    VALUE list = argv[2];
	int count = RARRAY(list)->len;
	for(int i = 0; i < count; i++) {
		VALUE item = rb_ary_entry(list, i);
		reference->append(QString::fromLatin1(StringValuePtr(item)));
	}

	SkeletonItem * skeletonItem = 0;

	if (argc == 3) {
		skeletonItem = new SkeletonItem(	QString::fromLatin1(StringValuePtr(argv[0])), 
											QString::fromLatin1(StringValuePtr(argv[1])),
											*reference );
	} else if (argc == 4) {
		QStringList defaultList;
		list = argv[3];
		int count = RARRAY(list)->len;
		for(int i = 0; i < count; i++) {
			VALUE item = rb_ary_entry(list, i);
			defaultList.append(QString::fromLatin1(StringValuePtr(item)));
		}

		skeletonItem = new SkeletonItem(	QString::fromLatin1(StringValuePtr(argv[0])), 
											QString::fromLatin1(StringValuePtr(argv[1])),
											*reference,
											defaultList );
	} else {
		return rb_call_super(argc, argv);
	}

	Smoke::ModuleIndex mi = qt_Smoke->findClass(SkeletonItemSTR);
	smokeruby_object  * o = alloc_smokeruby_object(	true, 
													mi.smoke, 
													mi.index, 
													skeletonItem );

	VALUE klass = rb_funcall(self, rb_intern("class"), 0);
	VALUE result = Data_Wrap_Struct(klass, smokeruby_mark, smokeruby_free, o);
	mapObject(result, result);
	rb_throw("newqt", result);
	/*NOTREACHED*/
	return self;
}

#define DEF_SKELETON_STRINGLIST_ITEM_CONSTRUCTOR(Method, SkeletonItem) namespace { char Method##STR[] = #SkeletonItem; }  \
        static NewConfigSkeletonItemFn new_kconfigskeleton_##Method = new_kconfigskeleton_stringlist_item<SkeletonItem, Method##STR >;

DEF_SKELETON_STRINGLIST_ITEM_CONSTRUCTOR( itemstringlist, KCoreConfigSkeleton::ItemStringList )
DEF_SKELETON_STRINGLIST_ITEM_CONSTRUCTOR( itempathlist, KCoreConfigSkeleton::ItemPathList )

/*
static VALUE
new_kconfigskeleton_itemulonglong(int argc, VALUE * argv, VALUE self)
{
}

static VALUE
new_kconfigskeleton_itemurllist(int argc, VALUE * argv, VALUE self)
{
}
*/

template <class T> T ruby_to_primitive(VALUE);
template <class T> VALUE primitive_to_ruby(T);

template <class SkeletonItem, class Item, const char *SkeletonItemSTR >
static VALUE new_kconfigskeleton_primitive_item(int argc, VALUE * argv, VALUE self)
{
	if (TYPE(self) == T_DATA) {
		// If a ruby block was passed then run that now
		if (rb_block_given_p()) {
			rb_funcall(qt_internal_module, rb_intern("run_initializer_block"), 2, self, rb_block_proc());
		}

		return self;
	}

	Item value = ruby_to_primitive<Item>(argv[2]);
	Item * reference = new Item(value);
	SkeletonItem * skeletonItem = 0;

	if (argc == 3) {
		skeletonItem = new SkeletonItem(	QString::fromLatin1(StringValuePtr(argv[0])), 
											QString::fromLatin1(StringValuePtr(argv[1])),
											*reference );
	} else if (argc == 4) {
		skeletonItem = new SkeletonItem(	QString::fromLatin1(StringValuePtr(argv[0])), 
											QString::fromLatin1(StringValuePtr(argv[1])),
											*reference,
											ruby_to_primitive<Item>(argv[3]) );
	} else {
		return rb_call_super(argc, argv);
	}

	Smoke::ModuleIndex mi = qt_Smoke->findClass(SkeletonItemSTR);
	smokeruby_object  * o = alloc_smokeruby_object(	true, 
													mi.smoke, 
													mi.index, 
													skeletonItem );

	VALUE klass = rb_funcall(self, rb_intern("class"), 0);
	VALUE result = Data_Wrap_Struct(klass, smokeruby_mark, smokeruby_free, o);
	mapObject(result, result);
	rb_throw("newqt", result);
}

#define DEF_SKELETON_PRIMITIVE_ITEM_CONSTRUCTOR(Method, SkeletonItem, Item) namespace { char Method##STR[] = #SkeletonItem; }  \
        static NewConfigSkeletonItemFn new_kconfigskeleton_##Method = new_kconfigskeleton_primitive_item<SkeletonItem, Item, Method##STR >;

DEF_SKELETON_PRIMITIVE_ITEM_CONSTRUCTOR( itembool, KCoreConfigSkeleton::ItemBool, bool )
DEF_SKELETON_PRIMITIVE_ITEM_CONSTRUCTOR( itemdouble, KCoreConfigSkeleton::ItemDouble, double )
DEF_SKELETON_PRIMITIVE_ITEM_CONSTRUCTOR( itemint, KCoreConfigSkeleton::ItemInt, int )
DEF_SKELETON_PRIMITIVE_ITEM_CONSTRUCTOR( itemuint, KCoreConfigSkeleton::ItemUInt, uint )

template <class SkeletonItem, class Item, const char *SkeletonItemSTR >
static VALUE new_kconfigskeleton_item(int argc, VALUE * argv, VALUE self)
{
	if (TYPE(self) == T_DATA) {
		// If a ruby block was passed then run that now
		if (rb_block_given_p()) {
			rb_funcall(qt_internal_module, rb_intern("run_initializer_block"), 2, self, rb_block_proc());
		}

		return self;
	}

	smokeruby_object *argv2 = value_obj_info(argv[2]);
    Item * arg2 = new Item(*((Item*) argv2->ptr));
	SkeletonItem * skeletonItem = 0;

	if (argc == 3) {
		skeletonItem = new SkeletonItem(	QString::fromLatin1(StringValuePtr(argv[0])), 
											QString::fromLatin1(StringValuePtr(argv[1])),
											*arg2,
											Item() );
	} else if (argc == 4) {
		smokeruby_object *argv3 = value_obj_info(argv[3]);
		skeletonItem = new SkeletonItem(	QString::fromLatin1(StringValuePtr(argv[0])), 
											QString::fromLatin1(StringValuePtr(argv[1])),
											*arg2,
											*((Item*) argv3->ptr) );
	} else {
		return rb_call_super(argc, argv);
	}

	smokeruby_object  * o = alloc_smokeruby_object(	true, 
													argv2->smoke, 
													argv2->smoke->idClass(SkeletonItemSTR).index, 
													skeletonItem );

	VALUE klass = rb_funcall(self, rb_intern("class"), 0);
	VALUE result = Data_Wrap_Struct(klass, smokeruby_mark, smokeruby_free, o);
	mapObject(result, result);
	rb_throw("newqt", result);
	/*NOTREACHED*/
	return self;
}

#define DEF_SKELETON_ITEM_CONSTRUCTOR(Method, SkeletonItem, Item) namespace { char Method##STR[] = #SkeletonItem; }  \
        static NewConfigSkeletonItemFn new_kconfigskeleton_##Method = new_kconfigskeleton_item<SkeletonItem, Item, Method##STR>;

DEF_SKELETON_ITEM_CONSTRUCTOR( itemurl, KCoreConfigSkeleton::ItemUrl, KUrl )
DEF_SKELETON_ITEM_CONSTRUCTOR( itemcolor, KConfigSkeleton::ItemColor, QColor )
DEF_SKELETON_ITEM_CONSTRUCTOR( itemfont, KConfigSkeleton::ItemFont, QFont )
DEF_SKELETON_ITEM_CONSTRUCTOR( itemdatetime, KCoreConfigSkeleton::ItemDateTime, QDateTime )
DEF_SKELETON_ITEM_CONSTRUCTOR( itempoint, KCoreConfigSkeleton::ItemPoint, QPoint )
DEF_SKELETON_ITEM_CONSTRUCTOR( itemrect, KCoreConfigSkeleton::ItemRect, QRect )
DEF_SKELETON_ITEM_CONSTRUCTOR( itemsize, KCoreConfigSkeleton::ItemSize, QSize )
DEF_SKELETON_ITEM_CONSTRUCTOR( itemproperty, KCoreConfigSkeleton::ItemProperty, QVariant )

// Note that the KDE::ConfigSkeletonItem constructors have to be special cased as they 
// expect to 'own' a reference to an item which is held outside themselves. So these
// constructors create the item on the heap outside the Ruby runtime. In Ruby the contents
// of KDE::ConfigSkeletonItems can only be accessed via property() and setProperty().
//
static void
init_kconfigskeletonitem_classes()
{
	VALUE klass;

	klass = rb_define_class_under(kconfigskeleton_class, "ItemBool", qt_base_class);
    rb_define_method(klass, "initialize", (VALUE (*) (...)) new_kconfigskeleton_itembool, -1);

	klass = rb_define_class_under(kconfigskeleton_class, "ItemColor", qt_base_class);
    rb_define_method(klass, "initialize", (VALUE (*) (...)) new_kconfigskeleton_itemcolor, -1);

	klass = rb_define_class_under(kconfigskeleton_class, "ItemFont", qt_base_class);
    rb_define_method(klass, "initialize", (VALUE (*) (...)) new_kconfigskeleton_itemfont, -1);

	klass = rb_define_class_under(kconfigskeleton_class, "ItemDateTime", qt_base_class);
    rb_define_method(klass, "initialize", (VALUE (*) (...)) new_kconfigskeleton_itemdatetime, -1);

	klass = rb_define_class_under(kconfigskeleton_class, "ItemDouble", qt_base_class);
    rb_define_method(klass, "initialize", (VALUE (*) (...)) new_kconfigskeleton_itemdouble, -1);

	klass = rb_define_class_under(kconfigskeleton_class, "ItemEnum", qt_base_class);
//    rb_define_method(klass, "initialize", (VALUE (*) (...)) new_kconfigskeleton_itemenum, -1);

	klass = rb_define_class_under(kconfigskeleton_class, "ItemInt", qt_base_class);
    rb_define_method(klass, "initialize", (VALUE (*) (...)) new_kconfigskeleton_itemint, -1);

	klass = rb_define_class_under(kconfigskeleton_class, "ItemIntList", qt_base_class);
	klass = rb_define_class_under(kconfigskeleton_class, "ItemLongLong", qt_base_class);

	klass = rb_define_class_under(kconfigskeleton_class, "ItemPassword", qt_base_class);
    rb_define_method(klass, "initialize", (VALUE (*) (...)) new_kconfigskeleton_itempassword, -1);

	klass = rb_define_class_under(kconfigskeleton_class, "ItemPath", qt_base_class);
    rb_define_method(klass, "initialize", (VALUE (*) (...)) new_kconfigskeleton_itempath, -1);

	klass = rb_define_class_under(kconfigskeleton_class, "ItemPathList", qt_base_class);
    rb_define_method(klass, "initialize", (VALUE (*) (...)) new_kconfigskeleton_itempathlist, -1);

	klass = rb_define_class_under(kconfigskeleton_class, "ItemPoint", qt_base_class);
    rb_define_method(klass, "initialize", (VALUE (*) (...)) new_kconfigskeleton_itempoint, -1);

	klass = rb_define_class_under(kconfigskeleton_class, "ItemProperty", qt_base_class);
    rb_define_method(klass, "initialize", (VALUE (*) (...)) new_kconfigskeleton_itemproperty, -1);

	klass = rb_define_class_under(kconfigskeleton_class, "ItemRect", qt_base_class);
    rb_define_method(klass, "initialize", (VALUE (*) (...)) new_kconfigskeleton_itemrect, -1);

	klass = rb_define_class_under(kconfigskeleton_class, "ItemSize", qt_base_class);
    rb_define_method(klass, "initialize", (VALUE (*) (...)) new_kconfigskeleton_itemsize, -1);

	klass = rb_define_class_under(kconfigskeleton_class, "ItemString", qt_base_class);
    rb_define_method(klass, "initialize", (VALUE (*) (...)) new_kconfigskeleton_itemstring, -1);

	klass = rb_define_class_under(kconfigskeleton_class, "ItemStringList", qt_base_class);
    rb_define_method(klass, "initialize", (VALUE (*) (...)) new_kconfigskeleton_itemstringlist, -1);

	klass = rb_define_class_under(kconfigskeleton_class, "ItemUInt", qt_base_class);
    rb_define_method(klass, "initialize", (VALUE (*) (...)) new_kconfigskeleton_itemuint, -1);

	klass = rb_define_class_under(kconfigskeleton_class, "ItemULongLong", qt_base_class);

	klass = rb_define_class_under(kconfigskeleton_class, "ItemUrl", qt_base_class);
    rb_define_method(klass, "initialize", (VALUE (*) (...)) new_kconfigskeleton_itemurl, -1);

	klass = rb_define_class_under(kconfigskeleton_class, "ItemUrlList", qt_base_class);
}


extern TypeHandler KDE_handlers[];

extern "C" {

VALUE kde_module;
VALUE kde_internal_module;
VALUE kparts_module;
VALUE kns_module;
VALUE kio_module;
VALUE dom_module;
VALUE kontact_module;
VALUE ktexteditor_module;
VALUE kate_module;
VALUE kmediaplayer_module;
VALUE koffice_module;
VALUE kwallet_module;
VALUE safesite_module;
VALUE sonnet_module;
VALUE soprano_module;
VALUE nepomuk_module;

static VALUE kde_module_method_missing(int argc, VALUE * argv, VALUE klass)
{
    return class_method_missing(argc, argv, klass);
}

static void
setup_kde_modules()
{
	kde_module = rb_define_module("KDE");
    rb_define_singleton_method(kde_module, "method_missing", (VALUE (*) (...)) kde_module_method_missing, -1);
    rb_define_singleton_method(kde_module, "const_missing", (VALUE (*) (...)) kde_module_method_missing, -1);

	kparts_module = rb_define_module("KParts");
    rb_define_singleton_method(kparts_module, "method_missing", (VALUE (*) (...)) kde_module_method_missing, -1);
    rb_define_singleton_method(kparts_module, "const_missing", (VALUE (*) (...)) kde_module_method_missing, -1);

	kns_module = rb_define_module("KNS");
	rb_define_singleton_method(kns_module, "method_missing", (VALUE (*) (...)) kde_module_method_missing, -1);
	rb_define_singleton_method(kns_module, "const_missing", (VALUE (*) (...)) kde_module_method_missing, -1);
	
	kio_module = rb_define_module("KIO");
	rb_define_singleton_method(kio_module, "method_missing", (VALUE (*) (...)) kde_module_method_missing, -1);
	rb_define_singleton_method(kio_module, "const_missing", (VALUE (*) (...)) kde_module_method_missing, -1);

	dom_module = rb_define_module("DOM");
    rb_define_singleton_method(dom_module, "method_missing", (VALUE (*) (...)) kde_module_method_missing, -1);
    rb_define_singleton_method(dom_module, "const_missing", (VALUE (*) (...)) kde_module_method_missing, -1);

	kontact_module = rb_define_module("Kontact");
    rb_define_singleton_method(kontact_module, "method_missing", (VALUE (*) (...)) kde_module_method_missing, -1);
    rb_define_singleton_method(kontact_module, "const_missing", (VALUE (*) (...)) kde_module_method_missing, -1);

	ktexteditor_module = rb_define_module("KTextEditor");
    rb_define_singleton_method(ktexteditor_module, "method_missing", (VALUE (*) (...)) kde_module_method_missing, -1);
    rb_define_singleton_method(ktexteditor_module, "const_missing", (VALUE (*) (...)) kde_module_method_missing, -1);

	kate_module = rb_define_module("Kate");
    rb_define_singleton_method(kate_module, "method_missing", (VALUE (*) (...)) kde_module_method_missing, -1);
    rb_define_singleton_method(kate_module, "const_missing", (VALUE (*) (...)) kde_module_method_missing, -1);

	kmediaplayer_module = rb_define_module("KMediaPlayer");
	rb_define_singleton_method(kmediaplayer_module, "method_missing", (VALUE (*) (...)) kde_module_method_missing, -1);
	rb_define_singleton_method(kmediaplayer_module, "const_missing", (VALUE (*) (...)) kde_module_method_missing, -1);

	koffice_module = rb_define_module("Ko");
	rb_define_singleton_method(koffice_module, "method_missing", (VALUE (*) (...)) kde_module_method_missing, -1);
	rb_define_singleton_method(koffice_module, "const_missing", (VALUE (*) (...)) kde_module_method_missing, -1);

	kwallet_module = rb_define_module("KWallet");
	rb_define_singleton_method(kwallet_module, "method_missing", (VALUE (*) (...)) kde_module_method_missing, -1);
	rb_define_singleton_method(kwallet_module, "const_missing", (VALUE (*) (...)) kde_module_method_missing, -1);

	safesite_module = rb_define_module("SafeSite");
	rb_define_singleton_method(safesite_module, "method_missing", (VALUE (*) (...)) kde_module_method_missing, -1);
	rb_define_singleton_method(safesite_module, "const_missing", (VALUE (*) (...)) kde_module_method_missing, -1);

	sonnet_module = rb_define_module("Sonnet");
	rb_define_singleton_method(sonnet_module, "method_missing", (VALUE (*) (...)) kde_module_method_missing, -1);
	rb_define_singleton_method(sonnet_module, "const_missing", (VALUE (*) (...)) kde_module_method_missing, -1);

	soprano_module = rb_define_module("Soprano");
	rb_define_singleton_method(soprano_module, "method_missing", (VALUE (*) (...)) kde_module_method_missing, -1);
	rb_define_singleton_method(soprano_module, "const_missing", (VALUE (*) (...)) kde_module_method_missing, -1);

	nepomuk_module = rb_define_module("Nepomuk");
	rb_define_singleton_method(nepomuk_module, "method_missing", (VALUE (*) (...)) kde_module_method_missing, -1);
	rb_define_singleton_method(nepomuk_module, "const_missing", (VALUE (*) (...)) kde_module_method_missing, -1);
}

Q_DECL_EXPORT void
Init_korundum4()
{
    rb_require("Qt");    // need to initialize the core runtime first
    init_kde_Smoke();

    kde_Smoke->binding = new QtRubySmokeBinding(kde_Smoke);

    smokeList << kde_Smoke;

    QtRubyModule module = { "KDE", resolve_classname_kde, classCreated };
    qtruby_modules[kde_Smoke] = module;

    install_handlers(KDE_handlers);

    setup_kde_modules();
    kde_internal_module = rb_define_module_under(kde_module, "Internal");

    rb_define_singleton_method(kde_internal_module, "getClassList", (VALUE (*) (...)) getClassList, 0);

	(void) qDBusRegisterMetaType<Soprano::Statement>();
	(void) qDBusRegisterMetaType<Soprano::Node>();
	(void) qDBusRegisterMetaType<Soprano::BindingSet>();
	(void) qRegisterMetaType<KUrl>();

    rb_require("KDE/korundum4.rb");
    rb_funcall(kde_internal_module, rb_intern("init_all_classes"), 0);
    init_kconfigskeletonitem_classes();
}

}
