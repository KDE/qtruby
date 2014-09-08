/*
 *   Copyright 2009-2011 by Richard Dale <richard.j.dale@gmail.com>

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

#include <QDebug>
#include <QApplication>

#include <global.h>
#include <marshall.h>

#include <smoke/qt5/qtwidgets_smoke.h>

#include "typeresolver.h"
#include "rubyqpainter.h"

namespace QtRuby {
extern Marshall::TypeHandler QtWidgetsHandlers[];
extern void registerQtWidgetsTypes();

static void initializeClasses(Smoke * smoke)
{
    Global::QGraphicsItemClassId = qtwidgets_Smoke->idClass("QGraphicsItem");
    Global::defineTypeResolver(Global::QGraphicsItemClassId, qgraphicsitemTypeResolver);

    for (int i = 1; i <= smoke->numClasses; i++) {
        Smoke::ModuleIndex classId(smoke, i);
        QString className = QString::fromLatin1(smoke->classes[i].className);

        if (    smoke->classes[i].external
                || className.contains("Internal")
                || className == "Qt" )
        {
            continue;
        }

        if (className.startsWith("Q"))
            className = className.mid(1).prepend("Qt::");

        VALUE klass = Global::initializeClass(classId, className);

        if (className == "Qt::Painter") {
            rb_define_method(klass, "drawLines", (VALUE (*) (...)) qpainter_drawlines, -1);
            rb_define_method(klass, "draw_lines", (VALUE (*) (...)) qpainter_drawlines, -1);
            rb_define_method(klass, "drawRects", (VALUE (*) (...)) qpainter_drawrects, -1);
            rb_define_method(klass, "draw_rects", (VALUE (*) (...)) qpainter_drawrects, -1);
        }
    }
}

}

extern "C" {

Q_DECL_EXPORT void
Init_qtwidgets()
{
    init_qtwidgets_Smoke();
    QtRuby::Module qtwidgets_module = { "qtwidgets", new QtRuby::Binding(qtwidgets_Smoke) };
    QtRuby::Global::modules[qtwidgets_Smoke] = qtwidgets_module;
    QtRuby::registerQtWidgetsTypes();
    QtRuby::Marshall::installHandlers(QtRuby::QtWidgetsHandlers);
    QtRuby::initializeClasses(qtwidgets_Smoke);
    rb_require("qtwidgets/qtwidgets.rb");

    return;
}

}
