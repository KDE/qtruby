/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <qstring.h>
#include <qregexp.h>
#include <qapplication.h>
#include <qcanvas.h>
#include <qlistview.h>
#include <qiconview.h>
#include <qtable.h>
#include <qpopupmenu.h>
#include <qlayout.h>
#include <qmetaobject.h>
#include <qvaluelist.h>
#include <qobjectlist.h>
#include <qtextcodec.h>
#include <qhostaddress.h>

#include <private/qucomextra_p.h>

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

#include "marshall.h"
#include "qtruby.h"
#include "smokeruby.h"

#ifndef HINT_BYTES
#define HINT_BYTES HINT_BYTE
#endif

extern "C" {
extern VALUE set_obj_info(const char * className, smokeruby_object * o);
extern VALUE qt_internal_module;
};

extern bool isDerivedFromByName(Smoke *smoke, const char *className, const char *baseClassName);
extern void mapPointer(VALUE obj, smokeruby_object *o, Smoke::Index classId, void *lastptr);

void
smokeruby_mark(void * p)
{
	VALUE obj;
    smokeruby_object * o = (smokeruby_object *) p;
    const char *className = o->smoke->classes[o->classId].className;
	
	if(do_debug & qtdb_gc) printf("Checking for mark (%s*)%p\n", className, o->ptr);
		
    if(o->ptr && o->allocated) {
		if (isDerivedFromByName(o->smoke, className, "QListView")) {
			QListView * listview = (QListView *) o->smoke->cast(o->ptr, o->classId, o->smoke->idClass("QListView"));
			QListViewItemIterator it(listview);
			QListViewItem *item;

			while ( (item = it.current()) != 0 ) {
				++it;
				obj = getPointerObject(item);
				if (obj != Qnil) {
					if(do_debug & qtdb_gc) printf("Marking (%s*)%p -> %p\n", className, item, (void*)obj);
					rb_gc_mark(obj);
				}
			}
			return;
		}
		
		if (isDerivedFromByName(o->smoke, className, "QTable")) {
			QTable * table = (QTable *) o->smoke->cast(o->ptr, o->classId, o->smoke->idClass("QTable"));
			QTableItem *item;

			for ( int row = 0; row < table->numRows(); row++ ) {
				for ( int col = 0; col < table->numCols(); col++ ) {
					item = table->item(row, col);
					obj = getPointerObject(item);
					if (obj != Qnil) {
						if(do_debug & qtdb_gc) printf("Marking (%s*)%p -> %p\n", className, item, (void*)obj);
						rb_gc_mark(obj);
					}
				}
			}
			return;		
		}
		
		if (isDerivedFromByName(o->smoke, className, "QObject")) {
			QObject * qobject = (QObject *) o->smoke->cast(o->ptr, o->classId, o->smoke->idClass("QObject"));
			const QObjectList *l = qobject->children();
			if (l == 0) {
				return;
			}
			QObjectListIt it( *l ); // iterate over the children
			QObject *child;

			while ( (child = it.current()) != 0 ) {
				++it;
				obj = getPointerObject(child);
				if (obj != Qnil) {
					if(do_debug & qtdb_gc) printf("Marking (%s*)%p -> %p\n", className, child, (void*)obj);
					rb_gc_mark(obj);
				}
			}
			return;
		}
	}
}

void
smokeruby_free(void * p)
{
    smokeruby_object *o = (smokeruby_object*)p;
    const char *className = o->smoke->classes[o->classId].className;
	
	if(do_debug & qtdb_gc) printf("Checking for delete (%s*)%p allocated: %s\n", className, o->ptr, o->allocated ? "true" : "false");
    
	if(!o->allocated || o->ptr == 0) {
		free(o);
		return;
	}
	
	unmapPointer(o, o->classId, 0);
	object_count --;
	
	if (	strcmp(className, "QObject") == 0
			|| strcmp(className, "QListBoxItem") == 0
			|| strcmp(className, "QStyleSheetItem") == 0
			|| strcmp(className, "QSqlCursor") == 0 )
	{
		// Don't delete instances of these classes for now
		free(o);
		return;
	} else if (isDerivedFromByName(o->smoke, className, "QLayoutItem")) {
		QLayoutItem * item = (QLayoutItem *) o->smoke->cast(o->ptr, o->classId, o->smoke->idClass("QLayoutItem"));
		if (item->layout() != 0 || item->widget() != 0 || item->spacerItem() != 0) {
			free(o);
			return;
		}
	} else if (strcmp(className, "QIconViewItem") == 0) {
		QIconViewItem * item = (QIconViewItem *) o->ptr;
		if (item->iconView() != 0) {
			free(o);
			return;
		}
	} else if (strcmp(className, "QCheckListItem") == 0) {
		QCheckListItem * item = (QCheckListItem *) o->ptr;
		if (item->parent() != 0 || item->listView() != 0) {
			free(o);
			return;
		}
	} else if (strcmp(className, "QListViewItem") == 0) {
		QListViewItem * item = (QListViewItem *) o->ptr;
		if (item->parent() != 0 || item->listView() != 0) {
			free(o);
			return;
		}
	} else if (isDerivedFromByName(o->smoke, className, "QTableItem")) {
		QTableItem * item = (QTableItem *) o->smoke->cast(o->ptr, o->classId, o->smoke->idClass("QTableItem"));
		if (item->table() != 0) {
			free(o);
			return;
		}
	} else if (strcmp(className, "QPopupMenu") == 0) {
		QPopupMenu * item = (QPopupMenu *) o->ptr;
		if (item->parentWidget(FALSE) != 0) {
			free(o);
			return;
		}
	} else if (isDerivedFromByName(o->smoke, className, "QWidget")) {
		QWidget * qwidget = (QWidget *) o->smoke->cast(o->ptr, o->classId, o->smoke->idClass("QWidget"));
		if (qwidget->parentWidget(TRUE) != 0) {
			free(o);
			return;
		}
	} else if (isDerivedFromByName(o->smoke, className, "QObject")) {
		QObject * qobject = (QObject *) o->smoke->cast(o->ptr, o->classId, o->smoke->idClass("QObject"));
		if (qobject->parent() != 0) {
			free(o);
			return;
		}
	}
			
	if(do_debug & qtdb_gc) printf("Deleting (%s*)%p\n", className, o->ptr);
	
	char *methodName = new char[strlen(className) + 2];
	methodName[0] = '~';
	strcpy(methodName + 1, className);
	Smoke::Index nameId = o->smoke->idMethodName(methodName);
	Smoke::Index meth = o->smoke->findMethod(o->classId, nameId);
	if(meth > 0) {
		Smoke::Method &m = o->smoke->methods[o->smoke->methodMaps[meth].method];
		Smoke::ClassFn fn = o->smoke->classes[m.classId].classFn;
		Smoke::StackItem i[1];
		(*fn)(m.method, o->ptr, i);
	}
	delete[] methodName;
	free(o);
	
    return;
}

/*
 * Given an approximate classname and a qt instance, try to improve the resolution of the name
 * by using the various Qt rtti mechanisms for QObjects, QEvents and QCanvasItems
 */
