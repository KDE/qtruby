/*
 *   Copyright 2003-2013 by Richard Dale <richard.j.dale@gmail.com>

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

#ifndef QTRUBY_INVOKE_SLOT_H
#define QTRUBY_INVOKE_SLOT_H

#include <smoke.h>
#include <ruby.h>

#include <QtCore/QMetaMethod>
#include <QtCore/QVector>

#include "qtruby_export.h"
#include "marshall.h"
#include "object.h"

namespace QtRuby {

    class QTRUBY_EXPORT InvokeSlot : public Marshall {

        class ReturnValue : Marshall {
        public:
            ReturnValue(Smoke* smoke, VALUE* returnValue, const QMetaMethod& metaMethod, void ** a);
            
            inline SmokeType type() { return m_type; }
            inline Marshall::Action action() { return Marshall::FromVALUE; }
            inline Smoke::StackItem& item();
            inline VALUE* var() { return m_returnValue; }
            inline Smoke*smoke() { return m_smoke; }
            inline bool cleanup() { return false; }

            void unsupported();
            void next();

        private:
            VALUE* m_returnValue;
            const QMetaMethod& m_metaMethod;
            void ** _a;
            Smoke* m_smoke;
            Smoke::StackItem m_stackItem;
            SmokeType m_type;
       };

    public:
        InvokeSlot(VALUE self, ID methodID, VALUE *valueList, const QMetaMethod& metaMethod, void ** a);
        ~InvokeSlot();

        SmokeType type();
        inline Marshall::Action action() { return Marshall::ToVALUE; }
        Smoke::StackItem &item();
        inline VALUE* var() { return &(m_argv[m_current]); }
        inline Smoke* smoke() { return m_smoke; }
        inline bool cleanup() { return false; }

        void unsupported();
        void callMethod();
        void next();
    private:
        VALUE m_self;
        ID m_methodID;
        const QMetaMethod& m_metaMethod;
        void ** _a;
        Smoke* m_smoke;
        Smoke::Stack m_stack;
        SmokeType* m_smokeTypes;
        int m_current;
        bool m_called;
        bool m_error;
        int m_argc;
        VALUE* m_argv;
    };
}

#endif // QTRUBY_INVOKE_SLOT_H

// kate: space-indent on; indent-width 4; replace-tabs on; mixed-indent off;

