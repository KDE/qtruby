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
#include <dcopobject.h>
#include <kapplication.h>

#include <ruby.h>

#include <marshall.h>
#include <qtruby.h>
#include <smokeruby.h>
#include <smoke.h>

extern TypeHandler KDE_handlers[];
extern void install_handlers(TypeHandler *);

Marshall::HandlerFn getMarshallFn(const SmokeType &type);

/*
	Copy items from the stream to the stack, each item has a corresponding description in the 
	args array of MocArguments. Used for marshalling DCOP replies and emitting DCOP signals.
*/
static void
smokeStackToStream(Marshall *m, Smoke::Stack stack, QDataStream* stream, int items, MocArgument* args)
{
	for(int i = 0; i < items; i++) {
		Smoke::StackItem *si = stack + i;
			
		switch(args[i].argType) {
		case xmoc_bool:
			*stream << si->s_bool;
			break;
		case xmoc_int:
			*stream << si->s_int;
			break;
		case xmoc_double:
			*stream << si->s_double;
			break;
		case xmoc_charstar:
			*stream << (char *) si->s_voidp;
			break;
		case xmoc_QString:
			{
				QString temp((const QString&) *((QString *) si->s_voidp));
				*stream << temp;
			}
			break;
		default:
			{
				const SmokeType &t = args[i].st;
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
					m->unsupported();
					break;
				case Smoke::t_enum:
					break;
				case Smoke::t_class:
				case Smoke::t_voidp:
					{
						// Look for methods of the form: QDataStream & operator<<(QDataStream&, const MyClass&)
						Smoke::Index meth = t.smoke()->findMethod("QGlobalSpace", "operator<<##");
						Smoke::Index i;
						if (meth > 0) {
							i = t.smoke()->methodMaps[meth].method;
							i = -i;		// turn into ambiguousMethodList index
							while (t.smoke()->ambiguousMethodList[i]) {
								Smoke::Method &method = t.smoke()->methods[t.smoke()->ambiguousMethodList[i]];
								QString	refType("const ");
								refType += t.name();
								refType += "&";
								if (	strcmp(	"QDataStream&", 
												t.smoke()->types[t.smoke()->argumentList[method.args+0]].name ) == 0 
										&& strcmp(	(const char *) refType, 
													t.smoke()->types[t.smoke()->argumentList[method.args+1]].name ) == 0 ) 
								{
									Smoke::ClassFn fn = t.smoke()->classes[method.classId].classFn;
									Smoke::Stack local_stack = new Smoke::StackItem[3];
									local_stack[1].s_voidp = stream;
									local_stack[2].s_voidp = si->s_voidp;
									// Call the QDataStream marshaller write method
									// with the instance to be marshalled
									(*fn)(method.method, 0, local_stack);
									delete local_stack;
									break;
								}
								i++;
							}
						}
					}
					break;
				default:
					break;
				}
			}
		}
	}
	return;
}

/*
	Copy items from the stack to the stream, each item has a corresponding description in the 
	args array of MocArguments. Used for marshalling the arguments to a DCOP slot invocation.
*/
static void
smokeStackFromStream(Marshall *m, Smoke::Stack stack, QDataStream* stream, int items, MocArgument* args)
{
	for(int i = 0; i < items; i++) {
		Smoke::StackItem *si = stack + i;
		
		switch(args[i].argType) {
		case xmoc_bool:
			*stream >> stack[i].s_bool;
			break;
		case xmoc_int:
			*stream >> stack[i].s_int;
			break;
		case xmoc_double:
			*stream >> stack[i].s_double;
			break;
		case xmoc_charstar:
			*stream >> (char *) stack[i].s_voidp;
			break;
		case xmoc_QString:
			{
				QString temp;
				*stream >> temp;
				stack[i].s_voidp = new QString(temp);
			}
			break;
		default:	// case xmoc_ptr:
			{
				const SmokeType &t = args[i].st;
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
					m->unsupported();
					break;
				case Smoke::t_class:
				case Smoke::t_voidp:
					{
						// Look for methods of the form: QDataStream & operator>>(QDataStream&, MyClass&)
						Smoke::Index meth = t.smoke()->findMethod("QGlobalSpace", "operator>>##");
						Smoke::Index i;
						if (meth > 0) {
							i = t.smoke()->methodMaps[meth].method;
							i = -i;		// turn into ambiguousMethodList index
							while (t.smoke()->ambiguousMethodList[i]) {
								Smoke::Method &method = t.smoke()->methods[t.smoke()->ambiguousMethodList[i]];
								QString	refType(t.name());
								refType += "&";
								if (	strcmp(	"QDataStream&", 
												t.smoke()->types[t.smoke()->argumentList[method.args+0]].name ) == 0 
										&& strcmp(	(const char *) refType, 
													t.smoke()->types[t.smoke()->argumentList[method.args+1]].name ) == 0 ) 
								{
									Smoke::ClassFn fn = t.smoke()->classes[method.classId].classFn;
									Smoke::Stack local_stack = new Smoke::StackItem[3];
									local_stack[1].s_voidp = stream;
									local_stack[2].s_voidp = si->s_voidp; 
									// Call the QDataStream marshaller read method
									// on the instance to be marshalled
									(*fn)(method.method, 0, local_stack);
									delete local_stack;
									break;
								}
								i++;
							}
						}					
					}
					break;
				}
			}
	    }
	}
}
		
