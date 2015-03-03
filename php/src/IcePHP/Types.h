// **********************************************************************
//
// Copyright (c) 2003-2011 ZeroC, Inc. All rights reserved.
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
#include <Ice/Stream.h>
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
ZEND_FUNCTION(IcePHP_defineProxy);
ZEND_FUNCTION(IcePHP_declareClass);
ZEND_FUNCTION(IcePHP_defineClass);
ZEND_FUNCTION(IcePHP_defineException);
ZEND_FUNCTION(IcePHP_stringify);
ZEND_FUNCTION(IcePHP_stringifyException);
}

#define ICEPHP_TYPE_FUNCTIONS \
    ZEND_FE(IcePHP_defineEnum,          NULL) \
    ZEND_FE(IcePHP_defineStruct,        NULL) \
    ZEND_FE(IcePHP_defineSequence,      NULL) \
    ZEND_FE(IcePHP_defineDictionary,    NULL) \
    ZEND_FE(IcePHP_defineProxy,         NULL) \
    ZEND_FE(IcePHP_declareClass,        NULL) \
    ZEND_FE(IcePHP_defineClass,         NULL) \
    ZEND_FE(IcePHP_defineException,     NULL) \
    ZEND_FE(IcePHP_stringify,           NULL) \
    ZEND_FE(IcePHP_stringifyException,  NULL)

namespace IcePHP
{

//
// This class is raised as an exception when object marshaling needs to be aborted.
//
class AbortMarshaling
{
};

typedef std::map<unsigned int, Ice::ObjectPtr> ObjectMap;

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
// Base class for type information.
//
class TypeInfo : public UnmarshalCallback
{
public:

    virtual std::string getId() const = 0;

    virtual bool validate(zval* TSRMLS_DC) = 0;

    virtual bool usesClasses(); // Default implementation returns false.

    virtual void unmarshaled(zval*, zval*, void* TSRMLS_DC); // Default implementation is assert(false).

    virtual void destroy();

protected:

    TypeInfo();

public:

    //
    // The marshal and unmarshal functions can raise Ice exceptions, and may raise
    // AbortMarshaling if an error occurs.
    //
    virtual void marshal(zval*, const Ice::OutputStreamPtr&, ObjectMap* TSRMLS_DC) = 0;
    virtual void unmarshal(const Ice::InputStreamPtr&, const UnmarshalCallbackPtr&, const CommunicatorInfoPtr&,
                           zval*, void* TSRMLS_DC) = 0;

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

    virtual bool validate(zval* TSRMLS_DC);

    virtual void marshal(zval*, const Ice::OutputStreamPtr&, ObjectMap* TSRMLS_DC);
    virtual void unmarshal(const Ice::InputStreamPtr&, const UnmarshalCallbackPtr&, const CommunicatorInfoPtr&,
                           zval*, void* TSRMLS_DC);

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

    virtual std::string getId() const;

    virtual bool validate(zval* TSRMLS_DC);

    virtual void marshal(zval*, const Ice::OutputStreamPtr&, ObjectMap* TSRMLS_DC);
    virtual void unmarshal(const Ice::InputStreamPtr&, const UnmarshalCallbackPtr&, const CommunicatorInfoPtr&,
                           zval*, void* TSRMLS_DC);

    virtual void print(zval*, IceUtilInternal::Output&, PrintObjectHistory* TSRMLS_DC);

    std::string id;
    Ice::StringSeq enumerators;
};
typedef IceUtil::Handle<EnumInfo> EnumInfoPtr;

class DataMember : public UnmarshalCallback
{
public:

    virtual void unmarshaled(zval*, zval*, void* TSRMLS_DC);

    std::string name;
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

    virtual bool validate(zval* TSRMLS_DC);

    virtual bool usesClasses();

    virtual void marshal(zval*, const Ice::OutputStreamPtr&, ObjectMap* TSRMLS_DC);
    virtual void unmarshal(const Ice::InputStreamPtr&, const UnmarshalCallbackPtr&, const CommunicatorInfoPtr&,
                           zval*, void* TSRMLS_DC);

    virtual void print(zval*, IceUtilInternal::Output&, PrintObjectHistory* TSRMLS_DC);

    virtual void destroy();

    std::string id;
    std::string name; // PHP class name
    DataMemberList members;
    zend_class_entry* zce;
};
typedef IceUtil::Handle<StructInfo> StructInfoPtr;

//
// Sequence information.
//
class SequenceInfo : public TypeInfo
{
public:

    virtual std::string getId() const;

    virtual bool validate(zval* TSRMLS_DC);

    virtual bool usesClasses();

    virtual void marshal(zval*, const Ice::OutputStreamPtr&, ObjectMap* TSRMLS_DC);
    virtual void unmarshal(const Ice::InputStreamPtr&, const UnmarshalCallbackPtr&, const CommunicatorInfoPtr&,
                           zval*, void* TSRMLS_DC);

    virtual void print(zval*, IceUtilInternal::Output&, PrintObjectHistory* TSRMLS_DC);

    virtual void unmarshaled(zval*, zval*, void* TSRMLS_DC);

    virtual void destroy();

    std::string id;
    TypeInfoPtr elementType;

private:

    void marshalPrimitiveSequence(const PrimitiveInfoPtr&, zval*, const Ice::OutputStreamPtr& TSRMLS_DC);
    void unmarshalPrimitiveSequence(const PrimitiveInfoPtr&, const Ice::InputStreamPtr&, const UnmarshalCallbackPtr&,
                                    zval*, void* TSRMLS_DC);
};
typedef IceUtil::Handle<SequenceInfo> SequenceInfoPtr;

//
// Dictionary information.
//
class DictionaryInfo : public TypeInfo
{
public:

