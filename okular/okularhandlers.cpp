/***************************************************************************
                          okularhandlers.cpp  -  Okular specific marshallers
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
#include <marshall_basetypes.h>


// DEF_LIST_MARSHALLER( OkularJobList, QList<Okular::Job*>, Okular::Job )

// DEF_VALUELIST_MARSHALLER( OkularAgentInstanceList, QList<Okular::AgentInstance>, Okular::AgentInstance )

TypeHandler Okular_handlers[] = {
//    { "Okular::AgentInstance::List", marshall_OkularAgentInstanceList },
    { 0, 0 }
};
