/***************************************************************************
    marshall_types.h - Derived from the PerlQt sources, see AUTHORS 
                       for details
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

#ifndef MARSHALL_TYPES_H
#define MARSHALL_TYPES_H

#include <QtCore/qstring.h>
#include <QtCore/qobject.h>
#include <QtCore/qmetaobject.h>

#include "marshall.h"
#include "qtruby.h"
#include "smokeruby.h"
#include "smoke.h"

Marshall::HandlerFn getMarshallFn(const SmokeType &type);

extern void smokeStackToQtStack(Smoke::Stack stack, void ** o, int items, MocArgument* args);
extern void smokeStackFromQtStack(Smoke::Stack stack, void ** _o, int items, MocArgument* args);

class MethodReturnValueBase : public Marshall 
{
public:
	MethodReturnValueBase(Smoke *smoke, Smoke::Index meth, Smoke::Stack stack);
	const Smoke::Method &method();
	Smoke::StackItem &item();
	Smoke *smoke();
	SmokeType type();
	void next();
	bool cleanup();
	void unsupported();
    VALUE * var();
protected:
	Smoke *_smoke;
	Smoke::Index _method;
	Smoke::Stack _stack;
	SmokeType _st;
	VALUE *_retval;
	virtual const char *classname();
};


class VirtualMethodReturnValue : public MethodReturnValueBase {
public:
	VirtualMethodReturnValue(Smoke *smoke, Smoke::Index meth, Smoke::Stack stack, VALUE retval);
	Marshall::Action action();

private:
	VALUE _retval2;
};


class MethodReturnValue : public MethodReturnValueBase {
public:
	MethodReturnValue(Smoke *smoke, Smoke::Index meth, Smoke::Stack stack, VALUE * retval);
    Marshall::Action action();

private:
	const char *classname();
};

class MethodCallBase : public Marshall
{
public:
	MethodCallBase(Smoke *smoke, Smoke::Index meth);
	MethodCallBase(Smoke *smoke, Smoke::Index meth, Smoke::Stack stack);
	Smoke *smoke();
	SmokeType type();
	Smoke::StackItem &item();
	const Smoke::Method &method();
	virtual int items() = 0;
	virtual void callMethod() = 0;	
	void next();
	void unsupported();

protected:
	Smoke *_smoke;
	Smoke::Index _method;
	Smoke::Stack _stack;
	int _cur;
	Smoke::Index *_args;
	bool _called;
	VALUE *_sp;
	virtual const char* classname();
};


class VirtualMethodCall : public MethodCallBase {
public:
	VirtualMethodCall(Smoke *smoke, Smoke::Index meth, Smoke::Stack stack, VALUE obj, VALUE *sp);
	~VirtualMethodCall();
	Marshall::Action action();
	VALUE * var();
	int items();
	void callMethod();
	bool cleanup();
 
private:
	VALUE _obj;
};


class MethodCall : public MethodCallBase {
public:
	MethodCall(Smoke *smoke, Smoke::Index method, VALUE target, VALUE *sp, int items);
	~MethodCall();
	Marshall::Action action();
	VALUE * var();

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

	int items();
	bool cleanup();
private:
	VALUE _target;
	void *_current_object;
	Smoke::Index _current_object_class;
	VALUE *_sp;
	int _items;
	VALUE _retval;
	const char *classname();
};


class SigSlotBase : public Marshall {
public:
	SigSlotBase(VALUE args);
	~SigSlotBase();
	const MocArgument &arg();
	SmokeType type();
	Smoke::StackItem &item();
	VALUE * var();
	Smoke *smoke();
	virtual const char *mytype() = 0;
	virtual void mainfunction() = 0;
	void unsupported();
	void next(); 

protected:
	MocArgument *_args;
	int _cur;
	bool _called;
	Smoke::Stack _stack;
	int _items;
	VALUE *_sp;
};


class EmitSignal : public SigSlotBase {
    QObject *_obj;
    int _id;
	VALUE * _result;
 public:
    EmitSignal(QObject *obj, int id, int items, VALUE args, VALUE * sp, VALUE * result);
    Marshall::Action action();
    Smoke::StackItem &item();
	const char *mytype();
	void emitSignal();
	void mainfunction();
	bool cleanup();
};


class InvokeSlot : public SigSlotBase {
    VALUE _obj;
    ID _slotname;
    void **_o;
public:
    InvokeSlot(VALUE obj, ID slotname, VALUE args, void ** o);
	~InvokeSlot();
    Marshall::Action action();
	const char *mytype();
    bool cleanup();
	void copyArguments();
	void invokeSlot(); 
	void mainfunction();
};

#endif