static const char *
resolve_classname(Marshall* m, void * ptr)
{
	if (isDerivedFromByName(m->smoke(), m->smoke()->classes[m->type().classId()].className, "QEvent")) {
		QEvent * qevent = (QEvent *) m->smoke()->cast(ptr, m->type().classId(), m->smoke()->idClass("QEvent"));
		switch (qevent->type()) {
		case QEvent::ChildInserted:
		case QEvent::ChildRemoved:
			return "Qt::ChildEvent";
		case QEvent::Close:
			return "Qt::CloseEvent";
		case QEvent::ContextMenu:
			return "Qt::ContextMenuEvent";
//		case QEvent::User:
//			return "Qt::CustomEvent";
		case QEvent::DragEnter:
			return "Qt::DragEnterEvent";
		case QEvent::DragLeave:
			return "Qt::DragLeaveEvent";
		case QEvent::DragMove:
			return "Qt::DragMoveEvent";
		case QEvent::DragResponse:
			return "Qt::DragResponseEvent";
		case QEvent::Drop:
			return "Qt::DropEvent";
		case QEvent::FocusIn:
		case QEvent::FocusOut:
			return "Qt::FocusEvent";
		case QEvent::Hide:
			return "Qt::HideEvent";
		case QEvent::KeyPress:
		case QEvent::KeyRelease:
			return "Qt::KeyEvent";
		case QEvent::IMStart:
		case QEvent::IMCompose:
		case QEvent::IMEnd:
			return "Qt::IMEvent";
		case QEvent::MouseButtonPress:
		case QEvent::MouseButtonRelease:
		case QEvent::MouseButtonDblClick:
		case QEvent::MouseMove:
			return "Qt::MouseEvent";
		case QEvent::Move:
			return "Qt::MoveEvent";
		case QEvent::Paint:
			return "Qt::PaintEvent";
		case QEvent::Resize:
			return "Qt::ResizeEvent";
		case QEvent::Show:
			return "Qt::ShowEvent";
	//	case QEvent::Tablet:
	//		 return "Qt::TabletEvent";
		case QEvent::Timer:
			return "Qt::TimerEvent";
		case QEvent::Wheel:
			return "Qt::WheelEvent";
		default:
			break;
		}
	} else if (isDerivedFromByName(m->smoke(), m->smoke()->classes[m->type().classId()].className, "QObject")) {
		QObject * qobject = (QObject *) m->smoke()->cast(ptr, m->type().classId(), m->smoke()->idClass("QObject"));
		const char * classname = qobject->className();
		Smoke::Index classId = m->smoke()->idClass(classname);
		if (classId != 0) {
			return m->smoke()->binding->className(classId);
		}
	} else if (isDerivedFromByName(m->smoke(), m->smoke()->classes[m->type().classId()].className, "QCanvasItem")) {
		QCanvasItem * qcanvasitem = (QCanvasItem *) m->smoke()->cast(ptr, m->type().classId(), m->smoke()->idClass("QCanvasItem"));
		switch (qcanvasitem->rtti()) {
		case QCanvasItem::Rtti_Sprite:
			return "Qt::CanvasSprite";
		case QCanvasItem::Rtti_PolygonalItem:
			return "Qt::CanvasPolygonalItem";
		case QCanvasItem::Rtti_Text:
			return "Qt::CanvasText";
		case QCanvasItem::Rtti_Polygon:
			return "Qt::CanvasPolygon";
		case QCanvasItem::Rtti_Rectangle:
			return "Qt::CanvasRectangle";
		case QCanvasItem::Rtti_Ellipse:
			return "Qt::CanvasEllipse";
		case QCanvasItem::Rtti_Line:
			return "Qt::CanvasLine";
		case QCanvasItem::Rtti_Spline:
			return "Qt::CanvasSpline";
		default:
			break;
		}
	} else if (isDerivedFromByName(m->smoke(), m->smoke()->classes[m->type().classId()].className, "QListViewItem")) {
		QListViewItem * item = (QListViewItem *) m->smoke()->cast(ptr, m->type().classId(), m->smoke()->idClass("QListViewItem"));
		switch (item->rtti()) {
		case 0:
			return "Qt::ListViewItem";
		case 1:
			return "Qt::CheckListItem";
		default:
			return "Qt::ListViewItem";
			break;
		}
	} else if (isDerivedFromByName(m->smoke(), m->smoke()->classes[m->type().classId()].className, "QTableItem")) {
		QTableItem * item = (QTableItem *) m->smoke()->cast(ptr, m->type().classId(), m->smoke()->idClass("QTableItem"));
		switch (item->rtti()) {
		case 0:
			return "Qt::TableItem";
		case 1:
			return "Qt::ComboTableItem";
		case 2:
			return "Qt::CheckTableItem";
		default:
			return "Qt::TableItem";
			break;
		}
	}
	
	return m->smoke()->binding->className(m->type().classId());
}

bool
matches_arg(Smoke *smoke, Smoke::Index meth, Smoke::Index argidx, const char *argtype)
{
    Smoke::Index *arg = smoke->argumentList + smoke->methods[meth].args + argidx;
    SmokeType type = SmokeType(smoke, *arg);
    if(type.name() && !strcmp(type.name(), argtype))
	return true;
    return false;
}

void *
construct_copy(smokeruby_object *o)
{
    const char *className = o->smoke->className(o->classId);
    int classNameLen = strlen(className);
    char *ccSig = new char[classNameLen + 2];       // copy constructor signature
    strcpy(ccSig, className);
    strcat(ccSig, "#");
    Smoke::Index ccId = o->smoke->idMethodName(ccSig);
    delete[] ccSig;

    char *ccArg = new char[classNameLen + 8];
    sprintf(ccArg, "const %s&", className);

    Smoke::Index ccMeth = o->smoke->findMethod(o->classId, ccId);

    if(!ccMeth) {
	return 0;
    }
	Smoke::Index method = o->smoke->methodMaps[ccMeth].method;
    if(method > 0) {
	// Make sure it's a copy constructor
	if(!matches_arg(o->smoke, method, 0, ccArg)) {
            delete[] ccArg;
	    return 0;
        }
        delete[] ccArg;
        ccMeth = method;
    } else {
        // ambiguous method, pick the copy constructor
	Smoke::Index i = -method;
	while(o->smoke->ambiguousMethodList[i]) {
	    if(matches_arg(o->smoke, o->smoke->ambiguousMethodList[i], 0, ccArg))
		break;
            i++;
	}
        delete[] ccArg;
	ccMeth = o->smoke->ambiguousMethodList[i];
	if(!ccMeth)
	    return 0;
    }

    // Okay, ccMeth is the copy constructor. Time to call it.
    Smoke::StackItem args[2];
    args[0].s_voidp = 0;
    args[1].s_voidp = o->ptr;
    Smoke::ClassFn fn = o->smoke->classes[o->classId].classFn;
    (*fn)(o->smoke->methods[ccMeth].method, 0, args);
    return args[0].s_voidp;
}

