
template <class T>
T marshall_from_ruby_to_smoke(VALUE);

template <class T>
VALUE marshall_from_smoke_to_ruby(T sv);

template <class T>
T marshall_from_ruby_to_smoke(Marshall *m) 
{
	VALUE obj = *(m->var());
	return marshall_from_ruby_to_smoke<T>(obj);
}

template <class T>
VALUE marshall_from_smoke_to_ruby(Marshall *m)
{
	return marshall_from_smoke_to_ruby<T>( (T)(m->item().s_voidp ) ); 
}

template <>
static bool marshall_from_ruby_to_smoke<bool>(VALUE v)
{
	if (TYPE(v) == T_OBJECT) {
		// A Qt::Boolean has been passed as a value
		VALUE temp = rb_funcall(qt_internal_module, rb_intern("get_qboolean"), 1, v);
		return (temp == Qtrue ? true : false);
	} else {
		return (v == Qtrue ? true : false);
	}
}

template <>
static VALUE marshall_from_smoke_to_ruby<bool>(bool sv)
{
	return sv ? Qtrue : Qfalse;
}

template <>
static char marshall_from_ruby_to_smoke<char>(VALUE v)
{
	return NUM2CHR(v);
}

template <>
static VALUE marshall_from_smoke_to_ruby<char>(char sv)
{
	return CHR2FIX(sv);
}

template <>
static unsigned char marshall_from_ruby_to_smoke<unsigned char>(VALUE v)
{
	return NUM2CHR(v);
}

template <>
static VALUE marshall_from_smoke_to_ruby<unsigned char>(unsigned char sv)
{
	return CHR2FIX(sv);
}

template <>
static short marshall_from_ruby_to_smoke<short>(VALUE v)
{
	return (short)NUM2INT(v);
}

template <>
static VALUE marshall_from_smoke_to_ruby<short>(short sv)
{
	return INT2NUM(sv);
}

template <>
static unsigned short marshall_from_ruby_to_smoke<unsigned short>(VALUE v)
{
	return (unsigned short)NUM2INT(v);
}

template <>
static VALUE marshall_from_smoke_to_ruby<unsigned short>(unsigned short sv)
{
	return INT2NUM(sv);
}

template <>
static int marshall_from_ruby_to_smoke<int>(VALUE v)
{
	if (TYPE(v) == T_OBJECT) {
		return (int)NUM2INT(rb_funcall(qt_internal_module, rb_intern("get_qinteger"), 1, v));
	} else {
		return (int)NUM2INT(v);
	}
}

template <>
static VALUE marshall_from_smoke_to_ruby<int>(int sv)
{
	return INT2NUM(sv);
}

template <>
static unsigned int marshall_from_ruby_to_smoke<unsigned int>(VALUE v)
{
	if (TYPE(v) == T_OBJECT) {
		return (unsigned int) NUM2UINT(rb_funcall(qt_internal_module, rb_intern("get_qinteger"), 1, v));
	} else {
		return (unsigned int) NUM2UINT(v);
	}
}

template <>
static VALUE marshall_from_smoke_to_ruby<unsigned int>(unsigned int sv)
{
	return UINT2NUM(sv);
}

template <>
static long marshall_from_ruby_to_smoke<long>(VALUE v)
{
	if (TYPE(v) == T_OBJECT) {
		return (long) NUM2LONG(rb_funcall(qt_internal_module, rb_intern("get_qinteger"), 1, v));
	} else {
		return (long) NUM2LONG(v);
	}
}

template <>
static VALUE marshall_from_smoke_to_ruby<long>(long sv)
{
	return INT2NUM(sv);
}

template <>
static unsigned long marshall_from_ruby_to_smoke<unsigned long>(VALUE v)
{
	if (TYPE(v) == T_OBJECT) {
		return (unsigned long) NUM2ULONG(rb_funcall(qt_internal_module, rb_intern("get_qinteger"), 1, v));
	} else {
		return (unsigned long) NUM2ULONG(v);
	}
}

template <>
static VALUE marshall_from_smoke_to_ruby<unsigned long>(unsigned long sv)
{
	return INT2NUM(sv);
}

template <>
static float marshall_from_ruby_to_smoke<float>(VALUE v)
{
	return (float) NUM2DBL(v);
}

template <>
static VALUE marshall_from_smoke_to_ruby<float>(float sv)
{
	return rb_float_new((double) sv);
}

template <>
static double marshall_from_ruby_to_smoke<double>(VALUE v)
{
	return (double) NUM2DBL(v);
}

