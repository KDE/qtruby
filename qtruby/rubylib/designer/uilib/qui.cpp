/***************************************************************************
                          qui.cpp  -  A ruby wrapper for the QWidgetFactory class
                             -------------------
    begin                : Wed Mar 14 2004
    copyright            : (C) 2004 by Richard Dale
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

#include <qwidgetfactory.h>
#include <qwidget.h>

#include "smoke.h"

#undef DEBUG
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#ifndef __USE_POSIX
#define __USE_POSIX
#endif
#ifndef __USE_XOPEN
#define __USE_XOPEN
#endif
#include <ruby.h>

#include "qtruby.h"
#include "smokeruby.h"

extern Smoke *qt_Smoke;
extern bool isDerivedFrom(Smoke *smoke, Smoke::Index classId, Smoke::Index baseId);

extern "C" {
extern VALUE set_obj_info(const char * className, smokeruby_object * o);

static VALUE qui_module;
static VALUE qwidget_factory_class;

static VALUE
create(int argc, VALUE * argv, VALUE /*klass*/)
{
	QWidget * topLevelWidget = 0;
	VALUE result = Qnil;
	
	if (argc == 0 || argc > 4) {
		rb_raise(rb_eArgError, "wrong number of arguments (%d)\n", argc);
	}
	
	QObject * connector = 0;
	if (argc >= 2) {
		if (TYPE(argv[1]) == T_DATA) {
			smokeruby_object *o = value_obj_info(argv[1]);
			if (o != 0) {
				connector = (QObject *) o->ptr;
			}
		} else {
			rb_raise(rb_eArgError, "invalid argument type\n");
		}
	}
	
	QWidget * parent = 0;
	if (argc >= 3) {
		if (TYPE(argv[2]) == T_DATA) {
			smokeruby_object *o = value_obj_info(argv[2]);
			if (o != 0) {
				parent = (QWidget *) o->ptr;
			}
		} else {
			rb_raise(rb_eArgError, "invalid argument type\n");
		}
	}
	
	const char * name = 0;
	if (argc >= 4) {
		if (TYPE(argv[3]) == T_STRING) {
			name = StringValuePtr(argv[3]);
		} else {
			rb_raise(rb_eArgError, "invalid argument type\n");
		}
	}
	
	if (TYPE(argv[0]) == T_STRING) {
		topLevelWidget = QWidgetFactory::create(QString(StringValuePtr(argv[0])), connector, parent, name);
	} else if (TYPE(argv[0]) == T_DATA) {
		QIODevice * dev = 0;
		smokeruby_object *o = value_obj_info(argv[0]);
		
		if (o != 0 && o->ptr != 0 && o->classId == qt_Smoke->idClass("QIODevice")) {
			dev = (QIODevice *) o->ptr;
		} else {
			rb_raise(rb_eArgError, "invalid argument type\n");
		}
		
		topLevelWidget = QWidgetFactory::create(dev, connector, parent, name);
	} else {
		rb_raise(rb_eArgError, "invalid argument type\n");
	}
	
	if (topLevelWidget != 0) {
		smokeruby_object  * o = (smokeruby_object *) malloc(sizeof(smokeruby_object));
		o->smoke = qt_Smoke;
		o->classId = qt_Smoke->idClass(topLevelWidget->className());
		o->ptr = topLevelWidget;
		o->allocated = false;
		
		const char * className = qt_Smoke->binding->className(o->classId);
		result = set_obj_info(className, o);
	}
	
	return result;
}

static VALUE
load_images(VALUE klass, VALUE dir)
{
	QWidgetFactory::loadImages(QString(StringValuePtr(dir)));
	return klass;
}

static VALUE
widgets(VALUE /*self*/)
{
    VALUE result = rb_ary_new();
	QStringList widgetList = QWidgetFactory::widgets();
	
	for (QStringList::Iterator it = widgetList.begin(); it != widgetList.end(); ++it) {
		QString widgetName = *it;
		if (widgetName.startsWith("Q")) {
			widgetName.replace(0, 1, QString("Qt::"));
		} else if (widgetName.startsWith("K")) {
			widgetName.replace(0, 1, QString("KDE::"));
		}
		rb_ary_push(result, rb_str_new2(widgetName.latin1()));
    }

    return result;
}

static VALUE
supports_widget(VALUE /*self*/, VALUE widget)
{
	QString widgetName(StringValuePtr(widget));
	
	if (widgetName.startsWith("Qt::")) {
		widgetName.replace(0, 4, QString("Q"));
	} else if (widgetName.startsWith("KDE::")) {
		widgetName.replace(0, 5, QString("K"));
	}
	
	return QWidgetFactory::supportsWidget(widgetName) ? Qtrue : Qfalse;
}

void
Init_qui()
{
    qui_module = rb_define_module("QUI");
    qwidget_factory_class = rb_define_class_under(qui_module, "WidgetFactory", rb_cObject);
    
	rb_define_singleton_method(qwidget_factory_class, "create", (VALUE (*) (...)) create, -1);
	rb_define_singleton_method(qwidget_factory_class, "loadImages", (VALUE (*) (...)) load_images, 1);
	rb_define_singleton_method(qwidget_factory_class, "load_images", (VALUE (*) (...)) load_images, 1);
	rb_define_singleton_method(qwidget_factory_class, "widgets", (VALUE (*) (...)) widgets, 0);
	rb_define_singleton_method(qwidget_factory_class, "supportsWidget", (VALUE (*) (...)) supports_widget, 1);
	rb_define_singleton_method(qwidget_factory_class, "supports_widget", (VALUE (*) (...)) supports_widget, 1);
}

};
