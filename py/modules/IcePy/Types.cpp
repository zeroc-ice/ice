// **********************************************************************
//
// Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Types.h>
#include <Util.h>
//#include <map>

using namespace std;
using namespace IcePy;

// TODO: Destroyer for maps?

typedef map<string, TypeInfoPtr> TypeInfoMap;
static TypeInfoMap _typeInfoMap;

typedef map<string, ExceptionInfoPtr> ExceptionInfoMap;
static ExceptionInfoMap _exceptionInfoMap;

static PrimitiveInfoPtr _boolType;
static PrimitiveInfoPtr _byteType;
static PrimitiveInfoPtr _shortType;
static PrimitiveInfoPtr _intType;
static PrimitiveInfoPtr _longType;
static PrimitiveInfoPtr _floatType;
static PrimitiveInfoPtr _doubleType;
static PrimitiveInfoPtr _stringType;

IcePy::TypeInfo::TypeInfo()
{
}

void
IcePy::TypeInfo::destroy()
{
}

string
IcePy::PrimitiveInfo::getName() const
{
    switch(kind)
    {
    case KindBool:
        return "bool";
    case KindByte:
        return "byte";
    case KindShort:
        return "short";
    case KindInt:
        return "int";
    case KindLong:
        return "long";
    case KindFloat:
        return "float";
    case KindDouble:
        return "double";
    case KindString:
        return "string";
    }
    assert(false);
}

string
IcePy::EnumInfo::getName() const
{
    return name;
}

string
IcePy::StructInfo::getName() const
{
    return name;
}

void
IcePy::StructInfo::destroy()
{
    for(DataMemberList::iterator p = members.begin(); p != members.end(); ++p)
    {
        p->type->destroy();
    }
    members.clear();
}

string
IcePy::SequenceInfo::getName() const
{
    return name;
}

void
IcePy::SequenceInfo::destroy()
{
    elementType->destroy();
    elementType = 0;
}

string
IcePy::DictionaryInfo::getName() const
{
    return name;
}

void
IcePy::DictionaryInfo::destroy()
{
    keyType->destroy();
    keyType = 0;
    valueType->destroy();
    valueType = 0;
}

bool
IcePy::OperationInfo::validateException(PyObject* ex) const
{
    for(ExceptionInfoList::const_iterator p = exceptions.begin(); p != exceptions.end(); ++p)
    {
        if(PyObject_IsInstance(ex, (*p)->pythonType.get()))
        {
            return true;
        }
    }

    return false;
}

string
IcePy::ClassInfo::getName() const
{
    return name;
}

void
IcePy::ClassInfo::destroy()
{
    base = 0;
    interfaces.clear();
    for(DataMemberList::iterator p = members.begin(); p != members.end(); ++p)
    {
        p->type->destroy();
    }
    members.clear();
    operations.clear();
}

IcePy::OperationInfoPtr
IcePy::ClassInfo::findOperation(const string& name) const
{
    OperationInfoMap::const_iterator p = operations.find(name);
    if(p != operations.end())
    {
        return p->second;
    }

    if(base)
    {
        OperationInfoPtr op = base->findOperation(name);
        if(op)
        {
            return op;
        }
    }

    for(ClassInfoList::const_iterator q = interfaces.begin(); q != interfaces.end(); ++q)
    {
        OperationInfoPtr op = (*q)->findOperation(name);
        if(op)
        {
            return op;
        }
    }

    return 0;
}

bool
IcePy::ClassInfo::hasOperations() const
{
    if(!operations.empty())
    {
        return true;
    }

    if(base && base->hasOperations())
    {
        return true;
    }

    for(ClassInfoList::const_iterator q = interfaces.begin(); q != interfaces.end(); ++q)
    {
        if((*q)->hasOperations())
        {
            return true;
        }
    }

    return false;
}

string
IcePy::ProxyInfo::getName() const
{
    return name;
}

