/***************************************************************************
                          Qt.cpp  -  description
                             -------------------
    begin                : Fri Jul 4 2003
    copyright            : (C) 2003 by Richard Dale
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

#include <stdio.h>
#include <stdarg.h>

#include <qglobal.h>
#include <qregexp.h>
#include <qstring.h>
#include <qptrdict.h>
#include <qapplication.h>
#include <qmetaobject.h>
#include <private/qucomextra_p.h>

#undef DEBUG
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
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

#define QTRUBY_VERSION "0.8.5"

extern Smoke *qt_Smoke;
extern void init_qt_Smoke();
extern void smokeruby_mark(void * ptr);
extern void smokeruby_free(void * ptr);

// int do_debug = qtdb_none;
int do_debug = qtdb_gc | qtdb_virtual;

QPtrDict<VALUE> pointer_map(2179);
int object_count = 0;

bool temporary_virtual_function_success = false;

QAsciiDict<Smoke::Index> methcache(2179);
QAsciiDict<Smoke::Index> classcache(2179);

VALUE ruby_self = Qundef;
VALUE qt_module = Qundef;
VALUE qt_internal_module = Qundef;
VALUE qt_base_class = Qundef;
VALUE qt_qmetaobject_class = Qundef;

Smoke::Index _current_method = 0;
/*
 * Type handling by moc is simple.
 *
 * If the type name matches /^(?:const\s+)?\Q$types\E&?$/, use the
 * static_QUType, where $types is join('|', qw(bool int double char* QString);
 *
 * Everything else is passed as a pointer! There are types which aren't
 * Smoke::tf_ptr but will have to be passed as a pointer. Make sure to keep
 * track of what's what.
 */

/*
 * Simply using typeids isn't enough for signals/slots. It will be possible
 * to declare signals and slots which use arguments which can't all be
 * found in a single smoke object. Instead, we need to store smoke => typeid
 * pairs. We also need additional informatation, such as whether we're passing
 * a pointer to the union element.
 */

enum MocArgumentType {
    xmoc_ptr,
    xmoc_bool,
    xmoc_int,
    xmoc_double,
    xmoc_charstar,
    xmoc_QString
};

struct MocArgument {
    // smoke object and associated typeid
    SmokeType st;
    MocArgumentType argType;
};


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
    if(!strcmp(smoke->classes[classId].className, "QObject"))
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

