// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICEPHP_TYPES_H
#define ICEPHP_TYPES_H

#include <Config.h>
#include <Communicator.h>
#include <Operation.h>
#include <IceUtil/OutputUtil.h>

//
// Global functions.
//
extern "C"
{
ZEND_FUNCTION(IcePHP_defineEnum);
ZEND_FUNCTION(IcePHP_defineStruct);
ZEND_FUNCTION(IcePHP_defineSequence);
ZEND_FUNCTION(IcePHP_defineDictionary);
ZEND_FUNCTION(IcePHP_declareProxy);
ZEND_FUNCTION(IcePHP_defineProxy);
ZEND_FUNCTION(IcePHP_declareClass);
ZEND_FUNCTION(IcePHP_defineClass);
ZEND_FUNCTION(IcePHP_defineException);
ZEND_FUNCTION(IcePHP_stringify);
ZEND_FUNCTION(IcePHP_stringifyException);
}

namespace IcePHP
{

//
// This class is raised as an exception when object marshaling needs to be aborted.
//
class AbortMarshaling
{
};

class ClassInfo;
typedef IceUtil::Handle<ClassInfo> ClassInfoPtr;
typedef std::vector<ClassInfoPtr> ClassInfoList;

typedef std::map<unsigned int, Ice::ObjectPtr> ObjectMap;

class ObjectReader;
typedef IceUtil::Handle<ObjectReader> ObjectReaderPtr;

struct PrintObjectHistory
{
    int index;
    std::map<unsigned int, int> objects;
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
    virtual void unmarshaled(zval*, zval*, void* TSRMLS_DC) = 0;
};
typedef IceUtil::Handle<UnmarshalCallback> UnmarshalCallbackPtr;

//
// ReadObjectCallback retains all of the information necessary to store an unmarshaled
// Slice value as a PHP object.
//
class ReadObjectCallback : public IceUtil::Shared
{
public:

    ReadObjectCallback(const ClassInfoPtr&, const UnmarshalCallbackPtr&, zval*, void* TSRMLS_DC);
    ~ReadObjectCallback();

    virtual void invoke(const ::Ice::ObjectPtr&);

private:

    ClassInfoPtr _info;
    UnmarshalCallbackPtr _cb;
    zval* _target;
    void* _closure;
#if ZTS
    TSRMLS_D;
#endif
};
typedef IceUtil::Handle<ReadObjectCallback> ReadObjectCallbackPtr;

//
// This class keeps track of PHP objects (instances of Slice classes
// and exceptions) that have preserved slices.
//
class StreamUtil
{
public:

    ~StreamUtil();

    //
    // Keep a reference to a ReadObjectCallback for patching purposes.
    //
    void add(const ReadObjectCallbackPtr&);

    //
    // Keep track of object instances that have preserved slices.
    //
    void add(const ObjectReaderPtr&);

    void updateSlicedData(TSRMLS_D);

    static void setSlicedDataMember(zval*, const Ice::SlicedDataPtr& TSRMLS_DC);
    static Ice::SlicedDataPtr getSlicedDataMember(zval*, ObjectMap* TSRMLS_DC);

private:

    std::vector<ReadObjectCallbackPtr> _callbacks;
    std::set<ObjectReaderPtr> _readers;
    static zend_class_entry* _slicedDataType;
    static zend_class_entry* _sliceInfoType;
};

//
// Base class for type information.
//
class TypeInfo : public UnmarshalCallback
{
public:

    virtual std::string getId() const = 0;

    virtual bool validate(zval*, bool TSRMLS_DC) = 0;

    virtual bool variableLength() const = 0;
    virtual int wireSize() const = 0;
    virtual Ice::OptionalFormat optionalFormat() const = 0;

    virtual bool usesClasses() const; // Default implementation returns false.

    virtual void unmarshaled(zval*, zval*, void* TSRMLS_DC); // Default implementation is assert(false).

    virtual void destroy();

protected:

    TypeInfo();

public:

    //
    // The marshal and unmarshal functions can raise Ice exceptions, and may raise
    // AbortMarshaling if an error occurs.
    //
    virtual void marshal(zval*, Ice::OutputStream*, ObjectMap*, bool TSRMLS_DC) = 0;
    virtual void unmarshal(Ice::InputStream*, const UnmarshalCallbackPtr&, const CommunicatorInfoPtr&,
                           zval*, void*, bool TSRMLS_DC) = 0;

