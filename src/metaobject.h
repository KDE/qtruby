/*
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

#ifndef QTRUBY_METAOBJECT_H
#define QTRUBY_METAOBJECT_H

#include <smoke.h>
#include <ruby.h>

#include <QtCore/qglobal.h>
#include <QtCore/QVector>
#include <QtCore/QPair>
#include <QtCore/QByteArray>

#include "qtruby_export.h"
#include "object.h"

namespace QtRuby {
    
    struct QTRUBY_EXPORT MetaObject
    {
    public:
        MetaObject() : resolver(0), mark(0), free(0) {}

        typedef VALUE (*CFunc)(ANYARGS);

        struct RubyMethod {
            QByteArray name;
            CFunc fn;
            int arity;
        };
        
        Smoke::ModuleIndex classId;
        inline const char * className()
        {
            return classId.smoke->classes[classId.index].className;
        }
            
        VALUE rubyClass;
        const char * rubyClassName();
        
        QVector<RubyMethod> rubyMethods;
        Object::TypeResolver resolver;
        RUBY_DATA_FUNC mark;
        RUBY_DATA_FUNC free;
    };
}

#endif // QTRUBY_METAOBJECT_H
