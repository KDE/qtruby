#include <qstring.h>
#include <qregexp.h>
#include <qapplication.h>
#include <qmetaobject.h>
#include <qvaluelist.h>
#include <private/qucomextra_p.h>

#include "smoke.h"

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
#include <ruby.h>

#include "marshall.h"
#include "qtruby.h"
#include "smokeruby.h"

#ifndef HINT_BYTES
#define HINT_BYTES HINT_BYTE
#endif

extern "C" {
extern VALUE set_obj_info(const char * className, smokeruby_object * o);
};

void
smokeruby_mark(void * /*p*/)
{
}

void
smokeruby_free(void * /*p*/)
{
//    smokeperl_object *o = (smokeperl_object*)mg->mg_ptr;

//    const char *className = o->smoke->classes[o->classId].className;
//    if(o->allocated && o->ptr) {
//        if(do_debug & qtdb_gc) printf("Deleting (%s*)%p\n", className, o->ptr);
//        SmokeClass sc(o->smoke, o->classId);
//        if(sc.hasVirtual())
//            unmapPointer(o, o->classId, 0);
//        object_count --;
//        char *methodName = new char[strlen(className) + 2];
//        methodName[0] = '~';
//        strcpy(methodName + 1, className);
//        Smoke::Index nameId = o->smoke->idMethodName(methodName);
//        Smoke::Index meth = o->smoke->findMethod(o->classId, nameId);
//        if(meth > 0) {
//            Smoke::Method &m = o->smoke->methods[o->smoke->methodMaps[meth].method];
//            Smoke::ClassFn fn = o->smoke->classes[m.classId].classFn;
//            Smoke::StackItem i[1];
//            (*fn)(m.method, o->ptr, i);
//        }
//        delete[] methodName;
//    }
    return;
}

bool
matches_arg(Smoke *smoke, Smoke::Index meth, Smoke::Index argidx, const char *argtype)
{
    Smoke::Index *arg = smoke->argumentList + smoke->methods[meth].args + argidx;
    SmokeType type = SmokeType(smoke, *arg);
    if(type.name() && !strcmp(type.name(), argtype))
	return true;
    return false;
}

void *
construct_copy(smokeruby_object *o)
{
    const char *className = o->smoke->className(o->classId);
    int classNameLen = strlen(className);
    char *ccSig = new char[classNameLen + 2];       // copy constructor signature
    strcpy(ccSig, className);
    strcat(ccSig, "#");
    Smoke::Index ccId = o->smoke->idMethodName(ccSig);
    delete[] ccSig;

    char *ccArg = new char[classNameLen + 8];
    sprintf(ccArg, "const %s&", className);

    Smoke::Index ccMeth = o->smoke->findMethod(o->classId, ccId);
    if(!ccMeth)
	return 0;
    if(ccMeth > 0) {
	Smoke::Index method = o->smoke->methodMaps[ccMeth].method;
	// Make sure it's a copy constructor
	if(!matches_arg(o->smoke, method, 0, ccArg)) {
            delete[] ccArg;
	    return 0;
        }
        ccMeth = method;
    } else {
        // ambiguous method, pick the copy constructor
	Smoke::Index i = -ccMeth;
	while(o->smoke->ambiguousMethodList[i]) {
	    if(matches_arg(o->smoke, o->smoke->ambiguousMethodList[i], 0, ccArg))
		break;
	}
        delete[] ccArg;
	ccMeth = o->smoke->ambiguousMethodList[i];
	if(!ccMeth)
	    return 0;
    }

    // Okay, ccMeth is the copy constructor. Time to call it.
    Smoke::StackItem args[2];
    args[0].s_voidp = 0;
    args[1].s_voidp = o->ptr;
    Smoke::ClassFn fn = o->smoke->classes[o->classId].classFn;
    (*fn)(o->smoke->methods[ccMeth].method, 0, args);
    return args[0].s_voidp;
}

