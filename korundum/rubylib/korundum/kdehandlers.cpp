/***************************************************************************
                          kdehandlers.cpp  -  KDE specific marshallers
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

#include <qtruby.h>
#include <smokeruby.h>

#include <dcopclient.h>
#include <dcopref.h>
#include <qptrlist.h>
#include <ktrader.h>
#include <kservicegroup.h>
#include <kservice.h>
#include <ksycocatype.h>
#include <kmainwindow.h>
#include <kfile.h>
#include <kfileview.h>
#include <kurl.h>
#include <kcmdlineargs.h>
#include <kaction.h>
#include <dom/dom_node.h>
#include <dom/dom_string.h>

extern "C" {
extern VALUE set_obj_info(const char * className, smokeruby_object * o);
};

void marshall_QCStringList(Marshall *m) {
    switch(m->action()) {
      case Marshall::FromVALUE: 
	{
	    VALUE list = *(m->var());
	    if (TYPE(list) != T_ARRAY) {
		m->item().s_voidp = 0;
		break;
	    }

	    int count = RARRAY(list)->len;
	    QCStringList *stringlist = new QCStringList;

	    for(long i = 0; i < count; i++) {
		VALUE item = rb_ary_entry(list, i);
		if(TYPE(item) != T_STRING) {
		    stringlist->append(QCString());
		    continue;
		}
		stringlist->append(QCString(StringValuePtr(item), RSTRING(item)->len + 1));
	    }

	    m->item().s_voidp = stringlist;
	    m->next();

	    if(m->cleanup()) {
		rb_ary_clear(list);
		for(QCStringList::Iterator it = stringlist->begin(); it != stringlist->end(); ++it)
		    rb_ary_push(list, rb_str_new2(static_cast<const char *>(*it)));
		delete stringlist;
	    }
	    break;
      }
      case Marshall::ToVALUE: 
	{
	    QCStringList *stringlist = static_cast<QCStringList *>(m->item().s_voidp);
	    if(!stringlist) {
		*(m->var()) = Qnil;
		break;
	    }

	    VALUE av = rb_ary_new();
	    for(QCStringList::Iterator it = stringlist->begin(); it != stringlist->end(); ++it) {
		VALUE rv = rb_str_new2(static_cast<const char *>((*it)));
		rb_ary_push(av, rv);
	    }

	    if(m->cleanup())
		delete stringlist;

	    *(m->var()) = av;
	}
	break;
      default:
	m->unsupported();
	break;
    }
}

#if defined (__i386__) && defined (__GNUC__) && __GNUC__ >= 2
#  define BREAKPOINT { __asm__ __volatile__ ("int $03"); }
#else
#  define BREAKPOINT { fprintf(stderr, "hit ctrl-c\n"); int b = 0; while (b == 0) { ; } }
#endif

void marshall_KCmdLineOptions(Marshall *m) {
	switch(m->action()) {
	case Marshall::FromVALUE: 
		{
			VALUE optionslist = *(m->var());
			if (optionslist == Qnil
			|| TYPE(optionslist) != T_ARRAY
			|| RARRAY(optionslist)->len == 0 )
			{
					m->item().s_voidp = 0;
					break;
			}

			// Allocate 'length + 1' entries, to include an all NULLs last entry
			KCmdLineOptions *cmdLineOptions = (KCmdLineOptions *) calloc(	RARRAY(optionslist)->len + 1, 
																			sizeof(struct KCmdLineOptions) );
			
			VALUE options;
			long i;
			for(i = 0; i < RARRAY(optionslist)->len; i++) {
				options = rb_ary_entry(optionslist, i);
				VALUE temp = rb_ary_entry(options, 0);
				cmdLineOptions[i].name = StringValuePtr(temp);
				temp = rb_ary_entry(options, 1);
				cmdLineOptions[i].description = StringValuePtr(temp);
				temp = rb_ary_entry(options, 2);
				cmdLineOptions[i].def = StringValuePtr(temp);
			}
			cmdLineOptions[i].name = 0;
			cmdLineOptions[i].description = 0;
			cmdLineOptions[i].def = 0;

			
			m->item().s_voidp = cmdLineOptions;
			m->next();
         /*
			if(m->cleanup()) {
			rb_ary_clear(optionslist);
			for(i = 0; cmdLineOptions[i].name; i++)
				options = rb_ary_new();
				rb_ary_push(options, rb_str_new2(cmdLineOptions[i].name));
				rb_ary_push(options, rb_str_new2(cmdLineOptions[i].description));
				rb_ary_push(options, rb_str_new2(cmdLineOptions[i].def));
				rb_ary_push(optionslist, options);
			}		
         */
		}
		break;
	case Marshall::ToVALUE: 
		{
		}
		break;
	default:
		m->unsupported();
		break;
	}
}

