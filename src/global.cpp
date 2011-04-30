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

#include <stdlib.h>

#include <QtCore/QHash>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QMutex>
#include <QtCore/QMutexLocker>
#include <QtCore/QThread>
#include <QtCore/QCoreApplication>
#include <QtCore/qdebug.h>

#include "global.h"
#include "debug.h"
#include "funcall.h"
#include "methodmissing.h"
#include "metaobject.h"

static uint qHash(const Smoke::ModuleIndex& mi) {
    return qHash(mi.index) ^ qHash(mi.smoke);
}

namespace QtRuby {
    namespace Global {

        
Smoke::ModuleIndex QObjectClassId;
Smoke::ModuleIndex QMetaObjectClassId;
Smoke::ModuleIndex QDateClassId;
Smoke::ModuleIndex QDateTimeClassId;
Smoke::ModuleIndex QTimeClassId;
Smoke::ModuleIndex QEventClassId;
Smoke::ModuleIndex QGraphicsItemClassId;

VALUE QtEnum;

VALUE QtModule;
VALUE QtInternalModule;
VALUE QtBaseClass;

QHash<Smoke*, Module> modules;

typedef QHash<const void *, QtRuby::Object> RubyValuesMap;
Q_GLOBAL_STATIC(RubyValuesMap, rubyValues)

typedef QHash<Smoke::ModuleIndex, QtRuby::MetaObject *> MetaObjectsMap;
Q_GLOBAL_STATIC(MetaObjectsMap, metaObjects)

typedef QHash<VALUE, QtRuby::MetaObject *> RubyMetaObjectsMap;
Q_GLOBAL_STATIC(RubyMetaObjectsMap, rubyMetaObjects)

VALUE
getRubyValue(const void *ptr)
{
    if (!rubyValues() || !rubyValues()->contains(ptr)) {
        if (Debug::DoDebug & Debug::GC) {
            // qWarning("QtRuby::Global::getRubyValue %p -> nil", ptr);
            if (!rubyValues()) {
                // qWarning("QtRuby::Global::getRubyValue rubyValues deleted");
            }
        }
        return Qnil;
    } else {
        if (Debug::DoDebug & Debug::GC) {
            // qWarning("QtRuby::Global::getRubyValue %p -> %p", ptr, (void *) rubyValues()->operator[](ptr).value);
        }
        return rubyValues()->operator[](ptr).value;
    }
}

void 
unmapPointer(Object::Instance * instance, const Smoke::ModuleIndex& classId, void *lastptr)
{
    Smoke * smoke = classId.smoke;
    void * ptr = instance->cast(classId);
    
    if (ptr != lastptr) {
        lastptr = ptr;
        if (rubyValues() && rubyValues()->contains(ptr)) {
            VALUE value = rubyValues()->operator[](ptr).value;
            
            if (Debug::DoDebug & Debug::GC) {
                Object::Instance * instance = Object::Instance::get(value);
                const char *className = instance->classId.smoke->classes[instance->classId.index].className;
                qWarning("QtRuby::Global::unmapPointer (%s*)%p -> 0x%8.8x size: %d", className, ptr, static_cast<uint>(value), rubyValues()->size() - 1);
            }
            
            rubyValues()->remove(ptr);
        }
    }

    for (   Smoke::Index * parent = smoke->inheritanceList + smoke->classes[classId.index].parents; 
            *parent != 0; 
            parent++ ) 
    {
        if (smoke->classes[*parent].external) {
            Smoke::ModuleIndex mi = Smoke::findClass(smoke->classes[*parent].className);
            if (mi != Smoke::NullModuleIndex)
                unmapPointer(instance, mi, lastptr);
        } else {
            unmapPointer(instance, Smoke::ModuleIndex(smoke, *parent), lastptr);
        }
    }
}

// Store pointer in g_rubyValues hash : "pointer_to_Qt_object" => weak ref to associated VALUE object
// Recurse to store it also as casted to its parent classes.

void 
mapPointer(VALUE obj, Object::Instance * instance, const Smoke::ModuleIndex& classId, void *lastptr)
{
    Smoke * smoke = classId.smoke;
    void * ptr = instance->cast(classId);
     
    if (ptr != lastptr) {
        lastptr = ptr; 
        
        if (Debug::DoDebug & Debug::GC) {
            Object::Instance * instance = Object::Instance::get(obj);
            const char *className = instance->classId.smoke->classes[instance->classId.index].className;
            qWarning("QtRuby::Global::mapPointer (%s*)%p -> %p size: %d", className, ptr, (void*)obj, rubyValues()->size() + 1);
        }
        
        QtRuby::Object value(obj, instance);
        rubyValues()->insert(ptr, value);
    }
    
    for (   Smoke::Index * parent = smoke->inheritanceList + smoke->classes[classId.index].parents; 
            *parent != 0; 
            parent++ ) 
    {
        if (smoke->classes[*parent].external) {
            Smoke::ModuleIndex mi = Smoke::findClass(smoke->classes[*parent].className);
            if (mi != Smoke::NullModuleIndex) {
                mapPointer(obj, instance, mi, lastptr);
            }
        } else {
            mapPointer(obj, instance, Smoke::ModuleIndex(smoke, *parent), lastptr);
        }
    }
    
    return;
}

QByteArray
rubyClassNameFromId(const Smoke::ModuleIndex& classId)
{
    if (metaObjects()->contains(classId))
        return QByteArray(metaObjects()->value(classId)->rubyClassName());
    else
        return QByteArray();
}

VALUE
rubyClassFromId(const Smoke::ModuleIndex& classId)
{
    if (metaObjects()->contains(classId))
        return metaObjects()->value(classId)->rubyClass;
    else
        return Qnil;
}

Smoke::ModuleIndex
idFromRubyClass(VALUE klass)
{
    if (rubyMetaObjects()->contains(klass))
        return rubyMetaObjects()->value(klass)->classId;
    else
        return Smoke::NullModuleIndex;
}

VALUE
wrapInstance(const Smoke::ModuleIndex& classId, void * ptr, Object::ValueOwnership ownership, VALUE klass)
{
    // qDebug() << Q_FUNC_INFO << "className:" << classId.smoke->classes[classId.index].className << " ownership:" << ownership;
    
    Object::Instance * instance = new Object::Instance();
    instance->classId = classId;
    instance->value = ptr;
    instance->ownership = ownership;
    resolveType(instance);

    if (klass == Qnil)
        klass = rubyClassFromId(classId);

    Q_ASSERT(metaObjects()->contains(classId));
    MetaObject * meta = metaObjects()->value(classId);
    VALUE obj = Data_Wrap_Struct(klass, meta->mark, meta->free, (void *) instance);
    
    if (ownership != Object::QtOwnership) {
        mapPointer(obj, instance, instance->classId);
    }

    return obj;
}

static MetaObject *
createMetaObject(const  Smoke::ModuleIndex& classId)
{
    MetaObject * meta = 0;
    if (metaObjects()->contains(classId)) {
        meta = metaObjects()->value(classId);
    } else {
        meta = new MetaObject();
        metaObjects()->insert(classId, meta);
    }

    return meta;
}

void
defineMethod(const Smoke::ModuleIndex& classId, const char* name, VALUE (*func)(ANYARGS), int argc)
{
    MetaObject * meta = createMetaObject(classId);
    MetaObject::RubyMethod method;
    method.name = QByteArray(name);
    method.func = func;
    method.argc = argc;
    meta->rubyMethods.append(method);
}

void 
defineTypeResolver(const Smoke::ModuleIndex& classId, Object::TypeResolver typeResolver)
{
    MetaObject * meta = createMetaObject(classId);
    meta->resolver = typeResolver;
}

void 
resolveType(Object::Instance * instance)
{
    Q_ASSERT(metaObjects()->contains(instance->classId));
    MetaObject * meta = metaObjects()->value(instance->classId);

    if (meta->resolver != 0) {
        (*meta->resolver)(instance);
    }
}

static VALUE
version(VALUE /*self*/)
{
    return rb_str_new2(QT_VERSION_STR);
}

static VALUE
qtruby_version(VALUE /*self*/)
{
    return rb_str_new2(QTRUBY_VERSION);
}

void
initialize()
{
    QtModule = rb_define_module("Qt");
    QtInternalModule = rb_define_module_under(QtModule, "Internal");
    QtBaseClass = rb_define_class_under(QtModule, "Base", rb_cObject);

    rb_define_singleton_method(QtBaseClass, "new", (VALUE (*) (...)) new_qt, -1);
    rb_define_method(QtBaseClass, "initialize", (VALUE (*) (...)) initialize_qt, -1);
    rb_define_singleton_method(QtBaseClass, "method_missing", (VALUE (*) (...)) class_method_missing, -1);
    rb_define_singleton_method(QtModule, "method_missing", (VALUE (*) (...)) module_method_missing, -1);
    rb_define_method(QtBaseClass, "method_missing", (VALUE (*) (...)) method_missing, -1);

    rb_define_singleton_method(QtBaseClass, "const_missing", (VALUE (*) (...)) class_method_missing, -1);
    rb_define_singleton_method(QtModule, "const_missing", (VALUE (*) (...)) module_method_missing, -1);
    rb_define_method(QtBaseClass, "const_missing", (VALUE (*) (...)) method_missing, -1);
    
    rb_define_method(QtBaseClass, "dispose", (VALUE (*) (...)) dispose, 0);
    rb_define_method(QtBaseClass, "isDisposed", (VALUE (*) (...)) is_disposed, 0);
    rb_define_method(QtBaseClass, "disposed?", (VALUE (*) (...)) is_disposed, 0);
    
    rb_define_module_function(QtInternalModule, "cast_object_to", (VALUE (*) (...)) cast_object_to, 2);
    rb_define_module_function(QtModule, "dynamic_cast", (VALUE (*) (...)) cast_object_to, 2);

    rb_define_method(rb_cObject, "qDebug", (VALUE (*) (...)) Debug::qdebug, 1);
    rb_define_method(rb_cObject, "qFatal", (VALUE (*) (...)) Debug::qfatal, 1);
    rb_define_method(rb_cObject, "qWarning", (VALUE (*) (...)) Debug::qwarning, 1);
    rb_define_module_function(QtInternalModule, "setDebug", (VALUE (*) (...)) Debug::setDebug, 1);
    rb_define_module_function(QtInternalModule, "debug", (VALUE (*) (...)) Debug::debugging, 0);
    
    rb_define_module_function(QtInternalModule, "set_qtruby_embedded", (VALUE (*) (...)) set_qtruby_embedded_wrapped, 1);
    rb_define_module_function(QtInternalModule, "application_terminated=", (VALUE (*) (...)) set_application_terminated, 1);
    
    rb_define_module_function(QtModule, "version", (VALUE (*) (...)) version, 0);
    rb_define_module_function(QtModule, "qtruby_version", (VALUE (*) (...)) qtruby_version, 0);

    Debug::DoDebug = (Debug::MethodMatches | Debug::MethodMissing | Debug::Calls | Debug::GC);
    // Debug::DoDebug = (Debug::MethodMatches | Debug::MethodMissing | Debug::Calls | Debug::GC | Debug::Virtual);
}

static void
initializeMetaObject(const Smoke::ModuleIndex& classId, MetaObject * meta)
{
    Smoke * smoke = classId.smoke;
    if (metaObjects()->contains(classId)) {
        MetaObject * current = metaObjects()->value(classId);

        Q_FOREACH(MetaObject::RubyMethod method, current->rubyMethods) {
            rb_define_method(meta->rubyClass, method.name, method.func, method.argc);
        }

        if (meta->resolver == 0 && current->resolver != 0)
            meta->resolver = current->resolver;
    }

    for (   Smoke::Index * parent = smoke->inheritanceList + smoke->classes[classId.index].parents;
            *parent != 0;
            parent++ )
    {
        if (smoke->classes[*parent].external) {
            Smoke::ModuleIndex mi = Smoke::findClass(smoke->classes[*parent].className);
            if (mi != Smoke::NullModuleIndex) {
                initializeMetaObject(mi, meta);
            }
        } else {
            initializeMetaObject(Smoke::ModuleIndex(smoke, *parent), meta);
        }
    }
}

VALUE
initializeClass(const Smoke::ModuleIndex& classId, const QString& rubyClassName)
{
    QStringList components = rubyClassName.split("::");
    VALUE klass = rb_define_module(components[0].toLatin1().constData());
    rb_define_singleton_method(klass, "method_missing", (VALUE (*) (...)) module_method_missing, -1);
    rb_define_singleton_method(klass, "const_missing", (VALUE (*) (...)) module_method_missing, -1);
    
    for (int i = 1; i < components.count(); ++i) {
        klass = rb_define_class_under(klass, components[i].toLatin1().constData(), QtBaseClass);
    }

    MetaObject * meta = createMetaObject(classId);
    meta->mark = Object::mark;
    meta->free = Object::free;
    meta->rubyClass = klass;
    meta->classId = classId;
    initializeMetaObject(classId, meta);
    rubyMetaObjects()->insert(klass, meta);
    
    return klass;
}

    } // namespace Global
} // namespace QtRuby