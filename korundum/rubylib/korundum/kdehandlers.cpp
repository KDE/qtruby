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

#include <kdeversion.h>
#include <dcopclient.h>
#include <dcopobject.h>
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
#include <kdockwidget.h>
#include <kfiletreebranch.h>
#include <kfiletreeviewitem.h>
#include <khtml_part.h>
#include <kparts/plugin.h>
#include <kuserprofile.h>
#include <kaboutdata.h>
#include <kplugininfo.h>
#if KDE_VERSION >= 0x030200
#include <kmountpoint.h>
#endif
#include <kio/jobclasses.h>
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

void marshall_KServicePtr(Marshall *m) {
	switch(m->action()) {
	case Marshall::FromVALUE: 
		{
		}
		break;
	case Marshall::ToVALUE: 
		{
		KSharedPtr<KService> *ptr = new KSharedPtr<KService>(*(KSharedPtr<KService>*)m->item().s_voidp);
	    if(ptr == 0) {
		*(m->var()) = Qnil;
		break;
	    }
	    KService * service = ptr->data();
	    
		VALUE obj = getPointerObject(service);
		if(obj == Qnil) {
		    smokeruby_object  * o = ALLOC(smokeruby_object);
		    o->smoke = m->smoke();
		    o->classId = m->smoke()->idClass("KService");
		    o->ptr = service;
		    o->allocated = true;
		    obj = set_obj_info("KDE::Service", o);
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
		KSharedPtr<KService> *ptr = new KSharedPtr<KService>(*it);
		KService * currentOffer = ptr->data();

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

void marshall_KServiceGroupList(Marshall *m) {
	switch(m->action()) {
	case Marshall::FromVALUE: 
		{
		}
		break;
	case Marshall::ToVALUE: 
		{
	    KServiceGroup::List *offerList = (KServiceGroup::List*)m->item().s_voidp;
	    if(!offerList) {
		*(m->var()) = Qnil;
		break;
	    }

	    VALUE av = rb_ary_new();

	    for(KServiceGroup::List::ConstIterator it = offerList->begin();
		it != offerList->end();
		++it) {
		KSycocaEntry *p = (*it);
		VALUE obj = Qnil;
		if (p->isType(KST_KService)) {
			KService *s = static_cast<KService *>(p);
			obj = getPointerObject(s);
			if(obj == Qnil) {
		    	smokeruby_object  * o = ALLOC(smokeruby_object);
		    	o->smoke = m->smoke();
		    	o->classId = m->smoke()->idClass("KService");
		    	o->ptr = s;
		    	o->allocated = true;
		    	obj = set_obj_info("KDE::Service", o);
			}
		} else if (p->isType(KST_KServiceGroup)) {
			KServiceGroup *g = static_cast<KServiceGroup *>(p);
			obj = getPointerObject(g);
			if(obj == Qnil) {
		    	smokeruby_object  * o = ALLOC(smokeruby_object);
		    	o->smoke = m->smoke();
		    	o->classId = m->smoke()->idClass("KServiceGroup");
		    	o->ptr = g;
		    	o->allocated = true;
		    	obj = set_obj_info("KDE::ServiceGroup", o);
			}
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

#if KDE_VERSION >= 0x030200
void marshall_KMountPointList(Marshall *m) {
	switch(m->action()) {
	case Marshall::FromVALUE: 
		{
		}
		break;
	case Marshall::ToVALUE: 
		{
	    KMountPoint::List *list = (KMountPoint::List*)m->item().s_voidp;
	    if(!list) {
		*(m->var()) = Qnil;
		break;
	    }

	    VALUE av = rb_ary_new();

	    for(KMountPoint::List::Iterator it = list->begin();
		it != list->end();
		++it) {
		KMountPoint * item = new KMountPoint(*((KMountPoint*)*it));

		VALUE obj = getPointerObject(item);
		if(obj == Qnil) {
		    smokeruby_object  * o = ALLOC(smokeruby_object);
		    o->smoke = m->smoke();
		    o->classId = m->smoke()->idClass("KMountPoint");
		    o->ptr = item;
		    o->allocated = true;
		    obj = set_obj_info("KDE::MountPoint", o);
		}
		rb_ary_push(av, obj);
            }

	    *(m->var()) = av;		
	    
		if(m->cleanup())
		delete list;
		}
		break;
	default:
		m->unsupported();
		break;
	}
}
#endif

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
		KSharedPtr<KService> *ptr = new KSharedPtr<KService>(*it);
		KService * currentOffer = ptr->data();

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

void marshall_KPluginInfoList(Marshall *m) {
	switch(m->action()) {
	case Marshall::FromVALUE: 
		{
	    }			
		break;
	case Marshall::ToVALUE: 
		{
	    KPluginInfo::List *valuelist = (KPluginInfo::List*)m->item().s_voidp;
	    if(!valuelist) {
		*(m->var()) = Qnil;
		break;
	    }

	    VALUE av = rb_ary_new();

	    int ix = m->smoke()->idClass("KPluginInfo");
	    const char * className = m->smoke()->binding->className(ix);

	    for(KPluginInfo::List::Iterator it = valuelist->begin();
		it != valuelist->end();
		++it) {
		void *p = (*it);

		if(m->item().s_voidp == 0) {
		    *(m->var()) = Qnil;
		    break;
		}

		VALUE obj = getPointerObject(p);
		if(obj == Qnil) {
		    smokeruby_object  * o = ALLOC(smokeruby_object);
		    o->smoke = m->smoke();
		    o->classId = o->smoke->idClass("KPluginInfo");
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

// Some time saving magic from Alex Kellett here..
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
		    o->smoke->idClass(ItemSTR)	        // to
		);
		cpplist->append((Item*)ptr);
	    }

	    m->item().s_voidp = cpplist;
	    m->next();

	    if(m->cleanup()) {
		rb_ary_clear(list);
		for( Item * it = cpplist->first();
		    it != 0;
		    it = cpplist->next()) {
		    VALUE obj = getPointerObject((void*)it);
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

	    for(Item * it = valuelist->first();
		it != 0;
		it = valuelist->next()) {
		void *p = it;

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

#define DEF_LIST_MARSHALLER(ListIdent,ItemList,Item) namespace { char ListIdent##STR[] = #Item; };  \
        Marshall::HandlerFn marshall_##ListIdent = marshall_ItemList<Item,ItemList,ListIdent##STR>;

DEF_LIST_MARSHALLER( KFileItemList, QPtrList<KFileItem>, KFileItem )
DEF_LIST_MARSHALLER( KMainWindowList, QPtrList<KMainWindow>, KMainWindow )
DEF_LIST_MARSHALLER( KActionList, QPtrList<KAction>, KAction )
DEF_LIST_MARSHALLER( DCOPObjectList, QPtrList<DCOPObject>, DCOPObject )
DEF_LIST_MARSHALLER( KDockWidgetList, QPtrList<KDockWidget>, KDockWidget )
DEF_LIST_MARSHALLER( KFileTreeBranch, QPtrList<KFileTreeBranch>, KFileTreeBranch )
DEF_LIST_MARSHALLER( KFileTreeViewItem, QPtrList<KFileTreeViewItem>, KFileTreeViewItem )
DEF_LIST_MARSHALLER( KPartList, QPtrList<KParts::Part>, KParts::Part )
DEF_LIST_MARSHALLER( KPartPluginList, QPtrList<KParts::Plugin>, KParts::Plugin )
DEF_LIST_MARSHALLER( KPartReadOnlyPartList, QPtrList<KParts::ReadOnlyPart>, KParts::ReadOnlyPart )
DEF_LIST_MARSHALLER( KServiceTypeProfileList, QPtrList<KServiceTypeProfile>, KServiceTypeProfile )

template <class Item, class ItemList, class ItemListIterator, const char *ItemSTR >
void marshall_ValueItemList(Marshall *m) {
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
		cpplist->append(*(Item*)ptr);
	    }

	    m->item().s_voidp = cpplist;
	    m->next();

	    if(m->cleanup()) {
		rb_ary_clear(list);
		for(ItemListIterator it = cpplist->begin();
		    it != cpplist->end();
		    ++it) {
		    VALUE obj = getPointerObject((void*)&(*it));
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
		void *p = &(*it);

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

#define DEF_VALUELIST_MARSHALLER(ListIdent,ItemList,Item,Itr) namespace { char ListIdent##STR[] = #Item; };  \
        Marshall::HandlerFn marshall_##ListIdent = marshall_ValueItemList<Item,ItemList,Itr,ListIdent##STR>;

DEF_VALUELIST_MARSHALLER( KAboutPersonList, QValueList<KAboutPerson>, KAboutPerson, QValueList<KAboutPerson>::Iterator )
DEF_VALUELIST_MARSHALLER( KAboutTranslatorList, QValueList<KAboutTranslator>, KAboutTranslator, QValueList<KAboutTranslator>::Iterator )
DEF_VALUELIST_MARSHALLER( KIOCopyInfoList, QValueList<KIO::CopyInfo>, KIO::CopyInfo, QValueList<KIO::CopyInfo>::Iterator )
DEF_VALUELIST_MARSHALLER( KServiceOfferList, QValueList<KServiceOffer>, KServiceOffer, QValueList<KServiceOffer>::Iterator )

template <class Key, class Value, class ItemMapIterator, const char *KeySTR, const char *ValueSTR >
void marshall_Map(Marshall *m) {
    switch(m->action()) {
      case Marshall::FromVALUE:
	{
	    VALUE hash = *(m->var());
	    if (TYPE(hash) != T_HASH) {
		m->item().s_voidp = 0;
		break;
	    }
		
		QMap<Key,Value> * map = new QMap<Key,Value>;
		
		// Convert the ruby hash to an array of key/value arrays
		VALUE temp = rb_funcall(hash, rb_intern("to_a"), 0);

		for (long i = 0; i < RARRAY(temp)->len; i++) {
			VALUE key = rb_ary_entry(rb_ary_entry(temp, i), 0);
			VALUE value = rb_ary_entry(rb_ary_entry(temp, i), 1);
			
			smokeruby_object *o = value_obj_info(key);
			if( !o || !o->ptr)
                   continue;
			void * key_ptr = o->ptr;
			key_ptr = o->smoke->cast(key_ptr, o->classId, o->smoke->idClass(KeySTR));
			
			o = value_obj_info(value);
			if( !o || !o->ptr)
                   continue;
			void * val_ptr = o->ptr;
			val_ptr = o->smoke->cast(val_ptr, o->classId, o->smoke->idClass(ValueSTR));
			
			(*map)[(Key)*(Key*)key_ptr] = (Value)*(Value*)val_ptr;
		}
	    
		m->item().s_voidp = map;
		m->next();
		
	    if(m->cleanup())
		delete map;
	}
	break;
      case Marshall::ToVALUE:
	{
	    QMap<Key,Value> *map = (QMap<Key,Value>*)m->item().s_voidp;
	    if(!map) {
		*(m->var()) = Qnil;
		break;
	    }
		
	    VALUE hv = rb_hash_new();
	    
		int key_ix = m->smoke()->idClass(KeySTR);
	    const char * key_className = m->smoke()->binding->className(key_ix);
		
		int val_ix = m->smoke()->idClass(ValueSTR);
	    const char * val_className = m->smoke()->binding->className(val_ix);
			
		ItemMapIterator it;
		for (it = map->begin(); it != map->end(); ++it) {
			void *key_p = new Key(it.key());
			VALUE key_obj = getPointerObject(key_p);
			smokeruby_object  * o;
			
			if (key_obj == Qnil) {
				o = ALLOC(smokeruby_object);
				o->classId = m->smoke()->idClass(KeySTR);
				o->smoke = m->smoke();
				o->ptr = key_p;
				o->allocated = true;
				key_obj = set_obj_info(key_className, o);
			}
			
			void *val_p = new Value(it.data());
			VALUE value_obj = getPointerObject(val_p);
				
			if (value_obj == Qnil) {
				o = ALLOC(smokeruby_object);
				o->classId = m->smoke()->idClass(ValueSTR);
				o->smoke = m->smoke();
				o->ptr = val_p;
				o->allocated = true;
				value_obj = set_obj_info(val_className, o);
			}
			
			rb_hash_aset(hv, key_obj, value_obj);
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

#define DEF_MAP_MARSHALLER(MapIdent,Key,Value) namespace { char KeyIdent##STR[] = #Key; char ValueIdent##STR[] = #Value; };  \
        Marshall::HandlerFn marshall_##MapIdent = marshall_Map<Key, Value,QMap<Key,Value>::Iterator,KeyIdent##STR, ValueIdent##STR>;

DEF_MAP_MARSHALLER( QMapKEntryKeyKEntry, KEntryKey, KEntry )

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
    { "KCmdLineOptions*", marshall_KCmdLineOptions },
    { "KFileItemList", marshall_KFileItemList },
    { "QPtrList<KAction>", marshall_KActionList },
    { "QPtrList<KAction>&", marshall_KActionList },
    { "KMimeType::List", marshall_KMimeTypeList },
    { "KMimeType::Ptr", marshall_KMimeTypePtr },
    { "KService::Ptr", marshall_KServicePtr },
    { "KService::List", marshall_KServiceList },
    { "KServiceGroup::List", marshall_KServiceGroupList },
    { "KServiceGroup::Ptr", marshall_KServiceGroupPtr },
#if KDE_VERSION >= 0x030200
    { "KMountPoint::List", marshall_KMountPointList },
#endif
    { "KServiceType::List", marshall_KServiceTypeList },
    { "KTrader::OfferList", marshall_KTraderOfferList },
    { "KURL::List", marshall_KURLList },
    { "KURL::List&", marshall_KURLList },
    { "KFileItemList", marshall_KFileItemList },
    { "QPtrList<KMainWindow>*", marshall_KMainWindowList },
    { "QPtrList<DCOPObject>", marshall_DCOPObjectList },
    { "QPtrList<KDockWidget>&", marshall_KDockWidgetList },
    { "QPtrList<KDockWidget>*", marshall_KDockWidgetList },
    { "KFileTreeBranchList&", marshall_KFileTreeBranch },
    { "KFileTreeViewItemList&", marshall_KFileTreeViewItem },
    { "QPtrList<KParts::Part>*", marshall_KPartList },
    { "QPtrList<KParts::Plugin>", marshall_KPartPluginList },
    { "QPtrList<KParts::ReadOnlyPart>", marshall_KPartReadOnlyPartList },
    { "QPtrList<KServiceTypeProfile>&", marshall_KServiceTypeProfileList },
    { "KPluginInfo::List>", marshall_KPluginInfoList },
    { "QValueList<KAboutPerson>", marshall_KAboutPersonList },
    { "QValueList<KAboutTranslator>", marshall_KAboutTranslatorList },
    { "QValueList<KIO::CopyInfo>&", marshall_KIOCopyInfoList },
    { "KServiceTypeProfile::OfferList", marshall_KServiceOfferList },
    { "KEntryMap", marshall_QMapKEntryKeyKEntry },
    { "KEntryMap&", marshall_QMapKEntryKeyKEntry },
    { "KEntryMap*", marshall_QMapKEntryKeyKEntry },
    { "QMap<QCString,DCOPRef>", marshall_QMapQCStringDCOPRef },
    { 0, 0 }
};