bool
IcePy::initTypes(PyObject* module)
{
    _boolType = new PrimitiveInfo;
    _boolType->kind = PrimitiveInfo::KindBool;
    _byteType = new PrimitiveInfo;
    _byteType->kind = PrimitiveInfo::KindByte;
    _shortType = new PrimitiveInfo;
    _shortType->kind = PrimitiveInfo::KindShort;
    _intType = new PrimitiveInfo;
    _intType->kind = PrimitiveInfo::KindInt;
    _longType = new PrimitiveInfo;
    _longType->kind = PrimitiveInfo::KindLong;
    _floatType = new PrimitiveInfo;
    _floatType->kind = PrimitiveInfo::KindFloat;
    _doubleType = new PrimitiveInfo;
    _doubleType->kind = PrimitiveInfo::KindDouble;
    _stringType = new PrimitiveInfo;
    _stringType->kind = PrimitiveInfo::KindString;

    if(!PyModule_AddIntConstant(module, "T_BOOL", TYPE_BOOL) < 0)
    {
        return false;
    }
    if(!PyModule_AddIntConstant(module, "T_BYTE", TYPE_BYTE) < 0)
    {
        return false;
    }
    if(!PyModule_AddIntConstant(module, "T_SHORT", TYPE_SHORT) < 0)
    {
        return false;
    }
    if(!PyModule_AddIntConstant(module, "T_INT", TYPE_INT) < 0)
    {
        return false;
    }
    if(!PyModule_AddIntConstant(module, "T_LONG", TYPE_LONG) < 0)
    {
        return false;
    }
    if(!PyModule_AddIntConstant(module, "T_FLOAT", TYPE_FLOAT) < 0)
    {
        return false;
    }
    if(!PyModule_AddIntConstant(module, "T_DOUBLE", TYPE_DOUBLE) < 0)
    {
        return false;
    }
    if(!PyModule_AddIntConstant(module, "T_STRING", TYPE_STRING) < 0)
    {
        return false;
    }
    if(!PyModule_AddIntConstant(module, "T_OBJECT", TYPE_OBJECT) < 0)
    {
        return false;
    }
    if(!PyModule_AddIntConstant(module, "T_OBJECT_PROXY", TYPE_OBJECT_PROXY) < 0)
    {
        return false;
    }
    if(!PyModule_AddIntConstant(module, "OP_NORMAL", OP_NORMAL) < 0)
    {
        return false;
    }
    if(!PyModule_AddIntConstant(module, "OP_NONMUTATING", OP_NONMUTATING) < 0)
    {
        return false;
    }
    if(!PyModule_AddIntConstant(module, "OP_IDEMPOTENT", OP_IDEMPOTENT) < 0)
    {
        return false;
    }
    return true;
}

IcePy::TypeInfoPtr
IcePy::getTypeInfo(const string& id)
{
    TypeInfoMap::iterator p = _typeInfoMap.find(id);
    if(p != _typeInfoMap.end())
    {
        return p->second;
    }
    return 0;
}

IcePy::ExceptionInfoPtr
IcePy::getExceptionInfo(const string& id)
{
    ExceptionInfoMap::iterator p = _exceptionInfoMap.find(id);
    if(p != _exceptionInfoMap.end())
    {
        return p->second;
    }
    return 0;
}

static IcePy::TypeInfoPtr
convertType(PyObject* obj)
{
    if(PyInt_Check(obj))
    {
        int i = static_cast<int>(PyInt_AS_LONG(obj));
        switch(i)
        {
        case TYPE_BOOL:
            return _boolType;
        case TYPE_BYTE:
            return _byteType;
        case TYPE_SHORT:
            return _shortType;
        case TYPE_INT:
            return _intType;
        case TYPE_LONG:
            return _longType;
        case TYPE_FLOAT:
            return _floatType;
        case TYPE_DOUBLE:
            return _doubleType;
        case TYPE_STRING:
            return _stringType;
        case TYPE_OBJECT:
            return getTypeInfo("::Ice::Object");
        case TYPE_OBJECT_PROXY:
            return getTypeInfo("::Ice::ObjectPrx");
        }
    }
    else
    {
        assert(PyString_Check(obj));
        char* id = PyString_AS_STRING(obj);
        TypeInfoMap::iterator p = _typeInfoMap.find(id);
        if(p != _typeInfoMap.end())
        {
            return p->second;
        }
    }

    return 0;
}

