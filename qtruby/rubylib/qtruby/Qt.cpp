/***************************************************************************
                          Qt.cpp  -  description
                             -------------------
    begin                : Fri Jul 4 2003
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

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <stdio.h>
#include <stdarg.h>

#include <qglobal.h>
#include <qregexp.h>
#include <qstring.h>
#include <qptrdict.h>
#include <qintdict.h>
#include <qapplication.h>
#include <qmetaobject.h>
#include <private/qucomextra_p.h>
#include <qvariant.h>
#include <qcursor.h>
#include <qobjectlist.h>
#include <qsignalslotimp.h>

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

// #define DEBUG

#define QTRUBY_VERSION "1.0.7"

extern Smoke *qt_Smoke;
extern void init_qt_Smoke();
extern void smokeruby_mark(void * ptr);
extern void smokeruby_free(void * ptr);

#ifdef DEBUG
int do_debug = qtdb_gc;
#else
int do_debug = qtdb_none;
#endif

QPtrDict<VALUE> pointer_map(2179);
int object_count = 0;

QAsciiDict<Smoke::Index> methcache(2179);
QAsciiDict<Smoke::Index> classcache(2179);
// Maps from a classname in the form Qt::Widget to an int id
QIntDict<char> classname(2179);

extern "C" {
VALUE qt_module = Qnil;
VALUE kde_module = Qnil;
VALUE kparts_module = Qnil;
VALUE kio_module = Qnil;
VALUE kns_module = Qnil;
VALUE dom_module = Qnil;
VALUE kontact_module = Qnil;
VALUE kate_module = Qnil;
VALUE ktexteditor_module = Qnil;
VALUE qt_internal_module = Qnil;
VALUE qt_base_class = Qnil;
VALUE qt_qmetaobject_class = Qnil;
VALUE kconfigskeleton_class = Qnil;
VALUE kconfigskeleton_itemenum_class = Qnil;
VALUE kconfigskeleton_itemenum_choice_class = Qnil;
VALUE kio_udsatom_class = Qnil;
VALUE kwin_class = Qnil;
bool application_terminated = FALSE;
};

#define logger logger_backend
void logger_backend(const char *format, ...) __attribute__ ((format (printf, 1, 2)));
void rb_str_catf(VALUE self, const char *format, ...) __attribute__ ((format (printf, 2, 3)));

static VALUE (*_new_kde)(int, VALUE *, VALUE) = 0;
static VALUE (*_kconfigskeletonitem_immutable)(VALUE) = 0;

Smoke::Index _current_method = 0;

extern TypeHandler Qt_handlers[];
void install_handlers(TypeHandler *);

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
    if(strcmp(smoke->classes[classId].className, "QObject") == 0)
	return true;
    for(Smoke::Index *p = smoke->inheritanceList + smoke->classes[classId].parents;
	*p;
	p++) {
	if(isQObject(smoke, *p))
	    return true;
    }
    return false;
}

bool isDerivedFrom(Smoke *smoke, Smoke::Index classId, Smoke::Index baseId) {
    if(classId == 0 && baseId == 0)
	return false;
    if(classId == baseId)
	return true;
    for(Smoke::Index *p = smoke->inheritanceList + smoke->classes[classId].parents;
	*p;
	p++) {
	if(isDerivedFrom(smoke, *p, baseId))
	    return true;
    }
    return false;
}

bool isDerivedFromByName(Smoke *smoke, const char *className, const char *baseClassName) {
    if(!smoke || !className || !baseClassName)
	return false;
    Smoke::Index idClass = smoke->idClass(className);
    Smoke::Index idBase = smoke->idClass(baseClassName);
    return isDerivedFrom(smoke, idClass, idBase);
}

VALUE getPointerObject(void *ptr) {
	if (pointer_map[ptr] == 0) {
	    return Qnil;
	} else {
		return *(pointer_map[ptr]);
	}
}

void unmapPointer(smokeruby_object *o, Smoke::Index classId, void *lastptr) {
    void *ptr = o->smoke->cast(o->ptr, o->classId, classId);
    if(ptr != lastptr) {
	lastptr = ptr;
	if (pointer_map[ptr] != 0) {
		VALUE * obj_ptr = pointer_map[ptr];
		
		if (do_debug & qtdb_gc) {
			const char *className = o->smoke->classes[o->classId].className;
			logger("unmapPointer (%s*)%p -> %p", className, ptr, obj_ptr);
		}
	    
		pointer_map.remove(ptr);
		free((void*) obj_ptr);
	}
    }
    for(Smoke::Index *i = o->smoke->inheritanceList + o->smoke->classes[classId].parents;
	*i;
	i++) {
	unmapPointer(o, *i, lastptr);
    }
}

// Store pointer in pointer_map hash : "pointer_to_Qt_object" => weak ref to associated Ruby object
// Recurse to store it also as casted to its parent classes.

void mapPointer(VALUE obj, smokeruby_object *o, Smoke::Index classId, void *lastptr) {
    void *ptr = o->smoke->cast(o->ptr, o->classId, classId);
	
    if (ptr != lastptr) {
		lastptr = ptr;
		VALUE * obj_ptr = (VALUE *) malloc(sizeof(VALUE));
		memcpy(obj_ptr, &obj, sizeof(VALUE));
		
		if (do_debug & qtdb_gc) {
			const char *className = o->smoke->classes[o->classId].className;
			logger("mapPointer (%s*)%p -> %p", className, ptr, (void*)obj);
		}
	
		pointer_map.insert(ptr, obj_ptr);
    }
	
    for(Smoke::Index *i = o->smoke->inheritanceList + o->smoke->classes[classId].parents;
	*i;
	i++) {
	mapPointer(obj, o, *i, lastptr);
    }
	
	return;
}

Marshall::HandlerFn getMarshallFn(const SmokeType &type);

class VirtualMethodReturnValue : public Marshall {
    Smoke *_smoke;
    Smoke::Index _method;
    Smoke::Stack _stack;
    SmokeType _st;
    VALUE _retval;
public:
    const Smoke::Method &method() { return _smoke->methods[_method]; }
    SmokeType type() { return _st; }
    Marshall::Action action() { return Marshall::FromVALUE; }
    Smoke::StackItem &item() { return _stack[0]; }
    VALUE * var() { return &_retval; }

    void unsupported() {
	rb_raise(rb_eArgError, "Cannot handle '%s' as return-type of virtual method %s::%s",
		type().name(),
		_smoke->className(method().classId),
		_smoke->methodNames[method().name]);
    }

    Smoke *smoke() { return _smoke; }
    void next() {}
    bool cleanup() { return false; }

    VirtualMethodReturnValue(Smoke *smoke, Smoke::Index meth, Smoke::Stack stack, VALUE retval) :
    _smoke(smoke), _method(meth), _stack(stack), _retval(retval) {
	_st.set(_smoke, method().ret);
	Marshall::HandlerFn fn = getMarshallFn(type());
	(*fn)(this);
   }
};

class VirtualMethodCall : public Marshall {
    Smoke *_smoke;
    Smoke::Index _method;
    Smoke::Stack _stack;
    VALUE _obj;
    int _cur;
    Smoke::Index *_args;
    VALUE *_sp;
    bool _called;

public:
    SmokeType type() { return SmokeType(_smoke, _args[_cur]); }
    Marshall::Action action() { return Marshall::ToVALUE; }
    Smoke::StackItem &item() { return _stack[_cur + 1]; }
    VALUE * var() { return _sp + _cur; }
    const Smoke::Method &method() { return _smoke->methods[_method]; }
    void unsupported() {
	rb_raise(rb_eArgError, "Cannot handle '%s' as argument of virtual method %s::%s",
		type().name(),
		_smoke->className(method().classId),
		_smoke->methodNames[method().name]);
    }
    Smoke *smoke() { return _smoke; }
    void callMethod() {
	if(_called) return;
	_called = true;
	
	VALUE _retval = rb_funcall2(_obj,
				    rb_intern(_smoke->methodNames[method().name]),
				    method().numArgs,
				    _sp );
	VirtualMethodReturnValue r(_smoke, _method, _stack, _retval);
    }

    void next() {
	int oldcur = _cur;
	_cur++;
	while(!_called && _cur < method().numArgs) {
	    Marshall::HandlerFn fn = getMarshallFn(type());
	    (*fn)(this);
	    _cur++;
	}
	callMethod();
	_cur = oldcur;
    }

    bool cleanup() { return false; }   // is this right?

    VirtualMethodCall(Smoke *smoke, Smoke::Index meth, Smoke::Stack stack, VALUE obj) :
    _smoke(smoke), _method(meth), _stack(stack), _obj(obj), _cur(-1), _sp(0), _called(false) {
	_sp = (VALUE *) calloc(method().numArgs, sizeof(VALUE));
	
	_args = _smoke->argumentList + method().args;
    }

    ~VirtualMethodCall() {
		free(_sp);
    }
};

class MethodReturnValue : public Marshall {
    Smoke *_smoke;
    Smoke::Index _method;
    VALUE * _retval;
    Smoke::Stack _stack;
public:
    MethodReturnValue(Smoke *smoke, Smoke::Index method, Smoke::Stack stack, VALUE * retval) :
    _smoke(smoke), _method(method), _retval(retval), _stack(stack) {
	Marshall::HandlerFn fn = getMarshallFn(type());
	(*fn)(this);
    }

    const Smoke::Method &method() { return _smoke->methods[_method]; }
    SmokeType type() { return SmokeType(_smoke, method().ret); }
    Marshall::Action action() { return Marshall::ToVALUE; }
    Smoke::StackItem &item() { return _stack[0]; }
    VALUE * var() {
    	return _retval;
    }
    void unsupported() {
	rb_raise(rb_eArgError, "Cannot handle '%s' as return-type of %s::%s",
		type().name(),
		strcmp(_smoke->className(method().classId), "QGlobalSpace") == 0 ? "" : _smoke->className(method().classId),
		_smoke->methodNames[method().name]);
    }
    Smoke *smoke() { return _smoke; }
    void next() {}
    bool cleanup() { return false; }
};

class MethodCall : public Marshall {
    int _cur;
    Smoke *_smoke;
    Smoke::Stack _stack;
    Smoke::Index _method;
    Smoke::Index *_args;
	VALUE _target;
	void *_current_object;
	Smoke::Index _current_object_class;
    VALUE *_sp;
    int _items;
    VALUE _retval;
    bool _called;
public:
    MethodCall(Smoke *smoke, Smoke::Index method, VALUE target, VALUE *sp, int items) :
	_cur(-1), _smoke(smoke), _method(method), _target(target), _current_object(0), _sp(sp), _items(items), _called(false)
    {
	
	if (_target != Qnil) {
	    smokeruby_object *o = value_obj_info(_target);
		if (o && o->ptr) {
		    _current_object = o->ptr;
		    _current_object_class = o->classId;
		}
	}
	
	_args = _smoke->argumentList + _smoke->methods[_method].args;
	_items = _smoke->methods[_method].numArgs;
	_stack = new Smoke::StackItem[items + 1];
	_retval = Qnil;
    }

    ~MethodCall() {
	delete[] _stack;
    }

    SmokeType type() {
    	return SmokeType(_smoke, _args[_cur]);
    }

    Marshall::Action action() {
    	return Marshall::FromVALUE;
    }
    Smoke::StackItem &item() {
    	return _stack[_cur + 1];
    }

    VALUE * var() {
	if(_cur < 0) return &_retval;
	return _sp + _cur;
    }

    inline const Smoke::Method &method() {
    	return _smoke->methods[_method];
    }

    void unsupported() {
    	if (strcmp(_smoke->className(method().classId), "QGlobalSpace") == 0) {
			rb_raise(rb_eArgError, "Cannot handle '%s' as argument to %s",
				type().name(),
				_smoke->methodNames[method().name]);
		} else {
			rb_raise(rb_eArgError, "Cannot handle '%s' as argument to %s::%s",
				type().name(),
				_smoke->className(method().classId),
				_smoke->methodNames[method().name]);
		}
    }

    Smoke *smoke() {
    	return _smoke;
    }

    inline void callMethod() {
	if(_called) return;
	_called = true;
		
	QString className(_smoke->className(method().classId));
	
	if (	! className.endsWith(_smoke->methodNames[method().name])
			&& TYPE(_target) != T_DATA 
			&& _target != Qnil
			&& !(method().flags & Smoke::mf_static) ) 
	{
		rb_raise(rb_eArgError, "Instance is not initialized, cannot call %s", 
					_smoke->methodNames[method().name]);
	}
	
	if (_target == Qnil && !(method().flags & Smoke::mf_static)) {
		rb_raise(rb_eArgError, "%s is not a class method\n", _smoke->methodNames[method().name]);
	}
	
	Smoke::ClassFn fn = _smoke->classes[method().classId].classFn;
	void *ptr = _smoke->cast(_current_object, _current_object_class, method().classId);
	_items = -1;
	(*fn)(method().method, ptr, _stack);
	MethodReturnValue r(_smoke, _method, _stack, &_retval);
    }

    void next() {
	int oldcur = _cur;
	_cur++;

	while(!_called && _cur < _items) {
	    Marshall::HandlerFn fn = getMarshallFn(type());
	    (*fn)(this);
	    _cur++;
	}
	callMethod();
	_cur = oldcur;
    }

    bool cleanup() {
    	return true;
    }
};

class UnencapsulatedQObject : public QObject {
public:
    QConnectionList *public_receivers(int signal) const { return receivers(signal); }
    void public_activate_signal(QConnectionList *clist, QUObject *o) { activate_signal(clist, o); }
};

class EmitSignal : public Marshall {
    UnencapsulatedQObject *_qobj;
    int _id;
    MocArgument *_args;
    VALUE *_sp;
    int _items;
    int _cur;
    Smoke::Stack _stack;
    bool _called;
public:
    EmitSignal(QObject *qobj, int id, int items, VALUE args, VALUE *sp) :
    _qobj((UnencapsulatedQObject*)qobj), _id(id), _sp(sp), _items(items),
    _cur(-1), _called(false)
    {
	_items = NUM2INT(rb_ary_entry(args, 0));
	Data_Get_Struct(rb_ary_entry(args, 1), MocArgument, _args);
	_stack = new Smoke::StackItem[_items];
    }
    ~EmitSignal() {
	delete[] _stack;
    }
    const MocArgument &arg() { return _args[_cur]; }
    SmokeType type() { return arg().st; }
    Marshall::Action action() { return Marshall::FromVALUE; }
    Smoke::StackItem &item() { return _stack[_cur]; }
    VALUE * var() { return _sp + _cur; }
    void unsupported() {
	rb_raise(rb_eArgError, "Cannot handle '%s' as signal argument", type().name());
    }
    Smoke *smoke() { return type().smoke(); }
    void emitSignal() {
	if(_called) return;
	_called = true;

	QConnectionList *clist = _qobj->public_receivers(_id);
	if(!clist) return;

	QUObject *o = new QUObject[_items + 1];
	for(int i = 0; i < _items; i++) {
	    QUObject *po = o + i + 1;
	    Smoke::StackItem *si = _stack + i;
	    switch(_args[i].argType) {
	      case xmoc_bool:
		static_QUType_bool.set(po, si->s_bool);
		break;
	      case xmoc_int:
		static_QUType_int.set(po, si->s_int);
		break;
	      case xmoc_double:
		static_QUType_double.set(po, si->s_double);
		break;
	      case xmoc_charstar:
		static_QUType_charstar.set(po, (char*)si->s_voidp);
		break;
	      case xmoc_QString:
		static_QUType_QString.set(po, *(QString*)si->s_voidp);
		break;
	      default:
		{
		    const SmokeType &t = _args[i].st;
		    void *p;
		    switch(t.elem()) {
		      case Smoke::t_bool:
			p = &si->s_bool;
			break;
		      case Smoke::t_char:
			p = &si->s_char;
			break;
		      case Smoke::t_uchar:
			p = &si->s_uchar;
			break;
		      case Smoke::t_short:
			p = &si->s_short;
			break;
		      case Smoke::t_ushort:
			p = &si->s_ushort;
			break;
		      case Smoke::t_int:
			p = &si->s_int;
			break;
		      case Smoke::t_uint:
			p = &si->s_uint;
			break;
		      case Smoke::t_long:
			p = &si->s_long;
			break;
		      case Smoke::t_ulong:
			p = &si->s_ulong;
			break;
		      case Smoke::t_float:
			p = &si->s_float;
			break;
		      case Smoke::t_double:
			p = &si->s_double;
			break;
		      case Smoke::t_enum:
			{
			    // allocate a new enum value
			    Smoke::EnumFn fn = SmokeClass(t).enumFn();
			    if(!fn) {
				rb_warning("Unknown enumeration %s\n", t.name());
				p = new int((int)si->s_enum);
				break;
			    }
			    Smoke::Index id = t.typeId();
			    (*fn)(Smoke::EnumNew, id, p, si->s_enum);
			    (*fn)(Smoke::EnumFromLong, id, p, si->s_enum);
			    // FIXME: MEMORY LEAK
			}
			break;
		      case Smoke::t_class:
		      case Smoke::t_voidp:
			p = si->s_voidp;
			break;
		      default:
			p = 0;
			break;
		    }
		    static_QUType_ptr.set(po, p);
		}
	    }
	}

	_qobj->public_activate_signal(clist, o);
        delete[] o;
    }
    void next() {
	int oldcur = _cur;
	_cur++;

	while(!_called && _cur < _items) {
	    Marshall::HandlerFn fn = getMarshallFn(type());
	    (*fn)(this);
	    _cur++;
	}

	emitSignal();
	_cur = oldcur;
    }
    bool cleanup() { return true; }
};

class InvokeSlot : public Marshall {
    VALUE _obj;
    ID _slotname;
    int _items;
    MocArgument *_args;
    QUObject *_o;
    int _cur;
    bool _called;
    VALUE *_sp;
    Smoke::Stack _stack;
public:
    const MocArgument &arg() { return _args[_cur]; }
    SmokeType type() { return arg().st; }
    Marshall::Action action() { return Marshall::ToVALUE; }
    Smoke::StackItem &item() { return _stack[_cur]; }
    VALUE * var() { return _sp + _cur; }
    Smoke *smoke() { return type().smoke(); }
    bool cleanup() { return false; }
    void unsupported() {
	rb_raise(rb_eArgError, "Cannot handle '%s' as slot argument\n", type().name());
    }
    void copyArguments() {
	for(int i = 0; i < _items; i++) {
	    QUObject *o = _o + i + 1;
	    switch(_args[i].argType) {
	      case xmoc_bool:
		_stack[i].s_bool = static_QUType_bool.get(o);
		break;
	      case xmoc_int:
		_stack[i].s_int = static_QUType_int.get(o);
		break;
	      case xmoc_double:
		_stack[i].s_double = static_QUType_double.get(o);
		break;
	      case xmoc_charstar:
		_stack[i].s_voidp = static_QUType_charstar.get(o);
		break;
	      case xmoc_QString:
		_stack[i].s_voidp = &static_QUType_QString.get(o);
		break;
	      default:	// case xmoc_ptr:
		{
		    const SmokeType &t = _args[i].st;
		    void *p = static_QUType_ptr.get(o);
		    switch(t.elem()) {
		      case Smoke::t_bool:
			_stack[i].s_bool = *(bool*)p;
			break;
		      case Smoke::t_char:
			_stack[i].s_char = *(char*)p;
			break;
		      case Smoke::t_uchar:
			_stack[i].s_uchar = *(unsigned char*)p;
			break;
		      case Smoke::t_short:
			_stack[i].s_short = *(short*)p;
			break;
		      case Smoke::t_ushort:
			_stack[i].s_ushort = *(unsigned short*)p;
			break;
		      case Smoke::t_int:
			_stack[i].s_int = *(int*)p;
			break;
		      case Smoke::t_uint:
			_stack[i].s_uint = *(unsigned int*)p;
			break;
		      case Smoke::t_long:
			_stack[i].s_long = *(long*)p;
			break;
		      case Smoke::t_ulong:
			_stack[i].s_ulong = *(unsigned long*)p;
			break;
		      case Smoke::t_float:
			_stack[i].s_float = *(float*)p;
			break;
		      case Smoke::t_double:
			_stack[i].s_double = *(double*)p;
			break;
		      case Smoke::t_enum:
			{
			    Smoke::EnumFn fn = SmokeClass(t).enumFn();
			    if(!fn) {
				rb_warning("Unknown enumeration %s\n", t.name());
				_stack[i].s_enum = *(int*)p;
				break;
			    }
			    Smoke::Index id = t.typeId();
			    (*fn)(Smoke::EnumToLong, id, p, _stack[i].s_enum);
			}
			break;
		      case Smoke::t_class:
		      case Smoke::t_voidp:
			_stack[i].s_voidp = p;
			break;
		    }
		}
	    }
	}
    }
    void invokeSlot() {
	if(_called) return;
	_called = true;

        (void) rb_funcall2(_obj, _slotname, _items, _sp);
    }

    void next() {
	int oldcur = _cur;
	_cur++;

	while(!_called && _cur < _items) {
	    Marshall::HandlerFn fn = getMarshallFn(type());
	    (*fn)(this);
	    _cur++;
	}

	invokeSlot();
	_cur = oldcur;
    }

    InvokeSlot(VALUE obj, ID slotname, VALUE args, QUObject *o) :
    _obj(obj), _slotname(slotname), _o(o), _cur(-1), _called(false)
    {
	_items = NUM2INT(rb_ary_entry(args, 0));
	Data_Get_Struct(rb_ary_entry(args, 1), MocArgument, _args);
	_sp = (VALUE *) calloc(_items, sizeof(VALUE));
	_stack = new Smoke::StackItem[_items];
	copyArguments();
    }

    ~InvokeSlot() {
	delete[] _stack;
	free(_sp);
    }
};

class QtRubySmokeBinding : public SmokeBinding {
public:
    QtRubySmokeBinding(Smoke *s) : SmokeBinding(s) {}

    void deleted(Smoke::Index classId, void *ptr) {
	VALUE obj = getPointerObject(ptr);
	smokeruby_object *o = value_obj_info(obj);
	if(do_debug & qtdb_gc) {
	    logger("%p->~%s()", ptr, smoke->className(classId));
	}
	if(!o || !o->ptr) {
	    return;
	}
	unmapPointer(o, o->classId, 0);
	o->ptr = 0;
    }

    bool callMethod(Smoke::Index method, void *ptr, Smoke::Stack args, bool /*isAbstract*/) {
	VALUE obj = getPointerObject(ptr);
	smokeruby_object *o = value_obj_info(obj);
	if(do_debug & qtdb_virtual) 
	    logger("virtual %p->%s::%s() called", ptr,
		    smoke->classes[smoke->methods[method].classId].className,
		    smoke->methodNames[smoke->methods[method].name]
		    );

	if(!o) {
	    if( do_debug & qtdb_virtual )   // if not in global destruction
		logger("Cannot find object for virtual method %p -> %p", ptr, &obj);
	    return false;
	}

	const char *methodName = smoke->methodNames[smoke->methods[method].name];
	
	// If the virtual method hasn't been overriden, just call the C++ one.
	// Special case open() to avoid a clash with the Kernel.open() method	
	if (	rb_respond_to(obj, rb_intern(methodName)) == 0
			|| strcmp(methodName, "open") == 0 ) 
	{
	    return false;
	}
	
	VirtualMethodCall c(smoke, method, args, obj);
	c.next();
	return true;
    }

    char *className(Smoke::Index classId) {
		return classname.find((int) classId);
    }
};

