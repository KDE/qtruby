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

#ifndef QTRUBY_VIRTUAL_METHOD_CALL_H
#define QTRUBY_VIRTUAL_METHOD_CALL_H

#include <smoke.h>
#include "marshall.h"

namespace QtRuby {

    class VirtualMethodCall : public Marshall {

        class ReturnValue : Marshall {
        public:
            ReturnValue(Smoke::ModuleIndex methodId, Smoke::Stack stack, VALUE returnValue);

            inline const Smoke::Method &method() { return m_methodId.smoke->methods[m_methodId.index]; }
            inline SmokeType type() { return m_type; }
            inline Marshall::Action action() { return Marshall::FromVALUE; }
            inline Smoke::StackItem &item() { return m_stack[0]; }
            inline VALUE * var() { return &m_returnValue; }
            inline Smoke *smoke() { return m_methodId.smoke; }
            inline bool cleanup() { return false; }

            void unsupported();
            void next();

        private:
            Smoke::ModuleIndex m_methodId;
            Smoke::Stack m_stack;
            SmokeType m_type;
            VALUE m_returnValue;
        };

    public:
        VirtualMethodCall(Smoke::ModuleIndex methodId, Smoke::Stack stack, VALUE obj, VALUE * args);

        ~VirtualMethodCall();

        inline SmokeType type() { return SmokeType(m_methodId.smoke, m_args[m_current]); }
        inline Marshall::Action action() { return Marshall::ToVALUE; }
        inline Smoke::StackItem &item() { return m_stack[m_current + 1]; }
        inline VALUE * var() { return &(m_valueList[m_current]); }
        inline const Smoke::Method &method() { return m_methodRef; }
        inline Smoke *smoke() { return m_methodId.smoke; }
        inline bool cleanup() { return false; }   // is this right?

        void unsupported();
        void callMethod();
        void next();

    private:
        Smoke::ModuleIndex m_methodId;
        Smoke::Stack m_stack;
        Smoke::Index * m_args;
        VALUE m_obj;
        VALUE * m_valueList;
        int m_current;
        bool m_called;
        Smoke::Method & m_methodRef;
    };

}

#endif // QTRUBY_VIRTUAL_METHOD_CALL_H

// kate: space-indent on; indent-width 4; replace-tabs on; mixed-indent off;

