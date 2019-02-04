//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_RUBY_TYPES_H
#define ICE_RUBY_TYPES_H

#include <Config.h>
#include <Util.h>
#include <Ice/FactoryTable.h>
#include <Ice/Object.h>
#include <Ice/SlicedData.h>
#include <IceUtil/OutputUtil.h>

namespace IceRuby
{

class ExceptionInfo;
typedef IceUtil::Handle<ExceptionInfo> ExceptionInfoPtr;
typedef std::vector<ExceptionInfoPtr> ExceptionInfoList;

class ClassInfo;
typedef IceUtil::Handle<ClassInfo> ClassInfoPtr;

class ProxyInfo;
typedef IceUtil::Handle<ProxyInfo> ProxyInfoPtr;
typedef std::vector<ProxyInfoPtr> ProxyInfoList;

//
// This class is raised as an exception when object marshaling needs to be aborted.
//
// TODO: Need an equivalent Ruby exception.
//
class AbortMarshaling
{
};

typedef std::map<VALUE, Ice::ObjectPtr> ObjectMap;

class ObjectReader;
typedef IceUtil::Handle<ObjectReader> ObjectReaderPtr;

struct PrintObjectHistory
{
    int index;
    std::map<VALUE, int> objects;
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
    virtual void unmarshaled(VALUE, VALUE, void*) = 0;
};
typedef IceUtil::Handle<UnmarshalCallback> UnmarshalCallbackPtr;

//
// ReadObjectCallback retains all of the information necessary to store an unmarshaled
// Slice value as a Ruby object.
//
class ReadObjectCallback : public IceUtil::Shared
{
public:

    ReadObjectCallback(const ClassInfoPtr&, const UnmarshalCallbackPtr&, VALUE, void*);

    void invoke(const ::Ice::ObjectPtr&);

private:

    ClassInfoPtr _info;
    UnmarshalCallbackPtr _cb;
    VALUE _target;
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

    static void setSlicedDataMember(VALUE, const Ice::SlicedDataPtr&);
    static Ice::SlicedDataPtr getSlicedDataMember(VALUE, ObjectMap*);

private:

    std::vector<ReadObjectCallbackPtr> _callbacks;
    std::set<ObjectReaderPtr> _readers;
    static VALUE _slicedDataType;
    static VALUE _sliceInfoType;
};

//
// Base class for type information.
//
class TypeInfo : public UnmarshalCallback
{
public:

    virtual std::string getId() const = 0;

    virtual bool validate(VALUE) = 0;

    virtual bool variableLength() const = 0;
    virtual int wireSize() const = 0;
    virtual Ice::OptionalFormat optionalFormat() const = 0;

    virtual bool usesClasses() const; // Default implementation returns false.

    virtual void unmarshaled(VALUE, VALUE, void*); // Default implementation is assert(false).

    virtual void destroy();

protected:

    TypeInfo();

public:

    //
    // The marshal and unmarshal functions can raise Ice exceptions, and may raise
    // AbortMarshaling if an error occurs.
    //
    virtual void marshal(VALUE, Ice::OutputStream*, ObjectMap*, bool) = 0;
    virtual void unmarshal(Ice::InputStream*, const UnmarshalCallbackPtr&, VALUE, void*, bool) = 0;

    virtual void print(VALUE, IceUtilInternal::Output&, PrintObjectHistory*) = 0;
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

    PrimitiveInfo();
    PrimitiveInfo(Kind);

    virtual std::string getId() const;

    virtual bool validate(VALUE);

    virtual bool variableLength() const;
    virtual int wireSize() const;
    virtual Ice::OptionalFormat optionalFormat() const;

    virtual void marshal(VALUE, Ice::OutputStream*, ObjectMap*, bool);
    virtual void unmarshal(Ice::InputStream*, const UnmarshalCallbackPtr&, VALUE, void*, bool);

    virtual void print(VALUE, IceUtilInternal::Output&, PrintObjectHistory*);

    static double toDouble(VALUE);

    Kind kind;
};
typedef IceUtil::Handle<PrimitiveInfo> PrimitiveInfoPtr;

//
// Enum information.
//
typedef std::map<Ice::Int, VALUE> EnumeratorMap;

class EnumInfo : public TypeInfo
{
public:

    EnumInfo(VALUE, VALUE, VALUE);

    virtual std::string getId() const;

    virtual bool validate(VALUE);

    virtual bool variableLength() const;
    virtual int wireSize() const;
    virtual Ice::OptionalFormat optionalFormat() const;

    virtual void marshal(VALUE, Ice::OutputStream*, ObjectMap*, bool);
    virtual void unmarshal(Ice::InputStream*, const UnmarshalCallbackPtr&, VALUE, void*, bool);

