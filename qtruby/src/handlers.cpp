/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QtCore/qdir.h>
#include <QtCore/qhash.h>
#include <QtCore/qlinkedlist.h>
#include <QtCore/qmetaobject.h>
#include <QtCore/qobject.h>
#include <QtCore/qpair.h>
#include <QtCore/qprocess.h>
#include <QtCore/qregexp.h>
#include <QtCore/qstring.h>
#include <QtCore/qtextcodec.h>
#include <QtCore/qurl.h>
#include <QtGui/qabstractbutton.h>
#include <QtGui/qaction.h>
#include <QtGui/qapplication.h>
#include <QtGui/qdockwidget.h>
#include <QtGui/qevent.h>
#include <QtGui/qlayout.h>
#include <QtGui/qlistwidget.h>
#include <QtGui/qpainter.h>
#include <QtGui/qpalette.h>
#include <QtGui/qpixmap.h>
#include <QtGui/qpolygon.h>
#include <QtGui/qtabbar.h>
#include <QtGui/qtablewidget.h>
#include <QtGui/qtextedit.h>
#include <QtGui/qtextlayout.h>
#include <QtGui/qtextobject.h>
#include <QtGui/qtoolbar.h>
#include <QtGui/qtreewidget.h>
#include <QtGui/qwidget.h>
#include <QtNetwork/qhostaddress.h>
#include <QtNetwork/qnetworkinterface.h>
#include <QtNetwork/qurlinfo.h>


#if QT_VERSION >= 0x40200
#include <QtGui/qgraphicsitem.h>
#include <QtGui/qgraphicsscene.h>
#include <QtGui/qstandarditemmodel.h>
#include <QtGui/qundostack.h>
#endif

#if QT_VERSION >= 0x40300
#include <QtGui/qmdisubwindow.h>
#include <QtNetwork/qsslcertificate.h>
#include <QtNetwork/qsslcipher.h>
#include <QtNetwork/qsslerror.h>
#include <QtXml/qxmlstream.h>
#endif

#if QT_VERSION >= 0x040400
#include <QtGui/qprinterinfo.h>
#include <QtNetwork/qnetworkcookie.h>
#endif

#include <smoke/smoke.h>

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
extern VALUE qvariant_class;
extern bool application_terminated;
}

extern void mapPointer(VALUE obj, smokeruby_object *o, Smoke::Index classId, void *lastptr);

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
			if(do_debug & qtdb_gc) qWarning("Marking (%s*)%p -> %p", child->metaObject()->className(), child, (void*)obj);
			rb_gc_mark(obj);
		}
		
		mark_qobject_children(child);
	}
}

void
mark_qtreewidgetitem_children(QTreeWidgetItem * item)
{
	VALUE obj;
	QTreeWidgetItem *child;

	for (int i = 0; i < item->childCount(); i++) {
		child = item->child(i);
		obj = getPointerObject(child);
		if (obj != Qnil) {
			if(do_debug & qtdb_gc) qWarning("Marking (%s*)%p -> %p", "QTreeWidgetItem", child, (void*)obj);
			rb_gc_mark(obj);
		}
		
		mark_qtreewidgetitem_children(child);
	}
}

void
mark_qstandarditem_children(QStandardItem * item)
{
	VALUE obj;

	for (int row = 0; row < item->rowCount(); row++) {
		for (int column = 0; column < item->columnCount(); column++) {
			QStandardItem * child = item->child(row, column);
			if (child != 0) {
				if (child->hasChildren()) {
					mark_qstandarditem_children(child);
				}
				obj = getPointerObject(child);
				if (obj != Qnil) {
					if (do_debug & qtdb_gc) qWarning("Marking (%s*)%p -> %p", "QStandardItem", item, (void*)obj);
					rb_gc_mark(obj);
				}
			}
		}
	}
}