static void
marshall_basetype(Marshall *m)
{
    switch(m->type().elem()) {
      case Smoke::t_bool:
	switch(m->action()) {
	  case Marshall::FromVALUE:
	    m->item().s_bool = *(m->var()) == Qtrue ? true : false;
	    break;
	  case Marshall::ToVALUE:
	    *(m->var()) = m->item().s_bool ? Qtrue : Qfalse;
	    break;
	  default:
	    m->unsupported();
	    break;
	}
	break;
      case Smoke::t_char:
	switch(m->action()) {
	  case Marshall::FromVALUE:
	    m->item().s_char = NUM2CHR(*(m->var()));
	    break;
	  case Marshall::ToVALUE:
	    *(m->var()) = CHR2FIX(m->item().s_char);
	    break;
	  default:
	    m->unsupported();
	    break;
	}
	break;
      case Smoke::t_uchar:
	switch(m->action()) {
	  case Marshall::FromVALUE:
	    m->item().s_uchar = NUM2CHR(*(m->var()));
	    break;
	  case Marshall::ToVALUE:
	    *(m->var()) = CHR2FIX(m->item().s_uchar);
	    break;
	  default:
	    m->unsupported();
	    break;
	}
	break;
      case Smoke::t_short:
	switch(m->action()) {
	  case Marshall::FromVALUE:
	    m->item().s_short = (short) NUM2INT(*(m->var()));
	    break;
	  case Marshall::ToVALUE:
	    *(m->var()) = INT2NUM(m->item().s_short);
	    break;
	  default:
	    m->unsupported();
	    break;
	}
	break;
      case Smoke::t_ushort:
	switch(m->action()) {
	  case Marshall::FromVALUE:
	    m->item().s_ushort = (unsigned short) NUM2INT(*(m->var()));
	    break;
	  case Marshall::ToVALUE:
	    *(m->var()) = INT2NUM(m->item().s_ushort);
	    break;
	  default:
	    m->unsupported();
	    break;
	}
	break;
      case Smoke::t_int:
	switch(m->action()) {
	  case Marshall::FromVALUE:
	    m->item().s_int = (int) NUM2INT(*(m->var()));
	    break;
	  case Marshall::ToVALUE:
	    *(m->var()) = INT2NUM(m->item().s_int);
	    break;
	  default:
	    m->unsupported();
	    break;
	}
	break;
      case Smoke::t_uint:
	switch(m->action()) {
	  case Marshall::FromVALUE:
	    m->item().s_uint = (unsigned int) NUM2UINT(*(m->var()));
	    break;
	  case Marshall::ToVALUE:
	    *(m->var()) = INT2NUM(m->item().s_uint);
	    break;
	  default:
	    m->unsupported();
	    break;
	}
	break;
      case Smoke::t_long:
	switch(m->action()) {
	  case Marshall::FromVALUE:
	    m->item().s_long = (long) NUM2LONG(*(m->var()));
	    break;
	  case Marshall::ToVALUE:
	    *(m->var()) = INT2NUM(m->item().s_long);
	    break;
	  default:
	    m->unsupported();
	    break;
	}
	break;
      case Smoke::t_ulong:
	switch(m->action()) {
	  case Marshall::FromVALUE:
	    m->item().s_ulong = (long) NUM2ULONG(*(m->var()));
	    break;
	  case Marshall::ToVALUE:
	    *(m->var()) = INT2NUM(m->item().s_ulong);
	    break;
	  default:
	    m->unsupported();
	    break;
	}
	break;
      case Smoke::t_float:
	switch(m->action()) {
	  case Marshall::FromVALUE:
	    m->item().s_float = (float) NUM2DBL(*(m->var()));
	    break;
	  case Marshall::ToVALUE:
	    *(m->var()) = rb_float_new((double) m->item().s_float);
	    break;
	  default:
	    m->unsupported();
	    break;
	}
	break;
      case Smoke::t_double:
	switch(m->action()) {
	  case Marshall::FromVALUE:
	    m->item().s_double = (double) NUM2DBL(*(m->var()));
	    break;
	  case Marshall::ToVALUE:
	    *(m->var()) = rb_float_new(m->item().s_double);
	    break;
	  default:
	    m->unsupported();
	    break;
	}
	break;
      case Smoke::t_enum:
	switch(m->action()) {
	  case Marshall::FromVALUE:
	    m->item().s_enum = (long) NUM2LONG(*(m->var()));
	    break;
	  case Marshall::ToVALUE:
	    *(m->var()) = INT2NUM(m->item().s_enum);
	    break;
	  default:
	    m->unsupported();
	    break;
	}
	break;
      case Smoke::t_class:
	switch(m->action()) {
	  case Marshall::FromVALUE:
	    {
		smokeruby_object *o = value_obj_info(*(m->var()));
		if(!o || !o->ptr) {
                    if(m->type().isRef()) {
                        rb_warning("References can't be null or undef\n");
                        m->unsupported();
                    }
		    m->item().s_class = 0;
		    break;
		}
		void *ptr = o->ptr;
		if(!m->cleanup() && m->type().isStack()) {
		    ptr = construct_copy(o);
		}
		const Smoke::Class &c = m->smoke()->classes[m->type().classId()];
		ptr = o->smoke->cast(
		    ptr,				// pointer
		    o->classId,				// from
		    o->smoke->idClass(c.className)	// to
		);
		m->item().s_class = ptr;
		break;
	    }
	    break;
	  case Marshall::ToVALUE:
	    {
		if(m->item().s_voidp == 0) {
			*(m->var()) = Qundef;
		    break;
		}

		void *p = m->item().s_voidp;
		VALUE obj = getPointerObject(p);
		if(obj != Qnil) {
			*(m->var()) = obj;
		    break;
		}
//		HV *hv = newHV();
//		obj = newRV_noinc((SV*)hv);
		// TODO: Generic mapping from C++ classname to Qt classname

		smokeruby_object  * o = ALLOC(smokeruby_object);
		o->smoke = m->smoke();
		o->classId = m->type().classId();
		o->ptr = p;
		o->allocated = false;

		if(m->type().isStack())
		    o->allocated = true;

//		sv_magic((SV*)hv, sv_qapp, '~', (char*)&o, sizeof(o));
//		MAGIC *mg = mg_find((SV*)hv, '~');
//		mg->mg_virtual = &vtbl_smoke;

		const char * classname = m->smoke()->binding->className(m->type().classId());
		obj = set_obj_info(classname, o);
//		sv_bless(obj, gv_stashpv(buf, TRUE));
//		delete[] buf;

		*(m->var()) = obj;
//		SvREFCNT_dec(obj);
	    }
	    break;
	  default:
	    m->unsupported();
	    break;
	}
	break;
      default:
	m->unsupported();
	break;
    }
}

