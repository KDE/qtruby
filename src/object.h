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

#ifndef QTRUBY_OBJECT_H
#define QTRUBY_OBJECT_H

#include <smoke.h>
#include <ruby.h>
#include <qtruby_export.h>

#include <QtCore/qmetatype.h>

namespace QtRuby {
    
    class QTRUBY_EXPORT Object
    {
    public:
        enum ValueOwnership {
            QtOwnership,
            ScriptOwnership,
            AutoOwnership
        };

        class Instance {
        public:
            Instance() : ownership(Object::QtOwnership) { }
            virtual void finalize();
            void dispose();
            virtual ~Instance();

            static Instance *get(VALUE object);
            inline const char * className()
            {
                return classId.smoke->classes[classId.index].className;
            }
        public:
            void* value;
            Object::ValueOwnership ownership;
            Smoke::ModuleIndex classId;
        };

        Object() : value(Qnil), instance(0) {}
        Object(VALUE v, Object::Instance * i) : value(v), instance(i) {}
        ~Object() {}

        typedef void (*TypeResolver)(Instance *);
        
        VALUE value;
        Instance * instance;

        static void mark(void * ptr);
        static void free(void * ptr);
    };

    extern VALUE dispose(VALUE self);
    extern VALUE is_disposed(VALUE self);
    extern VALUE cast_object_to(VALUE /*self*/, VALUE obj, VALUE new_klass);
}

Q_DECLARE_METATYPE( QtRuby::Object::Instance* )

#endif // SAMPLEIMPL_H
