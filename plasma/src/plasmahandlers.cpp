/***************************************************************************
                 plasmahandlers.cpp  -  Plasma specific marshallers
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

#include <ruby.h>

#include <qtruby.h>
#include <smokeruby.h>

#include <plasma/packagestructure.h>
#include <plasma/containment.h>
#include <plasma/applet.h>

void marshall_PackageStructurePtr(Marshall *m) {
	switch(m->action()) {
	case Marshall::FromVALUE: 
		{
		}
		break;
	case Marshall::ToVALUE: 
		{
		KSharedPtr<Plasma::PackageStructure> *ptr = new KSharedPtr<Plasma::PackageStructure>(*(KSharedPtr<Plasma::PackageStructure>*)m->item().s_voidp);
	    if(ptr == 0) {
		*(m->var()) = Qnil;
		break;
	    }
	    Plasma::PackageStructure * package = ptr->data();
	    
		VALUE obj = getPointerObject(package);
		if(obj == Qnil) {
		    smokeruby_object  * o = ALLOC(smokeruby_object);
		    o->smoke = m->smoke();
		    o->classId = m->smoke()->idClass("Plasma::PackageStructure").index;
		    o->ptr = package;
		    o->allocated = true;
		    obj = set_obj_info("Plasma::PackageStructure", o);
		}

	    *(m->var()) = obj;		
	    
		if(m->cleanup())
		;
		}
		break;
	default:
		m->unsupported();
		break;
	}
}

void marshall_QHashQStringQVariant(Marshall *m) {
	switch(m->action()) {
	case Marshall::FromVALUE:
	{
		VALUE hash = *(m->var());
		if (TYPE(hash) != T_HASH) {
			m->item().s_voidp = 0;
			break;
	    }
		
		QHash<QString,QVariant> * map = new QHash<QString,QVariant>;
		
		// Convert the ruby hash to an array of key/value arrays
		VALUE temp = rb_funcall(hash, rb_intern("to_a"), 0);

		for (long i = 0; i < RARRAY(temp)->len; i++) {
			VALUE key = rb_ary_entry(rb_ary_entry(temp, i), 0);
			VALUE value = rb_ary_entry(rb_ary_entry(temp, i), 1);
			
			smokeruby_object *o = value_obj_info(value);
			if (o == 0 || o->ptr == 0) {
				continue;
			}
			
			(*map)[QString(StringValuePtr(key))] = (QVariant)*(QVariant*)o->ptr;
		}
	    
		m->item().s_voidp = map;
		m->next();
		
	    if(m->cleanup())
		delete map;
	}
	break;
	case Marshall::ToVALUE:
	{
		QHash<QString,QVariant> *map = (QHash<QString,QVariant>*)m->item().s_voidp;
		if (!map) {
			*(m->var()) = Qnil;
			break;
		}
		
	    VALUE hv = rb_hash_new();
			
		QHash<QString,QVariant>::Iterator it;
		for (it = map->begin(); it != map->end(); ++it) {
			void *p = new QVariant(it.value());
			VALUE obj = getPointerObject(p);
				
			if (obj == Qnil) {
				smokeruby_object  * o = alloc_smokeruby_object(	true, 
																m->smoke(), 
																m->smoke()->idClass("QVariant").index, 
																p );
				obj = set_obj_info("Qt::Variant", o);
			}

			rb_hash_aset(hv, rb_str_new2(((QString*)&(it.key()))->toLatin1()), obj);
        }
		
		*(m->var()) = hv;
		m->next();
		
//		if(m->cleanup())
//			delete map;
	}
	break;
      default:
	m->unsupported();
	break;
    }
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
					VALUE obj = getPointerObject( cpplist->at(i) );
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
				void *p = valuelist->at(i);

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

DEF_LIST_MARSHALLER( PlasmaContainmentList, QList<Plasma::Containment*>, Plasma::Containment )
DEF_LIST_MARSHALLER( PlasmaAppletList, QList<Plasma::Applet*>, Plasma::Applet )

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

template <class Value, const char *ValueSTR >
void marshall_Hash(Marshall *m) {
	switch(m->action()) {
	case Marshall::FromVALUE:
	{
		VALUE hv = *(m->var());
		if (TYPE(hv) != T_HASH) {
			m->item().s_voidp = 0;
			break;
		}
		
		QHash<QString, Value*> * hash = new QHash<QString, Value*>;
		
		// Convert the ruby hash to an array of key/value arrays
		VALUE temp = rb_funcall(hv, rb_intern("to_a"), 0);

		for (long i = 0; i < RARRAY(temp)->len; i++) {
			VALUE key = rb_ary_entry(rb_ary_entry(temp, i), 0);
			VALUE value = rb_ary_entry(rb_ary_entry(temp, i), 1);
			
			smokeruby_object *o = value_obj_info(value);
			if( !o || !o->ptr)
				continue;
			void * val_ptr = o->ptr;
			val_ptr = o->smoke->cast(val_ptr, o->classId, o->smoke->idClass(ValueSTR).index);
			
			(*hash)[QString(StringValuePtr(key))] = (Value*)val_ptr;
		}
	    
		m->item().s_voidp = hash;
		m->next();
		
		if (m->cleanup())
			delete hash;
	}
	break;
	case Marshall::ToVALUE:
	{
		QHash<QString, Value*> *hash = (QHash<QString, Value*>*) m->item().s_voidp;
		if (hash == 0) {
			*(m->var()) = Qnil;
			break;
	    }
		
		VALUE hv = rb_hash_new();
		
		int val_ix = m->smoke()->idClass(ValueSTR).index;
	    const char * val_className = m->smoke()->binding->className(val_ix);
			
		for (QHashIterator<QString, Value*> it(*hash); it.hasNext(); it.next()) {
			void *val_p = it.value();
			VALUE value_obj = getPointerObject(val_p);
				
			if (value_obj == Qnil) {
				smokeruby_object *o = ALLOC(smokeruby_object);
				o->classId = m->smoke()->idClass(ValueSTR).index;
				o->smoke = m->smoke();
				o->ptr = val_p;
				o->allocated = true;
				value_obj = set_obj_info(val_className, o);
			}
			rb_hash_aset(hv, rb_str_new2(((QString*)&(it.key()))->toLatin1()), value_obj);
        }
		
		*(m->var()) = hv;
		m->next();
		
		if (m->cleanup())
			delete hash;
	}
	break;
      default:
	m->unsupported();
	break;
    }
}

#define DEF_HASH_MARSHALLER(HashIdent,Item) namespace { char HashIdent##STR[] = #Item; }  \
        Marshall::HandlerFn marshall_##HashIdent = marshall_Hash<Item,HashIdent##STR>;

DEF_HASH_MARSHALLER( QHashQStringApplet, Plasma::Applet )
DEF_HASH_MARSHALLER( QHashQStringDataContainer, Plasma::DataContainer )
DEF_HASH_MARSHALLER( QHashQStringDataEngine, Plasma::DataEngine )

TypeHandler Plasma_handlers[] = {
    { "Plasma::PackageStructure::Ptr", marshall_PackageStructurePtr },
    { "QHash<QString,QVariant>", marshall_QHashQStringQVariant },
    { "QHash<QString,QVariant>&", marshall_QHashQStringQVariant },
    { "Plasma::DataEngine::Data", marshall_QHashQStringQVariant },
    { "Plasma::DataEngine::Data&", marshall_QHashQStringQVariant },
    { "Plasma::DataEngine::SourceDict", marshall_QHashQStringDataContainer },
    { "Plasma::DataEngine::Dict", marshall_QHashQStringDataEngine },
    { "QList<Plasma::Containment*>", marshall_PlasmaContainmentList },
    { "QList<Plasma::Containment*>&", marshall_PlasmaContainmentList },
    { "Plasma::Applet::List", marshall_PlasmaAppletList },
    { 0, 0 }
};
