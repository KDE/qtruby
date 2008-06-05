/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef MARSHALL_COMPLEX_H
#define MARSHALL_COMPLEX_H

template <>
void marshall_from_ruby<long long>(Marshall *m) 
{
	VALUE obj = *(m->var());
	m->item().s_voidp = new long long;
	*(long long *)m->item().s_voidp = ruby_to_primitive<long long>(obj);
	
	m->next();
	
	if(m->cleanup() && m->type().isConst()) {
		delete (long long int *) m->item().s_voidp;
	}	
}

template <>
void marshall_from_ruby<unsigned long long>(Marshall *m) 
{
	VALUE obj = *(m->var());
	m->item().s_voidp = new unsigned long long;
	*(long long *)m->item().s_voidp = ruby_to_primitive<unsigned long long>(obj);

	m->next();
	
	if(m->cleanup() && m->type().isConst()) {
		delete (long long int *) m->item().s_voidp;
	}	
}

template <>
void marshall_from_ruby<int *>(Marshall *m) 
{
	VALUE rv = *(m->var());
	int *i = new int;
	
	if (rv == Qnil) {
		m->item().s_voidp = 0;
		return;
	} else if (TYPE(rv) == T_OBJECT) {
		// A Qt::Integer has been passed as an integer value
		VALUE temp = rb_funcall(qt_internal_module, rb_intern("get_qinteger"), 1, rv);
		*i = NUM2INT(temp);
		m->item().s_voidp = i;
		m->next();
		rb_funcall(qt_internal_module, rb_intern("set_qinteger"), 2, rv, INT2NUM(*i));
		rv = temp;
	} else {
		*i = NUM2INT(rv);
		m->item().s_voidp = i;
		m->next();
	}

	if(m->cleanup() && m->type().isConst()) {
		delete i;
	} else {
		m->item().s_voidp = new int((int)NUM2INT(rv));
	}
}

template <>
void marshall_to_ruby<int *>(Marshall *m)
{
	int *ip = (int*)m->item().s_voidp;
	VALUE rv = *(m->var());
	if(!ip) {
		rv = Qnil;
		return;
	}
	
	*(m->var()) = INT2NUM(*ip);
	m->next();
	if(!m->type().isConst())
		*ip = NUM2INT(*(m->var()));
}

template <>
void marshall_from_ruby<unsigned int *>(Marshall *m) 
{
	VALUE rv = *(m->var());
	unsigned int *i = new unsigned int;
	
	if (rv == Qnil) {
		m->item().s_voidp = 0;
		return;
	} else if (TYPE(rv) == T_OBJECT) {
		// A Qt::Integer has been passed as an integer value
		VALUE temp = rb_funcall(qt_internal_module, rb_intern("get_qinteger"), 1, rv);
		*i = NUM2INT(temp);
		m->item().s_voidp = i;
		m->next();
		rb_funcall(qt_internal_module, rb_intern("set_qinteger"), 2, rv, INT2NUM(*i));
		rv = temp;
	} else {
		*i = NUM2UINT(rv);
		m->item().s_voidp = i;
		m->next();
	}

	if(m->cleanup() && m->type().isConst()) {
		delete i;
	} else {
		m->item().s_voidp = new int((int)NUM2UINT(rv));
	}
}

template <>
void marshall_to_ruby<unsigned int *>(Marshall *m)
{
	unsigned int *ip = (unsigned int*) m->item().s_voidp;
	VALUE rv = *(m->var());
	if (ip == 0) {
		rv = Qnil;
		return;
	}
	
	*(m->var()) = UINT2NUM(*ip);
	m->next();
	if(!m->type().isConst())
		*ip = NUM2UINT(*(m->var()));
}

template <>
void marshall_from_ruby<bool *>(Marshall *m) 
{
   VALUE rv = *(m->var());
	bool * b = new bool;

	if (TYPE(rv) == T_OBJECT) {
		// A Qt::Boolean has been passed as a value
		VALUE temp = rb_funcall(qt_internal_module, rb_intern("get_qboolean"), 1, rv);
		*b = (temp == Qtrue ? true : false);
		m->item().s_voidp = b;
		m->next();
		rb_funcall(qt_internal_module, rb_intern("set_qboolean"), 2, rv, (*b ? Qtrue : Qfalse));
	} else {
		*b = (rv == Qtrue ? true : false);
		m->item().s_voidp = b;
		m->next();
	}

	if(m->cleanup() && m->type().isConst()) {
		delete b;
	}
}

template <>
void marshall_to_ruby<bool *>(Marshall *m)
{
	bool *ip = (bool*)m->item().s_voidp;
	if(!ip) {
		*(m->var()) = Qnil;
		return;
	}
	*(m->var()) = (*ip?Qtrue:Qfalse);
	m->next();
	if(!m->type().isConst())
	*ip = *(m->var()) == Qtrue ? true : false;
}