void
marshall_basetype(Marshall *m)
{
    switch(m->type().elem()) {
      case Smoke::t_bool:
	switch(m->action()) {
	  case Marshall::FromVALUE:
		if (TYPE(*(m->var())) == T_OBJECT) {
			// A Qt::Boolean has been passed as a value
			VALUE temp = rb_funcall(qt_internal_module, rb_intern("get_qboolean"), 1, *(m->var()));
	    	m->item().s_bool = (temp == Qtrue ? true : false);
		} else {
	    	m->item().s_bool = (*(m->var()) == Qtrue ? true : false);
		}
	    break;
	  case Marshall::ToVALUE:
	    *(m->var()) = m->item().s_bool ? Qtrue : Qfalse;
	    break;
	  default:
	    m->unsupported();
	    break;
	}
	break;
      case Smoke::t_char:
	switch(m->action()) {
	  case Marshall::FromVALUE:
	    m->item().s_char = NUM2CHR(*(m->var()));
	    break;
	  case Marshall::ToVALUE:
	    *(m->var()) = CHR2FIX(m->item().s_char);
	    break;
	  default:
	    m->unsupported();
	    break;
	}
	break;
      case Smoke::t_uchar:
	switch(m->action()) {
	  case Marshall::FromVALUE:
	    m->item().s_uchar = NUM2CHR(*(m->var()));
	    break;
	  case Marshall::ToVALUE:
	    *(m->var()) = CHR2FIX(m->item().s_uchar);
	    break;
	  default:
	    m->unsupported();
	    break;
	}
	break;
      case Smoke::t_short:
	switch(m->action()) {
	  case Marshall::FromVALUE:
	    m->item().s_short = (short) NUM2INT(*(m->var()));
	    break;
	  case Marshall::ToVALUE:
	    *(m->var()) = INT2NUM(m->item().s_short);
	    break;
	  default:
	    m->unsupported();
	    break;
	}
	break;
      case Smoke::t_ushort:
	switch(m->action()) {
	  case Marshall::FromVALUE:
	    m->item().s_ushort = (unsigned short) NUM2INT(*(m->var()));
	    break;
	  case Marshall::ToVALUE:
	    *(m->var()) = INT2NUM(m->item().s_ushort);
	    break;
	  default:
	    m->unsupported();
	    break;
	}
	break;
      case Smoke::t_int:
	switch(m->action()) {
	  case Marshall::FromVALUE:
	    if (TYPE(*(m->var())) == T_OBJECT) {
			m->item().s_int = (int) NUM2INT(rb_funcall(qt_internal_module, rb_intern("get_qinteger"), 1, *(m->var())));
		} else {
	    	m->item().s_int = (int) NUM2INT(*(m->var()));
		}
	    break;
	  case Marshall::ToVALUE:
	    *(m->var()) = INT2NUM(m->item().s_int);
	    break;
	  default:
	    m->unsupported();
	    break;
	}
	break;
      case Smoke::t_uint:
	switch(m->action()) {
	  case Marshall::FromVALUE:
	    if (TYPE(*(m->var())) == T_OBJECT) {
			m->item().s_int = (unsigned int) NUM2UINT(rb_funcall(qt_internal_module, rb_intern("get_qinteger"), 1, *(m->var())));
		} else {
	    	m->item().s_uint = (unsigned int) NUM2UINT(*(m->var()));
		}
	    break;
	  case Marshall::ToVALUE:
	    *(m->var()) = INT2NUM(m->item().s_uint);
	    break;
	  default:
	    m->unsupported();
	    break;
	}
	break;
      case Smoke::t_long:
	switch(m->action()) {
	  case Marshall::FromVALUE:
	    if (TYPE(*(m->var())) == T_OBJECT) {
			m->item().s_int = (long) NUM2LONG(rb_funcall(qt_internal_module, rb_intern("get_qinteger"), 1, *(m->var())));
		} else {
	    	m->item().s_long = (long) NUM2LONG(*(m->var()));
		}
	    break;
	  case Marshall::ToVALUE:
	    *(m->var()) = INT2NUM(m->item().s_long);
	    break;
	  default:
	    m->unsupported();
	    break;
	}
	break;
      case Smoke::t_ulong:
	switch(m->action()) {
	  case Marshall::FromVALUE:
	    if (TYPE(*(m->var())) == T_OBJECT) {
			m->item().s_int = (unsigned long) NUM2ULONG(rb_funcall(qt_internal_module, rb_intern("get_qinteger"), 1, *(m->var())));
		} else {
	    	m->item().s_ulong = (unsigned long) NUM2ULONG(*(m->var()));
		}
	    break;
	  case Marshall::ToVALUE:
	    *(m->var()) = INT2NUM(m->item().s_ulong);
	    break;
	  default:
	    m->unsupported();
	    break;
	}
	break;
      case Smoke::t_float:
	switch(m->action()) {
	  case Marshall::FromVALUE:
	    m->item().s_float = (float) NUM2DBL(*(m->var()));
	    break;
	  case Marshall::ToVALUE:
	    *(m->var()) = rb_float_new((double) m->item().s_float);
	    break;
	  default:
	    m->unsupported();
	    break;
	}
	break;
      case Smoke::t_double:
	switch(m->action()) {
	  case Marshall::FromVALUE:
	    m->item().s_double = (double) NUM2DBL(*(m->var()));
	    break;
	  case Marshall::ToVALUE:
	    *(m->var()) = rb_float_new(m->item().s_double);
	    break;
	  default:
	    m->unsupported();
	    break;
	}
	break;
      case Smoke::t_enum:
	switch(m->action()) {
	  case Marshall::FromVALUE:
	  	{
		if (TYPE(*(m->var())) == T_OBJECT) {
			// A Qt::Enum is a subclass of Qt::Integer, so 'get_qinteger()' can be called ok
			VALUE temp = rb_funcall(qt_internal_module, rb_intern("get_qinteger"), 1, *(m->var()));
	    	m->item().s_enum = (long) NUM2LONG(temp);
		} else {
	    	m->item().s_enum = (long) NUM2LONG(*(m->var()));
		}
		}
	    break;
	  case Marshall::ToVALUE:
		*(m->var()) = rb_funcall(	qt_internal_module, 
									rb_intern("create_qenum"), 
									2, INT2NUM(m->item().s_enum), rb_str_new2(m->type().name()) );
 
	    break;
	  default:
	    m->unsupported();
	    break;
	}
	break;
      case Smoke::t_class:
	switch(m->action()) {
	  case Marshall::FromVALUE:
	    {
		smokeruby_object *o = value_obj_info(*(m->var()));
		if(!o || !o->ptr) {
                    if(m->type().isRef()) {
                        rb_warning("References can't be nil\n");
                        m->unsupported();
                    }
		    m->item().s_class = 0;
		    break;
		}
		void *ptr = o->ptr;
		if(!m->cleanup() && m->type().isStack()) {
		    ptr = construct_copy(o);
		}
		const Smoke::Class &c = m->smoke()->classes[m->type().classId()];
		ptr = o->smoke->cast(
		    ptr,				// pointer
		    o->classId,				// from
		    o->smoke->idClass(c.className)	// to
		);
		m->item().s_class = ptr;
		break;
	    }
	    break;
	  case Marshall::ToVALUE:
	    {
		if(m->item().s_voidp == 0) {
                    *(m->var()) = Qnil;
		    break;
		}

		void *p = m->item().s_voidp;
		VALUE obj = getPointerObject(p);
		if(obj != Qnil) {
                    *(m->var()) = obj;
		    break;
		}

		smokeruby_object  * o = (smokeruby_object *) malloc(sizeof(smokeruby_object));
		o->smoke = m->smoke();
		o->classId = m->type().classId();
		o->ptr = p;
		o->allocated = false;

		const char * classname = resolve_classname(m, o->ptr);
		
		if(m->type().isConst() && m->type().isRef()) {
		    p = construct_copy( o );
		    if(p) {
			o->ptr = p;
			o->allocated = true;
		    }
		}
		
		obj = set_obj_info(classname, o);
		if (do_debug & qtdb_calls) {
			printf("allocating %s %p -> %p\n", classname, o->ptr, (void*)obj);
		}

		if(m->type().isStack()) {
		    o->allocated = true;
			// Keep a mapping of the pointer so that it is only wrapped once as a ruby VALUE
		    mapPointer(obj, o, o->classId, 0);
		}
			
		*(m->var()) = obj;
	    }
	    break;
	  default:
	    m->unsupported();
	    break;
	}
	break;
      default:
	m->unsupported();
	break;
    }
}

