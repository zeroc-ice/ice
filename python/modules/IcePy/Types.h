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
#include <Ice/FactoryTable.h>
#include <Ice/Object.h>
#include <Ice/SlicedDataF.h>
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


class ValueInfo;
typedef IceUtil::Handle<ValueInfo> ValueInfoPtr;

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
// ReadObjectCallback retains all of the information necessary to store an unmarshaled
// Slice value as a Python object.
//
class ReadObjectCallback : public IceUtil::Shared
{
public:

    ReadObjectCallback(const ValueInfoPtr&, const UnmarshalCallbackPtr&, PyObject*, void*);
    ~ReadObjectCallback();

    void invoke(const ::Ice::ObjectPtr&);

private:

    ValueInfoPtr _info;
    UnmarshalCallbackPtr _cb;
    PyObject* _target;
    void* _closure;
};
typedef IceUtil::Handle<ReadObjectCallback> ReadObjectCallbackPtr;

//
// This class assists during unmarshaling of Slice classes and exceptions.
// We attach an instance to a stream.
//
class StreamUtil
{
public:

    StreamUtil();
    ~StreamUtil();

    //
    // Keep a reference to a ReadObjectCallback for patching purposes.
    //
    void add(const ReadObjectCallbackPtr&);

    //
    // Keep track of object instances that have preserved slices.
    //
    void add(const ObjectReaderPtr&);

    //
    // Updated the sliced data information for all stored object instances.
    //
    void updateSlicedData();

    static void setSlicedDataMember(PyObject*, const Ice::SlicedDataPtr&);
    static Ice::SlicedDataPtr getSlicedDataMember(PyObject*, ObjectMap*);

private:

    std::vector<ReadObjectCallbackPtr> _callbacks;
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
    virtual void marshal(PyObject*, Ice::OutputStream*, ObjectMap*, bool, const Ice::StringSeq* = 0) = 0;
    virtual void unmarshal(Ice::InputStream*, const UnmarshalCallbackPtr&, PyObject*, void*, bool,
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

    virtual void marshal(PyObject*, Ice::OutputStream*, ObjectMap*, bool, const Ice::StringSeq* = 0);
    virtual void unmarshal(Ice::InputStream*, const UnmarshalCallbackPtr&, PyObject*, void*, bool,
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

    virtual void marshal(PyObject*, Ice::OutputStream*, ObjectMap*, bool, const Ice::StringSeq* = 0);
    virtual void unmarshal(Ice::InputStream*, const UnmarshalCallbackPtr&, PyObject*, void*, bool,
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

    virtual void marshal(PyObject*, Ice::OutputStream*, ObjectMap*, bool, const Ice::StringSeq* = 0);
    virtual void unmarshal(Ice::InputStream*, const UnmarshalCallbackPtr&, PyObject*, void*, bool,
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

    virtual void marshal(PyObject*, Ice::OutputStream*, ObjectMap*, bool, const Ice::StringSeq* = 0);
    virtual void unmarshal(Ice::InputStream*, const UnmarshalCallbackPtr&, PyObject*, void*, bool,
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
    void marshalPrimitiveSequence(const PrimitiveInfoPtr&, PyObject*, Ice::OutputStream*);
    void unmarshalPrimitiveSequence(const PrimitiveInfoPtr&, Ice::InputStream*, const UnmarshalCallbackPtr&,
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

    virtual void marshal(PyObject*, Ice::OutputStream*, ObjectMap*, bool, const Ice::StringSeq* = 0);
    virtual void unmarshal(Ice::InputStream*, const UnmarshalCallbackPtr&, PyObject*, void*, bool,
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

    virtual void marshal(PyObject*, Ice::OutputStream*, ObjectMap*, bool, const Ice::StringSeq* = 0);
    virtual void unmarshal(Ice::InputStream*, const UnmarshalCallbackPtr&, PyObject*, void*, bool,
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

    void define(PyObject*, PyObject*, PyObject*);

    virtual std::string getId() const;

    virtual bool validate(PyObject*);

    virtual bool variableLength() const;
    virtual int wireSize() const;
    virtual Ice::OptionalFormat optionalFormat() const;

    virtual bool usesClasses() const;

    virtual void marshal(PyObject*, Ice::OutputStream*, ObjectMap*, bool, const Ice::StringSeq* = 0);
    virtual void unmarshal(Ice::InputStream*, const UnmarshalCallbackPtr&, PyObject*, void*, bool,
                           const Ice::StringSeq* = 0);

    virtual void print(PyObject*, IceUtilInternal::Output&, PrintObjectHistory*);

    virtual void destroy();

    const std::string id;
    const ClassInfoPtr base;
    const ClassInfoList interfaces;
    const PyObjectHandle pythonType;
    const PyObjectHandle typeObj;
    const bool defined;
};

//
// Value type information
//

class ValueInfo : public TypeInfo
{
public:

    ValueInfo(const std::string&);

    void define(PyObject*, int, bool, bool, PyObject*, PyObject*);

    virtual std::string getId() const;

    virtual bool validate(PyObject*);

    virtual bool variableLength() const;
    virtual int wireSize() const;
    virtual Ice::OptionalFormat optionalFormat() const;

    virtual bool usesClasses() const;

    virtual void marshal(PyObject*, Ice::OutputStream*, ObjectMap*, bool, const Ice::StringSeq* = 0);
    virtual void unmarshal(Ice::InputStream*, const UnmarshalCallbackPtr&, PyObject*, void*, bool,
                           const Ice::StringSeq* = 0);

    virtual void print(PyObject*, IceUtilInternal::Output&, PrintObjectHistory*);

    virtual void destroy();

    void printMembers(PyObject*, IceUtilInternal::Output&, PrintObjectHistory*);

    const std::string id;
    const Ice::Int compactId;
    const bool preserve;
    const bool interface;
    const ValueInfoPtr base;
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

    virtual void marshal(PyObject*, Ice::OutputStream*, ObjectMap*, bool, const Ice::StringSeq* = 0);
    virtual void unmarshal(Ice::InputStream*, const UnmarshalCallbackPtr&, PyObject*, void*, bool,
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

    void marshal(PyObject*, Ice::OutputStream*, ObjectMap*);
    PyObject* unmarshal(Ice::InputStream*);

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

    void writeMembers(PyObject*, Ice::OutputStream*, const DataMemberList&, ObjectMap*) const;
};

//
// ObjectWriter wraps a Python object for marshaling.
//
class ObjectWriter : public Ice::Object
{
public:

    ObjectWriter(PyObject*, ObjectMap*, const ValueInfoPtr&);
    ~ObjectWriter();

    virtual void ice_preMarshal();

    virtual void _iceWrite(Ice::OutputStream*) const;
    virtual void _iceRead(Ice::InputStream*);

private:

    void writeMembers(Ice::OutputStream*, const DataMemberList&) const;

    PyObject* _object;
    ObjectMap* _map;
    ValueInfoPtr _info;
    ValueInfoPtr _formal;
};

//
// ObjectReader unmarshals the state of an Ice object.
//
class ObjectReader : public Ice::Object
{
public:

    ObjectReader(PyObject*, const ValueInfoPtr&);
    ~ObjectReader();

    virtual void ice_postUnmarshal();

    virtual void _iceWrite(Ice::OutputStream*) const;
    virtual void _iceRead(Ice::InputStream*);

    virtual ValueInfoPtr getInfo() const;

    PyObject* getObject() const; // Borrowed reference.

    Ice::SlicedDataPtr getSlicedData() const;

private:

    PyObject* _object;
    ValueInfoPtr _info;
    Ice::SlicedDataPtr _slicedData;
};

//
// ExceptionWriter wraps a Python user exception for marshaling.
//
class ExceptionWriter : public Ice::UserException
{
public:

    ExceptionWriter(const PyObjectHandle&, const ExceptionInfoPtr& = 0);
    ~ExceptionWriter() throw();

    virtual std::string ice_id() const;
#ifndef ICE_CPP11_MAPPING
    virtual Ice::UserException* ice_clone() const;
#endif
    virtual void ice_throw() const;

    virtual void _write(Ice::OutputStream*) const;
    virtual void _read(Ice::InputStream*);

    virtual bool _usesClasses() const;

protected:

    virtual void _writeImpl(Ice::OutputStream*) const {}
    virtual void _readImpl(Ice::InputStream*) {}

private:

    PyObjectHandle _ex;
    ExceptionInfoPtr _info;
    ObjectMap _objects;
};

//
// ExceptionReader creates a Python user exception and unmarshals it.
//
class ExceptionReader : public Ice::UserException
{
public:

    ExceptionReader(const ExceptionInfoPtr&);
    ~ExceptionReader() throw();

    virtual std::string ice_id() const;
#ifndef ICE_CPP11_MAPPING
    virtual Ice::UserException* ice_clone() const;
#endif
    virtual void ice_throw() const;

    virtual void _write(Ice::OutputStream*) const;
    virtual void _read(Ice::InputStream*);

    virtual bool _usesClasses() const;

    PyObject* getException() const; // Borrowed reference.

    Ice::SlicedDataPtr getSlicedData() const;

protected:

    virtual void _writeImpl(Ice::OutputStream*) const {}
    virtual void _readImpl(Ice::InputStream*) {}

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
ValueInfoPtr lookupValueInfo(const std::string&);
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
extern "C" PyObject* IcePy_declareValue(PyObject*, PyObject*);
extern "C" PyObject* IcePy_defineValue(PyObject*, PyObject*);
extern "C" PyObject* IcePy_defineException(PyObject*, PyObject*);
extern "C" PyObject* IcePy_stringify(PyObject*, PyObject*);
extern "C" PyObject* IcePy_stringifyException(PyObject*, PyObject*);

#endif