    virtual void print(zval*, IceUtilInternal::Output&, PrintObjectHistory* TSRMLS_DC) = 0;
};
typedef IceUtil::Handle<TypeInfo> TypeInfoPtr;

//
// Primitive type information.
//
class PrimitiveInfo : public TypeInfo
{
public:

    virtual std::string getId() const;

    virtual bool validate(zval*, bool TSRMLS_DC);

    virtual bool variableLength() const;
    virtual int wireSize() const;
    virtual Ice::OptionalFormat optionalFormat() const;

    virtual void marshal(zval*, Ice::OutputStream*, ObjectMap*, bool TSRMLS_DC);
    virtual void unmarshal(Ice::InputStream*, const UnmarshalCallbackPtr&, const CommunicatorInfoPtr&,
                           zval*, void*, bool TSRMLS_DC);

    virtual void print(zval*, IceUtilInternal::Output&, PrintObjectHistory* TSRMLS_DC);

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
class EnumInfo : public TypeInfo
{
public:

    EnumInfo(const std::string&, zval* TSRMLS_DC);

    virtual std::string getId() const;

    virtual bool validate(zval*, bool TSRMLS_DC);

    virtual bool variableLength() const;
    virtual int wireSize() const;
    virtual Ice::OptionalFormat optionalFormat() const;

    virtual void marshal(zval*, Ice::OutputStream*, ObjectMap*, bool TSRMLS_DC);
    virtual void unmarshal(Ice::InputStream*, const UnmarshalCallbackPtr&, const CommunicatorInfoPtr&,
                           zval*, void*, bool TSRMLS_DC);

    virtual void print(zval*, IceUtilInternal::Output&, PrintObjectHistory* TSRMLS_DC);

    const std::string id;
    const std::map<Ice::Int, std::string> enumerators;
    const Ice::Int maxValue;

};
typedef IceUtil::Handle<EnumInfo> EnumInfoPtr;

class DataMember : public UnmarshalCallback
{
public:

    virtual void unmarshaled(zval*, zval*, void* TSRMLS_DC);

    void setMember(zval*, zval* TSRMLS_DC);

    std::string name;
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

    StructInfo(const std::string&, const std::string&, zval* TSRMLS_DC);

    virtual std::string getId() const;

    virtual bool validate(zval*, bool TSRMLS_DC);

    virtual bool variableLength() const;
    virtual int wireSize() const;
    virtual Ice::OptionalFormat optionalFormat() const;

    virtual bool usesClasses() const;

    virtual void marshal(zval*, Ice::OutputStream*, ObjectMap*, bool TSRMLS_DC);
    virtual void unmarshal(Ice::InputStream*, const UnmarshalCallbackPtr&, const CommunicatorInfoPtr&,
                           zval*, void*, bool TSRMLS_DC);

    virtual void print(zval*, IceUtilInternal::Output&, PrintObjectHistory* TSRMLS_DC);

    virtual void destroy();

    const std::string id;
    const std::string name; // PHP class name
    const DataMemberList members;
    const zend_class_entry* zce;

private:

    bool _variableLength;
    int _wireSize;
    zval* _nullMarshalValue;
};
typedef IceUtil::Handle<StructInfo> StructInfoPtr;

//
// Sequence information.
//
class SequenceInfo : public TypeInfo
{
public:

    SequenceInfo(const std::string&, zval* TSRMLS_DC);

    virtual std::string getId() const;

    virtual bool validate(zval*, bool TSRMLS_DC);

    virtual bool variableLength() const;
    virtual int wireSize() const;
    virtual Ice::OptionalFormat optionalFormat() const;

    virtual bool usesClasses() const;

    virtual void marshal(zval*, Ice::OutputStream*, ObjectMap*, bool TSRMLS_DC);
    virtual void unmarshal(Ice::InputStream*, const UnmarshalCallbackPtr&, const CommunicatorInfoPtr&,
                           zval*, void*, bool TSRMLS_DC);

    virtual void print(zval*, IceUtilInternal::Output&, PrintObjectHistory* TSRMLS_DC);

    virtual void unmarshaled(zval*, zval*, void* TSRMLS_DC);

    virtual void destroy();

    const std::string id;
    const TypeInfoPtr elementType;

private:

    void marshalPrimitiveSequence(const PrimitiveInfoPtr&, zval*, Ice::OutputStream* TSRMLS_DC);
    void unmarshalPrimitiveSequence(const PrimitiveInfoPtr&, Ice::InputStream*, const UnmarshalCallbackPtr&,
                                    zval*, void* TSRMLS_DC);
};
typedef IceUtil::Handle<SequenceInfo> SequenceInfoPtr;

//
// Dictionary information.
//
class DictionaryInfo : public TypeInfo
{
public:

