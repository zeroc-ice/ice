// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
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

#include <set>

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

class ObjectReader;
typedef IceUtil::Handle<ObjectReader> ObjectReaderPtr;

//
// This class keeps track of Python objects (instances of Slice classes
// and exceptions) that have preserved slices.
//
class SlicedDataUtil
{
public:

    SlicedDataUtil();
    ~SlicedDataUtil();

    void add(const ObjectReaderPtr&);

    void update();

    static void setMember(PyObject*, const Ice::SlicedDataPtr&);
    static Ice::SlicedDataPtr getMember(PyObject*, ObjectMap*);

private:

    std::set<ObjectReaderPtr> _readers;
    static PyObject* _slicedDataType;
    static PyObject* _sliceInfoType;
};

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

    virtual bool variableLength() const = 0;
    virtual int wireSize() const = 0;
    virtual Ice::OptionalFormat optionalFormat() const = 0;

    virtual bool usesClasses() const; // Default implementation returns false.

    virtual void unmarshaled(PyObject*, PyObject*, void*); // Default implementation is assert(false).

    virtual void destroy();

protected:

    TypeInfo();

public:

    //
    // The marshal and unmarshal functions can raise Ice exceptions, and may raise
    // AbortMarshaling if an error occurs.
    //
    virtual void marshal(PyObject*, const Ice::OutputStreamPtr&, ObjectMap*, bool, const Ice::StringSeq* = 0) = 0;
    virtual void unmarshal(const Ice::InputStreamPtr&, const UnmarshalCallbackPtr&, PyObject*, void*, bool,
                           const Ice::StringSeq* = 0) = 0;

    virtual void print(PyObject*, IceUtilInternal::Output&, PrintObjectHistory*) = 0;
};
typedef IceUtil::Handle<TypeInfo> TypeInfoPtr;

//
// Primitive type information.
//
class PrimitiveInfo : public TypeInfo
{
public:

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

    PrimitiveInfo(Kind);

    virtual std::string getId() const;

    virtual bool validate(PyObject*);

    virtual bool variableLength() const;
    virtual int wireSize() const;
    virtual Ice::OptionalFormat optionalFormat() const;

    virtual void marshal(PyObject*, const Ice::OutputStreamPtr&, ObjectMap*, bool, const Ice::StringSeq* = 0);
    virtual void unmarshal(const Ice::InputStreamPtr&, const UnmarshalCallbackPtr&, PyObject*, void*, bool,
                           const Ice::StringSeq* = 0);

    virtual void print(PyObject*, IceUtilInternal::Output&, PrintObjectHistory*);

    const Kind kind;
};
typedef IceUtil::Handle<PrimitiveInfo> PrimitiveInfoPtr;

//
// Enum information.
//
typedef std::map<Ice::Int, PyObjectHandle> EnumeratorMap;

class EnumInfo : public TypeInfo
{
public:

    EnumInfo(const std::string&, PyObject*, PyObject*);

    virtual std::string getId() const;

    virtual bool validate(PyObject*);

    virtual bool variableLength() const;
    virtual int wireSize() const;
    virtual Ice::OptionalFormat optionalFormat() const;

    virtual void marshal(PyObject*, const Ice::OutputStreamPtr&, ObjectMap*, bool, const Ice::StringSeq* = 0);
    virtual void unmarshal(const Ice::InputStreamPtr&, const UnmarshalCallbackPtr&, PyObject*, void*, bool,
                           const Ice::StringSeq* = 0);

    virtual void print(PyObject*, IceUtilInternal::Output&, PrintObjectHistory*);

    Ice::Int valueForEnumerator(PyObject*) const;
    PyObject* enumeratorForValue(Ice::Int) const;

    const std::string id;
    const PyObjectHandle pythonType;
    const Ice::Int maxValue;
    const EnumeratorMap enumerators;
};
typedef IceUtil::Handle<EnumInfo> EnumInfoPtr;

