/***************************************************************************
                          solidhandlers.cpp  -  Solid specific marshallers
                             -------------------
    begin                : 08-06-2008
    copyright            : (C) 2008 by Richard Dale
    email                : richard.j.dale@gmail.com
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

#include <solid/device.h>

DEF_VALUELIST_MARSHALLER( SolidDeviceList, QList<Solid::Device>, Solid::Device )

TypeHandler Solid_handlers[] = {
    { "QList<Solid::Device>", marshall_SolidDeviceList },
    { 0, 0 }
};
