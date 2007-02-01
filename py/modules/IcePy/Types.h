// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICEPY_TYPES_H
#define ICEPY_TYPES_H

#include <Config.h>
#include <Util.h>
#include <Ice/Stream.h>
#include <IceUtil/OutputUtil.h>

namespace IcePy
{

class ExceptionInfo;
typedef IceUtil::Handle<ExceptionInfo> ExceptionInfoPtr;
typedef std::vector<ExceptionInfoPtr> ExceptionInfoList;

class ClassInfo;
typedef IceUtil::Handle<ClassInfo> ClassInfoPtr;
typedef std::vector<ClassInfoPtr> ClassInfoList;

//
// This class is raised as an exception when object marshaling needs to be aborted.
//
class AbortMarshaling
{
};

typedef std::map<PyObject*, Ice::ObjectPtr> ObjectMap;

struct PrintObjectHistory
{
    int index;
    std::map<PyObject*, int> objects;
};

//
// The delayed nature of class unmarshaling in the Ice protocol requires us to
// handle unmarshaling using a callback strategy. An instance of UnmarshalCallback
// is supplied to each type's unmarshal() member function. For all types except
// classes, the callback is invoked with the unmarshaled value before unmarshal()
// returns. For class instances, however, the callback may not be invoked until
// the stream's finished() function is called.
//
class UnmarshalCallback : public IceUtil::Shared
{
public:

    virtual ~UnmarshalCallback();

    //
    // The unmarshaled() member function receives the unmarshaled value. The
    // last two arguments are the values passed to unmarshal() for use by
    // UnmarshalCallback implementations.
    //
    virtual void unmarshaled(PyObject*, PyObject*, void*) = 0;
};
typedef IceUtil::Handle<UnmarshalCallback> UnmarshalCallbackPtr;

//
// Base class for type information.
//
class TypeInfo : public UnmarshalCallback
{
public:

    virtual std::string getId() const = 0;

    virtual bool validate(PyObject*) = 0;

    virtual bool usesClasses(); // Default implementation returns false.

    virtual void unmarshaled(PyObject*, PyObject*, void*); // Default implementation is assert(false).

    virtual void destroy();

protected:

    TypeInfo();

public:

    //
    // The marshal and unmarshal functions can raise Ice exceptions, and may raise
    // AbortMarshaling if an error occurs.
    //
    virtual void marshal(PyObject*, const Ice::OutputStreamPtr&, ObjectMap*, const Ice::StringSeq* = 0) = 0;
    virtual void unmarshal(const Ice::InputStreamPtr&, const UnmarshalCallbackPtr&, PyObject*, void*,
                           const Ice::StringSeq* = 0) = 0;

    virtual void print(PyObject*, IceUtil::Output&, PrintObjectHistory*) = 0;
};
typedef IceUtil::Handle<TypeInfo> TypeInfoPtr;

//
// Primitive type information.
//
class PrimitiveInfo : public TypeInfo
{
public:

    virtual std::string getId() const;

    virtual bool validate(PyObject*);

    virtual void marshal(PyObject*, const Ice::OutputStreamPtr&, ObjectMap*, const Ice::StringSeq* = 0);
    virtual void unmarshal(const Ice::InputStreamPtr&, const UnmarshalCallbackPtr&, PyObject*, void*,
                           const Ice::StringSeq* = 0);

    virtual void print(PyObject*, IceUtil::Output&, PrintObjectHistory*);

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

    virtual std::string getId() const;

    virtual bool validate(PyObject*);

    virtual void marshal(PyObject*, const Ice::OutputStreamPtr&, ObjectMap*, const Ice::StringSeq* = 0);
    virtual void unmarshal(const Ice::InputStreamPtr&, const UnmarshalCallbackPtr&, PyObject*, void*,
                           const Ice::StringSeq* = 0);

    virtual void print(PyObject*, IceUtil::Output&, PrintObjectHistory*);

    std::string id;
    EnumeratorList enumerators;
    PyObjectHandle pythonType;
};
typedef IceUtil::Handle<EnumInfo> EnumInfoPtr;

class DataMember : public UnmarshalCallback
{
public:

    virtual void unmarshaled(PyObject*, PyObject*, void*);