static void marshall_void(Marshall * /*m*/) {}
static void marshall_unknown(Marshall *m) {
    m->unsupported();
}

static void marshall_charP(Marshall *m) {
    switch(m->action()) {
      case Marshall::FromVALUE:
	{
	    VALUE rv = *(m->var());
	    if(rv == Qnil) {
                m->item().s_voidp = 0;
                break;
	    }
		
        m->item().s_voidp = StringValuePtr(rv);
	}
	break;
      case Marshall::ToVALUE:
	{
	    char *p = (char*)m->item().s_voidp;
	    if(p)
                *(m->var()) = rb_str_new2(p);
	    else
                *(m->var()) = Qnil;
	    if(m->cleanup())
		delete[] p;
	}
	break;
      default:
	m->unsupported();
	break;
    }
}

void marshall_ucharP(Marshall *m) {
    switch(m->action()) {
      case Marshall::FromVALUE:
	{
	    VALUE rv = *(m->var());
	    if(rv == Qnil) {
		m->item().s_voidp = 0;
		break;
	    }
		
        m->item().s_voidp = StringValuePtr(rv);
	}
	break;
      case Marshall::ToVALUE:
      default:
	m->unsupported();
	break;
    }
}

static const char * KCODE = 0;
static QTextCodec *codec = 0;

static void 
init_codec() {
	VALUE temp = rb_gv_get("$KCODE");
	KCODE = StringValuePtr(temp);
	if (strcmp(KCODE, "EUC") == 0) {
		codec = QTextCodec::codecForName("eucJP");
	} else if (strcmp(KCODE, "SJIS") == 0) {
		codec = QTextCodec::codecForName("Shift-JIS");
	}
}

QString* 
qstringFromRString(VALUE rstring) {
	if (KCODE == 0) {
		init_codec();
	}
	
	QString *	s;
	if (strcmp(KCODE, "UTF8") == 0)
		s = new QString(QString::fromUtf8(StringValuePtr(rstring), RSTRING(rstring)->len));
	else if (strcmp(KCODE, "EUC") == 0)
		s = new QString(codec->toUnicode(StringValuePtr(rstring)));
	else if (strcmp(KCODE, "SJIS") == 0)
		s = new QString(codec->toUnicode(StringValuePtr(rstring)));
	else if(strcmp(KCODE, "NONE") == 0)
		s = new QString(QString::fromLatin1(StringValuePtr(rstring)));
	else
		s = new QString(QString::fromLocal8Bit(StringValuePtr(rstring), RSTRING(rstring)->len));
	return s;
}

VALUE 
rstringFromQString(QString * s) {
	if (KCODE == 0) {
		init_codec();
	}
	
	if (strcmp(KCODE, "UTF8") == 0)
		return rb_str_new2(s->utf8());
	else if (strcmp(KCODE, "EUC") == 0)
		return rb_str_new2(codec->fromUnicode(*s));
	else if (strcmp(KCODE, "SJIS") == 0)
		return rb_str_new2(codec->fromUnicode(*s));
	else if (strcmp(KCODE, "NONE") == 0)
		return rb_str_new2(s->latin1());
	else
		return rb_str_new2(s->local8Bit());
}

static void marshall_QString(Marshall *m) {
    switch(m->action()) {
      case Marshall::FromVALUE:
	{
	    QString* s = 0;
	    if( *(m->var()) != Qnil) {
               s = qstringFromRString(*(m->var()));
            } else {
                s = new QString(QString::null);
            }
		
	    m->item().s_voidp = s;
	    m->next();
		
		if (!m->type().isConst() && *(m->var()) != Qnil && s != 0 && !s->isNull()) {
			rb_str_resize(*(m->var()), 0);
			VALUE temp = rstringFromQString(s);
			rb_str_cat2(*(m->var()), StringValuePtr(temp));
		}
	    
		if(s && m->cleanup())
		delete s;
	}
	break;
      case Marshall::ToVALUE:
	{
	    QString *s = (QString*)m->item().s_voidp;
	    if(s) {
	    	if (s->isNull()) {
                    *(m->var()) = Qnil;
	     	} else {
               *(m->var()) = rstringFromQString(s);
	     	}
	     	if(m->cleanup())
	     	delete s;
         } else {
                *(m->var()) = Qnil;
            }
	}
	break;
      default:
	m->unsupported();
	break;
    }
}

static void marshall_QByteArray(Marshall *m) {
    switch(m->action()) {
      case Marshall::FromVALUE:
	{
	    VALUE rv = *(m->var());
	    QByteArray *s = 0;
		VALUE data = Qnil;
	    if(rv != Qnil) {
			if (rb_respond_to(rv, rb_intern("data")) != 0) {
				// Qt::ByteArray - use the contents of the 'data' instance var
				data = rb_funcall(qt_internal_module, rb_intern("get_qbytearray"), 1, rv);
				if (TYPE(data) == T_DATA) {
					// A C++ QByteArray inside the Qt::ByteArray
					Data_Get_Struct(data, QByteArray, s);
				} else {
					// Or a ruby String inside
            		s = new QByteArray(RSTRING(data)->len);
					memcpy((void*)s->data(), StringValuePtr(data), RSTRING(data)->len);
				}
			} else {
				// Ordinary ruby String - use the contents of the string
            	s = new QByteArray(RSTRING(rv)->len);
				memcpy((void*)s->data(), StringValuePtr(rv), RSTRING(rv)->len);
			}
        } else {
            s = new QByteArray(0);
	    }
	    m->item().s_voidp = s;
	    
		m->next();
	    
		if(s && m->cleanup() && data == Qnil)
		delete s;
	}
	break;
      case Marshall::ToVALUE:
	{
		VALUE result;
	    QByteArray *s = (QByteArray*)m->item().s_voidp;
	    if(s) {
			VALUE string = rb_str_new2("");
			rb_str_cat(string, (const char *)s->data(), s->size());
			result = rb_funcall(qt_internal_module, rb_intern("create_qbytearray"), 2, string, Data_Wrap_Struct(rb_cObject, 0, 0, s));
        } else {
			result = Qnil;
		}
		*(m->var()) = result;
	    if(m->cleanup())
		delete s;
	}
	break;
      default:
	m->unsupported();
	break;
    }
}

#if 0
static const char *not_ascii(const char *s, uint &len)
{
    bool r = false;
    for(; *s ; s++, len--)
      if((uint)*s > 0x7F)
      {
        r = true;
        break;
      }
    return r ? s : 0L;
}
#endif