template <class Item, class ItemList, const char *ItemSTR >
void marshall_ItemList(Marshall *m) {
	switch(m->action()) {
		case Marshall::FromVALUE:
		{
			VALUE list = *(m->var());
			if (TYPE(list) != T_ARRAY) {
				m->item().s_voidp = 0;
				break;
			}

			int count = RARRAY(list)->len;
			ItemList *cpplist = new ItemList;
			long i;
			for(i = 0; i < count; i++) {
				VALUE item = rb_ary_entry(list, i);
				// TODO do type checking!
				smokeruby_object *o = value_obj_info(item);
				if(!o || !o->ptr)
					continue;
				void *ptr = o->ptr;
				ptr = o->smoke->cast(
					ptr,				// pointer
					o->classId,				// from
		    		o->smoke->idClass(ItemSTR).index	// to
				);
				cpplist->append((Item*)ptr);
			}

			m->item().s_voidp = cpplist;
			m->next();

			if (!m->type().isConst()) {
				rb_ary_clear(list);
	
				for(int i = 0; i < cpplist->size(); ++i ) {
					VALUE obj = getPointerObject( (void *) cpplist->at(i) );
					rb_ary_push(list, obj);
				}
			}

			if (m->cleanup()) {
				delete cpplist;
			}
		}
		break;
      
		case Marshall::ToVALUE:
		{
			ItemList *valuelist = (ItemList*)m->item().s_voidp;
			if(!valuelist) {
				*(m->var()) = Qnil;
				break;
			}

			VALUE av = rb_ary_new();

			for (int i=0;i<valuelist->size();++i) {
				void *p = (void *) valuelist->at(i);

				if (m->item().s_voidp == 0) {
					*(m->var()) = Qnil;
					break;
				}

				VALUE obj = getPointerObject(p);
				if (obj == Qnil) {
					smokeruby_object  * o = alloc_smokeruby_object(	false, 
																	m->smoke(), 
																	m->smoke()->idClass(ItemSTR).index, 
																	p );

					obj = set_obj_info(resolve_classname(o->smoke, o->classId, o->ptr), o);
				}
			
				rb_ary_push(av, obj);
			}

			*(m->var()) = av;
			m->next();

			if (m->cleanup()) {
				delete valuelist;
			}
		}
		break;

		default:
			m->unsupported();
		break;
   }
}

#define DEF_LIST_MARSHALLER(ListIdent,ItemList,Item) namespace { char ListIdent##STR[] = #Item; }  \
        Marshall::HandlerFn marshall_##ListIdent = marshall_ItemList<Item,ItemList,ListIdent##STR>;

template <class Item, class ItemList, const char *ItemSTR >
void marshall_ValueListItem(Marshall *m) {
	switch(m->action()) {
		case Marshall::FromVALUE:
		{
			VALUE list = *(m->var());
			if (TYPE(list) != T_ARRAY) {
				m->item().s_voidp = 0;
				break;
			}
			int count = RARRAY(list)->len;
			ItemList *cpplist = new ItemList;
			long i;
			for(i = 0; i < count; i++) {
				VALUE item = rb_ary_entry(list, i);
				// TODO do type checking!
				smokeruby_object *o = value_obj_info(item);

				// Special case for the QList<QVariant> type
				if (	qstrcmp(ItemSTR, "QVariant") == 0 
						&& (!o || !o->ptr || o->classId != o->smoke->idClass("QVariant").index) ) 
				{
					// If the value isn't a Qt::Variant, then try and construct
					// a Qt::Variant from it
					item = rb_funcall(qvariant_class, rb_intern("fromValue"), 1, item);
					if (item == Qnil) {
						continue;
					}
					o = value_obj_info(item);
				}

				if (!o || !o->ptr)
					continue;
				
				void *ptr = o->ptr;
				ptr = o->smoke->cast(
					ptr,				// pointer
					o->classId,				// from
					o->smoke->idClass(ItemSTR).index	        // to
				);
				cpplist->append(*(Item*)ptr);
			}

			m->item().s_voidp = cpplist;
			m->next();

			if (!m->type().isConst()) {
				rb_ary_clear(list);
				for(int i=0; i < cpplist->size(); ++i) {
					VALUE obj = getPointerObject((void*)&(cpplist->at(i)));
					rb_ary_push(list, obj);
				}
			}

			if (m->cleanup()) {
				delete cpplist;
			}
		}
		break;
      
		case Marshall::ToVALUE:
		{
			ItemList *valuelist = (ItemList*)m->item().s_voidp;
			if(!valuelist) {
				*(m->var()) = Qnil;
				break;
			}

			VALUE av = rb_ary_new();

			int ix = m->smoke()->idClass(ItemSTR).index;
			const char * className = m->smoke()->binding->className(ix);

			for(int i=0; i < valuelist->size() ; ++i) {
				void *p = (void *) &(valuelist->at(i));

				if(m->item().s_voidp == 0) {
				*(m->var()) = Qnil;
				break;
				}

				VALUE obj = getPointerObject(p);
				if(obj == Qnil) {
					smokeruby_object  * o = alloc_smokeruby_object(	false, 
																	m->smoke(), 
																	m->smoke()->idClass(ItemSTR).index, 
																	p );
					obj = set_obj_info(className, o);
				}
		
				rb_ary_push(av, obj);
			}

			*(m->var()) = av;
			m->next();

			if (m->cleanup()) {
				delete valuelist;
			}

		}
		break;
      
		default:
			m->unsupported();
		break;
	}
}

#define DEF_VALUELIST_MARSHALLER(ListIdent,ItemList,Item) namespace { char ListIdent##STR[] = #Item; }  \
        Marshall::HandlerFn marshall_##ListIdent = marshall_ValueListItem<Item,ItemList,ListIdent##STR>;

#endif