void
smokeruby_mark(void * p)
{
	VALUE obj;
    smokeruby_object * o = (smokeruby_object *) p;
    const char *className = o->smoke->classes[o->classId].className;
	
	if (do_debug & qtdb_gc) qWarning("Checking for mark (%s*)%p", className, o->ptr);

    if (o->ptr && o->allocated) {
		if (o->smoke->isDerivedFromByName(className, "QListWidget")) {
			QListWidget * listwidget = (QListWidget *) o->smoke->cast(o->ptr, o->classId, o->smoke->idClass("QListWidget").index);
			
			for (int i = 0; i < listwidget->count(); i++) {
				QListWidgetItem * item = listwidget->item(i);
				obj = getPointerObject(item);
				if (obj != Qnil) {
					if (do_debug & qtdb_gc) qWarning("Marking (%s*)%p -> %p", "QListWidgetItem", item, (void*)obj);
					rb_gc_mark(obj);
				}
			}
			return;
		}
	
		if (o->smoke->isDerivedFromByName(className, "QTableWidget")) {
			QTableWidget * table = (QTableWidget *) o->smoke->cast(o->ptr, o->classId, o->smoke->idClass("QTableWidget").index);
			QTableWidgetItem *item;

			for ( int row = 0; row < table->rowCount(); row++ ) {
				for ( int col = 0; col < table->columnCount(); col++ ) {
					item = table->item(row, col);
					obj = getPointerObject(item);
					if (obj != Qnil) {
						if(do_debug & qtdb_gc) qWarning("Marking (%s*)%p -> %p", className, item, (void*)obj);
						rb_gc_mark(obj);
					}
				}
			}
			return;		
		}

		if (o->smoke->isDerivedFromByName(className, "QTreeWidget")) {
			QTreeWidget * qtreewidget = (QTreeWidget *) o->smoke->cast(o->ptr, o->classId, o->smoke->idClass("QTreeWidget").index);

			for (int i = 0; i < qtreewidget->topLevelItemCount(); i++) {
				QTreeWidgetItem * item = qtreewidget->topLevelItem(i);
				obj = getPointerObject(item);
				if (obj != Qnil) {
					if (do_debug & qtdb_gc) qWarning("Marking (%s*)%p -> %p", "QTreeWidgetItem", item, (void*)obj);
					rb_gc_mark(obj);
				}
				mark_qtreewidgetitem_children(item);
			}
			return;
		}

		if (o->smoke->isDerivedFromByName(className, "QLayout")) {
			QLayout * qlayout = (QLayout *) o->smoke->cast(o->ptr, o->classId, o->smoke->idClass("QLayout").index);
			for (int i = 0; i < qlayout->count(); ++i) {
				QLayoutItem * item = qlayout->itemAt(i);
				if (item != 0) {
					obj = getPointerObject(item);
					if (obj != Qnil) {
						if (do_debug & qtdb_gc) qWarning("Marking (%s*)%p -> %p", "QLayoutItem", item, (void*)obj);
						rb_gc_mark(obj);
					}
				}
			}
			return;
		}

		if (o->smoke->isDerivedFromByName(className, "QStandardItemModel")) {
			QStandardItemModel * model = (QStandardItemModel *) o->smoke->cast(o->ptr, o->classId, o->smoke->idClass("QStandardItemModel").index);
			for (int row = 0; row < model->rowCount(); row++) {
				for (int column = 0; column < model->columnCount(); column++) {
					QStandardItem * item = model->item(row, column);
					if (item != 0) {
						if (item->hasChildren()) {
							mark_qstandarditem_children(item);
						}
						obj = getPointerObject(item);
						if (obj != Qnil) {
							if (do_debug & qtdb_gc) qWarning("Marking (%s*)%p -> %p", "QStandardItem", item, (void*)obj);
							rb_gc_mark(obj);
						}
					}
				}
			}
			return;
		}

#if QT_VERSION >= 0x40200
		if (o->smoke->isDerivedFromByName(className, "QGraphicsScene")) {
			QGraphicsScene * scene = (QGraphicsScene *) o->smoke->cast(o->ptr, o->classId, o->smoke->idClass("QGraphicsScene").index);
			QList<QGraphicsItem *> list = scene->items();
			for (int i = 0; i < list.size(); i++) {
				QGraphicsItem * item = list.at(i);
				if (item != 0) {
					obj = getPointerObject(item);
					if (obj != Qnil) {
						if (do_debug & qtdb_gc) qWarning("Marking (%s*)%p -> %p", "QGraphicsItem", item, (void*)obj);
						rb_gc_mark(obj);
					}
				}
			}			
			return;
		}
#endif

		if (qstrcmp(className, "QModelIndex") == 0) {
			QModelIndex * qmodelindex = (QModelIndex *) o->ptr;
			void * ptr = qmodelindex->internalPointer();
			if (ptr != 0 && ptr != (void *) Qnil) {
				rb_gc_mark((VALUE) ptr);
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
	
	if(do_debug & qtdb_gc) qWarning("Checking for delete (%s*)%p allocated: %s", className, o->ptr, o->allocated ? "true" : "false");
    
	if(application_terminated || !o->allocated || o->ptr == 0) {
		free_smokeruby_object(o);
		return;
	}
	
	unmapPointer(o, o->classId, 0);
	object_count --;
	
	if (	qstrcmp(className, "QObject") == 0
			|| qstrcmp(className, "QListBoxItem") == 0
			|| qstrcmp(className, "QStyleSheetItem") == 0
			|| qstrcmp(className, "QSqlCursor") == 0
			|| qstrcmp(className, "QModelIndex") == 0 )
	{
		// Don't delete instances of these classes for now
		free_smokeruby_object(o);
		return;
	} else if (o->smoke->isDerivedFromByName(className, "QLayoutItem")) {
		QLayoutItem * item = (QLayoutItem *) o->smoke->cast(o->ptr, o->classId, o->smoke->idClass("QLayoutItem").index);
		if (item->layout() != 0 || item->widget() != 0 || item->spacerItem() != 0) {
			free_smokeruby_object(o);
			return;
		}
	} else if (o->smoke->isDerivedFromByName(className, "QStandardItem")) {
		QStandardItem * item = (QStandardItem *) o->smoke->cast(o->ptr, o->classId, o->smoke->idClass("QStandardItem").index);
		if (item->model() != 0 || item->parent() != 0) {
			free_smokeruby_object(o);
			return;
		}
	} else if (qstrcmp(className, "QListWidgetItem") == 0) {
		QListWidgetItem * item = (QListWidgetItem *) o->ptr;
		if (item->listWidget() != 0) {
			free_smokeruby_object(o);
			return;
		}
	} else if (o->smoke->isDerivedFromByName(className, "QTableWidgetItem")) {
		QTableWidgetItem * item = (QTableWidgetItem *) o->smoke->cast(o->ptr, o->classId, o->smoke->idClass("QTableWidgetItem").index);
		if (item->tableWidget() != 0) {
			free_smokeruby_object(o);
			return;
		}
//	} else if (qstrcmp(className, "QPopupMenu") == 0) {
//		Q3PopupMenu * item = (Q3PopupMenu *) o->ptr;
//		if (item->parentWidget(false) != 0) {
//			free_smokeruby_object(o);
//			return;
//		}
	} else if (o->smoke->isDerivedFromByName(className, "QWidget")) {
		QWidget * qwidget = (QWidget *) o->smoke->cast(o->ptr, o->classId, o->smoke->idClass("QWidget").index);
		if (qwidget->parentWidget() != 0) {
			free_smokeruby_object(o);
			return;
		}
	} else if (o->smoke->isDerivedFromByName(className, "QObject")) {
		QObject * qobject = (QObject *) o->smoke->cast(o->ptr, o->classId, o->smoke->idClass("QObject").index);
		if (qobject->parent() != 0) {
			free_smokeruby_object(o);
			return;
		}
	}
			
	if(do_debug & qtdb_gc) qWarning("Deleting (%s*)%p", className, o->ptr);

	char *methodName = new char[strlen(className) + 2];
	methodName[0] = '~';
	strcpy(methodName + 1, className);
	Smoke::ModuleIndex nameId = o->smoke->findMethodName(className, methodName);
	Smoke::ModuleIndex classIdx = { o->smoke, o->classId };
	Smoke::ModuleIndex meth = o->smoke->findMethod(classIdx, nameId);
	if(meth.index > 0) {
		Smoke::Method &m = meth.smoke->methods[meth.smoke->methodMaps[meth.index].method];
		Smoke::ClassFn fn = meth.smoke->classes[m.classId].classFn;
		Smoke::StackItem i[1];
		(*fn)(m.method, o->ptr, i);
	}
	delete[] methodName;
	free_smokeruby_object(o);
	
    return;
}

/*
 * Given an approximate classname and a qt instance, try to improve the resolution of the name
 * by using the various Qt rtti mechanisms for QObjects, QEvents and QCanvasItems
 */
Q_DECL_EXPORT const char *
resolve_classname_qt(Smoke* smoke, int classId, void * ptr)
{
	if (smoke->isDerivedFromByName(smoke->classes[classId].className, "QEvent")) {
		QEvent * qevent = (QEvent *) smoke->cast(ptr, classId, smoke->idClass("QEvent").index);
		switch (qevent->type()) {
		case QEvent::Timer:
			return "Qt::TimerEvent";
		case QEvent::MouseButtonPress:
		case QEvent::MouseButtonRelease:
		case QEvent::MouseButtonDblClick:
		case QEvent::MouseMove:
			return "Qt::MouseEvent";
		case QEvent::KeyPress:
		case QEvent::KeyRelease:
		case QEvent::ShortcutOverride:
			return "Qt::KeyEvent";
		case QEvent::FocusIn:
		case QEvent::FocusOut:
			return "Qt::FocusEvent";
		case QEvent::Enter:
		case QEvent::Leave:
			return "Qt::Event";
		case QEvent::Paint:
			return "Qt::PaintEvent";
		case QEvent::Move:
			return "Qt::MoveEvent";
		case QEvent::Resize:
			return "Qt::ResizeEvent";
		case QEvent::Create:
		case QEvent::Destroy:
			return "Qt::Event";
		case QEvent::Show:
			return "Qt::ShowEvent";
		case QEvent::Hide:
			return "Qt::HideEvent";
		case QEvent::Close:
			return "Qt::CloseEvent";
		case QEvent::Quit:
		case QEvent::ParentChange:
		case QEvent::ParentAboutToChange:
		case QEvent::ThreadChange:
		case QEvent::WindowActivate:
		case QEvent::WindowDeactivate:
		case QEvent::ShowToParent:
		case QEvent::HideToParent:
			return "Qt::Event";
		case QEvent::Wheel:
			return "Qt::WheelEvent";
		case QEvent::WindowTitleChange:
		case QEvent::WindowIconChange:
		case QEvent::ApplicationWindowIconChange:
		case QEvent::ApplicationFontChange:
		case QEvent::ApplicationLayoutDirectionChange:
		case QEvent::ApplicationPaletteChange:
		case QEvent::PaletteChange:
			return "Qt::Event";
		case QEvent::Clipboard:
			return "Qt::ClipboardEvent";
		case QEvent::Speech:
		case QEvent::MetaCall:
		case QEvent::SockAct:
		case QEvent::WinEventAct:
		case QEvent::DeferredDelete:
			return "Qt::Event";
		case QEvent::DragEnter:
			return "Qt::DragEnterEvent";
		case QEvent::DragLeave:
			return "Qt::DragLeaveEvent";
		case QEvent::DragMove:
			return "Qt::DragMoveEvent";
		case QEvent::Drop:
			return "Qt::DropEvent";
		case QEvent::DragResponse:
			return "Qt::DragResponseEvent";
		case QEvent::ChildAdded:
		case QEvent::ChildRemoved:
		case QEvent::ChildPolished:
			return "Qt::ChildEvent";
		case QEvent::ShowWindowRequest:
		case QEvent::PolishRequest:
		case QEvent::Polish:
		case QEvent::LayoutRequest:
		case QEvent::UpdateRequest:
		case QEvent::EmbeddingControl:
		case QEvent::ActivateControl:
		case QEvent::DeactivateControl:
			return "Qt::Event";
		case QEvent::ContextMenu:
			return "Qt::ContextMenuEvent";
		case QEvent::InputMethod:
			return "Qt::InputMethodEvent";
		case QEvent::AccessibilityPrepare:
			return "Qt::Event";
		case QEvent::TabletMove:
		case QEvent::TabletPress:
		case QEvent::TabletRelease:
			return "Qt::TabletEvent";
		case QEvent::LocaleChange:
		case QEvent::LanguageChange:
		case QEvent::LayoutDirectionChange:
		case QEvent::Style:
		case QEvent::OkRequest:
		case QEvent::HelpRequest:
			return "Qt::Event";
		case QEvent::IconDrag:
			return "Qt::IconDragEvent";
		case QEvent::FontChange:
		case QEvent::EnabledChange:
		case QEvent::ActivationChange:
		case QEvent::StyleChange:
		case QEvent::IconTextChange:
		case QEvent::ModifiedChange:
		case QEvent::MouseTrackingChange:
			return "Qt::Event";
		case QEvent::WindowBlocked:
		case QEvent::WindowUnblocked:
		case QEvent::WindowStateChange:
			return "Qt::WindowStateChangeEvent";
		case QEvent::ToolTip:
		case QEvent::WhatsThis:
			return "Qt::HelpEvent";
		case QEvent::StatusTip:
			return "Qt::Event";
		case QEvent::ActionChanged:
		case QEvent::ActionAdded:
		case QEvent::ActionRemoved:
			return "Qt::ActionEvent";
		case QEvent::FileOpen:
			return "Qt::FileOpenEvent";
		case QEvent::Shortcut:
			return "Qt::ShortcutEvent";
		case QEvent::WhatsThisClicked:
			return "Qt::WhatsThisClickedEvent";
		case QEvent::ToolBarChange:
			return "Qt::ToolBarChangeEvent";
		case QEvent::ApplicationActivated:
		case QEvent::ApplicationDeactivated:
		case QEvent::QueryWhatsThis:
		case QEvent::EnterWhatsThisMode:
		case QEvent::LeaveWhatsThisMode:
		case QEvent::ZOrderChange:
			return "Qt::Event";
		case QEvent::HoverEnter:
		case QEvent::HoverLeave:
		case QEvent::HoverMove:
			return "Qt::HoverEvent";
		case QEvent::AccessibilityHelp:
		case QEvent::AccessibilityDescription:
			return "Qt::Event";
#if QT_VERSION >= 0x40200
		case QEvent::GraphicsSceneMouseMove:
		case QEvent::GraphicsSceneMousePress:
		case QEvent::GraphicsSceneMouseRelease:
		case QEvent::GraphicsSceneMouseDoubleClick:
			return "Qt::GraphicsSceneMouseEvent";
		case QEvent::GraphicsSceneContextMenu:
			return "Qt::GraphicsSceneContextMenuEvent";
		case QEvent::GraphicsSceneHoverEnter:
		case QEvent::GraphicsSceneHoverMove:
		case QEvent::GraphicsSceneHoverLeave:
			return "Qt::GraphicsSceneHoverEvent";
		case QEvent::GraphicsSceneHelp:
			return "Qt::GraphicsSceneHelpEvent";
		case QEvent::GraphicsSceneDragEnter:
		case QEvent::GraphicsSceneDragMove:
		case QEvent::GraphicsSceneDragLeave:
		case QEvent::GraphicsSceneDrop:
			return "Qt::GraphicsSceneDragDropEvent";
		case QEvent::GraphicsSceneWheel:
			return "Qt::GraphicsSceneWheelEvent";
		case QEvent::KeyboardLayoutChange:
			return "Qt::Event";
#endif
		default:
			break;
		}
	} else if (smoke->isDerivedFromByName(smoke->classes[classId].className, "QObject")) {
		QObject * qobject = (QObject *) smoke->cast(ptr, classId, smoke->idClass("QObject").index);
		const QMetaObject * meta = qobject->metaObject();

		while (meta != 0) {
			Smoke::Index classId = smoke->idClass(meta->className()).index;
			if (classId != 0) {
				return smoke->binding->className(classId);
			}

			meta = meta->superClass();
		}
	} else if (smoke->isDerivedFromByName(smoke->classes[classId].className, "QGraphicsItem")) {
		QGraphicsItem * item = (QGraphicsItem *) smoke->cast(ptr, classId, smoke->idClass("QGraphicsItem").index);
		switch (item->type()) {
		case 1:
			return "Qt::GraphicsItem";
		case 2:
			return "Qt::GraphicsPathItem";
		case 3:
			return "Qt::GraphicsRectItem";
		case 4:
			return "Qt::GraphicsEllipseItem";
		case 5:
			return "Qt::GraphicsPolygonItem";
		case 6:
			return "Qt::GraphicsLineItem";
		case 7:
			return "Qt::GraphicsPixmapItem";
		case 8:
			return "Qt::GraphicsTextItem";
		case 9:
			return "Qt::GraphicsSimpleTextItem";
		case 10:
			return "Qt::GraphicsItemGroup";
		default:
			return "Qt::GraphicsItem";
		}
	} else if (smoke->isDerivedFromByName(smoke->classes[classId].className, "QLayoutItem")) {
		QLayoutItem * item = (QLayoutItem *) smoke->cast(ptr, classId, smoke->idClass("QLayoutItem").index);
		if (item->widget() != 0) {
			return "Qt::WidgetItem";
		} else if (item->spacerItem() != 0) {
			return "Qt::SpacerItem";
		} else {
			return "Qt::Layout";
		}
	} else if (smoke->isDerivedFromByName(smoke->classes[classId].className, "QListWidgetItem")) {
		QListWidgetItem * item = (QListWidgetItem *) smoke->cast(ptr, classId, smoke->idClass("QListWidgetItem").index);
		switch (item->type()) {
		case 0:
			return "Qt::ListWidgetItem";
		default:
			return "Qt::ListWidgetItem";
			break;
		}
	} else if (smoke->isDerivedFromByName(smoke->classes[classId].className, "QTableWidgetItem")) {
		QTableWidgetItem * item = (QTableWidgetItem *) smoke->cast(ptr, classId, smoke->idClass("QTableWidgetItem").index);
		switch (item->type()) {
		case 0:
			return "Qt::TableWidgetItem";
		default:
			return "Qt::TableWidgetItem";
			break;
		}
	}
	
	return smoke->binding->className(classId);
}

bool
matches_arg(Smoke *smoke, Smoke::Index meth, Smoke::Index argidx, const char *argtype)
{
	Smoke::Index *arg = smoke->argumentList + smoke->methods[meth].args + argidx;
	SmokeType type = SmokeType(smoke, *arg);
	if (type.name() && qstrcmp(type.name(), argtype) == 0) {
		return true;
	}
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
    Smoke::ModuleIndex ccId = o->smoke->findMethodName(className, ccSig);
    delete[] ccSig;

    char *ccArg = new char[classNameLen + 8];
    sprintf(ccArg, "const %s&", className);

    Smoke::ModuleIndex classIdx = { o->smoke, o->classId };
    Smoke::ModuleIndex ccMeth = o->smoke->findMethod(classIdx, ccId);

    if(!ccMeth.index) {
	delete[] ccArg;
	return 0;
    }
	Smoke::Index method = ccMeth.smoke->methodMaps[ccMeth.index].method;
    if(method > 0) {
	// Make sure it's a copy constructor
	if(!matches_arg(o->smoke, method, 0, ccArg)) {
            delete[] ccArg;
	    return 0;
        }
        delete[] ccArg;
        ccMeth.index = method;
    } else {
        // ambiguous method, pick the copy constructor
	Smoke::Index i = -method;
	while(ccMeth.smoke->ambiguousMethodList[i]) {
	    if(matches_arg(ccMeth.smoke, ccMeth.smoke->ambiguousMethodList[i], 0, ccArg))
		break;
            i++;
	}
        delete[] ccArg;
	ccMeth.index = ccMeth.smoke->ambiguousMethodList[i];
	if(!ccMeth.index)
	    return 0;
    }

    // Okay, ccMeth is the copy constructor. Time to call it.
    Smoke::StackItem args[2];
    args[0].s_voidp = 0;
    args[1].s_voidp = o->ptr;
    Smoke::ClassFn fn = o->smoke->classes[o->classId].classFn;
    (*fn)(o->smoke->methods[ccMeth.index].method, 0, args);
    return args[0].s_voidp;
}

#include "marshall_basetypes.h"

template <class T>
static void marshall_it(Marshall *m)
{
	switch(m->action()) {
		case Marshall::FromVALUE:
			marshall_from_ruby<T>(m);
		break;
 
		case Marshall::ToVALUE:
			marshall_to_ruby<T>( m );
		break;
				
		default:
			m->unsupported();
		break;
	}
}

void
marshall_basetype(Marshall *m)
{
	switch(m->type().elem()) {

		case Smoke::t_bool:
			marshall_it<bool>(m);
		break;

		case Smoke::t_char:
			marshall_it<signed char>(m);
		break;
		
		case Smoke::t_uchar:
			marshall_it<unsigned char>(m);
		break;
 
		case Smoke::t_short:
			marshall_it<short>(m);
		break;
      
		case Smoke::t_ushort:
			marshall_it<unsigned short>(m);
		break;

		case Smoke::t_int:
			marshall_it<int>(m);
		break;
		
		case Smoke::t_uint:
			marshall_it<unsigned int>(m);
		break;
 
		case Smoke::t_long:
			marshall_it<long>(m);
		break;

		case Smoke::t_ulong:
			marshall_it<unsigned long>(m);
		break;
 
		case Smoke::t_float:
			marshall_it<float>(m);
		break;

		case Smoke::t_double:
			marshall_it<double>(m);
		break;

		case Smoke::t_enum:
			marshall_it<SmokeEnumWrapper>(m);
		break;
     
		case Smoke::t_class:
			marshall_it<SmokeClassWrapper>(m);
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

void marshall_ucharP(Marshall *m) {
  marshall_it<unsigned char *>(m);
}

static void marshall_doubleR(Marshall *m) {
	switch(m->action()) {
	case Marshall::FromVALUE:
	{
		VALUE rv = *(m->var());
		double * d = new double;
		*d = NUM2DBL(rv);
		m->item().s_voidp = d;
		m->next();
		if (m->cleanup() && m->type().isConst()) {
			delete d;
		} else {
			m->item().s_voidp = new double((double)NUM2DBL(rv));
		}
	}
	break;
	case Marshall::ToVALUE:
	{
		double *dp = (double*)m->item().s_voidp;
	    VALUE rv = *(m->var());
		if (dp == 0) {
			rv = Qnil;
			break;
		}
		*(m->var()) = rb_float_new(*dp);
		m->next();
		if (!m->type().isConst()) {
			*dp = NUM2DBL(*(m->var()));
		}
	}
	break;
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
	if (qstrcmp(KCODE, "EUC") == 0) {
		codec = QTextCodec::codecForName("eucJP");
	} else if (qstrcmp(KCODE, "SJIS") == 0) {
		codec = QTextCodec::codecForName("Shift-JIS");
	}
}

QString* 
qstringFromRString(VALUE rstring) {
	if (KCODE == 0) {
		init_codec();
	}
	
	if (qstrcmp(KCODE, "UTF8") == 0)
		return new QString(QString::fromUtf8(StringValuePtr(rstring), RSTRING(rstring)->len));
	else if (qstrcmp(KCODE, "EUC") == 0)
		return new QString(codec->toUnicode(StringValuePtr(rstring)));
	else if (qstrcmp(KCODE, "SJIS") == 0)
		return new QString(codec->toUnicode(StringValuePtr(rstring)));
	else if(qstrcmp(KCODE, "NONE") == 0)
		return new QString(QString::fromLatin1(StringValuePtr(rstring)));

	return new QString(QString::fromLocal8Bit(StringValuePtr(rstring), RSTRING(rstring)->len));
}

VALUE 
rstringFromQString(QString * s) {
	if (KCODE == 0) {
		init_codec();
	}
	
	if (qstrcmp(KCODE, "UTF8") == 0)
		return rb_str_new2(s->toUtf8());
	else if (qstrcmp(KCODE, "EUC") == 0)
		return rb_str_new2(codec->fromUnicode(*s));
	else if (qstrcmp(KCODE, "SJIS") == 0)
		return rb_str_new2(codec->fromUnicode(*s));
	else if (qstrcmp(KCODE, "NONE") == 0)
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
	
			if (s != 0 && m->cleanup()) {
				delete s;
			}
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
				if(m->cleanup() || m->type().isStack() ) {
					delete s;
				}
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

// The only way to convert a QChar to a QString is to
// pass a QChar to a QString constructor. However,
// QStrings aren't in the QtRuby api, so add this
// convenience method 'Qt::Char.to_s' to get a ruby
// string from a Qt::Char.
VALUE
qchar_to_s(VALUE self)
{
	smokeruby_object *o = value_obj_info(self);
	if (o == 0 || o->ptr == 0) {
		return Qnil;
	}

	QChar * qchar = (QChar*) o->ptr;
	QString s(*qchar);
	return rstringFromQString(&s);
}

void marshall_QDBusVariant(Marshall *m) {
	switch(m->action()) {
	case Marshall::FromVALUE: 
	{
		VALUE v = *(m->var());
		if (v == Qnil) {
			m->item().s_voidp = 0;
			break;
		}

		smokeruby_object *o = value_obj_info(v);
		if (!o || !o->ptr) {
			if (m->type().isRef()) {
				m->unsupported();
			}
		    m->item().s_class = 0;
		    break;
		}
		m->item().s_class = o->ptr;
		break;
	}

	case Marshall::ToVALUE: 
	{
		if (m->item().s_voidp == 0) {
			*(m->var()) = Qnil;
		    break;
		}

		void *p = m->item().s_voidp;
		VALUE obj = getPointerObject(p);
		if(obj != Qnil) {
			*(m->var()) = obj;
		    break;
		}
		smokeruby_object * o = alloc_smokeruby_object(false, m->smoke(), m->smoke()->findClass("QVariant").index, p);
		
		obj = set_obj_info("Qt::DBusVariant", o);
		if (do_debug & qtdb_calls) {
			printf("allocating %s %p -> %p\n", "Qt::DBusVariant", o->ptr, (void*)obj);
		}

		if (m->type().isStack()) {
		    o->allocated = true;
			// Keep a mapping of the pointer so that it is only wrapped once
		    mapPointer(obj, o, o->classId, 0);
		}
		
		*(m->var()) = obj;
	}
	
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

		rb_ary_clear(arglist);
		for(i = 0; argv[i]; i++) {
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
			
			if (m->cleanup()) {
				delete stringlist;
			}
	   
			break;
		}

      case Marshall::ToVALUE: 
	{
		QStringList *stringlist = static_cast<QStringList *>(m->item().s_voidp);
		if (!stringlist) {
			*(m->var()) = Qnil;
			break;
		}

		VALUE av = rb_ary_new();
		for (QStringList::Iterator it = stringlist->begin(); it != stringlist->end(); ++it) {
			VALUE rv = rstringFromQString(&(*it));
			rb_ary_push(av, rv);
		}

		*(m->var()) = av;

		if (m->cleanup()) {
			delete stringlist;
		}

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

		if (!m->type().isConst()) {
			rb_ary_clear(list);
			for (int i = 0; i < stringlist->size(); i++) {
				rb_ary_push(list, rb_str_new2((const char *) stringlist->at(i)));
			}
		}

		if(m->cleanup()) {
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


	    *(m->var()) = av;

		if (m->cleanup()) {
			delete stringlist;
		}
	}
	break;
      default:
	m->unsupported();
	break;
    }
}

void marshall_ExtraSelectionList(Marshall *m) {
	switch(m->action()) {
	case Marshall::FromVALUE: 
	{
		VALUE list = *(m->var());
		if (TYPE(list) != T_ARRAY) {
			m->item().s_voidp = 0;
			break;
		}

		int count = RARRAY(list)->len;
		QList<QTextEdit::ExtraSelection> *selectionlist = new QList<QTextEdit::ExtraSelection>;

		for (long i = 0; i < count; i++) {
			VALUE item = rb_ary_entry(list, i);
			QTextEdit::ExtraSelection selection;

			VALUE cursor = rb_funcall(item, rb_intern("cursor"), 0);
			smokeruby_object *c = value_obj_info(cursor);
            selection.cursor = *((QTextCursor*) c->ptr);

			VALUE format = rb_funcall(item, rb_intern("format"), 0);
			smokeruby_object *f = value_obj_info(format);
			selection.format = *((QTextCharFormat*) f->ptr);

			selectionlist->append(selection);
		}

		m->item().s_voidp = selectionlist;
		m->next();


		if (m->cleanup()) {
			delete selectionlist;
		}
		break;
	}
	case Marshall::ToVALUE: 
	{
		QList<QTextEdit::ExtraSelection> *selectionlist = static_cast<QList<QTextEdit::ExtraSelection>*>(m->item().s_voidp);
		if (selectionlist == 0) {
			*(m->var()) = Qnil;
			break;
		}

	    VALUE av = rb_ary_new();
		for (int i = 0; i < selectionlist->size(); i++) {
			QTextEdit::ExtraSelection selection = selectionlist->at(i);

			smokeruby_object  * c = alloc_smokeruby_object(	true, 
															m->smoke(), 
															m->smoke()->idClass("QTextCursor").index, 
															new QTextCursor(selection.cursor) );
			VALUE cursor = set_obj_info("Qt::TextCursor", c);

			smokeruby_object  * f = alloc_smokeruby_object(	true, 
															m->smoke(), 
															m->smoke()->idClass("QTextCharFormat").index, 
															new QTextCharFormat(selection.format) );
			VALUE format = set_obj_info("Qt::TextCharFormat", f);

			rb_ary_push(av, rb_funcall(qt_internal_module, rb_intern("create_extra_selection"), 2, cursor, format));
		}


		*(m->var()) = av;

		if (m->cleanup()) {
			delete selectionlist;
		}
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
		    		o->smoke->idClass(ItemSTR).index	// to
				);
				cpplist->append((Item*)ptr);
			}

			m->item().s_voidp = cpplist;
			m->next();

			if (!m->type().isConst()) {
				rb_ary_clear(list);
	
				for(int i = 0; i < cpplist->size(); ++i ) {
					VALUE obj = getPointerObject( cpplist->at(i) );
					rb_ary_push(list, obj);
				}
			}

			if (m->cleanup()) {
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

			for (int i=0;i<valuelist->size();++i) {
				void *p = valuelist->at(i);

				if (m->item().s_voidp == 0) {
					*(m->var()) = Qnil;
					break;
				}

				VALUE obj = getPointerObject(p);
				if (obj == Qnil) {
					smokeruby_object  * o = alloc_smokeruby_object(	false, 
																	m->smoke(), 
																	m->smoke()->idClass(ItemSTR).index, 
																	p );

					obj = set_obj_info(resolve_classname(o->smoke, o->classId, o->ptr), o);
				}
			
				rb_ary_push(av, obj);
			}

			*(m->var()) = av;
			m->next();

			if (m->cleanup()) {
				delete valuelist;
			}
		}
		break;

		default:
			m->unsupported();
		break;
   }
}

void marshall_QListCharStar(Marshall *m) {
	switch(m->action()) {
	case Marshall::FromVALUE:
	{
		VALUE av = *(m->var());
		if (TYPE(av) != T_ARRAY) {
			m->item().s_voidp = 0;
			break;
		}
		int count = RARRAY(av)->len;
		QList<const char*> *list = new QList<const char*>;
		long i;
		for(i = 0; i < count; i++) {
			VALUE item = rb_ary_entry(av, i);
			if (TYPE(item) != T_STRING) {
				list->append(0);
		    	continue;
			}
			list->append(StringValuePtr(item));
		}

		m->item().s_voidp = list;
	}
	break;
	case Marshall::ToVALUE:
	{
		QList<const char*> *list = (QList<const char*>*)m->item().s_voidp;
		if (list == 0) {
			*(m->var()) = Qnil;
			break;
		}

		VALUE av = rb_ary_new();

		for (	QList<const char*>::iterator i = list->begin(); 
				i != list->end(); 
				++i ) 
		{
		    rb_ary_push(av, rb_str_new2((const char *)*i));
		}
		
		*(m->var()) = av;
		m->next();
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

		if (!m->type().isConst()) {
			rb_ary_clear(list);
	
			for (	QList<int>::iterator i = valuelist->begin(); 
					i != valuelist->end(); 
					++i ) 
			{
				rb_ary_push(list, INT2NUM((int)*i));
			}
		}

		if (m->cleanup()) {
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
		m->next();

		if (m->cleanup()) {
			delete valuelist;
		}
	}
	break;
      default:
	m->unsupported();
	break;
    }
}


void marshall_QListUInt(Marshall *m) {
    switch(m->action()) {
      case Marshall::FromVALUE:
	{
	    VALUE list = *(m->var());
	    if (TYPE(list) != T_ARRAY) {
		m->item().s_voidp = 0;
		break;
	    }
	    int count = RARRAY(list)->len;
	    QList<uint> *valuelist = new QList<uint>;
	    long i;
	    for(i = 0; i < count; i++) {
		VALUE item = rb_ary_entry(list, i);
		if(TYPE(item) != T_FIXNUM && TYPE(item) != T_BIGNUM) {
		    valuelist->append(0);
		    continue;
		}
		valuelist->append(NUM2UINT(item));
	    }

	    m->item().s_voidp = valuelist;
	    m->next();

		if (!m->type().isConst()) {
			rb_ary_clear(list);
	
			for (	QList<uint>::iterator i = valuelist->begin(); 
					i != valuelist->end(); 
					++i ) 
			{
				rb_ary_push(list, UINT2NUM((int)*i));
			}
		}

		if (m->cleanup()) {
			delete valuelist;
	    }
	}
	break;
      case Marshall::ToVALUE:
	{
	    QList<uint> *valuelist = (QList<uint>*)m->item().s_voidp;
	    if(!valuelist) {
		*(m->var()) = Qnil;
		break;
	    }

	    VALUE av = rb_ary_new();

		for (	QList<uint>::iterator i = valuelist->begin(); 
				i != valuelist->end(); 
				++i ) 
		{
		    rb_ary_push(av, UINT2NUM((int)*i));
		}
		
	    *(m->var()) = av;
		m->next();

		if (m->cleanup()) {
			delete valuelist;
		}
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

		if (!m->type().isConst()) {
			rb_ary_clear(list);
	
			for (	QList<qreal>::iterator i = valuelist->begin(); 
					i != valuelist->end(); 
					++i ) 
			{
				rb_ary_push(list, rb_float_new((qreal)*i));
			}
		}

		if (m->cleanup()) {
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
		m->next();

		if (m->cleanup()) {
			delete valuelist;
		}
	}
	break;
      default:
	m->unsupported();
	break;
    }
}

void marshall_QVectorqreal(Marshall *m) {
	switch(m->action()) {
	case Marshall::FromVALUE:
	{
		VALUE list = *(m->var());

		list = rb_check_array_type(*(m->var()));
		if (NIL_P(list)) {
			m->item().s_voidp = 0;
			break;
		}

		int count = RARRAY(list)->len;
		QVector<qreal> *valuelist = new QVector<qreal>;
		long i;
		for (i = 0; i < count; i++) {
			valuelist->append(NUM2DBL(rb_ary_entry(list, i)));
		}

		m->item().s_voidp = valuelist;
		m->next();

		if (!m->type().isConst()) {
			rb_ary_clear(list);
	
			for (	QVector<qreal>::iterator i = valuelist->begin(); 
					i != valuelist->end(); 
					++i ) 
			{
				rb_ary_push(list, rb_float_new((qreal)*i));
			}
		}

		if (m->cleanup()) {
			delete valuelist;
		}
	}
	break;
	case Marshall::ToVALUE:
	{
	    QVector<qreal> *valuelist = (QVector<qreal>*)m->item().s_voidp;
	    if(!valuelist) {
		*(m->var()) = Qnil;
		break;
	    }

	    VALUE av = rb_ary_new();

		for (	QVector<qreal>::iterator i = valuelist->begin(); 
				i != valuelist->end(); 
				++i ) 
		{
		    rb_ary_push(av, rb_float_new((qreal)*i));
		}
		
	    *(m->var()) = av;
		m->next();

		if (m->cleanup()) {
			delete valuelist;
		}
	}
	break;
      default:
	m->unsupported();
	break;
    }
}

void marshall_QVectorint(Marshall *m) {
	switch(m->action()) {
	case Marshall::FromVALUE:
	{
		VALUE list = *(m->var());

		list = rb_check_array_type(*(m->var()));
		if (NIL_P(list)) {
			m->item().s_voidp = 0;
			break;
		}

		int count = RARRAY(list)->len;
		QVector<int> *valuelist = new QVector<int>;
		long i;
		for (i = 0; i < count; i++) {
			valuelist->append(NUM2INT(rb_ary_entry(list, i)));
		}

		m->item().s_voidp = valuelist;
		m->next();

		if (!m->type().isConst()) {
			rb_ary_clear(list);
	
			for (	QVector<int>::iterator i = valuelist->begin(); 
					i != valuelist->end(); 
					++i ) 
			{
				rb_ary_push(list, INT2NUM((int)*i));
			}
		}

		if (m->cleanup()) {
			delete valuelist;
		}
	}
	break;
	case Marshall::ToVALUE:
	{
	    QVector<int> *valuelist = (QVector<int>*)m->item().s_voidp;
	    if(!valuelist) {
		*(m->var()) = Qnil;
		break;
	    }

	    VALUE av = rb_ary_new();

		for (	QVector<int>::iterator i = valuelist->begin(); 
				i != valuelist->end(); 
				++i ) 
		{
		    rb_ary_push(av, INT2NUM((int)*i));
		}
		
	    *(m->var()) = av;
		m->next();

		if (m->cleanup()) {
			delete valuelist;
		}
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
			if (!o || !o->ptr || o->classId != o->smoke->findClass("QVariant").index) {
				// If the value isn't a Qt::Variant, then try and construct
				// a Qt::Variant from it
				value = rb_funcall(qvariant_class, rb_intern("fromValue"), 1, value);
				if (value == Qnil) {
					continue;
				}
				o = value_obj_info(value);
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
				smokeruby_object  * o = alloc_smokeruby_object(	true, 
																m->smoke(), 
																m->smoke()->idClass("QVariant").index, 
																p );
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

void marshall_QMapIntQVariant(Marshall *m) {
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
			if (!o || !o->ptr || o->classId != o->smoke->idClass("QVariant").index) {
				// If the value isn't a Qt::Variant, then try and construct
				// a Qt::Variant from it
				value = rb_funcall(qvariant_class, rb_intern("fromValue"), 1, value);
				if (value == Qnil) {
					continue;
				}
				o = value_obj_info(value);
			}
			
			(*map)[NUM2INT(key)] = (QVariant)*(QVariant*)o->ptr;
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
		if (!map) {
			*(m->var()) = Qnil;
			break;
	    }
		
	    VALUE hv = rb_hash_new();
			
		QMap<int,QVariant>::Iterator it;
		for (it = map->begin(); it != map->end(); ++it) {
			void *p = new QVariant(it.value());
			VALUE obj = getPointerObject(p);
				
			if (obj == Qnil) {
				smokeruby_object  * o = alloc_smokeruby_object(	true, 
																m->smoke(), 
																m->smoke()->idClass("QVariant").index, 
																p );
				obj = set_obj_info("Qt::Variant", o);
			}
			
			rb_hash_aset(hv, INT2NUM(it.key()), obj);
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
			ptr = o->smoke->cast(ptr, o->classId, o->smoke->idClass("QVariant").index);
			
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
				smokeruby_object  * o = alloc_smokeruby_object(	true, 
																m->smoke(), 
																m->smoke()->idClass("QVariant").index, 
																p );
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

void marshall_QPairQStringQStringList(Marshall *m) {
	switch(m->action()) {
	case Marshall::FromVALUE: 
	{
		VALUE list = *(m->var());
		if (TYPE(list) != T_ARRAY) {
			m->item().s_voidp = 0;
			break;
	    }

		QList<QPair<QString,QString> > * pairlist = new QList<QPair<QString,QString> >();
		int count = RARRAY(list)->len;

		for (long i = 0; i < count; i++) {
			VALUE item = rb_ary_entry(list, i);
			if (TYPE(item) != T_ARRAY || RARRAY(item)->len != 2) {
				continue;
			}
			VALUE s1 = rb_ary_entry(item, 0);
			VALUE s2 = rb_ary_entry(item, 1);
			QPair<QString,QString> * qpair = new QPair<QString,QString>(*(qstringFromRString(s1)),*(qstringFromRString(s2)));
			pairlist->append(*qpair);
		}

		m->item().s_voidp = pairlist;
		m->next();
			
		if (m->cleanup()) {
			delete pairlist;
		}
	   
		break;
	}

	case Marshall::ToVALUE: 
	{
		QList<QPair<QString,QString> > *pairlist = static_cast<QList<QPair<QString,QString> > * >(m->item().s_voidp);
		if (pairlist == 0) {
			*(m->var()) = Qnil;
			break;
		}

		VALUE av = rb_ary_new();
		for (QList<QPair<QString,QString> >::Iterator it = pairlist->begin(); it != pairlist->end(); ++it) {
			QPair<QString,QString> * pair = &(*it);
			VALUE rv1 = rstringFromQString(&(pair->first));
			VALUE rv2 = rstringFromQString(&(pair->second));
			VALUE pv = rb_ary_new();
			rb_ary_push(pv, rv1);
			rb_ary_push(pv, rv2);
			rb_ary_push(av, pv);
		}

		*(m->var()) = av;

		if (m->cleanup()) {
			delete pairlist;
		}

	}
	break;
	default:
		m->unsupported();
		break;
    }
}

void marshall_QPairqrealQColor(Marshall *m) {
	switch(m->action()) {
	case Marshall::FromVALUE:
	{
		VALUE list = *(m->var());
		if (TYPE(list) != T_ARRAY || RARRAY(list)->len != 2) {
			m->item().s_voidp = 0;
			break;
	    }

		qreal real;
		VALUE item1 = rb_ary_entry(list, 0);
		if (TYPE(item1) != T_FLOAT) {
		    real = 0;
		} else {
			real = NUM2DBL(item1);
		}
		
		VALUE item2 = rb_ary_entry(list, 1);

		smokeruby_object *o = value_obj_info(item2);
		if (o == 0 || o->ptr == 0) {
			m->item().s_voidp = 0;
			break;
		}
		
		QPair<qreal,QColor> * qpair = new QPair<qreal,QColor>(real, *((QColor *) o->ptr));
		m->item().s_voidp = qpair;
		m->next();

		if (m->cleanup()) {
			delete qpair;
		}
	}
	break;
	case Marshall::ToVALUE:
	{
		QPair<qreal,QColor> * qpair = static_cast<QPair<qreal,QColor> * >(m->item().s_voidp); 
		if (qpair == 0) {
			*(m->var()) = Qnil;
			break;
		}

		VALUE rv1 = rb_float_new(qpair->first);

		void *p = (void *) &(qpair->second);
		VALUE rv2 = getPointerObject(p);
		if (rv2 == Qnil) {
			smokeruby_object  * o = alloc_smokeruby_object(	false, 
															m->smoke(), 
															m->smoke()->idClass("QColor").index, 
															p );
			rv2 = set_obj_info("Qt::Color", o);
		}

		VALUE av = rb_ary_new();
		rb_ary_push(av, rv1);
		rb_ary_push(av, rv2);
		*(m->var()) = av;

		if (m->cleanup()) {
//			delete qpair;
		}
	}
		break;
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
		if (TYPE(item) != T_FIXNUM && TYPE(item) != T_BIGNUM) {
		    int0 = 0;
		} else {
			int0 = NUM2INT(item);
		}
		
		item = rb_ary_entry(list, 1);

		if (TYPE(item) != T_FIXNUM && TYPE(item) != T_BIGNUM) {
		    int1 = 0;
		} else {
			int1 = NUM2INT(item);
		}
		
		QPair<int,int> * qpair = new QPair<int,int>(int0,int1);
		m->item().s_voidp = qpair;
		m->next();

		if (m->cleanup()) {
			delete qpair;
		}
	}
	break;
      case Marshall::ToVALUE:
      default:
	m->unsupported();
	break;
    }
}

#define DEF_LIST_MARSHALLER(ListIdent,ItemList,Item) namespace { char ListIdent##STR[] = #Item; }  \
        Marshall::HandlerFn marshall_##ListIdent = marshall_ItemList<Item,ItemList,ListIdent##STR>;

DEF_LIST_MARSHALLER( QAbstractButtonList, QList<QAbstractButton*>, QAbstractButton )
DEF_LIST_MARSHALLER( QActionGroupList, QList<QActionGroup*>, QActionGroup )
DEF_LIST_MARSHALLER( QActionList, QList<QAction*>, QAction )
DEF_LIST_MARSHALLER( QListWidgetItemList, QList<QListWidgetItem*>, QListWidgetItem )
DEF_LIST_MARSHALLER( QObjectList, QList<QObject*>, QObject )
DEF_LIST_MARSHALLER( QTableWidgetList, QList<QTableWidget*>, QTableWidget )
DEF_LIST_MARSHALLER( QTableWidgetItemList, QList<QTableWidgetItem*>, QTableWidgetItem )
DEF_LIST_MARSHALLER( QTextFrameList, QList<QTextFrame*>, QTextFrame )
DEF_LIST_MARSHALLER( QTreeWidgetItemList, QList<QTreeWidgetItem*>, QTreeWidgetItem )
DEF_LIST_MARSHALLER( QTreeWidgetList, QList<QTreeWidget*>, QTreeWidget )
DEF_LIST_MARSHALLER( QWidgetList, QList<QWidget*>, QWidget )
DEF_LIST_MARSHALLER( QWidgetPtrList, QList<QWidget*>, QWidget )

#if QT_VERSION >= 0x40200
DEF_LIST_MARSHALLER( QGraphicsItemList, QList<QGraphicsItem*>, QGraphicsItem )
DEF_LIST_MARSHALLER( QStandardItemList, QList<QStandardItem*>, QStandardItem )
DEF_LIST_MARSHALLER( QUndoStackList, QList<QUndoStack*>, QUndoStack )
#endif

#if QT_VERSION >= 0x40300
DEF_LIST_MARSHALLER( QMdiSubWindowList, QList<QMdiSubWindow*>, QMdiSubWindow )
#endif

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

				// Special case for the QList<QVariant> type
				if (	qstrcmp(ItemSTR, "QVariant") == 0 
						&& (!o || !o->ptr || o->classId != o->smoke->idClass("QVariant").index) ) 
				{
					// If the value isn't a Qt::Variant, then try and construct
					// a Qt::Variant from it
					item = rb_funcall(qvariant_class, rb_intern("fromValue"), 1, item);
					if (item == Qnil) {
						continue;
					}
					o = value_obj_info(item);
				}

				if (!o || !o->ptr)
					continue;
				
				void *ptr = o->ptr;
				ptr = o->smoke->cast(
					ptr,				// pointer
					o->classId,				// from
					o->smoke->idClass(ItemSTR).index	        // to
				);
				cpplist->append(*(Item*)ptr);
			}

			m->item().s_voidp = cpplist;
			m->next();

			if (!m->type().isConst()) {
				rb_ary_clear(list);
				for(int i=0; i < cpplist->size(); ++i) {
					VALUE obj = getPointerObject((void*)&(cpplist->at(i)));
					rb_ary_push(list, obj);
				}
			}

			if (m->cleanup()) {
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

			int ix = m->smoke()->idClass(ItemSTR).index;
			const char * className = m->smoke()->binding->className(ix);

			for(int i=0; i < valuelist->size() ; ++i) {
				void *p = (void *) &(valuelist->at(i));

				if(m->item().s_voidp == 0) {
				*(m->var()) = Qnil;
				break;
				}

				VALUE obj = getPointerObject(p);
				if(obj == Qnil) {
					smokeruby_object  * o = alloc_smokeruby_object(	false, 
																	m->smoke(), 
																	m->smoke()->idClass(ItemSTR).index, 
																	p );
					obj = set_obj_info(className, o);
				}
		
				rb_ary_push(av, obj);
			}

			*(m->var()) = av;
			m->next();

			if (m->cleanup()) {
				delete valuelist;
			}

		}
		break;
      
		default:
			m->unsupported();
		break;
	}
}

#define DEF_VALUELIST_MARSHALLER(ListIdent,ItemList,Item) namespace { char ListIdent##STR[] = #Item; }  \
        Marshall::HandlerFn marshall_##ListIdent = marshall_ValueListItem<Item,ItemList,ListIdent##STR>;

DEF_VALUELIST_MARSHALLER( QColorVector, QVector<QColor>, QColor )
DEF_VALUELIST_MARSHALLER( QFileInfoList, QFileInfoList, QFileInfo )
DEF_VALUELIST_MARSHALLER( QHostAddressList, QList<QHostAddress>, QHostAddress )
DEF_VALUELIST_MARSHALLER( QImageTextKeyLangList, QList<QImageTextKeyLang>, QImageTextKeyLang )
DEF_VALUELIST_MARSHALLER( QKeySequenceList, QList<QKeySequence>, QKeySequence )
DEF_VALUELIST_MARSHALLER( QLineFVector, QVector<QLineF>, QLineF )
DEF_VALUELIST_MARSHALLER( QLineVector, QVector<QLine>, QLine )
DEF_VALUELIST_MARSHALLER( QModelIndexList, QList<QModelIndex>, QModelIndex )
DEF_VALUELIST_MARSHALLER( QNetworkAddressEntryList, QList<QNetworkAddressEntry>, QNetworkAddressEntry )
DEF_VALUELIST_MARSHALLER( QNetworkInterfaceList, QList<QNetworkInterface>, QNetworkInterface )
DEF_VALUELIST_MARSHALLER( QPixmapList, QList<QPixmap>, QPixmap )
DEF_VALUELIST_MARSHALLER( QPointFVector, QVector<QPointF>, QPointF )
DEF_VALUELIST_MARSHALLER( QPointVector, QVector<QPoint>, QPoint )
DEF_VALUELIST_MARSHALLER( QPolygonFList, QList<QPolygonF>, QPolygonF )
DEF_VALUELIST_MARSHALLER( QRectFList, QList<QRectF>, QRectF )
DEF_VALUELIST_MARSHALLER( QRectFVector, QVector<QRectF>, QRectF )
DEF_VALUELIST_MARSHALLER( QRectVector, QVector<QRect>, QRect )
DEF_VALUELIST_MARSHALLER( QRgbVector, QVector<QRgb>, QRgb )
DEF_VALUELIST_MARSHALLER( QTableWidgetSelectionRangeList, QList<QTableWidgetSelectionRange>, QTableWidgetSelectionRange )
DEF_VALUELIST_MARSHALLER( QTextBlockList, QList<QTextBlock>, QTextBlock )
DEF_VALUELIST_MARSHALLER( QTextFormatVector, QVector<QTextFormat>, QTextFormat )
DEF_VALUELIST_MARSHALLER( QTextLayoutFormatRangeList, QList<QTextLayout::FormatRange>, QTextLayout::FormatRange)
DEF_VALUELIST_MARSHALLER( QTextLengthVector, QVector<QTextLength>, QTextLength )
DEF_VALUELIST_MARSHALLER( QUrlList, QList<QUrl>, QUrl )
DEF_VALUELIST_MARSHALLER( QVariantList, QList<QVariant>, QVariant )
DEF_VALUELIST_MARSHALLER( QVariantVector, QVector<QVariant>, QVariant )

#if QT_VERSION >= 0x40300
DEF_VALUELIST_MARSHALLER( QSslCertificateList, QList<QSslCertificate>, QSslCertificate )
DEF_VALUELIST_MARSHALLER( QSslCipherList, QList<QSslCipher>, QSslCipher )
DEF_VALUELIST_MARSHALLER( QSslErrorList, QList<QSslError>, QSslError )
DEF_VALUELIST_MARSHALLER( QXmlStreamEntityDeclarations, QVector<QXmlStreamEntityDeclaration>, QXmlStreamEntityDeclaration )
DEF_VALUELIST_MARSHALLER( QXmlStreamNamespaceDeclarations, QVector<QXmlStreamNamespaceDeclaration>, QXmlStreamNamespaceDeclaration )
DEF_VALUELIST_MARSHALLER( QXmlStreamNotationDeclarations, QVector<QXmlStreamNotationDeclaration>, QXmlStreamNotationDeclaration )
#endif

#if QT_VERSION >= 0x40400
DEF_VALUELIST_MARSHALLER( QNetworkCookieList, QList<QNetworkCookie>, QNetworkCookie )
DEF_VALUELIST_MARSHALLER( QPrinterInfoList, QList<QPrinterInfo>, QPrinterInfo )
#endif

Q_DECL_EXPORT TypeHandler Qt_handlers[] = {
    { "bool*", marshall_it<bool *> },
    { "bool&", marshall_it<bool *> },
    { "char**", marshall_charP_array },
    { "char*",marshall_it<char *> },
    { "DOM::DOMTimeStamp", marshall_it<long long> },
    { "double*", marshall_doubleR },
    { "double&", marshall_doubleR },
    { "int*", marshall_it<int *> },
    { "int&", marshall_it<int *> },
    { "KIO::filesize_t", marshall_it<long long> },
    { "long long int", marshall_it<long long> },
    { "long long int&", marshall_it<long long> },
    { "QDBusVariant", marshall_QDBusVariant },
    { "QDBusVariant&", marshall_QDBusVariant },
    { "QFileInfoList", marshall_QFileInfoList },
    { "QGradiantStops", marshall_QPairqrealQColor },
    { "QGradiantStops&", marshall_QPairqrealQColor },
    { "unsigned int&", marshall_it<unsigned int *> },
    { "quint32&", marshall_it<unsigned int *> },
    { "uint&", marshall_it<unsigned int *> },
    { "qint32&", marshall_it<int *> },
    { "qint64", marshall_it<long long> },
    { "qint64&", marshall_it<long long> },
    { "QList<const char*>", marshall_QListCharStar },
    { "QList<int>", marshall_QListInt },
    { "QList<int>&", marshall_QListInt },
    { "QList<uint>", marshall_QListUInt },
    { "QList<uint>&", marshall_QListUInt },
    { "QList<QAbstractButton*>", marshall_QAbstractButtonList },
    { "QList<QActionGroup*>", marshall_QActionGroupList },
    { "QList<QAction*>", marshall_QActionList },
    { "QList<QAction*>&", marshall_QActionList },
    { "QList<QByteArray>", marshall_QByteArrayList },
    { "QList<QByteArray>*", marshall_QByteArrayList },
    { "QList<QByteArray>&", marshall_QByteArrayList },
    { "QList<QHostAddress>", marshall_QHostAddressList },
    { "QList<QHostAddress>&", marshall_QHostAddressList },
    { "QList<QImageTextKeyLang>", marshall_QImageTextKeyLangList },
    { "QList<QKeySequence>", marshall_QKeySequenceList },
    { "QList<QKeySequence>&", marshall_QKeySequenceList },
    { "QList<QListWidgetItem*>", marshall_QListWidgetItemList },
    { "QList<QListWidgetItem*>&", marshall_QListWidgetItemList },
    { "QList<QModelIndex>", marshall_QModelIndexList },
    { "QList<QModelIndex>&", marshall_QModelIndexList },
    { "QList<QNetworkAddressEntry>", marshall_QNetworkAddressEntryList },
    { "QList<QNetworkInterface>", marshall_QNetworkInterfaceList },
    { "QList<QPair<QString,QString> >", marshall_QPairQStringQStringList },
    { "QList<QPair<QString,QString> >&", marshall_QPairQStringQStringList },
    { "QList<QPixmap>", marshall_QPixmapList },
    { "QList<QPolygonF>", marshall_QPolygonFList },
    { "QList<QRectF>", marshall_QRectFList },
    { "QList<QRectF>&", marshall_QRectFList },
    { "QList<qreal>", marshall_QListqreal },
    { "QList<double>", marshall_QListqreal },
    { "QwtValueList", marshall_QListqreal },
    { "QwtValueList&", marshall_QListqreal },
    { "QList<double>&", marshall_QListqreal },
    { "QList<QStandardItem*>", marshall_QStandardItemList },
    { "QList<QStandardItem*>&", marshall_QStandardItemList },
    { "QList<QTableWidgetItem*>", marshall_QTableWidgetItemList },
    { "QList<QTableWidgetItem*>&", marshall_QTableWidgetItemList },
    { "QList<QTableWidgetSelectionRange>", marshall_QTableWidgetSelectionRangeList },
    { "QList<QTextBlock>", marshall_QTextBlockList },
    { "QList<QTextEdit::ExtraSelection>", marshall_ExtraSelectionList },
    { "QList<QTextEdit::ExtraSelection>&", marshall_ExtraSelectionList },
    { "QList<QTextFrame*>", marshall_QTextFrameList },
    { "QList<QTextLayout::FormatRange>", marshall_QTextLayoutFormatRangeList },
    { "QList<QTextLayout::FormatRange>&", marshall_QTextLayoutFormatRangeList },
    { "QList<QTreeWidgetItem*>", marshall_QTreeWidgetItemList },
    { "QList<QTreeWidgetItem*>&", marshall_QTreeWidgetItemList },
    { "QList<QUndoStack*>", marshall_QUndoStackList },
    { "QList<QUndoStack*>&", marshall_QUndoStackList },
    { "QList<QUrl>", marshall_QUrlList },
    { "QList<QUrl>&", marshall_QUrlList },
    { "QList<QVariant>", marshall_QVariantList },
    { "QList<QVariant>&", marshall_QVariantList },
    { "QList<QWidget*>", marshall_QWidgetPtrList },
    { "QList<QWidget*>&", marshall_QWidgetPtrList },
    { "qlonglong", marshall_it<long long> },
    { "qlonglong&", marshall_it<long long> },
    { "QMap<int,QVariant>", marshall_QMapintQVariant },
    { "QMap<int,QVariant>", marshall_QMapIntQVariant },
    { "QMap<int,QVariant>&", marshall_QMapIntQVariant },
    { "QMap<QString,QString>", marshall_QMapQStringQString },
    { "QMap<QString,QString>&", marshall_QMapQStringQString },
    { "QMap<QString,QVariant>", marshall_QMapQStringQVariant },
    { "QMap<QString,QVariant>&", marshall_QMapQStringQVariant },
    { "QVariantMap", marshall_QMapQStringQVariant },
    { "QVariantMap&", marshall_QMapQStringQVariant },
    { "QModelIndexList", marshall_QModelIndexList },
    { "QModelIndexList&", marshall_QModelIndexList },
    { "QObjectList", marshall_QObjectList },
    { "QObjectList&", marshall_QObjectList },
    { "QPair<int,int>&", marshall_QPairintint },
    { "Q_PID", marshall_it<Q_PID> },
    { "qreal*", marshall_doubleR },
    { "qreal&", marshall_doubleR },
    { "QRgb*", marshall_QRgb_array },
    { "QStringList", marshall_QStringList },
    { "QStringList*", marshall_QStringList },
    { "QStringList&", marshall_QStringList },
    { "QString", marshall_QString },
    { "QString*", marshall_QString },
    { "QString&", marshall_QString },
    { "quint64", marshall_it<unsigned long long> },
    { "quint64&", marshall_it<unsigned long long> },
    { "qulonglong", marshall_it<unsigned long long> },
    { "qulonglong&", marshall_it<unsigned long long> },
    { "QVariantList&", marshall_QVariantList },
    { "QVector<int>", marshall_QVectorint },
    { "QVector<int>&", marshall_QVectorint },
    { "QVector<QColor>", marshall_QColorVector },
    { "QVector<QColor>&", marshall_QColorVector },
    { "QVector<QLineF>", marshall_QLineFVector },
    { "QVector<QLineF>&", marshall_QLineFVector },
    { "QVector<QLine>", marshall_QLineVector },
    { "QVector<QLine>&", marshall_QLineVector },
    { "QVector<QPointF>", marshall_QPointFVector },
    { "QVector<QPointF>&", marshall_QPointFVector },
    { "QVector<QPoint>", marshall_QPointVector },
    { "QVector<QPoint>&", marshall_QPointVector },
    { "QVector<qreal>", marshall_QVectorqreal },
    { "QVector<qreal>&", marshall_QVectorqreal },
    { "QVector<QRectF>", marshall_QRectFVector },
    { "QVector<QRectF>&", marshall_QRectFVector },
    { "QVector<QRect>", marshall_QRectVector },
    { "QVector<QRect>&", marshall_QRectVector },
    { "QVector<QRgb>", marshall_QRgbVector },
    { "QVector<QRgb>&", marshall_QRgbVector },
    { "QVector<QTextFormat>", marshall_QTextFormatVector },
    { "QVector<QTextFormat>&", marshall_QTextFormatVector },
    { "QVector<QTextLength>", marshall_QTextLengthVector },
    { "QVector<QTextLength>&", marshall_QTextLengthVector },
    { "QVector<QVariant>", marshall_QVariantVector },
    { "QVector<QVariant>&", marshall_QVariantVector },
    { "QWidgetList", marshall_QWidgetList },
    { "QWidgetList&", marshall_QWidgetList },
    { "QwtArray<double>", marshall_QVectorqreal },
    { "QwtArray<double>&", marshall_QVectorqreal },
    { "QwtArray<int>", marshall_QVectorint },
    { "QwtArray<int>&", marshall_QVectorint },
    { "signed int&", marshall_it<int *> },
    { "uchar*", marshall_ucharP },
    { "unsigned long long int", marshall_it<long long> },
    { "unsigned long long int&", marshall_it<long long> },
    { "void", marshall_void },
    { "void**", marshall_voidP_array },
    { "WId", marshall_it<WId> },
#if QT_VERSION >= 0x40200
    { "QList<QGraphicsItem*>", marshall_QGraphicsItemList },
    { "QList<QGraphicsItem*>&", marshall_QGraphicsItemList },
    { "QList<QStandardItem*>", marshall_QStandardItemList },
    { "QList<QStandardItem*>&", marshall_QStandardItemList },
    { "QList<QUndoStack*>", marshall_QUndoStackList },
    { "QList<QUndoStack*>&", marshall_QUndoStackList },
#endif
#if QT_VERSION >= 0x40300
    { "QList<QMdiSubWindow*>", marshall_QMdiSubWindowList },
    { "QList<QSslCertificate>", marshall_QSslCertificateList },
    { "QList<QSslCertificate>&", marshall_QSslCertificateList },
    { "QList<QSslCipher>", marshall_QSslCipherList },
    { "QList<QSslCipher>&", marshall_QSslCipherList },
    { "QList<QSslError>", marshall_QSslErrorList },
    { "QList<QSslError>&", marshall_QSslErrorList },
    { "QXmlStreamEntityDeclarations", marshall_QXmlStreamEntityDeclarations },
    { "QXmlStreamNamespaceDeclarations", marshall_QXmlStreamNamespaceDeclarations },
    { "QXmlStreamNotationDeclarations", marshall_QXmlStreamNotationDeclarations },
#endif
#if QT_VERSION >= 0x040400
    { "QList<QNetworkCookie>", marshall_QNetworkCookieList },
    { "QList<QNetworkCookie>&", marshall_QNetworkCookieList },
    { "QList<QPrinterInfo>", marshall_QPrinterInfoList },
#endif
    { 0, 0 }
};

QHash<QByteArray, TypeHandler*> type_handlers;

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
