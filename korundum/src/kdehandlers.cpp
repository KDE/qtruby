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

#include <ruby.h>

#include <qtruby.h>
#include <smokeruby.h>
#include <marshall_basetypes.h>
#include <marshall_macros.h>

#include <kaboutdata.h>
#include <kaction.h>
#include <karchive.h>
#include <kautosavefile.h>
#include <kconfigdialogmanager.h>
#include <kconfigskeleton.h>
#include <kcoreconfigskeleton.h>
#include <kdatatool.h>
#include <kdeversion.h>
#include <kfile.h>
#include <kfileitem.h>
#include <khtml_part.h>
#include <kio/copyjob.h>
#include <kio/jobclasses.h>
#include <klocalizedstring.h>
#include <kmainwindow.h>
#include <kmountpoint.h>
#include <kmultitabbar.h>
#include <knewstuff2/core/entry.h>
#include <kparts/plugin.h>
#include <kplotobject.h>
#include <kplugininfo.h>
#include <kservicegroup.h>
#include <kservice.h>
#include <ksycocatype.h>
#include <ktoolbar.h>
#include <ktrader.h>
#include <kurl.h>
#include <kuser.h>


const char*
resolve_classname_kde(smokeruby_object * o)
{
	if (o->smoke->isDerivedFromByName(o->smoke->classes[o->classId].className, "KArchiveEntry")) {
		KArchiveEntry * entry = (KArchiveEntry *) o->smoke->cast(o->ptr, o->classId, o->smoke->idClass("KArchiveEntry").index);
		if (entry->isDirectory()) {
			return "KDE::ArchiveDirectory";
		} else {
			return "KDE::ArchiveFile";
		}
	}
	
	return qtruby_modules[o->smoke].binding->className(o->classId);
}

#if defined (__i386__) && defined (__GNUC__) && __GNUC__ >= 2
#  define BREAKPOINT { __asm__ __volatile__ ("int $03"); }
#else
#  define BREAKPOINT { fprintf(stderr, "hit ctrl-c\n"); int b = 0; while (b == 0) { ; } }
#endif


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
		    o->classId = m->smoke()->idClass("KService").index;
		    o->ptr = service;
		    o->allocated = true;
		    obj = set_obj_info("KDE::Service", o);
		}

	    *(m->var()) = obj;		
	    
// 		if(m->cleanup())
// 		;
		}
		break;
	default:
		m->unsupported();
		break;
	}
}

