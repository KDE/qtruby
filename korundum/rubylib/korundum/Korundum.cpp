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
#include <dcopclient.h>
#include <dcopobject.h>
#include <dcopref.h>
#include <kapplication.h>
#include <kurl.h>
#if KDE_VERSION >= 0x030200
#include <kconfigskeleton.h>
#endif
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

/*
	Copy items from the stack to the stream, each item has a corresponding description in the 
	args array of MocArguments. Used for marshalling the args to DCOP calls and sends, emitting 
	DCOP signals, and converting the return value of a DCOP slot to a stream.
*/
static void
smokeStackToStream(Marshall *m, Smoke::Stack stack, QDataStream* stream, int items, MocArgument* args)
{
	for(int i = 0; i < items; i++) {
		switch(args[i].argType) {
		case xmoc_bool:
			*stream << stack[i].s_bool;
			break;
		case xmoc_int:
			*stream << stack[i].s_int;
			break;
		case xmoc_double:
			*stream << stack[i].s_double;
			break;
		case xmoc_charstar:
			*stream << (char *) stack[i].s_voidp;
			break;
		case xmoc_QString:
			{
				QString temp((const QString&) *((QString *) stack[i].s_voidp));
				*stream << temp;
			}
			break;
		default:
			{
				const SmokeType &t = args[i].st;
				switch(t.elem()) {
				case Smoke::t_bool:
					*stream << stack[i].s_bool;
					break;
				case Smoke::t_char:
					*stream << stack[i].s_char;
					break;
				case Smoke::t_uchar:
					*stream << stack[i].s_uchar;
					break;
				case Smoke::t_short:
					*stream << stack[i].s_short;
					break;
				case Smoke::t_ushort:
					*stream << stack[i].s_ushort;
					break;
				case Smoke::t_int:
					*stream << stack[i].s_int;
					break;
				case Smoke::t_uint:
					*stream << stack[i].s_uint;
					break;
				case Smoke::t_long:
					*stream << stack[i].s_long;
					break;
				case Smoke::t_ulong:
					*stream << stack[i].s_ulong;
					break;
				case Smoke::t_float:
					*stream << stack[i].s_float;
					break;
				case Smoke::t_double:
					*stream << stack[i].s_double;
					break;
				case Smoke::t_enum:
					m->unsupported();
					break;
				case Smoke::t_class:
				case Smoke::t_voidp:
					{
						// Special case any types which are in the Smoke runtime, but
						// don't have QDataStream '<<' methods
						if (strcmp(t.name(), "QCString") == 0) {
							QCString temp((const QCString&) *((QCString *) stack[i].s_voidp));
							*stream << temp;
							break;
						} else if (strcmp(t.name(), "QCStringList") == 0) {
							QCStringList temp((const QCStringList&) *((QCStringList *) stack[i].s_voidp));
							*stream << temp;
							break;
						} else if (strcmp(t.name(), "QStringList") == 0) {
							QStringList temp((const QStringList&) *((QStringList *) stack[i].s_voidp));
							*stream << temp;
							break;
						} else if (strcmp(t.name(), "KURL::List") == 0) {
							KURL::List temp((const KURL::List&) *((KURL::List *) stack[i].s_voidp));
							*stream << temp;
							break;
						} else if (strcmp(t.name(), "QMap<QCString,DCOPRef>") == 0) {
							QMap<QCString,DCOPRef> temp((const QMap<QCString,DCOPRef>&) *((QMap<QCString,DCOPRef>*) stack[i].s_voidp));
							*stream << temp;
							break;
						}
						
						// Look for methods of the form: QDataStream & operator<<(QDataStream&, const MyClass&)
						Smoke::Index meth = t.smoke()->findMethod("QGlobalSpace", "operator<<##");
						Smoke::Index ix;
						if (meth > 0) {
							ix = t.smoke()->methodMaps[meth].method;
							ix = -ix;		// turn into ambiguousMethodList index
							while (t.smoke()->ambiguousMethodList[ix]) {
								Smoke::Method &method = t.smoke()->methods[t.smoke()->ambiguousMethodList[ix]];
								QString	refType("const ");
								refType += t.name();
								refType += "&";
								if (	strcmp(	"QDataStream&", 
												t.smoke()->types[t.smoke()->argumentList[method.args+0]].name ) == 0 
										&& strcmp(	refType.latin1(), 
													t.smoke()->types[t.smoke()->argumentList[method.args+1]].name ) == 0 ) 
								{
									Smoke::ClassFn fn = t.smoke()->classes[method.classId].classFn;
									Smoke::StackItem local_stack[3];
									local_stack[1].s_voidp = stream;
									local_stack[2].s_voidp = stack[i].s_voidp;
									// Call the QDataStream marshaller write method
									// with the instance to be marshalled
									(*fn)(method.method, 0, local_stack);
									break;
								}
								ix++;
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
	Copy items from the stream to the stack, each item has a corresponding description in the 
	args array of MocArguments. Used for marshalling the arguments to a DCOP slot invocation,
	and for converting a dcop reply to a ruby value.
*/
static void
smokeStackFromStream(Marshall *m, Smoke::Stack stack, QDataStream* stream, int items, MocArgument* args)
{
	for(int i = 0; i < items; i++) {
		switch(args[i].argType) {
		case xmoc_bool: 
			{
			*stream >> stack[i].s_bool;
			break;
			}
		case xmoc_int:
			{
			*stream >> stack[i].s_int;
			break;
			}
		case xmoc_double:
			*stream >> stack[i].s_double;
			break;
		case xmoc_charstar:
			*stream >> (char *&) stack[i].s_voidp;
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
					{
					*stream >> stack[i].s_bool;
					break;
					}
				case Smoke::t_char:
					{
					*stream >> stack[i].s_char;
					break;
					}
				case Smoke::t_uchar:
					{
					*stream >> stack[i].s_uchar;
					break;
					}
				case Smoke::t_short:
					{
					*stream >> stack[i].s_short;
					break;
					}
				case Smoke::t_ushort:
					{
					*stream >> stack[i].s_ushort;
					break;
					}
				case Smoke::t_int:
					{
					*stream >> stack[i].s_int;
					break;
					}
				case Smoke::t_uint:
					{
					*stream >> stack[i].s_uint;
					break;
					}
				case Smoke::t_long:
					{
					*stream >> stack[i].s_long;
					break;
					}
				case Smoke::t_ulong:
					{
					*stream >> stack[i].s_ulong;
					break;
					}
				case Smoke::t_float:
					*stream >> stack[i].s_float;
					break;
				case Smoke::t_double:
					*stream >> stack[i].s_double;
					break;
				case Smoke::t_enum:
					m->unsupported();
					break;
				case Smoke::t_class:
				case Smoke::t_voidp:
					{
						// Special case any types which are in the Smoke runtime, but
						// don't have QDataStream '>>' methods
						if (strcmp(t.name(), "QCString") == 0) {
							QCString temp;
							*stream >> temp;
							stack[i].s_voidp = new QCString(temp);
							break;
						} else if (strcmp(t.name(), "QCStringList") == 0) {
							QCStringList temp;
							*stream >> temp;
							stack[i].s_voidp = new QCStringList(temp);
							break;
						} else if (strcmp(t.name(), "QStringList") == 0) {
							QStringList temp;
							*stream >> temp;
							stack[i].s_voidp = new QStringList(temp);
							break;
						} else if (strcmp(t.name(), "KURL::List") == 0) {
							KURL::List temp;
							*stream >> temp;
							stack[i].s_voidp = new KURL::List(temp);
							break;
						} else if (strcmp(t.name(), "QMap<QCString,DCOPRef>") == 0) {
							QMap<QCString,DCOPRef> temp;
							*stream >> temp;
							stack[i].s_voidp = new QMap<QCString,DCOPRef>(temp);
							break;
						}
						
						// First construct an instance to read the QDataStream into,
						// so look for a no args constructor
    					Smoke::Index ctorId = t.smoke()->idMethodName(t.name());
						Smoke::Index ctorMeth = t.smoke()->findMethod(t.classId(), ctorId);
						Smoke::Index ctor = t.smoke()->methodMaps[ctorMeth].method;
						if(ctor < 1) {
							stack[i].s_voidp = 0;
							m->unsupported();
							break; // Ambiguous or non-existent method, shouldn't happen with a no arg constructor
						}
						// Okay, ctor is the constructor. Time to call it.
						Smoke::StackItem ctor_stack[1];
						ctor_stack[0].s_voidp = 0;
						Smoke::ClassFn classfn = t.smoke()->classes[t.classId()].classFn;
						(*classfn)(t.smoke()->methods[ctor].method, 0, ctor_stack);
						stack[i].s_voidp = ctor_stack[0].s_voidp;
						
						// Look for methods of the form: QDataStream & operator>>(QDataStream&, MyClass&)
						Smoke::Index meth = t.smoke()->findMethod("QGlobalSpace", "operator>>##");
						Smoke::Index ix;
						if (meth > 0) {
							ix = t.smoke()->methodMaps[meth].method;
							ix = -ix;		// turn into ambiguousMethodList index
							while (t.smoke()->ambiguousMethodList[ix]) {
								Smoke::Method &method = t.smoke()->methods[t.smoke()->ambiguousMethodList[ix]];
								QString	refType(t.name());
								refType += "&";
								if (	strcmp(	"QDataStream&", 
												t.smoke()->types[t.smoke()->argumentList[method.args+0]].name ) == 0 
										&& strcmp(	refType.latin1(), 
													t.smoke()->types[t.smoke()->argumentList[method.args+1]].name ) == 0 ) 
								{
									Smoke::ClassFn fn = t.smoke()->classes[method.classId].classFn;
									Smoke::StackItem local_stack[3];
									local_stack[1].s_voidp = stream;
									local_stack[2].s_voidp = stack[i].s_voidp;
									// Call the QDataStream marshaller read method
									// on the instance to be marshalled
									(*fn)(method.method, 0, local_stack);
									break;
								}
								ix++;
							}
						}					
					}
					break;
				}
			}
	    }
	}
}

/*
	Converts a QByteArray returned from a DCOP call to a ruby value.
*/
class DCOPReturn : public Marshall {
    MocArgument *	_replyType;
    Smoke::Stack _stack;
	VALUE * _result;
public:
	DCOPReturn(QDataStream & retval, VALUE * result, VALUE replyType) 
	{
		_result = result;
		VALUE temp = rb_funcall(qt_internal_module, rb_intern("getMocArguments"), 1, replyType);
		Data_Get_Struct(rb_ary_entry(temp, 1), MocArgument, _replyType);
		_stack = new Smoke::StackItem[1];
		smokeStackFromStream(this, _stack, &retval, 1, _replyType);
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
		rb_raise(rb_eArgError, "Cannot handle '%s' as DCOP return-type", type().name());
    }
	Smoke *smoke() { return type().smoke(); }
    
	void next() {}
    
	bool cleanup() { return false; }
	
	~DCOPReturn() 
	{
		delete[] _stack;
	}
};

class DCOPCall : public Marshall {
	VALUE _obj;
	QCString & _remFun;
    int _items;
    VALUE *_sp;
	QByteArray *_data;
	QDataStream *_stream;
    int _id;
    MocArgument *_args;
	bool _useEventLoop;
	int _timeout;
    int _cur;
    Smoke::Stack _stack;
    VALUE _result;
    bool _called;
public:
    DCOPCall(VALUE obj, QCString & remFun, int items, VALUE *sp, VALUE args, bool useEventLoop, int timeout) :
		_obj(obj), _remFun(remFun), _items(items), _sp(sp),
		_useEventLoop(useEventLoop), _timeout(timeout), _cur(-1), _called(false)
    {
		_data = new QByteArray();
		_stream = new QDataStream(*_data, IO_WriteOnly);
		Data_Get_Struct(rb_ary_entry(args, 1), MocArgument, _args);
		_stack = new Smoke::StackItem[_items];
		_result = Qnil;
    }
	
	~DCOPCall() 
	{
		delete[] _stack;
		delete _data;
		delete _stream;
	}
    const MocArgument &arg() { return _args[_cur]; }
    SmokeType type() { return arg().st; }
    Marshall::Action action() { return Marshall::FromVALUE; }
    Smoke::StackItem &item() { return _stack[_cur]; }
    VALUE * var() {
	if(_cur < 0) return &_result;
	return _sp + _cur;
    }
	
    void unsupported() 
	{
		rb_raise(rb_eArgError, "Cannot handle '%s' as a DCOP call argument", type().name());
    }
	
    Smoke *smoke() { return type().smoke(); }
	
    void dcopCall() 
	{
		if(_called) return;
		_called = true;

		smokeStackToStream(this, _stack, _stream, _items, _args);
		smokeruby_object *o = value_obj_info(_obj);
		DCOPRef * dcopRef = (DCOPRef *) o->smoke->cast(o->ptr, o->classId, o->smoke->idClass("DCOPRef"));
		DCOPClient* dc = dcopRef->dcopClient();
		QCString replyType;
		QByteArray dataReceived;
#if KDE_VERSION >= 0x030200
		bool ok = dc->call(dcopRef->app(), dcopRef->obj(), _remFun, *_data, replyType, dataReceived, _useEventLoop, _timeout);
#else
		bool ok = dc->call(dcopRef->app(), dcopRef->obj(), _remFun, *_data, replyType, dataReceived, _useEventLoop);
#endif
		
		if (!ok) {
			// Note that a failed dcop call returns 'nil', not 'false'
			_result = Qnil;
			return;
		} else if (replyType == "void" || replyType == "ASYNC") {
			_result = Qtrue;
			return;
		}
		
		QDataStream ds(dataReceived, IO_ReadOnly);
		
		if (replyType == "QValueList<DCOPRef>") {
			// Special case QValueList<DCOPRef> as a QDataStream marshaller 
			// isn't in the Smoke runtime
			QValueList<DCOPRef> valuelist;
			ds >> valuelist;
			_result = rb_ary_new();
			for (QValueListIterator<DCOPRef> it = valuelist.begin(); it != valuelist.end(); ++it) {
				void *p = new DCOPRef(*it);
				VALUE obj = getPointerObject(p);
				
				if (obj == Qnil) {
					smokeruby_object  * o = ALLOC(smokeruby_object);
					o->classId = qt_Smoke->idClass("DCOPRef");
					o->smoke = qt_Smoke;
					o->ptr = p;
					o->allocated = true;
					obj = set_obj_info("KDE::DCOPRef", o);
				}
				
				rb_ary_push(_result, obj);
			}
		} else if (replyType == "QValueList<QCString>") {
			// And special case this type too 
			QValueList<QCString> propertyList;
			ds >> propertyList;
			_result = rb_ary_new();
			for (QValueListIterator<QCString> it = propertyList.begin(); it != propertyList.end(); ++it) {
				rb_ary_push(_result, rb_str_new2((const char *) *it));
			}
		} else if (replyType == "QMap<QString,DCOPRef>") {
			// And another.. 
			QMap<QString,DCOPRef>	actionMap;
			ds >> actionMap;
			_result = rb_hash_new();
			
			QMap<QString,DCOPRef>::Iterator it;
			for (it = actionMap.begin(); it != actionMap.end(); ++it) {
				void *p = new DCOPRef(it.data());
				VALUE obj = getPointerObject(p);
				
				if (obj == Qnil) {
					smokeruby_object  * o = ALLOC(smokeruby_object);
					o->classId = qt_Smoke->idClass("DCOPRef");
					o->smoke = qt_Smoke;
					o->ptr = p;
					o->allocated = true;
					obj = set_obj_info("KDE::DCOPRef", o);
				}
				
				rb_hash_aset(_result, rb_str_new2(it.key().latin1()), obj);
        	}		
		} else {
			DCOPReturn dcopReturn(ds, &_result, rb_str_new2((const char *) replyType));
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

		dcopCall();
		_cur = oldcur;
    }
	
    bool cleanup() { return true; }
};

class DCOPSend : public Marshall {
	VALUE _obj;
	QCString & _remFun;
	QByteArray *_data;
	QDataStream *_stream;
    int _id;
    MocArgument *_args;
    int _items;
    VALUE *_sp;
    int _cur;
	VALUE * _result;
    Smoke::Stack _stack;
    bool _called;
public:
    DCOPSend(VALUE obj, QCString & remFun, int items, VALUE *sp, VALUE args, VALUE * result) :
		_obj(obj), _remFun(remFun), _items(items), _sp(sp), _cur(-1), _result(result), _called(false)
    {
		_data = new QByteArray();
		_stream = new QDataStream(*_data, IO_WriteOnly);
		Data_Get_Struct(rb_ary_entry(args, 1), MocArgument, _args);
		_stack = new Smoke::StackItem[_items];
    }
	
	~DCOPSend() 
	{
		delete[] _stack;
		delete _data;
		delete _stream;
	}
    const MocArgument &arg() { return _args[_cur]; }
    SmokeType type() { return arg().st; }
    Marshall::Action action() { return Marshall::FromVALUE; }
    Smoke::StackItem &item() { return _stack[_cur]; }
    VALUE * var() { return _sp + _cur; }
	
    void unsupported() 
	{
		rb_raise(rb_eArgError, "Cannot handle '%s' as a DCOP send argument", type().name());
    }
	
    Smoke *smoke() { return type().smoke(); }
	
    void dcopSend() 
	{
		if(_called) return;
		_called = true;

		smokeStackToStream(this, _stack, _stream, _items, _args);
		smokeruby_object *o = value_obj_info(_obj);
		DCOPRef * dcopRef = (DCOPRef *) o->smoke->cast(o->ptr, o->classId, o->smoke->idClass("DCOPRef"));
		DCOPClient* dc = dcopRef->dcopClient();
		bool ok = dc->send(dcopRef->app(), dcopRef->obj(), _remFun, *_data);
		*_result = (ok ? Qtrue : Qfalse);
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

		dcopSend();
		_cur = oldcur;
    }
	
    bool cleanup() { return true; }
};
		
class EmitDCOPSignal : public Marshall {
	VALUE _obj;
	const char * _signalName;
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
    EmitDCOPSignal(VALUE obj, const char * signalName, int items, VALUE *sp, VALUE args) :
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
		smokeruby_object *o = value_obj_info(_obj);
		DCOPObject * dcopObject = (DCOPObject *) o->smoke->cast(o->ptr, o->classId, o->smoke->idClass("DCOPObject"));
		dcopObject->emitDCOPSignal(_signalName, *_data);
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

/*
	Converts a ruby value returned by a DCOP slot invocation to a QByteArray
*/
class DCOPReplyValue : public Marshall {
    MocArgument *	_replyType;
    Smoke::Stack _stack;
	VALUE * _result;
public:
	DCOPReplyValue(QByteArray & retval, VALUE * result, VALUE replyType) 
	{
		QDataStream _retval(retval, IO_WriteOnly);
		_result = result;
		Data_Get_Struct(rb_ary_entry(replyType, 1), MocArgument, _replyType);
		_stack = new Smoke::StackItem[1];
		Marshall::HandlerFn fn = getMarshallFn(type());
		(*fn)(this);
		smokeStackToStream(this, _stack, &_retval, 1, _replyType);
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
	
	~DCOPReplyValue() {
		delete[] _stack;
	}
};

class InvokeDCOPSlot : public Marshall {
	VALUE			_obj;
	ID				_slotname;
	int				_items;
    MocArgument *	_args;
	QDataStream *	_stream;
	const char *	_replyTypeName;
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
		
		if (	strcmp(_replyTypeName, "QValueList<DCOPRef>") == 0
				&& TYPE(result) == T_ARRAY ) 
		{
			// Special case QValueList<DCOPRef> as a QDataStream marshaller 
			// isn't in the Smoke runtime
			QValueList<DCOPRef> windowList;
			
			for (long i = 0; i < RARRAY(result)->len; i++) {
				VALUE item = rb_ary_entry(result, i);
				smokeruby_object *o = value_obj_info(item);
				if( !o || !o->ptr)
                    continue;
				void * ptr = o->ptr;
				ptr = o->smoke->cast(ptr, o->classId, o->smoke->idClass("DCOPRef"));
				windowList.append((DCOPRef)*(DCOPRef*)ptr);
			}
			QDataStream retval(*_retval, IO_WriteOnly);
			retval << windowList;
		} else if (	strcmp(_replyTypeName, "QValueList<QCString>") == 0
					&& TYPE(result) == T_ARRAY ) 
		{
			// And special case this type too 
			QValueList<QCString> propertyList;
			
			for (long i = 0; i < RARRAY(result)->len; i++) {
				VALUE item = rb_ary_entry(result, i);
				propertyList.append(QCString(StringValuePtr(item)));
			}
			QDataStream retval(*_retval, IO_WriteOnly);
			retval << propertyList;
		} else if (	strcmp(_replyTypeName, "QMap<QString,DCOPRef>") == 0
					&& TYPE(result) == T_HASH ) 
		{
			// And another.. 
			QMap<QString,DCOPRef> actionMap;
			// Convert the ruby hash to an array of key/value arrays
			VALUE temp = rb_funcall(result, rb_intern("to_a"), 0);

			for (long i = 0; i < RARRAY(temp)->len; i++) {
				VALUE action = rb_ary_entry(rb_ary_entry(temp, i), 0);
				VALUE item = rb_ary_entry(rb_ary_entry(temp, i), 1);
				
				smokeruby_object *o = value_obj_info(item);
				if( !o || !o->ptr)
                    continue;
				void * ptr = o->ptr;
				ptr = o->smoke->cast(ptr, o->classId, o->smoke->idClass("DCOPRef"));
				
				actionMap[QString(StringValuePtr(action))] = (DCOPRef)*(DCOPRef*)ptr;
			}
			QDataStream retval(*_retval, IO_WriteOnly);
			retval << actionMap;
		} else if (_replyType != Qnil) {
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

    InvokeDCOPSlot(VALUE obj, ID slotname, VALUE args, QByteArray& data, VALUE replyTypeName, VALUE replyType, QByteArray& returnValue) :
		_obj(obj), _slotname(slotname), _replyType(replyType), _cur(-1), _called(false)
	{
		_replyTypeName = StringValuePtr(replyTypeName);
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
		delete _stream;
	}
};

extern "C" {
extern void Init_qtruby();
extern void set_new_kde(VALUE (*new_kde) (int, VALUE *, VALUE));
extern void set_kconfigskeletonitem_immutable(VALUE (*kconfigskeletonitem_immutable) (VALUE));
extern void set_kde_resolve_classname(const char * (*kde_resolve_classname) (Smoke*, int, void *));
extern const char * kde_resolve_classname(Smoke* smoke, int classId, void * ptr);
extern VALUE new_qt(int argc, VALUE * argv, VALUE klass);
extern VALUE new_qobject(int argc, VALUE * argv, VALUE klass);
extern VALUE qt_module;
extern VALUE qt_internal_module;
extern VALUE qt_base_class;
extern VALUE kde_module;
extern VALUE kio_module;
extern VALUE kparts_module;
extern VALUE khtml_module;

VALUE
getdcopinfo(VALUE self, QString & signalname)
{
    VALUE member = rb_funcall(	kde_internal_module, 
								rb_intern("fullSignalName"), 
								2, self, rb_str_new2(signalname) );
	signalname.setLatin1(StringValuePtr(member));
    return rb_funcall(	qt_internal_module, 
						rb_intern("getMocArguments"), 
						1, member );
}

VALUE
k_dcop_signal(int argc, VALUE * argv, VALUE self)
{
	VALUE dcopObject = rb_funcall(kde_module, rb_intern("createDCOPObject"), 1, self);
	
    QString signalname(rb_id2name(rb_frame_last_func()));
    VALUE args = getdcopinfo(self, signalname);

    if(args == Qnil) return Qfalse;

    EmitDCOPSignal signal(dcopObject, signalname.latin1(), argc, argv, args);
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
dcop_process(VALUE /*self*/, VALUE target, VALUE slotname, VALUE args, VALUE data, VALUE replyTypeName, VALUE replyType, VALUE replyData)
{
	VALUE _data = rb_funcall(qt_internal_module, rb_intern("get_qbytearray"), 1, data);
	QByteArray * dataArray = 0;
	Data_Get_Struct(_data, QByteArray, dataArray);
	
	VALUE _replyData = rb_funcall(qt_internal_module, rb_intern("get_qbytearray"), 1, replyData);
	QByteArray * replyArray = 0;
	Data_Get_Struct(_replyData, QByteArray, replyArray);

	InvokeDCOPSlot dcopSlot(target, rb_intern(StringValuePtr(slotname)), args, *dataArray, replyTypeName, replyType, *replyArray);
	dcopSlot.next();
	
	return Qtrue;
}

static VALUE
dcop_call(int argc, VALUE * argv, VALUE /*self*/)
{
	QCString fun(StringValuePtr(argv[1]));
	VALUE args = argv[2];
	bool useEventLoop = (argv[argc-2] == Qtrue ? true : false);
	int timeout = NUM2INT(argv[argc-1]);
	
	DCOPCall dcopCall(argv[0], fun, argc-5, argv+3, args, useEventLoop, timeout);
	dcopCall.next();
	
	return *(dcopCall.var());
}

static VALUE
dcop_send(int argc, VALUE * argv, VALUE /*self*/)
{
	QCString fun(StringValuePtr(argv[1]));
	VALUE args = argv[2];
	VALUE result = Qnil;
	
	DCOPSend dcopSend(argv[0], fun, argc-3, argv+3, args, &result);
	dcopSend.next();
	
	return result;
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
			rb_define_method(klass, StringValuePtr(signal), (VALUE (*) (...)) k_dcop_signal, -1);
		}
	}
	
	if (	rb_funcall(kde_module, rb_intern("hasDCOPSlots"), 1, klass) == Qtrue
			|| rb_funcall(kde_module, rb_intern("hasDCOPSignals"), 1, klass) == Qtrue ) 
	{
		VALUE dcop_object = rb_funcall(kde_module, rb_intern("createDCOPObject"), 1, instance);
		if (dcop_object != Qnil) {
			rb_define_method(klass, "interfaces", (VALUE (*) (...)) dcop_interfaces, 0);
			rb_define_method(klass, "functions", (VALUE (*) (...)) dcop_functions, 0);
			rb_define_method(klass, "connectDCOPSignal", (VALUE (*) (...)) dcop_connect_signal, 5);
			rb_define_method(klass, "disconnectDCOPSignal", (VALUE (*) (...)) dcop_disconnect_signal, 4);
		}
	}
	
	return instance;
}

#if KDE_VERSION >= 0x030200

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

#endif

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

void
Init_korundum()
{
	set_new_kde(new_kde);
#if KDE_VERSION >= 0x030200
	set_kconfigskeletonitem_immutable(kconfigskeletonitem_immutable);
#endif
	set_kde_resolve_classname(kde_resolve_classname);
		
	// The Qt extension is linked against libsmokeqt.so, but Korundum links against
	// libsmokekde.so only. Specifying both a 'require Qt' and a 'require Korundum',
	// would give a link error.
	// So call the Init_qtruby() initialization function explicitely, not via 'require Qt'
	// (Qt.o is linked into libqtruby.so, as well as the Qt.so extension).
	Init_qtruby();
    install_handlers(KDE_handlers);
	
    kde_internal_module = rb_define_module_under(kde_module, "Internal");
	rb_define_singleton_method(kde_module, "dcop_process", (VALUE (*) (...)) dcop_process, 7);
	rb_define_singleton_method(kde_module, "dcop_call", (VALUE (*) (...)) dcop_call, -1);
	rb_define_singleton_method(kde_module, "dcop_send", (VALUE (*) (...)) dcop_send, -1);
	
#if KDE_VERSION >= 0x030200

	rb_define_method(kconfigskeleton_class, "addItem", (VALUE (*) (...)) config_additem, -1);
	
	rb_define_method(kconfigskeleton_itemenum_choice_class, "name", (VALUE (*) (...)) choice_name, 0);
	rb_define_method(kconfigskeleton_itemenum_choice_class, "label", (VALUE (*) (...)) choice_label, 0);
	rb_define_method(kconfigskeleton_itemenum_choice_class, "whatsThis", (VALUE (*) (...)) choice_whatsthis, 0);
	rb_define_method(kconfigskeleton_itemenum_choice_class, "name=", (VALUE (*) (...)) set_choice_name, 1);
	rb_define_method(kconfigskeleton_itemenum_choice_class, "label=", (VALUE (*) (...)) set_choice_label, 1);
	rb_define_method(kconfigskeleton_itemenum_choice_class, "whatsThis=", (VALUE (*) (...)) set_choice_whatsthis, 1);

#endif
	
	rb_define_method(kio_udsatom_class, "m_str", (VALUE (*) (...)) udsatom_str, 0);
	rb_define_method(kio_udsatom_class, "m_long", (VALUE (*) (...)) udsatom_long, 0);
	rb_define_method(kio_udsatom_class, "m_uds", (VALUE (*) (...)) udsatom_uds, 0);
	rb_define_method(kio_udsatom_class, "m_str=", (VALUE (*) (...)) set_udsatom_str, 1);
	rb_define_method(kio_udsatom_class, "m_long=", (VALUE (*) (...)) set_udsatom_long, 1);
	rb_define_method(kio_udsatom_class, "m_uds=", (VALUE (*) (...)) set_udsatom_uds, 1);
	
	rb_require("KDE/korundum.rb");
}

};
