
template <class T> T* smoke_ptr(Marshall *m) { return (T*) m->item().s_voidp; }
//template <class T> T smoke_ptr(Marshall *m) { return (T) m->item().s_voidp; }

template<> bool* smoke_ptr<bool>(Marshall *m) { return &m->item().s_bool; }
template<> signed char* smoke_ptr<signed char>(Marshall *m) { return &m->item().s_char; }
template<> unsigned char* smoke_ptr<unsigned char>(Marshall *m) { return &m->item().s_uchar; }
template<> short* smoke_ptr<short>(Marshall *m) { return &m->item().s_short; }
template<> unsigned short* smoke_ptr<unsigned short>(Marshall *m) { return &m->item().s_ushort; }
template<> int* smoke_ptr<int>(Marshall *m) { return &m->item().s_int; }
template<> unsigned int* smoke_ptr<unsigned int>(Marshall *m) { return &m->item().s_uint; }
template<> long* smoke_ptr<long>(Marshall *m) { 	return &m->item().s_long; }
template<> unsigned long* smoke_ptr<unsigned long>(Marshall *m) { return &m->item().s_ulong; }
template<> float* smoke_ptr<float>(Marshall *m) { return &m->item().s_float; }
template<> double* smoke_ptr<double>(Marshall *m) { return &m->item().s_double; }
template<> void* smoke_ptr<void>(Marshall *m) { return m->item().s_voidp; }

template <class T> T ruby_to_primitive(VALUE);
template <class T> VALUE primitive_to_ruby(T);

template <class T> 
static void marshall_from_ruby(Marshall *m) 
{
	rb_warning("marshall from");
	VALUE obj = *(m->var());
	(*smoke_ptr<T>(m)) = ruby_to_primitive<T>(obj);
}

template <class T>
static void marshall_to_ruby(Marshall *m)
{
	rb_warning("marshall to");
	*(m->var()) = primitive_to_ruby<T>( *smoke_ptr<T>(m) ); 
}

#include "marshall_primitives.cpp"
#include "marshall_complex.cpp"

// Special case marshallers

template <> 
static void marshall_from_ruby<char *>(Marshall *m) 
{
	rb_warning("marshall from");
	VALUE obj = *(m->var());
	m->item().s_voidp = ruby_to_primitive<char*>(obj);
}

template <>
static void marshall_from_ruby<SmokeEnumWrapper>(Marshall *m)
{
	rb_warning("marshall from enum");
	VALUE v = *(m->var());

	if (TYPE(v) == T_OBJECT) {
		// A Qt::Enum is a subclass of Qt::Integer, so 'get_qinteger()' can be called ok
		VALUE temp = rb_funcall(qt_internal_module, rb_intern("get_qinteger"), 1, v);
		m->item().s_enum = (long) NUM2LONG(temp);
	} else {
		m->item().s_enum = (long) NUM2LONG(v);
	}

}

template <>
static void marshall_to_ruby<SmokeEnumWrapper>(Marshall *m)
{
	rb_warning("marshall to enum");
	long val = m->item().s_enum;
	*(m->var()) = rb_funcall(qt_internal_module, rb_intern("create_qenum"), 
		2, INT2NUM(val), rb_str_new2( m->type().name()) );
}

template <>
static void marshall_from_ruby<SmokeClassWrapper>(Marshall *m)
{
	rb_warning("marshall from class");
	VALUE v = *(m->var());

	if(v == Qnil) {
		m->item().s_class = 0;
		return;
	}
				
	if(TYPE(v) != T_DATA) {
		rb_raise(rb_eArgError, "Invalid type, expecting %s\n", m->type().name());
		return;
	}

	smokeruby_object *o = value_obj_info(v);
	if(!o || !o->ptr) {
		if(m->type().isRef()) {
			rb_warning("References can't be nil\n");
			m->unsupported();
		}
					
		m->item().s_class = 0;
		return;
	}
		
	void *ptr = o->ptr;
	if(!m->cleanup() && m->type().isStack()) {
		ptr = construct_copy(o);
	}
		
	const Smoke::Class &cl = m->smoke()->classes[m->type().classId()];
	
	ptr = o->smoke->cast(
		ptr,				// pointer
		o->classId,				// from
		o->smoke->idClass(cl.className)	// to
		);
				
	m->item().s_class = ptr;
	return;
}

template <>
static void marshall_to_ruby<SmokeClassWrapper>(Marshall *m)
{
	rb_warning("marshall to class");
	if(m->item().s_voidp == 0) {
		*(m->var()) = Qnil;
		return;
	}
	void *p = m->item().s_voidp;
	VALUE obj = getPointerObject(p);
	if(obj != Qnil) {
		*(m->var()) = obj;
		return ;
	}
	smokeruby_object  * o = (smokeruby_object *) malloc(sizeof(smokeruby_object));
	o->smoke = m->smoke();
	o->classId = m->type().classId();
	o->ptr = p;
	o->allocated = false;

	const char * classname = resolve_classname(o->smoke, o->classId, o->ptr);
	if(m->type().isConst() && m->type().isRef()) {
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

	if(m->type().isStack()) {
		o->allocated = true;
		// Keep a mapping of the pointer so that it is only wrapped once as a ruby VALUE
		mapPointer(obj, o, o->classId, 0);
	}
			
	*(m->var()) = obj;
}

template <>
static void marshall_to_ruby<char *>(Marshall *m)
{
	char *sv = (char*)m->item().s_voidp;
	VALUE obj;
	if(sv)
		obj = rb_str_new2(sv);
	else
		obj = Qnil;

	if(m->cleanup())
		delete[] sv;

	*(m->var()) = obj;
}

template <>
static void marshall_to_ruby<unsigned char *>(Marshall *m)
{
	m->unsupported();
}
