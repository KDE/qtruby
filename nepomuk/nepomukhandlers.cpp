/***************************************************************************
                          nepomukhandlers.cpp  -  Nepomuk specific marshallers
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

#include <nepomuk/resource.h>
#include <nepomuk/tag.h>

DEF_VALUELIST_MARSHALLER( NepomukResourceList, QList<Nepomuk::Resource>, Nepomuk::Resource )
DEF_VALUELIST_MARSHALLER( NepomukTagList, QList<Nepomuk::Tag>, Nepomuk::Tag )

TypeHandler Nepomuk_handlers[] = {
    { "QList<Nepomuk::Resource>", marshall_NepomukResourceList },
    { "QList<Nepomuk::Resource>&", marshall_NepomukResourceList },
    { "QList<Nepomuk::Tag>", marshall_NepomukTagList },
    { "QList<Nepomuk::Tag>&", marshall_NepomukTagList },
    { 0, 0 }
};
