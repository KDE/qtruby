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
#include <qptrlist.h>
#include <ktrader.h>
#include <kmainwindow.h>
#include <kfile.h>
#include <kfileview.h>
#include <kurl.h>
#include <kcmdlineargs.h>

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
		stringlist->append(StringValuePtr(item));
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
				cmdLineOptions[i].name = STR2CSTR(rb_ary_entry(options, 0));
				cmdLineOptions[i].description = STR2CSTR(rb_ary_entry(options, 1));
				cmdLineOptions[i].def = STR2CSTR(rb_ary_entry(options, 2));
			}
			cmdLineOptions[i].name = 0;
			cmdLineOptions[i].description = 0;
			cmdLineOptions[i].def = 0;
			
			m->item().s_voidp = cmdLineOptions;
			m->next();
			if(m->cleanup()) {
			rb_ary_clear(optionslist);
			for(i = 0; cmdLineOptions[i].name; i++)
				options = rb_ary_new();
				rb_ary_push(options, rb_str_new2(cmdLineOptions[i].name));
				rb_ary_push(options, rb_str_new2(cmdLineOptions[i].description));
				rb_ary_push(options, rb_str_new2(cmdLineOptions[i].def));
				rb_ary_push(optionslist, options);
			}		
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

void marshall_KTraderOfferList(Marshall *m) {
	switch(m->action()) {
	case Marshall::FromVALUE: 
		{
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
			}
			delete kurllist;
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
		    o->classId = m->type().classId();
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
    { "KTrader::OfferList", marshall_KTraderOfferList },
    { "KTrader::OfferList&", marshall_KTraderOfferList },
    { "KTrader::OfferList*", marshall_KTraderOfferList },
    { "KURL::List", marshall_KURLList },
    { "KURL::List&", marshall_KURLList },
    { "KURL::List*", marshall_KURLList },
    { 0, 0 }
};