static void marshall_void(Marshall * /*m*/) {}
static void marshall_unknown(Marshall *m) {
    m->unsupported();
}

static void marshall_charP(Marshall *m) {
    switch(m->action()) {
      case Marshall::FromVALUE:
	{
	    VALUE rv = *(m->var());
	    if(rv == Qundef) {
			m->item().s_voidp = 0;
			break;
	    }
	    if(m->cleanup()) {
			m->item().s_voidp = STR2CSTR(rv);
	    } else {
			m->item().s_voidp = STR2CSTR(rv);
	    }
	}
	break;
      case Marshall::ToVALUE:
	{
	    char *p = (char*)m->item().s_voidp;
	    if(p)
	    *(m->var()) = rb_str_new2(p);
	    else
	    *(m->var()) = Qundef;
	    if(m->cleanup())
		delete[] p;
	}
	break;
      default:
	m->unsupported();
	break;
    }
}

void marshall_ucharP(Marshall *m) {
    switch(m->action()) {
      case Marshall::FromVALUE:
	{
	    VALUE rv = *(m->var());
	    if(rv == Qundef) {
		m->item().s_voidp = 0;
		break;
	    }
	    if(m->cleanup()) {
			m->item().s_voidp = (void *) rb_str_new2("");
	    } else {
			m->item().s_voidp = STR2CSTR(rv);
	    }
	}
	break;
      case Marshall::ToVALUE:
	// This will need to be implemented with some sort of tied array
      default:
	m->unsupported();
	break;
    }
}

