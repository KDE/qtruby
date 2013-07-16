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

#include <QtCore/qdebug.h>
#include <QtCore/QMetaMethod>
#include <QtCore/QMetaObject>
#include <QtCore/QHash>

#include "object.h"
#include "global.h"
#include "invokeslot.h"
#include "rubyqmetaobject.h"

#define DEBUG

// The code to dynamically create QMetaObjects is derived from
// qmetaobjectbuilder.cpp in Qt Declarative

namespace QtRuby {

// copied from qmetaobject.cpp
// do not touch without touching the moc as well
enum PropertyFlags  {
    Invalid = 0x00000000,
    Readable = 0x00000001,
    Writable = 0x00000002,
    Resettable = 0x00000004,
    EnumOrFlag = 0x00000008,
    StdCppSet = 0x00000100,
//    Override = 0x00000200,
    Designable = 0x00001000,
    ResolveDesignable = 0x00002000,
    Scriptable = 0x00004000,
    ResolveScriptable = 0x00008000,
    Stored = 0x00010000,
    ResolveStored = 0x00020000,
    Editable = 0x00040000,
    ResolveEditable = 0x00080000,
    User = 0x00100000,
    ResolveUser = 0x00200000,
    Notify = 0x00400000,
    Revisioned = 0x00800000
};

enum MethodFlags  {
    AccessPrivate = 0x00,
    AccessProtected = 0x01,
    AccessPublic = 0x02,
    AccessMask = 0x03, //mask

    MethodMethod = 0x00,
    MethodSignal = 0x04,
    MethodSlot = 0x08,
    MethodConstructor = 0x0c,
    MethodTypeMask = 0x0c,