    virtual void print(VALUE, IceUtilInternal::Output&, PrintObjectHistory*);

    const std::string id;
    const VALUE rubyClass;
    const Ice::Int maxValue;
    const EnumeratorMap enumerators;
};
typedef IceUtil::Handle<EnumInfo> EnumInfoPtr;

class DataMember : public UnmarshalCallback
{
public:

    virtual void unmarshaled(VALUE, VALUE, void*);

    std::string name;
    TypeInfoPtr type;
    ID rubyID;
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

    StructInfo(VALUE, VALUE, VALUE);

    virtual std::string getId() const;

    virtual bool validate(VALUE);

    virtual bool variableLength() const;
    virtual int wireSize() const;
    virtual Ice::OptionalFormat optionalFormat() const;

    virtual bool usesClasses() const; // Default implementation returns false.

    virtual void marshal(VALUE, Ice::OutputStream*, ObjectMap*, bool);
    virtual void unmarshal(Ice::InputStream*, const UnmarshalCallbackPtr&, VALUE, void*, bool);

    virtual void print(VALUE, IceUtilInternal::Output&, PrintObjectHistory*);

    virtual void destroy();

    const std::string id;
    const DataMemberList members;
    const VALUE rubyClass;

private:

    bool _variableLength;
    int _wireSize;
    VALUE _nullMarshalValue;
};
typedef IceUtil::Handle<StructInfo> StructInfoPtr;

//
// Sequence information.
//
class SequenceInfo : public TypeInfo
{
public:

    SequenceInfo(VALUE, VALUE);

    virtual std::string getId() const;

    virtual bool validate(VALUE);

    virtual bool variableLength() const;
    virtual int wireSize() const;
    virtual Ice::OptionalFormat optionalFormat() const;

    virtual bool usesClasses() const; // Default implementation returns false.

    virtual void marshal(VALUE, Ice::OutputStream*, ObjectMap*, bool);
    virtual void unmarshal(Ice::InputStream*, const UnmarshalCallbackPtr&, VALUE, void*, bool);
    virtual void unmarshaled(VALUE, VALUE, void*);

    virtual void print(VALUE, IceUtilInternal::Output&, PrintObjectHistory*);

    virtual void destroy();

    const std::string id;
    const TypeInfoPtr elementType;

private:

    void marshalPrimitiveSequence(const PrimitiveInfoPtr&, VALUE, Ice::OutputStream*);
    void unmarshalPrimitiveSequence(const PrimitiveInfoPtr&, Ice::InputStream*, const UnmarshalCallbackPtr&,
                                    VALUE, void*);
};
typedef IceUtil::Handle<SequenceInfo> SequenceInfoPtr;

//
// Dictionary information.
//
class DictionaryInfo : public TypeInfo
{
public:

    DictionaryInfo(VALUE, VALUE, VALUE);

    virtual std::string getId() const;

    virtual bool validate(VALUE);

    virtual bool variableLength() const;
    virtual int wireSize() const;
    virtual Ice::OptionalFormat optionalFormat() const;

    virtual bool usesClasses() const; // Default implementation returns false.

    virtual void marshal(VALUE, Ice::OutputStream*, ObjectMap*, bool);
    virtual void unmarshal(Ice::InputStream*, const UnmarshalCallbackPtr&, VALUE, void*, bool);
    void marshalElement(VALUE, VALUE, Ice::OutputStream*, ObjectMap*);
    virtual void unmarshaled(VALUE, VALUE, void*);

    virtual void print(VALUE, IceUtilInternal::Output&, PrintObjectHistory*);
    void printElement(VALUE, VALUE, IceUtilInternal::Output&, PrintObjectHistory*);

    virtual void destroy();

    class KeyCallback : public UnmarshalCallback
    {
    public:

        virtual void unmarshaled(VALUE, VALUE, void*);

        VALUE key;
    };
    typedef IceUtil::Handle<KeyCallback> KeyCallbackPtr;

    const std::string id;
    const TypeInfoPtr keyType;
    const TypeInfoPtr valueType;

private:

    bool _variableLength;
    int _wireSize;
};
typedef IceUtil::Handle<DictionaryInfo> DictionaryInfoPtr;

typedef std::vector<TypeInfoPtr> TypeInfoList;

class ClassInfo : public TypeInfo
{
public:

    ClassInfo(VALUE, bool);

    void define(VALUE, VALUE, VALUE, VALUE, VALUE, VALUE);

    virtual std::string getId() const;

    virtual bool validate(VALUE);

    virtual bool variableLength() const;
    virtual int wireSize() const;
    virtual Ice::OptionalFormat optionalFormat() const;

    virtual bool usesClasses() const; // Default implementation returns false.

