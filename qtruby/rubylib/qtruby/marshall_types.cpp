class MethodReturnValueBase : public Marshall 
{
public:
	MethodReturnValueBase(Smoke *smoke, Smoke::Index meth, Smoke::Stack stack) :
		_smoke(smoke), _method(meth), _stack(stack) { 

		_st.set(_smoke, method().ret);

	}

	const Smoke::Method &method() { return _smoke->methods[_method]; }
	Smoke::StackItem &item() { return _stack[0]; }
	Smoke *smoke() { return _smoke; }
	SmokeType type() { return _st; }

	void next() {}
	bool cleanup() { return false; }

	void unsupported() {
		rb_raise(rb_eArgError, "Cannot handle '%s' as return-type of %s::%s",
		type().name(),
		classname(),
		_smoke->methodNames[method().name]);	
	}
    VALUE * var() { return _retval; }
protected:
	Smoke *_smoke;
	Smoke::Index _method;
	Smoke::Stack _stack;
	SmokeType _st;
	VALUE *_retval;
	virtual const char *classname() { return _smoke->className(method().classId); }
};

class VirtualMethodReturnValue : public MethodReturnValueBase {
public:
	VirtualMethodReturnValue(Smoke *smoke, Smoke::Index meth, Smoke::Stack stack, VALUE retval) :
		MethodReturnValueBase(smoke,meth,stack), _retval2(retval) {
		_retval = &_retval2;
		Marshall::HandlerFn fn = getMarshallFn(type());
		(*fn)(this);
	}

	Marshall::Action action() { return Marshall::FromVALUE; }

private:
	VALUE _retval2;
};

class MethodReturnValue : public MethodReturnValueBase {
public:
	MethodReturnValue(Smoke *smoke, Smoke::Index meth, Smoke::Stack stack, VALUE * retval) :
		MethodReturnValueBase(smoke,meth,stack) {
	 	_retval = retval;
		Marshall::HandlerFn fn = getMarshallFn(type());
		(*fn)(this);
	}

    Marshall::Action action() { return Marshall::ToVALUE; }

private:
	const char *classname() { 
		return strcmp(MethodReturnValueBase::classname(), "QGlobalSpace") == 0 ? "" : MethodReturnValueBase::classname(); 
	}
};



class MethodCallBase : public Marshall
{
public:
	MethodCallBase(Smoke *smoke, Smoke::Index meth) :
		_smoke(smoke), _method(meth), _cur(-1), _called(false), _sp(0)  {  }
	MethodCallBase(Smoke *smoke, Smoke::Index meth, Smoke::Stack stack) :
		_smoke(smoke), _method(meth), _stack(stack), _cur(-1), _called(false), _sp(0) {  }

	Smoke *smoke() { return _smoke; }
	SmokeType type() { return SmokeType(_smoke, _args[_cur]); }
	Smoke::StackItem &item() { return _stack[_cur + 1]; }
	const Smoke::Method &method() { return _smoke->methods[_method]; }

	virtual int items() = 0;
	virtual void callMethod() = 0;
	
	void next() {
		int oldcur = _cur;
		_cur++;
		while(!_called && _cur < items() ) {
			Marshall::HandlerFn fn = getMarshallFn(type());
			(*fn)(this);
			_cur++;
		}
	
		callMethod();
		_cur = oldcur;
	}

	void unsupported() {
		rb_raise(rb_eArgError, "Cannot handle '%s' as argument of %s::%s",
			type().name(),
			classname(),
			_smoke->methodNames[method().name]);
    }

protected:
	Smoke *_smoke;
	Smoke::Index _method;
	Smoke::Stack _stack;
	int _cur;
	Smoke::Index *_args;
	bool _called;
	VALUE *_sp;
	virtual const char* classname() { return _smoke->className(method().classId); }
};

class VirtualMethodCall : public MethodCallBase {
public:
	VirtualMethodCall(Smoke *smoke, Smoke::Index meth, Smoke::Stack stack, VALUE obj) :
		MethodCallBase(smoke,meth,stack), _obj(obj) {
		
		_sp = (VALUE *) calloc(method().numArgs, sizeof(VALUE));
		_args = _smoke->argumentList + method().args;
	}

	~VirtualMethodCall() {
		free(_sp);
	}

	Marshall::Action action() { return Marshall::ToVALUE; }
	VALUE * var() { return _sp + _cur; }
	
	int items() { return method().numArgs; }

	void callMethod() {
		if(_called) return;
		_called = true;
	
		VALUE _retval = rb_funcall2(_obj, rb_intern(_smoke->methodNames[method().name]),
			method().numArgs,	_sp );
	
		VirtualMethodReturnValue r(_smoke, _method, _stack, _retval);
	}

	bool cleanup() { return false; }   // is this right?
 
private:
	VALUE _obj;
};

class MethodCall : public MethodCallBase {
public:
	MethodCall(Smoke *smoke, Smoke::Index method, VALUE target, VALUE *sp, int items) :
		MethodCallBase(smoke,method), _target(target), _current_object(0), _sp(sp), _items(items)
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

	Marshall::Action action() { return Marshall::FromVALUE; }


	VALUE * var() {
		if(_cur < 0) return &_retval;
		return _sp + _cur;
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

	int items() { return _items; }

	bool cleanup() { return true; }
private:
	VALUE _target;
	void *_current_object;
	Smoke::Index _current_object_class;
	VALUE *_sp;
	int _items;
	VALUE _retval;
	const char *classname() { 
		return strcmp(MethodCallBase::classname(), "QGlobalSpace") == 0 ? "" : MethodCallBase::classname(); 
	}
};

class SigSlotBase : public Marshall {
public:
	SigSlotBase(VALUE args) : _cur(-1), _called(false) { 
		_items = NUM2INT(rb_ary_entry(args, 0));
		Data_Get_Struct(rb_ary_entry(args, 1), MocArgument, _args);
		_stack = new Smoke::StackItem[_items];
	}

