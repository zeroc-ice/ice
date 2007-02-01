// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
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

    virtual bool usesClasses(); // Default implementation returns false.

    virtual void unmarshaled(VALUE, VALUE, void*); // Default implementation is assert(false).

    virtual void destroy();

protected:

    TypeInfo();

public:

    //
    // The marshal and unmarshal functions can raise Ice exceptions, and may raise
    // AbortMarshaling if an error occurs.
    //
    virtual void marshal(VALUE, const Ice::OutputStreamPtr&, ObjectMap*) = 0;
    virtual void unmarshal(const Ice::InputStreamPtr&, const UnmarshalCallbackPtr&, VALUE, void*) = 0;

    virtual void print(VALUE, IceUtil::Output&, PrintObjectHistory*) = 0;
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

    virtual void marshal(VALUE, const Ice::OutputStreamPtr&, ObjectMap*);
    virtual void unmarshal(const Ice::InputStreamPtr&, const UnmarshalCallbackPtr&, VALUE, void*);

    virtual void print(VALUE, IceUtil::Output&, PrintObjectHistory*);

    static double toDouble(VALUE);

    Kind kind;
};
typedef IceUtil::Handle<PrimitiveInfo> PrimitiveInfoPtr;

//
// Enum information.
//
typedef std::vector<VALUE> EnumeratorList;

class EnumInfo : public TypeInfo
{
public:

    virtual std::string getId() const;

    virtual bool validate(VALUE);

    virtual void marshal(VALUE, const Ice::OutputStreamPtr&, ObjectMap*);
    virtual void unmarshal(const Ice::InputStreamPtr&, const UnmarshalCallbackPtr&, VALUE, void*);

    virtual void print(VALUE, IceUtil::Output&, PrintObjectHistory*);

    std::string id;
    EnumeratorList enumerators;
    VALUE rubyClass;
};
typedef IceUtil::Handle<EnumInfo> EnumInfoPtr;

class DataMember : public UnmarshalCallback
{
public:

    virtual void unmarshaled(VALUE, VALUE, void*);

    std::string name;
    TypeInfoPtr type;
    ID rubyID;
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

    virtual bool validate(VALUE);

    virtual bool usesClasses();

    virtual void marshal(VALUE, const Ice::OutputStreamPtr&, ObjectMap*);
    virtual void unmarshal(const Ice::InputStreamPtr&, const UnmarshalCallbackPtr&, VALUE, void*);

    virtual void print(VALUE, IceUtil::Output&, PrintObjectHistory*);

    virtual void destroy();

    std::string id;
    DataMemberList members;
    VALUE rubyClass;
};
typedef IceUtil::Handle<StructInfo> StructInfoPtr;

//
// Sequence information.
//
class SequenceInfo : public TypeInfo
{
public:

    virtual std::string getId() const;

    virtual bool validate(VALUE);

    virtual bool usesClasses();

    virtual void marshal(VALUE, const Ice::OutputStreamPtr&, ObjectMap*);
    virtual void unmarshal(const Ice::InputStreamPtr&, const UnmarshalCallbackPtr&, VALUE, void*);
    virtual void unmarshaled(VALUE, VALUE, void*);

    virtual void print(VALUE, IceUtil::Output&, PrintObjectHistory*);

    virtual void destroy();

    std::string id;
    TypeInfoPtr elementType;

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

    virtual std::string getId() const;

    virtual bool validate(VALUE);

    virtual bool usesClasses();

    virtual void marshal(VALUE, const Ice::OutputStreamPtr&, ObjectMap*);
    void marshalElement(VALUE, VALUE, const Ice::OutputStreamPtr&, ObjectMap*);
    virtual void unmarshal(const Ice::InputStreamPtr&, const UnmarshalCallbackPtr&, VALUE, void*);
    virtual void unmarshaled(VALUE, VALUE, void*);

    virtual void print(VALUE, IceUtil::Output&, PrintObjectHistory*);
    void printElement(VALUE, VALUE, IceUtil::Output&, PrintObjectHistory*);

    virtual void destroy();

    class KeyCallback : public UnmarshalCallback
    {
    public:

        virtual void unmarshaled(VALUE, VALUE, void*);

        VALUE key;
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

    virtual bool validate(VALUE);

    virtual bool usesClasses();

    virtual void marshal(VALUE, const Ice::OutputStreamPtr&, ObjectMap*);
    virtual void unmarshal(const Ice::InputStreamPtr&, const UnmarshalCallbackPtr&, VALUE, void*);

    virtual void print(VALUE, IceUtil::Output&, PrintObjectHistory*);

    virtual void destroy();

    void printMembers(VALUE, IceUtil::Output&, PrintObjectHistory*);

    bool isA(const ClassInfoPtr&);

    std::string id;
    bool isIceObject; // Is this the ClassInfo for Ice::Object?
    bool isAbstract;
    ClassInfoPtr base;
    ClassInfoList interfaces;
    DataMemberList members;
    VALUE rubyClass;
    VALUE typeObj;
    bool defined;
};

//
// Proxy information.
//
class ProxyInfo : public TypeInfo
{
public:

    virtual std::string getId() const;

    virtual bool validate(VALUE);

    virtual void marshal(VALUE, const Ice::OutputStreamPtr&, ObjectMap*);
    virtual void unmarshal(const Ice::InputStreamPtr&, const UnmarshalCallbackPtr&, VALUE, void*);

    virtual void print(VALUE, IceUtil::Output&, PrintObjectHistory*);

    virtual void destroy();

    std::string id;
    VALUE rubyClass;
    ClassInfoPtr classInfo;
    VALUE typeObj;
};
typedef IceUtil::Handle<ProxyInfo> ProxyInfoPtr;

//
// Exception information.
//
class ExceptionInfo : public IceUtil::Shared
{
public:

    void marshal(VALUE, const Ice::OutputStreamPtr&, ObjectMap*);
    VALUE unmarshal(const Ice::InputStreamPtr&);

    void print(VALUE, IceUtil::Output&);
    void printMembers(VALUE, IceUtil::Output&, PrintObjectHistory*);

    std::string id;
    ExceptionInfoPtr base;
    DataMemberList members;
    bool usesClasses;
    VALUE rubyClass;
};

//
// ObjectWriter wraps a Ruby object for marshaling.
//
class ObjectWriter : public Ice::ObjectWriter
{
public:

    ObjectWriter(const ClassInfoPtr&, VALUE, ObjectMap*);

    virtual void ice_preMarshal();

    virtual void write(const Ice::OutputStreamPtr&) const;

private:

    ClassInfoPtr _info;
    VALUE _object;
    ObjectMap* _map;
};

//
// ObjectReader unmarshals the state of an Ice object.
//
class ObjectReader : public Ice::ObjectReader
{
public:

    ObjectReader(VALUE, const ClassInfoPtr&);

    virtual void ice_postUnmarshal();

    virtual void read(const Ice::InputStreamPtr&, bool);

    virtual ClassInfoPtr getInfo() const;

    VALUE getObject() const; // Borrowed reference.

private:

    VALUE _object;
    ClassInfoPtr _info;
};
typedef IceUtil::Handle<ObjectReader> ObjectReaderPtr;

ClassInfoPtr lookupClassInfo(const std::string&);
ExceptionInfoPtr lookupExceptionInfo(const std::string&);

bool initTypes(VALUE);

VALUE createType(const TypeInfoPtr&);
TypeInfoPtr getType(VALUE);

VALUE createException(const ExceptionInfoPtr&);
ExceptionInfoPtr getException(VALUE);

}

#endif
