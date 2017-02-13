// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_RUBY_TYPES_H
#define ICE_RUBY_TYPES_H

#include <Config.h>
#include <Util.h>
#include <Ice/Stream.h>
#include <IceUtil/OutputUtil.h>

namespace IceRuby
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
// TODO: Need an equivalent Ruby exception.
//
class AbortMarshaling
{
};

typedef std::map<VALUE, Ice::ObjectPtr> ObjectMap;

class ObjectReader;
typedef IceUtil::Handle<ObjectReader> ObjectReaderPtr;

//
// This class keeps track of Ruby objects (instances of Slice classes
// and exceptions) that have preserved slices.
//
class SlicedDataUtil
{
public:

    SlicedDataUtil();
    ~SlicedDataUtil();

    void add(const ObjectReaderPtr&);

    void update();

    static void setMember(VALUE, const Ice::SlicedDataPtr&);
    static Ice::SlicedDataPtr getMember(VALUE, ObjectMap*);

private:

    std::set<ObjectReaderPtr> _readers;
    static VALUE _slicedDataType;
    static VALUE _sliceInfoType;
};

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
    virtual void marshal(VALUE, const Ice::OutputStreamPtr&, ObjectMap*, bool) = 0;
    virtual void unmarshal(const Ice::InputStreamPtr&, const UnmarshalCallbackPtr&, VALUE, void*, bool) = 0;

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

    virtual void marshal(VALUE, const Ice::OutputStreamPtr&, ObjectMap*, bool);
    virtual void unmarshal(const Ice::InputStreamPtr&, const UnmarshalCallbackPtr&, VALUE, void*, bool);

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

    virtual void marshal(VALUE, const Ice::OutputStreamPtr&, ObjectMap*, bool);
    virtual void unmarshal(const Ice::InputStreamPtr&, const UnmarshalCallbackPtr&, VALUE, void*, bool);

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

    virtual void marshal(VALUE, const Ice::OutputStreamPtr&, ObjectMap*, bool);
    virtual void unmarshal(const Ice::InputStreamPtr&, const UnmarshalCallbackPtr&, VALUE, void*, bool);

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

    virtual void marshal(VALUE, const Ice::OutputStreamPtr&, ObjectMap*, bool);
    virtual void unmarshal(const Ice::InputStreamPtr&, const UnmarshalCallbackPtr&, VALUE, void*, bool);
    virtual void unmarshaled(VALUE, VALUE, void*);

    virtual void print(VALUE, IceUtilInternal::Output&, PrintObjectHistory*);

    virtual void destroy();

    const std::string id;
    const TypeInfoPtr elementType;

private:

    void marshalPrimitiveSequence(const PrimitiveInfoPtr&, VALUE, const Ice::OutputStreamPtr&);
    void unmarshalPrimitiveSequence(const PrimitiveInfoPtr&, const Ice::InputStreamPtr&, const UnmarshalCallbackPtr&,
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

    virtual void marshal(VALUE, const Ice::OutputStreamPtr&, ObjectMap*, bool);
    virtual void unmarshal(const Ice::InputStreamPtr&, const UnmarshalCallbackPtr&, VALUE, void*, bool);
    void marshalElement(VALUE, VALUE, const Ice::OutputStreamPtr&, ObjectMap*);
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

    void define(VALUE, VALUE, VALUE, VALUE, VALUE, VALUE, VALUE);

    virtual std::string getId() const;

    virtual bool validate(VALUE);

    virtual bool variableLength() const;
    virtual int wireSize() const;
    virtual Ice::OptionalFormat optionalFormat() const;

    virtual bool usesClasses() const; // Default implementation returns false.

    virtual void marshal(VALUE, const Ice::OutputStreamPtr&, ObjectMap*, bool);
    virtual void unmarshal(const Ice::InputStreamPtr&, const UnmarshalCallbackPtr&, VALUE, void*, bool);

    virtual void print(VALUE, IceUtilInternal::Output&, PrintObjectHistory*);

    virtual void destroy();

    void printMembers(VALUE, IceUtilInternal::Output&, PrintObjectHistory*);

    bool isA(const ClassInfoPtr&);

    const std::string id;
    const Ice::Int compactId;
    const bool isBase; // Is this the ClassInfo for Ice::Object or Ice::LocalObject?
    const bool isLocal;
    const bool isAbstract;
    const bool preserve;
    const ClassInfoPtr base;
    const ClassInfoList interfaces;
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

    void define(VALUE, VALUE);

    virtual std::string getId() const;

    virtual bool validate(VALUE);

    virtual bool variableLength() const;
    virtual int wireSize() const;
    virtual Ice::OptionalFormat optionalFormat() const;

    virtual void marshal(VALUE, const Ice::OutputStreamPtr&, ObjectMap*, bool);
    virtual void unmarshal(const Ice::InputStreamPtr&, const UnmarshalCallbackPtr&, VALUE, void*, bool);

    virtual void print(VALUE, IceUtilInternal::Output&, PrintObjectHistory*);

    virtual void destroy();

    const std::string id;
    const VALUE rubyClass;
    const ClassInfoPtr classInfo;
    const VALUE typeObj;
};
typedef IceUtil::Handle<ProxyInfo> ProxyInfoPtr;

//
// Exception information.
//
class ExceptionInfo : public IceUtil::Shared
{
public:

    VALUE unmarshal(const Ice::InputStreamPtr&);

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
class ObjectWriter : public Ice::ObjectWriter
{
public:

    ObjectWriter(VALUE, ObjectMap*);
    virtual ~ObjectWriter();

    virtual void ice_preMarshal();

    virtual void write(const Ice::OutputStreamPtr&) const;

private:

    void writeMembers(const Ice::OutputStreamPtr&, const DataMemberList&) const;

    VALUE _object;
    ObjectMap* _map;
    ClassInfoPtr _info;
};

//
// ObjectReader unmarshals the state of an Ice object.
//
class ObjectReader : public Ice::ObjectReader
{
public:

    ObjectReader(VALUE, const ClassInfoPtr&);
    virtual ~ObjectReader();

    virtual void ice_postUnmarshal();

    virtual void read(const Ice::InputStreamPtr&);

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

    VALUE getException() const;

    Ice::SlicedDataPtr getSlicedData() const;

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
