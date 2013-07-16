/*
 * Copyright 2003-2011 Ian Monroe <imonroe@kde.org>
 * Copyright 2003-2011 by Richard Dale <richard.j.dale@gmail.com>
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

#ifndef QTRUBY_UTILS_H
#define QTRUBY_UTILS_H

#include <smoke.h>

#include <QtCore/QVector>
#include <QtCore/QByteArray>
#include <QtCore/QPair>

#include <QtScript/QScriptContext>
#include <QtScript/QScriptEngine>

#include "qtruby_export.h"
#include "object.h"
#include "marshall.h"

namespace QtRuby {
    
    enum MethodMatchesState {
        InitialState,
        ImplicitTypeConversionsState,
        ArgumentTypeConversionState
    };

    typedef QPair<QVector<Smoke::ModuleIndex>, int> MethodMatch;
    typedef QVector<MethodMatch> MethodMatches;

    QTRUBY_EXPORT MethodMatches resolveMethod(  Smoke::ModuleIndex classId,
                                                const QByteArray& methodName,
                                                int argc,
                                                VALUE * args,
                                                MethodMatchesState matchState = InitialState );

    QTRUBY_EXPORT void * constructCopy(Object::Instance *instance);
    
    QTRUBY_EXPORT SmokeType findSmokeType(const char* typeName, Smoke * smoke = 0);
}

#endif