class EmitDCOPSignal : public Marshall {
	DCOPObject * _obj;
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
    EmitDCOPSignal(DCOPObject * obj, char * signalName, int items, VALUE *sp, VALUE args) :
		_obj(obj), _signalName(signalName), _sp(sp), _items(items), _cur(-1), _called(false)
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

		smokeStackToStream(this, _stack, _stream, _items, _args);
		_obj->emitDCOPSignal(_signalName, *_data);
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

class DCOPReplyValue : public Marshall {
    MocArgument *	_replyType;
    QDataStream * _retval;
    Smoke::Stack _stack;
	VALUE * _result;
public:
	DCOPReplyValue(QByteArray & retval, VALUE * result, VALUE replyType) 
	{
		_retval = new QDataStream(retval, IO_WriteOnly);
		_result = result;
		Data_Get_Struct(rb_ary_entry(replyType, 1), MocArgument, _replyType);
		_stack = new Smoke::StackItem[1];
		Marshall::HandlerFn fn = getMarshallFn(type());
		(*fn)(this);
		smokeStackToStream(this, _stack, _retval, 1, _replyType);
    }

    SmokeType type() { 
		return _replyType[0].st; 
	}
    Marshall::Action action() { return Marshall::FromVALUE; }
    Smoke::StackItem &item() { return _stack[0]; }
    VALUE * var() {
    	return _result;
    }
	
	void unsupported() 
	{
		rb_raise(rb_eArgError, "Cannot handle '%s' as DCOP reply-type", type().name());
    }
	Smoke *smoke() { return type().smoke(); }
    
	void next() {}
    
	bool cleanup() { return false; }
};

class InvokeDCOPSlot : public Marshall {
	VALUE			_obj;
	ID				_slotname;
	int				_items;
    MocArgument *	_args;
	QDataStream *	_stream;
	VALUE			_replyType;
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
		smokeStackFromStream(this, _stack, _stream, _items, _args);
		return;
	}
	
