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

#include <QtCore/QLocale>

#include <QtGui/QAbstractTextDocumentLayout>
#include <QtGui/QColor>
#include <QtGui/QFontDatabase>
#include <QtGui/QImageTextKeyLang>
#include <QtGui/QKeySequence>
#include <QtGui/QPolygon>
#include <QtGui/QStandardItem>
#include <QtGui/QTextBlock>
#include <QtGui/QTextFormat>
#include <QtGui/QTextLayout>
#include <QtGui/QTextLength>

#include "marshall.h"
#include "global.h"
#include "rubymetatype.h"

Q_DECLARE_METATYPE2(QList<QPair<double,double> >)
Q_DECLARE_METATYPE2(QList<QPair<double,QPointF> >)
Q_DECLARE_METATYPE2(QVector<QPair<double,QColor> >)
Q_DECLARE_METATYPE(QAbstractTextDocumentLayout::Selection)
Q_DECLARE_METATYPE(QChar)
Q_DECLARE_METATYPE(QColor)
Q_DECLARE_METATYPE(QFontDatabase::WritingSystem)
Q_DECLARE_METATYPE(QList<QFontDatabase::WritingSystem>)
Q_DECLARE_METATYPE(QList<QTextOption::Tab>)
Q_DECLARE_METATYPE(QKeySequence)
Q_DECLARE_METATYPE(QList<QAction*>)
Q_DECLARE_METATYPE(QList<QChar>)
Q_DECLARE_METATYPE(QList<QColor>)
Q_DECLARE_METATYPE(QList<QKeySequence>)
Q_DECLARE_METATYPE(QList<QModelIndex>)
Q_DECLARE_METATYPE(QList<QPersistentModelIndex>)
Q_DECLARE_METATYPE(QList<QPolygonF>)
Q_DECLARE_METATYPE(QList<QStandardItem*>)
Q_DECLARE_METATYPE(QList<QTextBlock>)
Q_DECLARE_METATYPE(QList<QTextFrame*>)
Q_DECLARE_METATYPE(QList<QTextLayout::FormatRange>)
Q_DECLARE_METATYPE(QList<QWidget*>)
Q_DECLARE_METATYPE(QLocale::Country)
Q_DECLARE_METATYPE(QModelIndex)
Q_DECLARE_METATYPE(QPersistentModelIndex)
Q_DECLARE_METATYPE(QPolygonF)
Q_DECLARE_METATYPE(QStandardItem*)
Q_DECLARE_METATYPE(QTextBlock)
Q_DECLARE_METATYPE(QTextFrame*)
Q_DECLARE_METATYPE(QTextLayout::FormatRange)
Q_DECLARE_METATYPE(QVector<QAbstractTextDocumentLayout::Selection>)
Q_DECLARE_METATYPE(QVector<QColor>)
Q_DECLARE_METATYPE(QVector<QTextFormat>)
Q_DECLARE_METATYPE(QVector<QTextLength>)