static void marshall_QCString(Marshall *m) {
    switch(m->action()) {
      case Marshall::FromVALUE:
	{
	    QCString *s = 0;
	    VALUE rv = *(m->var());
	    if (rv == Qnil) {
		s = new QCString(); 
        } else {
		// Add 1 to the ruby string length to allow for a QCString '\0' terminator
		s = new QCString(StringValuePtr(*(m->var())), RSTRING(*(m->var()))->len + 1); 
		}
	    m->item().s_voidp = s;
	    
		m->next();
	    
		if (!m->type().isConst() && rv != Qnil && s != 0) {
			rb_str_resize(rv, 0);
			rb_str_cat2(rv, (const char *)*s);
		}
	    if(s && m->cleanup())
		delete s;
	}
	break;
      case Marshall::ToVALUE:
	{
	    QCString *s = (QCString*)m->item().s_voidp;
	    if(s && (const char *) *s != 0) {
		*(m->var()) = rb_str_new2((const char *)*s);
//                const char * p = (const char *)*s;
//                uint len =  s->length();
//                if(not_ascii(p,len))
//                {
//                  #if PERL_VERSION == 6 && PERL_SUBVERSION == 0
//                  QTextCodec* c = QTextCodec::codecForMib(106); // utf8
//                  if(c->heuristicContentMatch(p,len) >= 0)
//                  #else
//                  if(is_utf8_string((U8 *)p,len))
//                  #endif
//                    SvUTF8_on(*(m->var()));
//                }
	    } else {
			if (m->type().isConst()) {
                *(m->var()) = Qnil;
			} else {
                *(m->var()) = rb_str_new2("");
			}
	    }
		m->next();

		if (!m->type().isConst() && s != 0) {
			*s = (const char *) StringValuePtr(*(m->var()));
		}
	    
	    if(s && m->cleanup())
		delete s;
	}
	break;
      default:
	m->unsupported();
	break;
    }
}

static void marshall_QCOORD_array(Marshall *m) {
    switch(m->action()) {
      case Marshall::FromVALUE:
	{
	    VALUE av = *(m->var());
	    if (TYPE(av) != T_ARRAY) {
		m->item().s_voidp = 0;
		break;
	    }
	    int count = RARRAY(av)->len;
	    QCOORD *coord = new QCOORD[count + 2];
	    for(long i = 0; i < count; i++) {
		VALUE svp = rb_ary_entry(av, i);
		coord[i] = NUM2INT(svp);
	    }
	    m->item().s_voidp = coord;
	    m->next();
	}
	break;
      default:
	m->unsupported();
    }
}

static void marshall_longlong(Marshall *m) {
    switch(m->action()) {
      case Marshall::FromVALUE:
	{
	    m->item().s_voidp = new long long;
	    *(long long *)m->item().s_voidp = rb_num2ll(*(m->var()));
		
	    m->next();
		
	    if(m->cleanup()) {
			delete (long long *) m->item().s_voidp;
		}
	}
	break;
      case Marshall::ToVALUE:
	{
	    *(m->var()) = rb_ll2inum(*(long long *) m->item().s_voidp);
	}
	break;
      default:
	m->unsupported();
	break;
    }
}

static void marshall_ulonglong(Marshall *m) {
    switch(m->action()) {
      case Marshall::FromVALUE:
	{
	    m->item().s_voidp = new unsigned long long;
	    *(long long *)m->item().s_voidp = rb_num2ull(*(m->var()));
		
	    m->next();
		
	    if(m->cleanup()) {
			delete (unsigned long long *) m->item().s_voidp;
		}
	}
	break;
      case Marshall::ToVALUE:
	{
	    *(m->var()) = rb_ull2inum(*(unsigned long long *) m->item().s_voidp);
	}
	break;
      default:
	m->unsupported();
	break;
    }
}

static void marshall_intR(Marshall *m) {
    switch(m->action()) {
      case Marshall::FromVALUE:
	{
	    VALUE rv = *(m->var());
		int i = 0;
		if (TYPE(rv) == T_OBJECT) {
			// A Qt::Integer has been passed as an integer value
			VALUE temp = rb_funcall(qt_internal_module, rb_intern("get_qinteger"), 1, rv);
			i = NUM2INT(temp);
			m->item().s_voidp = &i;
			m->next();
			rb_funcall(qt_internal_module, rb_intern("set_qinteger"), 2, rv, INT2NUM(i));
			rv = temp;
		} else {
			i = NUM2INT(rv);
			m->item().s_voidp = &i;
			m->next();
		}
	    if(m->cleanup()) {
			;
	    } else {
		m->item().s_voidp = new int((int)NUM2INT(rv));
	    }
	}
	break;
      case Marshall::ToVALUE:
	{
	    int *ip = (int*)m->item().s_voidp;
	    VALUE rv = *(m->var());
	    if(!ip) {
	        rv = Qnil;
		break;
	    }
	    *(m->var()) = INT2NUM(*ip);
	    m->next();
	    if(!m->type().isConst())
		*ip = NUM2INT(*(m->var()));
	}
	break;
      default:
	m->unsupported();
	break;
    }
}

static void marshall_boolR(Marshall *m) {
    switch(m->action()) {
      case Marshall::FromVALUE:
	{
	    VALUE rv = *(m->var());
		bool b;
		if (TYPE(rv) == T_OBJECT) {
			// A Qt::Boolean has been passed as a value
			VALUE temp = rb_funcall(qt_internal_module, rb_intern("get_qboolean"), 1, rv);
			b = (temp == Qtrue ? true : false);
			m->item().s_voidp = &b;
			m->next();
			rb_funcall(qt_internal_module, rb_intern("set_qboolean"), 2, rv, (b ? Qtrue : Qfalse));
		} else {
			b = (rv == Qtrue ? true : false);
			m->item().s_voidp = &b;
			m->next();
		}
	}
	break;
      case Marshall::ToVALUE:
	{
	    bool *ip = (bool*)m->item().s_voidp;
	    if(!ip) {
	    *(m->var()) = Qnil;
		break;
	    }
	    *(m->var()) = (*ip?Qtrue:Qfalse);
	    m->next();
	    if(!m->type().isConst())
		*ip = *(m->var()) == Qtrue ? true : false;
	}
	break;
      default:
	m->unsupported();
	break;
    }
}

static void marshall_charP_array(Marshall *m) {
    switch(m->action()) {
      case Marshall::FromVALUE:
	{
	    VALUE arglist = *(m->var());
	    if (arglist == Qnil
	    || TYPE(arglist) != T_ARRAY
	    || RARRAY(arglist)->len == 0 )
	    {
                m->item().s_voidp = 0;
                break;
	    }

	    char **argv = new char *[RARRAY(arglist)->len + 1];
	    long i;
	    for(i = 0; i < RARRAY(arglist)->len; i++) {
                VALUE item = rb_ary_entry(arglist, i);
                char *s = StringValuePtr(item);
                argv[i] = new char[strlen(s) + 1];
                strcpy(argv[i], s);
	    }
	    argv[i] = 0;
	    m->item().s_voidp = argv;
	    m->next();
	    if(m->cleanup()) {
		rb_ary_clear(arglist);
		for(i = 0; argv[i]; i++)
		    rb_ary_push(arglist, rb_str_new2(argv[i]));
	    }
	}
	break;
      default:
	m->unsupported();
	break;
    }
}

