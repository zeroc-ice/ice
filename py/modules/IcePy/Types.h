// **********************************************************************
//
// Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICEPY_TYPES_H
#define ICEPY_TYPES_H

#include <Util.h>
#include <IceUtil/Shared.h>
#include <IceUtil/Handle.h>

namespace IcePy
{

const int TYPE_BOOL         = 0;
const int TYPE_BYTE         = 1;
const int TYPE_SHORT        = 2;
const int TYPE_INT          = 3;
const int TYPE_LONG         = 4;
const int TYPE_FLOAT        = 5;
const int TYPE_DOUBLE       = 6;
const int TYPE_STRING       = 7;
const int TYPE_OBJECT       = 8;
const int TYPE_OBJECT_PROXY = 9;

class ExceptionInfo;
typedef IceUtil::Handle<ExceptionInfo> ExceptionInfoPtr;
typedef std::vector<ExceptionInfoPtr> ExceptionInfoList;

class ClassInfo;
typedef IceUtil::Handle<ClassInfo> ClassInfoPtr;
typedef std::vector<ClassInfoPtr> ClassInfoList;

//
// Base class for type information.
//
class TypeInfo : public IceUtil::Shared
{
public:

    virtual std::string getName() const = 0;
    virtual void destroy();

protected:

    TypeInfo();
};
typedef IceUtil::Handle<TypeInfo> TypeInfoPtr;

//
// Primitive type information.
//
class PrimitiveInfo : public TypeInfo
{
public:

    virtual std::string getName() const;

    enum Kind
    {
        KindBool,
        KindByte,
        KindShort,
        KindInt,
        KindLong,
        KindFloat,
        KindDouble,
        KindString
    };

    Kind kind;
};
typedef IceUtil::Handle<PrimitiveInfo> PrimitiveInfoPtr;

//
// Enum information.
//
typedef std::vector<PyObjectHandle> EnumeratorList;

class EnumInfo : public TypeInfo
{
public:

    virtual std::string getName() const;

    std::string name;
    EnumeratorList enumerators;
    PyObjectHandle pythonType;
};
typedef IceUtil::Handle<EnumInfo> EnumInfoPtr;

struct DataMember
{
    std::string name;
    TypeInfoPtr type;
};
typedef std::vector<DataMember> DataMemberList;

//
// Struct information.
//
class StructInfo : public TypeInfo
{
public:

    virtual std::string getName() const;
    virtual void destroy();

    std::string name;
    DataMemberList members;
    PyObjectHandle pythonType;
};
typedef IceUtil::Handle<StructInfo> StructInfoPtr;

//
// Sequence information.
//
class SequenceInfo : public TypeInfo
{
public:

    virtual std::string getName() const;
    virtual void destroy();

    std::string name;
    TypeInfoPtr elementType;
};
typedef IceUtil::Handle<SequenceInfo> SequenceInfoPtr;

//
// Dictionary information.
//
class DictionaryInfo : public TypeInfo
{
public:

    virtual std::string getName() const;
    virtual void destroy();

    std::string name;
    TypeInfoPtr keyType;
    TypeInfoPtr valueType;
};
typedef IceUtil::Handle<DictionaryInfo> DictionaryInfoPtr;

typedef std::vector<TypeInfoPtr> TypeInfoList;

//
// Operation information.
//
const int OP_NORMAL      = 0;
const int OP_NONMUTATING = 1;
const int OP_IDEMPOTENT  = 2;

class OperationInfo : public IceUtil::Shared
{
public:

    //
    // Verify that the given Python exception is legal to be thrown from this operation.
    //
    bool validateException(PyObject*) const;

    std::string name;
    int mode;
    TypeInfoList inParams;
    TypeInfoList outParams;
    TypeInfoPtr returnType;
    ExceptionInfoList exceptions;
};
typedef IceUtil::Handle<OperationInfo> OperationInfoPtr;

//
// Class information.
//
typedef std::map<std::string, OperationInfoPtr> OperationInfoMap;

class ClassInfo : public TypeInfo
{
public:

    virtual std::string getName() const;
    virtual void destroy();

    OperationInfoPtr findOperation(const std::string&) const;
    bool hasOperations() const;

    std::string name;
    bool isInterface;
    ClassInfoPtr base;
    ClassInfoList interfaces;
    DataMemberList members;
    OperationInfoMap operations;
    PyObjectHandle pythonType;
};

//
// Proxy information.
//
class ProxyInfo : public TypeInfo
{
public:

    virtual std::string getName() const;

    std::string name;
    ClassInfoPtr _class;
    PyObjectHandle pythonType;
};
typedef IceUtil::Handle<ProxyInfo> ProxyInfoPtr;

//
// Exception information.
//
class ExceptionInfo : public IceUtil::Shared
{
public:

    std::string name;
    ExceptionInfoPtr base;
    DataMemberList members;
    bool usesClasses;
    PyObjectHandle pythonType;
};

bool initTypes(PyObject*);

TypeInfoPtr getTypeInfo(const std::string&);
ExceptionInfoPtr getExceptionInfo(const std::string&);

}

extern "C" PyObject* Ice_addEnum(PyObject*, PyObject*);
extern "C" PyObject* Ice_addStruct(PyObject*, PyObject*);
extern "C" PyObject* Ice_addSequence(PyObject*, PyObject*);
extern "C" PyObject* Ice_addDictionary(PyObject*, PyObject*);
extern "C" PyObject* Ice_addProxy(PyObject*, PyObject*);
extern "C" PyObject* Ice_defineProxy(PyObject*, PyObject*);
extern "C" PyObject* Ice_addClass(PyObject*, PyObject*);
extern "C" PyObject* Ice_defineClass(PyObject*, PyObject*);
extern "C" PyObject* Ice_addException(PyObject*, PyObject*);

#endif