extern "C"
PyObject*
Ice_addEnum(PyObject*, PyObject* args)
{
    char* id;
    PyObject* type;
    PyObject* enumerators;
    if(!PyArg_ParseTuple(args, "sOO", &id, &type, &enumerators))
    {
        return NULL;
    }

    assert(PyType_Check(type));
    assert(PyTuple_Check(enumerators));

    EnumInfoPtr info = new EnumInfo;
    info->name = id;
    info->pythonType = type;
    Py_INCREF(type);

    int sz = PyTuple_GET_SIZE(enumerators);
    for(int i = 0; i < sz; ++i)
    {
        PyObjectHandle e = PyTuple_GET_ITEM(enumerators, i);
        Py_INCREF(e.get());
        assert(PyObject_IsInstance(e.get(), type));
        info->enumerators.push_back(e);
    }

    assert(_typeInfoMap.find(id) == _typeInfoMap.end());
    _typeInfoMap.insert(TypeInfoMap::value_type(id, info));

    Py_INCREF(Py_None);
    return Py_None;
}

extern "C"
PyObject*
Ice_addStruct(PyObject*, PyObject* args)
{
    char* id;
    PyObject* type;
    PyObject* members;
    if(!PyArg_ParseTuple(args, "sOO", &id, &type, &members))
    {
        return NULL;
    }

    assert(PyType_Check(type));
    assert(PyTuple_Check(members));

    StructInfoPtr info = new StructInfo;
    info->name = id;
    info->pythonType = type;
    Py_INCREF(type);

    int sz = PyTuple_GET_SIZE(members);
    for(int i = 0; i < sz; ++i)
    {
        PyObject* m = PyTuple_GET_ITEM(members, i);
        assert(PyTuple_Check(m));
        assert(PyTuple_GET_SIZE(m) == 2);
        PyObject* s = PyTuple_GET_ITEM(m, 0); // Member name.
        assert(PyString_Check(s));
        PyObject* t = PyTuple_GET_ITEM(m, 1); // Member type.
        info->members.push_back(DataMember());
        info->members.back().name = PyString_AS_STRING(s);
        info->members.back().type = convertType(t);
    }

    assert(_typeInfoMap.find(id) == _typeInfoMap.end());
    _typeInfoMap.insert(TypeInfoMap::value_type(id, info));

    Py_INCREF(Py_None);
    return Py_None;
}

extern "C"
PyObject*
Ice_addSequence(PyObject*, PyObject* args)
{
    char* id;
    PyObject* elementType;
    if(!PyArg_ParseTuple(args, "sO", &id, &elementType))
    {
        return NULL;
    }

    SequenceInfoPtr info = new SequenceInfo;
    info->name = id;
    info->elementType = convertType(elementType);

    assert(_typeInfoMap.find(id) == _typeInfoMap.end());
    _typeInfoMap.insert(TypeInfoMap::value_type(id, info));

    Py_INCREF(Py_None);
    return Py_None;
}

extern "C"
PyObject*
Ice_addDictionary(PyObject*, PyObject* args)
{
    char* id;
    PyObject* keyType;
    PyObject* valueType;
    if(!PyArg_ParseTuple(args, "sOO", &id, &keyType, &valueType))
    {
        return NULL;
    }

    DictionaryInfoPtr info = new DictionaryInfo;
    info->name = id;
    info->keyType = convertType(keyType);
    info->valueType = convertType(valueType);

    assert(_typeInfoMap.find(id) == _typeInfoMap.end());
    _typeInfoMap.insert(TypeInfoMap::value_type(id, info));

    Py_INCREF(Py_None);
    return Py_None;
}