void marshall_KSharedConfigPtr(Marshall *m) {
	switch(m->action()) {
	case Marshall::FromVALUE: 
		{
			VALUE config = *(m->var());
			smokeruby_object *o = value_obj_info(config);
			m->item().s_voidp = new KSharedConfigPtr((KSharedConfig*) o->ptr);
			m->next();
			// delete (KSharedConfigPtr*) m->item().s_voidp;
		}
		break;
	case Marshall::ToVALUE: 
		{
		KSharedPtr<KSharedConfig> *ptr = new KSharedPtr<KSharedConfig>(*(KSharedPtr<KSharedConfig>*)m->item().s_voidp);
		if (ptr == 0) {
			*(m->var()) = Qnil;
			break;
		}
	    KSharedConfig * config = ptr->data();
	    
		VALUE obj = getPointerObject(config);
		if(obj == Qnil) {
		    smokeruby_object  * o = ALLOC(smokeruby_object);
		    o->smoke = m->smoke();
		    o->classId = m->smoke()->idClass("KSharedConfig").index;
		    o->ptr = config;
		    o->allocated = true;
		    obj = set_obj_info("KDE::SharedConfig", o);
		}

	    *(m->var()) = obj;		
	    
//		if(m->cleanup())
//			;
//		}
		break;
	default:
		m->unsupported();
		break;
	}
}

void marshall_KSharedMimeTypePtr(Marshall *m) {
	switch(m->action()) {
	case Marshall::FromVALUE: 
		{
			VALUE config = *(m->var());
			smokeruby_object *o = value_obj_info(config);
			m->item().s_voidp = new KSharedPtr<KMimeType>(*(static_cast<KSharedPtr<KMimeType>*>(o->ptr)));
			m->next();
			// delete (KSharedConfigPtr*) m->item().s_voidp;
		}
		break;
	case Marshall::ToVALUE: 
		{
		KSharedPtr<KMimeType> *ptr = new KSharedPtr<KMimeType>(*(static_cast<KSharedPtr<KMimeType>*>(m->item().s_voidp)));
		if (ptr == 0) {
			*(m->var()) = Qnil;
			break;
		}
	    KMimeType * config = ptr->data();
	    
		VALUE obj = getPointerObject(config);
		if(obj == Qnil) {
		    smokeruby_object  * o = ALLOC(smokeruby_object);
		    o->smoke = m->smoke();
		    o->classId = m->smoke()->idClass("KMimeType").index;
		    o->ptr = config;
		    o->allocated = true;
		    obj = set_obj_info("KDE::MimeType", o);
		}

	    *(m->var()) = obj;		
	    
//		if(m->cleanup())
//			;
//		}
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
		if (!offerList) {
			*(m->var()) = Qnil;
			break;
		}

		VALUE av = rb_ary_new();

		for (	KService::List::Iterator it = offerList->begin();
				it != offerList->end();
				++it ) 
		{
			KSharedPtr<KService> *ptr = new KSharedPtr<KService>(*it);
			KService * currentOffer = ptr->data();

			VALUE obj = getPointerObject(currentOffer);
			if (obj == Qnil) {
				smokeruby_object  * o = ALLOC(smokeruby_object);
				o->smoke = m->smoke();
				o->classId = m->smoke()->idClass("KService").index;
				o->ptr = currentOffer;
				o->allocated = false;
				obj = set_obj_info("KDE::Service", o);
			}
			rb_ary_push(av, obj);
		}

		*(m->var()) = av;		
	    
		if (m->cleanup())
			delete offerList;
	}
	break;
	default:
		m->unsupported();
		break;
	}
}

DEF_LIST_MARSHALLER( KActionList, QList<KAction*>, KAction )
DEF_LIST_MARSHALLER( KActionCollectionList, QList<KActionCollection*>, KActionCollection )
DEF_LIST_MARSHALLER( KAutoSaveFileList, QList<KAutoSaveFile*>, KAutoSaveFile )
DEF_LIST_MARSHALLER( KConfigDialogManagerList, QList<KConfigDialogManager*>, KConfigDialogManager )
DEF_LIST_MARSHALLER( KJobList, QList<KJob*>, KJob )
DEF_LIST_MARSHALLER( KMainWindowList, QList<KMainWindow*>, KMainWindow )
DEF_LIST_MARSHALLER( KMultiTabBarButtonList, QList<KMultiTabBarButton*>, KMultiTabBarButton )
DEF_LIST_MARSHALLER( KMultiTabBarTabList, QList<KMultiTabBarTab*>, KMultiTabBarTab )
DEF_LIST_MARSHALLER( KNSEntryList, QList<KNS::Entry*>, KNS::Entry )
DEF_LIST_MARSHALLER( KPartsPartList, QList<KParts::Part*>, KParts::Part )
DEF_LIST_MARSHALLER( KPartsPluginList, QList<KParts::Plugin*>, KParts::Plugin )
DEF_LIST_MARSHALLER( KPartsReadOnlyPartList, QList<KParts::ReadOnlyPart*>, KParts::ReadOnlyPart )
DEF_LIST_MARSHALLER( KPlotObjectList, QList<KPlotObject*>, KPlotObject )
DEF_LIST_MARSHALLER( KPlotPointList, QList<KPlotPoint*>, KPlotPoint )
DEF_LIST_MARSHALLER( KToolBarList, QList<KToolBar*>, KToolBar )
DEF_LIST_MARSHALLER( KXMLGUIClientList, QList<KXMLGUIClient*>, KXMLGUIClient )


DEF_VALUELIST_MARSHALLER( KAboutLicenseList, QList<KAboutLicense>, KAboutLicense )
DEF_VALUELIST_MARSHALLER( KAboutPersonList, QList<KAboutPerson>, KAboutPerson )
// DEF_VALUELIST_MARSHALLER( KCatalogNameList, QList<KCatalogName>, KCatalogName )
DEF_VALUELIST_MARSHALLER( KCoreConfigSkeletonItemEnumChoiceList, QList<KCoreConfigSkeleton::ItemEnum::Choice>, KCoreConfigSkeleton::ItemEnum::Choice )
DEF_VALUELIST_MARSHALLER( KDataToolInfoList, QList<KDataToolInfo>, KDataToolInfo )
DEF_VALUELIST_MARSHALLER( KFileItemList, QList<KFileItem>, KFileItem )
DEF_VALUELIST_MARSHALLER( KIOCopyInfoList, QList<KIO::CopyInfo>, KIO::CopyInfo )
DEF_VALUELIST_MARSHALLER( KPartsPluginPluginInfoList, QList<KParts::Plugin::PluginInfo>, KParts::Plugin::PluginInfo )
DEF_VALUELIST_MARSHALLER( KPluginInfoList, QList<KPluginInfo>, KPluginInfo )
DEF_VALUELIST_MARSHALLER( KServiceActionList, QList<KServiceAction>, KServiceAction )
DEF_VALUELIST_MARSHALLER( KServiceGroupPtrList, QList<KServiceGroup::Ptr>, KServiceGroup::Ptr )
DEF_VALUELIST_MARSHALLER( KTimeZoneLeapSecondsList, QList<KTimeZone::LeapSeconds>, KTimeZone::LeapSeconds )
DEF_VALUELIST_MARSHALLER( KTimeZonePhaseList, QList<KTimeZone::Phase>, KTimeZone::Phase )
DEF_VALUELIST_MARSHALLER( KTimeZoneTransitionList, QList<KTimeZone::Transition>, KTimeZone::Transition )
DEF_VALUELIST_MARSHALLER( KUrlList, QList<KUrl>, KUrl )
DEF_VALUELIST_MARSHALLER( KUserGroupList, QList<KUserGroup>, KUserGroup )
DEF_VALUELIST_MARSHALLER( KUserList, QList<KUser>, KUser )
DEF_VALUELIST_MARSHALLER( QColorList, QList<QColor>, QColor )

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

#define DEF_MAP_MARSHALLER(MapIdent,Qt::Key,Value) namespace { char KeyIdent##STR[] = #Qt::Key; char ValueIdent##STR[] = #Value; }  \
        Marshall::HandlerFn marshall_##MapIdent = marshall_Map<Qt::Key, Value,QMap<Qt::Key,Value>::Iterator,KeyIdent##STR, ValueIdent##STR>;

DEF_MAP_MARSHALLER( QMapKEntryKeyKEntry, KEntryKey, KEntry )

*/

TypeHandler KDE_handlers[] = {
    { "KFileItemList", marshall_KFileItemList },
    { "KFileItemList*", marshall_KFileItemList },
    { "KFileItemList&", marshall_KFileItemList },
    { "KNS::Entry::List", marshall_KNSEntryList },
    { "KPluginInfo::List", marshall_KPluginInfoList },
    { "KPluginInfo::List&", marshall_KPluginInfoList },
    { "KService::List", marshall_KServiceList },
    { "QList<KService::Ptr>", marshall_KServiceList },
    { "QList<KSharedPtr<KService> >", marshall_KServiceList },
    { "KService::Ptr", marshall_KServicePtr },
    { "KSharedConfig::Ptr", marshall_KSharedConfigPtr },
    { "KSharedConfig::Ptr&", marshall_KSharedConfigPtr },
    { "KSharedConfigPtr", marshall_KSharedConfigPtr },
    { "KSharedConfigPtr&", marshall_KSharedConfigPtr },
    { "KMimeType::Ptr", marshall_KSharedMimeTypePtr },
    { "KSharedPtr<KMimeType>", marshall_KSharedMimeTypePtr },
    { "KSharedPtr<KSharedConfig>", marshall_KSharedConfigPtr },
    { "KSharedPtr<KSharedConfig>&", marshall_KSharedConfigPtr },
    { "KUrl::List", marshall_KUrlList },
    { "KUrl::List&", marshall_KUrlList },
    { "KUrlList", marshall_KUrlList },
    { "KUrlList&", marshall_KUrlList },
    { "QList<KAboutLicense>", marshall_KAboutLicenseList },
    { "QList<KAboutPerson>", marshall_KAboutPersonList },
    { "QList<KActionCollection*>&", marshall_KActionCollectionList },
    { "QList<KAction*>", marshall_KActionList },
    { "QList<KAutoSaveFile*>", marshall_KAutoSaveFileList },
//    { "QList<KCatalogName>&", marshall_KCatalogNameList },
    { "QList<KConfigDialogManager*>", marshall_KConfigDialogManagerList },
    { "QList<KCoreConfigSkeleton::ItemEnum::Choice>", marshall_KCoreConfigSkeletonItemEnumChoiceList },
    { "QList<KCoreConfigSkeleton::ItemEnum::Choice>&", marshall_KCoreConfigSkeletonItemEnumChoiceList },
    { "QList<KDataToolInfo>", marshall_KDataToolInfoList },
    { "QList<KDataToolInfo>&", marshall_KDataToolInfoList },
    { "QList<KFileItem>&", marshall_KFileItemList },
    { "QList<KIO::CopyInfo>&", marshall_KIOCopyInfoList },
    { "QList<KJob*>&", marshall_KJobList },
    { "QList<KMainWindow*>", marshall_KMainWindowList },
    { "QList<KMainWindow*>&", marshall_KMainWindowList },
    { "QList<KMultiTabBarButton*>", marshall_KMultiTabBarButtonList },
    { "QList<KMultiTabBarTab*>", marshall_KMultiTabBarTabList },
    { "QList<KParts::Part*>", marshall_KPartsPartList },
    { "QList<KParts::Plugin*>", marshall_KPartsPluginList },
    { "QList<KParts::Plugin::PluginInfo>", marshall_KPartsPluginPluginInfoList },
    { "QList<KParts::Plugin::PluginInfo>&", marshall_KPartsPluginPluginInfoList },
    { "QList<KParts::ReadOnlyPart*>", marshall_KPartsReadOnlyPartList },
    { "QList<KPlotObject*>", marshall_KPlotObjectList },
    { "QList<KPlotObject*>&", marshall_KPlotObjectList },
    { "QList<KPlotPoint*>", marshall_KPlotPointList },
    { "QList<KPluginInfo>", marshall_KPluginInfoList },
    { "QList<KPluginInfo>&", marshall_KPluginInfoList },
    { "QList<KServiceAction>", marshall_KServiceActionList },
    { "QList<KServiceGroup::Ptr>", marshall_KServiceGroupPtrList },
    { "QList<KTimeZone::LeapSeconds>", marshall_KTimeZoneLeapSecondsList },
    { "QList<KTimeZone::LeapSeconds>&", marshall_KTimeZoneLeapSecondsList },
    { "QList<KTimeZone::Phase>", marshall_KTimeZonePhaseList },
    { "QList<KTimeZone::Phase>&", marshall_KTimeZonePhaseList },
    { "QList<KTimeZone::Transition>", marshall_KTimeZoneTransitionList },
    { "QList<KTimeZone::Transition>&", marshall_KTimeZoneTransitionList },
    { "QList<KToolBar*>", marshall_KToolBarList },
    { "QList<KUrl>", marshall_KUrlList },
    { "QList<KUserGroup>", marshall_KUserGroupList },
    { "QList<KUser>", marshall_KUserList },
    { "QList<KUser>&", marshall_KUserList },
    { "QList<KXMLGUIClient*>", marshall_KXMLGUIClientList },
    { "QList<KXMLGUIClient*>&", marshall_KXMLGUIClientList },
    { "QList<QColor>", marshall_QColorList },
    { "QList<QColor>&", marshall_QColorList },

    { 0, 0 }
};
