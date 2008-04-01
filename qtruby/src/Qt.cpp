/***************************************************************************
                          Qt.cpp  -  description
                             -------------------
    begin                : Fri Jul 4 2003
    copyright            : (C) 2003-2006 by Richard Dale
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

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <stdio.h>
#include <stdarg.h>

#include <QtCore/qabstractitemmodel.h>			
#include <QtCore/qglobal.h>
#include <QtCore/qhash.h>
#include <QtCore/qline.h>			
#include <QtCore/qmetaobject.h>
#include <QtCore/qobject.h>
#include <QtCore/qrect.h>			
#include <QtCore/qregexp.h>
#include <QtCore/qstring.h>
#include <QtCore/qvariant.h>
#include <QtGui/qapplication.h>
#include <QtGui/qbitmap.h>			
#include <QtGui/qcolor.h>			
#include <QtGui/qcursor.h>
#include <QtGui/qfont.h>			
#include <QtGui/qicon.h>			
#include <QtGui/qitemselectionmodel.h>
#include <QtGui/qpalette.h>			
#include <QtGui/qpen.h>			
#include <QtGui/qpixmap.h>			
#include <QtGui/qpolygon.h>			
#include <QtGui/qtextformat.h>			
#include <QtGui/qwidget.h>

#ifdef QT_QTDBUS
#include <QtDBus/qdbusargument.h>
#endif

extern bool qRegisterResourceData(int, const unsigned char *, const unsigned char *, const unsigned char *);
extern bool qUnregisterResourceData(int, const unsigned char *, const unsigned char *, const unsigned char *);

#undef DEBUG
#ifndef __USE_POSIX
#define __USE_POSIX
#endif
#ifndef __USE_XOPEN
#define __USE_XOPEN
#endif
#ifdef _BOOL
#define HAS_BOOL
#endif

#include <ruby.h>

#ifndef QT_VERSION_STR
#define QT_VERSION_STR "Unknown"
#endif

#undef free
#undef malloc

#include "marshall.h"
#include "qtruby.h"
#include "smokeruby.h"
#include "smoke.h"
#include "marshall_types.h"
// #define DEBUG

#define QTRUBY_VERSION "1.4.9"

// Don't use kdemacros.h/KDE_EXPORT here as it needs to be free of KDE dependencies
extern Q_DECL_EXPORT Smoke *qt_Smoke;
extern Q_DECL_EXPORT void init_qt_Smoke();
extern void smokeruby_mark(void * ptr);
extern void smokeruby_free(void * ptr);
extern VALUE qchar_to_s(VALUE self);

#ifdef DEBUG
int do_debug = qtdb_gc;
#else
int do_debug = qtdb_none;
#endif

typedef QHash<void *, VALUE *> PointerMap;
Q_GLOBAL_STATIC(PointerMap, pointer_map)
int object_count = 0;

QHash<QByteArray, Smoke::Index *> methcache;
QHash<QByteArray, Smoke::Index *> classcache;
// Maps from an int id to classname in the form Qt::Widget
QHash<int, QByteArray*> classname;

extern "C" {
VALUE dom_module = Qnil;
VALUE kate_module = Qnil;
VALUE kde_module = Qnil;
VALUE kio_module = Qnil;
VALUE kmediaplayer_module = Qnil;
VALUE kns_module = Qnil;
VALUE koffice_module = Qnil;
VALUE kontact_module = Qnil;
VALUE kparts_module = Qnil;
VALUE ktexteditor_module = Qnil;
VALUE kwallet_module = Qnil;
VALUE plasma_module = Qnil;
VALUE qext_scintilla_module = Qnil;
VALUE qt3_module = Qnil;
VALUE qt_internal_module = Qnil;
VALUE qt_module = Qnil;
VALUE qwt_module = Qnil;
VALUE safesite_module = Qnil;
VALUE sonnet_module = Qnil;
VALUE soprano_module = Qnil;
VALUE nepomuk_module = Qnil;


VALUE kconfiggroup_class = Qnil;
VALUE konsole_part_class = Qnil;
VALUE kwin_class = Qnil;
VALUE qlistmodel_class = Qnil;
VALUE qmetaobject_class = Qnil;
VALUE qtablemodel_class = Qnil;
VALUE qt_base_class = Qnil;
VALUE qtextlayout_class = Qnil;
VALUE qvariant_class = Qnil;

bool application_terminated = false;
}

#define logger logger_backend
void rb_str_catf(VALUE self, const char *format, ...) __attribute__ ((format (printf, 2, 3)));

static VALUE (*_new_kde)(int, VALUE *, VALUE) = 0;

Smoke::Index _current_method = 0;

extern TypeHandler Qt_handlers[];
void install_handlers(TypeHandler *);

smokeruby_object * 
alloc_smokeruby_object(bool allocated, Smoke * smoke, int classId, void * ptr)
{
    smokeruby_object * o = ALLOC(smokeruby_object);
	o->classId = classId;
	o->smoke = smoke;
	o->ptr = ptr;
	o->allocated = allocated;
	return o;
}

void
free_smokeruby_object(smokeruby_object * o)
{
	xfree(o);
	return;
}

smokeruby_object *value_obj_info(VALUE ruby_value) {  // ptr on success, null on fail
	if (TYPE(ruby_value) != T_DATA) {
		return 0;
	}

    smokeruby_object * o = 0;
    Data_Get_Struct(ruby_value, smokeruby_object, o);
    return o;
}

void *value_to_ptr(VALUE ruby_value) {  // ptr on success, null on fail
    smokeruby_object *o = value_obj_info(ruby_value);
    return o;
}

VALUE getPointerObject(void *ptr);

bool isQObject(Smoke *smoke, Smoke::Index classId) {
	if (qstrcmp(smoke->classes[classId].className, "QObject") == 0) {
		return true;
	}

	for (Smoke::Index *p = smoke->inheritanceList + smoke->classes[classId].parents; *p; p++) {
		if (isQObject(smoke, *p)) {
			return true;
		}
    }

    return false;
}

bool isDerivedFrom(Smoke *smoke, Smoke::Index classId, Smoke::Index baseId) {
	if (classId == 0 && baseId == 0) {
		return false;
	}

    if (classId == baseId) {
		return true;
	}

	for(Smoke::Index *p = smoke->inheritanceList + smoke->classes[classId].parents; *p; p++) {
		if (isDerivedFrom(smoke, *p, baseId)) {
			return true;
		}
    }
    return false;
}

bool isDerivedFromByName(Smoke *smoke, const char *className, const char *baseClassName) {
	if (!smoke || !className || !baseClassName) {
		return false;
	}
    Smoke::Index idClass = smoke->idClass(className);
    Smoke::Index idBase = smoke->idClass(baseClassName);
    return isDerivedFrom(smoke, idClass, idBase);
}

VALUE getPointerObject(void *ptr) {
	if (!pointer_map()->contains(ptr)) {
		if (do_debug & qtdb_gc) {
			qWarning("getPointerObject %p -> nil", ptr);
		}
	    return Qnil;
	} else {
		if (do_debug & qtdb_gc) {
			qWarning("getPointerObject %p -> %p", ptr, (void *) *(pointer_map()->operator[](ptr)));
		}
		return *(pointer_map()->operator[](ptr));
	}
}

void unmapPointer(smokeruby_object *o, Smoke::Index classId, void *lastptr) {
	void *ptr = o->smoke->cast(o->ptr, o->classId, classId);
	if (ptr != lastptr) {
		lastptr = ptr;
		if (pointer_map() && pointer_map()->contains(ptr)) {
			VALUE * obj_ptr = pointer_map()->operator[](ptr);
		
			if (do_debug & qtdb_gc) {
				const char *className = o->smoke->classes[o->classId].className;
				qWarning("unmapPointer (%s*)%p -> %p size: %d", className, ptr, obj_ptr, pointer_map()->size() - 1);
			}
	    
			pointer_map()->remove(ptr);
			xfree((void*) obj_ptr);
		}
    }

	for (Smoke::Index *i = o->smoke->inheritanceList + o->smoke->classes[classId].parents; *i; i++) {
		unmapPointer(o, *i, lastptr);
	}
}

// Store pointer in pointer_map hash : "pointer_to_Qt_object" => weak ref to associated Ruby object
// Recurse to store it also as casted to its parent classes.

void mapPointer(VALUE obj, smokeruby_object *o, Smoke::Index classId, void *lastptr) {
    void *ptr = o->smoke->cast(o->ptr, o->classId, classId);
	
    if (ptr != lastptr) {
		lastptr = ptr;
		VALUE * obj_ptr = ALLOC(VALUE);
		memcpy(obj_ptr, &obj, sizeof(VALUE));
		
		if (do_debug & qtdb_gc) {
			const char *className = o->smoke->classes[o->classId].className;
			qWarning("mapPointer (%s*)%p -> %p size: %d", className, ptr, (void*)obj, pointer_map()->size() + 1);
		}
	
		pointer_map()->insert(ptr, obj_ptr);
    }
	
	for (Smoke::Index *i = o->smoke->inheritanceList + o->smoke->classes[classId].parents; *i; i++) {
		mapPointer(obj, o, *i, lastptr);
	}
	
	return;
}


class QtRubySmokeBinding : public SmokeBinding {
public:
    QtRubySmokeBinding(Smoke *s) : SmokeBinding(s) {}

	void deleted(Smoke::Index classId, void *ptr) {
		if (!pointer_map()) {
			return;
		}
		VALUE obj = getPointerObject(ptr);
		smokeruby_object *o = value_obj_info(obj);
		if (do_debug & qtdb_gc) {
	    	qWarning("%p->~%s()", ptr, smoke->className(classId));
		}
		if (!o || !o->ptr) {
	    	return;
		}
		unmapPointer(o, o->classId, 0);
		o->ptr = 0;
	}

	bool callMethod(Smoke::Index method, void *ptr, Smoke::Stack args, bool /*isAbstract*/) {
		VALUE obj = getPointerObject(ptr);
		smokeruby_object *o = value_obj_info(obj);

		if (do_debug & qtdb_virtual) {
			Smoke::Method & meth = smoke->methods[method];
			QByteArray signature(smoke->methodNames[meth.name]);
			signature += "(";

			for (int i = 0; i < meth.numArgs; i++) {
				if (i != 0) signature += ", ";
				signature += smoke->types[smoke->argumentList[meth.args + i]].name;
			}

			signature += ")";
			if (meth.flags & Smoke::mf_const) {
				signature += " const";
			}

			qWarning(	"virtual %p->%s::%s called", 
						ptr,
						smoke->classes[smoke->methods[method].classId].className,
						(const char *) signature );
		}

		if (!o) {
	    	if( do_debug & qtdb_virtual )   // if not in global destruction
				qWarning("Cannot find object for virtual method %p -> %p", ptr, &obj);
	    	return false;
		}

		const char *methodName = smoke->methodNames[smoke->methods[method].name];
		if (qstrncmp(methodName, "operator", sizeof("operator") - 1) == 0) {
			methodName += (sizeof("operator") - 1);
		}

		// If the virtual method hasn't been overriden, just call the C++ one.
		if (rb_respond_to(obj, rb_intern(methodName)) == 0) {
	    	return false;
		}

		VirtualMethodCall c(smoke, method, args, obj, ALLOCA_N(VALUE, smoke->methods[method].numArgs));
		c.next();
		return true;
	}

	char *className(Smoke::Index classId) {
		return (char *) (const char *) *(classname.value((int) classId));
    }
};

/*
	Converts a C++ value returned by a signal invocation to a Ruby 
	reply type
*/
class SignalReturnValue : public Marshall {
    MocArgument *	_replyType;
    Smoke::Stack _stack;
	VALUE * _result;
public:
	SignalReturnValue(void ** o, VALUE * result, MocArgument * replyType) 
	{
		_result = result;
		_replyType = replyType;
		_stack = new Smoke::StackItem[1];
		smokeStackFromQtStack(_stack, o, 1, _replyType);
		Marshall::HandlerFn fn = getMarshallFn(type());
		(*fn)(this);
    }

    SmokeType type() { 
		return _replyType[0].st; 
	}
    Marshall::Action action() { return Marshall::ToVALUE; }
    Smoke::StackItem &item() { return _stack[0]; }
    VALUE * var() {
    	return _result;
    }
	
	void unsupported() 
	{
		rb_raise(rb_eArgError, "Cannot handle '%s' as signal reply-type", type().name());
    }
	Smoke *smoke() { return type().smoke(); }
    
	void next() {}
    
	bool cleanup() { return false; }
	
	~SignalReturnValue() {
		delete[] _stack;
	}
};

/* Note that the SignalReturnValue and EmitSignal classes really belong in
	marshall_types.cpp. However, for unknown reasons they don't link with certain
	versions of gcc. So they were moved here in to work round that bug.
*/
EmitSignal::EmitSignal(QObject *obj, int id, int items, VALUE args, VALUE *sp, VALUE * result) : SigSlotBase(args),
    _obj(obj), _id(id)
{ 
	_sp = sp;
	_result = result;
}

Marshall::Action 
EmitSignal::action() 
{ 
	return Marshall::FromVALUE; 
}

Smoke::StackItem &
EmitSignal::item() 
{ 
	return _stack[_cur]; 
}

const char *
EmitSignal::mytype() 
{ 
	return "signal"; 
}

void 
EmitSignal::emitSignal() 
{
	if (_called) return;
	_called = true;
	void ** o = new void*[_items];
	smokeStackToQtStack(_stack, o + 1, _items - 1, _args + 1);
	_obj->metaObject()->activate(_obj, _id, o);

	if (_args[0].argType != xmoc_void) {
		SignalReturnValue r(o, _result, _args);
	}
	delete[] o;
}

void 
EmitSignal::mainfunction() 
{ 
	emitSignal(); 
}

bool 
EmitSignal::cleanup() 
{ 
	return true; 
}

InvokeNativeSlot::InvokeNativeSlot(QObject *obj, int id, int items, VALUE args, VALUE *sp, VALUE * result) : SigSlotBase(args),
    _obj(obj), _id(id)
{ 
	_sp = sp;
	_result = result;
}

Marshall::Action 
InvokeNativeSlot::action() 
{ 
	return Marshall::FromVALUE; 
}

Smoke::StackItem &
InvokeNativeSlot::item() 
{ 
	return _stack[_cur]; 
}

const char *
InvokeNativeSlot::mytype() 
{ 
	return "slot"; 
}

void 
InvokeNativeSlot::invokeSlot() 
{
	if (_called) return;
	_called = true;
	void ** o = new void*[_items];
	smokeStackToQtStack(_stack, o + 1, _items - 1, _args + 1);
	void * ptr;
	o[0] = &ptr;
	_obj->qt_metacall(QMetaObject::InvokeMetaMethod, _id, o);
	
	if (_args[0].argType != xmoc_void) {
		SignalReturnValue r(o, _result, _args);
	}
	delete[] o;
}

void 
InvokeNativeSlot::mainfunction() 
{ 
	invokeSlot(); 
}

bool 
InvokeNativeSlot::cleanup() 
{ 
	return true; 
}

void rb_str_catf(VALUE self, const char *format, ...) 
{
#define CAT_BUFFER_SIZE 2048
static char p[CAT_BUFFER_SIZE];
	va_list ap;
	va_start(ap, format);
    qvsnprintf(p, CAT_BUFFER_SIZE, format, ap);
	p[CAT_BUFFER_SIZE - 1] = '\0';
	rb_str_cat2(self, p);
	va_end(ap);
}