    MethodCompatibility = 0x10,
    MethodCloned = 0x20,
    MethodScriptable = 0x40,
    MethodRevisioned = 0x80
};

struct QMetaObjectPrivate
{
    int revision;
    int className;
    int classInfoCount, classInfoData;
    int methodCount, methodData;
    int propertyCount, propertyData;
    int enumeratorCount, enumeratorData;
    int constructorCount, constructorData;
    int flags;
    int signalCount;
};

struct MetaMethodBuilder
{
    QByteArray signature;
    QByteArray returnType;
    QList<QByteArray> parameterNames;
    QByteArray tag;
    int attributes;
};

struct MetaClassInfoBuilder
{
    QByteArray name;
    QByteArray value;
};

struct MetaObjectBuilder {
    MetaObjectBuilder()
    : changed(false), superClass(0), flags(0), rubyMetaObject(Qnil), metaObject(0)
    {
    }
    void addMethods(int argc, VALUE * argv, int attributes);
    bool changed;
    QMetaObject *superClass;
    int flags;
    int signalCount;
    QByteArray className;
    QVector<MetaClassInfoBuilder> classInfos;
    QVector<MetaMethodBuilder> signalMethods;
    QVector<MetaMethodBuilder> slotMethods;
    QVector<MetaMethodBuilder> methods;
    VALUE rubyMetaObject;
    QMetaObject * metaObject;
#ifdef DEBUG
    void dump(const QMetaObject * meta, int stringDataLength);
#endif
};

// Align on a specific type boundary.
#define ALIGN(size,type)    \
    (size) = ((size) + sizeof(type) - 1) & ~(sizeof(type) - 1)

// Build a string into a QMetaObject representation.  Returns the
// position in the string table where the string was placed.
static int
buildString(char *buf, char *str, int *offset, const QByteArray& value, int empty)
{
    if (value.size() == 0 && empty >= 0)
        return empty;
    if (buf) {
        memcpy(str + *offset, value.constData(), value.size());
        str[*offset + value.size()] = '\0';
    }
    int posn = *offset;
    *offset += value.size() + 1;
    return posn;
}

// Build the parameter array string for a method.
static QByteArray
buildParameterNames(const QByteArray& signature, const QList<QByteArray>& parameterNames)
{
    // If the parameter name list is specified, then concatenate them.
    if (!parameterNames.isEmpty()) {
        QByteArray names;
        bool first = true;
        foreach (const QByteArray &name, parameterNames) {
            if (first)
                first = false;
            else
                names += (char)',';
            names += name;
        }
        return names;
    }

    // Count commas in the signature, excluding those inside template arguments.
    int index = signature.indexOf('(');
    if (index < 0)
        return QByteArray();
    ++index;
    if (index >= signature.size())
        return QByteArray();
    if (signature[index] == ')')
        return QByteArray();
    int count = 1;
    int brackets = 0;
    while (index < signature.size() && signature[index] != ',') {
        char ch = signature[index++];
        if (ch == '<')
            ++brackets;
        else if (ch == '>')
            --brackets;
        else if (ch == ',' && brackets <= 0)
            ++count;
    }
    return QByteArray(count - 1, ',');
}

// Build a QMetaObject in "buf" based on the information in "d".
// If "buf" is null, then return the number of bytes needed to
// build the QMetaObject.
static int buildMetaObject(MetaObjectBuilder *d, char *buf)
{
    int size = 0;
    int dataIndex;
    int enumIndex;
    int index;

    // Create the main QMetaObject structure at the start of the buffer.
    QMetaObject *meta = reinterpret_cast<QMetaObject *>(buf);
    size += sizeof(QMetaObject);
    ALIGN(size, int);
    if (buf) {
        meta->d.superdata = d->superClass;
        meta->d.extradata = 0;
    }

    // Populate the QMetaObjectPrivate structure.
    QMetaObjectPrivate *pmeta
        = reinterpret_cast<QMetaObjectPrivate *>(buf + size);
    int pmetaSize = size;
    dataIndex = 14;     // Number of fields in the QMetaObjectPrivate.
    if (buf) {
        pmeta->revision = 3;
        pmeta->flags = d->flags;
        pmeta->signalCount = d->signalCount;
        pmeta->className = 0;   // Class name is always the first string.

        pmeta->classInfoCount = d->classInfos.size();
        pmeta->classInfoData = dataIndex;
        dataIndex += 2 * d->classInfos.size();

        pmeta->methodCount = d->methods.size();
        pmeta->methodData = dataIndex;
        dataIndex += 5 * d->methods.size();
        pmeta->propertyCount = 0;
        pmeta->propertyData = 0;
        pmeta->enumeratorCount = 0;
        pmeta->enumeratorData = 0;
        pmeta->constructorCount = 0;
        pmeta->constructorData = 0;
    } else {
        dataIndex += 2 * d->classInfos.size();
        dataIndex += 5 * d->methods.size();
    }

    enumIndex = dataIndex;

    // Zero terminator at the end of the data offset table.
    ++dataIndex;

    // Find the start of the data and string tables.
    int *data = reinterpret_cast<int *>(pmeta);
    size += dataIndex * sizeof(int);
    char *str = reinterpret_cast<char *>(buf + size);
    if (buf) {
        meta->d.stringdata = str;
        meta->d.data = reinterpret_cast<uint *>(data);
    }

    // Reset the current data position to just past the QMetaObjectPrivate.
    dataIndex = 14;

    // Add the class name to the string table.
    int offset = 0;
    buildString(buf, str, &offset, d->className, -1);

    // Add a common empty string, which is used to indicate "void"
    // method returns, empty tag strings, etc.
    int empty = buildString(buf, str, &offset, QByteArray(), -1);

    // Output the class infos,
    for (index = 0; index < d->classInfos.size(); ++index) {
        int name = buildString(buf, str, &offset, d->classInfos[index].name, empty);
        int value = buildString(buf, str, &offset, d->classInfos[index].value, empty);
        if (buf) {
            data[dataIndex] = name;
            data[dataIndex + 1] = value;
        }
        dataIndex += 2;
    }

    // Output the methods in the class.
    for (index = 0; index < d->methods.size(); ++index) {
        MetaMethodBuilder *method = &(d->methods[index]);
        int sig = buildString(buf, str, &offset, method->signature, empty);
        int params;
        QByteArray names = buildParameterNames(method->signature, method->parameterNames);
        params = buildString(buf, str, &offset, names, empty);
        int ret = buildString(buf, str, &offset, method->returnType, empty);
        int tag = buildString(buf, str, &offset, method->tag, empty);
        int attrs = method->attributes;
        if (buf) {
            data[dataIndex]     = sig;
            data[dataIndex + 1] = params;
            data[dataIndex + 2] = ret;
            data[dataIndex + 3] = tag;
            data[dataIndex + 4] = attrs;
        }
        dataIndex += 5;
    }

    // One more empty string to act as a terminator.
    buildString(buf, str, &offset, QByteArray(), -1);

#ifdef DEBUG
    if (buf)
        d->dump(meta, offset);
#endif    
    size += offset;

    // Output the zero terminator in the data array.
    if (buf)
        data[enumIndex] = 0;


    // Align the final size and return it.
    ALIGN(size, void *);
    return size;
}

static VALUE
toMetaObject(MetaObjectBuilder * meta)
{
    qDebug() << Q_FUNC_INFO;
    if (meta->metaObject != 0) {
        Object::Instance * instance = Object::Instance::get(meta->rubyMetaObject);
        Q_ASSERT(instance->value == meta->metaObject);
        instance->value = 0;
        qFree(meta->metaObject);
    }
    meta->signalCount = meta->signalMethods.count();
    meta->methods = meta->signalMethods + meta->slotMethods;
    int size = buildMetaObject(meta, 0);
    
    char *buf = reinterpret_cast<char *>(qMalloc(size));
    buildMetaObject(meta, buf);
    meta->metaObject = reinterpret_cast<QMetaObject *>(buf);
    qDebug() << Q_FUNC_INFO << "class name:" <<  meta->metaObject->className();   
    meta->rubyMetaObject = Global::wrapInstance(Global::QMetaObjectClassId, meta->metaObject, Object::ScriptOwnership);
    return meta->rubyMetaObject;
}

typedef QHash<VALUE, MetaObjectBuilder *> MetaObjectBuilderMap;
Q_GLOBAL_STATIC(MetaObjectBuilderMap, metaObjectBuilders)

static MetaObjectBuilder *
createMetaObjectBuilder(VALUE klass)
{
    // qDebug() << Q_FUNC_INFO;
    MetaObjectBuilder * meta = 0;
    if (metaObjectBuilders()->contains(klass)) {
        meta = metaObjectBuilders()->value(klass);
    } else {
        meta = new MetaObjectBuilder();
        metaObjectBuilders()->insert(klass, meta);
        VALUE name = rb_funcall(klass, rb_intern("name"), 0);
        meta->className = QByteArray(StringValuePtr(name));
        rb_define_method(klass, "qt_metacall", (VALUE (*) (...)) qt_metacall, -1);
        rb_define_method(klass, "metaObject", (VALUE (*) (...)) metaObject, 0);
    }

    return meta;
}

static QMetaObject*
parentMetaObject(VALUE obj)
{
    qDebug() << Q_FUNC_INFO;
    Object::Instance * instance = Object::Instance::get(obj);
    Smoke::ModuleIndex nameId = instance->classId.smoke->idMethodName("metaObject");
    Smoke::ModuleIndex methodId = instance->classId.smoke->findMethod(instance->classId, nameId);
    if (methodId == Smoke::NullModuleIndex) {
        // Should never happen..
        return 0;
    }

    const Smoke::Method &methodRef = methodId.smoke->methods[methodId.smoke->methodMaps[methodId.index].method];
    Smoke::ClassFn fn = instance->classId.smoke->classes[methodRef.classId].classFn;
    Smoke::StackItem stack[1];
    (*fn)(methodRef.method, instance->value, stack);
    return reinterpret_cast<QMetaObject*>(stack[0].s_voidp);
}

static QMetaObject *
createMetaObject(VALUE klass, VALUE self)
{
    qDebug() << Q_FUNC_INFO;
    MetaObjectBuilder * meta = createMetaObjectBuilder(klass);
    VALUE superclass = rb_funcall(klass, rb_intern("superclass"), 0);
    Smoke::ModuleIndex superClassId = Global::idFromRubyClass(superclass);
    if (superClassId == Smoke::NullModuleIndex) {
        meta->superClass = createMetaObject(superclass, self);
    } else {
        meta->superClass = parentMetaObject(self);
    }

    meta->rubyMetaObject = toMetaObject(meta);
    meta->changed = false;
    return meta->metaObject;
}

VALUE
metaObject(VALUE self)
{
    // qDebug() << Q_FUNC_INFO;
    VALUE klass = rb_funcall(self, rb_intern("class"), 0);
    MetaObjectBuilder * meta = createMetaObjectBuilder(klass);
    if (meta->changed)
        meta->metaObject = createMetaObject(klass, self);
    return meta->rubyMetaObject;
}

ID
signatureToMethodID(const QByteArray& signature)
{
    QByteArray methodName;
    int index = signature.indexOf('(');
    if (index != -1) {
        methodName = signature.mid(0, index);
    }

    return rb_intern(methodName);
}

VALUE
qt_metacall(int argc, VALUE * argv, VALUE self)
{
    qDebug() << Q_FUNC_INFO << "argc:" << argc;
    QMetaObject::Call _c = static_cast<QMetaObject::Call>(NUM2UINT(rb_funcall(argv[0], rb_intern("to_i"), 0)));
    int _id = NUM2INT(argv[1]);
    void ** _a = 0;

    // Note that for a slot with no args and no return type,
    // it isn't an error to get a NULL value of _a here.
    Data_Get_Struct(argv[2], void*, _a);
    qDebug() << Q_FUNC_INFO << "c_:" <<  _c << "_id:" << _id << "_a:" << _a;
    
    Object::Instance * instance = Object::Instance::get(self);
    Smoke::ModuleIndex nameId = instance->classId.smoke->idMethodName("qt_metacall$$?");
    Smoke::ModuleIndex methodId = nameId.smoke->findMethod(instance->classId, nameId);
    if (methodId == Smoke::NullModuleIndex) {
        // Should never happen..
        rb_raise(   rb_eRuntimeError,
                    "Cannot find %s::qt_metacall() method\n",
                    instance->classId.smoke->classes[instance->classId.index].className );
        return _id;
    }
    
    const Smoke::Method &methodRef = methodId.smoke->methods[methodId.smoke->methodMaps[methodId.index].method];
    Smoke::ClassFn fn = methodId.smoke->classes[methodRef.classId].classFn;
    Smoke::StackItem stack[4];
    stack[1].s_enum = _c;
    stack[2].s_int = _id;
    stack[3].s_voidp = _a;
    (*fn)(methodRef.method, instance->value, stack);
    _id = stack[0].s_int;
    if (_id < 0) {
        return INT2NUM(_id);
    }
    
    VALUE klass = rb_funcall(self, rb_intern("class"), 0);
    MetaObjectBuilder * builder = createMetaObjectBuilder(klass);
    QMetaObject * meta = builder->metaObject;

    if (_c == QMetaObject::InvokeMetaMethod) {
        QMetaMethod method = meta->method(meta->methodOffset() + _id);
        ID methodID = signatureToMethodID(method.signature());
        qDebug() << Q_FUNC_INFO << method.signature();
        InvokeSlot slot(self, methodID, ALLOCA_N(VALUE, method.parameterTypes().count()), method, _a);
        slot.next();
    }

    return INT2NUM(_id);
}

void
MetaObjectBuilder::addMethods(int argc, VALUE * argv, int attributes)
{
    for (int i = 0; i < argc; ++i) {
        MetaMethodBuilder method;
        if (TYPE(argv[i]) == T_SYMBOL) {
            method.signature = QByteArray(rb_id2name(SYM2ID(argv[i])));
            method.signature.append("()");
        } else {
            method.signature = QByteArray(StringValuePtr(argv[i]));
            int index = method.signature.indexOf('(');
            int space = method.signature.lastIndexOf(' ', index);
            if (index != -1 && space != -1 && space < index) {
                method.returnType = method.signature.left(space);
                method.signature = method.signature.mid(space + 1);
            }
        }

        method.signature = QMetaObject::normalizedSignature(method.signature);
        method.attributes = attributes;
        if ((attributes & MethodSignal) != 0) {
            signalMethods.append(method);
        } else {
            slotMethods.append(method);
        }
    }
    // qDebug() << Q_FUNC_INFO << "returnType" << method.returnType << " signature:" << method.signature;
    return;
}

VALUE ruby_slots(int argc, VALUE * argv, VALUE self)
{
    MetaObjectBuilder * meta = createMetaObjectBuilder(self);
    meta->addMethods(argc, argv, MethodSlot | AccessPublic);
    meta->changed = true;
    return Qnil;
}

VALUE ruby_private_slots(int argc, VALUE * argv, VALUE self)
{
    MetaObjectBuilder * meta = createMetaObjectBuilder(self);
    meta->addMethods(argc, argv, MethodSlot | AccessPrivate);
    meta->changed = true;
    return Qnil;
}

VALUE ruby_signals(int argc, VALUE * argv, VALUE self)
{
    MetaObjectBuilder * meta = createMetaObjectBuilder(self);
    meta->addMethods(argc, argv, MethodSignal | AccessPublic);
    meta->changed = true;
    return Qnil;
}

VALUE ruby_classinfo(VALUE self, VALUE name, VALUE value)
{
    MetaObjectBuilder * meta = createMetaObjectBuilder(self);
    MetaClassInfoBuilder classInfo;
    classInfo.name = QByteArray(StringValuePtr(name));
    classInfo.value = QByteArray(StringValuePtr(value));
    meta->classInfos.append(classInfo);
    meta->changed = true;
    return Qnil;
}

#ifdef DEBUG
void MetaObjectBuilder::dump(const QMetaObject * meta, int stringDataLength)
{
    printf("make_metaObject() superdata: %p %s\n", meta->d.superdata, meta->d.superdata->className());

    printf(
    " // content:\n"
    "    %4d,       // revision\n"
    "    %4d,       // classname\n"
    "    %4d, %4d, // classinfo\n"
    "    %4d, %4d, // methods\n"
    "    %4d, %4d, // properties\n"
    "    %4d, %4d, // enums/sets\n"
    "    %4d, %4d, // constructors\n"
    "    %4d,       // flags\n"
    "    %4d,       // signalCount\n",
    meta->d.data[0], meta->d.data[1], meta->d.data[2], meta->d.data[3],
    meta->d.data[4], meta->d.data[5],
    meta->d.data[6], meta->d.data[7],
    meta->d.data[8], meta->d.data[9],
    meta->d.data[10], meta->d.data[11],
    meta->d.data[12], meta->d.data[13] );

    int s = meta->d.data[3];

    if (meta->d.data[2] > 0) {
        printf("\n // classinfo: key, value\n");
        for (uint j = 0; j < meta->d.data[2]; j++) {
            printf("      %d,    %d\n", meta->d.data[s + (j * 2)], meta->d.data[s + (j * 2) + 1]);
        }
    }

    s = meta->d.data[5];
    bool signal_headings = true;
    bool slot_headings = true;
    bool method_headings = true;

    for (uint j = 0; j < meta->d.data[4]; j++) {
        if (signal_headings && (meta->d.data[s + (j * 5) + 4] & 0x04) != 0) {
            printf("\n // signals: signature, parameters, type, tag, flags\n");
            signal_headings = false;
        }

        if (slot_headings && (meta->d.data[s + (j * 5) + 4] & 0x08) != 0) {
            printf("\n // slots: signature, parameters, type, tag, flags\n");
            slot_headings = false;
        }

        if (    method_headings
                && (meta->d.data[s + (j * 5) + 4] & 0x04) == 0
                && (meta->d.data[s + (j * 5) + 4] & 0x08) == 0 )
        {
            printf("\n // methods: signature, parameters, type, tag, flags\n");
            method_headings = false;
        }

        printf("    %4d, %4d, %4d, %4d, 0x%2.2x\n",
            meta->d.data[s + (j * 5)], meta->d.data[s + (j * 5) + 1], meta->d.data[s + (j * 5) + 2],
            meta->d.data[s + (j * 5) + 3], meta->d.data[s + (j * 5) + 4]);
    }

    if (meta->d.data[6] != 0) {
        printf("\n // properties: name, type, flags\n");
    }

    s = meta->d.data[7];
    for (uint j = 0; j < meta->d.data[6]; j++) {
        printf("    %4d, %4d, 0x%.8x\n",
            meta->d.data[s + (j * 3)], meta->d.data[s + (j * 3) + 1], meta->d.data[s + (j * 3) + 2]);
    }

    if (meta->d.data[6] != 0) {
        printf("\n // properties: notify_signal_id\n");
    }

    s += (meta->d.data[6] * 3);
    for (uint j = 0; j < meta->d.data[6]; j++) {
        printf("    %4d,\n",
            meta->d.data[s + j]);
    }

    if (meta->d.data[8] != 0) {
        printf("\n // enums: name, flags, count, data\n");
    }

    s = meta->d.data[9];
    for (uint j = 0; j < meta->d.data[8]; j++) {
        printf("    %4d, 0x%.1x, %4d, %4d\n",
            meta->d.data[s + (j * 4)], meta->d.data[s + (j * 4) + 1], meta->d.data[s + (j * 4) + 2], meta->d.data[s + (j * 4) + 3]);
    }

    s += (meta->d.data[8] * 4);

    if (meta->d.data[8] != 0) {
        printf("\n // enum data: key, value\n");

        for (uint j = 0; j < meta->d.data[s + (j * 2)] != 0; j++) {
            printf("    %4d, %4d\n",
                meta->d.data[s + (j * 2)], meta->d.data[s + (j * 2) + 1]);
        }
    }

    s = meta->d.data[11];

    if (meta->d.data[10] != 0) {
        printf("\n // constructors: signature, parameters, type, tag, flags\n");

        for (uint j = 0; j < meta->d.data[10]; j++) {
            printf("    %4d, %4d, %4d, %4d, 0x%02x\n",
                meta->d.data[s + (j * 5)], meta->d.data[s + (j * 5) + 1], meta->d.data[s + (j * 5) + 2],
                meta->d.data[s + (j * 5) + 3], meta->d.data[s + (j * 5) + 4]);
        }
    }

    printf("\n       0        // eod\n");

    printf("\nqt_meta_stringdata:\n    \"");

    int strlength = 0;
    for (int j = 0; j < stringDataLength; j++) {
        strlength++;
        if (meta->d.stringdata[j] == 0) {
            printf("\\0");
            if (strlength > 40) {
                printf("\"\n    \"");
                strlength = 0;
            }
        } else {
            printf("%c", meta->d.stringdata[j]);
        }
    }
    printf("\"\n\n");

    return;
}
#endif

}