    virtual std::string getId() const;

    virtual bool validate(zval* TSRMLS_DC);

    virtual bool usesClasses();

    virtual void marshal(zval*, const Ice::OutputStreamPtr&, ObjectMap* TSRMLS_DC);
    virtual void unmarshal(const Ice::InputStreamPtr&, const UnmarshalCallbackPtr&, const CommunicatorInfoPtr&,
                           zval*, void* TSRMLS_DC);

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
};
typedef IceUtil::Handle<DictionaryInfo> DictionaryInfoPtr;

class ExceptionInfo;
typedef IceUtil::Handle<ExceptionInfo> ExceptionInfoPtr;
typedef std::vector<ExceptionInfoPtr> ExceptionInfoList;

class ClassInfo;
typedef IceUtil::Handle<ClassInfo> ClassInfoPtr;
typedef std::vector<ClassInfoPtr> ClassInfoList;

typedef std::vector<TypeInfoPtr> TypeInfoList;

class ClassInfo : public TypeInfo
{
public:

    virtual std::string getId() const;

    virtual bool validate(zval* TSRMLS_DC);

    virtual bool usesClasses();

    virtual void marshal(zval*, const Ice::OutputStreamPtr&, ObjectMap* TSRMLS_DC);
    virtual void unmarshal(const Ice::InputStreamPtr&, const UnmarshalCallbackPtr&, const CommunicatorInfoPtr&,
                           zval*, void* TSRMLS_DC);

    virtual void print(zval*, IceUtilInternal::Output&, PrintObjectHistory* TSRMLS_DC);

    virtual void destroy();

    void printMembers(zval*, IceUtilInternal::Output&, PrintObjectHistory* TSRMLS_DC);

    bool isA(const std::string&) const;

    void addOperation(const std::string&, const OperationPtr&);
    OperationPtr getOperation(const std::string&) const;

    std::string id;
    std::string name; // PHP class name
    bool isAbstract;
    ClassInfoPtr base;
    ClassInfoList interfaces;
    DataMemberList members;
    bool defined;
    zend_class_entry* zce;

    typedef std::map<std::string, OperationPtr> OperationMap;
    OperationMap operations;
};

//
// Proxy information.
//
class ProxyInfo : public TypeInfo
{
public:

    virtual std::string getId() const;

    virtual bool validate(zval* TSRMLS_DC);

    virtual void marshal(zval*, const Ice::OutputStreamPtr&, ObjectMap* TSRMLS_DC);
    virtual void unmarshal(const Ice::InputStreamPtr&, const UnmarshalCallbackPtr&, const CommunicatorInfoPtr&,
                           zval*, void* TSRMLS_DC);

    virtual void print(zval*, IceUtilInternal::Output&, PrintObjectHistory* TSRMLS_DC);

    virtual void destroy();

    std::string id;
    ClassInfoPtr cls;
};
typedef IceUtil::Handle<ProxyInfo> ProxyInfoPtr;

//
// Exception information.
//
class ExceptionInfo : public IceUtil::Shared
{
public:

    zval* unmarshal(const Ice::InputStreamPtr&, const CommunicatorInfoPtr& TSRMLS_DC);

    void print(zval*, IceUtilInternal::Output& TSRMLS_DC);
    void printMembers(zval*, IceUtilInternal::Output&, PrintObjectHistory* TSRMLS_DC);

    bool isA(const std::string&) const;

    std::string id;
    std::string name; // PHP class name
    ExceptionInfoPtr base;
    DataMemberList members;
    bool usesClasses;
    zend_class_entry* zce;
};

ClassInfoPtr getClassInfoById(const std::string& TSRMLS_DC);
ClassInfoPtr getClassInfoByName(const std::string& TSRMLS_DC);
ExceptionInfoPtr getExceptionInfo(const std::string& TSRMLS_DC);

bool typesInit(TSRMLS_D);
bool typesRequestInit(TSRMLS_D);
bool typesRequestShutdown(TSRMLS_D);

//
// ObjectWriter wraps a PHP object for marshaling.
//
class ObjectWriter : public Ice::ObjectWriter
{
public:

    ObjectWriter(const ClassInfoPtr&, zval*, ObjectMap* TSRMLS_DC);
    ~ObjectWriter();

    virtual void ice_preMarshal();

    virtual void write(const Ice::OutputStreamPtr&) const;

private:

    ClassInfoPtr _info;
    zval* _object;
    ObjectMap* _map;
#if ZTS
    TSRMLS_D;
#endif
};

//
// ObjectReader unmarshals the state of an Ice object.
//
class ObjectReader : public Ice::ObjectReader
{
public:

    ObjectReader(zval*, const ClassInfoPtr&, const CommunicatorInfoPtr& TSRMLS_DC);
    ~ObjectReader();

    virtual void ice_postUnmarshal();

    virtual void read(const Ice::InputStreamPtr&, bool);

    virtual ClassInfoPtr getInfo() const;

    zval* getObject() const;

private:

    zval* _object;
    ClassInfoPtr _info;
    CommunicatorInfoPtr _communicator;
#if ZTS
    TSRMLS_D;
#endif
};
typedef IceUtil::Handle<ObjectReader> ObjectReaderPtr;

} // End of namespace IcePHP

#endif