    std::string name;
    std::vector<std::string> metaData;
    TypeInfoPtr type;
};
typedef IceUtil::Handle<DataMember> DataMemberPtr;
typedef std::vector<DataMemberPtr> DataMemberList;

//
// Struct information.
//
class StructInfo : public TypeInfo
{
public:

    virtual std::string getId() const;

    virtual bool validate(PyObject*);

    virtual bool usesClasses();

    virtual void marshal(PyObject*, const Ice::OutputStreamPtr&, ObjectMap*, const Ice::StringSeq* = 0);
    virtual void unmarshal(const Ice::InputStreamPtr&, const UnmarshalCallbackPtr&, PyObject*, void*,
                           const Ice::StringSeq* = 0);

    virtual void print(PyObject*, IceUtil::Output&, PrintObjectHistory*);

    virtual void destroy();

    std::string id;
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

    virtual std::string getId() const;

    virtual bool validate(PyObject*);

    virtual bool usesClasses();

    virtual void marshal(PyObject*, const Ice::OutputStreamPtr&, ObjectMap*, const Ice::StringSeq* = 0);
    virtual void unmarshal(const Ice::InputStreamPtr&, const UnmarshalCallbackPtr&, PyObject*, void*,
                           const Ice::StringSeq* = 0);

    virtual void print(PyObject*, IceUtil::Output&, PrintObjectHistory*);

    virtual void destroy();

    struct SequenceMapping : public UnmarshalCallback
    {
        enum Type { SEQ_DEFAULT, SEQ_TUPLE, SEQ_LIST };

        SequenceMapping(Type);
        SequenceMapping(const Ice::StringSeq&);

        static bool getType(const Ice::StringSeq&, Type&);

        virtual void unmarshaled(PyObject*, PyObject*, void*);

        PyObject* createContainer(int) const;
        void setItem(PyObject*, int, PyObject*) const;

        Type type;
    };
    typedef IceUtil::Handle<SequenceMapping> SequenceMappingPtr;

    std::string id;
    SequenceMappingPtr mapping;
    TypeInfoPtr elementType;

private:

    void marshalPrimitiveSequence(const PrimitiveInfoPtr&, PyObject*, const Ice::OutputStreamPtr&);
    void unmarshalPrimitiveSequence(const PrimitiveInfoPtr&, const Ice::InputStreamPtr&, const UnmarshalCallbackPtr&,
                                    PyObject*, void*, const SequenceMappingPtr&);
};
typedef IceUtil::Handle<SequenceInfo> SequenceInfoPtr;

//
// Dictionary information.
//
class DictionaryInfo : public TypeInfo
{
public:

    virtual std::string getId() const;

    virtual bool validate(PyObject*);

    virtual bool usesClasses();

    virtual void marshal(PyObject*, const Ice::OutputStreamPtr&, ObjectMap*, const Ice::StringSeq* = 0);
    virtual void unmarshal(const Ice::InputStreamPtr&, const UnmarshalCallbackPtr&, PyObject*, void*,
                           const Ice::StringSeq* = 0);
    virtual void unmarshaled(PyObject*, PyObject*, void*);

    virtual void print(PyObject*, IceUtil::Output&, PrintObjectHistory*);

    virtual void destroy();

    class KeyCallback : public UnmarshalCallback
    {
    public:

        virtual void unmarshaled(PyObject*, PyObject*, void*);

        PyObjectHandle key;
    };
    typedef IceUtil::Handle<KeyCallback> KeyCallbackPtr;

    std::string id;
    TypeInfoPtr keyType;
    TypeInfoPtr valueType;
};
typedef IceUtil::Handle<DictionaryInfo> DictionaryInfoPtr;

typedef std::vector<TypeInfoPtr> TypeInfoList;

class ClassInfo : public TypeInfo
{
public:

    virtual std::string getId() const;

    virtual bool validate(PyObject*);

    virtual bool usesClasses();

    virtual void marshal(PyObject*, const Ice::OutputStreamPtr&, ObjectMap*, const Ice::StringSeq* = 0);
    virtual void unmarshal(const Ice::InputStreamPtr&, const UnmarshalCallbackPtr&, PyObject*, void*,
                           const Ice::StringSeq* = 0);