class DataMember : public UnmarshalCallback
{
public:

    virtual void unmarshaled(PyObject*, PyObject*, void*);

    std::string name;
    std::vector<std::string> metaData;
    TypeInfoPtr type;
    bool optional;
    int tag;
};
typedef IceUtil::Handle<DataMember> DataMemberPtr;
typedef std::vector<DataMemberPtr> DataMemberList;

//
// Struct information.
//
class StructInfo : public TypeInfo
{
public:

    StructInfo(const std::string&, PyObject*, PyObject*);

    virtual std::string getId() const;

    virtual bool validate(PyObject*);

    virtual bool variableLength() const;
    virtual int wireSize() const;
    virtual Ice::OptionalFormat optionalFormat() const;

    virtual bool usesClasses() const;

    virtual void marshal(PyObject*, const Ice::OutputStreamPtr&, ObjectMap*, bool, const Ice::StringSeq* = 0);
    virtual void unmarshal(const Ice::InputStreamPtr&, const UnmarshalCallbackPtr&, PyObject*, void*, bool,
                           const Ice::StringSeq* = 0);

    virtual void print(PyObject*, IceUtilInternal::Output&, PrintObjectHistory*);

    virtual void destroy();

    static PyObject* instantiate(PyObject*);

    const std::string id;
    const DataMemberList members;
    const PyObjectHandle pythonType;

private:

    bool _variableLength;
    int _wireSize;
    PyObjectHandle _nullMarshalValue;
};
typedef IceUtil::Handle<StructInfo> StructInfoPtr;

//
// Sequence information.
//
class SequenceInfo : public TypeInfo
{
public:

    SequenceInfo(const std::string&, PyObject*, PyObject*);

    virtual std::string getId() const;

    virtual bool validate(PyObject*);

    virtual bool variableLength() const;
    virtual int wireSize() const;
    virtual Ice::OptionalFormat optionalFormat() const;

    virtual bool usesClasses() const;

    virtual void marshal(PyObject*, const Ice::OutputStreamPtr&, ObjectMap*, bool, const Ice::StringSeq* = 0);
    virtual void unmarshal(const Ice::InputStreamPtr&, const UnmarshalCallbackPtr&, PyObject*, void*, bool,
                           const Ice::StringSeq* = 0);

    virtual void print(PyObject*, IceUtilInternal::Output&, PrintObjectHistory*);

    virtual void destroy();

private:

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

    PyObject* getSequence(const PrimitiveInfoPtr&, PyObject*);
    void marshalPrimitiveSequence(const PrimitiveInfoPtr&, PyObject*, const Ice::OutputStreamPtr&);
    void unmarshalPrimitiveSequence(const PrimitiveInfoPtr&, const Ice::InputStreamPtr&, const UnmarshalCallbackPtr&,
                                    PyObject*, void*, const SequenceMappingPtr&);

public:

    const std::string id;
    const SequenceMappingPtr mapping;
    const TypeInfoPtr elementType;
};
typedef IceUtil::Handle<SequenceInfo> SequenceInfoPtr;

//
// Custom information.
//
class CustomInfo : public TypeInfo
{
public:

    CustomInfo(const std::string&, PyObject*);

    virtual std::string getId() const;

    virtual bool validate(PyObject*);

    virtual bool variableLength() const;
    virtual int wireSize() const;
    virtual Ice::OptionalFormat optionalFormat() const;

    virtual bool usesClasses() const;

    virtual void marshal(PyObject*, const Ice::OutputStreamPtr&, ObjectMap*, bool, const Ice::StringSeq* = 0);
    virtual void unmarshal(const Ice::InputStreamPtr&, const UnmarshalCallbackPtr&, PyObject*, void*, bool,
                           const Ice::StringSeq* = 0);

    virtual void print(PyObject*, IceUtilInternal::Output&, PrintObjectHistory*);

    virtual void destroy();

