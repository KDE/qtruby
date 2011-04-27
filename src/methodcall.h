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

#ifndef QTRUBY_METHOD_CALL_H
#define QTRUBY_METHOD_CALL_H

#include <smoke.h>
#include <ruby.h>

#include <QtCore/QVector>

#include "qtruby_export.h"
#include "marshall.h"
#include "object.h"

namespace QtRuby {

    class QTRUBY_EXPORT MethodCall : public Marshall {
        
       class ArgumentTypeConversion : Marshall {
        public:
            ArgumentTypeConversion(Smoke::ModuleIndex methodId, Smoke::StackItem& item, VALUE& value);
            ~ArgumentTypeConversion();
            
            inline const Smoke::Method &method() { return m_methodId.smoke->methods[m_methodId.index]; }
            inline SmokeType type() { return m_type; }
            inline Marshall::Action action() { return Marshall::FromVALUE; }
            inline Smoke::StackItem &item() { return m_stack[1]; }
            inline VALUE * var() { return &m_value; }
            inline Smoke * smoke() { return m_methodId.smoke; }
            inline bool cleanup() { return false; }

            void unsupported();
            void next();
            
        private:
            Smoke::ModuleIndex m_methodId;
            Smoke::StackItem& m_item;
            SmokeType m_type;
            VALUE& m_value;
            Smoke::Stack m_stack;
        };
        
        class ReturnValue : Marshall {
        public:
            ReturnValue(Smoke::ModuleIndex methodId, Smoke::Stack stack, VALUE * returnValue);

            inline const Smoke::Method &method() { return m_methodId.smoke->methods[m_methodId.index]; }
            inline SmokeType type() { return SmokeType(m_methodId.smoke, method().ret); }
            inline Marshall::Action action() { return Marshall::ToVALUE; }
            inline Smoke::StackItem &item() { return m_stack[0]; }
            inline VALUE * var() { return m_returnValue; }
            inline Smoke *smoke() { return m_methodId.smoke; }
            inline bool cleanup() { return false; }

            void unsupported();
            void next();
            
        private:
            Smoke::ModuleIndex m_methodId;
            Smoke::Stack m_stack;
            VALUE * m_returnValue;
        };
        
    public:
        MethodCall( const QVector<Smoke::ModuleIndex>& methodids,
                    VALUE target,
                    VALUE * args );
        ~MethodCall();

        inline SmokeType type() { return SmokeType(m_smoke, m_args[m_current]); }
        inline Marshall::Action action() { return Marshall::FromVALUE; }
        inline Smoke::StackItem &item() { return m_stack[m_current + 1]; }
        inline VALUE * var() {
            if (m_current < 0 || m_error) {
                return &m_returnValue;
            }
            
            return &(m_valueList[m_current]);
        }
        inline Smoke *smoke() { return m_smoke; }
        inline bool cleanup() { return true; }

        void unsupported();

        void callMethod();
        void next();
        
        inline bool hasTypeConversion() {
            return  (m_current + 1) < m_methodIds.count()
                    && m_methodIds[m_current + 1] != Smoke::NullModuleIndex;
        }
        
        inline Smoke::ModuleIndex typeConversion() {
            return m_methodIds[m_current + 1];
        }
    private:
        const QVector<Smoke::ModuleIndex>& m_methodIds;
        Smoke::Method & m_methodRef;
        int m_current;
        Smoke * m_smoke;
        Smoke::Stack m_stack;
        Smoke::ModuleIndex m_methodId;
        Smoke::Index * m_args;
        VALUE m_target;
        QtRuby::Object::Instance * m_instance;
        VALUE m_returnValue;
        VALUE * m_valueList;
        bool m_called;
        bool m_error;
    };
}

#endif // QTRUBY_METHOD_CALL_H

// kate: space-indent on; indent-width 4; replace-tabs on; mixed-indent off;