bool isDerivedFrom(Smoke *smoke, const char *className, const char *baseClassName) {
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

extern VALUE rb_hash_delete(VALUE hash, VALUE value);

void unmapPointer(smokeruby_object *o, Smoke::Index classId, void *lastptr) {
    void *ptr = o->smoke->cast(o->ptr, o->classId, classId);
    if(ptr != lastptr) {
	lastptr = ptr;
	if (pointer_map[ptr] != 0) {
		VALUE * obj_ptr = pointer_map[ptr];
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
    if(ptr != lastptr) {
	lastptr = ptr;
	VALUE * obj_ptr = (VALUE *) malloc(sizeof(VALUE));
	memcpy(obj_ptr, &obj, sizeof(VALUE));
	if (do_debug & qtdb_gc) {
		printf("mapPointer %p -> %p\n", ptr, obj_ptr);
	}
	// TODO: fix this should be a weak ref, but make a strong ref to the object for now
	rb_gc_register_address(&obj);
	
	pointer_map.insert(ptr, obj_ptr);
    }
    for(Smoke::Index *i = o->smoke->inheritanceList + o->smoke->classes[classId].parents;
	*i;
	i++) {
	mapPointer(obj, o, *i, lastptr);
    }
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
    VALUE _savethis;

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
	_savethis = ruby_self;
	ruby_self = obj;
	_sp = ALLOC_N(VALUE, method().numArgs);
	_args = _smoke->argumentList + method().args;
    }

    ~VirtualMethodCall() {
	ruby_self = _savethis;
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
	_retval = Qundef;
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

#ifdef DEBUG
        VALUE klass = rb_funcall(_obj, rb_intern("class"), 0);
        VALUE name = rb_funcall(klass, rb_intern("name"), 0);
	printf(	"In InvokeSlot::invokeSlot 3 items: %d classname: %s\n",
		_items,
		STR2CSTR(name) );
#endif
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
	_sp = ALLOC_N(VALUE, _items);
	_stack = new Smoke::StackItem[_items];
	copyArguments();
    }

    ~InvokeSlot() {
	delete[] _stack;
    }
};

class QtRubySmokeBinding : public SmokeBinding {
public:
    QtRubySmokeBinding(Smoke *s) : SmokeBinding(s) {}

    void deleted(Smoke::Index classId, void *ptr) {
	VALUE obj = getPointerObject(ptr);
	smokeruby_object *o = value_obj_info(obj);
	if(do_debug & qtdb_gc) {
	    printf("%p->~%s()\n", ptr, smoke->className(classId));
	    if(o && o->ptr)
		object_count --;
	    //printf("Remaining objects: %d\n", object_count);
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
	    fprintf(stderr, "virtual %p->%s::%s() called\n", ptr,
		    smoke->classes[smoke->methods[method].classId].className,
		    smoke->methodNames[smoke->methods[method].name]
		    );

//	    printf("virtual %p->%s::%s() called\n", ptr,
//		smoke->classes[smoke->methods[method].classId].className,
//		smoke->methodNames[smoke->methods[method].name]
//		);

	if(!o) {
	    if( do_debug & qtdb_virtual )   // if not in global destruction
		fprintf(stderr, "Cannot find object for virtual method %p -> %p\n", ptr, &obj);
	    return false;
	}

	const char *methodName = smoke->methodNames[smoke->methods[method].name];
	if (rb_respond_to(obj, rb_intern(methodName)) == 0) {
	    return false;
	}

	VirtualMethodCall c(smoke, method, args, obj);
	// exception variable, just temporary

	temporary_virtual_function_success = true;
	c.next();
	bool ret = temporary_virtual_function_success;
	temporary_virtual_function_success = true;
	return ret;
    }

    char *className(Smoke::Index classId) {
	const char *className = smoke->className(classId);
	char *buf = new char[strlen(className) + strlen("Qt::") + 1];
	strcpy(buf, "Qt::");
	strcat(buf, className + 1);
	return buf;
    }
};

extern "C" {

// ----------------   Helpers -------------------

//---------- XS Autoload (for all functions except fully qualified statics & enums) ---------

VALUE catArguments(VALUE * /*sp*/, int /*n*/)
{
    VALUE r=rb_str_new2("");
//    for(int i = 0; i < n; i++) {
//        if(i) sv_catpv(r, ", ");
//        if(!SvOK(sp[i])) {
//            sv_catpv(r, "undef");
//        } else if(SvROK(sp[i])) {
//            smokeruby_object *o = value_obj_info(sp[i]);
//            if(o)
//                sv_catpv(r, o->smoke->className(o->classId));
//            else
//                sv_catsv(r, sp[i]);
//        } else {
//            bool isString = SvPOK(sp[i]);
//            STRLEN len;
//            char *s = SvPV(sp[i], len);
//            if(isString) sv_catpv(r, "'");
//            sv_catpvn(r, s, len > 10 ? 10 : len);
//            if(len > 10) sv_catpv(r, "...");
//            if(isString) sv_catpv(r, "'");
//        }
//    }
    return r;
}

Smoke::Index package_classid(const char *p)
{
    Smoke::Index *item = classcache.find(p);
    if(item)
	return *item;
    char *nisa = new char[strlen(p)+6];
    strcpy(nisa, p);
    strcat(nisa, "::ISA");
//     AV* isa=get_av(nisa, true);
    delete[] nisa;
//     for(int i=0; i<=av_len(isa); i++) {
//         SV** np = av_fetch(isa, i, 0);
//         if(np) {
//             Smoke::Index ix = package_classid(SvPV_nolen(*np));
//             if(ix) {
//                 classcache.insert(p, new Smoke::Index(ix));
//                 return ix;
//             }
//         }
//     }
    return (Smoke::Index) 0;
}

VALUE
set_obj_info(const char * className, smokeruby_object * o)
{
    VALUE klass = rb_funcall(qt_internal_module,
			     rb_intern("find_class"),
			     1,
			     rb_str_new2(className) );
    VALUE obj = Data_Wrap_Struct(klass, smokeruby_mark, smokeruby_free, (void *) o);
    return obj;
}

char *get_VALUEtype(VALUE ruby_value)
{
    char *r = strdup("");
    if(ruby_value == Qundef)
	r = strdup("u");
    else if(TYPE(ruby_value) == T_FIXNUM || TYPE(ruby_value) == T_BIGNUM)
	r = strdup("i");
    else if(TYPE(ruby_value) == T_FLOAT)
	r = strdup("n");
    else if(TYPE(ruby_value) == T_STRING)
	r = strdup("s");
    else if(TYPE(ruby_value) == T_DATA) {
	smokeruby_object *o = value_obj_info(ruby_value);
	if(!o) {
	    r = strdup("a");
	} else {
	    r = strdup(o->smoke->className(o->classId));
    }
	}
    else {
	r = strdup("U");
	}

    return r;
}

void rb_str_catf(VALUE self, const char *format, ...) __attribute__ ((format (printf, 2, 3)));

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

static VALUE
metaObject(VALUE self)
{
    VALUE metaObject = rb_funcall(qt_internal_module, rb_intern("getMetaObject"), 1, self);
    return metaObject;
}

bool avoid_fetchmethod = false;

static VALUE
method_missing(int argc, VALUE * argv, VALUE self)
{
    VALUE klass = rb_funcall(self, rb_intern("class"), 0);
#ifdef DEBUG
    VALUE name = rb_funcall(klass, rb_intern("name"), 0);
    printf("In method_missing(argc: %d, argv[0]: %s TYPE: 0x%2.2x)\n",
	    argc,
	    rb_id2name( SYM2ID(argv[0]) ),
	    TYPE(self) );
#endif
	char * methodName = rb_id2name(SYM2ID(argv[0]));
    VALUE * savestack = ALLOCA_N(VALUE, argc + 3);
    savestack[0] = rb_str_new2("Qt");
    savestack[1] = rb_str_new2(methodName);
    savestack[2] = klass;
    savestack[3] = self;
    for (int count = 1; count < argc; count++) {
	savestack[count+3] = argv[count];
    }

    _current_method = -1;
    VALUE retval = rb_funcall2(qt_internal_module, rb_intern("do_method_missing"), argc+3, savestack);
    if (retval != Qnil)
       return retval;

    // If the method can't be found allow the default method_missing
    //	to display an error message, by calling super on the method
    if (_current_method == -1) {
		return rb_call_super(argc, argv);
    }

    MethodCall c(qt_Smoke, _current_method, self, savestack+4, argc-1);
    c.next();
    VALUE result = *(c.var());
    return result;
}

static VALUE
class_method_missing(int argc, VALUE * argv, VALUE klass)
{
#ifdef DEBUG
    printf("In class_method_missing(argc: %d, argv[0]: %s)\n",
	    argc,
	    rb_id2name( SYM2ID(argv[0]) ) );
#endif

    VALUE * savestack = ALLOCA_N(VALUE, argc + 3);
    savestack[0] = rb_str_new2("Qt");
    savestack[1] = rb_str_new2(rb_id2name(SYM2ID(argv[0])));
    savestack[2] = klass;
    savestack[3] = Qnil;
    for (int count = 1; count < argc; count++) {
	savestack[count+3] = argv[count];
    }

    _current_method = -1;
    VALUE retval = rb_funcall2(qt_internal_module, rb_intern("do_method_missing"), argc+3, savestack);
    if (retval != Qnil)
	return retval;

    // If the method can't be found allow the default method_missing
    //	to display an error message, by calling super on the method
    if (_current_method == -1) {
		QRegExp rx("[a-zA-Z]+");
		if (rx.search(rb_id2name(SYM2ID(argv[0]))) == -1) {
			// If an operator method hasn't been found as an instance method,
			// then look for a class method - after 'op(self,a)' try 'self.op(a)' 
	    	VALUE * method_stack = ALLOCA_N(VALUE, argc - 1);
	    	method_stack[0] = argv[0];
	    	for (int count = 1; count < argc; count++) {
			method_stack[count] = argv[count+1];
    		}
			return method_missing(argc-1, method_stack, argv[1]);
		} else {
			return rb_call_super(argc, argv);
		}
    }

    MethodCall c(qt_Smoke, _current_method, Qnil, savestack+4, argc-1);
    c.next();
    VALUE result = *(c.var());
    return result;
}

static VALUE module_method_missing(int argc, VALUE * argv, VALUE /*klass*/)
{
#ifdef DEBUG
    printf("In module_method_missing(argc: %d, argv[0]: %s)\n",
	    argc,
	    rb_id2name( SYM2ID(argv[0]) ) );
#endif

    return class_method_missing(argc, argv, qt_module);
}

static VALUE setThis(VALUE self, VALUE obj);
static VALUE mapObject(VALUE self, VALUE obj);

/*

class LCDRange < Qt::Widget

	def initialize(s, parent, name)
		super(parent, name)
		init()
		...

For a case such as the above, the QWidget can't be instantiated until
the initializer has been run up to the point where 'super(parent, name)'
is called. Only then, can the number and type of arguments passed to the
constructor can be known. However, the rest of the intializer
can't be run until 'self' is a proper T_DATA object with a wrapped C++
instance.

The solution is to run the initialize code twice. First, only up to the
'super(parent, name)' call, where the QWidget would get instantiated in
initialize_qt(). And then continue_new_instance() jumps out of the
initializer returning the wrapped object as a result.

The second time round 'self' will be the wrapped instance of type T_DATA,
so initialize() can be allowed to proceed to the end.
*/
static VALUE
initialize_qt(int argc, VALUE * argv, VALUE self)
{
    if (TYPE(self) == T_DATA) {
	return self;
    }

    VALUE klass = rb_funcall(self, rb_intern("class"), 0);
    VALUE constructor_name = rb_str_new2("new");

    VALUE * savestack = ALLOCA_N(VALUE, argc + 4);
    savestack[0] = rb_str_new2("Qt");
    savestack[1] = constructor_name;
    savestack[2] = klass;
    savestack[3] = self;
    for (int count = 0; count < argc; count++) {
	savestack[count+4] = argv[count];
    }

    _current_method = -1;
    VALUE retval = rb_funcall2(qt_internal_module, rb_intern("do_method_missing"), argc+4, savestack);
    if (retval != Qnil)
	return retval;

    // If the method can't be found, allow the default method_missing
    //	to display an error message, by calling super on the method
    if (_current_method == -1) {
	if (argc == 0) {
	    fprintf(stderr, "FATAL ERROR: unresolved constructor call\n");
            exit(0);
	} else {
            return rb_call_super(argc, argv);
        }
    }

    // Success. Cache result.
    //methcache.insert((const char *)mcid, new Smoke::Index(_current_method));

    MethodCall c(qt_Smoke, _current_method, self, savestack+4, argc);
    c.next();
    VALUE temp_obj = *(c.var());
    void * ptr = 0;
    Data_Get_Struct(temp_obj, smokeruby_object, ptr);
    VALUE result = Data_Wrap_Struct(klass, smokeruby_mark, smokeruby_free, ptr);
    mapObject(result, result);
    return rb_funcall(qt_internal_module, rb_intern("continue_new_instance"), 1, result);
}

static VALUE
new_qt(int argc, VALUE * argv, VALUE klass)
{
#ifdef DEBUG
    VALUE class_name = rb_funcall(klass, rb_intern("name"), 0);
    printf("In new_qt, argc: %d, self class_name: %s\n",
	    argc,
	    STR2CSTR(class_name) );
#endif

    VALUE * localstack = ALLOCA_N(VALUE, argc + 1);
    localstack[0] = rb_obj_alloc(klass);
    for (int count = 0; count < argc; count++) {
	localstack[count+1] = argv[count];
    }

    VALUE result = rb_funcall2(qt_internal_module, rb_intern("try_initialize"), argc+1, localstack);

    if (rb_respond_to(result, rb_intern("initialize")) != 0) {
	rb_obj_call_init(result, argc, argv);
    }

    return result;
}

static VALUE qt_invoke(VALUE self, VALUE id_value, VALUE quobject);
static VALUE qt_signal(int argc, VALUE * argv, VALUE self);

static VALUE
new_qobject(int argc, VALUE * argv, VALUE klass)
{
    if (rb_funcall(qt_internal_module, rb_intern("hasMembers"), 1, klass) == Qtrue) {
	// TODO: Don't do this everytime a new instance is created, just once..
	rb_define_method(klass, "qt_invoke", (VALUE (*) (...)) qt_invoke, 2);
	rb_define_method(klass, "qt_emit", (VALUE (*) (...)) qt_invoke, 2);
	rb_define_method(klass, "metaObject", (VALUE (*) (...)) metaObject, 0);
	VALUE signalNames = rb_funcall(qt_internal_module, rb_intern("getSignalNames"), 1, klass);
	for (long index = 0; index < RARRAY(signalNames)->len; index++) {
	    VALUE signal = rb_ary_entry(signalNames, index);
	    rb_define_method(klass, STR2CSTR(signal), (VALUE (*) (...)) qt_signal, -1);
	}
    }

    return new_qt(argc, argv, klass);
}

static VALUE
new_qapplication(int argc, VALUE * argv, VALUE klass)
{
    VALUE result = Qnil;

    if (argc == 1 && TYPE(argv[0]) == T_ARRAY) {
	// Convert '(ARGV)' to '(NUM, [$0]+ARGV)'
	VALUE * local_argv = ALLOCA_N(VALUE, argc + 1);
	rb_ary_unshift(argv[0], rb_gv_get("$0"));
	local_argv[0] = INT2NUM(RARRAY(argv[0])->len);
	local_argv[1] = argv[0];
	result = new_qobject(2, local_argv, klass);
    } else {
	result = new_qobject(argc, argv, klass);
    }

    rb_gv_set("$qApp", result);
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
    // FIXME: make slot inheritance work
    if(index < 0) return Qnil;

    if (isSignal) {
	member = rb_funcall(qt_internal_module, rb_intern("signalAt"), 2, self, INT2NUM(index));
    } else {
	member = rb_funcall(qt_internal_module, rb_intern("slotAt"), 2, self, INT2NUM(index));
    }

    VALUE mocArgs = rb_funcall(qt_internal_module, rb_intern("getMocArguments"), 1, member);
    slotname = STR2CSTR(member);

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
#ifdef DEBUG
    printf("In qt_signal argc: %d index: %d offset: %d\n", argc, index, offset);
#endif

    if(args == Qnil) return Qfalse;

    // Okay, we have the signal info. *whew*
//    if(items < argc)
//	rb_raise(rb_eArgError, "Insufficient arguments to emit signal");

    EmitSignal signal(qobj, offset + index, argc, args, argv);
    signal.next();

    return Qtrue;
}

static VALUE
qt_invoke(VALUE self, VALUE id_value, VALUE quobject)
{
    // Arguments: int id, QUObject *o
    int id = NUM2INT(id_value);
    QUObject *_o = 0;

    Data_Get_Struct(quobject, QUObject, _o);
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
	// throw an exception - evil style
	temporary_virtual_function_success = false;
	return Qfalse;
    }

    QString name(slotname);
    name.replace(QRegExp("\\(.*"), "");
    InvokeSlot slot(self, rb_intern((const char *) name), mocArgs, _o);
    slot.next();

    return Qtrue;
}


// --------------- Ruby C functions for Qt::_internal::* helpers  ----------------


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
	//	printf("\tparent: %s\n", qt_Smoke->classes[*parents].className);
	rb_ary_push(parents_list, rb_str_new2(qt_Smoke->classes[*parents++].className));
    }
    return parents_list;
}


static VALUE
dontRecurse(VALUE self)
{
    avoid_fetchmethod = true;
    return self;
}

static VALUE
allocateMocArguments(VALUE /*self*/, VALUE count_value)
{
    int count = NUM2INT(count_value);
    MocArgument * ptr = new MocArgument[count + 1];
    return Data_Wrap_Struct(rb_cObject, 0, 0, ptr);
}

static VALUE
setMocType(VALUE /*self*/, VALUE ptr, VALUE idx_value, VALUE name_value, VALUE static_type_value)
{
    int idx = NUM2INT(idx_value);
    char *name = STR2CSTR(name_value);
    char *static_type = STR2CSTR(static_type_value);
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

#ifdef DEBUG
static VALUE
setDebug(VALUE self, VALUE on_value)
{
    int on = NUM2INT(on_value);
    do_debug = on;
    return self;
}
#endif

#ifdef DEBUG
static VALUE
debugging(VALUE /*self*/)
{
    return INT2NUM(do_debug);
}
#endif

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
    char *className = STR2CSTR(className_value);
    char *base = STR2CSTR(base_value);
    return isDerivedFrom(qt_Smoke, className, base) ? Qtrue : Qfalse;
}

static VALUE
insert_pclassid(VALUE self, VALUE p_value, VALUE ix_value)
{
    char *p = STR2CSTR(p_value);
    int ix = NUM2INT(ix_value);
    classcache.insert(p, new Smoke::Index((Smoke::Index)ix));
    return self;
}

static VALUE
find_pclassid(VALUE /*self*/, VALUE p_value)
{
    char *p = STR2CSTR(p_value);
    Smoke::Index *r = classcache.find(p);
    if(r)
        return INT2NUM((int)*r);
    else
        return INT2NUM(0);
}

static VALUE
insert_mcid(VALUE self, VALUE mcid_value, VALUE ix_value)
{
    char *mcid = STR2CSTR(mcid_value);
    int ix = NUM2INT(ix_value);
    methcache.insert(mcid, new Smoke::Index((Smoke::Index)ix));
    return self;
}

    static VALUE
find_mcid(VALUE /*self*/, VALUE mcid_value)
{
    char *mcid = STR2CSTR(mcid_value);
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
    char *name = STR2CSTR(name_value);
    char *type = STR2CSTR(type_value);
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
    char *name = STR2CSTR(name_value);
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
    char *name = STR2CSTR(name_value);
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
    char *className = STR2CSTR(className_value);

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

#ifdef DEBUG
    printf(	"make_metaObject: %s slot_count: %d signal_count: %d\n",
	    className,
	    slot_count,
	    signal_count );
#endif

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

    smokeruby_object * o = (smokeruby_object *) ALLOC(smokeruby_object);
    o->smoke = qt_Smoke;
    o->classId = qt_Smoke->idClass("QMetaObject");
    o->ptr = meta;
    o->allocated = true;

    return Data_Wrap_Struct(qt_qmetaobject_class, smokeruby_mark, smokeruby_free, o);
}

static VALUE
dumpObjects(VALUE self)
{
/*
    hv_iterinit(pointer_map);
    HE *e;
    while(e = hv_iternext(pointer_map)) {
	STRLEN len;
	VALUE sv = HeVAL(e);
	printf("key = %s, refcnt = %d, weak = %d, ref? %d\n", HePV(e, len), SvREFCNT(sv), SvWEAKREF(sv), SvROK(sv)?1:0);
	if(SvRV(sv))
	    printf("REFCNT = %d\n", SvREFCNT(SvRV(sv)));
	//SvREFCNT_dec(HeVAL(e));
	//HeVAL(e) = &PL_sv_undef;
    }
 */
	return self;
}

static VALUE
setAllocated(VALUE self, VALUE obj, VALUE b_value)
{
    bool b = (bool) NUM2INT(b_value);
    smokeruby_object *o = value_obj_info(obj);
    if(o) {
	o->allocated = b;
    }
    return self;
}

static VALUE
setThis(VALUE self, VALUE obj)
{
    ruby_self = obj;
    return self;
}

static VALUE
deleteObject(VALUE self, VALUE obj)
{
    smokeruby_object *o = value_obj_info(obj);
    if(!o) { return Qnil; }
    QObject *qobj = (QObject*)o->smoke->cast(o->ptr, o->classId, o->smoke->idClass("QObject"));
    delete qobj;
    return self;
}

static VALUE
mapObject(VALUE self, VALUE obj)
{
    smokeruby_object *o = value_obj_info(obj);
    if(!o)
        return Qnil;
    SmokeClass c( o->smoke, o->classId );
    object_count ++;
    if(!c.hasVirtual() ) {
	return Qnil;
    }
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
	return INT2NUM(1);
    } else {
	return INT2NUM(0);
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
    char *name = STR2CSTR(name_value);
    return INT2NUM(qt_Smoke->idClass(name));
}

static VALUE
idMethodName(VALUE /*self*/, VALUE name_value)
{
    char *name = STR2CSTR(name_value);
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
    char *c = STR2CSTR(c_value);
    char *name = STR2CSTR(name_value);
    VALUE result = rb_ary_new();
    Smoke::Index meth = qt_Smoke->findMethod(c, name);
#ifdef DEBUG
    printf("DAMNIT on %s::%s => %d\n", c, name, meth);
#endif
    if(!meth) {
    	meth = qt_Smoke->findMethod("QGlobalSpace", name);
#ifdef DEBUG
    printf("DAMNIT on QGlobalSpace::%s => %d\n", name, meth);
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
		rb_ary_push(result, INT2NUM(qt_Smoke->methodMaps[meth].method));
	} else {		// multiple match
	    i = -i;		// turn into ambiguousMethodList index
	    while(qt_Smoke->ambiguousMethodList[i]) {
		rb_ary_push(result, INT2NUM(qt_Smoke->ambiguousMethodList[i]));
#ifdef DEBUG
		printf("Ambiguous Method %s::%s => %d\n", c, name, qt_Smoke->ambiguousMethodList[i]);
#endif
		i++;
	    }
	}
    }
    return result;
}

