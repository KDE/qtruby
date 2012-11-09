/*
 *   Copyright 2003-2011 by Richard Dale <richard.j.dale@gmail.com>

 *   Based on the PerlQt marshalling code by Ashley Winters

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

#ifndef QTRUBY_MARSHALL_H
#define QTRUBY_MARSHALL_H

#include <qglobal.h>
#include <QtScript/QScriptEngine>

#include <smoke.h>
#include <ruby.h>
#include <smoke/qtcore_smoke.h>

#include "qtruby_export.h"

namespace QtRuby {

    class SmokeType {
        Smoke::Type *m_type;        // derived from _smoke and _id, but cached
        Smoke *m_smoke;
        Smoke::Index m_id;
    public:
        SmokeType() : m_type(0), m_smoke(0), m_id(0) {}
        SmokeType(Smoke *s, Smoke::Index i) : m_smoke(s), m_id(i) {
            if (m_id < 0 || m_id > m_smoke->numTypes) {
                m_id = 0;
            }
            m_type = m_smoke->types + m_id;
        }
        // default copy constructors are fine, this is a constant structure

        // mutators
        void set(Smoke *s, Smoke::Index i) {
            m_smoke = s;
            m_id = i;
            m_type = m_smoke->types + m_id;
        }

        // accessors
        Smoke *smoke() const { return m_smoke; }
        Smoke::Index typeId() const { return m_id; }
        const Smoke::Type &type() const { return *m_type; }
        unsigned short flags() const { return m_type->flags; }
        unsigned short element() const { return m_type->flags & Smoke::tf_elem; }
        const char *name() const { return m_type->name; }
        Smoke::Index classId() const { return m_type->classId; }

        // tests
        bool isStack() const { return ((flags() & Smoke::tf_ref) == Smoke::tf_stack); }
        bool isPtr() const { return ((flags() & Smoke::tf_ref) == Smoke::tf_ptr); }
        bool isRef() const { return ((flags() & Smoke::tf_ref) == Smoke::tf_ref); }
        bool isConst() const { return (flags() & Smoke::tf_const); }
        bool isClass() const {
            return element() == Smoke::t_class && classId() != 0;
        }

        bool operator==(const SmokeType &b) const {
            const SmokeType &a = *this;
            if (a.name() == b.name()) {
                return true;
            }

            if (a.name() && b.name() && qstrcmp(a.name(), b.name()) == 0) {
                return true;
            }

            return false;
        }

        bool operator!=(const SmokeType &b) const {
            const SmokeType &a = *this;
            return !(a == b);
        }

    };

    class QTRUBY_EXPORT Marshall {
    public:
        /**
        * FromVALUE is used for virtual function return values and regular
        * method arguments.
        *
        * ToVALUE is used for method return-values and virtual function
        * arguments.
        */
        typedef void (*HandlerFn)(Marshall *);
        enum Action { FromVALUE, ToVALUE };
        virtual SmokeType type() = 0;
        virtual Action action() = 0;
        virtual Smoke::StackItem &item() = 0;
        virtual VALUE * var() = 0;
        virtual void unsupported() = 0;
        virtual Smoke *smoke() = 0;
        /**
        * For return-values, next() does nothing.
        * For FromVALUE, next() calls the method and returns.
        * For ToVALUE, next() calls the virtual function and returns.
        *
        * Required to reset Marshall object to the state it was
        * before being called when it returns.
        */
        virtual void next() = 0;
        /**
        * For FromVALUE, cleanup() returns false when the handler should free
        * any allocated memory after next().
        *
        * For ToVALUE, cleanup() returns true when the handler should delete
        * the pointer passed to it.
        */
        virtual bool cleanup() = 0;

        virtual ~Marshall() {}

        struct TypeHandler {
            const char *name;
            Marshall::HandlerFn fn;
        };

        static void installHandlers(TypeHandler * handler);
        static Marshall::HandlerFn getMarshallFn(const SmokeType &type);

        typedef VALUE (*MarshallFunction)(const void *);
        typedef void (*DemarshallFunction)(VALUE, void *);
    };

    extern QTRUBY_EXPORT Marshall::TypeHandler Handlers[];

}

#endif

// kate: space-indent on; indent-width 4; replace-tabs on; mixed-indent off;

