/*
 *   Copyright 2003-2013 by Richard Dale <richard.j.dale@gmail.com>

 *   Based on the PerlQt marshalling code by Ashley Winters

 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include <QtWidgets/QAbstractButton>
#include <QtWidgets/QAction>
#include <QtWidgets/QActionGroup>
#include <QtWidgets/QDockWidget>
#include <QtWidgets/QGraphicsItem>
#include <QtWidgets/QGraphicsView>
#include <QtWidgets/QGraphicsWidget>
#include <QtWidgets/QListWidgetItem>
#include <QtWidgets/QMdiSubWindow>
#include <QtWidgets/QTableWidgetSelectionRange>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QTreeWidgetItem>
#include <QtWidgets/QUndoStack>
#include <QtWidgets/QWizard>

#include "marshall.h"
#include "global.h"
#include "rubymetatype.h"

Q_DECLARE_METATYPE2(QList<QPair<double,double> >)
Q_DECLARE_METATYPE2(QList<QPair<double,QPointF> >)
Q_DECLARE_METATYPE2(QVector<QPair<double,QColor> >)
Q_DECLARE_METATYPE(QAbstractButton*)
Q_DECLARE_METATYPE(QAction*)
Q_DECLARE_METATYPE(QActionGroup*)
Q_DECLARE_METATYPE(QChar)
Q_DECLARE_METATYPE(QColor)
Q_DECLARE_METATYPE(QDockWidget*)
Q_DECLARE_METATYPE(QGraphicsView*)
Q_DECLARE_METATYPE(QGraphicsWidget*)
Q_DECLARE_METATYPE(QTableWidgetItem)
Q_DECLARE_METATYPE(QWizard::WizardButton)
Q_DECLARE_METATYPE(QListView::Flow)
Q_DECLARE_METATYPE(QListView::LayoutMode)
Q_DECLARE_METATYPE(QListView::Movement)
Q_DECLARE_METATYPE(QListView::ResizeMode)
Q_DECLARE_METATYPE(QListView::ViewMode)
Q_DECLARE_METATYPE(QListWidgetItem::ItemType)
Q_DECLARE_METATYPE(QList<QTableWidgetItem>)
Q_DECLARE_METATYPE(QList<QTextOption::Tab>)
Q_DECLARE_METATYPE(QList<QWizard::WizardButton>)
Q_DECLARE_METATYPE(QList<QListView::Flow>)
Q_DECLARE_METATYPE(QList<QListView::LayoutMode>)
Q_DECLARE_METATYPE(QList<QListView::Movement>)
Q_DECLARE_METATYPE(QList<QListView::ResizeMode>)
Q_DECLARE_METATYPE(QList<QListView::ViewMode>)
Q_DECLARE_METATYPE(QList<QListWidgetItem::ItemType>)
Q_DECLARE_METATYPE(QKeySequence)
Q_DECLARE_METATYPE(QList<QAbstractButton*>)
Q_DECLARE_METATYPE(QList<QAction*>)
Q_DECLARE_METATYPE(QList<QActionGroup*>)
Q_DECLARE_METATYPE(QList<QChar>)
Q_DECLARE_METATYPE(QList<QColor>)
Q_DECLARE_METATYPE(QList<QDockWidget*>)
Q_DECLARE_METATYPE(QList<QGraphicsItem*>)
Q_DECLARE_METATYPE(QList<QGraphicsView*>)
Q_DECLARE_METATYPE(QList<QGraphicsWidget*>)
Q_DECLARE_METATYPE(QList<QKeySequence>)
Q_DECLARE_METATYPE(QList<QListWidgetItem*>)
Q_DECLARE_METATYPE(QList<QMdiSubWindow*>)
Q_DECLARE_METATYPE(QList<QModelIndex>)
Q_DECLARE_METATYPE(QList<QPersistentModelIndex>)
Q_DECLARE_METATYPE(QList<QPolygonF>)
Q_DECLARE_METATYPE(QList<QTableWidgetItem*>)
Q_DECLARE_METATYPE(QList<QTableWidgetSelectionRange>)
Q_DECLARE_METATYPE(QList<QTextEdit::ExtraSelection>)
Q_DECLARE_METATYPE(QList<QTextFrame*>)
Q_DECLARE_METATYPE(QList<QTreeWidget*>)
Q_DECLARE_METATYPE(QList<QTreeWidgetItem*>)
Q_DECLARE_METATYPE(QList<QUndoStack*>)
Q_DECLARE_METATYPE(QList<QWidget*>)
Q_DECLARE_METATYPE(QListWidgetItem*)
Q_DECLARE_METATYPE(QLocale::Country)
Q_DECLARE_METATYPE(QMdiSubWindow*)
Q_DECLARE_METATYPE(QModelIndex)
Q_DECLARE_METATYPE(QPersistentModelIndex)
Q_DECLARE_METATYPE(QPolygonF)
Q_DECLARE_METATYPE(QTableWidgetItem*)
Q_DECLARE_METATYPE(QTableWidgetSelectionRange)
Q_DECLARE_METATYPE(QTextEdit::ExtraSelection)
Q_DECLARE_METATYPE(QTreeWidget*)
Q_DECLARE_METATYPE(QTreeWidgetItem*)
Q_DECLARE_METATYPE(QUndoStack*)
Q_DECLARE_METATYPE(QVector<QColor>)
Q_DECLARE_METATYPE(QVector<QTextFormat>)
Q_DECLARE_METATYPE(QVector<QTextLength>)

namespace QtRuby {

Marshall::TypeHandler QtWidgetsHandlers[] = {
    { "QList<QAbstractButton*>", marshall_Container<QList<QAbstractButton*> > },
    { "QList<QAbstractButton*>&", marshall_Container<QList<QAbstractButton*> > },
    { "QList<QActionGroup*>", marshall_Container<QList<QActionGroup*> > },
    { "QList<QActionGroup*>&", marshall_Container<QList<QActionGroup*> > },
    { "QList<QAction*>", marshall_Container<QList<QAction*> > },
    { "QList<QAction*>&", marshall_Container<QList<QAction*> > },
    { "QList<QChar>", marshall_Container<QList<QChar> > },
    { "QList<QColor>", marshall_Container<QList<QColor> > },
    { "QList<QColor>&", marshall_Container<QList<QColor> > },
    { "QList<QDockWidget*>", marshall_Container<QList<QDockWidget*> > },
    { "QList<QDockWidget*>&", marshall_Container<QList<QDockWidget*> > },
    { "QList<QGraphicsItem*>", marshall_Container<QList<QGraphicsItem*> > },
    { "QList<QGraphicsItem*>&", marshall_Container<QList<QGraphicsItem*> > },
    { "QList<QGraphicsView*>", marshall_Container<QList<QGraphicsView*> > },
    { "QList<QGraphicsView*>&", marshall_Container<QList<QGraphicsView*> > },
    { "QList<QGraphicsWidget*>", marshall_Container<QList<QGraphicsWidget*> > },
    { "QList<QGraphicsWidget*>&", marshall_Container<QList<QGraphicsWidget*> > },
    { "QList<QKeySequence>", marshall_Container<QList<QKeySequence> > },
    { "QList<QKeySequence>&", marshall_Container<QList<QKeySequence> > },
    { "QList<QKeySequence>&", marshall_Container<QList<QKeySequence> > },
    { "QList<QKeySequence>&", marshall_Container<QList<QKeySequence> > },
    { "QList<QListWidgetItem*>", marshall_Container<QList<QListWidgetItem*> > },
    { "QList<QListWidgetItem*>&", marshall_Container<QList<QListWidgetItem*> > },
    { "QList<QMdiSubWindow*>", marshall_Container<QList<QMdiSubWindow*> > },
    { "QList<QMdiSubWindow*>&", marshall_Container<QList<QMdiSubWindow*> > },
    { "QList<QModelIndex>", marshall_Container<QList<QModelIndex> > },
    { "QList<QModelIndex>&", marshall_Container<QList<QModelIndex> > },
    { "QList<QPair<double,double>>", marshall_Container<QList<QPair<double,double> > > },
    { "QList<QPair<double,QPointF>>", marshall_Container<QList<QPair<double,QPointF> > > },
    { "QList<QPersistentModelIndex>", marshall_Container<QList<QPersistentModelIndex> > },
    { "QList<QPolygonF>&", marshall_Container<QList<QPolygonF> > },
    { "QList<QTableWidgetItem*>", marshall_Container<QList<QTableWidgetItem*> > },
    { "QList<QTableWidgetItem*>&", marshall_Container<QList<QTableWidgetItem> > },
    { "QList<QTableWidgetSelectionRange>&", marshall_Container<QList<QTableWidgetSelectionRange> > },
    { "QList<QTextEdit::ExtraSelection>", marshall_Container<QList<QTextEdit::ExtraSelection> > },
    { "QList<QTextEdit::ExtraSelection>&", marshall_Container<QList<QTextEdit::ExtraSelection> > },
    { "QList<QTextFrame*>", marshall_Container<QList<QTextFrame*> > },
    { "QList<QTextFrame*>&", marshall_Container<QList<QTextFrame*> > },
    { "QList<QTextOption::Tab>", marshall_Container<QList<QTextOption::Tab> > },
    { "QList<QTextOption::Tab>&", marshall_Container<QList<QTextOption::Tab> > },
    { "QList<QTreeWidgetItem*>", marshall_Container<QList<QTreeWidgetItem*> > },
    { "QList<QTreeWidgetItem*>&", marshall_Container<QList<QTreeWidgetItem*> > },
    { "QList<QTreeWidget*>", marshall_Container<QList<QTreeWidget*> > },
    { "QList<QTreeWidget*>&", marshall_Container<QList<QTreeWidget*> > },
    { "QList<QUndoStack*>", marshall_Container<QList<QUndoStack*> > },
    { "QList<QUndoStack*>&", marshall_Container<QList<QUndoStack*> > },
    { "QList<QWidget*>", marshall_Container<QList<QWidget*> > },
    { "QList<QWidget*>&", marshall_Container<QList<QWidget*> > },
    { "QList<QWidget*>&", marshall_Container<QList<QWidget*> > },
    { "QList<QWizard::WizardButton>&", marshall_Container<QList<QWizard::WizardButton> > },
    { "QList<QListView::Flow>", marshall_Container<QList<QListView::Flow> > },
    { "QList<QListView::LayoutMode>", marshall_Container<QList<QListView::LayoutMode> > },
    { "QList<QListView::Movement>", marshall_Container<QList<QListView::Movement> > },
    { "QList<QListView::ResizeMode>", marshall_Container<QList<QListView::ResizeMode> > },
    { "QList<QListView::ViewMode>", marshall_Container<QList<QListView::ViewMode> > },
    { "QList<QListWidgetItem::ItemType>", marshall_Container<QList<QListWidgetItem::ItemType> > },
    { "QVector<QColor>", marshall_Container<QVector<QColor> > },
    { "QVector<QPair<double,QColor>>", marshall_Container<QVector<QPair<double,QColor> > > },
    { "QVector<QPair<double,QColor>>&", marshall_Container<QVector<QPair<double,QColor> > > },
    { "QVector<QTextFormat>", marshall_Container<QVector<QTextFormat> > },
    { "QVector<QTextLength>", marshall_Container<QVector<QTextLength> > },
    { "QVector<QTextLength>&", marshall_Container<QVector<QTextLength> > },

    { 0, 0 }
};

void registerQtWidgetsTypes()
{
    qRubyRegisterSequenceMetaType<QList<QTableWidgetItem> >();
    qRubyRegisterSequenceMetaType<QList<QTextOption::Tab> >();
    qRubyRegisterSequenceMetaType<QList<QWizard::WizardButton> >();
    qRubyRegisterSequenceMetaType<QList<QListView::Flow> >();
    qRubyRegisterSequenceMetaType<QList<QListView::LayoutMode> >();
    qRubyRegisterSequenceMetaType<QList<QListView::Movement> >();
    qRubyRegisterSequenceMetaType<QList<QListView::ResizeMode> >();
    qRubyRegisterSequenceMetaType<QList<QListView::ViewMode> >();
    qRubyRegisterSequenceMetaType<QList<QListWidgetItem::ItemType> >();

    qRubySmokeRegisterPairSequenceMetaType<QList<QPair<double,double> > >();
    qRubySmokeRegisterPairSequenceMetaType<QList<QPair<double,QPointF> > >();
    qRubySmokeRegisterPairSequenceMetaType<QVector<QPair<double,QColor> > >();
    qRubySmokeRegisterPointerSequenceMetaType<QList<QAbstractButton*> >();
    qRubySmokeRegisterPointerSequenceMetaType<QList<QAction*> >();
    qRubySmokeRegisterPointerSequenceMetaType<QList<QActionGroup*> >();
    qRubySmokeRegisterPointerSequenceMetaType<QList<QDockWidget*> >();
    qRubySmokeRegisterPointerSequenceMetaType<QList<QGraphicsItem*> >();
    qRubySmokeRegisterPointerSequenceMetaType<QList<QGraphicsView*> >();
    qRubySmokeRegisterPointerSequenceMetaType<QList<QGraphicsWidget*> >();
    qRubySmokeRegisterPointerSequenceMetaType<QList<QListWidgetItem*> >();
    qRubySmokeRegisterPointerSequenceMetaType<QList<QMdiSubWindow*> >();
    qRubySmokeRegisterPointerSequenceMetaType<QList<QTableWidgetItem*> >();
    qRubySmokeRegisterPointerSequenceMetaType<QList<QTreeWidget*> >();
    qRubySmokeRegisterPointerSequenceMetaType<QList<QTreeWidgetItem*> >();
    qRubySmokeRegisterPointerSequenceMetaType<QList<QUndoStack*> >();
    qRubySmokeRegisterPointerSequenceMetaType<QList<QWidget*> >();
    qRubySmokeRegisterSequenceMetaType<QList<QChar> >();
    qRubySmokeRegisterSequenceMetaType<QList<QColor> >();
    qRubySmokeRegisterSequenceMetaType<QList<QKeySequence> >();
    qRubySmokeRegisterSequenceMetaType<QList<QModelIndex> >();
    qRubySmokeRegisterSequenceMetaType<QList<QPersistentModelIndex> >();
    qRubySmokeRegisterSequenceMetaType<QList<QPolygonF> >();
    qRubySmokeRegisterSequenceMetaType<QList<QTableWidgetSelectionRange> >();
    qRubySmokeRegisterSequenceMetaType<QList<QTextEdit::ExtraSelection> >();
    qRubySmokeRegisterSequenceMetaType<QVector<QColor> >();
    qRubySmokeRegisterSequenceMetaType<QVector<QTextFormat> >();
    qRubySmokeRegisterSequenceMetaType<QVector<QTextLength> >();

    return;
}

}

// kate: space-indent on; indent-width 4; replace-tabs on; mixed-indent off;