template <>
static VALUE marshall_from_smoke_to_ruby<double>(double sv)
{
	return rb_float_new((double) sv);
}

template <>
static SmokeEnumWrapper marshall_from_ruby_to_smoke<SmokeEnumWrapper>(Marshall *m)
{
	VALUE v = *(m->var());
	SmokeEnumWrapper e;
	e.m = m;
	if (TYPE(v) == T_OBJECT) {
		// A Qt::Enum is a subclass of Qt::Integer, so 'get_qinteger()' can be called ok
		VALUE temp = rb_funcall(qt_internal_module, rb_intern("get_qinteger"), 1, v);
		e.m->item().s_enum = (long) NUM2LONG(temp);
	} else {
		e.m->item().s_enum = (long) NUM2LONG(v);
	}

	return e;
}

template <>
static VALUE marshall_from_smoke_to_ruby<SmokeEnumWrapper>(SmokeEnumWrapper sv)
{
	long val = sv.m->item().s_enum;
	return rb_funcall(qt_internal_module, rb_intern("create_qenum"), 
		2, INT2NUM(val), rb_str_new2(sv.m->type().name()) );
}

template <>
static SmokeClassWrapper marshall_from_ruby_to_smoke<SmokeClassWrapper>(Marshall *m)
{
	VALUE v = *(m->var());
	SmokeClassWrapper c;
	c.m = m;

	if(v == Qnil) {
		c.m->item().s_class = 0;
		return c;
	}
				
	if(TYPE(v) != T_DATA) {
		rb_raise(rb_eArgError, "Invalid type, expecting %s\n", c.m->type().name());
		return c;
	}
	rb_warning("References can't be nil2\n");		
	smokeruby_object *o = value_obj_info(v);
	if(!o || !o->ptr) {
		if(c.m->type().isRef()) {
			rb_warning("References can't be nil\n");
			c.m->unsupported();
		}
					
		c.m->item().s_class = 0;
		return c;
	}
		
	void *ptr = o->ptr;
	if(!c.m->cleanup() && c.m->type().isStack()) {
		ptr = construct_copy(o);
	}
		
	const Smoke::Class &cl = c.m->smoke()->classes[c.m->type().classId()];
	
	ptr = o->smoke->cast(
		ptr,				// pointer
		o->classId,				// from
		o->smoke->idClass(cl.className)	// to
		);
				
	c.m->item().s_class = ptr;
	return c;
}

template <>
static VALUE marshall_from_smoke_to_ruby<SmokeClassWrapper>(SmokeClassWrapper sv)
{
	if(sv.m->item().s_voidp == 0) {
		return Qnil;
	}

	void *p = sv.m->item().s_voidp;
	VALUE obj = getPointerObject(p);
	if(obj != Qnil) {
		return obj;
	}

	smokeruby_object  * o = (smokeruby_object *) malloc(sizeof(smokeruby_object));
	o->smoke = sv.m->smoke();
	o->classId = sv.m->type().classId();
	o->ptr = p;
	o->allocated = false;

	const char * classname = resolve_classname(o->smoke, o->classId, o->ptr);
		
	if(sv.m->type().isConst() && sv.m->type().isRef()) {
		p = construct_copy( o );
		if(p) {
			o->ptr = p;
			o->allocated = true;
		}
	}
		
	obj = set_obj_info(classname, o);
	if (do_debug & qtdb_calls) {
		printf("allocating %s %p -> %p\n", classname, o->ptr, (void*)obj);
	}

	if(sv.m->type().isStack()) {
		o->allocated = true;
		// Keep a mapping of the pointer so that it is only wrapped once as a ruby VALUE
		mapPointer(obj, o, o->classId, 0);
	}
			
	return obj;
}


template <>
static VALUE marshall_from_smoke_to_ruby<char *>(Marshall *m)
{
	char *sv = (char*)m->item().s_voidp;
	VALUE obj;
	if(sv)
		obj = rb_str_new2(sv);
	else
		obj = Qnil;

	if(m->cleanup())
		delete[] sv;

	return obj;
}

template <>
static char* marshall_from_ruby_to_smoke<char *>(VALUE v)
{
	if(v == Qnil)
		return 0;
	
	return StringValuePtr(v);
}

template <>
static VALUE marshall_from_smoke_to_ruby<unsigned char *>(Marshall *m)
{
	VALUE obj = Qnil;
	m->unsupported();
	return obj;
}

template <>
static unsigned char* marshall_from_ruby_to_smoke<unsigned char *>(VALUE v)
{
	if(v == Qnil)
		return 0;
	
	return (unsigned char*)StringValuePtr(v);
}
