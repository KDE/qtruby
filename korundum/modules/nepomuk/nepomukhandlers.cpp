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

#include <QtCore/qhash.h>
#include <QtCore/qurl.h>

#include <qtcore_smoke.h>
#include <qtruby.h>
#include <smokeruby.h>
#include <marshall_macros.h>

#include <nepomuk/class.h>
#include <nepomuk/property.h>
#include <nepomuk/resource.h>
#include <nepomuk/tag.h>
#include <nepomuk/variant.h>

void marshall_QHashQUrlNepomukVariant(Marshall *m) {
    switch(m->action()) {
    case Marshall::FromVALUE:
    {
        VALUE hash = *(m->var());
        if (TYPE(hash) != T_HASH) {
            m->item().s_voidp = 0;
            break;
        }
        
        QHash<QUrl, Nepomuk::Variant> * map = new QHash<QUrl, Nepomuk::Variant>;
        
        // Convert the ruby hash to an array of key/value arrays
        VALUE temp = rb_funcall(hash, rb_intern("to_a"), 0);

        for (long i = 0; i < RARRAY_LEN(temp); i++) {
            VALUE key = rb_ary_entry(rb_ary_entry(temp, i), 0);
            VALUE value = rb_ary_entry(rb_ary_entry(temp, i), 1);
            
            smokeruby_object *k = value_obj_info(key);
            smokeruby_object *v = value_obj_info(value);
            (*map)[(QUrl)*(QUrl*)k->ptr] = (Nepomuk::Variant)*(Nepomuk::Variant*)v->ptr;
        }
        
        m->item().s_voidp = map;
        m->next();
        
        if (m->cleanup()) {
            delete map;
        }
    }
    break;

    case Marshall::ToVALUE:
    {
        QHash<QUrl, Nepomuk::Variant> *map = (QHash<QUrl, Nepomuk::Variant>*)m->item().s_voidp;
        if (map == 0) {
            *(m->var()) = Qnil;
            break;
        }
        
        VALUE hv = rb_hash_new();
            
        QHash<QUrl, Nepomuk::Variant>::Iterator it;
        for (it = map->begin(); it != map->end(); ++it) {
            void * keyPtr = new QUrl(it.key());
            VALUE key = getPointerObject(keyPtr);

            if (key == Qnil) {
                smokeruby_object  * k = alloc_smokeruby_object( true, 
                                                                qtcore_Smoke, 
                                                                qtcore_Smoke->idClass("QUrl").index, 
                                                                keyPtr );
                key = set_obj_info("Qt::Url", k);
            }

            void * valuePtr = new Nepomuk::Variant(it.value());
            VALUE value = getPointerObject(valuePtr);
                
            if (value == Qnil) {
                smokeruby_object  * v = alloc_smokeruby_object( true, 
                                                                m->smoke(), 
                                                                m->smoke()->idClass("Nepomuk::Variant").index, 
                                                                valuePtr );
                value = set_obj_info("Nepomuk::Variant", v);
            }
            
            rb_hash_aset(hv, key, value);
        }
        
        *(m->var()) = hv;
        m->next();
        
        if (m->cleanup()) {
            delete map;
        }
    }
    break;

    default:
        m->unsupported();
        break;
    }
}

DEF_VALUELIST_MARSHALLER( NepomukResourceList, QList<Nepomuk::Resource>, Nepomuk::Resource )
DEF_VALUELIST_MARSHALLER( NepomukTagList, QList<Nepomuk::Tag>, Nepomuk::Tag )
DEF_VALUELIST_MARSHALLER( NepomukTypesClassList, QList<Nepomuk::Types::Class>, Nepomuk::Types::Class )
DEF_VALUELIST_MARSHALLER( NepomukTypesPropertyList, QList<Nepomuk::Types::Property>, Nepomuk::Types::Property )

TypeHandler Nepomuk_handlers[] = {
    { "QList<Nepomuk::Resource>", marshall_NepomukResourceList },
    { "QList<Nepomuk::Resource>&", marshall_NepomukResourceList },
    { "QList<Nepomuk::Tag>", marshall_NepomukTagList },
    { "QList<Nepomuk::Tag>&", marshall_NepomukTagList },
    { "QList<Nepomuk::Types::Class>", marshall_NepomukTypesClassList },
    { "QList<Nepomuk::Types::Class>&", marshall_NepomukTypesClassList },
    { "QList<Nepomuk::Types::Property>", marshall_NepomukTypesPropertyList },
    { "QList<Nepomuk::Types::Property>&", marshall_NepomukTypesPropertyList },
    { "QHash<QUrl,Nepomuk::Variant>", marshall_QHashQUrlNepomukVariant },
    { 0, 0 }
};

// kate: space-indent on; indent-width 4; replace-tabs on; mixed-indent off;