extern "C"
PyObject*
Ice_addProxy(PyObject*, PyObject* args)
{
    char* id;
    if(!PyArg_ParseTuple(args, "s", &id))
    {
        return NULL;
    }

    string proxyId = id;
    proxyId += "Prx";

    TypeInfoMap::iterator p = _typeInfoMap.find(proxyId);
    if(p == _typeInfoMap.end())
    {
        ProxyInfoPtr info = new ProxyInfo;
        info->name = proxyId;
        info->_class = ClassInfoPtr::dynamicCast(getTypeInfo(id));
        assert(info->_class);
        _typeInfoMap.insert(TypeInfoMap::value_type(proxyId, info));
    }
    else
    {
        assert(ProxyInfoPtr::dynamicCast(p->second));
    }

    Py_INCREF(Py_None);
    return Py_None;
}

extern "C"
PyObject*
Ice_defineProxy(PyObject*, PyObject* args)
{
    char* id;
    PyObject* type;
    if(!PyArg_ParseTuple(args, "sO", &id, &type))
    {
        return NULL;
    }

    assert(PyType_Check(type));

    ProxyInfoPtr info;

    string proxyId = id;
    proxyId += "Prx";

    TypeInfoMap::iterator p = _typeInfoMap.find(proxyId);
    if(p != _typeInfoMap.end())
    {
        info = ProxyInfoPtr::dynamicCast(p->second);
        assert(info);
    }
    else
    {
        info = new ProxyInfo;
        info->name = proxyId;
        _typeInfoMap.insert(TypeInfoMap::value_type(proxyId, info));
    }

    info->pythonType = type;
    Py_INCREF(type);

    Py_INCREF(Py_None);
    return Py_None;
}

extern "C"
PyObject*
Ice_addClass(PyObject*, PyObject* args)
{
    char* id;
    if(!PyArg_ParseTuple(args, "s", &id))
    {
        return NULL;
    }

    TypeInfoMap::iterator p = _typeInfoMap.find(id);
    if(p == _typeInfoMap.end())
    {
        ClassInfoPtr info = new ClassInfo;
        info->name = id;
        _typeInfoMap.insert(TypeInfoMap::value_type(id, info));
    }
    else
    {
        assert(ClassInfoPtr::dynamicCast(p->second));
    }

    Py_INCREF(Py_None);
    return Py_None;
}