static void marshall_QString(Marshall *m) {
    switch(m->action()) {
      case Marshall::FromVALUE:
	{
	    QString *s = 0;
	    if(*(m->var()) != Qundef || m->type().isStack()) {
	    	s = new QString( STR2CSTR(*(m->var())) );
//               if(SvUTF8(*(m->var())))
//		    s = new QString(QString::fromUtf8(SvPV_nolen(*(m->var()))));
//                else if(PL_hints & HINT_LOCALE)
//                    s = new QString(QString::fromLocal8Bit(SvPV_nolen(*(m->var()))));
//                else
//                    s = new QString(QString::fromLatin1(SvPV_nolen(*(m->var()))));
//            }
		} else if(m->type().isRef()) {
			s = new QString;
		}
		
	    m->item().s_voidp = s;
	    m->next();
	    if(s && m->cleanup())
		delete s;
	}
	break;
      case Marshall::ToVALUE:
	{
	    QString *s = (QString*)m->item().s_voidp;
	    if(s) {
	    	if (s->isNull()) {
				*(m->var()) = Qnil;
	     	} else {
				*(m->var()) = rb_str_new2(s->latin1());
			}
//                if(!(PL_hints & HINT_BYTES))
//                {
//		    sv_setpv_mg(m->var(), (const char *)s->utf8());
//                    SvUTF8_on(*(m->var()));
//                }
//                else if(PL_hints & HINT_LOCALE)
//                    sv_setpv_mg(m->var(), (const char *)s->local8Bit());
//                else
//                    sv_setpv_mg(m->var(), (const char *)s->latin1());
        } else {
			*(m->var()) = Qnil;
		}
		
	    if(m->cleanup())
		delete s;
	}
	break;
      default:
	m->unsupported();
	break;
    }
}

#if 0
static const char *not_ascii(const char *s, uint &len)
{
    bool r = false;
    for(; *s ; s++, len--)
      if((uint)*s > 0x7F)
      {
        r = true;
        break;
      }
    return r ? s : 0L;
}
#endif

static void marshall_QCString(Marshall *m) {
    switch(m->action()) {
      case Marshall::FromVALUE:
	{
	    QCString *s = 0;
	    VALUE rv = *(m->var());
	    if (rv != Qundef || m->type().isStack()) {
		s = new QCString(STR2CSTR(*(m->var())));
		}
	    m->item().s_voidp = s;
	    m->next();
	    if(s && m->cleanup())
		delete s;
	}
	break;
      case Marshall::ToVALUE:
	{
	    QCString *s = (QCString*)m->item().s_voidp;
	    if(s) {
		*(m->var()) = rb_str_new2((const char *)*s);
//                const char * p = (const char *)*s;
//                uint len =  s->length();
//                if(not_ascii(p,len))
//                {
//                  #if PERL_VERSION == 6 && PERL_SUBVERSION == 0
//                  QTextCodec* c = QTextCodec::codecForMib(106); // utf8
//                  if(c->heuristicContentMatch(p,len) >= 0)
//                  #else
//                  if(is_utf8_string((U8 *)p,len))
//                  #endif
//                    SvUTF8_on(*(m->var()));
//                }
		} else {
			*(m->var()) = Qundef;
		}
	    if(m->cleanup())
		delete s;
	}
	break;
      default:
	m->unsupported();
	break;
    }
}

static void marshall_QCOORD_array(Marshall *m) {
    switch(m->action()) {
      case Marshall::FromVALUE:
	{
	    VALUE av = *(m->var());
	    if (TYPE(av) != T_ARRAY) {
//	    if(!SvROK(sv) || SvTYPE(SvRV(sv)) != SVt_PVAV ||
//		av_len((AV*)SvRV(sv)) < 0) {
		m->item().s_voidp = 0;
		break;
	    }
//	    AV *av = (AV*)SvRV(sv);
	    int count = RARRAY(av)->len;
	    QCOORD *coord = new QCOORD[count + 2];
	    for(long i = 0; i <= count; i++) {
		VALUE svp = rb_ary_entry(av, i);
		coord[i] = NUM2INT(svp);
//		coord[i] = svp ? SvIV(*svp) : 0;
	    }
	    m->item().s_voidp = coord;
	    m->next();
	}
	break;
      default:
	m->unsupported();
    }
}

