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

#include <ruby.h>

#include <marshall.h>
#include <qtruby.h>
#include <smokeruby.h>
#include <smoke.h>

extern "C" {
extern VALUE qt_internal_module;
extern VALUE qt_base_class;
extern VALUE kconfiggroup_class;
extern VALUE kcoreconfigskeleton_class;
extern VALUE kconfigskeleton_class;

extern VALUE set_obj_info(const char * className, smokeruby_object * o);
extern void set_kde_resolve_classname(const char * (*kde_resolve_classname) (Smoke*, int, void *));
extern const char * kde_resolve_classname(Smoke* smoke, int classId, void * ptr);
}

extern TypeHandler KDE_handlers[];
extern void install_handlers(TypeHandler *);
extern Smoke *qt_Smoke;

static VALUE kde_internal_module;
Marshall::HandlerFn getMarshallFn(const SmokeType &type);

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
extern Q_DECL_EXPORT void Init_korundum4();
extern void Init_qtruby4();
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

	int ix = qt_Smoke->idClass(SkeletonItemSTR);
	const char * className = qt_Smoke->binding->className(ix);

	smokeruby_object  * result = alloc_smokeruby_object(	true, 
															qt_Smoke, 
															qt_Smoke->idClass(SkeletonItemSTR), 
															skeletonItem );

	rb_throw("newqt", set_obj_info(className, result));
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

	int ix = qt_Smoke->idClass(SkeletonItemSTR);
	const char * className = qt_Smoke->binding->className(ix);

	smokeruby_object  * result = alloc_smokeruby_object(	true, 
															qt_Smoke, 
															qt_Smoke->idClass(SkeletonItemSTR), 
															skeletonItem );

	rb_throw("newqt", set_obj_info(className, result));
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

	int ix = qt_Smoke->idClass(SkeletonItemSTR);
	const char * className = qt_Smoke->binding->className(ix);

	smokeruby_object  * result = alloc_smokeruby_object(	true, 
															qt_Smoke, 
															qt_Smoke->idClass(SkeletonItemSTR), 
															skeletonItem );

	rb_throw("newqt", set_obj_info(className, result));
	/*NOTREACHED*/
	return self;
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

	int ix = argv2->smoke->idClass(SkeletonItemSTR);
	const char * className = argv2->smoke->binding->className(ix);

	smokeruby_object  * result = alloc_smokeruby_object(	true, 
															argv2->smoke, 
															argv2->smoke->idClass(SkeletonItemSTR), 
															skeletonItem );

	rb_throw("newqt", set_obj_info(className, result));
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

extern "C" {

static VALUE
new_kde(int argc, VALUE * argv, VALUE klass)
{
	// Note this should really call only new_qt if the instance is a QObject,
	// and otherwise call new_qt().
	VALUE instance = new_qt(argc, argv, klass);	
	return instance;
}

void
Init_korundum4()
{
	if (qt_internal_module != Qnil) {
		rb_fatal("require 'Korundum' must not follow require 'Qt'\n");
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
	init_kconfigskeletonitem_classes();

	(void) qDBusRegisterMetaType<Soprano::Statement>();
	(void) qDBusRegisterMetaType<Soprano::Node>();
	(void) qDBusRegisterMetaType<Soprano::BindingSet>();
	(void) qRegisterMetaType<KUrl>();

	rb_require("KDE/korundum4.rb");
	rb_require("KDE/soprano.rb");
}

}