	~SigSlotBase() { delete[] _stack; }

	const MocArgument &arg() { return _args[_cur]; }
	SmokeType type() { return arg().st; }
	Smoke::StackItem &item() { return _stack[_cur]; }
	VALUE * var() { return _sp + _cur; }
	Smoke *smoke() { return type().smoke(); }
	virtual const char *mytype() = 0;
	virtual void mainfunction() = 0;

	void unsupported() 
	{
		rb_raise(rb_eArgError, "Cannot handle '%s' as %s argument\n", type().name(), mytype() );
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

		mainfunction();
		_cur = oldcur;
    }

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
 public:
    EmitSignal(QObject *obj, int id, int items, VALUE args, VALUE *sp) : SigSlotBase(args),
    	_obj(obj), _id(id) { 
		_sp = sp;
	}

    Marshall::Action action() { return Marshall::FromVALUE; }
    Smoke::StackItem &item() { return _stack[_cur]; }
	const char *mytype() { return "signal"; }

	void emitSignal() 
	{
		if (_called) return;
		_called = true;
		void ** o = new void*[_items + 1];

		for (int i = 0; i < _items; i++) {
			Smoke::StackItem *si = _stack + i;
			switch(_args[i].argType) {
			case xmoc_bool:
				o[i + 1] = &si->s_bool;
				break;
			case xmoc_int:
				o[i + 1] = &si->s_int;
				break;
			case xmoc_double:
				o[i + 1] = &si->s_double;
				break;
			case xmoc_charstar:
				o[i + 1] = &si->s_voidp;
				break;
			case xmoc_QString:
				o[i + 1] = si->s_voidp;
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
					if (!fn) {
						rb_warning("Unknown enumeration %s\n", t.name());
						p = new int((int)si->s_enum);
						break;
					}
					Smoke::Index id = t.typeId();
					(*fn)(Smoke::EnumNew, id, p, si->s_enum);
					(*fn)(Smoke::EnumFromLong, id, p, si->s_enum);
					// FIXME: MEMORY LEAK
					break;
				}
				case Smoke::t_class:
				case Smoke::t_voidp:
					p = si->s_voidp;
					break;
				default:
					p = 0;
					break;
				}
				o[i + 1] = p;
			}
			}
		}

		_obj->metaObject()->activate(_obj, _id, o);
		delete[] o;
    }

	void mainfunction() { emitSignal(); }

	bool cleanup() { return true; }
};

class InvokeSlot : public SigSlotBase {
    VALUE _obj;
    ID _slotname;
    void **_o;
public:
    InvokeSlot(VALUE obj, ID slotname, VALUE args, void ** o) : SigSlotBase(args),
    _obj(obj), _slotname(slotname), _o(o)
    {
		_sp = (VALUE *) calloc(_items, sizeof(VALUE));
		copyArguments();
    }

	~InvokeSlot() { free(_sp);	}

    Marshall::Action action() { return Marshall::ToVALUE; }
	const char *mytype() { return "slot"; }

    bool cleanup() { return false; }



	void copyArguments() 
	{
		for (int i = 0; i < _items; i++) {
			void *o = _o[i + 1];
			switch(_args[i].argType) {
			case xmoc_bool:
			_stack[i].s_bool = *(bool*)o;
			break;
			case xmoc_int:
			_stack[i].s_int = *(int*)o;
			break;
			case xmoc_double:
			_stack[i].s_double = *(double*)o;
			break;
			case xmoc_charstar:
			_stack[i].s_voidp = o;
			break;
			case xmoc_QString:
			_stack[i].s_voidp = o;
			break;
			default:	// case xmoc_ptr:
			{
				const SmokeType &t = _args[i].st;
				void *p = o;
				switch(t.elem()) {
				case Smoke::t_bool:
				_stack[i].s_bool = **(bool**)o;
				break;
				case Smoke::t_char:
				_stack[i].s_char = **(char**)o;
				break;
				case Smoke::t_uchar:
				_stack[i].s_uchar = **(unsigned char**)o;
				break;
				case Smoke::t_short:
				_stack[i].s_short = **(short**)p;
				break;
				case Smoke::t_ushort:
				_stack[i].s_ushort = **(unsigned short**)p;
				break;
				case Smoke::t_int:
				_stack[i].s_int = **(int**)p;
				break;
				case Smoke::t_uint:
				_stack[i].s_uint = **(unsigned int**)p;
				break;
				case Smoke::t_long:
				_stack[i].s_long = **(long**)p;
				break;
				case Smoke::t_ulong:
				_stack[i].s_ulong = **(unsigned long**)p;
				break;
				case Smoke::t_float:
				_stack[i].s_float = **(float**)p;
				break;
				case Smoke::t_double:
				_stack[i].s_double = **(double**)p;
				break;
				case Smoke::t_enum:
				{
					Smoke::EnumFn fn = SmokeClass(t).enumFn();
					if(!fn) {
						rb_warning("Unknown enumeration %s\n", t.name());
						_stack[i].s_enum = **(int**)p;
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
	void invokeSlot() 
	{
		if (_called) return;
		_called = true;
		(void) rb_funcall2(_obj, _slotname, _items, _sp);
	}

	void mainfunction() { invokeSlot(); }

};