extern "C"
PyObject*
Ice_defineClass(PyObject*, PyObject* args)
{
    char* id;
    PyObject* type;
    int isInterface;
    char* baseId;
    PyObject* interfaces;
    PyObject* members;
    PyObject* operations;
    if(!PyArg_ParseTuple(args, "sOisOOO", &id, &type, &isInterface, &baseId, &interfaces, &members, &operations))
    {
        return NULL;
    }

    assert(PyType_Check(type));
    assert(PyTuple_Check(interfaces));
    assert(PyTuple_Check(members));
    assert(PyDict_Check(operations));

    ClassInfoPtr info;

    TypeInfoMap::iterator p = _typeInfoMap.find(id);
    if(p != _typeInfoMap.end())
    {
        info = ClassInfoPtr::dynamicCast(p->second);
        assert(info);
    }
    else
    {
        info = new ClassInfo;
        info->name = id;
        _typeInfoMap.insert(TypeInfoMap::value_type(id, info));
    }

    info->isInterface = isInterface ? true : false;
    if(strlen(baseId) > 0)
    {
        TypeInfoPtr t = getTypeInfo(baseId);
        assert(t);
        info->base = ClassInfoPtr::dynamicCast(t);
        assert(info->base);
    }
    info->pythonType = type;
    Py_INCREF(type);

    int i, sz;
    sz = PyTuple_GET_SIZE(interfaces);
    for(i = 0; i < sz; ++i)
    {
        PyObject* o = PyTuple_GET_ITEM(interfaces, i);
        assert(PyString_Check(o));
        info->interfaces.push_back(ClassInfoPtr::dynamicCast(convertType(o)));
    }

    sz = PyTuple_GET_SIZE(members);
    for(i = 0; i < sz; ++i)
    {
        PyObject* m = PyTuple_GET_ITEM(members, i);
        assert(PyTuple_Check(m));
        assert(PyTuple_GET_SIZE(m) == 2);
        PyObject* s = PyTuple_GET_ITEM(m, 0); // Member name.
        assert(PyString_Check(s));
        PyObject* t = PyTuple_GET_ITEM(m, 1); // Member type.
        info->members.push_back(DataMember());
        info->members.back().name = PyString_AS_STRING(s);
        info->members.back().type = convertType(t);
    }

    int pos = 0;
    PyObject* key;
    PyObject* value;
    while(PyDict_Next(operations, &pos, &key, &value))
    {
        assert(PyString_Check(key));
        assert(PyTuple_Check(value));
        assert(PyTuple_GET_SIZE(value) == 5);

        PyObject* item;

        OperationInfoPtr op = new OperationInfo;
        op->name = PyString_AS_STRING(key);

        //
        // mode
        //
        item = PyTuple_GET_ITEM(value, 0);
        assert(PyInt_Check(item));
        assert(PyInt_AS_LONG(item) >= OP_NORMAL && PyInt_AS_LONG(item) <= OP_IDEMPOTENT);
        op->mode = (Ice::OperationMode)static_cast<int>(PyInt_AS_LONG(item));

        //
        // inParams
        //
        item = PyTuple_GET_ITEM(value, 1);
        assert(PyTuple_Check(item));
        sz = PyTuple_GET_SIZE(item);
        for(i = 0; i < sz; ++i)
        {
            TypeInfoPtr t = convertType(PyTuple_GET_ITEM(item, i));
            assert(t);
            op->inParams.push_back(t);
        }

        //
        // outParams
        //
        item = PyTuple_GET_ITEM(value, 2);
        assert(PyTuple_Check(item));
        sz = PyTuple_GET_SIZE(item);
        for(i = 0; i < sz; ++i)
        {
            TypeInfoPtr t = convertType(PyTuple_GET_ITEM(item, i));
            assert(t);
            op->outParams.push_back(t);
        }

        //
        // returnType
        //
        item = PyTuple_GET_ITEM(value, 3);
        if(item != Py_None)
        {
            op->returnType = convertType(item);
        }

        //
        // exceptions
        //
        item = PyTuple_GET_ITEM(value, 4);
        assert(PyTuple_Check(item));
        sz = PyTuple_GET_SIZE(item);
        for(i = 0; i < sz; ++i)
        {
            PyObject* s = PyTuple_GET_ITEM(item, i);
            assert(PyString_Check(s));
            op->exceptions.push_back(getExceptionInfo(PyString_AS_STRING(s)));
        }

        info->operations.insert(OperationInfoMap::value_type(op->name, op));
    }

    Py_INCREF(Py_None);
    return Py_None;
}

extern "C"
PyObject*
Ice_addException(PyObject*, PyObject* args)
{
    char* id;
    PyObject* type;
    char* baseId;
    PyObject* members;
    int usesClasses;
    if(!PyArg_ParseTuple(args, "sOsOi", &id, &type, &baseId, &members, &usesClasses))
    {
        return NULL;
    }

    assert(PyClass_Check(type));
    assert(PyTuple_Check(members));

    ExceptionInfoPtr info = new ExceptionInfo;
    info->name = id;
    if(strlen(baseId) > 0)
    {
        info->base = getExceptionInfo(baseId);
        assert(info->base);
    }
    info->usesClasses = usesClasses ? true : false;
    info->pythonType = type;
    Py_INCREF(type);

    int sz = PyTuple_GET_SIZE(members);
    for(int i = 0; i < sz; ++i)
    {
        PyObject* m = PyTuple_GET_ITEM(members, i);
        assert(PyTuple_Check(m));
        assert(PyTuple_GET_SIZE(m) == 2);
        PyObject* s = PyTuple_GET_ITEM(m, 0); // Member name.
        assert(PyString_Check(s));
        PyObject* t = PyTuple_GET_ITEM(m, 1); // Member type.
        info->members.push_back(DataMember());
        info->members.back().name = PyString_AS_STRING(s);
        info->members.back().type = convertType(t);
    }

    assert(_exceptionInfoMap.find(id) == _exceptionInfoMap.end());
    _exceptionInfoMap.insert(ExceptionInfoMap::value_type(id, info));

    Py_INCREF(Py_None);
    return Py_None;
}
