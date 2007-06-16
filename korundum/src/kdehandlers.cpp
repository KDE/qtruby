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
#include <ktrader.h>
#include <kservicegroup.h>
#include <kservice.h>
#include <ksycocatype.h>
#include <kmainwindow.h>
#include <kfile.h>
#include <kfileitem.h>
#include <kfileview.h>
#include <kurl.h>
#include <kcmdlineargs.h>
#include <kaction.h>
#include <kfiletreebranch.h>
#include <khtml_part.h>
#include <kparts/plugin.h>
#include <kaboutdata.h>
#include <karchive.h>
#include <kconfigskeleton.h>
#include <kplugininfo.h>
#include <kmountpoint.h>
#include <kio/jobclasses.h>
#include <dom/dom_node.h>
#include <dom/dom_element.h>
#include <dom/dom_string.h>

#include <kmultitabbar.h>
#include <kdatatool.h>
#include <kuser.h>
#include <ktoolbar.h>
#include <kio/copyjob.h>

extern "C" {
extern VALUE set_obj_info(const char * className, smokeruby_object * o);
};

extern bool isDerivedFromByName(Smoke *smoke, const char *className, const char *baseClassName);

extern "C" {
/*
 * Given an approximate classname and a kde instance, try to improve the resolution of the name
 * by using the various KDE rtti mechanisms
 */
const char *
kde_resolve_classname(Smoke * smoke, int classId, void * ptr)
{
	if (isDerivedFromByName(smoke, smoke->classes[classId].className, "KArchiveEntry")) {
		KArchiveEntry * entry = (KArchiveEntry *) smoke->cast(ptr, classId, smoke->idClass("KArchiveEntry"));
		if (entry->isDirectory()) {
			return "KDE::ArchiveDirectory";
		} else {
			return "KDE::ArchiveFile";
		}
	} else if (strcmp(smoke->classes[classId].className, "DOM::Node") == 0) {
		DOM::Node * node = (DOM::Node *) smoke->cast(ptr, classId, smoke->idClass("DOM::Node"));
		switch (node->nodeType()) {
		case DOM::Node::ELEMENT_NODE:
			if (((DOM::Element*)node)->isHTMLElement()) {
				return "DOM::HTMLElement";
			} else {
				return "DOM::Element";
			}
		case DOM::Node::ATTRIBUTE_NODE:
			return "DOM::Attr";
		case DOM::Node::TEXT_NODE:
			return "DOM::Text";
		case DOM::Node::CDATA_SECTION_NODE:
			return "DOM::CDATASection";
		case DOM::Node::ENTITY_REFERENCE_NODE:
			return "DOM::EntityReference";
		case DOM::Node::ENTITY_NODE:
			return "DOM::Entity";
		case DOM::Node::PROCESSING_INSTRUCTION_NODE:
			return "DOM::ProcessingInstruction";
		case DOM::Node::COMMENT_NODE:
			return "DOM::Comment";
		case DOM::Node::DOCUMENT_NODE:
			return "DOM::Document";
		case DOM::Node::DOCUMENT_TYPE_NODE:
			return "DOM::DocumentType";
		case DOM::Node::DOCUMENT_FRAGMENT_NODE:
			return "DOM::DocumentFragment";
		case DOM::Node::NOTATION_NODE:
			return "DOM::Notation";
		}
	}
	
	return smoke->binding->className(classId);
}

};

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

/*
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
*/

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

/*
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
*/

/*
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
*/