namespace QtRuby {

Marshall::TypeHandler QtGuiHandlers[] = {
    { "QList<QAction*>", marshall_Container<QList<QAction*> > },
    { "QList<QAction*>&", marshall_Container<QList<QAction*> > },
    { "QList<QChar>", marshall_Container<QList<QChar> > },
    { "QList<QColor>", marshall_Container<QList<QColor> > },
    { "QList<QColor>&", marshall_Container<QList<QColor> > },
    { "QList<QFontDatabase::WritingSystem>", marshall_Container<QList<QFontDatabase::WritingSystem> > },
    { "QList<QKeySequence>", marshall_Container<QList<QKeySequence> > },
    { "QList<QKeySequence>&", marshall_Container<QList<QKeySequence> > },
    { "QList<QKeySequence>&", marshall_Container<QList<QKeySequence> > },
    { "QList<QKeySequence>&", marshall_Container<QList<QKeySequence> > },
    { "QList<QModelIndex>", marshall_Container<QList<QModelIndex> > },
    { "QList<QModelIndex>&", marshall_Container<QList<QModelIndex> > },
    { "QList<QPair<double,double>>", marshall_Container<QList<QPair<double,double> > > },
    { "QList<QPair<double,QPointF>>", marshall_Container<QList<QPair<double,QPointF> > > },
    { "QList<QPersistentModelIndex>", marshall_Container<QList<QPersistentModelIndex> > },
    { "QList<QPolygonF>&", marshall_Container<QList<QPolygonF> > },
    { "QList<QStandardItem*>", marshall_Container<QList<QStandardItem*> > },
    { "QList<QStandardItem*>&", marshall_Container<QList<QStandardItem*> > },
    { "QList<QTextBlock>&", marshall_Container<QList<QTextBlock> > },
    { "QList<QTextFrame*>", marshall_Container<QList<QTextFrame*> > },
    { "QList<QTextFrame*>&", marshall_Container<QList<QTextFrame*> > },
    { "QList<QTextLayout::FormatRange>", marshall_Container<QList<QTextLayout::FormatRange> > },
    { "QList<QTextLayout::FormatRange>&", marshall_Container<QList<QTextLayout::FormatRange> > },
    { "QList<QTextOption::Tab>", marshall_Container<QList<QTextOption::Tab> > },
    { "QList<QTextOption::Tab>&", marshall_Container<QList<QTextOption::Tab> > },
    { "QVector<QAbstractTextDocumentLayout::Selection>", marshall_Container<QVector<QAbstractTextDocumentLayout::Selection> > },
    { "QVector<QAbstractTextDocumentLayout::Selection>&", marshall_Container<QVector<QAbstractTextDocumentLayout::Selection> > },
    { "QVector<QColor>", marshall_Container<QVector<QColor> > },
    { "QVector<QPair<double,QColor>>", marshall_Container<QVector<QPair<double,QColor> > > },
    { "QVector<QPair<double,QColor>>&", marshall_Container<QVector<QPair<double,QColor> > > },
    { "QVector<QTextFormat>", marshall_Container<QVector<QTextFormat> > },
    { "QVector<QTextLength>", marshall_Container<QVector<QTextLength> > },
    { "QVector<QTextLength>&", marshall_Container<QVector<QTextLength> > },

    { 0, 0 }
};

void registerQtGuiTypes()
{
    qRubyRegisterSequenceMetaType<QList<QFontDatabase::WritingSystem> >();
    qRubyRegisterSequenceMetaType<QList<QTextOption::Tab> >();

    qRubySmokeRegisterPairSequenceMetaType<QList<QPair<double,double> > >();
    qRubySmokeRegisterPairSequenceMetaType<QList<QPair<double,QPointF> > >();
    qRubySmokeRegisterPairSequenceMetaType<QVector<QPair<double,QColor> > >();
    qRubySmokeRegisterPointerSequenceMetaType<QList<QStandardItem*> >();
    qRubySmokeRegisterPointerSequenceMetaType<QList<QTextFrame*> >();
    qRubySmokeRegisterSequenceMetaType<QList<QChar> >();
    qRubySmokeRegisterSequenceMetaType<QList<QColor> >();
    qRubySmokeRegisterSequenceMetaType<QList<QKeySequence> >();
    qRubySmokeRegisterSequenceMetaType<QList<QModelIndex> >();
    qRubySmokeRegisterSequenceMetaType<QList<QPersistentModelIndex> >();
    qRubySmokeRegisterSequenceMetaType<QList<QPolygonF> >();
    qRubySmokeRegisterSequenceMetaType<QList<QTextBlock> >();
    qRubySmokeRegisterSequenceMetaType<QList<QTextLayout::FormatRange> >();
    qRubySmokeRegisterSequenceMetaType<QVector<QColor> >();
    qRubySmokeRegisterSequenceMetaType<QVector<QTextFormat> >();
    qRubySmokeRegisterSequenceMetaType<QVector<QTextLength> >();

    return;
}

}

// kate: space-indent on; indent-width 4; replace-tabs on; mixed-indent off;