static void marshall_intR(Marshall *m) {
    switch(m->action()) {
      case Marshall::FromVALUE:
	{
	    VALUE rv = *(m->var());
	    if(m->type().isPtr() &&		// is pointer
	       TYPE(rv) != T_BIGNUM) {   // and real undef
		m->item().s_voidp = 0;		// pass null pointer
		break;
	    }
	    if(m->cleanup()) {
		int i = 0;
//		int i = NUM2INT(rv);
		m->item().s_voidp = &i;
		m->next();
		// How to do this in Ruby?
//		sv_setiv_mg(sv, (IV)i);
	    } else {
		m->item().s_voidp = new int((int)NUM2INT(rv));
//		if(PL_dowarn)
//		    rb_warning("Leaking memory from int& handler");
	    }
	}
	break;
      case Marshall::ToVALUE:
	{
	    int *ip = (int*)m->item().s_voidp;
	    VALUE rv = *(m->var());
	    if(!ip) {
	    rv = Qundef;
		break;
	    }
		*(m->var()) = INT2NUM(*ip);
	    m->next();
		// How to do this in Ruby?
//	    if(!m->type().isConst())
//		*ip = (int)SvIV(sv);
	}
	break;
      default:
	m->unsupported();
	break;
    }
}

static void marshall_boolR(Marshall *m) {
    switch(m->action()) {
      case Marshall::FromVALUE:
	{
	    VALUE rv = *(m->var());
	    if(m->type().isPtr() &&		// is pointer
	       TYPE(rv) != T_BIGNUM) {   // and real undef
		m->item().s_voidp = 0;		// pass null pointer
		break;
	    }
	    if(m->cleanup()) {
		bool i = rv == Qtrue ? true : false;
		m->item().s_voidp = &i;
		m->next();
//		sv_setsv_mg(sv, boolSV(i));
	    } else {
		m->item().s_voidp = new bool(rv == Qtrue?true:false);
//		if(PL_dowarn)
//		    rb_warning("Leaking memory from bool& handler");
	    }
	}
	break;
      case Marshall::ToVALUE:
	{
	    bool *ip = (bool*)m->item().s_voidp;
	    if(!ip) {
	    *(m->var()) = Qundef;
		break;
	    }
	    *(m->var()) = INT2NUM(*ip?1:0);
	    m->next();
//	    if(!m->type().isConst())
//		*ip = SvTRUE(sv)? true : false;
	}
	break;
      default:
	m->unsupported();
	break;
    }
}

static void marshall_charP_array(Marshall *m) {
    switch(m->action()) {
      case Marshall::FromVALUE:
	{
	    VALUE arglist = *(m->var());
	    if (	arglist == Qnil
	    		|| TYPE(arglist) != T_ARRAY
	      		|| RARRAY(arglist)->len == 0 )
		{
			m->item().s_voidp = 0;
			break;
	    }

//	    AV *arglist = (AV*)SvRV(sv);
	    int count = RARRAY(arglist)->len;
	    char **argv = new char *[count + 2];
	    long i;
	    for(i = 0; i < count; i++) {
		VALUE item = rb_ary_entry(arglist, i);
//		if(!item || !SvOK(*item)) {
//		    argv[i] = new char[1];
//		    argv[i][0] = 0;	// should undef warn?
//		    continue;
//		}

		char *s = STR2CSTR(item);
		int len = strlen(s);
//		char *s = SvPV(*item, len);
		argv[i] = new char[len + 1];
		strcpy(argv[i], s);
//		argv[i][len] = 0;	// null terminazi? yes
	    }
	    argv[i] = 0;
	    m->item().s_voidp = argv;
	    m->next();
	    if(m->cleanup()) {
		rb_ary_clear(arglist);
		for(i = 0; argv[i]; i++)
		    rb_ary_push(arglist, rb_str_new2(argv[i]));

		// perhaps we should check current_method?
	    }
	}
	break;
      default:
	m->unsupported();
	break;
    }
}

