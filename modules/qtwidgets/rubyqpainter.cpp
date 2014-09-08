/*
 *   Copyright 2003-2013 by Richard Dale <richard.j.dale@gmail.com>

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

#include <QtGui/QPainter>

#include <object.h>
#include <global.h>
#include <methodcall.h>

#include "rubyqpainter.h"

namespace QtRuby {

// The QtRuby runtime's overloaded method resolution mechanism can't currently
// distinguish between Ruby Arrays containing different sort of instances.
// Unfortunately Qt::Painter.drawLines() and Qt::Painter.drawRects() methods can
// be passed a Ruby Array as an argument containing either Qt::Points or Qt::PointFs
// for instance. These methods need to call the correct Qt C++ methods, so special case
// the overload method resolution for now..
VALUE
qpainter_drawlines(int argc, VALUE * argv, VALUE self)
{
static Smoke::Index drawlines_pointf_vector = 0;
static Smoke::Index drawlines_point_vector = 0;
static Smoke::Index drawlines_linef_vector = 0;
static Smoke::Index drawlines_line_vector = 0;

    if (argc == 1 && TYPE(argv[0]) == T_ARRAY && RARRAY_LEN(argv[0]) > 0) {
        Object::Instance * instance = Object::Instance::get(self);

        if (drawlines_point_vector == 0) {
            Smoke::ModuleIndex nameId = instance->classId.smoke->findMethodName("QPainter", "drawLines?");
            Smoke::ModuleIndex meth = instance->classId.smoke->findMethod(instance->classId, nameId);
            Smoke::Index i = meth.smoke->methodMaps[meth.index].method;
            i = -i;     // turn into ambiguousMethodList index
            while (meth.smoke->ambiguousMethodList[i] != 0) {
                const char * argType = meth.smoke->types[meth.smoke->argumentList[meth.smoke->methods[meth.smoke->ambiguousMethodList[i]].args]].name;

                if (qstrcmp(argType, "const QVector<QPointF>&" ) == 0) {
                    drawlines_pointf_vector = meth.smoke->ambiguousMethodList[i];
                } else if (qstrcmp(argType, "const QVector<QPoint>&" ) == 0) {
                    drawlines_point_vector = meth.smoke->ambiguousMethodList[i];
                } else if (qstrcmp(argType, "const QVector<QLineF>&" ) == 0) {
                    drawlines_linef_vector = meth.smoke->ambiguousMethodList[i];
                } else if (qstrcmp(argType, "const QVector<QLine>&" ) == 0) {
                    drawlines_line_vector = meth.smoke->ambiguousMethodList[i];
                }

                i++;
            }
        }

        //smokeruby_object * o = value_obj_info(rb_ary_entry(argv[0], 0));
        Object::Instance * args = Object::Instance::get(argv[0]);
        QVector<Smoke::ModuleIndex> methodIds;

        if (qstrcmp(instance->className(), "QPointF") == 0) {
            methodIds << Smoke::ModuleIndex(instance->classId.smoke, drawlines_pointf_vector);
        } else if (qstrcmp(instance->className(), "QPoint") == 0) {
            methodIds << Smoke::ModuleIndex(instance->classId.smoke, drawlines_point_vector);
        } else if (qstrcmp(instance->className(), "QLineF") == 0) {
            methodIds << Smoke::ModuleIndex(instance->classId.smoke, drawlines_linef_vector);
        } else if (qstrcmp(instance->className(), "QLine") == 0) {
            methodIds << Smoke::ModuleIndex(instance->classId.smoke, drawlines_line_vector);
        } else {
            return rb_call_super(argc, argv);
        }

        methodIds << Smoke::NullModuleIndex;
        MethodCall c(methodIds, self, argv);
        c.next();
        return self;
    }

    return rb_call_super(argc, argv);
}

VALUE
qpainter_drawrects(int argc, VALUE * argv, VALUE self)
{
static Smoke::Index drawrects_rectf_vector = 0;
static Smoke::Index drawrects_rect_vector = 0;

    if (argc == 1 && TYPE(argv[0]) == T_ARRAY && RARRAY_LEN(argv[0]) > 0) {
        Object::Instance * instance = Object::Instance::get(self);

        if (drawrects_rectf_vector == 0) {
            Smoke::ModuleIndex nameId = instance->classId.smoke->findMethodName("QPainter", "drawRects?");
            Smoke::ModuleIndex meth = instance->classId.smoke->findMethod(instance->classId, nameId);
            Smoke::Index i = meth.smoke->methodMaps[meth.index].method;
            i = -i;     // turn into ambiguousMethodList index
            while (meth.smoke->ambiguousMethodList[i] != 0) {
                const char * argType = meth.smoke->types[meth.smoke->argumentList[meth.smoke->methods[meth.smoke->ambiguousMethodList[i]].args]].name;

                if (qstrcmp(argType, "const QVector<QRectF>&" ) == 0) {
                    drawrects_rectf_vector = meth.smoke->ambiguousMethodList[i];
                } else if (qstrcmp(argType, "const QVector<QRect>&" ) == 0) {
                    drawrects_rect_vector = meth.smoke->ambiguousMethodList[i];
                }

                i++;
            }
        }

        Object::Instance * arg = Object::Instance::get(argv[0]);
        QVector<Smoke::ModuleIndex> methodIds;

        if (qstrcmp(instance->className(), "QRectF") == 0) {
            methodIds << Smoke::ModuleIndex(instance->classId.smoke, drawrects_rectf_vector);
        } else if (qstrcmp(instance->className(), "QRect") == 0) {
            methodIds << Smoke::ModuleIndex(instance->classId.smoke, drawrects_rect_vector);
        } else {
            return rb_call_super(argc, argv);
        }

        methodIds << Smoke::NullModuleIndex;
        MethodCall c(methodIds, self, argv);
        c.next();
        return self;
    }

    return rb_call_super(argc, argv);
}

}