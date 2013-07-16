/*
 *   Copyright 2003-2013 by Richard Dale <richard.j.dale@gmail.com>

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

#include "methodmissing.h"
#include "global.h"
#include "object.h"
#include "methodcall.h"
#include "utils.h"

#include <QtCore/QByteArray>
#include <QtCore/QRegExp>
#include <QtCore/QMetaMethod>
#include <QtGui/QApplication>
#include <QtCore/qdebug.h>

namespace QtRuby {

static QHash<QByteArray, QVector<Smoke::ModuleIndex> > methodCache;

static const char * valueToTypeFlag(VALUE value)
{
    const char *r = "";
    if (value == Qnil)
        r = "u";
    else if (TYPE(value) == T_FIXNUM || TYPE(value) == T_BIGNUM)
        r = "i";
    else if (TYPE(value) == T_FLOAT)
        r = "n";
    else if (TYPE(value) == T_STRING)
        r = "s";
    else if(value == Qtrue || value == Qfalse)
        r = "B";
    else if ((TYPE(value) == T_OBJECT && rb_funcall(value, rb_intern("class"), 0) == Global::QtEnumClass)) {
        VALUE temp = rb_funcall(value, rb_intern("type"), 0);
        r = StringValuePtr(temp);
    } else if (TYPE(value) == T_DATA) {
        Object::Instance * instance = Object::Instance::get(value);

        if (instance == 0 || instance->classId == Smoke::NullModuleIndex) {
            r = "a";
        } else {
            r = instance->className();
        }
    } else {
        r = "U";
    }

    return r;
}

static QByteArray selectorSignature(int argc, VALUE * argv, VALUE klass, const char * methodName)
{
    // Look in the cache
    QByteArray selector;
    selector = rb_class2name(klass);
    selector += ';';
    selector += methodName;

    for(int i = 0; i < argc; ++i) {
        selector += ';';
        selector += valueToTypeFlag(argv[i]);
    }

    return selector;
}

VALUE method_missing(int argc, VALUE * argv, VALUE self)
{
    const char * methodName = rb_id2name(SYM2ID(argv[0]));
    VALUE klass = rb_funcall(self, rb_intern("class"), 0);
    QByteArray selector = selectorSignature(argc - 1, argv + 1, klass, methodName);
    // qDebug() << Q_FUNC_INFO << "selector:" << selector;
    Object::Instance * instance = Object::Instance::get(self);

    if (instance == 0 || instance->isNull()) {
        return rb_call_super(argc, argv);
    }

    // Look for 'thing?' methods, and try to match isThing() or hasThing() in the Smoke runtime
    QByteArray pred(methodName);

    if (pred.endsWith("?")) {
        // Drop the trailing '?'
        pred.replace(pred.length() - 1, 1, "");

        pred.replace(0, 1, pred.mid(0, 1).toUpper());
        pred.replace(0, 0, "is");
        Smoke::ModuleIndex methodId = instance->classId.smoke->findMethod(  instance->className(),
                                                                            static_cast<const char *>(pred) );

        if (methodId == Smoke::NullModuleIndex) {
            pred.replace(0, 2, "has");
            methodId = instance->classId.smoke->findMethod( instance->className(),
                                                            static_cast<const char *>(pred) );
        }

        if (methodId != Smoke::NullModuleIndex) {
            methodName = (char *) static_cast<const char *>(pred);
        }
    }

    {
        if (!methodCache.contains(selector)) {
            MethodMatches matches = QtRuby::resolveMethod(instance->classId, methodName, argc - 1, argv + 1);

            if (matches.count() == 0 || matches[0].second >= 100) {
                // QString message = QString("%1 is not defined").arg(methodName);
            } else if (matches.count() > 1 && matches[0].second == matches[1].second) {
                // Error if the first two have the same matchDistance
                // QString message = QString("overloaded %1() call not resolved").arg(methodName);
            } else {
                // Good, found a single best match in matches[0]
                methodCache[selector] = matches[0].first;
            }

            if (!methodCache.contains(selector)) {
                if (    qstrcmp(methodName, "-") == 0
                        || qstrcmp(methodName, "+") == 0
                        || qstrcmp(methodName, "/") == 0
                        || qstrcmp(methodName, "%") == 0
                        || qstrcmp(methodName, "|") == 0 )
                {
                    // Look for operator methods of the form 'operator+=', 'operator-=' and so on..
                    QByteArray op1("operator");
                    op1 += methodName;
                    op1 += "=";
                    matches = QtRuby::resolveMethod(instance->classId, op1, argc - 1, argv + 1);
                    if (matches.count() == 0 || matches[0].second >= 100) {
                    } else {
                        methodCache[selector] = matches[0].first;
                    }
                }

                if (!methodCache.contains(selector)) {
                    // Check for property getter/setter calls, and for slots in QObject classes
                    // not in the smoke library
                    if (Smoke::isDerivedFrom(instance->classId, Smoke::findClass("QObject"))) {
                        QObject * qobject = reinterpret_cast<QObject*>(instance->cast(Global::QObjectClassId));
                        QByteArray name;

                        name = rb_id2name(SYM2ID(argv[0]));
                        const QMetaObject * meta = qobject->metaObject();

                        if (argc == 1) {
                            if (name.endsWith("?")) {
                                name.replace(0, 1, pred.mid(0, 1).toUpper());
                                name.replace(0, 0, "is");
                                if (meta->indexOfProperty(name) == -1) {
                                    name.replace(0, 2, "has");
                                }
                            }

                            if (meta->indexOfProperty(name) != -1) {
                                VALUE qvariant = rb_funcall(self, rb_intern("property"), 1, rb_str_new2(name));
                                return rb_funcall(qvariant, rb_intern("value"), 0);
                            }
                        }

                        if (argc == 2 && name.endsWith("=")) {
                            name.replace("=", "");
                            if (meta->indexOfProperty(name) != -1) {
                                VALUE qvariant = rb_funcall(self, rb_intern("qVariantFromValue"), 1, argv[1]);
                                return rb_funcall(self, rb_intern("setProperty"), 2, rb_str_new2(name), qvariant);
                            }
                        }

                        int classId = instance->classId.smoke->idClass(meta->className()).index;

                        // The class isn't in the Smoke lib. But if it is called 'local::Merged'
                        // it is from a QDBusInterface and the slots are remote, so don't try to
                        // those.
                        while ( classId == 0
                                && qstrcmp(meta->className(), "local::Merged") != 0
                                && qstrcmp(meta->superClass()->className(), "QDBusAbstractInterface") != 0 )
                        {
                            // Assume the QObject has slots which aren't in the Smoke library, so try
                            // and call the slot directly
                            for (int id = meta->methodOffset(); id < meta->methodCount(); id++) {
                                if (meta->method(id).methodType() == QMetaMethod::Slot) {
                                    QByteArray signature(meta->method(id).signature());
                                    QByteArray methodName = signature.mid(0, signature.indexOf('('));

                                    // Don't check that the types of the ruby args match the c++ ones for now,
                                    // only that the name and arg count is the same.
                                    if (name == methodName && meta->method(id).parameterTypes().count() == (argc - 1)) {
//                                        QList<MocArgument*> args = get_moc_arguments(   instance->classId.smoke, meta->method(id).typeName(),
//                                                                                        meta->method(id).parameterTypes() );
//                                        VALUE result = Qnil;
//                                        QtRuby::InvokeNativeSlot slot(qobject, id, argc - 1, args, argv + 1, &result);
//                                        slot.next();
//                                        return result;
                                    }
                                }
                            }
                            meta = meta->superClass();
                            classId = instance->classId.smoke->idClass(meta->className()).index;
                        }
                    }

                    return rb_call_super(argc, argv);
                }
            }
        }
    }

    QtRuby::MethodCall methodCall(methodCache[selector], self, argv + 1);
    methodCall.next();
    VALUE result = *(methodCall.var());
    return result;
}

VALUE class_method_missing(int argc, VALUE * argv, VALUE klass)
{
    VALUE result = Qnil;
    const char * methodName = rb_id2name(SYM2ID(argv[0]));
    QByteArray selector = selectorSignature(argc - 1, argv + 1, klass, methodName);
    qDebug() << Q_FUNC_INFO << "selector:" << selector;
    Smoke::ModuleIndex classId = Global::idFromRubyClass(klass, true);

    if (!methodCache.contains(selector)) {
        MethodMatches matches = QtRuby::resolveMethod(classId, methodName, argc - 1, argv + 1);

        if (matches.count() == 0 || matches[0].second >= 100) {
            rb_raise(rb_eArgError, "%s is not defined\n", methodName);
        } else if (matches.count() > 1 && matches[0].second == matches[1].second) {
            // Error if the first two have the same matchDistance
            rb_raise(rb_eArgError, "overloaded %s() call not resolved\n", methodName);
        } else {
            // Good, found a single best match in matches[0]
            methodCache[selector] = matches[0].first;
        }
    }

    if (!methodCache.contains(selector)) {
        QRegExp rx("[a-zA-Z]+");

        if (rx.indexIn(methodName) == -1) {
            // If an operator method hasn't been found as an instance method,
            // then look for a class method - after 'op(self,a)' try 'self.op(a)'
            VALUE * methodStack = ALLOCA_N(VALUE, argc - 1);
            methodStack[0] = argv[0];
            for (int count = 1; count < argc - 1; count++) {
                methodStack[count] = argv[count+1];
            }
            result = method_missing(argc-1, methodStack, argv[1]);
            return result;
        } else {
            return rb_call_super(argc, argv);
        }
    }

    QtRuby::MethodCall methodCall(methodCache[selector], Qnil, argv + 1);
    methodCall.next();
    result = *(methodCall.var());
    return result;
}

VALUE module_method_missing(int argc, VALUE * argv, VALUE klass)
{
    return class_method_missing(argc, argv, klass);
}

/*

class LCDRange < Qt::Widget

    def initialize(s, parent, name)
        super(parent, name)
        init()
        ...

For a case such as the above, the QWidget can't be instantiated until
the initializer has been run up to the point where 'super(parent, name)'
is called. Only then, can the number and type of arguments passed to the
constructor be known. However, the rest of the intializer
can't be run until 'self' is a proper T_DATA object with a wrapped C++
instance.

The solution is to run the initialize code twice. First, only up to the
'super(parent, name)' call, where the QWidget would get instantiated in
initialize_qt(). And then rb_throw() jumps out of the
initializer returning the wrapped object as a result.

The second time round 'self' will be the wrapped instance of type T_DATA,
so initialize() can be allowed to proceed to the end.
*/
VALUE initialize_qt(int argc, VALUE * argv, VALUE self)
{
    if (TYPE(self) == T_DATA) {
        // If a ruby block was passed then run that now
        if (rb_block_given_p()) {
            rb_funcall(Global::QtInternalModule, rb_intern("run_initializer_block"), 2, self, rb_block_proc());
        }

        return self;
    }

    VALUE result = Qnil;

    {
        VALUE klass = rb_funcall(self, rb_intern("class"), 0);
        Smoke::ModuleIndex classId = Global::idFromRubyClass(klass, true);
        QByteArray className(classId.smoke->classes[classId.index].className);
        QByteArray selector = selectorSignature(argc, argv, klass, className);
        // qDebug() << Q_FUNC_INFO << "selector:" << selector;

        if (!methodCache.contains(selector)) {
            MethodMatches matches = QtRuby::resolveMethod(classId, className, argc, argv);

            if (matches.count() == 0 || matches[0].second >= 100) {
                rb_raise(rb_eArgError, "undefined constructor call %s\n", rb_class2name(klass));
            } else if (matches.count() > 1 && matches[0].second == matches[1].second) {
                rb_raise(rb_eArgError, "unresolved constructor call %s\n", rb_class2name(klass));
            } else {
                // Good, found a single best match in matches[0]
                methodCache[selector] = matches[0].first;
            }
        }

        // Allocate the MethodCall within a C block. Otherwise, because the continue_new_instance()
        // call below will longjmp out, it wouldn't give C++ an opportunity to clean up
        QtRuby::MethodCall constructor(methodCache[selector], self, argv);
        constructor.next();
        VALUE value = *(constructor.var());
        Object::Instance * instance = Object::Instance::get(value);

        // qDebug() << "klass:" << klass << " rb_funcall(value, rb_intern(class), 0):" << rb_funcall(value, rb_intern("class"), 0);
        if (klass == rb_funcall(value, rb_intern("class"), 0)) {
            instance->ownership = Object::ScriptOwnership;
            Global::mapPointer(value, instance, instance->classId);
            result = value;
        } else {
            result = Global::wrapInstance(instance->classId, instance->value, Object::ScriptOwnership, klass);
            instance->value = 0;
        }
    }

    // Off with a longjmp, never to return..
    rb_throw("newqt", result);
    /*NOTREACHED*/
    return self;
}

VALUE
new_qt(int argc, VALUE * argv, VALUE klass)
{
    VALUE * tempStack = ALLOCA_N(VALUE, argc + 1);
    tempStack[0] = rb_obj_alloc(klass);

    for (int count = 0; count < argc; count++) {
        tempStack[count+1] = argv[count];
    }

    VALUE result = rb_funcall2(Global::QtInternalModule, rb_intern("try_initialize"), argc+1, tempStack);
    rb_obj_call_init(result, argc, argv);

    return result;
}

}

// kate: space-indent on; indent-width 4; replace-tabs on; mixed-indent off;