void marshall_QStringList(Marshall *m) {
    switch(m->action()) {
      case Marshall::FromVALUE:
	{
	    VALUE list = *(m->var());
	    if (TYPE(list) != T_ARRAY) {
//	    if(!SvROK(sv) || SvTYPE(SvRV(sv)) != SVt_PVAV ||
//		av_len((AV*)SvRV(sv)) < 0) {
		m->item().s_voidp = 0;
		break;
	    }
//	    AV *list = (AV*)SvRV(sv);
	    int count = RARRAY(list)->len;
	    QStringList *stringlist = new QStringList;
	    long i;
//            bool lc = PL_hints & HINT_LOCALE;
	    for(i = 0; i <= count; i++) {
		VALUE item = rb_ary_entry(list, i);
		if(TYPE(item) != T_STRING) {
		    stringlist->append(QString());
		    continue;
		}

//               if(SvUTF8(*item))
//		    stringlist->append(QString::fromUtf8(SvPV_nolen(*item)));
//                else if(lc)
//                    stringlist->append(QString::fromLocal8Bit(SvPV_nolen(*item)));
//                else
//                    stringlist->append(QString::fromLatin1(SvPV_nolen(*item)));
			stringlist->append(QString::fromUtf8(STR2CSTR(item)));
	    }

	    m->item().s_voidp = stringlist;
	    m->next();

	    if(m->cleanup()) {
		rb_ary_clear(list);
		for(QStringList::Iterator it = stringlist->begin();
		    it != stringlist->end();
		    ++it)
		    rb_ary_push(list, rb_str_new2((const char *)*it));
		delete stringlist;
	    }
	}
	break;
      case Marshall::ToVALUE:
	{
	    QStringList *stringlist = (QStringList*)m->item().s_voidp;
	    if(!stringlist) {
	    *(m->var()) = Qundef;
		break;
	    }

	    VALUE av = rb_ary_new();
//	    {
//		VALUE rv = newRV_noinc((SV*)av);
//		sv_setsv_mg(m->var(), rv);
//		SvREFCNT_dec(rv);
//	    }
//            if(!(PL_hints & HINT_BYTES))
//                for(QStringList::Iterator it = stringlist->begin();
//                    it != stringlist->end();
//                    ++it) {
//                    VALUE sv = newSVpv((const char *)(*it).utf8(), 0);
//                    SvUTF8_on(sv);
//                    av_push(av, sv);
//                }
//           else if(PL_hints & HINT_LOCALE)
//                for(QStringList::Iterator it = stringlist->begin();
//                    it != stringlist->end();
//                    ++it) {
//                    VALUE sv = newSVpv((const char *)(*it).local8Bit(), 0);
//                    av_push(av, sv);
//                }
//            else
	        for(QStringList::Iterator it = stringlist->begin();
		    it != stringlist->end();
		    ++it) {
                    VALUE rv = rb_str_new2((const char *)(*it).latin1());
		    rb_ary_push(av, rv);
                }
	    if(m->cleanup())
		delete stringlist;
	}
	break;
      default:
	m->unsupported();
	break;
    }
}

void marshall_QValueListInt(Marshall *m) {
    switch(m->action()) {
      case Marshall::FromVALUE:
	{
	    VALUE list = *(m->var());
	    if (TYPE(list) != T_ARRAY) {
		m->item().s_voidp = 0;
		break;
	    }
	    int count = RARRAY(list)->len;
	    QValueList<int> *valuelist = new QValueList<int>;
	    long i;
	    for(i = 0; i <= count; i++) {
	    VALUE item = rb_ary_entry(list, i);
		if(TYPE(item) != T_FIXNUM && TYPE(item) != T_BIGNUM) {
		    valuelist->append(0);
		    continue;
		}

	    valuelist->append(NUM2INT(item));
	    }

	    m->item().s_voidp = valuelist;
	    m->next();

	    if(m->cleanup()) {
		rb_ary_clear(list);
		for(QValueListIterator<int> it = valuelist->begin();
		    it != valuelist->end();
		    ++it)
		    rb_ary_push(list, INT2NUM((int)*it));
		delete valuelist;
	    }
	}
	break;
      case Marshall::ToVALUE:
	{
	    QValueList<int> *valuelist = (QValueList<int>*)m->item().s_voidp;
	    if(!valuelist) {
		*(m->var()) = Qundef;
		break;
	    }

	    VALUE av = rb_ary_new();
//	    AV *av = newAV();
//	    {
//		VALUE rv = newRV_noinc((SV*)av);
//		sv_setsv_mg(m->var(), rv);
//		SvREFCNT_dec(rv);
//	    }

	    for(QValueListIterator<int> it = valuelist->begin();
		it != valuelist->end();
		++it)
		rb_ary_push(av, INT2NUM((int)*it));
//		av_push(av, newSViv((int)*it));
	    if(m->cleanup())
		delete valuelist;
	}
	break;
      default:
	m->unsupported();
	break;
    }
}

