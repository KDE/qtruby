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
#include <qpainter.h>
#include <qpalette.h>
#include <qlistwidget.h>
#include <qtablewidget.h>
#include <qtoolbar.h>
#include <qdockwidget.h>
#include <qurlinfo.h>
#include <qlayout.h>
#include <qmetaobject.h>
#include <qlinkedlist.h>
#include <qobject.h>
#include <qtextcodec.h>
#include <qhostaddress.h>
#include <qpair.h>
#include <qevent.h>
#include <qpixmap.h>
#include <qaction.h>
#include <qtreewidget.h>
#include <qtextobject.h>
#include <qtextlayout.h>
#include <qabstractbutton.h>
#include <qlistwidget.h>
#include <qtablewidget.h>
#include <qpolygon.h>
#include <qurl.h>
#include <qdir.h>
#include <qobject.h>
#include <qwidget.h>
#include <qtabbar.h>


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
extern bool application_terminated;
};

extern bool isDerivedFromByName(Smoke *smoke, const char *className, const char *baseClassName);
extern void mapPointer(VALUE obj, smokeruby_object *o, Smoke::Index classId, void *lastptr);

static const char * (*_kde_resolve_classname)(Smoke*, int, void*) = 0;

extern "C" {

void
set_kde_resolve_classname(const char * (*kde_resolve_classname) (Smoke*, int, void *))
{
	_kde_resolve_classname = kde_resolve_classname;
}

};

void
mark_qobject_children(QObject * qobject)
{
	VALUE obj;
	
	const QList<QObject*> l = qobject->children();
	
	if (l.count() == 0) {
		return;
	}

	QObject *child;

	for (int i=0; i < l.size(); ++i) {
		child = l.at(i);
		obj = getPointerObject(child);
		if (obj != Qnil) {
			if(do_debug & qtdb_gc) printf("Marking (%s*)%p -> %p\n", child->metaObject()->className(), child, (void*)obj);
			rb_gc_mark(obj);
		}
		
		mark_qobject_children(child);
	}
}

