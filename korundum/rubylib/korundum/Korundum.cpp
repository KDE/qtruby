/***************************************************************************
                          Korundum.cpp  -  Runtime for KDE services, DCOP etc
                             -------------------
    begin                : Sun Sep 28 2003
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

#include <qobject.h>
#include <qstringlist.h>

#include <dcopclient.h>
#include <kapplication.h>

#include <ruby.h>

#include <marshall.h>
#include <qtruby.h>
#include <smokeruby.h>
#include <smoke.h>

extern TypeHandler KDE_handlers[];
extern void install_handlers(TypeHandler *);

Marshall::HandlerFn getMarshallFn(const SmokeType &type);

class EmitDCOPSignal : public Marshall {
	VALUE _obj;
	char * _signalName;
	QByteArray *_data;
	QDataStream *_stream;
    int _id;
    MocArgument *_args;
    VALUE *_sp;
    int _items;
    int _cur;
    Smoke::Stack _stack;
    bool _called;
public:
    EmitDCOPSignal(VALUE obj, char * signalName, int items, VALUE *sp, VALUE args) :
		_obj(obj), _signalName(signalName), _items(items), _sp(sp), _cur(-1), _called(false)
    {
		_data = new QByteArray();
		_stream = new QDataStream(*_data, IO_WriteOnly);
		Data_Get_Struct(rb_ary_entry(args, 1), MocArgument, _args);
		_stack = new Smoke::StackItem[_items];
    }
	
	~EmitDCOPSignal() 
	{
		delete[] _stack;
	}
    const MocArgument &arg() { return _args[_cur]; }
    SmokeType type() { return arg().st; }
    Marshall::Action action() { return Marshall::FromVALUE; }
    Smoke::StackItem &item() { return _stack[_cur]; }
    VALUE * var() { return _sp + _cur; }
	
    void unsupported() 
	{
		rb_raise(rb_eArgError, "Cannot handle '%s' as a DCOP signal argument", type().name());
    }
	
    Smoke *smoke() { return type().smoke(); }
	
    void emitSignal() 
	{
		if(_called) return;
		_called = true;

//	QUObject *o = new QUObject[_items + 1];
		for(int i = 0; i < _items; i++) {
//	    QUObject *po = o + i + 1;
			Smoke::StackItem *si = _stack + i;
			switch(_args[i].argType) {
			case xmoc_bool:
				*_stream << si->s_bool;
				break;
			case xmoc_int:
				*_stream << si->s_int;
				break;
			case xmoc_double:
				*_stream << si->s_double;
				break;
			case xmoc_charstar:
				*_stream << (char *) si->s_voidp;
				break;
			case xmoc_QString:
				*_stream << *((QString *) si->s_voidp);
			break;
				default:
				{
					const SmokeType &t = _args[i].st;
					void *p;
					switch(t.elem()) {
					case Smoke::t_bool:
					case Smoke::t_char:
					case Smoke::t_uchar:
					case Smoke::t_short:
					case Smoke::t_ushort:
					case Smoke::t_int:
					case Smoke::t_uint:
					case Smoke::t_long:
					case Smoke::t_ulong:
					case Smoke::t_float:
					case Smoke::t_double:
						unsupported();
						break;
					case Smoke::t_enum:
						{
			    // allocate a new enum value
//			    Smoke::EnumFn fn = SmokeClass(t).enumFn();
//			    if(!fn) {
//				rb_warning("Unknown enumeration %s\n", t.name());
//				p = new int((int)si->s_enum);
//				break;
//			    }
//			    Smoke::Index id = t.typeId();
//			    (*fn)(Smoke::EnumNew, id, p, si->s_enum);
//			    (*fn)(Smoke::EnumFromLong, id, p, si->s_enum);
			    // FIXME: MEMORY LEAK
						}
						break;
					case Smoke::t_class:
					case Smoke::t_voidp:
//			p = si->s_voidp;
						break;
					default:
						p = 0;
						break;
					}
				}
			}
		}
		
		kapp->dcopClient()->emitDCOPSignal(_signalName, *_data);
    }
	
    void next() 
	{
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

class DCOPReturnValue : public Marshall {
    Smoke *_smoke;
    Smoke::Index _returnType;
    QDataStream * _retval;
    Smoke::Stack _stack;
	VALUE * _result;
public:
	DCOPReturnValue(Smoke *smoke, Smoke::Index returnType, Smoke::Stack stack, QByteArray & retval, VALUE result) :
		_smoke(smoke), _returnType(returnType), _stack(stack)
	{
		_retval = new QDataStream(retval, IO_WriteOnly);
		_result = &result;
		Marshall::HandlerFn fn = getMarshallFn(type());
		(*fn)(this);
    }

    const Smoke::Method &method() { }
//    const Smoke::Method &method() { return _smoke->methods[_method]; }
    SmokeType type() { return SmokeType(_smoke, _returnType); }
    Marshall::Action action() { return Marshall::FromVALUE; }
    Smoke::StackItem &item() { return _stack[0]; }
    VALUE * var() {
    	return _result;
    }
	
	void unsupported() 
	{
		rb_raise(rb_eArgError, "Cannot handle '%s' as DCOP return-type of %s::%s",
		type().name(),
		strcmp(_smoke->className(method().classId), "QGlobalSpace") == 0 ? "" : _smoke->className(method().classId),
		_smoke->methodNames[method().name]);
    }
    Smoke *smoke() { return _smoke; }
    void next() {}
    bool cleanup() { return false; }
};

class InvokeDCOPSlot : public Marshall {
	VALUE			_obj;
	ID				_slotname;
	int				_items;
    MocArgument *	_args;
	QDataStream *	_stream;
	QByteArray *	_retval;
	int				_cur;
	bool			_called;
	VALUE *			_sp;
	Smoke::Stack	_stack;
public:
    const MocArgument &arg() { return _args[_cur]; }
	SmokeType type() { return arg().st; }
	Marshall::Action action() { return Marshall::ToVALUE; }
	Smoke::StackItem &item() { return _stack[_cur]; }
	VALUE * var() { return _sp + _cur; }
	Smoke *smoke() { return type().smoke(); }
    bool cleanup() { return false; }
    
	void unsupported() 
	{
		rb_raise(rb_eArgError, "Cannot handle '%s' as DCOP slot argument\n", type().name());
	}
	
    void copyArguments() 
	{
	for(int i = 0; i < _items; i++) {
	    switch(_args[i].argType) {
	      case xmoc_bool:
		*_stream >> _stack[i].s_bool;
		break;
	      case xmoc_int:
		*_stream >> _stack[i].s_int;
		break;
	      case xmoc_double:
		*_stream >> _stack[i].s_double;
		break;
	      case xmoc_charstar:
		*_stream >> (char *) _stack[i].s_voidp;
		break;
	      case xmoc_QString:
		*_stream >> *(QString *) _stack[i].s_voidp;
		break;
	      default:	// case xmoc_ptr:
		{
		    const SmokeType &t = _args[i].st;
		    switch(t.elem()) {
		      case Smoke::t_bool:
		      case Smoke::t_char:
		      case Smoke::t_uchar:
		      case Smoke::t_short:
		      case Smoke::t_ushort:
		      case Smoke::t_int:
		      case Smoke::t_uint:
		      case Smoke::t_long:
		      case Smoke::t_ulong:
		      case Smoke::t_float:
		      case Smoke::t_double:
		      case Smoke::t_enum:
			{
//			    Smoke::EnumFn fn = SmokeClass(t).enumFn();
//			    if(!fn) {
//				rb_warning("Unknown enumeration %s\n", t.name());
//				_stack[i].s_enum = *(int*)p;
//				break;
//			    }
//			    Smoke::Index id = t.typeId();
//			    (*fn)(Smoke::EnumToLong, id, p, _stack[i].s_enum);
			}
			unsupported();
			break;
		      case Smoke::t_class:
		      case Smoke::t_voidp:
			_stack[i].s_voidp = 0;
			break;
		    }
		}
	    }
	}
	}
	
    void invokeSlot() 
	{
		if (_called) {
			return;
		}
		_called = true;
        VALUE result = rb_funcall2(_obj, _slotname, _items, _sp);
		
		DCOPReturnValue r(smoke(), 0, _stack, *_retval, result);
   }

    void next() 
	{
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

    InvokeDCOPSlot(VALUE obj, ID slotname, VALUE args, QByteArray &data, QByteArray & returnValue) :
		_obj(obj), _slotname(slotname), _cur(-1), _called(false)
	{
		_items = NUM2INT(rb_ary_entry(args, 0));
		_stream = new QDataStream(data, IO_ReadOnly);
		_retval = &returnValue;

		Data_Get_Struct(rb_ary_entry(args, 1), MocArgument, _args);
		_sp = ALLOC_N(VALUE, _items);
		_stack = new Smoke::StackItem[_items];
		copyArguments();
    }

	~InvokeDCOPSlot() {
		delete[] _stack;
	}
};

extern "C" {
extern void Init_Qt();
extern void set_new_kde(VALUE (*new_kde) (int, VALUE *, VALUE));
extern VALUE new_qt(int argc, VALUE * argv, VALUE klass);
extern VALUE qt_module;
extern VALUE qt_internal_module;
extern VALUE qt_base_class;
extern VALUE kde_module;
extern VALUE kio_module;
extern VALUE kparts_module;
extern VALUE khtml_module;

static VALUE kde_internal_module;

VALUE
getdcopinfo(VALUE self, char * signalname)
{
    VALUE member = rb_funcall(	kde_internal_module, 
								rb_intern("fullSignalName"), 
								2, self, rb_str_new2(signalname) );
    return rb_funcall(	qt_internal_module, 
						rb_intern("getMocArguments"), 
						1, member );
}

VALUE
k_dcop_signal(int argc, VALUE * argv, VALUE self)
{
    smokeruby_object *o = value_obj_info(self);
	
    char * signalname = rb_id2name(rb_frame_last_func());
    VALUE args = getdcopinfo(self, signalname);

    if(args == Qnil) return Qfalse;

    EmitDCOPSignal signal(self, signalname, argc, argv, args);
    signal.next();

    return Qtrue;
}

static VALUE
dcop_functions(VALUE self)
{
	VALUE dcopObject = rb_funcall(kde_module, rb_intern("createDCOPObject"), 1, self);
	return rb_funcall(dcopObject, rb_intern("functions"), 0);
}

static VALUE
dcop_interfaces(VALUE self)
{
	VALUE dcopObject = rb_funcall(kde_module, rb_intern("createDCOPObject"), 1, self);
	return rb_funcall(dcopObject, rb_intern("interfaces"), 0);
}

static VALUE
dcop_process(VALUE self, VALUE target, VALUE slotname, VALUE args, VALUE data, VALUE replyType, VALUE replyData)
{
	VALUE _data = rb_funcall(qt_internal_module, rb_intern("get_qbytearray"), 1, data);
	QByteArray * dataArray = 0;
	Data_Get_Struct(_data, QByteArray, dataArray);
	
	VALUE _replyData = rb_funcall(qt_internal_module, rb_intern("get_qbytearray"), 1, replyData);
	QByteArray * replyArray = 0;
	Data_Get_Struct(_replyData, QByteArray, replyArray);
	InvokeDCOPSlot slot(target, rb_intern(STR2CSTR(slotname)), args, *dataArray, *replyArray);
	slot.next();
	
	return Qtrue;
}

static VALUE
new_kde(int argc, VALUE * argv, VALUE klass)
{
	VALUE instance = new_qt(argc, argv, klass);
	
	if (rb_funcall(kde_module, rb_intern("hasDCOPSignals"), 1, klass) == Qtrue) {
		VALUE signalNames = rb_funcall(kde_module, rb_intern("getDCOPSignalNames"), 1, klass);
		for (long index = 0; index < RARRAY(signalNames)->len; index++) {
			VALUE signal = rb_ary_entry(signalNames, index);
			rb_define_method(klass, STR2CSTR(signal), (VALUE (*) (...)) k_dcop_signal, -1);
		}
	}
	
	if (rb_funcall(kde_module, rb_intern("hasDCOPSlots"), 1, klass) == Qtrue) {
		rb_funcall(kde_module, rb_intern("createDCOPObject"), 1, instance);
		rb_define_method(klass, "interfaces", (VALUE (*) (...)) dcop_interfaces, 0);
		rb_define_method(klass, "functions", (VALUE (*) (...)) dcop_functions, 0);
	}
	
	return instance;
}

void
Init_Korundum()
{
	set_new_kde(new_kde);
	
	// The Qt extension is linked against libsmokeqt.so, but Korundum links against
	// libsmokekde.so only. Specifying both a 'require Qt' and a 'require Korundum',
	// would give a link error.
	// So call the Init_Qt() initialization function explicitely, not via 'require Qt'
	// (Qt.o is linked into libqtruby.so, as well as the Qt.so extension).
	Init_Qt();
    install_handlers(KDE_handlers);
	
    kde_internal_module = rb_define_module_under(kde_module, "Internal");
	rb_define_singleton_method(kde_module, "dcop_process", (VALUE (*) (...)) dcop_process, 6);
	
	rb_require("KDE/Korundum.rb");
}

};
