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

void marshall_SolidDeviceInterfaceType(Marshall *m) {
    switch(m->action()) {

    case Marshall::FromVALUE:
    {
        VALUE v = *(m->var());

        if (v == Qnil) {
            m->item().s_voidp = 0;
        } else if (TYPE(v) == T_OBJECT) {
            // Both Qt::Enum and Qt::Integer have a value() method, so 'get_qinteger()' can be called ok
            VALUE temp = rb_funcall(qt_internal_module, rb_intern("get_qinteger"), 1, v);
            m->item().s_voidp = new long((long) NUM2LONG(temp));
        } else {
            m->item().s_voidp = new long(NUM2LONG(v));
        }

        m->next();
        if (m->item().s_voidp != 0) {
            delete (long*) m->item().s_voidp;
        }
    }
    break;

    case Marshall::ToVALUE:
    {
        void * ptr = m->item().s_voidp;
        if (ptr == 0) {
            *(m->var()) = Qnil;
        } else {
            *(m->var()) = rb_funcall(qt_internal_module, rb_intern("create_qenum"),
                             2, LONG2NUM(*((long *) ptr)), rb_str_new2(m->type().name()) );
        }
    }
    break;

    default:
        m->unsupported();
        break;
    }
}

DEF_VALUELIST_MARSHALLER( SolidDeviceList, QList<Solid::Device>, Solid::Device )

TypeHandler Solid_handlers[] = {
    { "QList<Solid::Device>", marshall_SolidDeviceList },
    { "Solid::DeviceInterface::Type", marshall_SolidDeviceInterfaceType },
    { "Solid::DeviceInterface::Type&", marshall_SolidDeviceInterfaceType },

    { 0, 0 }
};
