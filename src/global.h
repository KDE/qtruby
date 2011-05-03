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

#ifndef QTRUBY_GLOBAL_H
#define QTRUBY_GLOBAL_H

#include <QtCore/QHash>

#include <smoke.h>
#include <ruby.h>

#include "qtruby_export.h"
#include "smokebinding.h"
#include "object.h"

#if !defined RSTRING_LEN
#define RSTRING_LEN(a) RSTRING(a)->len
#endif
#if !defined RSTRING_PTR
#define RSTRING_PTR(a) RSTRING(a)->ptr
#endif
#if !defined RARRAY_LEN
#define RARRAY_LEN(a) RARRAY(a)->len
#endif
#if !defined RARRAY_PTR
#define RARRAY_PTR(a) RARRAY(a)->ptr
#endif
#if !defined StringValueCStr
#define StringValueCStr(s) STR2CSTR(s)
#endif

#define QTRUBY_VERSION "3.0.0"

namespace QtRuby {

    namespace Global {
        QTRUBY_EXPORT extern QHash<Smoke*, Module> modules;

        QTRUBY_EXPORT extern Smoke::ModuleIndex QObjectClassId;
        QTRUBY_EXPORT extern Smoke::ModuleIndex QMetaObjectClassId;
        QTRUBY_EXPORT extern Smoke::ModuleIndex QDateClassId;
        QTRUBY_EXPORT extern Smoke::ModuleIndex QDateTimeClassId;
        QTRUBY_EXPORT extern Smoke::ModuleIndex QTimeClassId;
        QTRUBY_EXPORT extern Smoke::ModuleIndex QEventClassId;
        QTRUBY_EXPORT extern Smoke::ModuleIndex QGraphicsItemClassId;
        QTRUBY_EXPORT extern Smoke::ModuleIndex QVariantClassId;

        QTRUBY_EXPORT extern VALUE QtModule;
        QTRUBY_EXPORT extern VALUE QtInternalModule;
        QTRUBY_EXPORT extern VALUE QtBaseClass;
        QTRUBY_EXPORT extern VALUE QtEnumClass;

        QTRUBY_EXPORT extern VALUE QListModelClass;
        QTRUBY_EXPORT extern VALUE QMetaObjectClass;
        QTRUBY_EXPORT extern VALUE QTableModelClass;
        QTRUBY_EXPORT extern VALUE QVariantClass;

        QTRUBY_EXPORT VALUE getRubyValue(const void * ptr);

        QTRUBY_EXPORT void unmapPointer(    Object::Instance * instance,
                                            const Smoke::ModuleIndex& classId, 
                                            void * lastptr = 0 );

        QTRUBY_EXPORT void mapPointer(  VALUE obj,
                                        Object::Instance * instance, 
                                        const Smoke::ModuleIndex& classId, 
                                        void * lastptr = 0 );

        QTRUBY_EXPORT VALUE rubyClassFromId(const Smoke::ModuleIndex& classId);
        QTRUBY_EXPORT QByteArray rubyClassNameFromId(const Smoke::ModuleIndex& classId);
        QTRUBY_EXPORT Smoke::ModuleIndex idFromRubyClass(VALUE klass, bool superclasses = false);

        QTRUBY_EXPORT VALUE wrapInstance(   const Smoke::ModuleIndex& classId,
                                            void * ptr,
                                            Object::ValueOwnership ownership = Object::QtOwnership,
                                            VALUE klass = Qnil);

        QTRUBY_EXPORT void defineMethod(const Smoke::ModuleIndex& classId, const char* name, VALUE (*func)(ANYARGS), int argc);
        QTRUBY_EXPORT void defineSingletonMethod(const Smoke::ModuleIndex& classId, const char* name, VALUE (*func)(ANYARGS), int argc);

        QTRUBY_EXPORT void defineTypeResolver(const Smoke::ModuleIndex& baseClass, Object::TypeResolver);
        QTRUBY_EXPORT void resolveType(Object::Instance * instance);

        QTRUBY_EXPORT void initialize();
        QTRUBY_EXPORT VALUE initializeClass(    const Smoke::ModuleIndex& classId,
                                                const QString& rubyClassName );
    }
}

#endif