void rb_str_catf(VALUE self, const char *format, ...) 
{
    va_list ap;
    va_start(ap, format);
    char *p = 0;
    int len;
    if (len = vasprintf(&p, format, ap), len != -1) {
	rb_str_cat(self, p, len);
	free(p);
    }
    va_end(ap);
}

void logger_backend(const char *format, ...) 
{
    va_list ap;
    va_start(ap, format);
    char *p = 0;
    int len;
    VALUE val_str = rb_str_new2("");
    if (len = vasprintf(&p, format, ap), len != -1) {
	rb_str_cat(val_str, p, len);
	free(p);
    }
    // TODO - allow qtruby programs to override this fprintf with their own logging
    fprintf(stdout, "%s\n", StringValuePtr(val_str));
	fflush(stdout);
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

VALUE
set_obj_info(const char * className, smokeruby_object * o)
{
    VALUE klass = rb_funcall(qt_internal_module,
			     rb_intern("find_class"),
			     1,
			     rb_str_new2(className) );
	Smoke::Index *r = classcache.find(className);
	if (r != 0) {
		o->classId = (int)*r;
	}
    VALUE obj = Data_Wrap_Struct(klass, smokeruby_mark, smokeruby_free, (void *) o);
    return obj;
}

static VALUE mapObject(VALUE self, VALUE obj);

VALUE
cast_object_to(VALUE /*self*/, VALUE object, VALUE new_klassname)
{
    smokeruby_object *o = value_obj_info(object);
    VALUE klass = rb_funcall(qt_internal_module, rb_intern("find_class"), 1, new_klassname );
    if (klass == Qnil)
	rb_raise(rb_eArgError, "unable to find class to cast to\n");

    char *casted_klassname = rb_class2name(klass);
    char *blah = (char*) malloc((strlen(casted_klassname) - strlen("DE::")) * sizeof(char));
    strcpy(blah, ""); // strcat(blah, "K");
    strcat(blah, casted_klassname + strlen("KDE::"));

    smokeruby_object *o_cast = (smokeruby_object *) malloc(sizeof(smokeruby_object));
    memcpy(o_cast, o, sizeof(smokeruby_object));
    o_cast->allocated = false;
    int cast_to_id = o->smoke->idClass(blah);
    o_cast->ptr = o->smoke->cast(o_cast->ptr, o_cast->classId, cast_to_id);
    o_cast->classId = cast_to_id;

    VALUE obj = Data_Wrap_Struct(klass, 0, free, (void *) o_cast);
    mapPointer(obj, o_cast, o_cast->classId, 0);
    return obj;
}

const char *
get_VALUEtype(VALUE ruby_value)
{
	char * classname = rb_obj_classname(ruby_value);
    const char *r = "";
    if(ruby_value == Qnil)
	r = "u";
    else if(TYPE(ruby_value) == T_FIXNUM || TYPE(ruby_value) == T_BIGNUM || strcmp(classname, "Qt::Integer") == 0)
	r = "i";
    else if(TYPE(ruby_value) == T_FLOAT)
	r = "n";
    else if(TYPE(ruby_value) == T_STRING)
	r = "s";
    else if(strcmp(classname, "Qt::ByteArray") == 0)
	r = "b";
    else if(ruby_value == Qtrue || ruby_value == Qfalse || strcmp(classname, "Qt::Boolean") == 0)
	r = "B";
    else if(strcmp(classname, "Qt::Enum") == 0) {
	VALUE temp = rb_funcall(qt_internal_module, rb_intern("get_qenum_type"), 1, ruby_value);
	r = StringValuePtr(temp);
    } else if(TYPE(ruby_value) == T_DATA) {
	smokeruby_object *o = value_obj_info(ruby_value);
	if(!o) {
	    r = "a";
	} else {
	    r = o->smoke->classes[o->classId].className;
    }
	}
    else {
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

// Used to display debugging info about the signals a Qt::Object has connected.
// Returns a Hash with keys of the signals names, and values of Arrays of 
// Qt::Connections for the target slots
static VALUE
receivers_qobject(VALUE self)
{
	if (TYPE(self) != T_DATA) {
		return Qnil;
	}
		
	smokeruby_object * o = 0;
    Data_Get_Struct(self, smokeruby_object, o);	
	UnencapsulatedQObject * qobject = (UnencapsulatedQObject *) o->smoke->cast(o->ptr, o->classId, o->smoke->idClass("QObject"));
	VALUE result = rb_hash_new();
	QStrList signalNames = qobject->metaObject()->signalNames(true);
	
	for (int sig = 0; sig < qobject->metaObject()->numSignals(true); sig++) {
		QConnectionList * clist = qobject->public_receivers(sig);
		if (clist != 0) {
			VALUE name = rb_str_new2(signalNames.at(sig));
			VALUE members = rb_ary_new();
			
			for (	QConnection * connection = clist->first(); 
					connection != 0; 
					connection = clist->next() ) 
			{
				VALUE obj = getPointerObject(connection);
				if (obj == Qnil) {
					smokeruby_object * c = ALLOC(smokeruby_object);
					c->classId = o->smoke->idClass("QConnection");
					c->smoke = o->smoke;
					c->ptr = connection;
					c->allocated = false;
					obj = set_obj_info("Qt::Connection", c);
				}
				
				rb_ary_push(members, obj);
			}
			
			rb_hash_aset(result, name, members);
		}
	}
	
	return result;
}

// Takes a variable name and a QProperty with QVariant value, and returns a '
// variable=value' pair with the value in ruby inspect style
static QCString
inspectProperty(Smoke * smoke, const QMetaProperty * property, const char * name, QVariant & value)
{
	if (property->isEnumType()) {
		QMetaObject * metaObject = *(property->meta);
		return QCString().sprintf(	" %s=%s::%s",
									name,
									smoke->binding->className(smoke->idClass(metaObject->className())), 
									property->valueToKey(value.toInt()) );
	}
	
	switch (value.type()) {
	case QVariant::String:
	case QVariant::CString:
	{
		if (value.toString().isNull()) {
			return QCString().sprintf(" %s=nil", name); 
		} else {
			return QCString().sprintf(	" %s=\"%s\"", 
										name, 
										value.toString().latin1() );
		}
	}
		
	case QVariant::Bool:
	{
		QString rubyName;
		QRegExp name_re("^(is|has)(.)(.*)");
		
		if (name_re.search(name) != -1) {
			rubyName = name_re.cap(2).lower() + name_re.cap(3) + "?";
		} else {
			rubyName = name;
		}
		
		return QCString().sprintf(" %s=%s", rubyName.latin1(), value.toString().latin1());
	}
			
	case QVariant::Color:
	{
		QColor c = value.toColor();
		return QCString().sprintf(" %s=#<Qt::Color:0x0 %s>", name, c.name().latin1());
	}
			
	case QVariant::Cursor:
	{
		QCursor c = value.toCursor();
		return QCString().sprintf(" %s=#<Qt::Cursor:0x0 shape=%d>", name, c.shape());
	}
	
	case QVariant::Double:
	{
		return QCString().sprintf(" %s=%.4f", name, value.toDouble());
	}
	
	case QVariant::Font:
	{
		QFont f = value.toFont();
		return QCString().sprintf(	" %s=#<Qt::Font:0x0 family=%s, pointSize=%d, weight=%d, italic=%s, bold=%s, underline=%s, strikeOut=%s>", 
									name, 
									f.family().latin1(), f.pointSize(), f.weight(), 
									f.italic() ? "true" : "false", f.bold() ? "true" : "false",
									f.underline() ? "true" : "false", f.strikeOut() ? "true" : "false" );
	}
	
	case QVariant::Point:
	{
		QPoint p = value.toPoint();
		return QCString().sprintf(	" %s=#<Qt::Point:0x0 x=%d, y=%d>", 
									name, 
									p.x(), p.y() );
	}
	
	case QVariant::Rect:
	{
		QRect r = value.toRect();
		return QCString().sprintf(	" %s=#<Qt::Rect:0x0 left=%d, right=%d, top=%d, bottom=%d>", 
									name, 
									r.left(), r.right(), r.top(), r.bottom() );
	}
	
	case QVariant::Size:
	{
		QSize s = value.toSize();
		return QCString().sprintf(	" %s=#<Qt::Size:0x0 width=%d, height=%d>", 
									name, 
									s.width(), s.height() );
	}
	
	case QVariant::SizePolicy:
	{
		QSizePolicy s = value.toSizePolicy();
		return QCString().sprintf(	" %s=#<Qt::SizePolicy:0x0 horData=%d, verData=%d>", 
									name, 
									s.horData(), s.verData() );
	}
	
	case QVariant::Brush:
	case QVariant::ColorGroup:
	case QVariant::Image:
	case QVariant::Palette:
	case QVariant::Pixmap:
	case QVariant::Region:
	{
		return QCString().sprintf(" %s=#<Qt::%s:0x0>", name, value.typeName() + 1);
	}
	
	default:
		return QCString().sprintf(	" %s=%s", 
									name, 
									(value.isNull() || value.toString().isNull()) ? "nil" : value.toString().latin1() );
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
	
	QCString value_list;
	value_list.append(QCString().sprintf(" name=\"%s\"", qobject->name()));
	
	if (qobject->isWidgetType()) {
		QWidget * w = (QWidget *) qobject;
		value_list.append(QCString().sprintf(	", x=%d, y=%d, width=%d, height=%d", 
												w->x(),
												w->y(),
												w->width(),
												w->height() ) ); 
	}
		
	value_list.append(">");
	rb_str_cat(inspect_str, value_list.data(), strlen(value_list.data()));
	
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
	UnencapsulatedQObject * qobject = (UnencapsulatedQObject *) o->smoke->cast(o->ptr, o->classId, o->smoke->idClass("QObject"));
	QStrList names = qobject->metaObject()->propertyNames(true);
	
	QCString value_list;		
	
	if (qobject->parent() != 0) {
		QCString parentInspectString;
		VALUE obj = getPointerObject(qobject->parent());
		if (obj != Qnil) {
			VALUE parent_inspect_str = rb_funcall(obj, rb_intern("to_s"), 0, 0);	
			rb_str_resize(parent_inspect_str, RSTRING(parent_inspect_str)->len - 1);
			parentInspectString = StringValuePtr(parent_inspect_str);
		} else {
			parentInspectString.sprintf("#<%s:0x0", qobject->parent()->className());
		}
		
		if (qobject->parent()->isWidgetType()) {
			QWidget * w = (QWidget *) qobject->parent();
			value_list = QCString().sprintf(	"  parent=%s name=\"%s\", x=%d, y=%d, width=%d, height=%d>,\n", 
												parentInspectString.data(),
												w->name(),
												w->x(),
												w->y(),
												w->width(),
												w->height() );
		} else {
			value_list = QCString().sprintf(	"  parent=%s name=\"%s\">,\n", 
												parentInspectString.data(),
												qobject->parent()->name() );
		}
		
		rb_funcall(pp, rb_intern("text"), 1, rb_str_new2(value_list.data()));
	}
	
	if (qobject->children() != 0) {
		value_list = QCString().sprintf("  children=Array (%d element(s)),\n", qobject->children()->count());
		rb_funcall(pp, rb_intern("text"), 1, rb_str_new2(value_list.data()));
	}
	
	value_list = QCString("  metaObject=#<Qt::MetaObject:0x0");
	value_list.append(QCString().sprintf(" className=%s", qobject->metaObject()->className()));
	
	if (qobject->metaObject()->superClass() != 0) {
		value_list.append(QCString().sprintf(", superClass=#<Qt::MetaObject:0x0>", qobject->metaObject()->superClass()));
	}		
	
	if (qobject->metaObject()->numSignals() > 0) {
		value_list.append(QCString().sprintf(", signalNames=Array (%d element(s))", qobject->metaObject()->numSignals()));
	}		
	
	if (qobject->metaObject()->numSlots() > 0) {
		value_list.append(QCString().sprintf(", slotNames=Array (%d element(s))", qobject->metaObject()->numSlots()));
	}
	
	value_list.append(">,\n");
	rb_funcall(pp, rb_intern("text"), 1, rb_str_new2(value_list.data()));
				
	int signalCount = 0;
	for (int sig = 0; sig < qobject->metaObject()->numSignals(true); sig++) {
		QConnectionList * clist = qobject->public_receivers(sig);
		if (clist != 0) {
			signalCount++;
		}
	}
	
	if (signalCount > 0) {
		value_list = QCString().sprintf(" receivers=Hash (%d element(s)),\n", signalCount);
		rb_funcall(pp, rb_intern("text"), 1, rb_str_new2(value_list.data()));
	}		
	
	int	index = 0;
	const char * name = names.first();
	
	if (name != 0) {
		QVariant value = qobject->property(name);
		const QMetaProperty * property = qobject->metaObject()->property(index, true);
		value_list = " " + inspectProperty(o->smoke, property, name, value);
		rb_funcall(pp, rb_intern("text"), 1, rb_str_new2(value_list.data()));
		index++;
	
		for (	name = names.next(); 
				name != 0; 
				name = names.next(), index++ ) 
		{
			rb_funcall(pp, rb_intern("text"), 1, rb_str_new2(",\n"));
						
			value = qobject->property(name);
			property = qobject->metaObject()->property(index, true);
			value_list = " " + inspectProperty(o->smoke, property, name, value);
			rb_funcall(pp, rb_intern("text"), 1, rb_str_new2(value_list.data()));
		}
	}
	
	rb_funcall(pp, rb_intern("text"), 1, rb_str_new2(">"));
	
	return self;
}

static VALUE
metaObject(VALUE self)
{
    VALUE metaObject = rb_funcall(qt_internal_module, rb_intern("getMetaObject"), 1, self);
    return metaObject;
}

static QCString
find_cached_selector(int argc, VALUE * argv, VALUE klass, char * methodName)
{
    // Look in the cache
	QCString mcid(rb_class2name(klass));
	mcid += ';';
	mcid += methodName;
	for(int i=3; i<argc ; i++)
	{
		mcid += ';';
		mcid += get_VALUEtype(argv[i]);
	}
	
	Smoke::Index *rcid = methcache.find((const char *)mcid);
#ifdef DEBUG
	if (do_debug & qtdb_calls) logger("method_missing mcid: %s", (const char *) mcid);
#endif
	
	if (rcid) {
		// Got a hit
#ifdef DEBUG
		if (do_debug & qtdb_calls) logger("method_missing cache hit, mcid: %s", (const char *) mcid);
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
	QRegExp px("^.*[?]$");
	QString pred(rb_id2name(SYM2ID(argv[0])));
	if (px.search(pred) != -1) {
		smokeruby_object *o = value_obj_info(self);
		if(!o || !o->ptr) {
			rb_call_super(argc, argv);
		}
		
		// Drop the trailing '?'
		pred.replace(pred.length() - 1, 1, "");
		
		pred.replace(0, 1, pred.at(0).upper());
		pred.replace(0, 0, QString("is"));
		Smoke::Index meth = o->smoke->findMethod(o->smoke->classes[o->classId].className, pred.latin1());
		
		if (meth == 0) {
			pred.replace(0, 2, QString("has"));
			meth = o->smoke->findMethod(o->smoke->classes[o->classId].className, pred.latin1());
		}
		
		if (meth > 0) {
			methodName = (char *) pred.latin1();
		}
	}
		
	VALUE * temp_stack = (VALUE *) calloc(argc+3, sizeof(VALUE));
    temp_stack[0] = rb_str_new2("Qt");
    temp_stack[1] = rb_str_new2(methodName);
    temp_stack[2] = klass;
    temp_stack[3] = self;
    for (int count = 1; count < argc; count++) {
	temp_stack[count+3] = argv[count];
    }

	{
		QCString mcid = find_cached_selector(argc+3, temp_stack, klass, methodName);

		if (_current_method == -1) {
			// Find the C++ method to call. Do that from Ruby for now

			VALUE retval = rb_funcall2(qt_internal_module, rb_intern("do_method_missing"), argc+3, temp_stack);
			if (_current_method == -1) {
				QRegExp rx("^[-+%/|]$");
				QString op(rb_id2name(SYM2ID(argv[0])));
				if (rx.search(op) != -1) {
					// Look for operator methods of the form 'operator+=', 'operator-=' and so on..
					temp_stack[1] = rb_str_new2(op.append("=").latin1());
					retval = rb_funcall2(qt_internal_module, rb_intern("do_method_missing"), argc+3, temp_stack);
				}

				if (_current_method == -1) {
					free(temp_stack);
					rb_call_super(argc, argv);
				}
			}
			// Success. Cache result.
			methcache.insert((const char *)mcid, new Smoke::Index(_current_method));
		}
	}
	
    MethodCall c(qt_Smoke, _current_method, self, temp_stack+4, argc-1);
    c.next();
    VALUE result = *(c.var());
	free(temp_stack);
	
    return result;
}

static VALUE
class_method_missing(int argc, VALUE * argv, VALUE klass)
{
	VALUE result = Qnil;
	char * methodName = rb_id2name(SYM2ID(argv[0]));
	VALUE * temp_stack = (VALUE *) calloc(argc+3, sizeof(VALUE));
    temp_stack[0] = rb_str_new2("Qt");
    temp_stack[1] = rb_str_new2(methodName);
    temp_stack[2] = klass;
    temp_stack[3] = Qnil;
    for (int count = 1; count < argc; count++) {
	temp_stack[count+3] = argv[count];
    }

    {
		QCString mcid = find_cached_selector(argc+3, temp_stack, klass, methodName);

		if (_current_method == -1) {
			VALUE retval = rb_funcall2(qt_internal_module, rb_intern("do_method_missing"), argc+3, temp_stack);
         Q_UNUSED(retval);
			if (_current_method != -1) {
				// Success. Cache result.
				methcache.insert((const char *)mcid, new Smoke::Index(_current_method));
			}
		}
	}

    if (_current_method == -1) {
		QRegExp rx("[a-zA-Z]+");
		if (rx.search(methodName) == -1) {
			// If an operator method hasn't been found as an instance method,
			// then look for a class method - after 'op(self,a)' try 'self.op(a)' 
	    	VALUE * method_stack = (VALUE *) calloc(argc - 1, sizeof(VALUE));
	    	method_stack[0] = argv[0];
	    	for (int count = 1; count < argc - 1; count++) {
			method_stack[count] = argv[count+1];
    		}
			result = method_missing(argc-1, method_stack, argv[1]);
			free(method_stack);
			free(temp_stack);
			return result;
		} else {
			rb_call_super(argc, argv);
		}
    }

    MethodCall c(qt_Smoke, _current_method, Qnil, temp_stack+4, argc-1);
    c.next();
    result = *(c.var());
	free(temp_stack);
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

    VALUE * temp_stack = (VALUE *) calloc(argc+4, sizeof(VALUE));
    temp_stack[0] = rb_str_new2("Qt");
    temp_stack[1] = constructor_name;
    temp_stack[2] = klass;
    temp_stack[3] = self;
    for (int count = 0; count < argc; count++) {
	temp_stack[count+4] = argv[count];
    }

	{ 
		// Put this in a C block so that the mcid will be de-allocated at the end of the block,
		// rather than on f'n exit, to avoid the longjmp problem described below
		QCString mcid = find_cached_selector(argc+4, temp_stack, klass, rb_class2name(klass));

		if (_current_method == -1) {
			retval = rb_funcall2(qt_internal_module, rb_intern("do_method_missing"), argc+4, temp_stack);
			if (_current_method != -1) {
				// Success. Cache result.
				methcache.insert((const char *)mcid, new Smoke::Index(_current_method));
			}
		}
	}

	if (_current_method == -1) {
		free(temp_stack);
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
	smokeruby_object  * o = (smokeruby_object *) malloc(sizeof(smokeruby_object));
	memcpy(o, p, sizeof(smokeruby_object));
	p->ptr = 0;
	p->allocated = false;
	o->allocated = true;
    VALUE result = Data_Wrap_Struct(klass, smokeruby_mark, smokeruby_free, o);
    mapObject(result, result);
	free(temp_stack);
	// Off with a longjmp, never to return..
    rb_throw("newqt", result);
	/*NOTREACHED*/
	return self;
}

VALUE
new_qt(int argc, VALUE * argv, VALUE klass)
{
    VALUE * temp_stack = (VALUE *) calloc(argc + 1, sizeof(VALUE));
    temp_stack[0] = rb_obj_alloc(klass);
    for (int count = 0; count < argc; count++) {
	temp_stack[count+1] = argv[count];
    }

    VALUE result = rb_funcall2(qt_internal_module, rb_intern("try_initialize"), argc+1, temp_stack);

    if (rb_respond_to(result, rb_intern("initialize")) != 0) {
	rb_obj_call_init(result, argc, argv);
    }
	
	free(temp_stack);
    return result;
}

static VALUE qt_invoke(int argc, VALUE * argv, VALUE self);
static VALUE qt_signal(int argc, VALUE * argv, VALUE self);

VALUE
new_qobject(int argc, VALUE * argv, VALUE klass)
{
	// TODO: Don't do this everytime a new instance is created, just once..
	rb_define_method(klass, "qt_invoke", (VALUE (*) (...)) qt_invoke, -1);
	rb_define_method(klass, "qt_emit", (VALUE (*) (...)) qt_invoke, -1);
	rb_define_method(klass, "metaObject", (VALUE (*) (...)) metaObject, 0);
	VALUE signalNames = rb_funcall(qt_internal_module, rb_intern("getSignalNames"), 1, klass);
	for (long index = 0; index < RARRAY(signalNames)->len; index++) {
	    VALUE signal = rb_ary_entry(signalNames, index);
	    rb_define_method(klass, StringValuePtr(signal), (VALUE (*) (...)) qt_signal, -1);
	}

    return new_qt(argc, argv, klass);
}

static VALUE
new_qapplication(int argc, VALUE * argv, VALUE klass)
{
    VALUE result = Qnil;

    if (argc == 1 && TYPE(argv[0]) == T_ARRAY) {
	// Convert '(ARGV)' to '(NUM, [$0]+ARGV)'
	VALUE * local_argv = (VALUE *) calloc(argc + 1, sizeof(VALUE));
	VALUE temp = rb_ary_dup(argv[0]);
	rb_ary_unshift(temp, rb_gv_get("$0"));
	local_argv[0] = INT2NUM(RARRAY(temp)->len);
	local_argv[1] = temp;
	result = new_qobject(2, local_argv, klass);
	free(local_argv);
    } else {
	result = new_qobject(argc, argv, klass);
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


VALUE
getmetainfo(VALUE self, int &offset, int &index)
{
    char * signalname = rb_id2name(rb_frame_last_func());
    VALUE metaObject_value = rb_funcall(qt_internal_module, rb_intern("getMetaObject"), 1, self);

    smokeruby_object *ometa = value_obj_info(metaObject_value);
    if(!ometa) return 0;
    QMetaObject *metaobject = (QMetaObject*)ometa->ptr;

    offset = metaobject->signalOffset();

    VALUE signalInfo = rb_funcall(qt_internal_module, rb_intern("signalInfo"), 2, self, rb_str_new2(signalname));
    VALUE member = rb_ary_entry(signalInfo, 0);
    index = NUM2INT(rb_ary_entry(signalInfo, 1));
    return rb_funcall(qt_internal_module, rb_intern("getMocArguments"), 1, member);
}

VALUE
getslotinfo(VALUE self, int id, char *&slotname, int &index, bool isSignal = false)
{
    VALUE member;
	
    VALUE metaObject_value = rb_funcall(qt_internal_module, rb_intern("getMetaObject"), 1, self);
    smokeruby_object *ometa = value_obj_info(metaObject_value);
    if(!ometa) return Qnil;

    QMetaObject *metaobject = (QMetaObject*)ometa->ptr;

    int offset = isSignal ? metaobject->signalOffset() : metaobject->slotOffset();

    index = id - offset;   // where we at
    if(index < 0) return Qnil;

    if (isSignal) {
	member = rb_funcall(qt_internal_module, rb_intern("signalAt"), 2, self, INT2NUM(index));
    } else {
	member = rb_funcall(qt_internal_module, rb_intern("slotAt"), 2, self, INT2NUM(index));
    }

    VALUE mocArgs = rb_funcall(qt_internal_module, rb_intern("getMocArguments"), 1, member);
    slotname = StringValuePtr(member);

    return mocArgs;
}

static VALUE
qt_signal(int argc, VALUE * argv, VALUE self)
{
	smokeruby_object *o = value_obj_info(self);
    QObject *qobj = (QObject*)o->smoke->cast(
	o->ptr,
	o->classId,
	o->smoke->idClass("QObject")
    );
    if(qobj->signalsBlocked()) return Qfalse;

    int offset;
    int index;

    VALUE args = getmetainfo(self, offset, index);

    if(args == Qnil) return Qfalse;

    // Okay, we have the signal info. *whew*
    EmitSignal signal(qobj, offset + index, argc, args, argv);
    signal.next();

    return Qtrue;
}

static VALUE
qt_invoke(int argc, VALUE * argv, VALUE self)
{
    // Arguments: int id, QUObject *o
    int id = NUM2INT(argv[0]);
    QUObject *_o = 0;

    Data_Get_Struct(rb_ary_entry(argv[1], 0), QUObject, _o);
    if(_o == 0) {
    	rb_raise(rb_eRuntimeError, "Cannot create QUObject\n");
    }

    smokeruby_object *o = value_obj_info(self);
    (void) (QObject*)o->smoke->cast(
	o->ptr,
	o->classId,
	o->smoke->idClass("QObject")
    );

    // Now, I need to find out if this means me
    int index;
    char *slotname;
    bool isSignal = strcmp(rb_id2name(rb_frame_last_func()), "qt_emit") == 0;
    VALUE mocArgs = getslotinfo(self, id, slotname, index, isSignal);
    if(mocArgs == Qnil) {
		// No ruby slot or signal found, assume the target is a C++ one
		return rb_call_super(argc, argv);
    }

    QString name(slotname);
    name.replace(QRegExp("\\(.*"), "");
    InvokeSlot slot(self, rb_intern(name.latin1()), mocArgs, _o);
    slot.next();

    return Qtrue;
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
    char *name = StringValuePtr(name_value);
    char *static_type = StringValuePtr(static_type_value);
    Smoke::Index typeId = qt_Smoke->idType(name);
    if(!typeId) return Qfalse;
    MocArgument *arg = 0;
    Data_Get_Struct(ptr, MocArgument, arg);
    arg[idx].st.set(qt_Smoke, typeId);
    if(strcmp(static_type, "ptr") == 0)
	arg[idx].argType = xmoc_ptr;
    else if(strcmp(static_type, "bool") == 0)
	arg[idx].argType = xmoc_bool;
    else if(strcmp(static_type, "int") == 0)
	arg[idx].argType = xmoc_int;
    else if(strcmp(static_type, "double") == 0)
	arg[idx].argType = xmoc_double;
    else if(strcmp(static_type, "char*") == 0)
	arg[idx].argType = xmoc_charstar;
    else if(strcmp(static_type, "QString") == 0)
	arg[idx].argType = xmoc_QString;
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
    Smoke::Index classId = qt_Smoke->idClass(enumName);
	// If something is a smoke type but not a class it must be an enum.
	// Note this is true iff this function is called from qtruby.rb/checkarg()
	return (typeId > 0 && classId == 0 ? Qtrue : Qfalse);
}

static VALUE
insert_pclassid(VALUE self, VALUE p_value, VALUE ix_value)
{
    char *p = StringValuePtr(p_value);
    int ix = NUM2INT(ix_value);
    classcache.insert(p, new Smoke::Index((Smoke::Index)ix));
    classname.insert(ix, strdup(p));
    return self;
}

static VALUE
find_pclassid(VALUE /*self*/, VALUE p_value)
{
    char *p = StringValuePtr(p_value);
    Smoke::Index *r = classcache.find(p);
    if(r)
        return INT2NUM((int)*r);
    else
        return INT2NUM(0);
}

static VALUE
insert_mcid(VALUE self, VALUE mcid_value, VALUE ix_value)
{
    char *mcid = StringValuePtr(mcid_value);
    int ix = NUM2INT(ix_value);
    methcache.insert(mcid, new Smoke::Index((Smoke::Index)ix));
    return self;
}

static VALUE
find_mcid(VALUE /*self*/, VALUE mcid_value)
{
    char *mcid = StringValuePtr(mcid_value);
    Smoke::Index *r = methcache.find(mcid);
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

static VALUE
make_QUParameter(VALUE /*self*/, VALUE name_value, VALUE type_value, VALUE /*extra*/, VALUE inout_value)
{
    char *name = StringValuePtr(name_value);
    char *type = StringValuePtr(type_value);
    int inout = NUM2INT(inout_value);
    QUParameter *p = new QUParameter;
    p->name = new char[strlen(name) + 1];
    strcpy((char*)p->name, name);
    if(strcmp(type, "bool") == 0)
	p->type = &static_QUType_bool;
    else if(strcmp(type, "int") == 0)
	p->type = &static_QUType_int;
    else if(strcmp(type, "double") == 0)
	p->type = &static_QUType_double;
    else if(strcmp(type, "char*") == 0 || strcmp(type, "const char*") == 0)
	p->type = &static_QUType_charstar;
    else if(strcmp(type, "QString") == 0 || strcmp(type, "QString&") == 0 ||
	    strcmp(type, "const QString") == 0 || strcmp(type, "const QString&") == 0)
	p->type = &static_QUType_QString;
    else
	p->type = &static_QUType_ptr;
    // Lacking support for several types. Evil.
    p->inOut = inout;
    p->typeExtra = 0;
    return Data_Wrap_Struct(rb_cObject, 0, 0, p);
}

static VALUE
make_QMetaData(VALUE /*self*/, VALUE name_value, VALUE method)
{
    char *name = StringValuePtr(name_value);
    QMetaData *m = new QMetaData;		// will be deleted
    m->name = new char[strlen(name) + 1];
    strcpy((char*)m->name, name);
    Data_Get_Struct(method, QUMethod, m->method);
    m->access = QMetaData::Public;
    return Data_Wrap_Struct(rb_cObject, 0, 0, m);
}

static VALUE
make_QUMethod(VALUE /*self*/, VALUE name_value, VALUE params)
{
    char *name = StringValuePtr(name_value);
    QUMethod *m = new QUMethod;			// permanent memory allocation
    m->name = new char[strlen(name) + 1];	// this too
    strcpy((char*)m->name, name);
    m->parameters = 0;
    m->count = RARRAY(params)->len;

    if (m->count > 0) {
	m->parameters = new QUParameter[m->count];
	for (long i = 0; i < m->count; i++) {
	    VALUE param = rb_ary_entry(params, i);
	    QUParameter *p = 0;
	    Data_Get_Struct(param, QUParameter, p);
	    ((QUParameter *) m->parameters)[i] = *p;
	    delete p;
	}
    }
    return Data_Wrap_Struct(rb_cObject, 0, 0, m);
}

static VALUE
make_QMetaData_tbl(VALUE /*self*/, VALUE list)
{
    long count = RARRAY(list)->len;
    QMetaData *m = new QMetaData[count];

    for (long i = 0; i < count; i++) {
	VALUE item = rb_ary_entry(list, i);

	QMetaData *old = 0;
	Data_Get_Struct(item, QMetaData, old);
	m[i] = *old;
	delete old;
    }

    return Data_Wrap_Struct(rb_cObject, 0, 0, m);
}

static VALUE
make_metaObject(VALUE /*self*/, VALUE className_value, VALUE parent, VALUE slot_tbl_value, VALUE slot_count_value, VALUE signal_tbl_value, VALUE signal_count_value)
{
    char *className = strdup(StringValuePtr(className_value));

    QMetaData * slot_tbl = 0;
    int slot_count = 0;
    if (slot_tbl_value != Qnil) {
    	Data_Get_Struct(slot_tbl_value, QMetaData, slot_tbl);
    	slot_count = NUM2INT(slot_count_value);
    }

    QMetaData * signal_tbl = 0;
    int signal_count = 0;
    if (signal_tbl_value != Qnil) {
    	Data_Get_Struct(signal_tbl_value, QMetaData, signal_tbl);
    	signal_count = NUM2INT(signal_count_value);
    }

    smokeruby_object *po = value_obj_info(parent);
    if(!po || !po->ptr) {
    	rb_raise(rb_eRuntimeError, "Cannot create metaObject\n");
    }
    
	QMetaObject *meta = QMetaObject::new_metaobject(
	className, (QMetaObject*)po->ptr,
	(const QMetaData*)slot_tbl, slot_count,	// slots
	(const QMetaData*)signal_tbl, signal_count,	// signals
	0, 0,	// properties
	0, 0,	// enums
	0, 0);

    smokeruby_object * o = (smokeruby_object *) malloc(sizeof(smokeruby_object));
    o->smoke = qt_Smoke;
    o->classId = qt_Smoke->idClass("QMetaObject");
    o->ptr = meta;
    o->allocated = true;

    return Data_Wrap_Struct(qt_qmetaobject_class, smokeruby_mark, smokeruby_free, o);
}

static VALUE
setAllocated(VALUE self, VALUE obj, VALUE b_value)
{
    bool b = b_value != Qfalse && b_value != Qnil;
    smokeruby_object *o = value_obj_info(obj);
    if(o) {
	o->allocated = b;
    }
    return self;
}

static VALUE
dispose(VALUE self)
{
    smokeruby_object *o = value_obj_info(self);
    if(!o || !o->ptr) { return Qnil; }
	
    const char *className = o->smoke->classes[o->classId].className;
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
    if(!o || !o->ptr) { return Qtrue; }
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

static VALUE
isValidAllocatedPointer(VALUE /*self*/, VALUE obj)
{
    smokeruby_object *o = value_obj_info(obj);
    if(o && o->ptr && o->allocated) {
	return Qtrue;
    } else {
	return Qfalse;
    }
}

static VALUE
findAllocatedObjectFor(VALUE /*self*/, VALUE obj)
{
    smokeruby_object *o = value_obj_info(obj);
    VALUE ret;
    if(o && o->ptr && (ret = getPointerObject(o->ptr)))
        return ret;
    return Qnil;
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
#ifdef DEBUG
    if (do_debug & qtdb_calls) logger("DAMNIT on %s::%s => %d", c, name, meth);
#endif
    if(!meth) {
    	meth = qt_Smoke->findMethod("QGlobalSpace", name);
#ifdef DEBUG
    if (do_debug & qtdb_calls) logger("DAMNIT on QGlobalSpace::%s => %d", name, meth);
#endif
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
#ifdef DEBUG
				if (do_debug & qtdb_calls) logger("Ambiguous Method %s::%s => %d", c, name, qt_Smoke->ambiguousMethodList[i]);
#endif

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
	if (do_debug & qtdb_calls) logger("findAllMethods called with classid = %d, pat == %s", c, pat);
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
kde_package_to_class(const char * package)
{
	VALUE klass = Qnil;
	QString packageName(package);
	
	if (packageName.startsWith("KDE::ConfigSkeleton::ItemEnum::")) {
		klass = rb_define_class_under(kconfigskeleton_itemenum_class, package+strlen("KDE::ConfigSkeleton::EnumItem::"), qt_base_class);
		rb_define_singleton_method(klass, "new", (VALUE (*) (...)) _new_kde, -1);
		kconfigskeleton_itemenum_choice_class = klass;
	} else if (packageName.startsWith("KDE::ConfigSkeleton::")) {
		klass = rb_define_class_under(kconfigskeleton_class, package+strlen("KDE::ConfigSkeleton::"), qt_base_class);
		rb_define_singleton_method(klass, "new", (VALUE (*) (...)) _new_kde, -1);
		rb_define_method(klass, "immutable?", (VALUE (*) (...)) _kconfigskeletonitem_immutable, 0);
		rb_define_method(klass, "isImmutable", (VALUE (*) (...)) _kconfigskeletonitem_immutable, 0);
	} else if (packageName.startsWith("KDE::Win::")) {
		klass = rb_define_class_under(kwin_class, package+strlen("KDE::Win::"), qt_base_class);
		rb_define_singleton_method(klass, "new", (VALUE (*) (...)) _new_kde, -1);
	} else if (packageName.startsWith("KDE::")) {
		klass = rb_define_class_under(kde_module, package+strlen("KDE::"), qt_base_class);
		rb_define_singleton_method(klass, "new", (VALUE (*) (...)) _new_kde, -1);
	} else if (packageName.startsWith("KParts::")) {
		klass = rb_define_class_under(kparts_module, package+strlen("KParts::"), qt_base_class);
		rb_define_singleton_method(klass, "new", (VALUE (*) (...)) _new_kde, -1);
	} else if (packageName.startsWith("KNS::")) {
		klass = rb_define_class_under(kns_module, package+strlen("KNS::"), qt_base_class);
		rb_define_singleton_method(klass, "new", (VALUE (*) (...)) _new_kde, -1);
	} else if (packageName.startsWith("KIO::")) {
		klass = rb_define_class_under(kio_module, package+strlen("KIO::"), qt_base_class);
		rb_define_singleton_method(klass, "new", (VALUE (*) (...)) _new_kde, -1);
		if (packageName == "KIO::UDSAtom") {
			kio_udsatom_class = klass;
		}
	} else if (packageName.startsWith("DOM::")) {
		klass = rb_define_class_under(dom_module, package+strlen("DOM::"), qt_base_class);
		rb_define_singleton_method(klass, "new", (VALUE (*) (...)) _new_kde, -1);
	} else if (packageName.startsWith("Kontact::")) {
		klass = rb_define_class_under(kontact_module, package+strlen("Kontact::"), qt_base_class);
		rb_define_singleton_method(klass, "new", (VALUE (*) (...)) _new_kde, -1);
	} else if (packageName.startsWith("Kate::")) {
		klass = rb_define_class_under(kate_module, package+strlen("Kate::"), qt_base_class);
		rb_define_singleton_method(klass, "new", (VALUE (*) (...)) _new_kde, -1);
	} else if (packageName.startsWith("KTextEditor::")) {
		klass = rb_define_class_under(ktexteditor_module, package+strlen("KTextEditor::"), qt_base_class);
		rb_define_singleton_method(klass, "new", (VALUE (*) (...)) _new_kde, -1);
	}
	
	return klass;
}

static VALUE
create_qobject_class(VALUE /*self*/, VALUE package_value)
{
    const char *package = StringValuePtr(package_value);
	VALUE klass;
	
	if (QString(package).startsWith("Qt::")) {
    	klass = rb_define_class_under(qt_module, package+strlen("Qt::"), qt_base_class);
		if (strcmp(package, "Qt::Application") == 0) {
		rb_define_singleton_method(klass, "new", (VALUE (*) (...)) new_qapplication, -1);
		rb_define_method(klass, "ARGV", (VALUE (*) (...)) qapplication_argv, 0);
		} else {
		rb_define_singleton_method(klass, "new", (VALUE (*) (...)) new_qobject, -1);
		}
	} else {
		klass = kde_package_to_class(package);
	}

	rb_define_method(klass, "inspect", (VALUE (*) (...)) inspect_qobject, 0);
	rb_define_method(klass, "pretty_print", (VALUE (*) (...)) pretty_print_qobject, 1);
	rb_define_method(klass, "receivers", (VALUE (*) (...)) receivers_qobject, 0);
	rb_define_method(klass, "className", (VALUE (*) (...)) class_name, 0);
    
	return klass;
}

static VALUE
create_qt_class(VALUE /*self*/, VALUE package_value)
{
    const char *package = StringValuePtr(package_value);
	VALUE klass;
	
	if (QString(package).startsWith("Qt::")) {
    	klass = rb_define_class_under(qt_module, package+strlen("Qt::"), qt_base_class);
	} else {
		klass = kde_package_to_class(package);
	}

    if (strcmp(package, "Qt::MetaObject") == 0) {
	qt_qmetaobject_class = klass;
    }

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
}

void
set_kconfigskeletonitem_immutable(VALUE (*kconfigskeletonitem_immutable) (VALUE))
{
	_kconfigskeletonitem_immutable = kconfigskeletonitem_immutable;
}

static VALUE
set_application_terminated(VALUE /*self*/, VALUE yn)
{
    application_terminated = (yn == Qtrue ? true : false);
	return Qnil;
}

void
Init_qtruby()
{
    init_qt_Smoke();
    qt_Smoke->binding = new QtRubySmokeBinding(qt_Smoke);
    install_handlers(Qt_handlers);

    methcache.setAutoDelete(1);
    classcache.setAutoDelete(1);

    qt_module = rb_define_module("Qt");
    qt_internal_module = rb_define_module_under(qt_module, "Internal");
    qt_base_class = rb_define_class_under(qt_module, "Base", rb_cObject);

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
    
	rb_define_method(rb_cObject, "qDebug", (VALUE (*) (...)) qdebug, 1);
	rb_define_method(rb_cObject, "qFatal", (VALUE (*) (...)) qfatal, 1);
	rb_define_method(rb_cObject, "qWarning", (VALUE (*) (...)) qwarning, 1);

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

	kconfigskeleton_class = rb_define_class_under(kde_module, "ConfigSkeleton", qt_base_class);
	kconfigskeleton_itemenum_class = rb_define_class_under(kconfigskeleton_class, "ItemEnum", qt_base_class);

	kwin_class = rb_define_class_under(kde_module, "Win", qt_base_class);

	kate_module = rb_define_module("Kate");
    rb_define_singleton_method(kate_module, "method_missing", (VALUE (*) (...)) kde_module_method_missing, -1);
    rb_define_singleton_method(kate_module, "const_missing", (VALUE (*) (...)) kde_module_method_missing, -1);

    rb_define_method(qt_internal_module, "getMethStat", (VALUE (*) (...)) getMethStat, 0);
    rb_define_method(qt_internal_module, "getClassStat", (VALUE (*) (...)) getClassStat, 0);
    rb_define_method(qt_internal_module, "getIsa", (VALUE (*) (...)) getIsa, 1);
    rb_define_method(qt_internal_module, "allocateMocArguments", (VALUE (*) (...)) allocateMocArguments, 1);
    rb_define_method(qt_internal_module, "setMocType", (VALUE (*) (...)) setMocType, 4);
    rb_define_method(qt_internal_module, "setDebug", (VALUE (*) (...)) setDebug, 1);
    rb_define_method(qt_internal_module, "debug", (VALUE (*) (...)) debugging, 0);
    rb_define_method(qt_internal_module, "getTypeNameOfArg", (VALUE (*) (...)) getTypeNameOfArg, 2);
    rb_define_method(qt_internal_module, "classIsa", (VALUE (*) (...)) classIsa, 2);
    rb_define_method(qt_internal_module, "isEnum", (VALUE (*) (...)) isEnum, 1);
    rb_define_method(qt_internal_module, "insert_pclassid", (VALUE (*) (...)) insert_pclassid, 2);
    rb_define_method(qt_internal_module, "find_pclassid", (VALUE (*) (...)) find_pclassid, 1);
    rb_define_method(qt_internal_module, "insert_mcid", (VALUE (*) (...)) insert_mcid, 2);
    rb_define_method(qt_internal_module, "find_mcid", (VALUE (*) (...)) find_mcid, 1);
    rb_define_method(qt_internal_module, "getVALUEtype", (VALUE (*) (...)) getVALUEtype, 1);
    rb_define_method(qt_internal_module, "make_QUParameter", (VALUE (*) (...)) make_QUParameter, 4);
    rb_define_method(qt_internal_module, "make_QMetaData", (VALUE (*) (...)) make_QMetaData, 2);
    rb_define_method(qt_internal_module, "make_QUMethod", (VALUE (*) (...)) make_QUMethod, 2);
    rb_define_method(qt_internal_module, "make_QMetaData_tbl", (VALUE (*) (...)) make_QMetaData_tbl, 1);
    rb_define_method(qt_internal_module, "make_metaObject", (VALUE (*) (...)) make_metaObject, 6);
    rb_define_method(qt_internal_module, "setAllocated", (VALUE (*) (...)) setAllocated, 2);
    rb_define_method(qt_internal_module, "mapObject", (VALUE (*) (...)) mapObject, 1);
    // isQOjbect => isaQObject
    rb_define_method(qt_internal_module, "isQObject", (VALUE (*) (...)) isaQObject, 1);
    rb_define_method(qt_internal_module, "isValidAllocatedPointer", (VALUE (*) (...)) isValidAllocatedPointer, 1);
    rb_define_method(qt_internal_module, "findAllocatedObjectFor", (VALUE (*) (...)) findAllocatedObjectFor, 1);
    rb_define_method(qt_internal_module, "idClass", (VALUE (*) (...)) idClass, 1);
    rb_define_method(qt_internal_module, "idMethodName", (VALUE (*) (...)) idMethodName, 1);
    rb_define_method(qt_internal_module, "idMethod", (VALUE (*) (...)) idMethod, 2);
    rb_define_method(qt_internal_module, "findMethod", (VALUE (*) (...)) findMethod, 2);
    rb_define_method(qt_internal_module, "findAllMethods", (VALUE (*) (...)) findAllMethods, -1);
    rb_define_method(qt_internal_module, "dumpCandidates", (VALUE (*) (...)) dumpCandidates, 1);
    rb_define_method(qt_internal_module, "isObject", (VALUE (*) (...)) isObject, 1);
    rb_define_method(qt_internal_module, "setCurrentMethod", (VALUE (*) (...)) setCurrentMethod, 1);
    rb_define_method(qt_internal_module, "getClassList", (VALUE (*) (...)) getClassList, 0);
    rb_define_method(qt_internal_module, "create_qt_class", (VALUE (*) (...)) create_qt_class, 1);
    rb_define_method(qt_internal_module, "create_qobject_class", (VALUE (*) (...)) create_qobject_class, 1);
    rb_define_method(qt_internal_module, "version", (VALUE (*) (...)) version, 0);
    rb_define_method(qt_internal_module, "qtruby_version", (VALUE (*) (...)) qtruby_version, 0);
    rb_define_method(qt_internal_module, "cast_object_to", (VALUE (*) (...)) cast_object_to, 2);
    rb_define_method(qt_internal_module, "application_terminated=", (VALUE (*) (...)) set_application_terminated, 1);

	rb_include_module(qt_module, qt_internal_module);
	rb_require("Qt/qtruby.rb");

    // Do package initialization
    rb_funcall(qt_internal_module, rb_intern("init_all_classes"), 0);
}

};