    virtual void print(PyObject*, IceUtil::Output&, PrintObjectHistory*);

    virtual void destroy();

    void printMembers(PyObject*, IceUtil::Output&, PrintObjectHistory*);

    std::string id;
    bool isAbstract;
    ClassInfoPtr base;
    ClassInfoList interfaces;
    DataMemberList members;
    PyObjectHandle pythonType;
    PyObjectHandle typeObj;
    bool defined;
};

//
// Proxy information.
//
class ProxyInfo : public TypeInfo
{
public:

    virtual std::string getId() const;

    virtual bool validate(PyObject*);

    virtual void marshal(PyObject*, const Ice::OutputStreamPtr&, ObjectMap*, const Ice::StringSeq* = 0);
    virtual void unmarshal(const Ice::InputStreamPtr&, const UnmarshalCallbackPtr&, PyObject*, void*,
                           const Ice::StringSeq* = 0);

    virtual void print(PyObject*, IceUtil::Output&, PrintObjectHistory*);

    virtual void destroy();

    std::string id;
    PyObjectHandle pythonType;
    PyObjectHandle typeObj;
};
typedef IceUtil::Handle<ProxyInfo> ProxyInfoPtr;

//
// Exception information.
//
class ExceptionInfo : public IceUtil::Shared
{
public:

    void marshal(PyObject*, const Ice::OutputStreamPtr&, ObjectMap*);
    PyObject* unmarshal(const Ice::InputStreamPtr&);

    void print(PyObject*, IceUtil::Output&);
    void printMembers(PyObject*, IceUtil::Output&, PrintObjectHistory*);

    std::string id;
    ExceptionInfoPtr base;
    DataMemberList members;
    bool usesClasses;
    PyObjectHandle pythonType;
};

//
// ObjectWriter wraps a Python object for marshaling.
//
class ObjectWriter : public Ice::ObjectWriter
{
public:

    ObjectWriter(const ClassInfoPtr&, PyObject*, ObjectMap*);
    ~ObjectWriter();

    virtual void ice_preMarshal();

    virtual void write(const Ice::OutputStreamPtr&) const;

private:

    ClassInfoPtr _info;
    PyObject* _object;
    ObjectMap* _map;
};

//
// ObjectReader unmarshals the state of an Ice object.
//
class ObjectReader : public Ice::ObjectReader
{
public:

    ObjectReader(PyObject*, const ClassInfoPtr&);
    ~ObjectReader();

    virtual void ice_postUnmarshal();

    virtual void read(const Ice::InputStreamPtr&, bool);

    virtual ClassInfoPtr getInfo() const;

    PyObject* getObject() const; // Borrowed reference.

private:

    PyObject* _object;
    ClassInfoPtr _info;
};
typedef IceUtil::Handle<ObjectReader> ObjectReaderPtr;

ClassInfoPtr lookupClassInfo(const std::string&);
ExceptionInfoPtr lookupExceptionInfo(const std::string&);

bool initTypes(PyObject*);

PyObject* createType(const TypeInfoPtr&);
TypeInfoPtr getType(PyObject*);

PyObject* createException(const ExceptionInfoPtr&);
ExceptionInfoPtr getException(PyObject*);

}

extern "C" PyObject* IcePy_defineEnum(PyObject*, PyObject*);
extern "C" PyObject* IcePy_defineStruct(PyObject*, PyObject*);
extern "C" PyObject* IcePy_defineSequence(PyObject*, PyObject*);
extern "C" PyObject* IcePy_defineDictionary(PyObject*, PyObject*);
extern "C" PyObject* IcePy_declareProxy(PyObject*, PyObject*);
extern "C" PyObject* IcePy_defineProxy(PyObject*, PyObject*);
extern "C" PyObject* IcePy_declareClass(PyObject*, PyObject*);
extern "C" PyObject* IcePy_defineClass(PyObject*, PyObject*);
extern "C" PyObject* IcePy_defineException(PyObject*, PyObject*);
extern "C" PyObject* IcePy_stringify(PyObject*, PyObject*);
extern "C" PyObject* IcePy_stringifyException(PyObject*, PyObject*);

#endif
