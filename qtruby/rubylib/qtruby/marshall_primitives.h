/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

template <>
static bool ruby_to_primitive<bool>(VALUE v)
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
static VALUE primitive_to_ruby<bool>(bool sv)
{
	return sv ? Qtrue : Qfalse;
}

template <>
static signed char ruby_to_primitive<signed char>(VALUE v)
{
	return NUM2CHR(v);
}

template <>
static VALUE primitive_to_ruby<signed char>(signed char sv)
{
	return CHR2FIX(sv);
}

template <>
static unsigned char ruby_to_primitive<unsigned char>(VALUE v)
{
	return NUM2CHR(v);
}

template <>
static VALUE primitive_to_ruby<unsigned char>(unsigned char sv)
{
	return CHR2FIX(sv);
}

template <>
static short ruby_to_primitive<short>(VALUE v)
{
	return (short)NUM2INT(v);
}

template <>
static VALUE primitive_to_ruby<short>(short sv)
{
	return INT2NUM(sv);
}

template <>
static unsigned short ruby_to_primitive<unsigned short>(VALUE v)
{
	return (unsigned short)NUM2UINT(v);
}

template <>
static VALUE primitive_to_ruby<unsigned short>(unsigned short sv)
{
	return UINT2NUM((unsigned int) sv);
}

template <>
static int ruby_to_primitive<int>(VALUE v)
{
	if (TYPE(v) == T_OBJECT) {
		return (int)NUM2INT(rb_funcall(qt_internal_module, rb_intern("get_qinteger"), 1, v));
	} else {
		return (int)NUM2INT(v);
	}
}

template <>
static VALUE primitive_to_ruby<int>(int sv)
{
	return INT2NUM(sv);
}

template <>
static unsigned int ruby_to_primitive<unsigned int>(VALUE v)
{
	if (TYPE(v) == T_OBJECT) {
		return (unsigned int) NUM2UINT(rb_funcall(qt_internal_module, rb_intern("get_qinteger"), 1, v));
	} else {
		return (unsigned int) NUM2UINT(v);
	}
}

template <>
static VALUE primitive_to_ruby<unsigned int>(unsigned int sv)
{
	return UINT2NUM(sv);
}

template <>
static long ruby_to_primitive<long>(VALUE v)
{
	if (TYPE(v) == T_OBJECT) {
		return (long) NUM2LONG(rb_funcall(qt_internal_module, rb_intern("get_qinteger"), 1, v));
	} else {
		return (long) NUM2LONG(v);
	}
}

template <>
static VALUE primitive_to_ruby<long>(long sv)
{
	return INT2NUM(sv);
}

template <>
static unsigned long ruby_to_primitive<unsigned long>(VALUE v)
{
	if (TYPE(v) == T_OBJECT) {
		return (unsigned long) NUM2ULONG(rb_funcall(qt_internal_module, rb_intern("get_qinteger"), 1, v));
	} else {
		return (unsigned long) NUM2ULONG(v);
	}
}

template <>
static VALUE primitive_to_ruby<unsigned long>(unsigned long sv)
{
	return INT2NUM(sv);
}

template <>
static long long ruby_to_primitive<long long>(VALUE v)
{
	return NUM2LL(v);
}

template <>
static VALUE primitive_to_ruby<long long>(long long sv)
{
	return LL2NUM(sv);
}

template <>
static unsigned long long ruby_to_primitive<unsigned long long>(VALUE v)
{
	return rb_num2ull(v);
}

template <>
static VALUE primitive_to_ruby<unsigned long long>(unsigned long long sv)
{
	return rb_ull2inum(sv);
}

template <>
static float ruby_to_primitive<float>(VALUE v)
{
	return (float) NUM2DBL(v);
}

template <>
static VALUE primitive_to_ruby<float>(float sv)
{
	return rb_float_new((double) sv);
}

template <>
static double ruby_to_primitive<double>(VALUE v)
{
	return (double) NUM2DBL(v);
}

template <>
static VALUE primitive_to_ruby<double>(double sv)
{
	return rb_float_new((double) sv);
}

template <>
static char* ruby_to_primitive<char *>(VALUE v)
{
	if(v == Qnil)
		return 0;
	
	return StringValuePtr(v);
}

template <>
static unsigned char* ruby_to_primitive<unsigned char *>(VALUE v)
{
	if(v == Qnil)
		return 0;
	
	return (unsigned char*)StringValuePtr(v);
}

template <>
static VALUE primitive_to_ruby<int*>(int* sv)
{
	if(!sv) {
		return Qnil;
	}
	
	return primitive_to_ruby<int>(*sv);
}

#if defined(Q_OS_WIN32)
template <>
static struct _PROCESS_INFORMATION* ruby_to_primitive<struct _PROCESS_INFORMATION*>(VALUE v)
{
	if(v == Qnil)
		return 0;
	
	return NUM2INT(v);
}

template <>
static VALUE primitive_to_ruby<struct _PROCESS_INFORMATION*>(struct _PROCESS_INFORMATION* sv)
{
	return INT2NUM(sv);
}
#endif