/*
void marshall_UDSEntryList(Marshall *m) {
	switch(m->action()) {
	case Marshall::FromVALUE:
	{
		VALUE list = *(m->var());
		if (TYPE(list) != T_ARRAY) {
			m->item().s_voidp = 0;
			break;
		}
		
		KIO::UDSEntryList *cpplist = new KIO::UDSEntryList;
		
		for(long i = 0; i < RARRAY(list)->len; i++) {
			VALUE item = rb_ary_entry(list, i);
			KIO::UDSEntry *cppsublist = new KIO::UDSEntry;
			
			for (int j = 0; j < RARRAY(item)->len; j++) {
				VALUE subitem = rb_ary_entry(item, j);
				smokeruby_object *o = value_obj_info(subitem);
				if(!o || !o->ptr)
					continue;
				void *ptr = o->ptr;
				ptr = o->smoke->cast(ptr, o->classId, o->smoke->idClass("KIO::UDSAtom"));
				cppsublist->append(*(KIO::UDSAtom*)ptr);
			}
			
			cpplist->append(*cppsublist);
		}

		m->item().s_voidp = cpplist;
		m->next();
	}
	break;
	case Marshall::ToVALUE:
	{
	  	KIO::UDSEntryList *valuelist = (KIO::UDSEntryList*)m->item().s_voidp;
		if (!valuelist) {
			*(m->var()) = Qnil;
			break;
		}

		VALUE av = rb_ary_new();

		int ix = m->smoke()->idClass("KIO::UDSAtom");

		for (	KIO::UDSEntryList::Iterator it = valuelist->begin();
				it != valuelist->end();
				++it ) 
		{
			KIO::UDSEntry * udsentry = &(*it);
			VALUE subav = rb_ary_new();

			for (	KIO::UDSEntry::Iterator it = udsentry->begin();
					it != udsentry->end();
					++it ) 
			{
				void * p = &(*it);
				VALUE obj = getPointerObject(p);
				if(obj == Qnil) {
					smokeruby_object  * o = ALLOC(smokeruby_object);
					o->smoke = m->smoke();
					o->classId = ix;
					o->ptr = p;
					o->allocated = false;
					obj = set_obj_info("KIO::UDSAtom", o);
				}
				
				rb_ary_push(subav, obj);
			}
			
			rb_ary_push(av, subav);
		}
		
		*(m->var()) = av;
	}
	break;
	default:
		m->unsupported();
		break;
	}
}
*/

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
		    		o->smoke->idClass(ItemSTR)	// to
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
																	m->smoke()->idClass(ItemSTR), 
																	p );

					obj = set_obj_info(kde_resolve_classname(o->smoke, o->classId, o->ptr), o);
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

#define DEF_LIST_MARSHALLER(ListIdent,ItemList,Item) namespace { char ListIdent##STR[] = #Item; };  \
        Marshall::HandlerFn marshall_##ListIdent = marshall_ItemList<Item,ItemList,ListIdent##STR>;

DEF_LIST_MARSHALLER( KActionList, QList<KAction*>, KAction )
DEF_LIST_MARSHALLER( KActionCollectionList, QList<KActionCollection*>, KActionCollection )
DEF_LIST_MARSHALLER( KJobList, QList<KJob*>, KJob )
DEF_LIST_MARSHALLER( KMainWindowList, QList<KMainWindow*>, KMainWindow )
DEF_LIST_MARSHALLER( KMultiTabBarButtonList, QList<KMultiTabBarButton*>, KMultiTabBarButton )
DEF_LIST_MARSHALLER( KMultiTabBarTabList, QList<KMultiTabBarTab*>, KMultiTabBarTab )
DEF_LIST_MARSHALLER( KPartsPartList, QList<KParts::Part*>, KParts::Part )
DEF_LIST_MARSHALLER( KPartsPluginList, QList<KParts::Plugin*>, KParts::Plugin )
DEF_LIST_MARSHALLER( KPartsReadOnlyPartList, QList<KParts::ReadOnlyPart*>, KParts::ReadOnlyPart )
DEF_LIST_MARSHALLER( KPluginInfoList, QList<KPluginInfo*>, KPluginInfo )
DEF_LIST_MARSHALLER( KToolBarList, QList<KToolBar*>, KToolBar )
DEF_LIST_MARSHALLER( KXMLGUIClientList, QList<KXMLGUIClient*>, KXMLGUIClient )
DEF_LIST_MARSHALLER( KFileItemList, QList<KFileItem*>, KFileItem )

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
					o->smoke->idClass(ItemSTR)	        // to
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

			int ix = m->smoke()->idClass(ItemSTR);
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
																	m->smoke()->idClass(ItemSTR), 
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

#define DEF_VALUELIST_MARSHALLER(ListIdent,ItemList,Item) namespace { char ListIdent##STR[] = #Item; };  \
        Marshall::HandlerFn marshall_##ListIdent = marshall_ValueListItem<Item,ItemList,ListIdent##STR>;