    void invokeSlot() 
	{
		if (_called) {
			return;
		}
		_called = true;
        VALUE result = rb_funcall2(_obj, _slotname, _items, _sp);
		if (_replyType != Qnil) {
			DCOPReplyValue dcopReply(*_retval, &result, _replyType);
		}
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

    InvokeDCOPSlot(VALUE obj, ID slotname, VALUE args, QByteArray& data, VALUE replyType, QByteArray& returnValue) :
		_obj(obj), _slotname(slotname), _replyType(replyType), _cur(-1), _called(false)
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
extern void Init_qtruby();
extern void set_new_kde(VALUE (*new_kde) (int, VALUE *, VALUE));
extern VALUE new_qt(int argc, VALUE * argv, VALUE klass);
extern VALUE new_qobject(int argc, VALUE * argv, VALUE klass);
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
	VALUE dcopObject = rb_funcall(kde_module, rb_intern("createDCOPObject"), 1, self);
    smokeruby_object *o = value_obj_info(dcopObject);
	
    char * signalname = rb_id2name(rb_frame_last_func());
    VALUE args = getdcopinfo(self, signalname);

    if(args == Qnil) return Qfalse;

    EmitDCOPSignal signal((DCOPObject *) o->ptr, signalname, argc, argv, args);
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
dcop_connect_signal(VALUE self, VALUE sender, VALUE senderObj, VALUE signal, VALUE slot, VALUE volatile_connect)
{
	VALUE dcopObject = rb_funcall(kde_module, rb_intern("createDCOPObject"), 1, self);
	return rb_funcall(dcopObject, rb_intern("connectDCOPSignal"), 5, sender, senderObj, signal, slot, volatile_connect);
}

static VALUE
dcop_disconnect_signal(VALUE self, VALUE sender, VALUE senderObj, VALUE signal, VALUE slot)
{
	VALUE dcopObject = rb_funcall(kde_module, rb_intern("createDCOPObject"), 1, self);
	return rb_funcall(dcopObject, rb_intern("disconnectDCOPSignal"), 4, sender, senderObj, signal, slot);
}

static VALUE
dcop_process(VALUE /*self*/, VALUE target, VALUE slotname, VALUE args, VALUE data, VALUE replyType, VALUE replyData)
{
	VALUE _data = rb_funcall(qt_internal_module, rb_intern("get_qbytearray"), 1, data);
	QByteArray * dataArray = 0;
	Data_Get_Struct(_data, QByteArray, dataArray);
	
	VALUE _replyData = rb_funcall(qt_internal_module, rb_intern("get_qbytearray"), 1, replyData);
	QByteArray * replyArray = 0;
	Data_Get_Struct(_replyData, QByteArray, replyArray);

	InvokeDCOPSlot dcopSlot(target, rb_intern(STR2CSTR(slotname)), args, *dataArray, replyType, *replyArray);
	dcopSlot.next();
	
	return Qtrue;
}

static VALUE
new_kde(int argc, VALUE * argv, VALUE klass)
{
	// Note this should really call only new_qobject if the instance is a QObject,
	// and otherwise call new_qt().
	VALUE instance = new_qobject(argc, argv, klass);
	
	if (rb_funcall(kde_module, rb_intern("hasDCOPSignals"), 1, klass) == Qtrue) {
		VALUE signalNames = rb_funcall(kde_module, rb_intern("getDCOPSignalNames"), 1, klass);
		for (long index = 0; index < RARRAY(signalNames)->len; index++) {
			VALUE signal = rb_ary_entry(signalNames, index);
			rb_define_method(klass, STR2CSTR(signal), (VALUE (*) (...)) k_dcop_signal, -1);
		}
	}
	
	if (	rb_funcall(kde_module, rb_intern("hasDCOPSlots"), 1, klass) == Qtrue
			|| rb_funcall(kde_module, rb_intern("hasDCOPSignals"), 1, klass) == Qtrue ) 
	{
		rb_funcall(kde_module, rb_intern("createDCOPObject"), 1, instance);
		rb_define_method(klass, "interfaces", (VALUE (*) (...)) dcop_interfaces, 0);
		rb_define_method(klass, "functions", (VALUE (*) (...)) dcop_functions, 0);
		rb_define_method(klass, "connectDCOPSignal", (VALUE (*) (...)) dcop_connect_signal, 5);
		rb_define_method(klass, "disconnectDCOPSignal", (VALUE (*) (...)) dcop_disconnect_signal, 4);
	}
	
	return instance;
}

void
Init_korundum()
{
	set_new_kde(new_kde);
	
	// The Qt extension is linked against libsmokeqt.so, but Korundum links against
	// libsmokekde.so only. Specifying both a 'require Qt' and a 'require Korundum',
	// would give a link error.
	// So call the Init_qtruby() initialization function explicitely, not via 'require Qt'
	// (Qt.o is linked into libqtruby.so, as well as the Qt.so extension).
	Init_qtruby();
    install_handlers(KDE_handlers);
	
    kde_internal_module = rb_define_module_under(kde_module, "Internal");
	rb_define_singleton_method(kde_module, "dcop_process", (VALUE (*) (...)) dcop_process, 6);
	
	rb_require("KDE/korundum.rb");
}

};
