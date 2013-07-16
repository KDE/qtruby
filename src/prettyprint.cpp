/*
 * Copyright 2003-2013 by Richard Dale <richard.j.dale@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) version 3, or any
 * later version accepted by the membership of KDE e.V. (or its
 * successor approved by the membership of KDE e.V.), which shall
 * act as a proxy defined in Section 6 of version 3 of the license.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <QtCore/QObject>
#include <QtCore/QMetaProperty>
#include <QtCore/QVariant>

#include <QtGui/QColor>
#include <QtGui/QCursor>
#include <QtGui/QFont>
#include <QtGui/QWidget>

#include "prettyprint.h"
#include "object.h"
#include "global.h"

namespace QtRuby {

// Takes a variable name and a QProperty with QVariant value, and returns a '
// variable=value' pair with the value in ruby inspect style
static QString
inspectProperty(QMetaProperty property, const char * name, QVariant & value)
{
    if (property.isEnumType()) {
        QMetaEnum e = property.enumerator();
        return QString(" %1=%2::%3").arg(name).arg(e.scope()).arg(e.valueToKey(value.toInt()));
    }

    switch (value.type()) {
    case QVariant::String:
    {
        if (value.toString().isNull()) {
            return QString(" %1=nil").arg(name);
        } else {
            return QString(" %1=%2").arg(name).arg(value.toString());
        }
    }

    case QVariant::Bool:
    {
        QString rubyName;
        QRegExp name_re("^(is|has)(.)(.*)");

        if (name_re.indexIn(name) != -1) {
            rubyName = name_re.cap(2).toLower() + name_re.cap(3) + "?";
        } else {
            rubyName = name;
        }

        return QString(" %1=%2").arg(rubyName).arg(value.toString());
    }

    case QVariant::Color:
    {
        QColor c = value.value<QColor>();
        return QString(" %1=#<Qt::Color:0x0 %2>").arg(name).arg(c.name());
    }

    case QVariant::Cursor:
    {
        QCursor c = value.value<QCursor>();
        return QString(" %1=#<Qt::Cursor:0x0 shape=%2>").arg(name).arg(c.shape());
    }

    case QVariant::Double:
    {
        return QString(" %1=%2").arg(name).arg(value.toDouble());
    }

    case QVariant::Font:
    {
        QFont f = value.value<QFont>();
        return QString( " %1=#<Qt::Font:0x0 family=%2, pointSize=%3, weight=%4, italic=%5, bold=%6, underline=%7, strikeOut=%8>")
                                    .arg(name)
                                    .arg(f.family())
                                    .arg(f.pointSize())
                                    .arg(f.weight())
                                    .arg(f.italic() ? "true" : "false")
                                    .arg(f.bold() ? "true" : "false")
                                    .arg(f.underline() ? "true" : "false")
                                    .arg(f.strikeOut() ? "true" : "false");
    }

    case QVariant::Line:
    {
        QLine l = value.toLine();
        return QString(" %1=#<Qt::Line:0x0 x1=%2, y1=%3, x2=%4, y2=%5>")
                        .arg(name)
                        .arg(l.x1())
                        .arg(l.y1())
                        .arg(l.x2())
                        .arg(l.y2());
    }

    case QVariant::LineF:
    {
        QLineF l = value.toLineF();
        return QString(" %1=#<Qt::LineF:0x0 x1=%2, y1=%3, x2=%4, y2=%5>")
                        .arg(name)
                        .arg(l.x1())
                        .arg(l.y1())
                        .arg(l.x2())
                        .arg(l.y2());
    }

    case QVariant::Point:
    {
        QPoint p = value.toPoint();
        return QString(" %1=#<Qt::Point:0x0 x=%2, y=%3>").arg(name).arg(p.x()).arg(p.y());
    }

    case QVariant::PointF:
    {
        QPointF p = value.toPointF();
        return QString(" %1=#<Qt::PointF:0x0 x=%2, y=%3>").arg(name).arg(p.x()).arg(p.y());
    }

    case QVariant::Rect:
    {
        QRect r = value.toRect();
        return QString(" %1=#<Qt::Rect:0x0 left=%2, right=%3, top=%4, bottom=%5>")
                                    .arg(name)
                                    .arg(r.left()).arg(r.right()).arg(r.top()).arg(r.bottom());
    }

    case QVariant::RectF:
    {
        QRectF r = value.toRectF();
        return QString(" %1=#<Qt::RectF:0x0 left=%2, right=%3, top=%4, bottom=%5>")
                                    .arg(name)
                                    .arg(r.left()).arg(r.right()).arg(r.top()).arg(r.bottom());
    }

    case QVariant::Size:
    {
        QSize s = value.toSize();
        return QString(" %1=#<Qt::Size:0x0 width=%2, height=%3>")
                                    .arg(name)
                                    .arg(s.width()).arg(s.height());
    }

    case QVariant::SizeF:
    {
        QSizeF s = value.toSizeF();
        return QString(" %1=#<Qt::SizeF:0x0 width=%2, height=%3>")
                                    .arg(name)
                                    .arg(s.width()).arg(s.height());
    }

    case QVariant::SizePolicy:
    {
        QSizePolicy s = value.value<QSizePolicy>();
        return QString(" %1=#<Qt::SizePolicy:0x0 horizontalPolicy=%2, verticalPolicy=%3>")
                                    .arg(name)
                                    .arg(s.horizontalPolicy())
                                    .arg(s.verticalPolicy());
    }

    case QVariant::Brush:
//  case QVariant::ColorGroup:
    case QVariant::Image:
    case QVariant::Palette:
    case QVariant::Pixmap:
    case QVariant::Region:
    {
        return QString(" %1=#<Qt::%2:0x0>").arg(name).arg(value.typeName() + 1);
    }

    default:
        return QString(" %1=%2").arg(name)
                                    .arg((value.isNull() || value.toString().isNull()) ? "nil" : value.toString() );
    }
}

// Retrieves the properties for a QObject and returns them as 'name=value' pairs
// in a ruby inspect string. For example:
//
//      #<Qt::HBoxLayout:0x30139030 name=unnamed, margin=0, spacing=0, resizeMode=3>
//
VALUE
inspect_qobject(VALUE self)
{
    if (TYPE(self) != T_DATA) {
        return Qnil;
    }

    // Start with #<Qt::HBoxLayout:0x30139030> from the original inspect() call
    // Drop the closing '>'
    VALUE inspect_str = rb_call_super(0, 0);
    rb_str_resize(inspect_str, RSTRING_LEN(inspect_str) - 1);

    Object::Instance * instance = Object::Instance::get(self);
    QObject * qobject = reinterpret_cast<QObject*>(instance->cast(QtRuby::Global::QObjectClassId));

    QString value_list;
    value_list.append(QString(" objectName=\"%1\"").arg(qobject->objectName()));

    if (qobject->isWidgetType()) {
        QWidget * w = (QWidget *) qobject;
        value_list.append(QString(", x=%1, y=%2, width=%3, height=%4")
                                                .arg(w->x())
                                                .arg(w->y())
                                                .arg(w->width())
                                                .arg(w->height()) );
    }

    value_list.append(">");
    rb_str_cat2(inspect_str, value_list.toLatin1());

    return inspect_str;
}

// Retrieves the properties for a QObject and pretty_prints them as 'name=value' pairs
// For example:
//
//      #<Qt::HBoxLayout:0x30139030
//       name=unnamed,
//       margin=0,
//       spacing=0,
//       resizeMode=3>
//
VALUE
pretty_print_qobject(VALUE self, VALUE pp)
{
    if (TYPE(self) != T_DATA) {
        return Qnil;
    }

    // Start with #<Qt::HBoxLayout:0x30139030>
    // Drop the closing '>'
    VALUE inspect_str = rb_funcall(self, rb_intern("to_s"), 0, 0);
    rb_str_resize(inspect_str, RSTRING_LEN(inspect_str) - 1);
    rb_funcall(pp, rb_intern("text"), 1, inspect_str);
    rb_funcall(pp, rb_intern("breakable"), 0);

    Object::Instance * instance = Object::Instance::get(self);
    QObject * qobject = reinterpret_cast<QObject*>(instance->cast(QtRuby::Global::QObjectClassId));

    QString value_list;

    if (qobject->parent() != 0) {
        QString parentInspectString;
        VALUE obj = Global::getRubyValue(qobject->parent());
        if (obj != Qnil) {
            VALUE parent_inspect_str = rb_funcall(obj, rb_intern("to_s"), 0, 0);
            rb_str_resize(parent_inspect_str, RSTRING_LEN(parent_inspect_str) - 1);
            parentInspectString = StringValuePtr(parent_inspect_str);
        } else {
            parentInspectString.sprintf("#<%s:0x0", qobject->parent()->metaObject()->className());
        }

        if (qobject->parent()->isWidgetType()) {
            QWidget * w = (QWidget *) qobject->parent();
            value_list = QString("  parent=%1 objectName=\"%2\", x=%3, y=%4, width=%5, height=%6>,\n")
                                                .arg(parentInspectString)
                                                .arg(w->objectName())
                                                .arg(w->x())
                                                .arg(w->y())
                                                .arg(w->width())
                                                .arg(w->height());
        } else {
            value_list = QString("  parent=%1 objectName=\"%2\">,\n")
                                                .arg(parentInspectString)
                                                .arg(qobject->parent()->objectName());
        }

        rb_funcall(pp, rb_intern("text"), 1, rb_str_new2(value_list.toLatin1()));
    }

    if (qobject->children().count() != 0) {
        value_list = QString("  children=Array (%1 element(s)),\n")
                                .arg(qobject->children().count());
        rb_funcall(pp, rb_intern("text"), 1, rb_str_new2(value_list.toLatin1()));
    }

    value_list = QString("  metaObject=#<Qt::MetaObject:0x0");
    value_list.append(QString(" className=%1").arg(qobject->metaObject()->className()));

    if (qobject->metaObject()->superClass() != 0) {
        value_list.append(  QString(", superClass=#<Qt::MetaObject:0x0 className=%1>")
                            .arg(qobject->metaObject()->superClass()->className()) );
    }

    value_list.append(">,\n");
    rb_funcall(pp, rb_intern("text"), 1, rb_str_new2(value_list.toLatin1()));

    QMetaProperty property = qobject->metaObject()->property(0);
    QVariant value = property.read(qobject);
    value_list = " " + inspectProperty(property, property.name(), value);
    rb_funcall(pp, rb_intern("text"), 1, rb_str_new2(value_list.toLatin1()));

    for (int index = 1; index < qobject->metaObject()->propertyCount(); index++) {
        rb_funcall(pp, rb_intern("text"), 1, rb_str_new2(",\n"));

        property = qobject->metaObject()->property(index);
        value = property.read(qobject);
        value_list = " " + inspectProperty(property, property.name(), value);
        rb_funcall(pp, rb_intern("text"), 1, rb_str_new2(value_list.toLatin1()));
    }

    rb_funcall(pp, rb_intern("text"), 1, rb_str_new2(">"));

    return self;
}

}

// kate: space-indent on; indent-width 4; replace-tabs on; mixed-indent off;