void marshall_QStringList(Marshall *m) {
    switch(m->action()) {
      case Marshall::FromVALUE: 
	{
	    VALUE list = *(m->var());
	    if (TYPE(list) != T_ARRAY) {
		m->item().s_voidp = 0;
		break;
	    }

	    int count = RARRAY(list)->len;
	    QStringList *stringlist = new QStringList;

	    for(long i = 0; i < count; i++) {
		VALUE item = rb_ary_entry(list, i);
		if(TYPE(item) != T_STRING) {
		    stringlist->append(QString());
		    continue;
		}
		stringlist->append(*(qstringFromRString(item)));
	    }

	    m->item().s_voidp = stringlist;
	    m->next();

	    if(m->cleanup()) {
		rb_ary_clear(list);
		for(QStringList::Iterator it = stringlist->begin(); it != stringlist->end(); ++it)
		    rb_ary_push(list, rstringFromQString(&(*it)));
		delete stringlist;
	    }
	    break;
      }
      case Marshall::ToVALUE: 
	{
	    QStringList *stringlist = static_cast<QStringList *>(m->item().s_voidp);
	    if(!stringlist) {
		*(m->var()) = Qnil;
		break;
	    }

	    VALUE av = rb_ary_new();
	    for(QStringList::Iterator it = stringlist->begin(); it != stringlist->end(); ++it) {
		VALUE rv = rstringFromQString(&(*it));
		rb_ary_push(av, rv);
	    }

	    if(m->cleanup())
		delete stringlist;

	    *(m->var()) = av;
	}
	break;
      default:
	m->unsupported();
	break;
    }
}

void marshall_QStrList(Marshall *m) {
    switch(m->action()) {
      case Marshall::FromVALUE: 
	{
	    VALUE list = *(m->var());
	    if (TYPE(list) != T_ARRAY) {
		m->item().s_voidp = 0;
		break;
	    }

	    int count = RARRAY(list)->len;
	    QStrList *stringlist = new QStrList;

	    for(long i = 0; i < count; i++) {
		VALUE item = rb_ary_entry(list, i);
		if(TYPE(item) != T_STRING) {
		    stringlist->append(QString());
		    continue;
		}
		stringlist->append(QString::fromUtf8(StringValuePtr(item), RSTRING(item)->len));
	    }

	    m->item().s_voidp = stringlist;
	    m->next();

	    if(m->cleanup()) {
		rb_ary_clear(list);
		for(QStrList::Iterator it = stringlist->begin(); it != stringlist->end(); ++it)
		    rb_ary_push(list, rb_str_new2(static_cast<const char *>(*it)));
		delete stringlist;
	    }
	    break;
      }
      case Marshall::ToVALUE: 
	{
	    QStrList *stringlist = static_cast<QStrList *>(m->item().s_voidp);
	    if(!stringlist) {
		*(m->var()) = Qnil;
		break;
	    }

	    VALUE av = rb_ary_new();
	    for(QStrList::Iterator it = stringlist->begin(); it != stringlist->end(); ++it) {
		VALUE rv = rb_str_new2(static_cast<const char *>(*it));
		rb_ary_push(av, rv);
	    }

	    if(m->cleanup())
		delete stringlist;

	    *(m->var()) = av;
	}
	break;
      default:
	m->unsupported();
	break;
    }
}

template <class Item, class ItemList, class ItemListIterator, const char *ItemSTR >
void marshall_ItemList(Marshall *m) {
    switch(m->action()) {
      case Marshall::FromVALUE:
	{
	    VALUE list = *(m->var());
	    if (TYPE(list) != T_ARRAY) {
		m->item().s_voidp = 0;
		break;
	    }
	    int count = RARRAY(list)->len;
	    ItemList *cpplist = new ItemList;
	    long i;
	    for(i = 0; i < count; i++) {
		VALUE item = rb_ary_entry(list, i);
                // TODO do type checking!
		smokeruby_object *o = value_obj_info(item);
		if(!o || !o->ptr)
                    continue;
		void *ptr = o->ptr;
		ptr = o->smoke->cast(
		    ptr,				// pointer
		    o->classId,				// from
		    o->smoke->idClass(ItemSTR)	        // to
		);
		cpplist->append((Item*)ptr);
	    }

	    m->item().s_voidp = cpplist;
	    m->next();

	    if(m->cleanup()) {
		rb_ary_clear(list);
		for(ItemListIterator it = cpplist->begin();
		    it != cpplist->end();
		    ++it) {
		    VALUE obj = getPointerObject((void*)(*it));
		    rb_ary_push(list, obj);
		}
		delete cpplist;
	    }
	}
	break;
      case Marshall::ToVALUE:
	{
	    ItemList *valuelist = (ItemList*)m->item().s_voidp;
	    if(!valuelist) {
		*(m->var()) = Qnil;
		break;
	    }

	    VALUE av = rb_ary_new();

	    int ix = m->smoke()->idClass(ItemSTR);
	    const char * className = m->smoke()->binding->className(ix);

	    for(ItemListIterator it = valuelist->begin();
		it != valuelist->end();
		++it) {
		void *p = *it;

		if(m->item().s_voidp == 0) {
		    *(m->var()) = Qnil;
		    break;
		}

		VALUE obj = getPointerObject(p);
		if(obj == Qnil) {
		    smokeruby_object  * o = ALLOC(smokeruby_object);
		    o->smoke = m->smoke();
		    o->classId = m->type().classId();
		    o->ptr = p;
		    o->allocated = false;
		    obj = set_obj_info(className, o);
		}
		rb_ary_push(av, obj);
            }

	    if(m->cleanup())
		delete valuelist;
	    else
	        *(m->var()) = av;
	}
	break;
      default:
	m->unsupported();
	break;
    }
}

void marshall_QValueListInt(Marshall *m) {
    switch(m->action()) {
      case Marshall::FromVALUE:
	{
	    VALUE list = *(m->var());
	    if (TYPE(list) != T_ARRAY) {
		m->item().s_voidp = 0;
		break;
	    }
	    int count = RARRAY(list)->len;
	    QValueList<int> *valuelist = new QValueList<int>;
	    long i;
	    for(i = 0; i < count; i++) {
		VALUE item = rb_ary_entry(list, i);
		if(TYPE(item) != T_FIXNUM && TYPE(item) != T_BIGNUM) {
		    valuelist->append(0);
		    continue;
		}
		valuelist->append(NUM2INT(item));
	    }

	    m->item().s_voidp = valuelist;
	    m->next();

	    if(m->cleanup()) {
		rb_ary_clear(list);
		for(QValueListIterator<int> it = valuelist->begin();
		    it != valuelist->end();
		    ++it)
		    rb_ary_push(list, INT2NUM((int)*it));
		delete valuelist;
	    }
	}
	break;
      case Marshall::ToVALUE:
	{
	    QValueList<int> *valuelist = (QValueList<int>*)m->item().s_voidp;
	    if(!valuelist) {
		*(m->var()) = Qnil;
		break;
	    }

	    VALUE av = rb_ary_new();

	    for(QValueListIterator<int> it = valuelist->begin();
		it != valuelist->end();
		++it)
		rb_ary_push(av, INT2NUM(*it));
		
	    *(m->var()) = av;
		
	    if(m->cleanup())
		delete valuelist;
	}
	break;
      default:
	m->unsupported();
	break;
    }
}

void marshall_voidP(Marshall *m) {
    switch(m->action()) {
      case Marshall::FromVALUE:
	{
	    VALUE rv = *(m->var());
	    if (rv != Qnil)
		m->item().s_voidp = (void*)NUM2INT(*(m->var()));
	    else
		m->item().s_voidp = 0;
	}
	break;
      case Marshall::ToVALUE:
	{
	    *(m->var()) = Data_Wrap_Struct(rb_cObject, 0, 0, m->item().s_voidp);
	}
	break;
      default:
	m->unsupported();
	break;
    }
}

