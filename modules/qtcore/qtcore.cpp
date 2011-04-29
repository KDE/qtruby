/*
 *   Copyright 2003-2011 by Richard Dale <richard.j.dale@gmail.com>

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

#include <smoke/qtcore_smoke.h>

#include <global.h>
#include <marshall.h>
#include <rubymetatype.h>
#include <utils.h>
#include <object.h>

#include "rubyqobject.h"
#include "typeresolver.h"

namespace QtRuby {
extern Marshall::TypeHandler QtCoreHandlers[];
extern void registerQtCoreTypes();
}

extern "C" {

Q_DECL_EXPORT void
Init_qtcore()
{
    init_qtcore_Smoke();
    QtRuby::Module qtcore_module = { "qtcore", new QtRuby::Binding(qtcore_Smoke) };
    QtRuby::Global::modules[qtcore_Smoke] = qtcore_module;

    QtRuby::Global::QObjectClassId = qtcore_Smoke->idClass("QObject");
    QtRuby::Global::QMetaObjectClassId = qtcore_Smoke->idClass("QMetaObject");
    QtRuby::Global::QDateClassId = qtcore_Smoke->idClass("QDate");
    QtRuby::Global::QDateTimeClassId = qtcore_Smoke->idClass("QDateTime");
    QtRuby::Global::QTimeClassId = qtcore_Smoke->idClass("QTime");
    QtRuby::Global::QEventClassId = qtcore_Smoke->idClass("QEvent");

    QtRuby::Marshall::installHandlers(QtRuby::QtCoreHandlers);

    QtRuby::Global::initialize();

    QtRuby::Global::defineTypeResolver(QtRuby::Global::QEventClassId, QtRuby::qeventTypeResolver);
    QtRuby::Global::defineTypeResolver(QtRuby::Global::QObjectClassId, QtRuby::qobjectTypeResolver );

    QtRuby::Global::defineMethod(   QtRuby::Global::QObjectClassId,
                                    "inherits",
                                    (VALUE (*) (...)) QtRuby::inherits_qobject,
                                    -1 );
    QtRuby::Global::defineMethod(   QtRuby::Global::QObjectClassId,
                                    "findChildren",
                                    (VALUE (*) (...)) QtRuby::find_qobject_children,
                                    -1);
    QtRuby::Global::defineMethod(   QtRuby::Global::QObjectClassId,
                                    "findChild",
                                    (VALUE (*) (...)) QtRuby::find_qobject_child,
                                    -1 );
    QtRuby::Global::defineMethod(   QtRuby::Global::QObjectClassId,
                                    "qobject_cast",
                                    (VALUE (*) (...)) QtRuby::qobject_qt_metacast,
                                    1 );

    rb_require("qtcore/qtcore.rb");
    
    Smoke * smoke = qtcore_Smoke;
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
    
   //  QtRuby::Global::initializeClasses(qtcore_Smoke);

    QtRuby::registerQtCoreTypes();
        
    return;
}

}
