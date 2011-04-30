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

#include <QtCore/qdebug.h>

#include <global.h>
#include <marshall.h>

#include <smoke/qtgui_smoke.h>

#include "typeresolver.h"

namespace QtRuby {
extern Marshall::TypeHandler QtGuiHandlers[];
extern void registerQtGuiTypes();
}

extern "C" {

Q_DECL_EXPORT void
Init_qtgui()
{
    init_qtgui_Smoke();
    QtRuby::Module qtgui_module = { "qtgui", new QtRuby::Binding(qtgui_Smoke) };
    QtRuby::Global::modules[qtgui_Smoke] = qtgui_module;
    QtRuby::Global::QGraphicsItemClassId = qtgui_Smoke->idClass("QGraphicsItem");
    QtRuby::Marshall::installHandlers(QtRuby::QtGuiHandlers);
    QtRuby::Global::defineTypeResolver(QtRuby::Global::QGraphicsItemClassId,
                                         QtRuby::qgraphicsitemTypeResolver);
    
    rb_require("qtgui/qtgui.rb");

    Smoke * smoke = qtgui_Smoke;
    for (int i = 1; i <= smoke->numClasses; i++) {
        Smoke::ModuleIndex classId(smoke, i);
        QString className = QString::fromLatin1(smoke->classes[i].className);

        if (    smoke->classes[i].external
                || className.contains("Internal")
                || className == "Qt"
                || className == "QGlobalSpace") {
            continue;
        }

        if (className.startsWith("Q"))
            className = className.mid(1).prepend("Qt::");

        VALUE klass = QtRuby::Global::initializeClass(classId, className);
        // VALUE name = rb_funcall(klass, rb_intern("name"), 0);
        // qDebug() << "name:" << StringValuePtr(name);
    }

    QtRuby::registerQtGuiTypes();

    /*
    for (int i = 0; i < 2000; i++) {
        if (QMetaType::isRegistered(i)) {
            QByteArray typeName(QMetaType::typeName(i));
            if (!typeName.isEmpty())
                qDebug() << "typeName:" << QMetaType::typeName(i);
        }
    }
    */

    return;
}

}