    const std::string id;
    const PyObjectHandle pythonType;
};
typedef IceUtil::Handle<CustomInfo> CustomInfoPtr;

//
// Dictionary information.
//
class DictionaryInfo : public TypeInfo
{
public:

    DictionaryInfo(const std::string&, PyObject*, PyObject*);

    virtual std::string getId() const;

    virtual bool validate(PyObject*);

    virtual bool variableLength() const;
    virtual int wireSize() const;
    virtual Ice::OptionalFormat optionalFormat() const;

    virtual bool usesClasses() const;

    virtual void marshal(PyObject*, const Ice::OutputStreamPtr&, ObjectMap*, bool, const Ice::StringSeq* = 0);
    virtual void unmarshal(const Ice::InputStreamPtr&, const UnmarshalCallbackPtr&, PyObject*, void*, bool,
                           const Ice::StringSeq* = 0);
    virtual void unmarshaled(PyObject*, PyObject*, void*);

    virtual void print(PyObject*, IceUtilInternal::Output&, PrintObjectHistory*);

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

private:

    bool _variableLength;
    int _wireSize;
};
typedef IceUtil::Handle<DictionaryInfo> DictionaryInfoPtr;

typedef std::vector<TypeInfoPtr> TypeInfoList;

class ClassInfo : public TypeInfo
{
public:

    ClassInfo(const std::string&);

    void define(PyObject*, int, bool, bool, PyObject*, PyObject*, PyObject*);

    virtual std::string getId() const;

    virtual bool validate(PyObject*);

    virtual bool variableLength() const;
    virtual int wireSize() const;
    virtual Ice::OptionalFormat optionalFormat() const;

    virtual bool usesClasses() const;

    virtual void marshal(PyObject*, const Ice::OutputStreamPtr&, ObjectMap*, bool, const Ice::StringSeq* = 0);
    virtual void unmarshal(const Ice::InputStreamPtr&, const UnmarshalCallbackPtr&, PyObject*, void*, bool,
                           const Ice::StringSeq* = 0);

    virtual void print(PyObject*, IceUtilInternal::Output&, PrintObjectHistory*);

    virtual void destroy();

    void printMembers(PyObject*, IceUtilInternal::Output&, PrintObjectHistory*);

    const std::string id;
    const Ice::Int compactId;
    const bool isAbstract;
    const bool preserve;
    const ClassInfoPtr base;
    const ClassInfoList interfaces;
    const DataMemberList members;
    const DataMemberList optionalMembers;
    const PyObjectHandle pythonType;
    const PyObjectHandle typeObj;
    const bool defined;
};

//
// Proxy information.
//
class ProxyInfo : public TypeInfo
{
public:

    ProxyInfo(const std::string&);

    void define(PyObject*);

    virtual std::string getId() const;

    virtual bool validate(PyObject*);

    virtual bool variableLength() const;
    virtual int wireSize() const;
    virtual Ice::OptionalFormat optionalFormat() const;

    virtual void marshal(PyObject*, const Ice::OutputStreamPtr&, ObjectMap*, bool, const Ice::StringSeq* = 0);
    virtual void unmarshal(const Ice::InputStreamPtr&, const UnmarshalCallbackPtr&, PyObject*, void*, bool,
                           const Ice::StringSeq* = 0);

    virtual void print(PyObject*, IceUtilInternal::Output&, PrintObjectHistory*);

    virtual void destroy();

    const std::string id;
    const PyObjectHandle pythonType;
    const PyObjectHandle typeObj;
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

    void print(PyObject*, IceUtilInternal::Output&);
    void printMembers(PyObject*, IceUtilInternal::Output&, PrintObjectHistory*);

    std::string id;
    bool preserve;
    ExceptionInfoPtr base;
    DataMemberList members;
    DataMemberList optionalMembers;
    bool usesClasses;
    PyObjectHandle pythonType;

private:

    void writeMembers(PyObject*, const Ice::OutputStreamPtr&, const DataMemberList&, ObjectMap*) const;
};