void marshall_QMapQStringQString(Marshall *m) {
    switch(m->action()) {
      case Marshall::FromVALUE:
	{
	    VALUE hash = *(m->var());
	    if (TYPE(hash) != T_HASH) {
		m->item().s_voidp = 0;
		break;
	    }
		
		QMap<QString,QString> * map = new QMap<QString,QString>;
		
		// Convert the ruby hash to an array of key/value arrays
		VALUE temp = rb_funcall(hash, rb_intern("to_a"), 0);

		for (long i = 0; i < RARRAY(temp)->len; i++) {
			VALUE key = rb_ary_entry(rb_ary_entry(temp, i), 0);
			VALUE value = rb_ary_entry(rb_ary_entry(temp, i), 1);
			(*map)[QString(StringValuePtr(key))] = QString(StringValuePtr(value));
		}
	    
		m->item().s_voidp = map;
		m->next();
		
	    if(m->cleanup())
		delete map;
	}
	break;
      case Marshall::ToVALUE:
	{
	    QMap<QString,QString> *map = (QMap<QString,QString>*)m->item().s_voidp;
	    if(!map) {
		*(m->var()) = Qnil;
		break;
	    }
		
	    VALUE hv = rb_hash_new();
			
		QMap<QString,QString>::Iterator it;
		for (it = map->begin(); it != map->end(); ++it) {
			rb_hash_aset(hv, rstringFromQString((QString*)&(it.key())), rstringFromQString((QString*) &(it.data())));
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

void marshall_QMapQStringQVariant(Marshall *m) {
    switch(m->action()) {
      case Marshall::FromVALUE:
	{
	    VALUE hash = *(m->var());
	    if (TYPE(hash) != T_HASH) {
		m->item().s_voidp = 0;
		break;
	    }
		
		QMap<QString,QVariant> * map = new QMap<QString,QVariant>;
		
		// Convert the ruby hash to an array of key/value arrays
		VALUE temp = rb_funcall(hash, rb_intern("to_a"), 0);

		for (long i = 0; i < RARRAY(temp)->len; i++) {
			VALUE key = rb_ary_entry(rb_ary_entry(temp, i), 0);
			VALUE value = rb_ary_entry(rb_ary_entry(temp, i), 1);
			
			smokeruby_object *o = value_obj_info(value);
			if( !o || !o->ptr)
                   continue;
			void * ptr = o->ptr;
			ptr = o->smoke->cast(ptr, o->classId, o->smoke->idClass("QVariant"));
			
			(*map)[QString(StringValuePtr(key))] = (QVariant)*(QVariant*)ptr;
		}
	    
		m->item().s_voidp = map;
		m->next();
		
	    if(m->cleanup())
		delete map;
	}
	break;
      case Marshall::ToVALUE:
	{
	    QMap<QString,QVariant> *map = (QMap<QString,QVariant>*)m->item().s_voidp;
	    if(!map) {
		*(m->var()) = Qnil;
		break;
	    }
		
	    VALUE hv = rb_hash_new();
			
		QMap<QString,QVariant>::Iterator it;
		for (it = map->begin(); it != map->end(); ++it) {
			void *p = new QVariant(it.data());
			VALUE obj = getPointerObject(p);
				
			if (obj == Qnil) {
				smokeruby_object  * o = ALLOC(smokeruby_object);
				o->classId = m->smoke()->idClass("QVariant");
				o->smoke = m->smoke();
				o->ptr = p;
				o->allocated = true;
				obj = set_obj_info("Qt::Variant", o);
			}
			
			rb_hash_aset(hv, rstringFromQString((QString*)&(it.key())), obj);
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

void marshall_QUObject(Marshall *m) {
    switch(m->action()) {
      case Marshall::FromVALUE:
	{
	    VALUE array = *(m->var());
	    if (array != Qnil && TYPE(array) == T_ARRAY) {
		VALUE rv = rb_ary_entry(array, 0);
		Data_Get_Struct(rv, QUObject, m->item().s_voidp);
	    } else {
		m->item().s_voidp = 0;
		}
	}
	break;
      case Marshall::ToVALUE:
	{
	    VALUE rv = Data_Wrap_Struct(rb_cObject, 0, 0, m->item().s_voidp);
		VALUE array = rb_ary_new2(1);
		rb_ary_push(array, rv);
	    *(m->var()) = array;
	}
	break;
      default:
	m->unsupported();
	break;
    }
}

void marshall_QRgb_array(Marshall *m) {
    switch(m->action()) {
      case Marshall::FromVALUE:
	{
	    VALUE list = *(m->var());
	    if (TYPE(list) != T_ARRAY) {
		m->item().s_voidp = 0;
		break;
	    }
	    int count = RARRAY(list)->len;
	    QRgb *rgb = new QRgb[count + 2];
	    long i;
	    for(i = 0; i < count; i++) {
		VALUE item = rb_ary_entry(list, i);
		if(TYPE(item) != T_FIXNUM && TYPE(item) != T_BIGNUM) {
		    rgb[i] = 0;
		    continue;
		}

		rgb[i] = NUM2INT(item);
	    }
	    m->item().s_voidp = rgb;
	    m->next();
	    if(m->cleanup())
		delete[] rgb;
	}
	break;
      case Marshall::ToVALUE:
	// Implement this with a tied array or something
      default:
	m->unsupported();
	break;
    }
}

#define DEF_LIST_MARSHALLER(ListIdent,ItemList,Item,Itr) namespace { char ListIdent##STR[] = #Item; };  \
        Marshall::HandlerFn marshall_##ListIdent = marshall_ItemList<Item,ItemList,Itr,ListIdent##STR>;

#include <qcanvas.h>
#include <qdir.h>
#include <qobjectlist.h>
#include <qwidgetlist.h>
#include <qdockwindow.h>
#include <qnetworkprotocol.h>
#include <qtoolbar.h>
#include <qtabbar.h>

#if QT_VERSION >= 0x030200
DEF_LIST_MARSHALLER( QPtrListQNetworkOperation, QPtrList<QNetworkOperation>, QNetworkOperation, QPtrListStdIterator<QNetworkOperation> )
DEF_LIST_MARSHALLER( QPtrListQToolBar, QPtrList<QToolBar>, QToolBar, QPtrListStdIterator<QToolBar> )
DEF_LIST_MARSHALLER( QPtrListQTab, QPtrList<QTab>, QTab, QPtrListStdIterator<QTab> )
DEF_LIST_MARSHALLER( QPtrListQDockWindow, QPtrList<QDockWindow>, QDockWindow, QPtrListStdIterator<QDockWindow> )
DEF_LIST_MARSHALLER( QFileInfoList, QFileInfoList, QFileInfo, QFileInfoList::Iterator )
DEF_LIST_MARSHALLER( QObjectList, QObjectList, QObject, QPtrListStdIterator<QObject> )
DEF_LIST_MARSHALLER( QWidgetList, QWidgetList, QWidget, QPtrListStdIterator<QWidget> )
#endif

DEF_LIST_MARSHALLER( QCanvasItemList, QCanvasItemList, QCanvasItem, QValueListIterator<QCanvasItem*> )

template <class Item, class ItemList, class ItemListIterator, const char *ItemSTR >
void marshall_ValueItemList(Marshall *m) {
    switch(m->action()) {
      case Marshall::FromVALUE:
	{
	    VALUE list = *(m->var());
	    if (TYPE(list) != T_ARRAY) {
		m->item().s_voidp = 0;
		break;
	    }
	    int count = RARRAY(list)->len;
	    ItemList *cpplist = new ItemList;
	    long i;
	    for(i = 0; i < count; i++) {
		VALUE item = rb_ary_entry(list, i);
                // TODO do type checking!
		smokeruby_object *o = value_obj_info(item);
		if(!o || !o->ptr)
                    continue;
		void *ptr = o->ptr;
		ptr = o->smoke->cast(
		    ptr,				// pointer
		    o->classId,				// from
		    o->smoke->idClass(ItemSTR)	        // to
		);
		cpplist->append(*(Item*)ptr);
	    }

	    m->item().s_voidp = cpplist;
	    m->next();

	    if(m->cleanup()) {
		rb_ary_clear(list);
		for(ItemListIterator it = cpplist->begin();
		    it != cpplist->end();
		    ++it) {
		    VALUE obj = getPointerObject((void*)&(*it));
		    rb_ary_push(list, obj);
		}
		delete cpplist;
	    }
	}
	break;
      case Marshall::ToVALUE:
	{
	    ItemList *valuelist = (ItemList*)m->item().s_voidp;
	    if(!valuelist) {
		*(m->var()) = Qnil;
		break;
	    }

	    VALUE av = rb_ary_new();

	    int ix = m->smoke()->idClass(ItemSTR);
	    const char * className = m->smoke()->binding->className(ix);

	    for(ItemListIterator it = valuelist->begin();
		it != valuelist->end();
		++it) {
		void *p = &(*it);

		if(m->item().s_voidp == 0) {
		    *(m->var()) = Qnil;
		    break;
		}

		VALUE obj = getPointerObject(p);
		if(obj == Qnil) {
		    smokeruby_object  * o = ALLOC(smokeruby_object);
		    o->smoke = m->smoke();
		    o->classId = o->smoke->idClass(ItemSTR);
		    o->ptr = p;
		    o->allocated = false;
		    obj = set_obj_info(className, o);
		}
		rb_ary_push(av, obj);
            }

	    if(m->cleanup())
		delete valuelist;
	    else
	        *(m->var()) = av;
	}
	break;
      default:
	m->unsupported();
	break;
    }
}

#define DEF_VALUELIST_MARSHALLER(ListIdent,ItemList,Item,Itr) namespace { char ListIdent##STR[] = #Item; };  \
        Marshall::HandlerFn marshall_##ListIdent = marshall_ValueItemList<Item,ItemList,Itr,ListIdent##STR>;

DEF_VALUELIST_MARSHALLER( QVariantList, QValueList<QVariant>, QVariant, QValueList<QVariant>::Iterator )
DEF_VALUELIST_MARSHALLER( QPixmapList, QValueList<QPixmap>, QPixmap, QValueList<QPixmap>::Iterator )
DEF_VALUELIST_MARSHALLER( QIconDragItemList, QValueList<QIconDragItem>, QIconDragItem, QValueList<QIconDragItem>::Iterator )
DEF_VALUELIST_MARSHALLER( QImageTextKeyLangList, QValueList<QImageTextKeyLang>, QImageTextKeyLang, QValueList<QImageTextKeyLang>::Iterator )
DEF_VALUELIST_MARSHALLER( QUrlInfoList, QValueList<QUrlInfo>, QUrlInfo, QValueList<QUrlInfo>::Iterator )
DEF_VALUELIST_MARSHALLER( QTranslatorMessageList, QValueList<QTranslatorMessage>, QTranslatorMessage, QValueList<QTranslatorMessage>::Iterator )
DEF_VALUELIST_MARSHALLER( QHostAddressList, QValueList<QHostAddress>, QHostAddress, QValueList<QHostAddress>::Iterator )

TypeHandler Qt_handlers[] = {
    { "QString", marshall_QString },
    { "QString&", marshall_QString },
    { "QString*", marshall_QString },
    { "QCString", marshall_QCString },
    { "QCString&", marshall_QCString },
    { "QCString*", marshall_QCString },
    { "QStringList", marshall_QStringList },
    { "QStringList&", marshall_QStringList },
    { "QStringList*", marshall_QStringList },
    { "QStrList", marshall_QStrList },
    { "QStrList&", marshall_QStrList },
    { "QStrList*", marshall_QStrList },
    { "long long int", marshall_longlong },
    { "long long int&", marshall_longlong },
    { "Q_INT64", marshall_longlong },
    { "unsigned long long int", marshall_ulonglong },
    { "unsigned long long int&", marshall_ulonglong },
    { "Q_UINT64", marshall_ulonglong },
    { "int&", marshall_intR },
    { "int*", marshall_intR },
    { "bool&", marshall_boolR },
    { "bool*", marshall_boolR },
    { "char*", marshall_charP },
    { "char**", marshall_charP_array },
    { "uchar*", marshall_ucharP },
    { "QRgb*", marshall_QRgb_array },
    { "QUObject*", marshall_QUObject },
    { "const QCOORD*", marshall_QCOORD_array },
    { "void", marshall_void },
    { "QByteArray", marshall_QByteArray },
    { "QByteArray&", marshall_QByteArray },
    { "QValueList<int>", marshall_QValueListInt },
    { "QValueList<int>&", marshall_QValueListInt },
    { "QValueList<QVariant>", marshall_QVariantList },
    { "QValueList<QVariant>&", marshall_QVariantList },
    { "QValueList<QPixmap>", marshall_QPixmapList },
    { "QValueList<QIconDragItem>&", marshall_QIconDragItemList },
    { "QValueList<QImageTextKeyLang>", marshall_QImageTextKeyLangList },
    { "QValueList<QUrlInfo>&", marshall_QUrlInfoList },
    { "QValueList<QTranslatorMessage>", marshall_QTranslatorMessageList },
    { "QValueList<QHostAddress>", marshall_QHostAddressList },
    { "QCanvasItemList", marshall_QCanvasItemList },
    { "QMap<QString,QString>", marshall_QMapQStringQString },
    { "QMap<QString,QString>&", marshall_QMapQStringQString },
    { "QMap<QString,QVariant>", marshall_QMapQStringQVariant },
    { "QMap<QString,QVariant>&", marshall_QMapQStringQVariant },
#if QT_VERSION >= 0x030200
    { "QWidgetList", marshall_QWidgetList },
    { "QWidgetList*", marshall_QWidgetList },
    { "QWidgetList&", marshall_QWidgetList },
    { "QObjectList*", marshall_QObjectList },
    { "QObjectList&", marshall_QObjectList },
    { "QFileInfoList*", marshall_QFileInfoList },
    { "QPtrList<QToolBar>", marshall_QPtrListQToolBar },
    { "QPtrList<QTab>*", marshall_QPtrListQTab },
    { "QPtrList<QDockWindow>", marshall_QPtrListQDockWindow },
    { "QPtrList<QDockWindow>*", marshall_QPtrListQDockWindow },
    { "QPtrList<QNetworkOperation>", marshall_QPtrListQNetworkOperation },
    { "QPtrList<QNetworkOperation>&", marshall_QPtrListQNetworkOperation },
#endif
    { 0, 0 }
};

QAsciiDict<TypeHandler> type_handlers(199);

void install_handlers(TypeHandler *h) {
    while(h->name) {
	type_handlers.insert(h->name, h);
	h++;
    }
}

Marshall::HandlerFn getMarshallFn(const SmokeType &type) {
    if(type.elem())
	return marshall_basetype;
    if(!type.name())
	return marshall_void;
	TypeHandler *h = type_handlers[type.name()];
    if(h == 0 && type.isConst() && strlen(type.name()) > strlen("const ")) {
    	h = type_handlers[type.name() + strlen("const ")];
    }
	
    if(h != 0) {
	return h->fn;
    }

    return marshall_unknown;
}