void marshall_voidP(Marshall *m) {
    switch(m->action()) {
      case Marshall::FromVALUE:
	{
	    VALUE rv = *(m->var());
//	    if(SvROK(sv) && SvRV(sv) && SvOK(SvRV(sv)))
		if (rv != Qundef)
		m->item().s_voidp = (void*)NUM2INT(*(m->var()));
//		m->item().s_voidp = (void*)SvIV(SvRV(*(m->var())));
	    else
		m->item().s_voidp = 0;
	}
	break;
      case Marshall::ToVALUE:
	{
//	    VALUE sv = newSViv((IV)m->item().s_voidp);
//	    VALUE rv = newRV_noinc(sv);
//	    sv_setsv_mg(m->var(), rv);
//	    SvREFCNT_dec(rv);
		*(m->var()) = Data_Wrap_Struct(rb_cObject, 0, 0, m->item().s_voidp);
	}
	break;
      default:
	m->unsupported();
	break;
    }
}

void marshall_QRgb_array(Marshall *m) {
    switch(m->action()) {
      case Marshall::FromVALUE:
	{
	    VALUE list = *(m->var());
	    if (TYPE(list) != T_ARRAY) {
//	    if(!SvROK(sv) || SvTYPE(SvRV(sv)) != SVt_PVAV ||
//		av_len((AV*)SvRV(sv)) < 0) {
		m->item().s_voidp = 0;
		break;
	    }
//	    AV *list = (AV*)SvRV(sv);
	    int count = RARRAY(list)->len;
	    QRgb *rgb = new QRgb[count + 2];
	    long i;
	    for(i = 0; i <= count; i++) {
		VALUE item = rb_ary_entry(list, i);
		if(TYPE(item) != T_FIXNUM && TYPE(item) != T_BIGNUM) {
		    rgb[i] = 0;
		    continue;
		}

		rgb[i] = NUM2INT(item);
	    }
	    m->item().s_voidp = rgb;
	    m->next();
	    if(m->cleanup())
		delete[] rgb;
	}
	break;
      case Marshall::ToVALUE:
	// Implement this with a tied array or something
      default:
	m->unsupported();
	break;
    }
}

TypeHandler Qt_handlers[] = {
    { "QString", marshall_QString },
    { "QString&", marshall_QString },
    { "QString*", marshall_QString },
    { "QCString", marshall_QCString },
    { "QCString&", marshall_QCString },
    { "QCString*", marshall_QCString },
    { "QStringList", marshall_QStringList },
    { "QStringList&", marshall_QStringList },
    { "QStringList*", marshall_QStringList },
    { "int&", marshall_intR },
    { "int*", marshall_intR },
    { "bool&", marshall_boolR },
    { "bool*", marshall_boolR },
    { "char*", marshall_charP },
    { "char**", marshall_charP_array },
    { "uchar*", marshall_ucharP },
    { "QRgb*", marshall_QRgb_array },
    { "QUObject*", marshall_voidP },
    { "const QCOORD*", marshall_QCOORD_array },
    { "void", marshall_void },
    { "QValueList<int>", marshall_QValueListInt },
    { "QValueList<int>*", marshall_QValueListInt },
    { "QValueList<int>&", marshall_QValueListInt },
    { 0, 0 }
};

static VALUE type_handlers = 0;

void install_handlers(TypeHandler *h) {
    if(type_handlers == 0) {
    	type_handlers = rb_hash_new();
		rb_gc_register_address(&type_handlers);     
    }
    
    while(h->name) {
	rb_hash_aset(type_handlers, rb_str_new2(h->name), INT2NUM((int)h));
	h++;
    }
}

Marshall::HandlerFn getMarshallFn(const SmokeType &type) {
    if(type.elem())
	return marshall_basetype;
    if(!type.name())
	return marshall_void;
    if(!type_handlers) {
	return marshall_unknown;
    }
    unsigned int len = strlen(type.name());
	VALUE name = rb_str_new2(type.name());
    VALUE svp = rb_hash_aref(type_handlers, name);
    if(svp == Qnil && type.isConst() && len > strlen("const ")) {
    	svp = rb_hash_aref(type_handlers, rb_str_new2(type.name() + strlen("const ")));
	}
	
	if(svp != Qnil) {
		TypeHandler *h = (TypeHandler*)NUM2INT(svp);
		return h->fn;
    }

    return marshall_unknown;
}