//
// ObjectWriter wraps a Python object for marshaling.
//
class ObjectWriter : public Ice::ObjectWriter
{
public:

    ObjectWriter(PyObject*, ObjectMap*);
    ~ObjectWriter();

    virtual void ice_preMarshal();

    virtual void write(const Ice::OutputStreamPtr&) const;

private:

    void writeMembers(const Ice::OutputStreamPtr&, const DataMemberList&) const;

    PyObject* _object;
    ObjectMap* _map;
    ClassInfoPtr _info;
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

    virtual void read(const Ice::InputStreamPtr&);

    virtual ClassInfoPtr getInfo() const;

    PyObject* getObject() const; // Borrowed reference.

    Ice::SlicedDataPtr getSlicedData() const;

private:

    PyObject* _object;
    ClassInfoPtr _info;
    Ice::SlicedDataPtr _slicedData;
};

//
// ExceptionWriter wraps a Python user exception for marshaling.
//
class ExceptionWriter : public Ice::UserExceptionWriter
{
public:

    ExceptionWriter(const Ice::CommunicatorPtr&, const PyObjectHandle&, const ExceptionInfoPtr& = 0);
    ~ExceptionWriter() throw();

    virtual void write(const Ice::OutputStreamPtr&) const;
    virtual bool usesClasses() const;

    virtual std::string ice_name() const;
    virtual Ice::UserException* ice_clone() const;
    virtual void ice_throw() const;

private:

    PyObjectHandle _ex;
    ExceptionInfoPtr _info;
    ObjectMap _objects;
};

//
// ExceptionReader creates a Python user exception and unmarshals it.
//
class ExceptionReader : public Ice::UserExceptionReader
{
public:

    ExceptionReader(const Ice::CommunicatorPtr&, const ExceptionInfoPtr&);
    ~ExceptionReader() throw();

    virtual void read(const Ice::InputStreamPtr&) const;
    virtual bool usesClasses() const;

    virtual std::string ice_name() const;
    virtual Ice::UserException* ice_clone() const;
    virtual void ice_throw() const;

    PyObject* getException() const; // Borrowed reference.

    Ice::SlicedDataPtr getSlicedData() const;

private:

    ExceptionInfoPtr _info;
    PyObjectHandle _ex;
    Ice::SlicedDataPtr _slicedData;
};

class IdResolver : public Ice::CompactIdResolver
{
public:

    virtual ::std::string resolve(Ice::Int) const;
};

ClassInfoPtr lookupClassInfo(const std::string&);
ExceptionInfoPtr lookupExceptionInfo(const std::string&);

extern PyObject* Unset;

bool initTypes(PyObject*);

PyObject* createType(const TypeInfoPtr&);
TypeInfoPtr getType(PyObject*);

PyObject* createException(const ExceptionInfoPtr&);
ExceptionInfoPtr getException(PyObject*);

}

extern "C" PyObject* IcePy_defineEnum(PyObject*, PyObject*);
extern "C" PyObject* IcePy_defineStruct(PyObject*, PyObject*);
extern "C" PyObject* IcePy_defineSequence(PyObject*, PyObject*);
extern "C" PyObject* IcePy_defineCustom(PyObject*, PyObject*);
extern "C" PyObject* IcePy_defineDictionary(PyObject*, PyObject*);
extern "C" PyObject* IcePy_declareProxy(PyObject*, PyObject*);
extern "C" PyObject* IcePy_defineProxy(PyObject*, PyObject*);
extern "C" PyObject* IcePy_declareClass(PyObject*, PyObject*);
extern "C" PyObject* IcePy_defineClass(PyObject*, PyObject*);
extern "C" PyObject* IcePy_defineException(PyObject*, PyObject*);
extern "C" PyObject* IcePy_stringify(PyObject*, PyObject*);
extern "C" PyObject* IcePy_stringifyException(PyObject*, PyObject*);

#endif