    DictionaryInfo(const std::string&, zval*, zval* TSRMLS_DC);

    virtual std::string getId() const;

    virtual bool validate(zval*, bool TSRMLS_DC);

    virtual bool variableLength() const;
    virtual int wireSize() const;
    virtual Ice::OptionalFormat optionalFormat() const;

    virtual bool usesClasses() const;

    virtual void marshal(zval*, Ice::OutputStream*, ObjectMap*, bool TSRMLS_DC);
    virtual void unmarshal(Ice::InputStream*, const UnmarshalCallbackPtr&, const CommunicatorInfoPtr&,
                           zval*, void*, bool TSRMLS_DC);

    virtual void print(zval*, IceUtilInternal::Output&, PrintObjectHistory* TSRMLS_DC);

    virtual void destroy();

    class KeyCallback : public UnmarshalCallback
    {
    public:

        KeyCallback();
        ~KeyCallback();

        virtual void unmarshaled(zval*, zval*, void* TSRMLS_DC);

        zval* key;
    };
    typedef IceUtil::Handle<KeyCallback> KeyCallbackPtr;

    class ValueCallback : public UnmarshalCallback
    {
    public:

        ValueCallback(zval* TSRMLS_DC);
        ~ValueCallback();

        virtual void unmarshaled(zval*, zval*, void* TSRMLS_DC);

        zval* key;
    };
    typedef IceUtil::Handle<ValueCallback> ValueCallbackPtr;

    std::string id;
    TypeInfoPtr keyType;
    TypeInfoPtr valueType;

private:

    bool _variableLength;
    int _wireSize;
};
typedef IceUtil::Handle<DictionaryInfo> DictionaryInfoPtr;

class ExceptionInfo;
typedef IceUtil::Handle<ExceptionInfo> ExceptionInfoPtr;
typedef std::vector<ExceptionInfoPtr> ExceptionInfoList;

typedef std::vector<TypeInfoPtr> TypeInfoList;

class ClassInfo : public TypeInfo
{
public:

    ClassInfo(const std::string& TSRMLS_DC);

    void define(const std::string&, Ice::Int, bool, bool, zval*, zval*, zval* TSRMLS_DC);

    virtual std::string getId() const;

    virtual bool validate(zval*, bool TSRMLS_DC);

    virtual bool variableLength() const;
    virtual int wireSize() const;
    virtual Ice::OptionalFormat optionalFormat() const;

    virtual bool usesClasses() const;

    virtual void marshal(zval*, Ice::OutputStream*, ObjectMap*, bool TSRMLS_DC);
    virtual void unmarshal(Ice::InputStream*, const UnmarshalCallbackPtr&, const CommunicatorInfoPtr&,
                           zval*, void*, bool TSRMLS_DC);

    virtual void print(zval*, IceUtilInternal::Output&, PrintObjectHistory* TSRMLS_DC);

    virtual void destroy();

    void printMembers(zval*, IceUtilInternal::Output&, PrintObjectHistory* TSRMLS_DC);

    bool isA(const std::string&) const;

    void addOperation(const std::string&, const OperationPtr&);
    OperationPtr getOperation(const std::string&) const;

    const std::string id;
    const std::string name; // PHP class name
    const Ice::Int compactId;
    const bool isAbstract;
    const bool preserve;
    const ClassInfoPtr base;
    const ClassInfoList interfaces;
    const DataMemberList members;
    const DataMemberList optionalMembers;
    const zend_class_entry* zce;
    bool defined;

    typedef std::map<std::string, OperationPtr> OperationMap;
    OperationMap operations;
};

//
// Proxy information.
//
class ProxyInfo : public TypeInfo
{
public:

    ProxyInfo(const std::string& TSRMLS_DC);

    void define(const ClassInfoPtr& TSRMLS_DC);

    virtual std::string getId() const;

    virtual bool validate(zval*, bool TSRMLS_DC);

    virtual bool variableLength() const;
    virtual int wireSize() const;
    virtual Ice::OptionalFormat optionalFormat() const;

    virtual void marshal(zval*, Ice::OutputStream*, ObjectMap*, bool TSRMLS_DC);
    virtual void unmarshal(Ice::InputStream*, const UnmarshalCallbackPtr&, const CommunicatorInfoPtr&,
                           zval*, void*, bool TSRMLS_DC);