void marshall_KMimeTypeList(Marshall *m) {
	switch(m->action()) {
	case Marshall::FromVALUE: 
		{
		}
		break;
	case Marshall::ToVALUE: 
		{
	    KMimeType::List *offerList = (KMimeType::List*)m->item().s_voidp;
	    if(!offerList) {
		*(m->var()) = Qnil;
		break;
	    }

	    VALUE av = rb_ary_new();

	    for(KMimeType::List::Iterator it = offerList->begin();
		it != offerList->end();
		++it) {
        KMimeType * item  = new KMimeType (*(KMimeType*)((KMimeType::Ptr)(*it)).data());

		VALUE obj = getPointerObject(item);
		if(obj == Qnil) {
		    smokeruby_object  * o = ALLOC(smokeruby_object);
		    o->smoke = m->smoke();
		    o->classId = m->smoke()->idClass("KMimeType");
		    o->ptr = item;
		    o->allocated = true;
		    obj = set_obj_info("KDE::MimeType", o);
		}
		rb_ary_push(av, obj);
            }

	    *(m->var()) = av;		
	    
		if(m->cleanup())
		delete offerList;
		}
		break;
	default:
		m->unsupported();
		break;
	}
}

void marshall_KMimeTypePtr(Marshall *m) {
	switch(m->action()) {
	case Marshall::FromVALUE: 
		{
		}
		break;
	case Marshall::ToVALUE: 
		{
	    KMimeType::Ptr ptr(*(KMimeType::Ptr*)m->item().s_voidp);
	    if(ptr == 0) {
		*(m->var()) = Qnil;
		break;
	    }
	    KMimeType * mimeType = new KMimeType(*(KMimeType*)ptr);
	    
		VALUE obj = getPointerObject(mimeType);
		if(obj == Qnil) {
		    smokeruby_object  * o = ALLOC(smokeruby_object);
		    o->smoke = m->smoke();
		    o->classId = m->smoke()->idClass("KMimeType");
		    o->ptr = mimeType;
		    o->allocated = true;
		    obj = set_obj_info("KDE::MimeType", o);
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

void marshall_KServiceList(Marshall *m) {
	switch(m->action()) {
	case Marshall::FromVALUE: 
		{
		}
		break;
	case Marshall::ToVALUE: 
		{
	    KService::List *offerList = (KService::List*)m->item().s_voidp;
	    if(!offerList) {
		*(m->var()) = Qnil;
		break;
	    }

	    VALUE av = rb_ary_new();

	    for(KService::List::Iterator it = offerList->begin();
		it != offerList->end();
		++it) {
		KService::Ptr ptr = *it;
		// Increment the reference count to prevent C++ garbage collection.
		// The contents of the offerList ruby Array should really be deref'd 
		// when it's gc'd.
		ptr->_KShared_ref();
		KService * currentOffer = ptr;

		VALUE obj = getPointerObject(currentOffer);
		if(obj == Qnil) {
		    smokeruby_object  * o = ALLOC(smokeruby_object);
		    o->smoke = m->smoke();
		    o->classId = m->smoke()->idClass("KService");
		    o->ptr = currentOffer;
		    o->allocated = false;
		    obj = set_obj_info("KDE::Service", o);
		}
		rb_ary_push(av, obj);
            }

	    *(m->var()) = av;		
	    
		if(m->cleanup())
		delete offerList;
		}
		break;
	default:
		m->unsupported();
		break;
	}
}

void marshall_KServiceGroupPtr(Marshall *m) {
	switch(m->action()) {
	case Marshall::FromVALUE: 
		{
		}
		break;
	case Marshall::ToVALUE: 
		{
	    KServiceGroup::Ptr ptr(*(KServiceGroup::Ptr*)m->item().s_voidp);
	    if(ptr == 0) {
		*(m->var()) = Qnil;
		break;
	    }
	    KServiceGroup * serviceGroup = new KServiceGroup(*(KServiceGroup*)ptr);

		VALUE obj = getPointerObject(serviceGroup);
		if(obj == Qnil) {
		    smokeruby_object  * o = ALLOC(smokeruby_object);
		    o->smoke = m->smoke();
		    o->classId = m->smoke()->idClass("KServiceGroup");
		    o->ptr = serviceGroup;
		    o->allocated = true;
		    obj = set_obj_info("KDE::ServiceGroup", o);
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

void marshall_KServiceTypeList(Marshall *m) {
	switch(m->action()) {
	case Marshall::FromVALUE: 
		{
		}
		break;
	case Marshall::ToVALUE: 
		{
	    KServiceType::List *offerList = (KServiceType::List*)m->item().s_voidp;
	    if(!offerList) {
		*(m->var()) = Qnil;
		break;
	    }

	    VALUE av = rb_ary_new();

	    for(KServiceType::List::Iterator it = offerList->begin();
		it != offerList->end();
		++it) {
		KServiceType * currentOffer = new KServiceType(*((KServiceType*)*it));

		VALUE obj = getPointerObject(currentOffer);
		if(obj == Qnil) {
		    smokeruby_object  * o = ALLOC(smokeruby_object);
		    o->smoke = m->smoke();
		    o->classId = m->smoke()->idClass("KServiceType");
		    o->ptr = currentOffer;
		    o->allocated = true;
		    obj = set_obj_info("KDE::ServiceType", o);
		}
		rb_ary_push(av, obj);
            }

	    *(m->var()) = av;		
	    
		if(m->cleanup())
		delete offerList;
		}
		break;
	default:
		m->unsupported();
		break;
	}
}

void marshall_KTraderOfferList(Marshall *m) {
	switch(m->action()) {
	case Marshall::FromVALUE: 
		{
		}
		break;
	case Marshall::ToVALUE: 
		{
	    KTrader::OfferList *offerList = (KTrader::OfferList*)m->item().s_voidp;
	    if(!offerList) {
		*(m->var()) = Qnil;
		break;
	    }

	    VALUE av = rb_ary_new();

	    for(KTrader::OfferList::Iterator it = offerList->begin();
		it != offerList->end();
		++it) {
		KService::Ptr ptr = *it;
		// Increment the reference count to prevent C++ garbage collection.
		// The contents of the offerList ruby Array should really be deref'd 
		// when it's gc'd.
		ptr->_KShared_ref();
		KService * currentOffer = ptr;

		VALUE obj = getPointerObject(currentOffer);
		if(obj == Qnil) {
		    smokeruby_object  * o = ALLOC(smokeruby_object);
		    o->smoke = m->smoke();
		    o->classId = m->smoke()->idClass("KService");
		    o->ptr = currentOffer;
		    o->allocated = false;
		    obj = set_obj_info("KDE::Service", o);
		}
		rb_ary_push(av, obj);
            }

	    *(m->var()) = av;		
	    
		if(m->cleanup())
		delete offerList;
		}
		break;
	default:
		m->unsupported();
		break;
	}
}


void marshall_KURLList(Marshall *m) {
	switch(m->action()) {
	case Marshall::FromVALUE: 
		{
			VALUE list = *(m->var());
			if (TYPE(list) != T_ARRAY) {
				m->item().s_voidp = 0;
				break;
			}
			int count = RARRAY(list)->len;
			KURL::List *kurllist = new KURL::List;
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
					o->smoke->idClass("KURL")	        // to
					);
				kurllist->append((KURL&)*(KURL*)ptr);
			}

			m->item().s_voidp = kurllist;
			m->next();

			if(m->cleanup()) {
				rb_ary_clear(list);
				for (	KURL::List::Iterator it = kurllist->begin();
						it != kurllist->end();
						++it ) 
				{
					VALUE obj = getPointerObject((void*)&(*it));
					rb_ary_push(list, obj);
				}
				delete kurllist;
			}
	    }			
		break;
	case Marshall::ToVALUE: 
		{
	    KURL::List *kurllist = (KURL::List*)m->item().s_voidp;
	    if(!kurllist) {
		*(m->var()) = Qnil;
		break;
	    }

	    VALUE av = rb_ary_new();

	    int ix = m->smoke()->idClass("KURL");
	    const char * className = m->smoke()->binding->className(ix);

	    for(KURL::List::Iterator it = kurllist->begin();
		it != kurllist->end();
		++it) {
		void *p = &(*it);

		if(m->item().s_voidp == 0) {
		    *(m->var()) = Qnil;
		    break;
		}

		VALUE obj = getPointerObject(p);
		if(obj == Qnil) {
		    smokeruby_object  * o = ALLOC(smokeruby_object);
		    o->smoke = m->smoke();
		    o->classId = ix;
		    o->ptr = p;
		    o->allocated = false;
		    obj = set_obj_info(className, o);
		}
		rb_ary_push(av, obj);
            }

	    if(m->cleanup())
		delete kurllist;
	    else
	        *(m->var()) = av;		}
		break;
	default:
		m->unsupported();
		break;
	}
}


// Some time saving magic from Alex Kellett here..
template <class Item, class ItemList, class ItemListIterator, const char *ItemSTR >
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
		    o->smoke->idClass(ItemSTR)	        // to
		);
		cpplist->append((Item*)ptr);
	    }

	    m->item().s_voidp = cpplist;
	    m->next();

	    if(m->cleanup()) {
		rb_ary_clear(list);
		for(ItemListIterator it = cpplist->begin();
		    it != cpplist->end();
		    ++it) {
		    VALUE obj = getPointerObject((void*)(*it));
		    rb_ary_push(list, obj);
		}
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

	    int ix = m->smoke()->idClass(ItemSTR);
	    const char * className = m->smoke()->binding->className(ix);

	    for(ItemListIterator it = valuelist->begin();
		it != valuelist->end();
		++it) {
		void *p = *it;

		if(m->item().s_voidp == 0) {
		    *(m->var()) = Qnil;
		    break;
		}

		VALUE obj = getPointerObject(p);
		if(obj == Qnil) {
		    smokeruby_object  * o = ALLOC(smokeruby_object);
		    o->smoke = m->smoke();
		    o->classId = o->smoke->idClass(ItemSTR);
		    o->ptr = p;
		    o->allocated = false;
		    obj = set_obj_info(className, o);
		}
		rb_ary_push(av, obj);
            }

	    if(m->cleanup())
		delete valuelist;
	    else
	        *(m->var()) = av;
	}
	break;
      default:
	m->unsupported();
	break;
    }
}

#define DEF_LIST_MARSHALLER(ListIdent,ItemList,Item,Itr) namespace { char ListIdent##STR[] = #Item; };  \
        Marshall::HandlerFn marshall_##ListIdent = marshall_ItemList<Item,ItemList,Itr,ListIdent##STR>;

DEF_LIST_MARSHALLER( KFileItemList, KFileItemList, KFileItem, KFileItemList::Iterator )
DEF_LIST_MARSHALLER( KMainWindowList, QPtrList<KMainWindow>, KMainWindow, QPtrList<KMainWindow>::Iterator )
DEF_LIST_MARSHALLER( KActionList, QPtrList<KAction>, KAction, QPtrList<KAction>::Iterator )

void marshall_QMapQCStringDCOPRef(Marshall *m) {
    switch(m->action()) {
      case Marshall::FromVALUE:
	{
	    VALUE hash = *(m->var());
	    if (TYPE(hash) != T_HASH) {
		m->item().s_voidp = 0;
		break;
	    }
		
		QMap<QCString,DCOPRef> * map = new QMap<QCString,DCOPRef>;
		
		// Convert the ruby hash to an array of key/value arrays
		VALUE temp = rb_funcall(hash, rb_intern("to_a"), 0);

		for (long i = 0; i < RARRAY(temp)->len; i++) {
			VALUE key = rb_ary_entry(rb_ary_entry(temp, i), 0);
			VALUE value = rb_ary_entry(rb_ary_entry(temp, i), 1);
			
			smokeruby_object *o = value_obj_info(value);
			if( !o || !o->ptr)
                   continue;
			void * ptr = o->ptr;
			ptr = o->smoke->cast(ptr, o->classId, o->smoke->idClass("DCOPRef"));
			
			(*map)[QCString(StringValuePtr(key))] = (DCOPRef)*(DCOPRef*)ptr;
		}
	    
		m->item().s_voidp = map;
		m->next();
		
	    if(m->cleanup())
		delete map;
	}
	break;
      case Marshall::ToVALUE:
	{
	    QMap<QCString,DCOPRef> *map = (QMap<QCString,DCOPRef>*)m->item().s_voidp;
	    if(!map) {
		*(m->var()) = Qnil;
		break;
	    }
		
	    VALUE hv = rb_hash_new();
			
		QMap<QCString,DCOPRef>::Iterator it;
		for (it = map->begin(); it != map->end(); ++it) {
			void *p = new DCOPRef(it.data());
			VALUE obj = getPointerObject(p);
				
			if (obj == Qnil) {
				smokeruby_object  * o = ALLOC(smokeruby_object);
				o->classId = m->smoke()->idClass("DCOPRef");
				o->smoke = m->smoke();
				o->ptr = p;
				o->allocated = true;
				obj = set_obj_info("KDE::DCOPRef", o);
			}
			
			rb_hash_aset(hv, rb_str_new2((const char *) it.key()), obj);
        }
		
		*(m->var()) = hv;
		m->next();
		
	    if(m->cleanup())
		delete map;
	}
	break;
      default:
	m->unsupported();
	break;
    }
}

TypeHandler KDE_handlers[] = {
    { "QCStringList", marshall_QCStringList },
    { "QCStringList&", marshall_QCStringList },
    { "QCStringList*", marshall_QCStringList },
    { "KCmdLineOptions", marshall_KCmdLineOptions },
    { "KCmdLineOptions&", marshall_KCmdLineOptions },
    { "KCmdLineOptions*", marshall_KCmdLineOptions },
    { "KFileItemList", marshall_KFileItemList },
    { "KFileItemList&", marshall_KFileItemList },
    { "KFileItemList*", marshall_KFileItemList },
    { "KMainWindowList", marshall_KMainWindowList },
    { "KMainWindowList&", marshall_KMainWindowList },
    { "KMainWindowList*", marshall_KMainWindowList },
    { "QPtrList<KAction>", marshall_KActionList },
    { "QPtrList<KAction>&", marshall_KActionList },
    { "QPtrList<KAction>*", marshall_KActionList },
    { "KMimeType::List", marshall_KMimeTypeList },
    { "KMimeType::Ptr", marshall_KMimeTypePtr },
    { "KService::List", marshall_KServiceList },
    { "KServiceGroup::Ptr", marshall_KServiceGroupPtr },
    { "KServiceType::List", marshall_KServiceTypeList },
    { "KTrader::OfferList", marshall_KTraderOfferList },
    { "KTrader::OfferList&", marshall_KTraderOfferList },
    { "KTrader::OfferList*", marshall_KTraderOfferList },
    { "KURL::List", marshall_KURLList },
    { "KURL::List&", marshall_KURLList },
    { "KURL::List*", marshall_KURLList },
    { "QMap<QCString,DCOPRef>", marshall_QMapQCStringDCOPRef },
    { 0, 0 }
};