DEF_VALUELIST_MARSHALLER( KAboutPersonList, QList<KAboutPerson>, KAboutPerson )
DEF_VALUELIST_MARSHALLER( KAboutTranslatorList, QList<KAboutTranslator>, KAboutTranslator )
DEF_VALUELIST_MARSHALLER( ChoicesList, QList<KConfigSkeleton::ItemEnum::Choice>, KConfigSkeleton::ItemEnum::Choice )
DEF_VALUELIST_MARSHALLER( KDataToolInfoList, QList<KDataToolInfo>, KDataToolInfo )
DEF_VALUELIST_MARSHALLER( KIOCopyInfoList, QList<KIO::CopyInfo>, KIO::CopyInfo )
DEF_VALUELIST_MARSHALLER( KPartsPluginPluginInfoList, QList<KParts::Plugin::PluginInfo>, KParts::Plugin::PluginInfo )DEF_VALUELIST_MARSHALLER( KUserList, QList<KUser>, KUser )
DEF_VALUELIST_MARSHALLER( KUserGroupList, QList<KUserGroup>, KUserGroup )
DEF_VALUELIST_MARSHALLER( KUrlList, QList<KUrl>, KUrl )

/*
template <class Qt::Key, class Value, class ItemMapIterator, const char *KeySTR, const char *ValueSTR >
void marshall_Map(Marshall *m) {
    switch(m->action()) {
      case Marshall::FromVALUE:
	{
	    VALUE hash = *(m->var());
	    if (TYPE(hash) != T_HASH) {
		m->item().s_voidp = 0;
		break;
	    }
		
		QMap<Qt::Key,Value> * map = new QMap<Qt::Key,Value>;
		
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
			
			(*map)[(Qt::Key)*(Qt::Key*)key_ptr] = (Value)*(Value*)val_ptr;
		}
	    
		m->item().s_voidp = map;
		m->next();
		
	    if(m->cleanup())
		delete map;
	}
	break;
      case Marshall::ToVALUE:
	{
	    QMap<Qt::Key,Value> *map = (QMap<Qt::Key,Value>*)m->item().s_voidp;
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

#define DEF_MAP_MARSHALLER(MapIdent,Qt::Key,Value) namespace { char KeyIdent##STR[] = #Qt::Key; char ValueIdent##STR[] = #Value; };  \
        Marshall::HandlerFn marshall_##MapIdent = marshall_Map<Qt::Key, Value,QMap<Qt::Key,Value>::Iterator,KeyIdent##STR, ValueIdent##STR>;

DEF_MAP_MARSHALLER( QMapKEntryKeyKEntry, KEntryKey, KEntry )

*/

TypeHandler KDE_handlers[] = {
    { "KCmdLineOptions*", marshall_KCmdLineOptions },
    { "KService::Ptr", marshall_KServicePtr },
    { "KService::List", marshall_KServiceList },

    { "QList<KAboutPerson>", marshall_KAboutPersonList },
    { "QList<KAboutTranslator>", marshall_KAboutTranslatorList },
    { "QList<KAction*>", marshall_KActionList },
    { "QList<KActionCollection*>&", marshall_KActionCollectionList },
    { "QList<KConfigSkeleton::ItemEnum::Choice>", marshall_ChoicesList },
    { "QList<KConfigSkeleton::ItemEnum::Choice>&", marshall_ChoicesList },
    { "QList<KDataToolInfo>", marshall_KDataToolInfoList },
    { "QList<KDataToolInfo>&", marshall_KDataToolInfoList },
    { "QList<KIO::CopyInfo>&", marshall_KIOCopyInfoList },
    { "QList<KJob*>&", marshall_KJobList },
    { "QList<KMainWindow*>&", marshall_KMainWindowList },
    { "QList<KMultiTabBarButton*>", marshall_KMultiTabBarButtonList },
    { "QList<KMultiTabBarTab*>", marshall_KMultiTabBarTabList },
    { "QList<KParts::Part*>", marshall_KPartsPartList },
    { "QList<KParts::Plugin*>", marshall_KPartsPluginList },
    { "QList<KParts::Plugin::PluginInfo>", marshall_KPartsPluginPluginInfoList },
    { "QList<KParts::Plugin::PluginInfo>&", marshall_KPartsPluginPluginInfoList },
    { "QList<KParts::ReadOnlyPart*>", marshall_KPartsReadOnlyPartList },
    { "QList<KPluginInfo*>&", marshall_KPluginInfoList },
    { "QList<KToolBar*>", marshall_KToolBarList },
    { "QList<KUser>", marshall_KUserList },
    { "QList<KUser>&", marshall_KUserList },
    { "QList<KUserGroup>", marshall_KUserGroupList },
    { "QList<KXMLGUIClient*>&", marshall_KXMLGUIClientList },
    { "KFileItemList", marshall_KFileItemList },
    { "KFileItemList&", marshall_KFileItemList },
    { "KFileItemList*", marshall_KFileItemList },
    { "KUrlList", marshall_KUrlList },
    { "KUrlList&", marshall_KUrlList },
    { 0, 0 }
};