void
smokeruby_mark(void * p)
{
	VALUE obj;
    smokeruby_object * o = (smokeruby_object *) p;
    const char *className = o->smoke->classes[o->classId].className;
	
	if(do_debug & qtdb_gc) printf("Checking for mark (%s*)%p\n", className, o->ptr);
		
    if(o->ptr && o->allocated) {
/*
		if (isDerivedFromByName(o->smoke, className, "QListWidget")) {
			QListWidget * listwidget = (QListWidget *) o->smoke->cast(o->ptr, o->classId, o->smoke->idClass("QListWidget"));
			QListViewItemIterator it(listwidget);
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
*/		
		if (isDerivedFromByName(o->smoke, className, "QTableWidget")) {
			QTableWidget * table = (QTableWidget *) o->smoke->cast(o->ptr, o->classId, o->smoke->idClass("QTableWidget"));
			QTableWidgetItem *item;

			for ( int row = 0; row < table->rowCount(); row++ ) {
				for ( int col = 0; col < table->columnCount(); col++ ) {
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
			mark_qobject_children(qobject);
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
    
	if(application_terminated || !o->allocated || o->ptr == 0) {
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
//	} else if (strcmp(className, "QIconViewItem") == 0) {
//		Q3IconViewItem * item = (Q3IconViewItem *) o->ptr;
//		if (item->iconView() != 0) {
//			free(o);
//			return;
//		}
//	} else if (strcmp(className, "QCheckListItem") == 0) {
//		Q3CheckListItem * item = (Q3CheckListItem *) o->ptr;
//		if (item->parent() != 0 || item->listView() != 0) {
//			free(o);
//			return;
//		}
	} else if (strcmp(className, "QListWidgetItem") == 0) {
		QListWidgetItem * item = (QListWidgetItem *) o->ptr;
		if (item->listWidget() != 0) {
			free(o);
			return;
		}
	} else if (isDerivedFromByName(o->smoke, className, "QTableWidgetItem")) {
		QTableWidgetItem * item = (QTableWidgetItem *) o->smoke->cast(o->ptr, o->classId, o->smoke->idClass("QTableWidgetItem"));
		if (item->tableWidget() != 0) {
			free(o);
			return;
		}
//	} else if (strcmp(className, "QPopupMenu") == 0) {
//		Q3PopupMenu * item = (Q3PopupMenu *) o->ptr;
//		if (item->parentWidget(false) != 0) {
//			free(o);
//			return;
//		}
	} else if (isDerivedFromByName(o->smoke, className, "QWidget")) {
		QWidget * qwidget = (QWidget *) o->smoke->cast(o->ptr, o->classId, o->smoke->idClass("QWidget"));
		if (qwidget->parentWidget() != 0) {
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
resolve_classname(Smoke* smoke, int classId, void * ptr)
{
	if (isDerivedFromByName(smoke, smoke->classes[classId].className, "QEvent")) {
		QEvent * qevent = (QEvent *) smoke->cast(ptr, classId, smoke->idClass("QEvent"));
		switch (qevent->type()) {
		case QEvent::ChildAdded:
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
		case QEvent::InputMethod:
			return "Qt::IMEvent";
//		case QEvent::InputMethodStart:
//		case QEvent::InputMethodCompose:
//		case QEvent::InputMethodEnd:
//			return "Qt::IMEvent";
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
	} else if (isDerivedFromByName(smoke, smoke->classes[classId].className, "QObject")) {
		QObject * qobject = (QObject *) smoke->cast(ptr, classId, smoke->idClass("QObject"));
		const QMetaObject * meta = qobject->metaObject();

		while (meta != 0) {
			Smoke::Index classId = smoke->idClass(meta->className());
			if (classId != 0) {
				return smoke->binding->className(classId);
			}

			meta = meta->superClass();
		}
	} else if (isDerivedFromByName(smoke, smoke->classes[classId].className, "QListWidgetItem")) {
		QListWidgetItem * item = (QListWidgetItem *) smoke->cast(ptr, classId, smoke->idClass("QListWidgetItem"));
		switch (item->type()) {
		case 0:
			return "Qt::ListWidgetItem";
		default:
			return "Qt::ListWidgetItem";
			break;
		}
	} else if (isDerivedFromByName(smoke, smoke->classes[classId].className, "QTableWidgetItem")) {
		QTableWidgetItem * item = (QTableWidgetItem *) smoke->cast(ptr, classId, smoke->idClass("QTableWidgetItem"));
		switch (item->type()) {
		case 0:
			return "Qt::TableWidgetItem";
		default:
			return "Qt::TableWidgetItem";
			break;
		}
	}
	
	if (_kde_resolve_classname != 0) {
		return (*_kde_resolve_classname)(smoke, classId, ptr);
	}
	
	return smoke->binding->className(classId);
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
					m->item().s_ushort = (unsigned short) NUM2UINT(*(m->var()));
				break;
				
				case Marshall::ToVALUE:
					*(m->var()) = UINT2NUM(m->item().s_ushort);
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
					*(m->var()) = UINT2NUM(m->item().s_uint);
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
					*(m->var()) = rb_funcall(qt_internal_module, rb_intern("create_qenum"), 
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
					if(*(m->var()) == Qnil) {
						m->item().s_class = 0;
						break;
					}
				
					if(TYPE(*(m->var())) != T_DATA) {
						rb_raise(rb_eArgError, "Invalid type, expecting %s\n", m->type().name());
						break;
					}
		
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

				const char * classname = resolve_classname(o->smoke, o->classId, o->ptr);
		
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
	
	if (strcmp(KCODE, "UTF8") == 0)
		return new QString(QString::fromUtf8(StringValuePtr(rstring), RSTRING(rstring)->len));
	else if (strcmp(KCODE, "EUC") == 0)
		return new QString(codec->toUnicode(StringValuePtr(rstring)));
	else if (strcmp(KCODE, "SJIS") == 0)
		return new QString(codec->toUnicode(StringValuePtr(rstring)));
	else if(strcmp(KCODE, "NONE") == 0)
		return new QString(QString::fromLatin1(StringValuePtr(rstring)));

	return new QString(QString::fromLocal8Bit(StringValuePtr(rstring), RSTRING(rstring)->len));
}

VALUE 
rstringFromQString(QString * s) {
	if (KCODE == 0) {
		init_codec();
	}
	
	if (strcmp(KCODE, "UTF8") == 0)
		return rb_str_new2(s->toUtf8());
	else if (strcmp(KCODE, "EUC") == 0)
		return rb_str_new2(codec->fromUnicode(*s));
	else if (strcmp(KCODE, "SJIS") == 0)
		return rb_str_new2(codec->fromUnicode(*s));
	else if (strcmp(KCODE, "NONE") == 0)
		return rb_str_new2(s->toLatin1());
	else
		return rb_str_new2(s->toLocal8Bit());
}

static void marshall_QString(Marshall *m) {
	switch(m->action()) {
		case Marshall::FromVALUE:
		{
			QString* s = 0;
			if( *(m->var()) != Qnil) {
				s = qstringFromRString(*(m->var()));
			} else {
				s = new QString();
			}

			m->item().s_voidp = s;
			m->next();
		
			if (!m->type().isConst() && *(m->var()) != Qnil && s != 0 && !s->isNull()) {
				rb_str_resize(*(m->var()), 0);
				VALUE temp = rstringFromQString(s);
				rb_str_cat2(*(m->var()), StringValuePtr(temp));
			}
	
			if(s && m->type().isConst() && m->cleanup())
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
						s = new QByteArray(RSTRING(data)->len, '\0');
						memcpy((void*)s->data(), StringValuePtr(data), RSTRING(data)->len);
					}
				} else {
					// Ordinary ruby String - use the contents of the string
					s = new QByteArray(RSTRING(rv)->len, '\0');
					memcpy((void*)s->data(), StringValuePtr(rv), RSTRING(rv)->len);
				}
			} else {
				s = new QByteArray(0, '\0');
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
				result = rb_funcall(qt_internal_module, rb_intern("create_qbytearray"), 
					2, string, Data_Wrap_Struct(rb_cObject, 0, 0, s));
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

/*
static void marshall_QCString(Marshall *m) {
    switch(m->action()) {
      case Marshall::FromVALUE:
	{
	    Q3CString *s = 0;
	    VALUE rv = *(m->var());
	    if (rv == Qnil) {
		s = new Q3CString(); 
        } else {
		// Add 1 to the ruby string length to allow for a QCString '\0' terminator
		s = new Q3CString(StringValuePtr(*(m->var())), RSTRING(*(m->var()))->len + 1); 
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
	    Q3CString *s = (Q3CString*)m->item().s_voidp;
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
*/

static void marshall_longlong(Marshall *m) {
    switch(m->action()) {
      case Marshall::FromVALUE:
	{
	    m->item().s_voidp = new long long;
	    *(long long *)m->item().s_voidp = rb_num2ll(*(m->var()));
		
	    m->next();
		
	    if(m->cleanup() && m->type().isConst()) {
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
		
	    if(m->cleanup() && m->type().isConst()) {
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
		int * i = new int;
		if (TYPE(rv) == T_OBJECT) {
			// A Qt::Integer has been passed as an integer value
			VALUE temp = rb_funcall(qt_internal_module, rb_intern("get_qinteger"), 1, rv);
			*i = NUM2INT(temp);
			m->item().s_voidp = i;
			m->next();
			rb_funcall(qt_internal_module, rb_intern("set_qinteger"), 2, rv, INT2NUM(*i));
			rv = temp;
		} else {
			*i = NUM2INT(rv);
			m->item().s_voidp = i;
			m->next();
		}
	    if(m->cleanup() && m->type().isConst()) {
			delete i;
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
		bool * b = new bool;
		if (TYPE(rv) == T_OBJECT) {
			// A Qt::Boolean has been passed as a value
			VALUE temp = rb_funcall(qt_internal_module, rb_intern("get_qboolean"), 1, rv);
			*b = (temp == Qtrue ? true : false);
			m->item().s_voidp = b;
			m->next();
			rb_funcall(qt_internal_module, rb_intern("set_qboolean"), 2, rv, (*b ? Qtrue : Qfalse));
		} else {
			*b = (rv == Qtrue ? true : false);
			m->item().s_voidp = b;
			m->next();
		}
	    	if(m->cleanup() && m->type().isConst()) {
			delete b;
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

			if (stringlist != 0 && !m->type().isConst()) {
				rb_ary_clear(list);
				for(QStringList::Iterator it = stringlist->begin(); it != stringlist->end(); ++it)
				rb_ary_push(list, rstringFromQString(&(*it)));
			}
			
			if (stringlist != 0 && m->type().isConst() && m->cleanup())
				delete stringlist;
	   
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


void marshall_QByteArrayList(Marshall *m) {
    switch(m->action()) {
      case Marshall::FromVALUE: 
	{
	    VALUE list = *(m->var());
	    if (TYPE(list) != T_ARRAY) {
		m->item().s_voidp = 0;
		break;
	    }

	    int count = RARRAY(list)->len;
	    QList<QByteArray> *stringlist = new QList<QByteArray>;

	    for(long i = 0; i < count; i++) {
		VALUE item = rb_ary_entry(list, i);
		if(TYPE(item) != T_STRING) {
		    stringlist->append(QByteArray());
		    continue;
		}
		stringlist->append(QByteArray(StringValuePtr(item), RSTRING(item)->len));
	    }

	    m->item().s_voidp = stringlist;
	    m->next();

	    if(m->cleanup()) {
		rb_ary_clear(list);
		for (int i = 0; i < stringlist->size(); i++) {
		    rb_ary_push(list, rb_str_new2((const char *) stringlist->at(i)));
		}
		delete stringlist;
	    }
	    break;
      }
      case Marshall::ToVALUE: 
	{
	    QList<QByteArray> *stringlist = static_cast<QList<QByteArray>*>(m->item().s_voidp);
	    if(!stringlist) {
		*(m->var()) = Qnil;
		break;
	    }

	    VALUE av = rb_ary_new();
		for (int i = 0; i < stringlist->size(); i++) {
			VALUE rv = rb_str_new2((const char *) stringlist->at(i));
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


template <class Item, class ItemList, const char *ItemSTR >
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
		    		o->smoke->idClass(ItemSTR)	// to
				);
				cpplist->append((Item*)ptr);
			}

			m->item().s_voidp = cpplist;
			m->next();

			if(m->cleanup()) {
			rb_ary_clear(list);
	
//				for(ItemListIterator it = cpplist->begin();
//					it != cpplist->end();
//					++it) {
//					VALUE obj = getPointerObject((void*)*it);
				for(int i = 0; i < cpplist->size(); ++i ) {
					VALUE obj = getPointerObject( cpplist->at(i) );
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

			//for (ItemListIterator it = valuelist->begin();
			//	it != valuelist->end();
			//	++it) {
			//	void *p = *it;
			for(int i=0;i<valuelist->size();++i) {
				void *p = valuelist->at(i);

				if(m->item().s_voidp == 0) {
					*(m->var()) = Qnil;
					break;
				}

				VALUE obj = getPointerObject(p);
				if(obj == Qnil) {
					smokeruby_object  * o = ALLOC(smokeruby_object);
					o->smoke = m->smoke();
					o->classId = m->smoke()->idClass(ItemSTR);
					o->ptr = p;
					o->allocated = false;
					obj = set_obj_info(resolve_classname(o->smoke, o->classId, o->ptr), o);
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

void marshall_QListInt(Marshall *m) {
    switch(m->action()) {
      case Marshall::FromVALUE:
	{
	    VALUE list = *(m->var());
	    if (TYPE(list) != T_ARRAY) {
		m->item().s_voidp = 0;
		break;
	    }
	    int count = RARRAY(list)->len;
	    QList<int> *valuelist = new QList<int>;
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

		for (	QList<int>::iterator i = valuelist->begin(); 
				i != valuelist->end(); 
				++i ) 
		{
		    rb_ary_push(list, INT2NUM((int)*i));
		}
		delete valuelist;
	    }
	}
	break;
      case Marshall::ToVALUE:
	{
	    QList<int> *valuelist = (QList<int>*)m->item().s_voidp;
	    if(!valuelist) {
		*(m->var()) = Qnil;
		break;
	    }

	    VALUE av = rb_ary_new();

		for (	QList<int>::iterator i = valuelist->begin(); 
				i != valuelist->end(); 
				++i ) 
		{
		    rb_ary_push(av, INT2NUM((int)*i));
		}
		
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

void marshall_QListqreal(Marshall *m) {
    switch(m->action()) {
      case Marshall::FromVALUE:
	{
	    VALUE list = *(m->var());
	    if (TYPE(list) != T_ARRAY) {
		m->item().s_voidp = 0;
		break;
	    }
	    int count = RARRAY(list)->len;
	    QList<qreal> *valuelist = new QList<qreal>;
	    long i;
	    for(i = 0; i < count; i++) {
		VALUE item = rb_ary_entry(list, i);
		if(TYPE(item) != T_FLOAT) {
		    valuelist->append(0.0);
		    continue;
		}
		valuelist->append(NUM2DBL(item));
	    }

	    m->item().s_voidp = valuelist;
	    m->next();

	    if(m->cleanup()) {
		rb_ary_clear(list);

		for (	QList<qreal>::iterator i = valuelist->begin(); 
				i != valuelist->end(); 
				++i ) 
		{
		    rb_ary_push(list, rb_float_new((qreal)*i));
		}
		delete valuelist;
	    }
	}
	break;
      case Marshall::ToVALUE:
	{
	    QList<qreal> *valuelist = (QList<qreal>*)m->item().s_voidp;
	    if(!valuelist) {
		*(m->var()) = Qnil;
		break;
	    }

	    VALUE av = rb_ary_new();

		for (	QList<qreal>::iterator i = valuelist->begin(); 
				i != valuelist->end(); 
				++i ) 
		{
		    rb_ary_push(av, rb_float_new((qreal)*i));
		}
		
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
			rb_hash_aset(hv, rstringFromQString((QString*)&(it.key())), rstringFromQString((QString*) &(it.value())));
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
			void *p = new QVariant(it.value());
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

void marshall_QMapintQVariant(Marshall *m) {
    switch(m->action()) {
      case Marshall::FromVALUE:
	{
	    VALUE hash = *(m->var());
	    if (TYPE(hash) != T_HASH) {
		m->item().s_voidp = 0;
		break;
	    }
		
		QMap<int,QVariant> * map = new QMap<int,QVariant>;
		
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
			
			(*map)[NUM2INT(key)] = (QVariant)*(QVariant*)ptr;
		}
	    
		m->item().s_voidp = map;
		m->next();
		
	    if(m->cleanup())
		delete map;
	}
	break;
      case Marshall::ToVALUE:
	{
	    QMap<int,QVariant> *map = (QMap<int,QVariant>*)m->item().s_voidp;
	    if(!map) {
		*(m->var()) = Qnil;
		break;
	    }
		
	    VALUE hv = rb_hash_new();
			
		QMap<int,QVariant>::Iterator it;
		for (it = map->begin(); it != map->end(); ++it) {
			void *p = new QVariant(it.value());
			VALUE obj = getPointerObject(p);
				
			if (obj == Qnil) {
				smokeruby_object  * o = ALLOC(smokeruby_object);
				o->classId = m->smoke()->idClass("QVariant");
				o->smoke = m->smoke();
				o->ptr = p;
				o->allocated = true;
				obj = set_obj_info("Qt::Variant", o);
			}
			
			rb_hash_aset(hv, INT2NUM((int)(it.key())), obj);
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

void marshall_voidP_array(Marshall *m) {
    switch(m->action()) {
	case Marshall::FromVALUE:
	{
	    VALUE rv = *(m->var());
		if (rv != Qnil) {
			Data_Get_Struct(rv, void*, m->item().s_voidp);
		} else {
			m->item().s_voidp = 0;
		}
	}
	break;
	case Marshall::ToVALUE:
	{
		VALUE rv = Data_Wrap_Struct(rb_cObject, 0, 0, m->item().s_voidp);
		*(m->var()) = rv;
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

		rgb[i] = NUM2UINT(item);
	    }
	    m->item().s_voidp = rgb;
	    m->next();
	}
	break;
      case Marshall::ToVALUE:
	// Implement this with a tied array or something
      default:
	m->unsupported();
	break;
    }
}

void marshall_QPairintint(Marshall *m) {
    switch(m->action()) {
      case Marshall::FromVALUE:
	{
	    VALUE list = *(m->var());
	    if (TYPE(list) != T_ARRAY || RARRAY(list)->len != 2) {
		m->item().s_voidp = 0;
		break;
	    }
		int int0;
		int int1;
		VALUE item = rb_ary_entry(list, 0);
		if(TYPE(item) != T_FIXNUM && TYPE(item) != T_BIGNUM) {
		    int0 = 0;
		} else {
			int0 = NUM2INT(item);
		}
		
		item = rb_ary_entry(list, 1);
		if(TYPE(item) != T_FIXNUM && TYPE(item) != T_BIGNUM) {
		    int1 = 0;
		} else {
			int1 = NUM2INT(item);
		}
		
		QPair<int,int> * qpair = new QPair<int,int>(int0,int1);
	    m->item().s_voidp = qpair;
	    m->next();
	    if(m->cleanup())
		delete qpair;
	}
	break;
      case Marshall::ToVALUE:
      default:
	m->unsupported();
	break;
    }
}

#define DEF_LIST_MARSHALLER(ListIdent,ItemList,Item) namespace { char ListIdent##STR[] = #Item; };  \
        Marshall::HandlerFn marshall_##ListIdent = marshall_ItemList<Item,ItemList,ListIdent##STR>;

DEF_LIST_MARSHALLER( QAbstractButtonList, QList<QAbstractButton*>, QAbstractButton )
DEF_LIST_MARSHALLER( QListWidgetItemList, QList<QListWidgetItem*>, QListWidgetItem )
DEF_LIST_MARSHALLER( QTableWidgetItemList, QList<QTableWidgetItem*>, QTableWidgetItem )
DEF_LIST_MARSHALLER( QObjectList, QList<QObject*>, QObject )
DEF_LIST_MARSHALLER( QWidgetList, QList<QWidget*>, QWidget )
DEF_LIST_MARSHALLER( QActionList, QList<QAction*>, QAction )
DEF_LIST_MARSHALLER( QTextFrameList, QList<QTextFrame*>, QTextFrame )
DEF_LIST_MARSHALLER( QTreeWidgetItemList, QList<QTreeWidgetItem*>, QTreeWidgetItem )

template <class Item, class ItemList, const char *ItemSTR >
void marshall_ValueListItem(Marshall *m) {
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
				for(int i=0; i < cpplist->size(); ++i) {
					VALUE obj = getPointerObject((void*)&(cpplist[i]));
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

			for(int i=0; i < valuelist->size() ; ++i) {
				void *p = &valuelist[i];

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

#define DEF_VALUELIST_MARSHALLER(ListIdent,ItemList,Item) namespace { char ListIdent##STR[] = #Item; };  \
        Marshall::HandlerFn marshall_##ListIdent = marshall_ValueListItem<Item,ItemList,ListIdent##STR>;

DEF_VALUELIST_MARSHALLER( QTableWidgetSelectionRangeList, QList<QTableWidgetSelectionRange>, QTableWidgetSelectionRange )
DEF_VALUELIST_MARSHALLER( QTextLayoutFormatRangeList, QList<QTextLayout::FormatRange>, QTextLayout::FormatRange)
DEF_VALUELIST_MARSHALLER( QVariantList, QList<QVariant>, QVariant )
DEF_VALUELIST_MARSHALLER( QPixmapList, QList<QPixmap>, QPixmap )
DEF_VALUELIST_MARSHALLER( QHostAddressList, QList<QHostAddress>, QHostAddress )
DEF_VALUELIST_MARSHALLER( QPolygonFList, QList<QPolygonF>, QPolygonF )
DEF_VALUELIST_MARSHALLER( QImageTextKeyLangList, QLinkedList<QImageTextKeyLang>, QImageTextKeyLang )
DEF_VALUELIST_MARSHALLER( QUrlList, QList<QUrl>, QUrl )
DEF_VALUELIST_MARSHALLER( QFileInfoList, QFileInfoList, QFileInfo )
DEF_VALUELIST_MARSHALLER( QTextBlockList, QList<QTextBlock>, QTextBlock )

TypeHandler Qt_handlers[] = {
    { "QString", marshall_QString },
    { "QString&", marshall_QString },
    { "QString*", marshall_QString },
//    { "QCString", marshall_QCString },
//    { "QCString&", marshall_QCString },
//    { "QCString*", marshall_QCString },
    { "QStringList", marshall_QStringList },
    { "QStringList&", marshall_QStringList },
    { "QStringList*", marshall_QStringList },
    { "QList<QByteArray>", marshall_QByteArrayList },
    { "QList<QByteArray>&", marshall_QByteArrayList },
    { "QList<QByteArray>*", marshall_QByteArrayList },
    { "long long int", marshall_longlong },
    { "long long int&", marshall_longlong },
    { "Q_INT64", marshall_longlong },
    { "Q_INT64&", marshall_longlong },
    { "Q_LLONG", marshall_longlong },
    { "Q_LLONG&", marshall_longlong },
    { "KIO::filesize_t", marshall_longlong },
    { "DOM::DOMTimeStamp", marshall_ulonglong },
    { "unsigned long long int", marshall_ulonglong },
    { "unsigned long long int&", marshall_ulonglong },
    { "Q_UINT64", marshall_ulonglong },
    { "Q_UINT64&", marshall_ulonglong },
    { "Q_ULLONG", marshall_ulonglong },
    { "Q_ULLONG&", marshall_ulonglong },
    { "int&", marshall_intR },
    { "int*", marshall_intR },
    { "bool&", marshall_boolR },
    { "bool*", marshall_boolR },
    { "char*", marshall_charP },
    { "char**", marshall_charP_array },
    { "uchar*", marshall_ucharP },
    { "QRgb*", marshall_QRgb_array },
    { "QPair<int,int>&", marshall_QPairintint },
    { "void**", marshall_voidP_array },
    { "void", marshall_void },
    { "QByteArray", marshall_QByteArray },
    { "QByteArray&", marshall_QByteArray },
    { "QList<qreal>", marshall_QListqreal },
    { "QList<int>", marshall_QListInt },
    { "QList<int>&", marshall_QListInt },
    { "QList<QVariant>", marshall_QVariantList },
    { "QList<QTableWidgetSelectionRange>", marshall_QTableWidgetSelectionRangeList },
    { "QList<QTextLayout::FormatRange>", marshall_QTextLayoutFormatRangeList },
    { "QList<QTextLayout::FormatRange>&", marshall_QTextLayoutFormatRangeList },
    { "QList<QTextBlock>", marshall_QTextBlockList },
    { "QList<QPolygonF>", marshall_QPolygonFList },
    { "QList<QHostAddress>", marshall_QHostAddressList },
    { "QList<QHostAddress>&", marshall_QHostAddressList },
    { "QList<QVariant>", marshall_QVariantList },
    { "QList<QVariant>&", marshall_QVariantList },
    { "QList<QPixmap>", marshall_QPixmapList },
    { "QValueList<QImageTextKeyLang>", marshall_QImageTextKeyLangList },
    { "QList<QUrl>", marshall_QUrlList },
    { "QList<QUrl>&", marshall_QUrlList },
    { "QMap<int,QVariant>", marshall_QMapintQVariant },
    { "QMap<QString,QString>", marshall_QMapQStringQString },
    { "QMap<QString,QString>&", marshall_QMapQStringQString },
    { "QMap<QString,QVariant>", marshall_QMapQStringQVariant },
    { "QMap<QString,QVariant>&", marshall_QMapQStringQVariant },
    { "QList<QTextFrame*>", marshall_QTextFrameList },
    { "QList<QAction*>", marshall_QActionList },
    { "QList<QTreeWidgetItem*>", marshall_QActionList },
    { "QList<QAbstractButton*>", marshall_QAbstractButtonList },
    { "QList<QListWidgetItem*>", marshall_QListWidgetItemList },
    { "QList<QTableWidgetItem*>", marshall_QTableWidgetItemList },
    { "QWidgetList", marshall_QWidgetList },
    { "QWidgetList&", marshall_QWidgetList },
    { "QObjectList", marshall_QObjectList },
    { "QObjectList&", marshall_QObjectList },
    { "QFileInfoList", marshall_QFileInfoList },
    { 0, 0 }
};

#include <qhash.h>
QHash<QString, TypeHandler*> type_handlers;

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