static VALUE
findMethodFromIds(VALUE /*self*/, VALUE idclass_value, VALUE idmethodname_value)
{
    int idclass = NUM2INT(idclass_value);
    int idmethodname = NUM2INT(idmethodname_value);
    VALUE result = Qundef;
    Smoke::Index meth = qt_Smoke->findMethod(idclass, idmethodname);
    if(!meth) {
	// empty list
    } else if(meth > 0) {
	Smoke::Index i = qt_Smoke->methodMaps[meth].method;
	if(i >= 0) {	// single match
		result = INT2NUM((int) i);
//	    PUSHs(sv_2mortal(newSViv((IV)i)));
	} else {		// multiple match
	    i = -i;		// turn into ambiguousMethodList index
	    while(qt_Smoke->ambiguousMethodList[i]) {
		result = INT2NUM((int) qt_Smoke->ambiguousMethodList[i]);
//		PUSHs(sv_2mortal(newSViv(
//		    (IV)qt_Smoke->ambiguousMethodList[i]
//		)));
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
    if(classid != Qundef) {
        Smoke::Index c = (Smoke::Index) NUM2INT(classid);
        char * pat = 0L;
        if(argc > 1 && TYPE(argv[1]) == T_STRING)
            pat = STR2CSTR(argv[1]);
#ifdef DEBUG
	printf("findAllMethods called with classid = %d, pat == %s\n", c, pat);
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
			rb_ary_push(meths, INT2NUM((int)ix));
                    } else {		// multiple match
                        ix = -ix;		// turn into ambiguousMethodList index
                        while(qt_Smoke->ambiguousMethodList[ix]) {
                          rb_ary_push(meths, INT2NUM((int)qt_Smoke->ambiguousMethodList[ix]));
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

#if 0
static VALUE
rb_catArguments(VALUE self, VALUE r_args)
{
/*
    if(SvROK(r_args) && SvTYPE(SvRV(r_args)) == SVt_PVAV) {
        AV* args=(AV*)SvRV(r_args);
        for(int i = 0; i <= av_len(args); i++) {
            VALUE *arg=av_fetch(args, i, 0);
	    if(i) sv_catpv(return self;, ", ");
	    if(!arg || !SvOK(*arg)) {
		sv_catpv(return self;, "undef");
	    } else if(SvROK(*arg)) {
		smokeruby_object *o = value_obj_info(*arg);
		if(o)
		    sv_catpv(return self;, o->smoke->className(o->classId));
		else
		    sv_catsv(return self;, *arg);
	    } else {
		bool isString = SvPOK(*arg);
		STRLEN len;
		char *s = SvPV(*arg, len);
		if(isString) sv_catpv(return self;, "'");
		sv_catpvn(return self;, s, len > 10 ? 10 : len);
		if(len > 10) sv_catpv(return self;, "...");
		if(isString) sv_catpv(return self;, "'");
	    }
	}
    }
  */
    return self;
}
#endif

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
create_qobject_class(VALUE /*self*/, VALUE package_value)
{
    char *package = STR2CSTR(package_value);
    VALUE klass = rb_define_class_under(qt_module, package+strlen("Qt::"), qt_base_class);

    if (strcmp(package, "Qt::Application") == 0) {
	rb_define_singleton_method(klass, "new", (VALUE (*) (...)) new_qapplication, -1);
    } else {
	rb_define_singleton_method(klass, "new", (VALUE (*) (...)) new_qobject, -1);
    }

    return klass;
}

static VALUE
create_qt_class(VALUE /*self*/, VALUE package_value)
{
    char *package = STR2CSTR(package_value);
    VALUE klass = rb_define_class_under(qt_module, package+strlen("Qt::"), qt_base_class);

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
Init_Qt()
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

    rb_define_method(qt_internal_module, "getMethStat", (VALUE (*) (...)) getMethStat, 0);
    rb_define_method(qt_internal_module, "getClassStat", (VALUE (*) (...)) getClassStat, 0);
    rb_define_method(qt_internal_module, "getIsa", (VALUE (*) (...)) getIsa, 1);
    rb_define_method(qt_internal_module, "dontRecurse", (VALUE (*) (...)) dontRecurse, 0);
    rb_define_method(qt_internal_module, "allocateMocArguments", (VALUE (*) (...)) allocateMocArguments, 1);
    rb_define_method(qt_internal_module, "setMocType", (VALUE (*) (...)) setMocType, 4);
#ifdef DEBUG
    rb_define_method(qt_internal_module, "setDebug", (VALUE (*) (...)) setDebug, 1);
    rb_define_method(qt_internal_module, "debug", (VALUE (*) (...)) debugging, 0);
#endif
    rb_define_method(qt_internal_module, "getTypeNameOfArg", (VALUE (*) (...)) getTypeNameOfArg, 2);
    rb_define_method(qt_internal_module, "classIsa", (VALUE (*) (...)) classIsa, 2);
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
    rb_define_method(qt_internal_module, "dumpObjects", (VALUE (*) (...)) dumpObjects, 0);
    rb_define_method(qt_internal_module, "setAllocated", (VALUE (*) (...)) setAllocated, 2);
    rb_define_method(qt_internal_module, "setThis", (VALUE (*) (...)) setThis, 1);
    rb_define_method(qt_internal_module, "deleteObject", (VALUE (*) (...)) deleteObject, 1);
    rb_define_method(qt_internal_module, "mapObject", (VALUE (*) (...)) mapObject, 1);
    // isQOjbect => isaQObject
    rb_define_method(qt_internal_module, "isQObject", (VALUE (*) (...)) isaQObject, 1);
    rb_define_method(qt_internal_module, "isValidAllocatedPointer", (VALUE (*) (...)) isValidAllocatedPointer, 1);
    rb_define_method(qt_internal_module, "findAllocatedObjectFor", (VALUE (*) (...)) findAllocatedObjectFor, 1);
    rb_define_method(qt_internal_module, "idClass", (VALUE (*) (...)) idClass, 1);
    rb_define_method(qt_internal_module, "idMethodName", (VALUE (*) (...)) idMethodName, 1);
    rb_define_method(qt_internal_module, "idMethod", (VALUE (*) (...)) idMethod, 2);
    rb_define_method(qt_internal_module, "findMethod", (VALUE (*) (...)) findMethod, 2);
    rb_define_method(qt_internal_module, "findMethodFromIds", (VALUE (*) (...)) findMethodFromIds, 2);
    rb_define_method(qt_internal_module, "findAllMethods", (VALUE (*) (...)) findAllMethods, -1);
    rb_define_method(qt_internal_module, "dumpCandidates", (VALUE (*) (...)) dumpCandidates, 1);
    rb_define_method(qt_internal_module, "catArguments", (VALUE (*) (...)) catArguments, 1);
    rb_define_method(qt_internal_module, "isObject", (VALUE (*) (...)) isObject, 1);
    rb_define_method(qt_internal_module, "setCurrentMethod", (VALUE (*) (...)) setCurrentMethod, 1);
    rb_define_method(qt_internal_module, "getClassList", (VALUE (*) (...)) getClassList, 0);
    rb_define_method(qt_internal_module, "create_qt_class", (VALUE (*) (...)) create_qt_class, 1);
    rb_define_method(qt_internal_module, "create_qobject_class", (VALUE (*) (...)) create_qobject_class, 1);
    rb_define_method(qt_internal_module, "version", (VALUE (*) (...)) version, 0);
    rb_define_method(qt_internal_module, "qtruby_version", (VALUE (*) (...)) qtruby_version, 0);

	rb_include_module(qt_module, qt_internal_module);
	rb_require("Qt/Qt.rb");

    // Do package initialization
    rb_funcall(qt_internal_module, rb_intern("init"), 0);
}

};