extern "C" {

static VALUE
qdebug(VALUE klass, VALUE msg)
{
	qDebug("%s", StringValuePtr(msg));
	return klass;
}

static VALUE
qfatal(VALUE klass, VALUE msg)
{
	qFatal("%s", StringValuePtr(msg));
	return klass;
}

static VALUE
qwarning(VALUE klass, VALUE msg)
{
	qWarning("%s", StringValuePtr(msg));
	return klass;
}

// ----------------   Helpers -------------------

//---------- All functions except fully qualified statics & enums ---------

static VALUE qobject_metaobject(VALUE self);
static VALUE kde_package_to_class(const char * package, VALUE base_class);

VALUE
set_obj_info(const char * className, smokeruby_object * o)
{
    VALUE klass = rb_funcall(qt_internal_module,
			     rb_intern("find_class"),
			     1,
			     rb_str_new2(className) );
	if (klass == Qnil) {
		rb_raise(rb_eRuntimeError, "Class '%s' not found", className);
	}

	Smoke::Index *r = classcache.value(className);
	if (r != 0) {
		o->classId = (int)*r;
	}
	// If the instance is a subclass of QObject, then check to see if the
	// className from its QMetaObject is in the Smoke library. If not then
	// create a Ruby class for it dynamically. Remove the first letter from 
	// any class names beginning with 'Q' or 'K' and put them under the Qt:: 
	// or KDE:: modules respectively.
	if (isDerivedFrom(o->smoke, o->classId, o->smoke->idClass("QObject"))) {
		QObject * qobject = (QObject *) o->smoke->cast(o->ptr, o->classId, o->smoke->idClass("QObject"));
		const QMetaObject * meta = qobject->metaObject();
		int classId = o->smoke->idClass(meta->className());
		// The class isn't in the Smoke lib..
		if (classId == 0) {
			VALUE new_klass = Qnil;
			QByteArray className(meta->className());

			// The konsolePart class is in kdebase, and so it can't be in the Smoke library.
			// This hack instantiates a Ruby KDE::KonsolePart instance
			if (className == "konsolePart") {
				new_klass = konsole_part_class;
			} else if (className == "QTableModel") {
				new_klass = qtablemodel_class;
			} else if (className == "QListModel") {
				new_klass = qlistmodel_class;
			} else if (className.startsWith("Q")) {
				className.replace("Q", "");
				className = className.mid(0, 1).toUpper() + className.mid(1);
    			new_klass = rb_define_class_under(qt_module, className, klass);
			} else if (kde_module == Qnil) {
    			new_klass = rb_define_class(className, klass);
			} else {
				new_klass = kde_package_to_class(className, klass);
			}

			if (new_klass != Qnil) {
				klass = new_klass;

				for (int id = meta->enumeratorOffset(); id < meta->enumeratorCount(); id++) {
					// If there are any enum keys with the same scope as the new class then
					// add them
					if (qstrcmp(meta->className(), meta->enumerator(id).scope()) == 0) {
						for (int i = 0; i < meta->enumerator(id).keyCount(); i++) {
							rb_define_const(	klass, 
												meta->enumerator(id).key(i), 
												INT2NUM(meta->enumerator(id).value(i)) );
						}
					}
				}
			}

			// Add a Qt::Object.metaObject method which will do dynamic despatch on the
			// metaObject() virtual method so that the true QMetaObject of the class 
			// is returned, rather than for the one for the parent class that is in
			// the Smoke library.
			rb_define_method(klass, "metaObject", (VALUE (*) (...)) qobject_metaobject, 0);
		}
	}

    VALUE obj = Data_Wrap_Struct(klass, smokeruby_mark, smokeruby_free, (void *) o);
    return obj;
}

static VALUE mapObject(VALUE self, VALUE obj);

VALUE
cast_object_to(VALUE /*self*/, VALUE object, VALUE new_klass)
{
    smokeruby_object *o = value_obj_info(object);

	VALUE new_klassname = rb_funcall(new_klass, rb_intern("name"), 0);

    Smoke::Index * cast_to_id = classcache.value(StringValuePtr(new_klassname));
	if (cast_to_id == 0) {
		rb_raise(rb_eArgError, "unable to find class \"%s\" to cast to\n", StringValuePtr(new_klassname));
	}

	smokeruby_object * o_cast = alloc_smokeruby_object(	o->allocated, 
														o->smoke, 
														(int) *cast_to_id, 
														o->smoke->cast(o->ptr, o->classId, (int) *cast_to_id) );

    VALUE obj = Data_Wrap_Struct(new_klass, smokeruby_mark, smokeruby_free, (void *) o_cast);
    mapPointer(obj, o_cast, o_cast->classId, 0);
    return obj;
}

VALUE 
kross2smoke(VALUE /*self*/, VALUE krobject, VALUE new_klass)
{
  VALUE new_klassname = rb_funcall(new_klass, rb_intern("name"), 0);
  
  Smoke::Index * cast_to_id = classcache.value(StringValuePtr(new_klassname));
  if (cast_to_id == 0) {
    rb_raise(rb_eArgError, "unable to find class \"%s\" to cast to\n", StringValuePtr(new_klassname));
  }
  
  void* o;
  Data_Get_Struct(krobject, void, o);
  
  smokeruby_object * o_cast = alloc_smokeruby_object(false, qt_Smoke, (int) *cast_to_id, o);
  
  VALUE obj = Data_Wrap_Struct(new_klass, smokeruby_mark, smokeruby_free, (void *) o_cast);
  mapPointer(obj, o_cast, o_cast->classId, 0);
  return obj;
}

VALUE
smoke2kross(VALUE /* self*/, VALUE sobj)
{
  smokeruby_object * o;
  Data_Get_Struct(sobj, smokeruby_object, o);	
  
  return Data_Wrap_Struct(rb_cObject, 0, 0, o->ptr );
}

VALUE
qvariant_value(VALUE /*self*/, VALUE variant_value_klass, VALUE variant_value)
{
	char * classname = rb_class2name(variant_value_klass);
    smokeruby_object *o = value_obj_info(variant_value);
	if (o == 0 || o->ptr == 0) {
		return Qnil;
	}

	QVariant * variant = (QVariant*) o->ptr;

    Smoke::Index * value_class_id = classcache.value(classname);
	if (value_class_id == 0) {
		return Qnil;
	}

	void * value_ptr = 0;
	VALUE result = Qnil;
	smokeruby_object * vo = 0;

	if (qstrcmp(classname, "Qt::Pixmap") == 0) {
		QPixmap v = qVariantValue<QPixmap>(*variant);
		value_ptr = (void *) new QPixmap(v);
	} else if (qstrcmp(classname, "Qt::Font") == 0) {
		QFont v = qVariantValue<QFont>(*variant);
		value_ptr = (void *) new QFont(v);
	} else if (qstrcmp(classname, "Qt::Brush") == 0) {
		QBrush v = qVariantValue<QBrush>(*variant);
		value_ptr = (void *) new QBrush(v);
	} else if (qstrcmp(classname, "Qt::Color") == 0) {
		QColor v = qVariantValue<QColor>(*variant);
		value_ptr = (void *) new QColor(v);
	} else if (qstrcmp(classname, "Qt::Palette") == 0) {
		QPalette v = qVariantValue<QPalette>(*variant);
		value_ptr = (void *) new QPalette(v);
	} else if (qstrcmp(classname, "Qt::Icon") == 0) {
		QIcon v = qVariantValue<QIcon>(*variant);
		value_ptr = (void *) new QIcon(v);
	} else if (qstrcmp(classname, "Qt::Image") == 0) {
		QImage v = qVariantValue<QImage>(*variant);
		value_ptr = (void *) new QImage(v);
	} else if (qstrcmp(classname, "Qt::Polygon") == 0) {
		QPolygon v = qVariantValue<QPolygon>(*variant);
		value_ptr = (void *) new QPolygon(v);
	} else if (qstrcmp(classname, "Qt::Region") == 0) {
		QRegion v = qVariantValue<QRegion>(*variant);
		value_ptr = (void *) new QRegion(v);
	} else if (qstrcmp(classname, "Qt::Bitmap") == 0) {
		QBitmap v = qVariantValue<QBitmap>(*variant);
		value_ptr = (void *) new QBitmap(v);
	} else if (qstrcmp(classname, "Qt::Cursor") == 0) {
		QCursor v = qVariantValue<QCursor>(*variant);
		value_ptr = (void *) new QCursor(v);
	} else if (qstrcmp(classname, "Qt::SizePolicy") == 0) {
		QSizePolicy v = qVariantValue<QSizePolicy>(*variant);
		value_ptr = (void *) new QSizePolicy(v);
	} else if (qstrcmp(classname, "Qt::KeySequence") == 0) {
		QKeySequence v = qVariantValue<QKeySequence>(*variant);
		value_ptr = (void *) new QKeySequence(v);
	} else if (qstrcmp(classname, "Qt::Pen") == 0) {
		QPen v = qVariantValue<QPen>(*variant);
		value_ptr = (void *) new QPen(v);
	} else if (qstrcmp(classname, "Qt::TextLength") == 0) {
		QTextLength v = qVariantValue<QTextLength>(*variant);
		value_ptr = (void *) new QTextLength(v);
	} else if (qstrcmp(classname, "Qt::TextFormat") == 0) {
		QTextFormat v = qVariantValue<QTextFormat>(*variant);
		value_ptr = (void *) new QTextFormat(v);
	} else if (qstrcmp(classname, "Qt::Variant") == 0) {
		value_ptr = (void *) new QVariant(*((QVariant *) variant->constData()));
	} else if (variant->type() >= QVariant::UserType) { 
		value_ptr = QMetaType::construct(QMetaType::type(variant->typeName()), (void *) variant->constData());
	} else {
		// Assume the value of the Qt::Variant can be obtained
		// with a call such as Qt::Variant.toPoint()
		QByteArray toValueMethodName(classname);
		if (toValueMethodName.startsWith("Qt::")) {
			toValueMethodName.remove(0, strlen("Qt::"));
		}
		toValueMethodName.prepend("to");
		return rb_funcall(variant_value, rb_intern(toValueMethodName), 1, variant_value);
	}

	vo = alloc_smokeruby_object(true, o->smoke, *value_class_id, value_ptr);
	result = set_obj_info(classname, vo);

	return result;
}

VALUE
qvariant_from_value(int argc, VALUE * argv, VALUE self)
{
	if (argc == 2) {
		Smoke::Index nameId = 0;
		if (TYPE(argv[0]) == T_DATA) {
			nameId = qt_Smoke->idMethodName("QVariant#");
		} else if (TYPE(argv[0]) == T_ARRAY || TYPE(argv[0]) == T_ARRAY) {
			nameId = qt_Smoke->idMethodName("QVariant?");
		} else {
			nameId = qt_Smoke->idMethodName("QVariant$");
		}

		Smoke::Index meth = qt_Smoke->findMethod(qt_Smoke->idClass("QVariant"), nameId);
		Smoke::Index i = qt_Smoke->methodMaps[meth].method;
		i = -i;		// turn into ambiguousMethodList index
		while (qt_Smoke->ambiguousMethodList[i] != 0) {
			if (	qstrcmp(	qt_Smoke->types[qt_Smoke->argumentList[qt_Smoke->methods[qt_Smoke->ambiguousMethodList[i]].args]].name,
								StringValuePtr(argv[1]) ) == 0 )
			{
				_current_method = qt_Smoke->ambiguousMethodList[i];
				MethodCall c(qt_Smoke, _current_method, self, argv, 0);
				c.next();
				return *(c.var());
			}

			i++;
		}
	}

	char * classname = rb_obj_classname(argv[0]);
    smokeruby_object *o = value_obj_info(argv[0]);
	if (o == 0 || o->ptr == 0) {
		// Assume the Qt::Variant can be created with a
		// Qt::Variant.new(obj) call
		if (qstrcmp(classname, "Qt::Enum") == 0) {
			return rb_funcall(qvariant_class, rb_intern("new"), 1, rb_funcall(argv[0], rb_intern("to_i"), 0));
		} else {
			return rb_funcall(qvariant_class, rb_intern("new"), 1, argv[0]);
		}
	}

	QVariant * v = 0;

	if (qstrcmp(classname, "Qt::Pixmap") == 0) {
		v = new QVariant(qVariantFromValue(*(QPixmap*) o->ptr));
	} else if (qstrcmp(classname, "Qt::Font") == 0) {
		v = new QVariant(qVariantFromValue(*(QFont*) o->ptr));
	} else if (qstrcmp(classname, "Qt::Brush") == 0) {
		v = new QVariant(qVariantFromValue(*(QBrush*) o->ptr));
	} else if (qstrcmp(classname, "Qt::Color") == 0) {
		v = new QVariant(qVariantFromValue(*(QColor*) o->ptr));
	} else if (qstrcmp(classname, "Qt::Palette") == 0) {
		v = new QVariant(qVariantFromValue(*(QPalette*) o->ptr));
	} else if (qstrcmp(classname, "Qt::Icon") == 0) {
		v = new QVariant(qVariantFromValue(*(QIcon*) o->ptr));
	} else if (qstrcmp(classname, "Qt::Image") == 0) {
		v = new QVariant(qVariantFromValue(*(QImage*) o->ptr));
	} else if (qstrcmp(classname, "Qt::Polygon") == 0) {
		v = new QVariant(qVariantFromValue(*(QPolygon*) o->ptr));
	} else if (qstrcmp(classname, "Qt::Region") == 0) {
		v = new QVariant(qVariantFromValue(*(QRegion*) o->ptr));
	} else if (qstrcmp(classname, "Qt::Bitmap") == 0) {
		v = new QVariant(qVariantFromValue(*(QBitmap*) o->ptr));
	} else if (qstrcmp(classname, "Qt::Cursor") == 0) {
		v = new QVariant(qVariantFromValue(*(QCursor*) o->ptr));
	} else if (qstrcmp(classname, "Qt::SizePolicy") == 0) {
		v = new QVariant(qVariantFromValue(*(QSizePolicy*) o->ptr));
	} else if (qstrcmp(classname, "Qt::KeySequence") == 0) {
		v = new QVariant(qVariantFromValue(*(QKeySequence*) o->ptr));
	} else if (qstrcmp(classname, "Qt::Pen") == 0) {
		v = new QVariant(qVariantFromValue(*(QPen*) o->ptr));
	} else if (qstrcmp(classname, "Qt::TextLength") == 0) {
		v = new QVariant(qVariantFromValue(*(QTextLength*) o->ptr));
	} else if (qstrcmp(classname, "Qt::TextFormat") == 0) {
		v = new QVariant(qVariantFromValue(*(QTextFormat*) o->ptr));
	} else if (QVariant::nameToType(o->smoke->classes[o->classId].className) >= QVariant::UserType) {
		v = new QVariant(QMetaType::type(o->smoke->classes[o->classId].className), o->ptr);
	} else {
		// Assume the Qt::Variant can be created with a
		// Qt::Variant.new(obj) call
		return rb_funcall(qvariant_class, rb_intern("new"), 1, argv[0]);
	}

	smokeruby_object * vo = alloc_smokeruby_object(true, o->smoke, o->smoke->idClass("QVariant"), v);
	VALUE result = set_obj_info("Qt::Variant", vo);

	return result;
}

const char *
get_VALUEtype(VALUE ruby_value)
{
	char * classname = rb_obj_classname(ruby_value);
	const char *r = "";
	if (ruby_value == Qnil)
		r = "u";
	else if (TYPE(ruby_value) == T_FIXNUM || TYPE(ruby_value) == T_BIGNUM || qstrcmp(classname, "Qt::Integer") == 0)
		r = "i";
	else if (TYPE(ruby_value) == T_FLOAT)
		r = "n";
	else if (TYPE(ruby_value) == T_STRING)
		r = "s";
	else if(ruby_value == Qtrue || ruby_value == Qfalse || qstrcmp(classname, "Qt::Boolean") == 0)
		r = "B";
	else if (qstrcmp(classname, "Qt::Enum") == 0) {
		VALUE temp = rb_funcall(qt_internal_module, rb_intern("get_qenum_type"), 1, ruby_value);
		r = StringValuePtr(temp);
	} else if (TYPE(ruby_value) == T_DATA) {
		smokeruby_object *o = value_obj_info(ruby_value);
		if (o == 0 || o->smoke == 0) {
			r = "a";
		} else {
			r = o->smoke->classes[o->classId].className;
		}
	} else {
		r = "U";
	}

    return r;
}

VALUE prettyPrintMethod(Smoke::Index id) 
{
    VALUE r = rb_str_new2("");
    Smoke::Method &meth = qt_Smoke->methods[id];
    const char *tname = qt_Smoke->types[meth.ret].name;
    if(meth.flags & Smoke::mf_static) rb_str_catf(r, "static ");
    rb_str_catf(r, "%s ", (tname ? tname:"void"));
    rb_str_catf(r, "%s::%s(", qt_Smoke->classes[meth.classId].className, qt_Smoke->methodNames[meth.name]);
    for(int i = 0; i < meth.numArgs; i++) {
	if(i) rb_str_catf(r, ", ");
	tname = qt_Smoke->types[qt_Smoke->argumentList[meth.args+i]].name;
	rb_str_catf(r, "%s", (tname ? tname:"void"));
    }
    rb_str_catf(r, ")");
    if(meth.flags & Smoke::mf_const) rb_str_catf(r, " const");
    return r;
}

//---------- Ruby methods (for all functions except fully qualified statics & enums) ---------


// Takes a variable name and a QProperty with QVariant value, and returns a '
// variable=value' pair with the value in ruby inspect style
static QString
inspectProperty(QMetaProperty property, const char * name, QVariant & value)
{
	if (property.isEnumType()) {
		QMetaEnum e = property.enumerator();
		return QString(" %1=%2::%3").arg(name).arg(e.scope()).arg(e.valueToKey(value.toInt()));
	}
	
	switch (value.type()) {
	case QVariant::String:
	{
		if (value.toString().isNull()) {
			return QString(" %1=nil").arg(name); 
		} else {
			return QString(" %1=%2").arg(name).arg(value.toString());
		}
	}
		
	case QVariant::Bool:
	{
		QString rubyName;
		QRegExp name_re("^(is|has)(.)(.*)");
		
		if (name_re.indexIn(name) != -1) {
			rubyName = name_re.cap(2).toLower() + name_re.cap(3) + "?";
		} else {
			rubyName = name;
		}
		
		return QString(" %1=%2").arg(rubyName).arg(value.toString());
	}

	case QVariant::Color:
	{
		QColor c = value.value<QColor>();
		return QString(" %1=#<Qt::Color:0x0 %2>").arg(name).arg(c.name());
	}
			
	case QVariant::Cursor:
	{
		QCursor c = value.value<QCursor>();
		return QString(" %1=#<Qt::Cursor:0x0 shape=%2>").arg(name).arg(c.shape());
	}
	
	case QVariant::Double:
	{
		return QString(" %1=%2").arg(name).arg(value.toDouble());
	}
	
	case QVariant::Font:
	{
		QFont f = value.value<QFont>();
		return QString(	" %1=#<Qt::Font:0x0 family=%2, pointSize=%3, weight=%4, italic=%5, bold=%6, underline=%7, strikeOut=%8>")
									.arg(name)
									.arg(f.family())
									.arg(f.pointSize())
									.arg(f.weight())
									.arg(f.italic() ? "true" : "false")
									.arg(f.bold() ? "true" : "false")
									.arg(f.underline() ? "true" : "false")
									.arg(f.strikeOut() ? "true" : "false");
	}
	
	case QVariant::Line:
	{
		QLine l = value.toLine();
		return QString(" %1=#<Qt::Line:0x0 x1=%2, y1=%3, x2=%4, y2=%5>")
						.arg(name)
						.arg(l.x1())
						.arg(l.y1())
						.arg(l.x2())
						.arg(l.y2());
	}
	
	case QVariant::LineF:
	{
		QLineF l = value.toLineF();
		return QString(" %1=#<Qt::LineF:0x0 x1=%2, y1=%3, x2=%4, y2=%5>")
						.arg(name)
						.arg(l.x1())
						.arg(l.y1())
						.arg(l.x2())
						.arg(l.y2());
	}
	
	case QVariant::Point:
	{
		QPoint p = value.toPoint();
		return QString(" %1=#<Qt::Point:0x0 x=%2, y=%3>").arg(name).arg(p.x()).arg(p.y());
	}
	
	case QVariant::PointF:
	{
		QPointF p = value.toPointF();
		return QString(" %1=#<Qt::PointF:0x0 x=%2, y=%3>").arg(name).arg(p.x()).arg(p.y());
	}
	
	case QVariant::Rect:
	{
		QRect r = value.toRect();
		return QString(" %1=#<Qt::Rect:0x0 left=%2, right=%3, top=%4, bottom=%5>")
									.arg(name)
									.arg(r.left()).arg(r.right()).arg(r.top()).arg(r.bottom());
	}
	
	case QVariant::RectF:
	{
		QRectF r = value.toRectF();
		return QString(" %1=#<Qt::RectF:0x0 left=%2, right=%3, top=%4, bottom=%5>")
									.arg(name)
									.arg(r.left()).arg(r.right()).arg(r.top()).arg(r.bottom());
	}
	
	case QVariant::Size:
	{
		QSize s = value.toSize();
		return QString(" %1=#<Qt::Size:0x0 width=%2, height=%3>")
									.arg(name) 
									.arg(s.width()).arg(s.height());
	}
	
	case QVariant::SizeF:
	{
		QSizeF s = value.toSizeF();
		return QString(" %1=#<Qt::SizeF:0x0 width=%2, height=%3>")
									.arg(name) 
									.arg(s.width()).arg(s.height());
	}
	
	case QVariant::SizePolicy:
	{
		QSizePolicy s = value.value<QSizePolicy>();
		return QString(" %1=#<Qt::SizePolicy:0x0 horizontalPolicy=%2, verticalPolicy=%3>")
									.arg(name)
									.arg(s.horizontalPolicy())
									.arg(s.verticalPolicy());
	}
	
	case QVariant::Brush:
//	case QVariant::ColorGroup:
	case QVariant::Image:
	case QVariant::Palette:
	case QVariant::Pixmap:
	case QVariant::Region:
	{
		return QString(" %1=#<Qt::%2:0x0>").arg(name).arg(value.typeName() + 1);
	}
	
	default:
		return QString(" %1=%2").arg(name)
									.arg((value.isNull() || value.toString().isNull()) ? "nil" : value.toString() );
	}
}

// Retrieves the properties for a QObject and returns them as 'name=value' pairs
// in a ruby inspect string. For example:
//
//		#<Qt::HBoxLayout:0x30139030 name=unnamed, margin=0, spacing=0, resizeMode=3>
//
static VALUE
inspect_qobject(VALUE self)
{
	if (TYPE(self) != T_DATA) {
		return Qnil;
	}
	
	// Start with #<Qt::HBoxLayout:0x30139030> from the original inspect() call
	// Drop the closing '>'
	VALUE inspect_str = rb_call_super(0, 0);	
	rb_str_resize(inspect_str, RSTRING(inspect_str)->len - 1);
	
	smokeruby_object * o = 0;
    Data_Get_Struct(self, smokeruby_object, o);	
	QObject * qobject = (QObject *) o->smoke->cast(o->ptr, o->classId, o->smoke->idClass("QObject"));
	
	QString value_list;
	value_list.append(QString(" objectName=\"%1\"").arg(qobject->objectName()));
	
	if (qobject->isWidgetType()) {
		QWidget * w = (QWidget *) qobject;
		value_list.append(QString(", x=%1, y=%2, width=%3, height=%4")
												.arg(w->x())
												.arg(w->y())
												.arg(w->width())
												.arg(w->height()) ); 
	}
		
	value_list.append(">");
	rb_str_cat2(inspect_str, value_list.toLatin1());
	
	return inspect_str;
}

// Retrieves the properties for a QObject and pretty_prints them as 'name=value' pairs
// For example:
//
//		#<Qt::HBoxLayout:0x30139030
//		 name=unnamed,
//		 margin=0,
//		 spacing=0,
//		 resizeMode=3>
//
static VALUE
pretty_print_qobject(VALUE self, VALUE pp)
{
	if (TYPE(self) != T_DATA) {
		return Qnil;
	}
	
	// Start with #<Qt::HBoxLayout:0x30139030>
	// Drop the closing '>'
	VALUE inspect_str = rb_funcall(self, rb_intern("to_s"), 0, 0);	
	rb_str_resize(inspect_str, RSTRING(inspect_str)->len - 1);
	rb_funcall(pp, rb_intern("text"), 1, inspect_str);
	rb_funcall(pp, rb_intern("breakable"), 0);
	
	smokeruby_object * o = 0;
    Data_Get_Struct(self, smokeruby_object, o);	
	QObject * qobject = (QObject *) o->smoke->cast(o->ptr, o->classId, o->smoke->idClass("QObject"));
	
	QString value_list;		
	
	if (qobject->parent() != 0) {
		QString parentInspectString;
		VALUE obj = getPointerObject(qobject->parent());
		if (obj != Qnil) {
			VALUE parent_inspect_str = rb_funcall(obj, rb_intern("to_s"), 0, 0);	
			rb_str_resize(parent_inspect_str, RSTRING(parent_inspect_str)->len - 1);
			parentInspectString = StringValuePtr(parent_inspect_str);
		} else {
			parentInspectString.sprintf("#<%s:0x0", qobject->parent()->metaObject()->className());
		}
		
		if (qobject->parent()->isWidgetType()) {
			QWidget * w = (QWidget *) qobject->parent();
			value_list = QString("  parent=%1 objectName=\"%2\", x=%3, y=%4, width=%5, height=%6>,\n")
												.arg(parentInspectString)
												.arg(w->objectName())
												.arg(w->x())
												.arg(w->y())
												.arg(w->width())
												.arg(w->height());
		} else {
			value_list = QString("  parent=%1 objectName=\"%2\">,\n")
												.arg(parentInspectString)
												.arg(qobject->parent()->objectName());
		}
		
		rb_funcall(pp, rb_intern("text"), 1, rb_str_new2(value_list.toLatin1()));
	}
	
	if (qobject->children().count() != 0) {
		value_list = QString("  children=Array (%1 element(s)),\n")
								.arg(qobject->children().count());
		rb_funcall(pp, rb_intern("text"), 1, rb_str_new2(value_list.toLatin1()));
	}
	
	value_list = QString("  metaObject=#<Qt::MetaObject:0x0");
	value_list.append(QString(" className=%1").arg(qobject->metaObject()->className()));
	
	if (qobject->metaObject()->superClass() != 0) {
		value_list.append(	QString(", superClass=#<Qt::MetaObject:0x0 className=%1>")
							.arg(qobject->metaObject()->superClass()->className()) );
	}		
	
	value_list.append(">,\n");
	rb_funcall(pp, rb_intern("text"), 1, rb_str_new2(value_list.toLatin1()));

	QMetaProperty property = qobject->metaObject()->property(0);
	QVariant value = property.read(qobject);
	value_list = " " + inspectProperty(property, property.name(), value);
	rb_funcall(pp, rb_intern("text"), 1, rb_str_new2(value_list.toLatin1()));

	for (int index = 1; index < qobject->metaObject()->propertyCount(); index++) {
		rb_funcall(pp, rb_intern("text"), 1, rb_str_new2(",\n"));

		property = qobject->metaObject()->property(index);
		value = property.read(qobject);
		value_list = " " + inspectProperty(property, property.name(), value);
		rb_funcall(pp, rb_intern("text"), 1, rb_str_new2(value_list.toLatin1()));
	}

	rb_funcall(pp, rb_intern("text"), 1, rb_str_new2(">"));
	
	return self;
}

static VALUE
q_register_resource_data(VALUE /*self*/, VALUE version, VALUE tree_value, VALUE name_value, VALUE data_value)
{
	const unsigned char * tree = (const unsigned char *) malloc(RSTRING(tree_value)->len);
	memcpy((void *) tree, (const void *) RSTRING(tree_value)->ptr, RSTRING(tree_value)->len);

	const unsigned char * name = (const unsigned char *) malloc(RSTRING(name_value)->len);
	memcpy((void *) name, (const void *) RSTRING(name_value)->ptr, RSTRING(name_value)->len);

	const unsigned char * data = (const unsigned char *) malloc(RSTRING(data_value)->len);
	memcpy((void *) data, (const void *) RSTRING(data_value)->ptr, RSTRING(data_value)->len);

	return qRegisterResourceData(NUM2INT(version), tree, name, data) ? Qtrue : Qfalse;
}

static VALUE
q_unregister_resource_data(VALUE /*self*/, VALUE version, VALUE tree_value, VALUE name_value, VALUE data_value)
{
	const unsigned char * tree = (const unsigned char *) malloc(RSTRING(tree_value)->len);
	memcpy((void *) tree, (const void *) RSTRING(tree_value)->ptr, RSTRING(tree_value)->len);

	const unsigned char * name = (const unsigned char *) malloc(RSTRING(name_value)->len);
	memcpy((void *) name, (const void *) RSTRING(name_value)->ptr, RSTRING(name_value)->len);

	const unsigned char * data = (const unsigned char *) malloc(RSTRING(data_value)->len);
	memcpy((void *) data, (const void *) RSTRING(data_value)->ptr, RSTRING(data_value)->len);

	return qUnregisterResourceData(NUM2INT(version), tree, name, data) ? Qtrue : Qfalse;
}

static VALUE
qabstract_item_model_rowcount(int argc, VALUE * argv, VALUE self)
{
    smokeruby_object *o = value_obj_info(self);
	QAbstractItemModel * model = (QAbstractItemModel *) o->ptr;
	if (argc == 0) {
		return INT2NUM(model->rowCount());
	}

	if (argc == 1) {
		smokeruby_object * mi = value_obj_info(argv[0]);
		QModelIndex * modelIndex = (QModelIndex *) mi->ptr;
		return INT2NUM(model->rowCount(*modelIndex));
	}

	rb_raise(rb_eArgError, "Invalid argument list");
}

static VALUE
qabstract_item_model_columncount(int argc, VALUE * argv, VALUE self)
{
    smokeruby_object *o = value_obj_info(self);
	QAbstractItemModel * model = (QAbstractItemModel *) o->ptr;
	if (argc == 0) {
		return INT2NUM(model->columnCount());
	}

	if (argc == 1) {
		smokeruby_object * mi = value_obj_info(argv[0]);
		QModelIndex * modelIndex = (QModelIndex *) mi->ptr;
		return INT2NUM(model->columnCount(*modelIndex));
	}

	rb_raise(rb_eArgError, "Invalid argument list");
}

static VALUE
qabstract_item_model_data(int argc, VALUE * argv, VALUE self)
{
    smokeruby_object * o = value_obj_info(self);
	QAbstractItemModel * model = (QAbstractItemModel *) o->ptr;
    smokeruby_object * mi = value_obj_info(argv[0]);
	QModelIndex * modelIndex = (QModelIndex *) mi->ptr;
	QVariant value;
	if (argc == 1) {
		value = model->data(*modelIndex);
	} else if (argc == 2) {
		value = model->data(*modelIndex, NUM2INT(rb_funcall(argv[1], rb_intern("to_i"), 0)));
	} else {
		rb_raise(rb_eArgError, "Invalid argument list");
	}


	smokeruby_object  * result = alloc_smokeruby_object(	true, 
															o->smoke, 
															o->smoke->idClass("QVariant"), 
															new QVariant(value) );
	return set_obj_info("Qt::Variant", result);
}

static VALUE
qabstract_item_model_setdata(int argc, VALUE * argv, VALUE self)
{
    smokeruby_object *o = value_obj_info(self);
	QAbstractItemModel * model = (QAbstractItemModel *) o->ptr;
    smokeruby_object * mi = value_obj_info(argv[0]);
	QModelIndex * modelIndex = (QModelIndex *) mi->ptr;
    smokeruby_object * v = value_obj_info(argv[1]);
	QVariant * variant = (QVariant *) v->ptr;

	if (argc == 2) {
		return (model->setData(*modelIndex, *variant) ? Qtrue : Qfalse);
	}

	if (argc == 3) {
		return (model->setData(	*modelIndex, 
								*variant,
								NUM2INT(rb_funcall(argv[2], rb_intern("to_i"), 0)) ) ? Qtrue : Qfalse);
	}

	rb_raise(rb_eArgError, "Invalid argument list");
}

static VALUE
qabstract_item_model_flags(VALUE self, VALUE model_index)
{
    smokeruby_object *o = value_obj_info(self);
	QAbstractItemModel * model = (QAbstractItemModel *) o->ptr;
    smokeruby_object * mi = value_obj_info(model_index);
	const QModelIndex * modelIndex = (const QModelIndex *) mi->ptr;
	return INT2NUM((int) model->flags(*modelIndex));
}

static VALUE
qabstract_item_model_insertrows(int argc, VALUE * argv, VALUE self)
{
    smokeruby_object *o = value_obj_info(self);
	QAbstractItemModel * model = (QAbstractItemModel *) o->ptr;

	if (argc == 2) {
		return (model->insertRows(NUM2INT(argv[0]), NUM2INT(argv[1])) ? Qtrue : Qfalse);
	}

	if (argc == 3) {
    	smokeruby_object * mi = value_obj_info(argv[2]);
		const QModelIndex * modelIndex = (const QModelIndex *) mi->ptr;
		return (model->insertRows(NUM2INT(argv[0]), NUM2INT(argv[1]), *modelIndex) ? Qtrue : Qfalse);
	}

	rb_raise(rb_eArgError, "Invalid argument list");
}

static VALUE
qabstract_item_model_insertcolumns(int argc, VALUE * argv, VALUE self)
{
    smokeruby_object *o = value_obj_info(self);
	QAbstractItemModel * model = (QAbstractItemModel *) o->ptr;

	if (argc == 2) {
		return (model->insertColumns(NUM2INT(argv[0]), NUM2INT(argv[1])) ? Qtrue : Qfalse);
	}

	if (argc == 3) {
    	smokeruby_object * mi = value_obj_info(argv[2]);
		const QModelIndex * modelIndex = (const QModelIndex *) mi->ptr;
		return (model->insertColumns(NUM2INT(argv[0]), NUM2INT(argv[1]), *modelIndex) ? Qtrue : Qfalse);
	}

	rb_raise(rb_eArgError, "Invalid argument list");
}

static VALUE
qabstract_item_model_removerows(int argc, VALUE * argv, VALUE self)
{
    smokeruby_object *o = value_obj_info(self);
	QAbstractItemModel * model = (QAbstractItemModel *) o->ptr;

	if (argc == 2) {
		return (model->removeRows(NUM2INT(argv[0]), NUM2INT(argv[1])) ? Qtrue : Qfalse);
	}

	if (argc == 3) {
    	smokeruby_object * mi = value_obj_info(argv[2]);
		const QModelIndex * modelIndex = (const QModelIndex *) mi->ptr;
		return (model->removeRows(NUM2INT(argv[0]), NUM2INT(argv[1]), *modelIndex) ? Qtrue : Qfalse);
	}

	rb_raise(rb_eArgError, "Invalid argument list");
}

static VALUE
qabstract_item_model_removecolumns(int argc, VALUE * argv, VALUE self)
{
    smokeruby_object *o = value_obj_info(self);
	QAbstractItemModel * model = (QAbstractItemModel *) o->ptr;

	if (argc == 2) {
		return (model->removeColumns(NUM2INT(argv[0]), NUM2INT(argv[1])) ? Qtrue : Qfalse);
	}

	if (argc == 3) {
    	smokeruby_object * mi = value_obj_info(argv[2]);
		const QModelIndex * modelIndex = (const QModelIndex *) mi->ptr;
		return (model->removeRows(NUM2INT(argv[0]), NUM2INT(argv[1]), *modelIndex) ? Qtrue : Qfalse);
	}

	rb_raise(rb_eArgError, "Invalid argument list");
}

// There is a QByteArray operator method in the Smoke lib that takes a QString
// arg and returns a QString. This is normally the desired behaviour, so
// special case a '+' method here.
static VALUE
qbytearray_append(VALUE self, VALUE str)
{
    smokeruby_object *o = value_obj_info(self);
	QByteArray * bytes = (QByteArray *) o->ptr;
	(*bytes) += (const char *) StringValuePtr(str);
	return self;
}

#ifdef QT_QTDBUS 
static VALUE
qdbusargument_endarraywrite(VALUE self)
{
    smokeruby_object *o = value_obj_info(self);
	QDBusArgument * arg = (QDBusArgument *) o->ptr;
	arg->endArray();
	return self;
}

static VALUE
qdbusargument_endmapwrite(VALUE self)
{
    smokeruby_object *o = value_obj_info(self);
	QDBusArgument * arg = (QDBusArgument *) o->ptr;
	arg->endMap();
	return self;
}

static VALUE
qdbusargument_endmapentrywrite(VALUE self)
{
    smokeruby_object *o = value_obj_info(self);
	QDBusArgument * arg = (QDBusArgument *) o->ptr;
	arg->endMapEntry();
	return self;
}

static VALUE
qdbusargument_endstructurewrite(VALUE self)
{
    smokeruby_object *o = value_obj_info(self);
	QDBusArgument * arg = (QDBusArgument *) o->ptr;
	arg->endStructure();
	return self;
}

static VALUE
qvariant_qdbusobjectpath_value(VALUE self)
{
    smokeruby_object *o = value_obj_info(self);
	QVariant * arg = (QVariant *) o->ptr;
	QString s = qVariantValue<QDBusObjectPath>(*arg).path();
	return rb_str_new2(s.toLatin1());
}

static VALUE
qvariant_qdbussignature_value(VALUE self)
{
    smokeruby_object *o = value_obj_info(self);
	QVariant * arg = (QVariant *) o->ptr;
	QString s = qVariantValue<QDBusSignature>(*arg).signature();
	return rb_str_new2(s.toLatin1());
}


#endif

// The QtRuby runtime's overloaded method resolution mechanism can't currently
// distinguish between Ruby Arrays containing different sort of instances.
// Unfortunately Qt::Painter.drawLines() and Qt::Painter.drawRects() methods can
// be passed a Ruby Array as an argument containing either Qt::Points or Qt::PointFs
// for instance. These methods need to call the correct Qt C++ methods, so special case
// the overload method resolution for now..
static VALUE
qpainter_drawlines(int argc, VALUE * argv, VALUE self)
{
static Smoke::Index drawlines_pointf_vector = 0;
static Smoke::Index drawlines_point_vector = 0;
static Smoke::Index drawlines_linef_vector = 0;
static Smoke::Index drawlines_line_vector = 0;

	if (argc == 1 && TYPE(argv[0]) == T_ARRAY && RARRAY(argv[0])->len > 0) {
		if (drawlines_point_vector == 0) {
			Smoke::Index nameId = qt_Smoke->idMethodName("drawLines?");
			Smoke::Index meth = qt_Smoke->findMethod(qt_Smoke->idClass("QPainter"), nameId);
			Smoke::Index i = qt_Smoke->methodMaps[meth].method;
			i = -i;		// turn into ambiguousMethodList index
			while (qt_Smoke->ambiguousMethodList[i] != 0) {
				const char * argType = qt_Smoke->types[qt_Smoke->argumentList[qt_Smoke->methods[qt_Smoke->ambiguousMethodList[i]].args]].name;

				if (qstrcmp(argType, "const QVector<QPointF>&" ) == 0) {
					drawlines_pointf_vector = qt_Smoke->ambiguousMethodList[i];
				} else if (qstrcmp(argType, "const QVector<QPoint>&" ) == 0) {
					drawlines_point_vector = qt_Smoke->ambiguousMethodList[i];
				} else if (qstrcmp(argType, "const QVector<QLineF>&" ) == 0) {
					drawlines_linef_vector = qt_Smoke->ambiguousMethodList[i];
				} else if (qstrcmp(argType, "const QVector<QLine>&" ) == 0) {
					drawlines_line_vector = qt_Smoke->ambiguousMethodList[i];
				}

				i++;
			}
		}

		smokeruby_object * o = value_obj_info(rb_ary_entry(argv[0], 0));

		if (qstrcmp(o->smoke->classes[o->classId].className, "QPointF") == 0) {
			_current_method = drawlines_pointf_vector;
		} else if (qstrcmp(o->smoke->classes[o->classId].className, "QPoint") == 0) {
			_current_method = drawlines_point_vector;
		} else if (qstrcmp(o->smoke->classes[o->classId].className, "QLineF") == 0) {
			_current_method = drawlines_linef_vector;
		} else if (qstrcmp(o->smoke->classes[o->classId].className, "QLine") == 0) {
			_current_method = drawlines_line_vector;
		} else {
			return rb_call_super(argc, argv);
		}

		MethodCall c(qt_Smoke, _current_method, self, argv, argc-1);
		c.next();
		return self;
	}

	return rb_call_super(argc, argv);
}

static VALUE
qpainter_drawrects(int argc, VALUE * argv, VALUE self)
{
static Smoke::Index drawlines_rectf_vector = 0;
static Smoke::Index drawlines_rect_vector = 0;

	if (argc == 1 && TYPE(argv[0]) == T_ARRAY && RARRAY(argv[0])->len > 0) {
		if (drawlines_rectf_vector == 0) {
			Smoke::Index nameId = qt_Smoke->idMethodName("drawRects?");
			Smoke::Index meth = qt_Smoke->findMethod(qt_Smoke->idClass("QPainter"), nameId);
			Smoke::Index i = qt_Smoke->methodMaps[meth].method;
			i = -i;		// turn into ambiguousMethodList index
			while (qt_Smoke->ambiguousMethodList[i] != 0) {
				const char * argType = qt_Smoke->types[qt_Smoke->argumentList[qt_Smoke->methods[qt_Smoke->ambiguousMethodList[i]].args]].name;

				if (qstrcmp(argType, "const QVector<QRectF>&" ) == 0) {
					drawlines_rectf_vector = qt_Smoke->ambiguousMethodList[i];
				} else if (qstrcmp(argType, "const QVector<QRect>&" ) == 0) {
					drawlines_rect_vector = qt_Smoke->ambiguousMethodList[i];
				}

				i++;
			}
		}

		smokeruby_object * o = value_obj_info(rb_ary_entry(argv[0], 0));

		if (qstrcmp(o->smoke->classes[o->classId].className, "QRectF") == 0) {
			_current_method = drawlines_rectf_vector;
		} else if (qstrcmp(o->smoke->classes[o->classId].className, "QRect") == 0) {
			_current_method = drawlines_rect_vector;
		} else {
			return rb_call_super(argc, argv);
		}

		MethodCall c(qt_Smoke, _current_method, self, argv, argc-1);
		c.next();
		return self;
	}

	return rb_call_super(argc, argv);
}

static VALUE
qabstractitemmodel_createindex(int argc, VALUE * argv, VALUE self)
{
	if (argc == 2 || argc == 3) {
		smokeruby_object * o = value_obj_info(self);
		Smoke::Index nameId = o->smoke->idMethodName("createIndex$$$");
		Smoke::Index meth = o->smoke->findMethod(qt_Smoke->idClass("QAbstractItemModel"), nameId);
		Smoke::Index i = o->smoke->methodMaps[meth].method;
		i = -i;		// turn into ambiguousMethodList index
		while (o->smoke->ambiguousMethodList[i] != 0) {
			if (	qstrcmp(	o->smoke->types[o->smoke->argumentList[o->smoke->methods[o->smoke->ambiguousMethodList[i]].args + 2]].name,
							"void*" ) == 0 )
			{
	    		Smoke::Method &m = o->smoke->methods[o->smoke->ambiguousMethodList[i]];
				Smoke::ClassFn fn = o->smoke->classes[m.classId].classFn;
				Smoke::StackItem stack[4];
				stack[1].s_int = NUM2INT(argv[0]);
				stack[2].s_int = NUM2INT(argv[1]);
				if (argc == 2) {
					stack[3].s_voidp = (void*) Qnil;
				} else {
					stack[3].s_voidp = (void*) argv[2];
				}
				(*fn)(m.method, o->ptr, stack);
				smokeruby_object  * result = alloc_smokeruby_object(	true, 
																		o->smoke, 
																		o->smoke->idClass("QModelIndex"), 
																		stack[0].s_voidp );

				return set_obj_info("Qt::ModelIndex", result);
			}

			i++;
		}
	}

	return rb_call_super(argc, argv);
}

static VALUE
qmodelindex_internalpointer(VALUE self)
{

    smokeruby_object *o = value_obj_info(self);
	QModelIndex * index = (QModelIndex *) o->ptr;
	void * ptr = index->internalPointer();
	return ptr != 0 ? (VALUE) ptr : Qnil;
}

static VALUE
qitemselection_at(VALUE self, VALUE i)
{
    smokeruby_object *o = value_obj_info(self);
	QItemSelection * item = (QItemSelection *) o->ptr;
	QItemSelectionRange range = item->at(NUM2INT(i));

	smokeruby_object  * result = alloc_smokeruby_object(	true, 
															o->smoke, 
															o->smoke->idClass("QItemSelectionRange"), 
															new QItemSelectionRange(range) );

	return set_obj_info("Qt::ItemSelectionRange", result);
}

static VALUE
qitemselection_count(VALUE self)
{
    smokeruby_object *o = value_obj_info(self);
	QItemSelection * item = (QItemSelection *) o->ptr;
	return INT2NUM(item->count());
}

static VALUE
qpolygon_count(VALUE self)
{
    smokeruby_object *o = value_obj_info(self);
	QPolygon * item = (QPolygon *) o->ptr;
	return INT2NUM(item->count());
}

static VALUE
qpolygonf_count(VALUE self)
{
    smokeruby_object *o = value_obj_info(self);
	QPolygonF * item = (QPolygonF *) o->ptr;
	return INT2NUM(item->count());
}

static VALUE
metaObject(VALUE self)
{
    VALUE metaObject = rb_funcall(qt_internal_module, rb_intern("getMetaObject"), 2, Qnil, self);
    return metaObject;
}

/* This shouldn't be needed, but kalyptus doesn't generate a staticMetaObject
	method for QObject::staticMetaObject, although it does for all the other
	classes, and it isn't obvious what the problem with it is. 
	So add this as a hack to work round the bug.
*/
static VALUE
qobject_staticmetaobject(VALUE /*klass*/)
{
	QMetaObject * meta = new QMetaObject(QObject::staticMetaObject);

	smokeruby_object  * m = alloc_smokeruby_object(	true, 
													qt_Smoke, 
													qt_Smoke->idClass("QMetaObject"), 
													meta );

	VALUE obj = set_obj_info("Qt::MetaObject", m);
	return obj;
}

static VALUE
qobject_metaobject(VALUE self)
{
	smokeruby_object * o = value_obj_info(self);
	QObject * qobject = (QObject *) o->smoke->cast(o->ptr, o->classId, o->smoke->idClass("QObject"));
	QMetaObject * meta = (QMetaObject *) qobject->metaObject();
	VALUE obj = getPointerObject(meta);
	if (obj != Qnil) {
		return obj;
	}

	smokeruby_object  * m = alloc_smokeruby_object(	false, 
													o->smoke, 
													o->smoke->idClass("QMetaObject"), 
													meta );

	obj = set_obj_info("Qt::MetaObject", m);
	return obj;
}

static VALUE
new_qvariant(int argc, VALUE * argv, VALUE self)
{
static Smoke::Index new_qvariant_qlist = 0;
static Smoke::Index new_qvariant_qmap = 0;

	if (new_qvariant_qlist == 0) {
		Smoke::Index nameId = qt_Smoke->idMethodName("QVariant?");
		Smoke::Index meth = qt_Smoke->findMethod(qt_Smoke->idClass("QVariant"), nameId);
		Smoke::Index i = qt_Smoke->methodMaps[meth].method;
		i = -i;		// turn into ambiguousMethodList index
		while (qt_Smoke->ambiguousMethodList[i] != 0) {
			const char * argType = qt_Smoke->types[qt_Smoke->argumentList[qt_Smoke->methods[qt_Smoke->ambiguousMethodList[i]].args]].name;

			if (qstrcmp(argType, "const QList<QVariant>&" ) == 0) {
				new_qvariant_qlist = qt_Smoke->ambiguousMethodList[i];
			} else if (qstrcmp(argType, "const QMap<QString,QVariant>&" ) == 0) {
				new_qvariant_qmap = qt_Smoke->ambiguousMethodList[i];
			}

			i++;
		}
	}

	if (argc == 1 && TYPE(argv[0]) == T_HASH) {
		_current_method = new_qvariant_qmap;
		MethodCall c(qt_Smoke, _current_method, self, argv, argc-1);
		c.next();
    	return *(c.var());
	} else if (	argc == 1 
				&& TYPE(argv[0]) == T_ARRAY
				&& RARRAY(argv[0])->len > 0
				&& TYPE(rb_ary_entry(argv[0], 0)) != T_STRING )
	{
		_current_method = new_qvariant_qlist;
		MethodCall c(qt_Smoke, _current_method, self, argv, argc-1);
		c.next();
		return *(c.var());
	}

	return rb_call_super(argc, argv);
}

static QByteArray *
find_cached_selector(int argc, VALUE * argv, VALUE klass, char * methodName)
{
    // Look in the cache
static QByteArray * mcid = 0;
	if (mcid == 0) {
		mcid = new QByteArray();
	}
	*mcid = rb_class2name(klass);
	*mcid += ';';
	*mcid += methodName;
	for(int i=3; i<argc ; i++)
	{
		*mcid += ';';
		*mcid += get_VALUEtype(argv[i]);
	}
	
	Smoke::Index *rcid = methcache.value(*mcid);
#ifdef DEBUG
	if (do_debug & qtdb_calls) qWarning("method_missing mcid: %s", (const char *) *mcid);
#endif
	
	if (rcid) {
		// Got a hit
#ifdef DEBUG
		if (do_debug & qtdb_calls) qWarning("method_missing cache hit, mcid: %s", (const char *) *mcid);
#endif
		_current_method = *rcid;
	} else {
		_current_method = -1;
	}
	
	return mcid;
}

static VALUE
method_missing(int argc, VALUE * argv, VALUE self)
{
	char * methodName = rb_id2name(SYM2ID(argv[0]));
    VALUE klass = rb_funcall(self, rb_intern("class"), 0);

	// Look for 'thing?' methods, and try to match isThing() or hasThing() in the Smoke runtime
static QByteArray * pred = 0;
	if (pred == 0) {
		pred = new QByteArray();
	}
	
	*pred = methodName;
	if (pred->endsWith("?")) {
		smokeruby_object *o = value_obj_info(self);
		if(!o || !o->ptr) {
			return rb_call_super(argc, argv);
		}
		
		// Drop the trailing '?'
		pred->replace(pred->length() - 1, 1, "");
		
		pred->replace(0, 1, pred->mid(0, 1).toUpper());
		pred->replace(0, 0, "is");
		Smoke::Index meth = o->smoke->findMethod(o->smoke->classes[o->classId].className, (const char *) *pred);
		
		if (meth == 0) {
			pred->replace(0, 2, "has");
			meth = o->smoke->findMethod(o->smoke->classes[o->classId].className, *pred);
		}
		
		if (meth > 0) {
			methodName = (char *) (const char *) *pred;
		}
	}
		
	VALUE * temp_stack = ALLOCA_N(VALUE, argc+3);
    temp_stack[0] = rb_str_new2("Qt");
    temp_stack[1] = rb_str_new2(methodName);
    temp_stack[2] = klass;
    temp_stack[3] = self;
    for (int count = 1; count < argc; count++) {
		temp_stack[count+3] = argv[count];
    }

	{
		QByteArray * mcid = find_cached_selector(argc+3, temp_stack, klass, methodName);

		if (_current_method == -1) {
			// Find the C++ method to call. Do that from Ruby for now

			VALUE retval = rb_funcall2(qt_internal_module, rb_intern("do_method_missing"), argc+3, temp_stack);
			if (_current_method == -1) {
				char * op = rb_id2name(SYM2ID(argv[0]));
				if (	qstrcmp(op, "-") == 0
						|| qstrcmp(op, "+") == 0
						|| qstrcmp(op, "/") == 0
						|| qstrcmp(op, "%") == 0
						|| qstrcmp(op, "|") == 0 )
				{
					// Look for operator methods of the form 'operator+=', 'operator-=' and so on..
					char op1[3];
					op1[0] = op[0];
					op1[1] = '=';
					op1[2] = '\0';
					temp_stack[1] = rb_str_new2(op1);
					retval = rb_funcall2(qt_internal_module, rb_intern("do_method_missing"), argc+3, temp_stack);
				}

				if (_current_method == -1) {
					// Check for property getter/setter calls, and for slots in QObject classes
					// not in the smoke library
					smokeruby_object *o = value_obj_info(self);
					if (	o != 0 
							&& o->ptr != 0 
							&& isDerivedFrom(o->smoke, o->classId, o->smoke->idClass("QObject")) )
					{
						QObject * qobject = (QObject *) o->smoke->cast(o->ptr, o->classId, o->smoke->idClass("QObject"));
static QByteArray * name = 0;
						if (name == 0) {
							name = new QByteArray();
						}
						
						*name = rb_id2name(SYM2ID(argv[0]));
						const QMetaObject * meta = qobject->metaObject();

						if (argc == 1) {
							if (name->endsWith("?")) {
								name->replace(0, 1, pred->mid(0, 1).toUpper());
								name->replace(0, 0, "is");
								if (meta->indexOfProperty(*name) == -1) {
									name->replace(0, 2, "has");
								}
							}

							if (meta->indexOfProperty(*name) != -1) {
								VALUE qvariant = rb_funcall(self, rb_intern("property"), 1, rb_str_new2(*name));
								return rb_funcall(qvariant, rb_intern("value"), 0);
							}
						}

						if (argc == 2 && name->endsWith("=")) {
							name->replace("=", "");
							if (meta->indexOfProperty(*name) != -1) {
								VALUE qvariant = rb_funcall(self, rb_intern("qVariantFromValue"), 1, argv[1]);
								return rb_funcall(self, rb_intern("setProperty"), 2, rb_str_new2(*name), qvariant);
							}
						}

						int classId = o->smoke->idClass(meta->className());

						// The class isn't in the Smoke lib. But if it is called 'local::Merged'
						// it is from a QDBusInterface and the slots are remote, so don't try to
						// those.
						while (	classId == 0 
								&& qstrcmp(meta->className(), "local::Merged") != 0
								&& qstrcmp(meta->superClass()->className(), "QDBusAbstractInterface") != 0 ) 
						{
							// Assume the QObject has slots which aren't in the Smoke library, so try
							// and call the slot directly
							for (int id = meta->methodOffset(); id < meta->methodCount(); id++) {
								if (meta->method(id).methodType() == QMetaMethod::Slot) {
									QByteArray signature(meta->method(id).signature());
									QByteArray methodName = signature.mid(0, signature.indexOf('('));

									// Don't check that the types of the ruby args match the c++ ones for now,
									// only that the name and arg count is the same.
									if (*name == methodName && meta->method(id).parameterTypes().count() == (argc - 1)) {
										VALUE args = rb_funcall(	qt_internal_module, 
																	rb_intern("getMocArguments"), 
																	2, 
																	rb_str_new2(meta->method(id).typeName()), 
																	rb_str_new2(meta->method(id).signature()) );									
										VALUE result = Qnil;
										InvokeNativeSlot slot(qobject, id, argc - 1, args, argv + 1, &result);
										slot.next();
										return result;
									}
								}
							}
							meta = meta->superClass();
							classId = o->smoke->idClass(meta->className());
						}
					}
					
					return rb_call_super(argc, argv);
				}
			}
			// Success. Cache result.
			methcache.insert(*mcid, new Smoke::Index(_current_method));
		}
	}
	
    MethodCall c(qt_Smoke, _current_method, self, temp_stack+4, argc-1);
    c.next();
    VALUE result = *(c.var());
    return result;
}

static VALUE
class_method_missing(int argc, VALUE * argv, VALUE klass)
{
	VALUE result = Qnil;
	char * methodName = rb_id2name(SYM2ID(argv[0]));
	VALUE * temp_stack = ALLOCA_N(VALUE, argc+3);
    temp_stack[0] = rb_str_new2("Qt");
    temp_stack[1] = rb_str_new2(methodName);
    temp_stack[2] = klass;
    temp_stack[3] = Qnil;
    for (int count = 1; count < argc; count++) {
		temp_stack[count+3] = argv[count];
    }

    {
		QByteArray * mcid = find_cached_selector(argc+3, temp_stack, klass, methodName);

		if (_current_method == -1) {
			VALUE retval = rb_funcall2(qt_internal_module, rb_intern("do_method_missing"), argc+3, temp_stack);
			Q_UNUSED(retval);
			if (_current_method != -1) {
				// Success. Cache result.
				methcache.insert(*mcid, new Smoke::Index(_current_method));
			}
		}
	}

    if (_current_method == -1) {
static QRegExp * rx = 0;
		if (rx == 0) {
			rx = new QRegExp("[a-zA-Z]+");
		}
		
		if (rx->indexIn(methodName) == -1) {
			// If an operator method hasn't been found as an instance method,
			// then look for a class method - after 'op(self,a)' try 'self.op(a)' 
	    	VALUE * method_stack = ALLOCA_N(VALUE, argc - 1);
	    	method_stack[0] = argv[0];
	    	for (int count = 1; count < argc - 1; count++) {
				method_stack[count] = argv[count+1];
    		}
			result = method_missing(argc-1, method_stack, argv[1]);
			return result;
		} else {
			return rb_call_super(argc, argv);
		}
    }

    MethodCall c(qt_Smoke, _current_method, Qnil, temp_stack+4, argc-1);
    c.next();
    result = *(c.var());
    return result;
}

static VALUE module_method_missing(int argc, VALUE * argv, VALUE /*klass*/)
{
    return class_method_missing(argc, argv, qt_module);
}

static VALUE kde_module_method_missing(int argc, VALUE * argv, VALUE klass)
{
    return class_method_missing(argc, argv, klass);
}

/*

class LCDRange < Qt::Widget

	def initialize(s, parent, name)
		super(parent, name)
		init()
		...

For a case such as the above, the QWidget can't be instantiated until
the initializer has been run up to the point where 'super(parent, name)'
is called. Only then, can the number and type of arguments passed to the
constructor be known. However, the rest of the intializer
can't be run until 'self' is a proper T_DATA object with a wrapped C++
instance.

The solution is to run the initialize code twice. First, only up to the
'super(parent, name)' call, where the QWidget would get instantiated in
initialize_qt(). And then rb_throw() jumps out of the
initializer returning the wrapped object as a result.

The second time round 'self' will be the wrapped instance of type T_DATA,
so initialize() can be allowed to proceed to the end.
*/
static VALUE
initialize_qt(int argc, VALUE * argv, VALUE self)
{
	VALUE retval;
	VALUE temp_obj;
	
	if (TYPE(self) == T_DATA) {
		// If a ruby block was passed then run that now
		if (rb_block_given_p()) {
			rb_funcall(qt_internal_module, rb_intern("run_initializer_block"), 2, self, rb_block_proc());
		}

		return self;
	}

	VALUE klass = rb_funcall(self, rb_intern("class"), 0);
	VALUE constructor_name = rb_str_new2("new");

	VALUE * temp_stack = ALLOCA_N(VALUE, argc+4);

	temp_stack[0] = rb_str_new2("Qt");
	temp_stack[1] = constructor_name;
	temp_stack[2] = klass;
	temp_stack[3] = self;
	
	for (int count = 0; count < argc; count++) {
		temp_stack[count+4] = argv[count];
	}

	{ 
		QByteArray * mcid = find_cached_selector(argc+4, temp_stack, klass, rb_class2name(klass));

		if (_current_method == -1) {
			retval = rb_funcall2(qt_internal_module, rb_intern("do_method_missing"), argc+4, temp_stack);
			if (_current_method != -1) {
				// Success. Cache result.
				methcache.insert(*mcid, new Smoke::Index(_current_method));
			}
		}
	}

	if (_current_method == -1) {
		// Another longjmp here..
		rb_raise(rb_eArgError, "unresolved constructor call %s\n", rb_class2name(klass));
	}
	
	{
		// Allocate the MethodCall within a C block. Otherwise, because the continue_new_instance()
		// call below will longjmp out, it wouldn't give C++ an opportunity to clean up
		MethodCall c(qt_Smoke, _current_method, self, temp_stack+4, argc);
		c.next();
		temp_obj = *(c.var());
	}
	
	smokeruby_object * p = 0;
	Data_Get_Struct(temp_obj, smokeruby_object, p);

	smokeruby_object  * o = alloc_smokeruby_object(	true, 
													p->smoke, 
													p->classId, 
													p->ptr );
	p->ptr = 0;
	p->allocated = false;

	VALUE result = Data_Wrap_Struct(klass, smokeruby_mark, smokeruby_free, o);
	mapObject(result, result);
	// Off with a longjmp, never to return..
	rb_throw("newqt", result);
	/*NOTREACHED*/
	return self;
}

VALUE
new_qt(int argc, VALUE * argv, VALUE klass)
{
    VALUE * temp_stack = ALLOCA_N(VALUE, argc + 1);
	temp_stack[0] = rb_obj_alloc(klass);

	for (int count = 0; count < argc; count++) {
		temp_stack[count+1] = argv[count];
	}

	VALUE result = rb_funcall2(qt_internal_module, rb_intern("try_initialize"), argc+1, temp_stack);
	rb_obj_call_init(result, argc, argv);
	
	return result;
}

static VALUE
new_qapplication(int argc, VALUE * argv, VALUE klass)
{
 	VALUE result = Qnil;

	if (argc == 1 && TYPE(argv[0]) == T_ARRAY) {
		// Convert '(ARGV)' to '(NUM, [$0]+ARGV)'
		VALUE * local_argv = ALLOCA_N(VALUE, argc + 1);
		VALUE temp = rb_ary_dup(argv[0]);
		rb_ary_unshift(temp, rb_gv_get("$0"));
		local_argv[0] = INT2NUM(RARRAY(temp)->len);
		local_argv[1] = temp;
		result = new_qt(2, local_argv, klass);
	} else {
		result = new_qt(argc, argv, klass);
	}

	rb_gv_set("$qApp", result);
	return result;
}

// Returns $qApp.ARGV() - the original ARGV array with Qt command line options removed
static VALUE
qapplication_argv(VALUE /*self*/)
{
	VALUE result = rb_ary_new();
	// Drop argv[0], as it isn't included in the ruby global ARGV
	for (int index = 1; index < qApp->argc(); index++) {
		rb_ary_push(result, rb_str_new2(qApp->argv()[index]));
	}
	
	return result;
}

//----------------- Sig/Slot ------------------


static VALUE
qt_signal(int argc, VALUE * argv, VALUE self)
{
	smokeruby_object *o = value_obj_info(self);
	QObject *qobj = (QObject*)o->smoke->cast(o->ptr, o->classId, o->smoke->idClass("QObject"));
    if (qobj->signalsBlocked()) {
		return Qfalse;
	}

	QLatin1String signalname(rb_id2name(rb_frame_last_func()));
	VALUE metaObject_value = rb_funcall(qt_internal_module, rb_intern("getMetaObject"), 2, Qnil, self);

	smokeruby_object *ometa = value_obj_info(metaObject_value);
	if (ometa == 0) {
		return Qnil;
	}

    int i = -1;
	const QMetaObject * m = (QMetaObject*) ometa->ptr;
    for (i = m->methodCount() - 1; i > -1; i--) {
		if (m->method(i).methodType() == QMetaMethod::Signal) {
			QString name(m->method(i).signature());
static QRegExp * rx = 0;
			if (rx == 0) {
				rx = new QRegExp("\\(.*");
			}
			name.replace(*rx, "");

			if (name == signalname) {
				break;
			}
		}
    }

	if (i == -1) {
		return Qnil;
	}

	VALUE args = rb_funcall(	qt_internal_module, 
								rb_intern("getMocArguments"), 
								2, 
								rb_str_new2(m->method(i).typeName()),
								rb_str_new2(m->method(i).signature()) );

	VALUE result = Qnil;
	// Okay, we have the signal info. *whew*
	EmitSignal signal(qobj, i, argc, args, argv, &result);
	signal.next();

	return result;
}

static VALUE
qt_metacall(int /*argc*/, VALUE * argv, VALUE self)
{
	// Arguments: QMetaObject::Call _c, int id, void ** _o
	QMetaObject::Call _c = (QMetaObject::Call) NUM2INT(	rb_funcall(	qt_internal_module,
																	rb_intern("get_qinteger"), 
																	1, 
																	argv[0] ) );
	int id = NUM2INT(argv[1]);
	void ** _o = 0;

	// Note that for a slot with no args and no return type,
	// it isn't an error to get a NULL value of _o here.
	Data_Get_Struct(argv[2], void*, _o);
	// Assume the target slot is a C++ one
	smokeruby_object *o = value_obj_info(self);
	Smoke::Index nameId = o->smoke->idMethodName("qt_metacall$$?");
	Smoke::Index meth = o->smoke->findMethod(o->classId, nameId);
	if (meth > 0) {
		Smoke::Method &m = o->smoke->methods[o->smoke->methodMaps[meth].method];
		Smoke::ClassFn fn = o->smoke->classes[m.classId].classFn;
		Smoke::StackItem i[4];
		i[1].s_enum = _c;
		i[2].s_int = id;
		i[3].s_voidp = _o;
		(*fn)(m.method, o->ptr, i);
		int ret = i[0].s_int;
		if (ret < 0) {
			return INT2NUM(ret);
		}
	} else {
		// Should never happen..
		rb_raise(rb_eRuntimeError, "Cannot find %s::qt_metacall() method\n", 
			o->smoke->classes[o->classId].className );
	}

    if (_c != QMetaObject::InvokeMetaMethod) {
		return argv[1];
	}

	QObject *qobj = (QObject*) o->ptr;
	// get obj metaobject with a virtual call
	const QMetaObject *metaobject = qobj->metaObject();

	// get method/property count
	int count = 0;
	if (_c == QMetaObject::InvokeMetaMethod) {
		count = metaobject->methodCount();
	} else {
		count = metaobject->propertyCount();
	}

	if (_c == QMetaObject::InvokeMetaMethod) {
		QMetaMethod method = metaobject->method(id);

		if (method.methodType() == QMetaMethod::Signal) {
			metaobject->activate(qobj, id, (void**) _o);
			return INT2NUM(id - count);
		}

    	VALUE mocArgs = rb_funcall(	qt_internal_module, 
									rb_intern("getMocArguments"), 
									2, 
									rb_str_new2(method.typeName()),
									rb_str_new2(method.signature()) );

		QString name(method.signature());
static QRegExp * rx = 0;
		if (rx == 0) {
			rx = new QRegExp("\\(.*");
		}
		name.replace(*rx, "");
		InvokeSlot slot(self, rb_intern(name.toLatin1()), mocArgs, _o);
		slot.next();
	}
	
	return INT2NUM(id - count);
}

static VALUE
qobject_connect(int argc, VALUE * argv, VALUE self)
{
	if (rb_block_given_p()) {
		if (argc == 1) {
			return rb_funcall(qt_internal_module, rb_intern("signal_connect"), 3, self, argv[0], rb_block_proc());
		} else if (argc == 2) {
			return rb_funcall(qt_internal_module, rb_intern("connect"), 4, argv[0], argv[1], self, rb_block_proc());
		} else if (argc == 3) {
			return rb_funcall(qt_internal_module, rb_intern("connect"), 4, argv[0], argv[1], argv[2], rb_block_proc());
		} else {
			rb_raise(rb_eArgError, "Invalid argument list");
		}
	} else {
		return rb_call_super(argc, argv);
	}
}

static VALUE
qtimer_single_shot(int argc, VALUE * argv, VALUE /*self*/)
{
	if (rb_block_given_p()) {
		if (argc == 2) {
			return rb_funcall(qt_internal_module, rb_intern("single_shot_timer_connect"), 3, argv[0], argv[1], rb_block_proc());
		} else {
			rb_raise(rb_eArgError, "Invalid argument list");
		}
	} else {
		return rb_call_super(argc, argv);
	}
}

// --------------- Ruby C functions for Qt::_internal.* helpers  ----------------


static VALUE
getMethStat(VALUE /*self*/)
{
    VALUE result_list = rb_ary_new();
    rb_ary_push(result_list, INT2NUM((int)methcache.size()));
    rb_ary_push(result_list, INT2NUM((int)methcache.count()));
    return result_list;
}

static VALUE
getClassStat(VALUE /*self*/)
{
    VALUE result_list = rb_ary_new();
    rb_ary_push(result_list, INT2NUM((int)classcache.size()));
    rb_ary_push(result_list, INT2NUM((int)classcache.count()));
    return result_list;
}

static VALUE
getIsa(VALUE /*self*/, VALUE classId)
{
    VALUE parents_list = rb_ary_new();

    Smoke::Index *parents =
	qt_Smoke->inheritanceList +
	qt_Smoke->classes[NUM2INT(classId)].parents;

    while(*parents) {
	//logger("\tparent: %s", qt_Smoke->classes[*parents].className);
	rb_ary_push(parents_list, rb_str_new2(qt_Smoke->classes[*parents++].className));
    }
    return parents_list;
}

// Return the class name of a QObject. Note that the name will be in the 
// form of Qt::Widget rather than QWidget. Is this a bug or a feature?
static VALUE
class_name(VALUE self)
{
    VALUE klass = rb_funcall(self, rb_intern("class"), 0);
    return rb_funcall(klass, rb_intern("name"), 0);
}

// Allow classnames in both 'Qt::Widget' and 'QWidget' formats to be
// used as an argument to Qt::Object.inherits()
static VALUE
inherits_qobject(int argc, VALUE * argv, VALUE /*self*/)
{
	if (argc != 1) {
		return rb_call_super(argc, argv);
	}

	Smoke::Index * classId = classcache.value(StringValuePtr(argv[0]));

	if (classId == 0) {
		return rb_call_super(argc, argv);
	} else {
		VALUE super_class = rb_str_new2(qt_Smoke->classes[*classId].className);
		return rb_call_super(argc, &super_class);
	}
}

/* Adapted from the internal function qt_qFindChildren() in qobject.cpp */
static void 
rb_qFindChildren_helper(VALUE parent, const QString &name, VALUE re,
                         const QMetaObject &mo, VALUE list)
{
    if (parent == Qnil || list == Qnil)
        return;
	VALUE children = rb_funcall(parent, rb_intern("children"), 0);
    VALUE rv = Qnil;
    for (int i = 0; i < RARRAY(children)->len; ++i) {
        rv = RARRAY(children)->ptr[i];
		smokeruby_object *o = value_obj_info(rv);
		QObject * obj = (QObject *) o->smoke->cast(o->ptr, o->classId, o->smoke->idClass("QObject"));
		
		// The original code had 'if (mo.cast(obj))' as a test, but it doesn't work here
        if (obj->qt_metacast(mo.className()) != 0) {
            if (re != Qnil) {
				VALUE re_test = rb_funcall(re, rb_intern("=~"), 1, rb_funcall(rv, rb_intern("objectName"), 0));
				if (re_test != Qnil && re_test != Qfalse) {
					rb_ary_push(list, rv);
				}
            } else {
                if (name.isNull() || obj->objectName() == name) {
					rb_ary_push(list, rv);
				}
            }
        }
        rb_qFindChildren_helper(rv, name, re, mo, list);
    }
	return;
}

/* Should mimic Qt4's QObject::findChildren method with this syntax:
     obj.findChildren(Qt::Widget, "Optional Widget Name")
*/
static VALUE
find_qobject_children(int argc, VALUE *argv, VALUE self)
{
	if (argc < 1 || argc > 2) rb_raise(rb_eArgError, "Invalid argument list");
	Check_Type(argv[0], T_CLASS);

	QString name;
	VALUE re = Qnil;
	if (argc == 2) {
		// If the second arg isn't a String, assume it's a regular expression
		if (TYPE(argv[1]) == T_STRING) {
			name = QString::fromLatin1(StringValuePtr(argv[1]));
		} else {
			re = argv[1];
		}
	}
		
	VALUE metaObject = rb_funcall(argv[0], rb_intern("staticMetaObject"), 0);
	smokeruby_object *o = value_obj_info(metaObject);
	QMetaObject * mo = (QMetaObject*) o->ptr;
	VALUE result = rb_ary_new();
	rb_qFindChildren_helper(self, name, re, *mo, result);
	return result;
}

/* Adapted from the internal function qt_qFindChild() in qobject.cpp */
static VALUE
rb_qFindChild_helper(VALUE parent, const QString &name, const QMetaObject &mo)
{
    if (parent == Qnil)
        return Qnil;
	VALUE children = rb_funcall(parent, rb_intern("children"), 0);
    VALUE rv;
	int i;
    for (i = 0; i < RARRAY(children)->len; ++i) {
        rv = RARRAY(children)->ptr[i];
		smokeruby_object *o = value_obj_info(rv);
		QObject * obj = (QObject *) o->smoke->cast(o->ptr, o->classId, o->smoke->idClass("QObject"));
        if (obj->qt_metacast(mo.className()) != 0 && (name.isNull() || obj->objectName() == name))
            return rv;
    }
    for (i = 0; i < RARRAY(children)->len; ++i) {
        rv = rb_qFindChild_helper(RARRAY(children)->ptr[i], name, mo);
        if (rv != Qnil)
            return rv;
    }
    return Qnil;
}

static VALUE
find_qobject_child(int argc, VALUE *argv, VALUE self)
{
	if (argc < 1 || argc > 2) rb_raise(rb_eArgError, "Invalid argument list");
	Check_Type(argv[0], T_CLASS);

	QString name;
	if (argc == 2) {
		name = QString::fromLatin1(StringValuePtr(argv[1]));
	}
		
	VALUE metaObject = rb_funcall(argv[0], rb_intern("staticMetaObject"), 0);
	smokeruby_object *o = value_obj_info(metaObject);
	QMetaObject * mo = (QMetaObject*) o->ptr;
	return rb_qFindChild_helper(self, name, *mo);
}

static void
mocargs_free(void * ptr)
{
    MocArgument * mocArgs = (MocArgument *) ptr;
	delete[] mocArgs;
	return;
}

static VALUE
allocateMocArguments(VALUE /*self*/, VALUE count_value)
{
    int count = NUM2INT(count_value);
    MocArgument * ptr = new MocArgument[count + 1];
    return Data_Wrap_Struct(rb_cObject, 0, mocargs_free, ptr);
}

static VALUE
setMocType(VALUE /*self*/, VALUE ptr, VALUE idx_value, VALUE name_value, VALUE static_type_value)
{
	int idx = NUM2INT(idx_value);
	QByteArray name(StringValuePtr(name_value));
	char *static_type = StringValuePtr(static_type_value);
	MocArgument *arg = 0;
	Data_Get_Struct(ptr, MocArgument, arg);
	Smoke::Index typeId = 0;

	if (name.isEmpty()) {
		arg[idx].argType = xmoc_void;
		return Qtrue;
	}

	if (qstrcmp(static_type, "ptr") == 0) {
		arg[idx].argType = xmoc_ptr;
		typeId = qt_Smoke->idType((const char *) name);
		if (typeId == 0 && !name.contains('*')) {
			name += "&";
			typeId = qt_Smoke->idType((const char *) name);
		}
	} else if (qstrcmp(static_type, "bool") == 0) {
		arg[idx].argType = xmoc_bool;
		typeId = qt_Smoke->idType((const char *) name);
	} else if (qstrcmp(static_type, "int") == 0) {
		arg[idx].argType = xmoc_int;
		typeId = qt_Smoke->idType((const char *) name);
	} else if (qstrcmp(static_type, "uint") == 0) {
		arg[idx].argType = xmoc_uint;
		typeId = qt_Smoke->idType((const char *) name);
	} else if (qstrcmp(static_type, "long") == 0) {
		arg[idx].argType = xmoc_long;
		typeId = qt_Smoke->idType((const char *) name);
	} else if (qstrcmp(static_type, "ulong") == 0) {
		arg[idx].argType = xmoc_ulong;
		typeId = qt_Smoke->idType((const char *) name);
	} else if (qstrcmp(static_type, "double") == 0) {
		arg[idx].argType = xmoc_double;
		typeId = qt_Smoke->idType((const char *) name);
	} else if (qstrcmp(static_type, "char*") == 0) {
		arg[idx].argType = xmoc_charstar;
		typeId = qt_Smoke->idType((const char *) name);
	} else if (qstrcmp(static_type, "QString") == 0) {
		arg[idx].argType = xmoc_QString;
		name += "*";
		typeId = qt_Smoke->idType((const char *) name);
	}

	if (typeId == 0) {
		rb_raise(rb_eArgError, "Cannot handle '%s' as slot argument\n", StringValuePtr(name_value));
		return Qfalse;
	}

	arg[idx].st.set(qt_Smoke, typeId);
	return Qtrue;
}

static VALUE
setDebug(VALUE self, VALUE on_value)
{
    int on = NUM2INT(on_value);
    do_debug = on;
    return self;
}

static VALUE
debugging(VALUE /*self*/)
{
    return INT2NUM(do_debug);
}

static VALUE
getTypeNameOfArg(VALUE /*self*/, VALUE method_value, VALUE idx_value)
{
    int method = NUM2INT(method_value);
    int idx = NUM2INT(idx_value);
    Smoke::Method &m = qt_Smoke->methods[method];
    Smoke::Index *args = qt_Smoke->argumentList + m.args;
    return rb_str_new2((char*)qt_Smoke->types[args[idx]].name);
}

static VALUE
classIsa(VALUE /*self*/, VALUE className_value, VALUE base_value)
{
    char *className = StringValuePtr(className_value);
    char *base = StringValuePtr(base_value);
    return isDerivedFromByName(qt_Smoke, className, base) ? Qtrue : Qfalse;
}

static VALUE
isEnum(VALUE /*self*/, VALUE enumName_value)
{
    char *enumName = StringValuePtr(enumName_value);
    Smoke::Index typeId = qt_Smoke->idType(enumName);
	return	typeId > 0 
			&& (	(qt_Smoke->types[typeId].flags & Smoke::tf_elem) == Smoke::t_enum
					|| (qt_Smoke->types[typeId].flags & Smoke::tf_elem) == Smoke::t_ulong
					|| (qt_Smoke->types[typeId].flags & Smoke::tf_elem) == Smoke::t_long
					|| (qt_Smoke->types[typeId].flags & Smoke::tf_elem) == Smoke::t_uint
					|| (qt_Smoke->types[typeId].flags & Smoke::tf_elem) == Smoke::t_int ) ? Qtrue : Qfalse;
}

static VALUE
insert_pclassid(VALUE self, VALUE p_value, VALUE ix_value)
{
    char *p = StringValuePtr(p_value);
    int ix = NUM2INT(ix_value);
    classcache.insert(QByteArray(p), new Smoke::Index((Smoke::Index)ix));
    classname.insert(ix, new QByteArray(p));
    return self;
}

static VALUE
find_pclassid(VALUE /*self*/, VALUE p_value)
{
    char *p = StringValuePtr(p_value);
    Smoke::Index *r = classcache.value(QByteArray(p));
    if(r)
        return INT2NUM((int)*r);
    else
        return INT2NUM(0);
}

static VALUE
getVALUEtype(VALUE /*self*/, VALUE ruby_value)
{
    return rb_str_new2(get_VALUEtype(ruby_value));
}

static QMetaObject* 
parent_meta_object(VALUE obj) 
{
	smokeruby_object* o = value_obj_info(obj);
	Smoke::Index nameId = o->smoke->idMethodName("metaObject");
	Smoke::Index meth = o->smoke->findMethod(o->classId, nameId);
	if (meth <= 0) {
		// Should never happen..
	}

	Smoke::Method &methodId = o->smoke->methods[o->smoke->methodMaps[meth].method];
	Smoke::ClassFn fn = o->smoke->classes[methodId.classId].classFn;
	Smoke::StackItem i[1];
	(*fn)(methodId.method, o->ptr, i);
	return (QMetaObject*) i[0].s_voidp;
}

static VALUE
make_metaObject(VALUE /*self*/, VALUE obj, VALUE parentMeta, VALUE stringdata_value, VALUE data_value)
{
	QMetaObject* superdata = 0;

	if (parentMeta == Qnil) {
		// The parent class is a Smoke class, so call metaObject() on the
		// instance to get it via a smoke library call
		superdata = parent_meta_object(obj);
	} else {
		// The parent class is a custom Ruby class whose metaObject
		// was constructed at runtime
		smokeruby_object* p = value_obj_info(parentMeta);
		superdata = (QMetaObject *) p->ptr;
	}

	char *stringdata = new char[RSTRING(stringdata_value)->len];

	int count = RARRAY(data_value)->len;
	uint * data = new uint[count];

	memcpy(	(void *) stringdata, RSTRING(stringdata_value)->ptr, RSTRING(stringdata_value)->len );
	
	for (long i = 0; i < count; i++) {
		VALUE rv = rb_ary_entry(data_value, i);
		data[i] = NUM2UINT(rv);
	}
	
	QMetaObject ob = { 
		{ superdata, stringdata, data, 0 }
	} ;

	QMetaObject * meta = new QMetaObject;
	*meta = ob;

#ifdef DEBUG
	printf("make_metaObject() superdata: %p %s\n", meta->d.superdata, superdata->className());
	
	printf(
	" // content:\n"
	"       %d,       // revision\n"
	"       %d,       // classname\n"
	"       %d,   %d, // classinfo\n"
	"       %d,   %d, // methods\n"
	"       %d,   %d, // properties\n"
	"       %d,   %d, // enums/sets\n",
	data[0], data[1], data[2], data[3], 
	data[4], data[5], data[6], data[7], data[8], data[9]);

	int s = data[3];

	if (data[2] > 0) {
		printf("\n // classinfo: key, value\n");
		for (uint j = 0; j < data[2]; j++) {
			printf("      %d,    %d\n", data[s + (j * 2)], data[s + (j * 2) + 1]);
		}
	}

	s = data[5];
	bool signal_headings = true;
	bool slot_headings = true;

	for (uint j = 0; j < data[4]; j++) {
		if (signal_headings && (data[s + (j * 5) + 4] & 0x04) != 0) {
			printf("\n // signals: signature, parameters, type, tag, flags\n");
			signal_headings = false;
		} 

		if (slot_headings && (data[s + (j * 5) + 4] & 0x08) != 0) {
			printf("\n // slots: signature, parameters, type, tag, flags\n");
			slot_headings = false;
		}

		printf("      %d,   %d,   %d,   %d, 0x%2.2x\n", 
			data[s + (j * 5)], data[s + (j * 5) + 1], data[s + (j * 5) + 2], 
			data[s + (j * 5) + 3], data[s + (j * 5) + 4]);
	}

	s += (data[4] * 5);
	for (uint j = 0; j < data[6]; j++) {
		printf("\n // properties: name, type, flags\n");
		printf("      %d,   %d,   0x%8.8x\n", 
			data[s + (j * 3)], data[s + (j * 3) + 1], data[s + (j * 3) + 2]);
	}

	s += (data[6] * 3);
	for (int i = s; i < count; i++) {
		printf("\n       %d        // eod\n", data[i]);
	}

	printf("\nqt_meta_stringdata:\n    \"");

    int strlength = 0;
	for (int j = 0; j < RSTRING(stringdata_value)->len; j++) {
        strlength++;
		if (meta->d.stringdata[j] == 0) {
			printf("\\0");
			if (strlength > 40) {
				printf("\"\n    \"");
				strlength = 0;
			}
		} else {
			printf("%c", meta->d.stringdata[j]);
		}
	}
	printf("\"\n\n");

#endif
	smokeruby_object  * m = alloc_smokeruby_object(	true, 
													qt_Smoke, 
													qt_Smoke->idClass("QMetaObject"), 
													meta );

    return Data_Wrap_Struct(qmetaobject_class, smokeruby_mark, smokeruby_free, m);
}

static VALUE
add_metaobject_methods(VALUE self, VALUE klass)
{
	rb_define_method(klass, "qt_metacall", (VALUE (*) (...)) qt_metacall, -1);
	rb_define_method(klass, "metaObject", (VALUE (*) (...)) metaObject, 0);
	return self;
}

static VALUE
add_signal_methods(VALUE self, VALUE klass, VALUE signalNames)
{
	for (long index = 0; index < RARRAY(signalNames)->len; index++) {
		VALUE signal = rb_ary_entry(signalNames, index);
		rb_define_method(klass, StringValuePtr(signal), (VALUE (*) (...)) qt_signal, -1);
	}
	return self;
}

static VALUE
dispose(VALUE self)
{
    smokeruby_object *o = value_obj_info(self);
    if(!o || !o->ptr) { return Qnil; }

    const char *className = o->smoke->classes[o->classId].className;
	if(do_debug & qtdb_gc) printf("Deleting (%s*)%p\n", className, o->ptr);
	
	unmapPointer(o, o->classId, 0);
	object_count--;

	char *methodName = new char[strlen(className) + 2];
	methodName[0] = '~';
	strcpy(methodName + 1, className);
	Smoke::Index nameId = o->smoke->idMethodName(methodName);
	Smoke::Index meth = o->smoke->findMethod(o->classId, nameId);
	if(meth > 0) {
		Smoke::Method &m = o->smoke->methods[o->smoke->methodMaps[meth].method];
		Smoke::ClassFn fn = o->smoke->classes[m.classId].classFn;
		Smoke::StackItem i[1];
		(*fn)(m.method, o->ptr, i);
	}
	delete[] methodName;
	o->ptr = 0;
	o->allocated = false;
	
	return self;
}

static VALUE
is_disposed(VALUE self)
{
	smokeruby_object *o = value_obj_info(self);
	if(o && o->ptr) { return Qtrue; }
	return Qfalse;
}

static VALUE
mapObject(VALUE self, VALUE obj)
{
    smokeruby_object *o = value_obj_info(obj);
    if(!o)
        return Qnil;
    mapPointer(obj, o, o->classId, 0);
    return self;
}

static VALUE
isaQObject(VALUE /*self*/, VALUE classid)
{
    int classid_value = NUM2INT(classid);
    return isQObject(qt_Smoke, classid_value) ? Qtrue : Qfalse;
}

// Returns the Smoke classId of a ruby instance
static VALUE
idInstance(VALUE /*self*/, VALUE instance)
{
    smokeruby_object *o = value_obj_info(instance);
    if(!o)
        return Qnil;
	
    return INT2NUM(o->classId);
}

static VALUE
idClass(VALUE /*self*/, VALUE name_value)
{
    char *name = StringValuePtr(name_value);
    return INT2NUM(qt_Smoke->idClass(name));
}

static VALUE
idMethodName(VALUE /*self*/, VALUE name_value)
{
    char *name = StringValuePtr(name_value);
    return INT2NUM(qt_Smoke->idMethodName(name));
}

static VALUE
idMethod(VALUE /*self*/, VALUE idclass_value, VALUE idmethodname_value)
{
    int idclass = NUM2INT(idclass_value);
    int idmethodname = NUM2INT(idmethodname_value);
    return INT2NUM(qt_Smoke->idMethod(idclass, idmethodname));
}

static VALUE
findMethod(VALUE /*self*/, VALUE c_value, VALUE name_value)
{
    char *c = StringValuePtr(c_value);
    char *name = StringValuePtr(name_value);
    VALUE result = rb_ary_new();
    Smoke::Index meth = qt_Smoke->findMethod(c, name);
//#ifdef DEBUG
    if (do_debug & qtdb_calls) qWarning("Found method %s::%s => %d", c, name, meth);
//#endif
    if(!meth) {
    	meth = qt_Smoke->findMethod("QGlobalSpace", name);
//#ifdef DEBUG
    if (do_debug & qtdb_calls) qWarning("Found method QGlobalSpace::%s => %d", name, meth);
//#endif
	}
	
    if(!meth) {
    	return result;
	// empty list
    } else if(meth > 0) {
	Smoke::Index i = qt_Smoke->methodMaps[meth].method;
	if(!i) {		// shouldn't happen
	    rb_raise(rb_eArgError, "Corrupt method %s::%s", c, name);
	} else if(i > 0) {	// single match
	    Smoke::Method &methodRef = qt_Smoke->methods[i];
		if ((methodRef.flags & Smoke::mf_internal) == 0) {
			rb_ary_push(result, INT2NUM(i));
		}
	} else {		// multiple match
	    i = -i;		// turn into ambiguousMethodList index
	    while(qt_Smoke->ambiguousMethodList[i]) {
	    	Smoke::Method &methodRef = qt_Smoke->methods[qt_Smoke->ambiguousMethodList[i]];
			if ((methodRef.flags & Smoke::mf_internal) == 0) {
				rb_ary_push(result, INT2NUM(qt_Smoke->ambiguousMethodList[i]));
//#ifdef DEBUG
				if (do_debug & qtdb_calls) qWarning("Ambiguous Method %s::%s => %d", c, name, qt_Smoke->ambiguousMethodList[i]);
//#endif

			}
		i++;
	    }
	}
    }
    return result;
}

// findAllMethods(classid [, startingWith]) : returns { "mungedName" => [index in methods, ...], ... }

static VALUE
findAllMethods(int argc, VALUE * argv, VALUE /*self*/)
{
    VALUE classid = argv[0];
    VALUE result = rb_hash_new();
    if(classid != Qnil) {
        Smoke::Index c = (Smoke::Index) NUM2INT(classid);
		if (c > qt_Smoke->numClasses) {
			return Qnil;
		}
        char * pat = 0L;
        if(argc > 1 && TYPE(argv[1]) == T_STRING)
            pat = StringValuePtr(argv[1]);
#ifdef DEBUG
	if (do_debug & qtdb_calls) qWarning("findAllMethods called with classid = %d, pat == %s", c, pat);
#endif
        Smoke::Index imax = qt_Smoke->numMethodMaps;
        Smoke::Index imin = 0, icur = -1, methmin, methmax;
	methmin = -1; methmax = -1; // kill warnings
        int icmp = -1;
        while(imax >= imin) {
            icur = (imin + imax) / 2;
            icmp = qt_Smoke->leg(qt_Smoke->methodMaps[icur].classId, c);
            if(!icmp) {
                Smoke::Index pos = icur;
                while(icur && qt_Smoke->methodMaps[icur-1].classId == c)
                    icur --;
                methmin = icur;
                icur = pos;
                while(icur < imax && qt_Smoke->methodMaps[icur+1].classId == c)
                    icur ++;
                methmax = icur;
                break;
            }
            if (icmp > 0)
		imax = icur - 1;
	    else
		imin = icur + 1;
        }
        if(!icmp) {
            for(Smoke::Index i=methmin ; i <= methmax ; i++) {
                Smoke::Index m = qt_Smoke->methodMaps[i].name;
                if(!pat || !strncmp(qt_Smoke->methodNames[m], pat, strlen(pat))) {
                    Smoke::Index ix= qt_Smoke->methodMaps[i].method;
		    VALUE meths = rb_ary_new();
                    if(ix >= 0) {	// single match
	    				Smoke::Method &methodRef = qt_Smoke->methods[ix];
						if ((methodRef.flags & Smoke::mf_internal) == 0) {
							rb_ary_push(meths, INT2NUM((int)ix));
						}
                    } else {		// multiple match
                        ix = -ix;		// turn into ambiguousMethodList index
                        while(qt_Smoke->ambiguousMethodList[ix]) {
	    					Smoke::Method &methodRef = qt_Smoke->methods[qt_Smoke->ambiguousMethodList[ix]];
							if ((methodRef.flags & Smoke::mf_internal) == 0) {
                          		rb_ary_push(meths, INT2NUM((int)qt_Smoke->ambiguousMethodList[ix]));
							}
                          ix++;
                        }
                    }
		    rb_hash_aset(result, rb_str_new2(qt_Smoke->methodNames[m]), meths);
                }
            }
        }
    }
    return result;
}

/*
	Flags values
		0					All methods, except enum values and protected non-static methods
		mf_static			Static methods only
		mf_enum				Enums only
		mf_protected		Protected non-static methods only
*/

#define PUSH_QTRUBY_METHOD		\
		if (	(methodRef.flags & (Smoke::mf_internal|Smoke::mf_ctor|Smoke::mf_dtor)) == 0 \
				&& strcmp(qt_Smoke->methodNames[methodRef.name], "operator=") != 0 \
				&& strcmp(qt_Smoke->methodNames[methodRef.name], "operator!=") != 0 \
				&& strcmp(qt_Smoke->methodNames[methodRef.name], "operator--") != 0 \
				&& strcmp(qt_Smoke->methodNames[methodRef.name], "operator++") != 0 \
				&& strncmp(qt_Smoke->methodNames[methodRef.name], "operator ", strlen("operator ")) != 0 \
				&& (	(flags == 0 && (methodRef.flags & (Smoke::mf_static|Smoke::mf_enum|Smoke::mf_protected)) == 0) \
						|| (	flags == Smoke::mf_static \
								&& (methodRef.flags & Smoke::mf_enum) == 0 \
								&& (methodRef.flags & Smoke::mf_static) == Smoke::mf_static ) \
						|| (flags == Smoke::mf_enum && (methodRef.flags & Smoke::mf_enum) == Smoke::mf_enum) \
						|| (	flags == Smoke::mf_protected \
								&& (methodRef.flags & Smoke::mf_static) == 0 \
								&& (methodRef.flags & Smoke::mf_protected) == Smoke::mf_protected ) ) ) { \
			if (strncmp(qt_Smoke->methodNames[methodRef.name], "operator", strlen("operator")) == 0) { \
				if (op_re.indexIn(qt_Smoke->methodNames[methodRef.name]) != -1) { \
					rb_ary_push(result, rb_str_new2((op_re.cap(1) + op_re.cap(2)).toLatin1())); \
				} else { \
					rb_ary_push(result, rb_str_new2(qt_Smoke->methodNames[methodRef.name] + strlen("operator"))); \
				} \
			} else if (predicate_re.indexIn(qt_Smoke->methodNames[methodRef.name]) != -1 && methodRef.numArgs == 0) { \
				rb_ary_push(result, rb_str_new2((predicate_re.cap(2).toLower() + predicate_re.cap(3) + "?").toLatin1())); \
			} else if (set_re.indexIn(qt_Smoke->methodNames[methodRef.name]) != -1 && methodRef.numArgs == 1) { \
				rb_ary_push(result, rb_str_new2((set_re.cap(2).toLower() + set_re.cap(3) + "=").toLatin1())); \
			} else { \
				rb_ary_push(result, rb_str_new2(qt_Smoke->methodNames[methodRef.name])); \
			} \
		}
 
static VALUE
findAllMethodNames(VALUE /*self*/, VALUE result, VALUE classid, VALUE flags_value)
{
	QRegExp predicate_re("^(is|has)(.)(.*)");
	QRegExp set_re("^(set)([A-Z])(.*)");
	QRegExp op_re("operator(.*)(([-%~/+|&*])|(>>)|(<<)|(&&)|(\\|\\|)|(\\*\\*))=$");

	unsigned short flags = (unsigned short) NUM2UINT(flags_value);
	if (classid != Qnil) {
		Smoke::Index c = (Smoke::Index) NUM2INT(classid);
		if (c > qt_Smoke->numClasses) {
			return Qnil;
		}
#ifdef DEBUG
		if (do_debug & qtdb_calls) qWarning("findAllMethodNames called with classid = %d", c);
#endif
		Smoke::Index imax = qt_Smoke->numMethodMaps;
		Smoke::Index imin = 0, icur = -1, methmin, methmax;
		methmin = -1; methmax = -1; // kill warnings
		int icmp = -1;

		while (imax >= imin) {
			icur = (imin + imax) / 2;
			icmp = qt_Smoke->leg(qt_Smoke->methodMaps[icur].classId, c);
			if (icmp == 0) {
				Smoke::Index pos = icur;
				while(icur && qt_Smoke->methodMaps[icur-1].classId == c)
					icur --;
				methmin = icur;
				icur = pos;
				while(icur < imax && qt_Smoke->methodMaps[icur+1].classId == c)
					icur ++;
				methmax = icur;
				break;
			}
			if (icmp > 0)
				imax = icur - 1;
			else
				imin = icur + 1;
		}

        if (icmp == 0) {
 			for (Smoke::Index i=methmin ; i <= methmax ; i++) {
				Smoke::Index ix= qt_Smoke->methodMaps[i].method;
				if (ix >= 0) {	// single match
					Smoke::Method &methodRef = qt_Smoke->methods[ix];
					PUSH_QTRUBY_METHOD
				} else {		// multiple match
					ix = -ix;		// turn into ambiguousMethodList index
					while (qt_Smoke->ambiguousMethodList[ix]) {
						Smoke::Method &methodRef = qt_Smoke->methods[qt_Smoke->ambiguousMethodList[ix]];
						PUSH_QTRUBY_METHOD
						ix++;
					}
				}
            }
        }
    }
    return result;
}

static VALUE
dumpCandidates(VALUE /*self*/, VALUE rmeths)
{
    VALUE errmsg = rb_str_new2("");
    if(rmeths != Qnil) {
	int count = RARRAY(rmeths)->len;
        for(int i = 0; i < count; i++) {
	    rb_str_catf(errmsg, "\t");
	    int id = NUM2INT(rb_ary_entry(rmeths, i));
	    Smoke::Method &meth = qt_Smoke->methods[id];
	    const char *tname = qt_Smoke->types[meth.ret].name;
	    if(meth.flags & Smoke::mf_enum) {
			rb_str_catf(errmsg, "enum ");
			rb_str_catf(errmsg, "%s::%s", qt_Smoke->classes[meth.classId].className, qt_Smoke->methodNames[meth.name]);
			rb_str_catf(errmsg, "\n");
	    } else {
			if(meth.flags & Smoke::mf_static) rb_str_catf(errmsg, "static ");
			rb_str_catf(errmsg, "%s ", (tname ? tname:"void"));
			rb_str_catf(errmsg, "%s::%s(", qt_Smoke->classes[meth.classId].className, qt_Smoke->methodNames[meth.name]);
			for(int i = 0; i < meth.numArgs; i++) {
			if(i) rb_str_catf(errmsg, ", ");
			tname = qt_Smoke->types[qt_Smoke->argumentList[meth.args+i]].name;
			rb_str_catf(errmsg, "%s", (tname ? tname:"void"));
			}
			rb_str_catf(errmsg, ")");
			if(meth.flags & Smoke::mf_const) rb_str_catf(errmsg, " const");
			rb_str_catf(errmsg, "\n");
        	}
        }
    }
    return errmsg;
}

static VALUE
isObject(VALUE /*self*/, VALUE obj)
{
    void * ptr = 0;
    ptr = value_to_ptr(obj);
    return (ptr > 0 ? Qtrue : Qfalse);
}

static VALUE
setCurrentMethod(VALUE self, VALUE meth_value)
{
    int meth = NUM2INT(meth_value);
    // FIXME: damn, this is lame, and it doesn't handle ambiguous methods
    _current_method = meth;  //qt_Smoke->methodMaps[meth].method;
    return self;
}

static VALUE
getClassList(VALUE /*self*/)
{
    VALUE class_list = rb_ary_new();

    for(int i = 1; i <= qt_Smoke->numClasses; i++) {
	rb_ary_push(class_list, rb_str_new2(qt_Smoke->classes[i].className));
    }

    return class_list;
}

static VALUE
kde_package_to_class(const char * package, VALUE base_class)
{
	VALUE klass = Qnil;
	QString packageName(package);
	
static QRegExp * scope_op = 0;
	if (scope_op == 0) {
		scope_op = new QRegExp("^([^:]+)::([^:]+)$");
	}

	if (packageName.startsWith("KDE::Win::")) {
		klass = rb_define_class_under(kwin_class, package+strlen("KDE::Win::"), base_class);
		rb_define_singleton_method(klass, "new", (VALUE (*) (...)) _new_kde, -1);
	} else if (packageName.startsWith("KDE::")) {
		klass = rb_define_class_under(kde_module, package+strlen("KDE::"), base_class);
		rb_define_singleton_method(klass, "new", (VALUE (*) (...)) _new_kde, -1);
	} else if (packageName.startsWith("KParts::")) {
		klass = rb_define_class_under(kparts_module, package+strlen("KParts::"), base_class);
		rb_define_singleton_method(klass, "new", (VALUE (*) (...)) _new_kde, -1);
		if (packageName == "KParts::ReadOnlyPart") {
			konsole_part_class = rb_define_class_under(kde_module, "KonsolePart", klass);
		}
	} else if (packageName.startsWith("KNS::")) {
		klass = rb_define_class_under(kns_module, package+strlen("KNS::"), base_class);
		rb_define_singleton_method(klass, "new", (VALUE (*) (...)) _new_kde, -1);
	} else if (packageName.startsWith("KIO::")) {
		klass = rb_define_class_under(kio_module, package+strlen("KIO::"), base_class);
		rb_define_singleton_method(klass, "new", (VALUE (*) (...)) _new_kde, -1);
	} else if (packageName.startsWith("DOM::")) {
		klass = rb_define_class_under(dom_module, package+strlen("DOM::"), base_class);
		rb_define_singleton_method(klass, "new", (VALUE (*) (...)) _new_kde, -1);
	} else if (packageName.startsWith("Kontact::")) {
		klass = rb_define_class_under(kontact_module, package+strlen("Kontact::"), base_class);
		rb_define_singleton_method(klass, "new", (VALUE (*) (...)) _new_kde, -1);
	} else if (packageName.startsWith("Ko") && scope_op->indexIn(packageName) == -1) {
		klass = rb_define_class_under(koffice_module, package+strlen("Ko"), base_class);
		rb_define_singleton_method(klass, "new", (VALUE (*) (...)) _new_kde, -1);
	} else if (packageName.startsWith("Kate::")) {
		klass = rb_define_class_under(kate_module, package+strlen("Kate::"), base_class);
		rb_define_singleton_method(klass, "new", (VALUE (*) (...)) _new_kde, -1);
	} else if (packageName.startsWith("Kate")) {
		klass = rb_define_class_under(kate_module, package+strlen("Kate"), base_class);
		rb_define_singleton_method(klass, "new", (VALUE (*) (...)) _new_kde, -1);
	} else if (packageName.startsWith("KMediaPlayer::")) {
		klass = rb_define_class_under(kmediaplayer_module, package+strlen("KMediaPlayer::"), base_class);
		rb_define_singleton_method(klass, "new", (VALUE (*) (...)) _new_kde, -1);
	} else if (packageName.startsWith("KTextEditor::")) {
		klass = rb_define_class_under(ktexteditor_module, package+strlen("KTextEditor::"), base_class);
		rb_define_singleton_method(klass, "new", (VALUE (*) (...)) _new_kde, -1);
	} else if (packageName.startsWith("KWallet::")) {
		klass = rb_define_class_under(kwallet_module, package+strlen("KWallet::"), base_class);
		rb_define_singleton_method(klass, "new", (VALUE (*) (...)) _new_kde, -1);
	} else if (packageName.startsWith("Plasma::")) {
		klass = rb_define_class_under(plasma_module, package+strlen("Plasma::"), base_class);
		rb_define_singleton_method(klass, "new", (VALUE (*) (...)) _new_kde, -1);
	} else if (packageName.startsWith("SafeSite::")) {
		klass = rb_define_class_under(safesite_module, package+strlen("SafeSite::"), base_class);
		rb_define_singleton_method(klass, "new", (VALUE (*) (...)) _new_kde, -1);
	} else if (packageName.startsWith("Sonnet::")) {
		klass = rb_define_class_under(sonnet_module, package+strlen("Sonnet::"), base_class);
		rb_define_singleton_method(klass, "new", (VALUE (*) (...)) _new_kde, -1);
	} else if (packageName.startsWith("Soprano::")) {
		klass = rb_define_class_under(soprano_module, package+strlen("Soprano::"), base_class);
		rb_define_singleton_method(klass, "new", (VALUE (*) (...)) _new_kde, -1);
	} else if (packageName.startsWith("Nepomuk::")) {
		klass = rb_define_class_under(nepomuk_module, package+strlen("Nepomuk::"), base_class);
		rb_define_singleton_method(klass, "new", (VALUE (*) (...)) _new_kde, -1);
	} else if (scope_op->indexIn(packageName) != -1) {
		// If an unrecognised classname of the form 'XXXXXX::YYYYYY' is found,
		// then create a module XXXXXX to put the class YYYYYY under
		VALUE module = rb_define_module(scope_op->cap(1).toLatin1());
		klass = rb_define_class_under(module, scope_op->cap(2).toLatin1(), base_class);
	} else if (	packageName.startsWith("K") 
				&& packageName.mid(1, 1).contains(QRegExp("[A-Z]")) == 1 ) 
	{
		klass = rb_define_class_under(kde_module, package+strlen("K"), base_class);
		if (packageName == QLatin1String("KConfigGroup")) {
			kconfiggroup_class = klass;
		}
	} else {
		packageName = packageName.mid(0, 1).toUpper() + packageName.mid(1);
		klass = rb_define_class_under(kde_module, packageName.toLatin1(), base_class);
	}
	
	return klass;
}

static VALUE
create_qobject_class(VALUE /*self*/, VALUE package_value)
{
	const char *package = strdup(StringValuePtr(package_value));
	VALUE klass;
	
	QString packageName(package);

	if (packageName.startsWith("Qt::")) {
		klass = rb_define_class_under(qt_module, package+strlen("Qt::"), qt_base_class);
		
		if (packageName == "Qt::Application" || packageName == "Qt::CoreApplication" ) {
			rb_define_singleton_method(klass, "new", (VALUE (*) (...)) new_qapplication, -1);
			rb_define_method(klass, "ARGV", (VALUE (*) (...)) qapplication_argv, 0);
		} else if (packageName == "Qt::Object") {
			rb_define_singleton_method(klass, "staticMetaObject", (VALUE (*) (...)) qobject_staticmetaobject, 0);
		} else if (packageName == "Qt::AbstractTableModel") {
			qtablemodel_class = rb_define_class_under(qt_module, "TableModel", klass);
			rb_define_method(qtablemodel_class, "rowCount", (VALUE (*) (...)) qabstract_item_model_rowcount, -1);
			rb_define_method(qtablemodel_class, "columnCount", (VALUE (*) (...)) qabstract_item_model_columncount, -1);
			rb_define_method(qtablemodel_class, "data", (VALUE (*) (...)) qabstract_item_model_data, -1);
			rb_define_method(qtablemodel_class, "setData", (VALUE (*) (...)) qabstract_item_model_setdata, -1);
			rb_define_method(qtablemodel_class, "flags", (VALUE (*) (...)) qabstract_item_model_flags, 1);
			rb_define_method(qtablemodel_class, "insertRows", (VALUE (*) (...)) qabstract_item_model_insertrows, -1);
			rb_define_method(qtablemodel_class, "insertColumns", (VALUE (*) (...)) qabstract_item_model_insertcolumns, -1);
			rb_define_method(qtablemodel_class, "removeRows", (VALUE (*) (...)) qabstract_item_model_removerows, -1);
			rb_define_method(qtablemodel_class, "removeColumns", (VALUE (*) (...)) qabstract_item_model_removecolumns, -1);
			
			qlistmodel_class = rb_define_class_under(qt_module, "ListModel", klass);
			rb_define_method(qlistmodel_class, "rowCount", (VALUE (*) (...)) qabstract_item_model_rowcount, -1);
			rb_define_method(qlistmodel_class, "columnCount", (VALUE (*) (...)) qabstract_item_model_columncount, -1);
			rb_define_method(qlistmodel_class, "data", (VALUE (*) (...)) qabstract_item_model_data, -1);
			rb_define_method(qlistmodel_class, "setData", (VALUE (*) (...)) qabstract_item_model_setdata, -1);
			rb_define_method(qlistmodel_class, "flags", (VALUE (*) (...)) qabstract_item_model_flags, 1);
			rb_define_method(qlistmodel_class, "insertRows", (VALUE (*) (...)) qabstract_item_model_insertrows, -1);
			rb_define_method(qlistmodel_class, "insertColumns", (VALUE (*) (...)) qabstract_item_model_insertcolumns, -1);
			rb_define_method(qlistmodel_class, "removeRows", (VALUE (*) (...)) qabstract_item_model_removerows, -1);
			rb_define_method(qlistmodel_class, "removeColumns", (VALUE (*) (...)) qabstract_item_model_removecolumns, -1);
		} else if (packageName == "Qt::AbstractItemModel") {
			rb_define_method(klass, "createIndex", (VALUE (*) (...)) qabstractitemmodel_createindex, -1);
		} else if (packageName == "Qt::Timer") {
			rb_define_singleton_method(klass, "singleShot", (VALUE (*) (...)) qtimer_single_shot, -1);
		}
	} else if (packageName.startsWith("Qsci::")) {
		if (qext_scintilla_module == Qnil) {
			qext_scintilla_module = rb_define_module("Qsci");
		}
		klass = rb_define_class_under(qext_scintilla_module, package+strlen("Qsci::"), qt_base_class);
	} else if (packageName.startsWith("Qwt::")) {
		if (qwt_module == Qnil) {
			qwt_module = rb_define_module("Qwt");
		}
		klass = rb_define_class_under(qwt_module, package+strlen("Qwt::"), qt_base_class);
	} else if (packageName.startsWith("Qt3::")) {
		if (qt3_module == Qnil) {
			qt3_module = rb_define_module("Qt3");
		}
		klass = rb_define_class_under(qt3_module, package+strlen("Qt3::"), qt_base_class);
	} else {
		klass = kde_package_to_class(package, qt_base_class);
	}

	rb_define_method(klass, "inspect", (VALUE (*) (...)) inspect_qobject, 0);
	rb_define_method(klass, "pretty_print", (VALUE (*) (...)) pretty_print_qobject, 1);
	rb_define_method(klass, "className", (VALUE (*) (...)) class_name, 0);
	rb_define_method(klass, "inherits", (VALUE (*) (...)) inherits_qobject, -1);
	rb_define_method(klass, "findChildren", (VALUE (*) (...)) find_qobject_children, -1);
	rb_define_method(klass, "findChild", (VALUE (*) (...)) find_qobject_child, -1);   
	rb_define_method(klass, "connect", (VALUE (*) (...)) qobject_connect, -1);   
	rb_define_singleton_method(klass, "connect", (VALUE (*) (...)) qobject_connect, -1);   

	free((void *) package);
	return klass;
}

static VALUE
create_qt_class(VALUE /*self*/, VALUE package_value)
{
	const char *package = strdup(StringValuePtr(package_value));
	VALUE klass;
	QString packageName(package);
	
	if (packageName.startsWith("Qt::TextLayout::")) {
		klass = rb_define_class_under(qtextlayout_class, package+strlen("Qt::TextLayout::"), qt_base_class);
	} else if (packageName.startsWith("Qt::")) {
		klass = rb_define_class_under(qt_module, package+strlen("Qt::"), qt_base_class);
	} else if (packageName.startsWith("Qsci::")) {
		if (qext_scintilla_module == Qnil) {
			qext_scintilla_module = rb_define_module("Qsci");
		}
    	klass = rb_define_class_under(qext_scintilla_module, package+strlen("Qsci::"), qt_base_class);
	} else if (packageName.startsWith("Qwt::")) {
		if (qwt_module == Qnil) {
			qwt_module = rb_define_module("Qwt");
		}
    	klass = rb_define_class_under(qwt_module, package+strlen("Qwt::"), qt_base_class);
	} else if (packageName.startsWith("Qt3::")) {
		if (qt3_module == Qnil) {
			qt3_module = rb_define_module("Qt3");
		}
    	klass = rb_define_class_under(qt3_module, package+strlen("Qt3::"), qt_base_class);
	} else {
		klass = kde_package_to_class(package, qt_base_class);
	}

	if (packageName == "Qt::MetaObject") {
		qmetaobject_class = klass;
	} else if (packageName == "Qt::TextLayout") {
		qtextlayout_class = klass;
	} else if (packageName == "Qt::Variant") {
		qvariant_class = klass;
		rb_define_singleton_method(qvariant_class, "fromValue", (VALUE (*) (...)) qvariant_from_value, -1);
    	rb_define_singleton_method(qvariant_class, "new", (VALUE (*) (...)) new_qvariant, -1);
#ifdef QT_QTDBUS 
		rb_define_method(klass, "qdbusobjectpath_value", (VALUE (*) (...)) qvariant_qdbusobjectpath_value, 1);
		rb_define_method(klass, "qdbussignature_value", (VALUE (*) (...)) qvariant_qdbussignature_value, 1);
#endif
	} else if (packageName == "Qt::ByteArray") {
		rb_define_method(klass, "+", (VALUE (*) (...)) qbytearray_append, 1);
	} else if (packageName == "Qt::Char") {
		rb_define_method(klass, "to_s", (VALUE (*) (...)) qchar_to_s, 0);
	} else if (packageName == "Qt::ItemSelection") {
		rb_define_method(klass, "[]", (VALUE (*) (...)) qitemselection_at, 1);
		rb_define_method(klass, "at", (VALUE (*) (...)) qitemselection_at, 1);
		rb_define_method(klass, "count", (VALUE (*) (...)) qitemselection_count, 0);
		rb_define_method(klass, "length", (VALUE (*) (...)) qitemselection_count, 0);
	} else if (packageName == "Qt::Painter") {
		rb_define_method(klass, "drawLines", (VALUE (*) (...)) qpainter_drawlines, -1);
		rb_define_method(klass, "drawRects", (VALUE (*) (...)) qpainter_drawrects, -1);
	} else if (packageName == "Qt::Polygon") {
		rb_define_method(klass, "size", (VALUE (*) (...)) qpolygon_count, 0);
		rb_define_method(klass, "count", (VALUE (*) (...)) qpolygon_count, 0);
	} else if (packageName == "Qt::PolygonF") {
		rb_define_method(klass, "size", (VALUE (*) (...)) qpolygonf_count, 0);
		rb_define_method(klass, "count", (VALUE (*) (...)) qpolygonf_count, 0);
	} else if (packageName == "Qt::ModelIndex") {
		rb_define_method(klass, "internalPointer", (VALUE (*) (...)) qmodelindex_internalpointer, 0);
#ifdef QT_QTDBUS
	} else if (packageName == "Qt::DBusArgument") {
		rb_define_method(klass, "endArrayWrite", (VALUE (*) (...)) qdbusargument_endarraywrite, 0);
		rb_define_method(klass, "endMapEntryWrite", (VALUE (*) (...)) qdbusargument_endmapentrywrite, 0);
		rb_define_method(klass, "endMapWrite", (VALUE (*) (...)) qdbusargument_endmapwrite, 0);
		rb_define_method(klass, "endStructureWrite", (VALUE (*) (...)) qdbusargument_endstructurewrite, 0);
#endif
	}

	free((void *) package);
	return klass;
}

static VALUE
version(VALUE /*self*/)
{
    return rb_str_new2(QT_VERSION_STR);
}

static VALUE
qtruby_version(VALUE /*self*/)
{
    return rb_str_new2(QTRUBY_VERSION);
}

void
set_new_kde(VALUE (*new_kde) (int, VALUE *, VALUE))
{
	_new_kde = new_kde;

	if (qt_module == Qnil) {
		qt_module = rb_define_module("Qt");
		qt_internal_module = rb_define_module_under(qt_module, "Internal");
		qt_base_class = rb_define_class_under(qt_module, "Base", rb_cObject);
	}

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

	plasma_module = rb_define_module("Plasma");
    rb_define_singleton_method(plasma_module, "method_missing", (VALUE (*) (...)) kde_module_method_missing, -1);
    rb_define_singleton_method(plasma_module, "const_missing", (VALUE (*) (...)) kde_module_method_missing, -1);

	kwin_class = rb_define_class_under(kde_module, "Win", qt_base_class);

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

static VALUE
set_application_terminated(VALUE /*self*/, VALUE yn)
{
    application_terminated = (yn == Qtrue ? true : false);
	return Qnil;
}

extern Q_DECL_EXPORT void
Init_qtruby4()
{
	if (qt_Smoke != 0L) {
		// This function must have been called twice because both
		// 'require Qt' and 'require Korundum' statements have
		// been included in a ruby program
		rb_fatal("require 'Qt' must not follow require 'Korundum'\n");
		return;
	}

    init_qt_Smoke();
    qt_Smoke->binding = new QtRubySmokeBinding(qt_Smoke);
    install_handlers(Qt_handlers);

	if (qt_module == Qnil) {
		qt_module = rb_define_module("Qt");
		qt_internal_module = rb_define_module_under(qt_module, "Internal");
		qt_base_class = rb_define_class_under(qt_module, "Base", rb_cObject);
	}

    rb_define_singleton_method(qt_base_class, "new", (VALUE (*) (...)) new_qt, -1);
    rb_define_method(qt_base_class, "initialize", (VALUE (*) (...)) initialize_qt, -1);
    rb_define_singleton_method(qt_base_class, "method_missing", (VALUE (*) (...)) class_method_missing, -1);
    rb_define_singleton_method(qt_module, "method_missing", (VALUE (*) (...)) module_method_missing, -1);
    rb_define_method(qt_base_class, "method_missing", (VALUE (*) (...)) method_missing, -1);

    rb_define_singleton_method(qt_base_class, "const_missing", (VALUE (*) (...)) class_method_missing, -1);
    rb_define_singleton_method(qt_module, "const_missing", (VALUE (*) (...)) module_method_missing, -1);
    rb_define_method(qt_base_class, "const_missing", (VALUE (*) (...)) method_missing, -1);

    rb_define_method(qt_base_class, "dispose", (VALUE (*) (...)) dispose, 0);
    rb_define_method(qt_base_class, "isDisposed", (VALUE (*) (...)) is_disposed, 0);
    rb_define_method(qt_base_class, "disposed?", (VALUE (*) (...)) is_disposed, 0);

	rb_define_method(qt_base_class, "qVariantValue", (VALUE (*) (...)) qvariant_value, 2);
	rb_define_method(qt_base_class, "qVariantFromValue", (VALUE (*) (...)) qvariant_from_value, -1);
    
	rb_define_method(rb_cObject, "qDebug", (VALUE (*) (...)) qdebug, 1);
	rb_define_method(rb_cObject, "qFatal", (VALUE (*) (...)) qfatal, 1);
	rb_define_method(rb_cObject, "qWarning", (VALUE (*) (...)) qwarning, 1);

    rb_define_module_function(qt_internal_module, "getMethStat", (VALUE (*) (...)) getMethStat, 0);
    rb_define_module_function(qt_internal_module, "getClassStat", (VALUE (*) (...)) getClassStat, 0);
    rb_define_module_function(qt_internal_module, "getIsa", (VALUE (*) (...)) getIsa, 1);
    rb_define_module_function(qt_internal_module, "allocateMocArguments", (VALUE (*) (...)) allocateMocArguments, 1);
    rb_define_module_function(qt_internal_module, "setMocType", (VALUE (*) (...)) setMocType, 4);
    rb_define_module_function(qt_internal_module, "setDebug", (VALUE (*) (...)) setDebug, 1);
    rb_define_module_function(qt_internal_module, "debug", (VALUE (*) (...)) debugging, 0);
    rb_define_module_function(qt_internal_module, "getTypeNameOfArg", (VALUE (*) (...)) getTypeNameOfArg, 2);
    rb_define_module_function(qt_internal_module, "classIsa", (VALUE (*) (...)) classIsa, 2);
    rb_define_module_function(qt_internal_module, "isEnum", (VALUE (*) (...)) isEnum, 1);
    rb_define_module_function(qt_internal_module, "insert_pclassid", (VALUE (*) (...)) insert_pclassid, 2);
    rb_define_module_function(qt_internal_module, "find_pclassid", (VALUE (*) (...)) find_pclassid, 1);
    rb_define_module_function(qt_internal_module, "getVALUEtype", (VALUE (*) (...)) getVALUEtype, 1);

    rb_define_module_function(qt_internal_module, "make_metaObject", (VALUE (*) (...)) make_metaObject, 4);
    rb_define_module_function(qt_internal_module, "addMetaObjectMethods", (VALUE (*) (...)) add_metaobject_methods, 1);
    rb_define_module_function(qt_internal_module, "addSignalMethods", (VALUE (*) (...)) add_signal_methods, 2);
    rb_define_module_function(qt_internal_module, "mapObject", (VALUE (*) (...)) mapObject, 1);
    // isQOjbect => isaQObject
    rb_define_module_function(qt_internal_module, "isQObject", (VALUE (*) (...)) isaQObject, 1);
    rb_define_module_function(qt_internal_module, "idInstance", (VALUE (*) (...)) idInstance, 1);
    rb_define_module_function(qt_internal_module, "idClass", (VALUE (*) (...)) idClass, 1);
    rb_define_module_function(qt_internal_module, "idMethodName", (VALUE (*) (...)) idMethodName, 1);
    rb_define_module_function(qt_internal_module, "idMethod", (VALUE (*) (...)) idMethod, 2);
    rb_define_module_function(qt_internal_module, "findMethod", (VALUE (*) (...)) findMethod, 2);
    rb_define_module_function(qt_internal_module, "findAllMethods", (VALUE (*) (...)) findAllMethods, -1);
    rb_define_module_function(qt_internal_module, "findAllMethodNames", (VALUE (*) (...)) findAllMethodNames, 3);
    rb_define_module_function(qt_internal_module, "dumpCandidates", (VALUE (*) (...)) dumpCandidates, 1);
    rb_define_module_function(qt_internal_module, "isObject", (VALUE (*) (...)) isObject, 1);
    rb_define_module_function(qt_internal_module, "setCurrentMethod", (VALUE (*) (...)) setCurrentMethod, 1);
    rb_define_module_function(qt_internal_module, "getClassList", (VALUE (*) (...)) getClassList, 0);
    rb_define_module_function(qt_internal_module, "create_qt_class", (VALUE (*) (...)) create_qt_class, 1);
    rb_define_module_function(qt_internal_module, "create_qobject_class", (VALUE (*) (...)) create_qobject_class, 1);
    rb_define_module_function(qt_internal_module, "cast_object_to", (VALUE (*) (...)) cast_object_to, 2);
    rb_define_module_function(qt_internal_module, "kross2smoke", (VALUE (*) (...)) kross2smoke, 2);
    rb_define_module_function(qt_internal_module, "smoke2kross", (VALUE (*) (...)) smoke2kross, 1);
    rb_define_module_function(qt_internal_module, "application_terminated=", (VALUE (*) (...)) set_application_terminated, 1);
    
	rb_define_module_function(qt_module, "version", (VALUE (*) (...)) version, 0);
    rb_define_module_function(qt_module, "qtruby_version", (VALUE (*) (...)) qtruby_version, 0);

    rb_define_module_function(qt_module, "qRegisterResourceData", (VALUE (*) (...)) q_register_resource_data, 4);
    rb_define_module_function(qt_module, "qUnregisterResourceData", (VALUE (*) (...)) q_unregister_resource_data, 4);

	rb_require("Qt/qtruby4.rb");

    // Do package initialization
    rb_funcall(qt_internal_module, rb_intern("init_all_classes"), 0);
}

}