    virtual void print(zval*, IceUtilInternal::Output&, PrintObjectHistory* TSRMLS_DC);

    virtual void destroy();

    const std::string id;
    const ClassInfoPtr cls;
    bool defined;
};
typedef IceUtil::Handle<ProxyInfo> ProxyInfoPtr;

//
// Exception information.
//
class ExceptionInfo : public IceUtil::Shared
{
public:

    zval* unmarshal(Ice::InputStream*, const CommunicatorInfoPtr& TSRMLS_DC);

    void print(zval*, IceUtilInternal::Output& TSRMLS_DC);
    void printMembers(zval*, IceUtilInternal::Output&, PrintObjectHistory* TSRMLS_DC);

    bool isA(const std::string&) const;

    std::string id;
    std::string name; // PHP class name
    bool preserve;
    ExceptionInfoPtr base;
    DataMemberList members;
    DataMemberList optionalMembers;
    bool usesClasses;
    zend_class_entry* zce;
};

ClassInfoPtr getClassInfoById(const std::string& TSRMLS_DC);
ClassInfoPtr getClassInfoByName(const std::string& TSRMLS_DC);
ExceptionInfoPtr getExceptionInfo(const std::string& TSRMLS_DC);

bool isUnset(zval* TSRMLS_DC);
void assignUnset(zval* TSRMLS_DC);

bool typesInit(INIT_FUNC_ARGS);
bool typesRequestInit(TSRMLS_D);
bool typesRequestShutdown(TSRMLS_D);

//
// ObjectWriter wraps a PHP object for marshaling.
//
class ObjectWriter : public Ice::Object
{
public:

    ObjectWriter(zval*, ObjectMap*, const ClassInfoPtr& TSRMLS_DC);
    ~ObjectWriter();

    virtual void ice_preMarshal();

    virtual void iceWrite(Ice::OutputStream*) const;
    virtual void iceRead(Ice::InputStream*);

private:

    void writeMembers(Ice::OutputStream*, const DataMemberList&) const;

    zval* _object;
    ObjectMap* _map;
    ClassInfoPtr _info;
#if ZTS
    TSRMLS_D;
#endif
};

//
// ObjectReader unmarshals the state of an Ice object.
//
class ObjectReader : public Ice::Object
{
public:

    ObjectReader(zval*, const ClassInfoPtr&, const CommunicatorInfoPtr& TSRMLS_DC);
    ~ObjectReader();

    virtual void ice_postUnmarshal();

    virtual void iceWrite(Ice::OutputStream*) const;
    virtual void iceRead(Ice::InputStream*);

    virtual ClassInfoPtr getInfo() const;

    zval* getObject() const;

    Ice::SlicedDataPtr getSlicedData() const;

private:

    zval* _object;
    ClassInfoPtr _info;
    CommunicatorInfoPtr _communicator;
    Ice::SlicedDataPtr _slicedData;
#if ZTS
    TSRMLS_D;
#endif
};

//
// ExceptionReader creates a PHP user exception and unmarshals it.
//
class ExceptionReader : public Ice::UserException
{
public:

    ExceptionReader(const CommunicatorInfoPtr&, const ExceptionInfoPtr& TSRMLS_DC);
    ~ExceptionReader() throw();

    virtual std::string ice_id() const;
    virtual ExceptionReader* ice_clone() const;
    virtual void ice_throw() const;

    virtual void iceWrite(Ice::OutputStream*) const;
    virtual void iceRead(Ice::InputStream*);
    virtual bool iceUsesClasses() const;

    ExceptionInfoPtr getInfo() const;

    zval* getException() const;

    Ice::SlicedDataPtr getSlicedData() const;

    using Ice::UserException::iceRead;
    using Ice::UserException::iceWrite;

protected:

    virtual void iceWriteImpl(Ice::OutputStream*) const {}
    virtual void iceReadImpl(Ice::InputStream*) {}

private:

    CommunicatorInfoPtr _communicatorInfo;
    ExceptionInfoPtr _info;
    zval* _ex;
    Ice::SlicedDataPtr _slicedData;
#if ZTS
    TSRMLS_D;
#endif
};

class IdResolver : public Ice::CompactIdResolver
{
public:

    IdResolver(TSRMLS_D);

    virtual std::string resolve(Ice::Int) const;

private:

#if ZTS
    TSRMLS_D;
#endif
};

} // End of namespace IcePHP

#endif
