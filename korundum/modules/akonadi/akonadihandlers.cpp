/***************************************************************************
                          akonadihandlers.cpp  -  Akonadi specific marshallers
                             -------------------
    begin                : Thurs May 29 2008
    copyright            : (C) 2008 by Richard Dale
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
#include <marshall_macros.h>

#include <akonadi/agentinstance.h>
#include <akonadi/agenttype.h>
#include <akonadi/attribute.h>
#include <akonadi/collection.h>
#include <akonadi/item.h>

DEF_LIST_MARSHALLER( AkonadiAttributeList, QList<Akonadi::Attribute*>, Akonadi::Attribute )

DEF_VALUELIST_MARSHALLER( AkonadiAgentInstanceList, QList<Akonadi::AgentInstance>, Akonadi::AgentInstance )
DEF_VALUELIST_MARSHALLER( AkonadiAgentTypeList, QList<Akonadi::AgentType>, Akonadi::AgentType )
DEF_VALUELIST_MARSHALLER( AkonadiCollectionList, QList<Akonadi::Collection>, Akonadi::Collection )
DEF_VALUELIST_MARSHALLER( AkonadiItemList, QList<Akonadi::Item>, Akonadi::Item )

TypeHandler Akonadi_handlers[] = {
    { "Akonadi::AgentInstance::List", marshall_AkonadiAgentInstanceList },
    { "Akonadi::AgentType::List", marshall_AkonadiAgentTypeList },
    { "Akonadi::Attribute::List", marshall_AkonadiAttributeList },
    { "Akonadi::Collection::List", marshall_AkonadiCollectionList },
    { "Akonadi::Collection::List&", marshall_AkonadiCollectionList },
    { "Akonadi::Item::List", marshall_AkonadiItemList },
    { "Akonadi::Item::List&", marshall_AkonadiItemList },
    { 0, 0 }
};
