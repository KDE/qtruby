/*
 *   Copyright 2009 by Richard Dale <richard.j.dale@gmail.com>

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

#include <global.h>
#include <marshall.h>

#include <smoke/phonon_smoke.h>

namespace QtRuby {
extern Marshall::TypeHandler PhononHandlers[];
extern void registerPhononTypes();
}

extern "C" {

Q_DECL_EXPORT void
Init_phonon()
{
    init_phonon_Smoke();
    QtRuby::Module phonon_module = { "phonon", new QtRuby::Binding(phonon_Smoke) };
    QtRuby::Global::modules[phonon_Smoke] = phonon_module;
    QtRuby::Marshall::installHandlers(QtRuby::PhononHandlers);

    Smoke * smoke = phonon_Smoke;
    for (int i = 1; i <= smoke->numClasses; i++) {
        Smoke::ModuleIndex classId(smoke, i);
        QString className = QString::fromLatin1(smoke->classes[i].className);

        if (    smoke->classes[i].external
                || className.contains("Internal")
                || className == "QGlobalSpace") {
            continue;
        }

        VALUE klass = QtRuby::Global::initializeClass(classId, className);
    }

    QtRuby::registerPhononTypes();

    return;
}

}