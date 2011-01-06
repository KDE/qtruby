/***************************************************************************
                 plasmahandlers.cpp  -  Plasma specific marshallers
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
#include <marshall_macros.h>
#include <qtcore_smoke.h>

#include <plasma/packagestructure.h>
#include <plasma/containment.h>
#include <plasma/applet.h>
#include <plasma/datacontainer.h>

void marshall_PackageStructurePtr(Marshall *m) {
    switch(m->action()) {
    case Marshall::FromVALUE: 
    {
        break;
    }

    case Marshall::ToVALUE: 
    {
        KSharedPtr<Plasma::PackageStructure> *ptr = new KSharedPtr<Plasma::PackageStructure>(*(KSharedPtr<Plasma::PackageStructure>*)m->item().s_voidp);
        if (ptr == 0) {
            *(m->var()) = Qnil;
            break;
        }
        Plasma::PackageStructure * package = ptr->data();
 
        VALUE obj = getPointerObject(package);
        if (obj == Qnil) {
            smokeruby_object  * o = ALLOC(smokeruby_object);
            o->smoke = m->smoke();
            o->classId = m->smoke()->idClass("Plasma::PackageStructure").index;
            o->ptr = package;
            o->allocated = false;
            obj = set_obj_info("Plasma::PackageStructure", o);
        }

        *(m->var()) = obj;		

        if (m->cleanup()) {
        }
        break;
    }

    default:
        m->unsupported();
        break;
    }
}

void marshall_QHashQStringQVariant(Marshall *m) {
	switch(m->action()) {
	case Marshall::FromVALUE:
	{
		VALUE hash = *(m->var());
		if (TYPE(hash) != T_HASH) {
			m->item().s_voidp = 0;
			break;
	    }
		
		QHash<QString,QVariant> * map = new QHash<QString,QVariant>;
		
		// Convert the ruby hash to an array of key/value arrays
		VALUE temp = rb_funcall(hash, rb_intern("to_a"), 0);

		for (long i = 0; i < RARRAY_LEN(temp); i++) {
			VALUE key = rb_ary_entry(rb_ary_entry(temp, i), 0);
			VALUE value = rb_ary_entry(rb_ary_entry(temp, i), 1);
			
			smokeruby_object *o = value_obj_info(value);
			if (o == 0 || o->ptr == 0) {
				continue;
			}
			
			(*map)[QString(StringValuePtr(key))] = (QVariant)*(QVariant*)o->ptr;
		}
	    
		m->item().s_voidp = map;
		m->next();
		
	    if(m->cleanup())
		delete map;
	}
	break;
	case Marshall::ToVALUE:
	{
		QHash<QString,QVariant> *map = (QHash<QString,QVariant>*)m->item().s_voidp;
		if (!map) {
			*(m->var()) = Qnil;
			break;
		}
		
	    VALUE hv = rb_hash_new();
			
		QHash<QString,QVariant>::Iterator it;
		for (it = map->begin(); it != map->end(); ++it) {
			void *p = new QVariant(it.value());
			VALUE obj = getPointerObject(p);
				
			if (obj == Qnil) {
				smokeruby_object  * o = alloc_smokeruby_object(	true, 
																qtcore_Smoke, 
																qtcore_Smoke->idClass("QVariant").index, 
																p );
				obj = set_obj_info("Qt::Variant", o);
			}

			rb_hash_aset(hv, rb_str_new2(((QString*)&(it.key()))->toLatin1()), obj);
        }
		
		*(m->var()) = hv;
		m->next();
		
//		if(m->cleanup())
//			delete map;
	}
	break;
      default:
	m->unsupported();
	break;
    }
}

DEF_LIST_MARSHALLER( PlasmaContainmentList, QList<Plasma::Containment*>, Plasma::Containment )
DEF_LIST_MARSHALLER( PlasmaAppletList, QList<Plasma::Applet*>, Plasma::Applet )

DEF_HASH_MARSHALLER( QHashQStringApplet, Plasma::Applet )
DEF_HASH_MARSHALLER( QHashQStringDataContainer, Plasma::DataContainer )
DEF_HASH_MARSHALLER( QHashQStringDataEngine, Plasma::DataEngine )

TypeHandler Plasma_handlers[] = {
    { "Plasma::PackageStructure::Ptr", marshall_PackageStructurePtr },
    { "QHash<QString,QVariant>", marshall_QHashQStringQVariant },
    { "QHash<QString,QVariant>&", marshall_QHashQStringQVariant },
    { "Plasma::DataEngine::Data", marshall_QHashQStringQVariant },
    { "Plasma::DataEngine::Data&", marshall_QHashQStringQVariant },
    { "Plasma::DataEngine::SourceDict", marshall_QHashQStringDataContainer },
    { "Plasma::DataEngine::Dict", marshall_QHashQStringDataEngine },
    { "QList<Plasma::Containment*>", marshall_PlasmaContainmentList },
    { "QList<Plasma::Containment*>&", marshall_PlasmaContainmentList },
    { "Plasma::Applet::List", marshall_PlasmaAppletList },
    { 0, 0 }
};