    virtual void marshal(VALUE, Ice::OutputStream*, ObjectMap*, bool);
    virtual void unmarshal(Ice::InputStream*, const UnmarshalCallbackPtr&, VALUE, void*, bool);

    virtual void print(VALUE, IceUtilInternal::Output&, PrintObjectHistory*);

    virtual void destroy();

    void printMembers(VALUE, IceUtilInternal::Output&, PrintObjectHistory*);

    bool isA(const ClassInfoPtr&);

    const std::string id;
    const Ice::Int compactId;
    const bool isBase; // Is this the ClassInfo for Ice::Object or Ice::LocalObject?
    const bool isLocal;
    const bool preserve;
    const bool interface;
    const ClassInfoPtr base;
    const DataMemberList members;
    const DataMemberList optionalMembers;
    const VALUE rubyClass;
    const VALUE typeObj;
    const bool defined;
};

//
// Proxy information.
//
class ProxyInfo : public TypeInfo
{
public:

    ProxyInfo(VALUE);

    void define(VALUE, VALUE, VALUE);

    virtual std::string getId() const;

    virtual bool validate(VALUE);

    virtual bool variableLength() const;
    virtual int wireSize() const;
    virtual Ice::OptionalFormat optionalFormat() const;

    virtual void marshal(VALUE, Ice::OutputStream*, ObjectMap*, bool);
    virtual void unmarshal(Ice::InputStream*, const UnmarshalCallbackPtr&, VALUE, void*, bool);

    virtual void print(VALUE, IceUtilInternal::Output&, PrintObjectHistory*);

    virtual void destroy();

    bool isA(const ProxyInfoPtr&);

    const std::string id;
    const bool isBase; // Is this the ClassInfo for Ice::ObjectPrx?
    const ProxyInfoPtr base;
    const ProxyInfoList interfaces;
    const VALUE rubyClass;
    const VALUE typeObj;
};

//
// Exception information.
//
class ExceptionInfo : public IceUtil::Shared
{
public:

    VALUE unmarshal(Ice::InputStream*);

    void print(VALUE, IceUtilInternal::Output&);
    void printMembers(VALUE, IceUtilInternal::Output&, PrintObjectHistory*);

    std::string id;
    bool preserve;
    ExceptionInfoPtr base;
    DataMemberList members;
    DataMemberList optionalMembers;
    bool usesClasses;
    VALUE rubyClass;
};

//
// ObjectWriter wraps a Ruby object for marshaling.
//
class ObjectWriter : public Ice::Object
{
public:

    ObjectWriter(VALUE, ObjectMap*, const ClassInfoPtr&);
    virtual ~ObjectWriter();

    virtual void ice_preMarshal();

    virtual void _iceWrite(Ice::OutputStream*) const;
    virtual void _iceRead(Ice::InputStream*);

private:

    void writeMembers(Ice::OutputStream*, const DataMemberList&) const;

    VALUE _object;
    ObjectMap* _map;
    ClassInfoPtr _info;
    ClassInfoPtr _formal;
};

//
// ObjectReader unmarshals the state of an Ice object.
//
class ObjectReader : public Ice::Object
{
public:

    ObjectReader(VALUE, const ClassInfoPtr&);
    virtual ~ObjectReader();

    virtual void ice_postUnmarshal();

    virtual void _iceWrite(Ice::OutputStream*) const;
    virtual void _iceRead(Ice::InputStream*);

    virtual ClassInfoPtr getInfo() const;

    VALUE getObject() const; // Borrowed reference.

    Ice::SlicedDataPtr getSlicedData() const;

private:

    VALUE _object;
    ClassInfoPtr _info;
    Ice::SlicedDataPtr _slicedData;
};

//
// ExceptionReader creates a Ruby user exception and unmarshals it.
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

    VALUE getException() const;

    Ice::SlicedDataPtr getSlicedData() const;

protected:

    virtual void _writeImpl(Ice::OutputStream*) const {}
    virtual void _readImpl(Ice::InputStream*) {}

private:

    ExceptionInfoPtr _info;
    VALUE _ex;
    Ice::SlicedDataPtr _slicedData;
};

class IdResolver : public Ice::CompactIdResolver
{
public:

    virtual ::std::string resolve(Ice::Int) const;
};

ClassInfoPtr lookupClassInfo(const std::string&);
ExceptionInfoPtr lookupExceptionInfo(const std::string&);

extern VALUE Unset;

bool initTypes(VALUE);

VALUE createType(const TypeInfoPtr&);
TypeInfoPtr getType(VALUE);

VALUE createException(const ExceptionInfoPtr&);
ExceptionInfoPtr getException(VALUE);

}

#endif
