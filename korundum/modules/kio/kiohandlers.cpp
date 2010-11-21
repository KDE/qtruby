/***************************************************************************
                          kiohandlers.cpp  -  KIO specific marshallers
                             -------------------
    begin                : Mon Feb 8 2010
    copyright            : (C) 2008 by Jonathan Schmidt-Dominé
    email                : devel@the-user.org
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
#include <marshall_macros.h>

#include <kbookmarkmenu.h>
#include <QAction>
#include <QList>
#include <kfileitemdelegate.h>
#include <QAbstractItemModel>
#include <QNetworkCookie>
#include <KService>

// DEF_LIST_MARSHALLER( KIOKBookmarkMenuList, QList<KBookmarkMenu*>&, KBookmarkMenu )
DEF_LIST_MARSHALLER( QActionList, QList<QAction*>, QAction )
// DEF_LIST_MARSHALLER( QActionListRef, QList<QAction*>&, QAction )
// DEF_MAP_MARSHALLER( QStringMap, QMap<QString,QString>*, QString )
DEF_VALUELIST_MARSHALLER( KFileItemDelegeteInformationList, QList<KFileItemDelegate::Information>, KFileItemDelegate::Information )
DEF_VALUELIST_MARSHALLER( UIntList, QList<unsigned int>, unsigned int )
DEF_VALUELIST_MARSHALLER( QModelIndexList, QList<QModelIndex>, QModelIndex )
DEF_VALUELIST_MARSHALLER( QNetworkCookieList, QList<QNetworkCookie>, QNetworkCookie )


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
            
            for (   KService::List::Iterator it = offerList->begin();
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


TypeHandler KIO_handlers[] = {
// { "QList<KBookmarkMenu*>&", marshall_KIOKBookmarkMenuList },
{ "QList<KFileItemDelegate::Information>", marshall_KFileItemDelegeteInformationList },
{ "QList<KSharedPtr<KService>>", marshall_KServiceList },
{ "QList<QAction*>", marshall_QActionList },
// { "QList<QAction*>&", marshall_QActionListRef },
{ "QList<QModelIndex>", marshall_QModelIndexList },
{ "QList<QNetworkCookie>", marshall_QNetworkCookieList },
// { "QList<QPair<QString,QString>>", 0, Smoke::t_voidp|Smoke::tf_stack },     //267
// { "QList<QPair<QString,QString>>&", 0, Smoke::t_voidp|Smoke::tf_ref },      //268
// { "QList<QPair<QString,unsigned short>>", 0, Smoke::t_voidp|Smoke::tf_stack },      //269
{ "QList<unsigned int>", marshall_UIntList },
// { "QMap<QString,QString>*", marshall_QStringMap },
// { "QMap<int,QVariant>",  },
// { "QMenu*",  },
    { 0, 0 }
};
