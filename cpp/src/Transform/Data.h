// **********************************************************************
//
// Copyright (c) 2003
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
//
// **********************************************************************

#ifndef TRANSFORM_DATA_H
#define TRANSFORM_DATA_H

#include <Ice/Ice.h>
#include <Slice/Parser.h>
#include <IceUtil/OutputUtil.h>
#include <Transform/Error.h>

namespace Transform
{

class DataFactory;
typedef IceUtil::Handle<DataFactory> DataFactoryPtr;

class Data;
typedef IceUtil::Handle<Data> DataPtr;

typedef std::map<std::string, DataPtr> DataMemberMap;
typedef std::vector<DataPtr> DataList;
typedef std::map<DataPtr, DataPtr> DataMap;

class ObjectWriter;
class ObjectReader;

class ObjectData;
typedef IceUtil::Handle<ObjectData> ObjectDataPtr;
typedef std::map<const ObjectData*, ObjectDataPtr> ObjectDataMap;
typedef std::map<const ObjectData*, bool> ObjectDataHistory;

class DataInterceptor
{
public:

    virtual ~DataInterceptor();

    virtual bool preTransform(const DataPtr&, const DataPtr&) = 0;
    virtual void postTransform(const DataPtr&, const DataPtr&) = 0;
    virtual ObjectDataMap& getObjectMap() = 0;
    virtual bool purgeObjects() const = 0;
    virtual Slice::TypePtr getRename(const Slice::TypePtr&) const = 0;
};

class DataInitializer : virtual public IceUtil::SimpleShared
{
public:

    virtual ~DataInitializer();

    virtual void initialize(const DataFactoryPtr&, const DataPtr&, const Ice::CommunicatorPtr&) = 0;
};
typedef IceUtil::Handle<DataInitializer> DataInitializerPtr;

class DataFactory : public IceUtil::SimpleShared
{
public:

    DataFactory(const Ice::CommunicatorPtr&, const Slice::UnitPtr&, const ErrorReporterPtr&);

    DataPtr create(const Slice::TypePtr&, bool);
    DataPtr createBoolean(bool, bool);
    DataPtr createInteger(Ice::Long, bool);
    DataPtr createDouble(double, bool);
    DataPtr createString(const std::string&, bool);
    DataPtr createNil(bool);
    DataPtr createObject(const Slice::TypePtr&, bool);

    Slice::BuiltinPtr getBuiltin(Slice::Builtin::Kind) const;

    void addInitializer(const std::string&, const DataInitializerPtr&);
    void enableInitializers();
    void disableInitializers();
    ErrorReporterPtr getErrorReporter() const;

private:

    DataPtr createImpl(const Slice::TypePtr&, bool);
    void initialize(const DataPtr&);

    Ice::CommunicatorPtr _communicator;
    Slice::UnitPtr _unit; // Only used for creating builtin types.
    ErrorReporterPtr _errorReporter;
    typedef std::map<std::string, DataInitializerPtr> InitMap;
    InitMap _initializers;
    bool _initializersEnabled;
};

class Data : public IceUtil::SimpleShared
{
public:

    virtual ~Data();

    virtual DataPtr getMember(const std::string&) const = 0;
    virtual DataPtr getElement(const DataPtr&) const = 0;

    virtual bool operator==(const Data&) const = 0;
    virtual bool operator!=(const Data&) const = 0;
    virtual bool operator<(const Data&) const = 0;

    virtual void registerObjects(ObjectDataMap&) const = 0;
    virtual void transform(const DataPtr&, DataInterceptor&) = 0;
    virtual void set(const DataPtr&, DataInterceptor&, bool) = 0;
    virtual DataPtr clone() const = 0;

    bool readOnly() const;
    virtual Slice::TypePtr getType() const = 0;
    void print(std::ostream&) const;
    virtual void destroy() = 0;

    virtual void marshal(IceInternal::BasicStream&) const = 0;
    virtual void unmarshal(IceInternal::BasicStream&) = 0;

    virtual bool booleanValue(bool = false) const = 0;
    virtual Ice::Long integerValue(bool = false) const = 0;
    virtual double doubleValue(bool = false) const = 0;
    virtual std::string stringValue(bool = false) const = 0;
    virtual std::string toString() const = 0;

    virtual void printI(IceUtil::Output&, ObjectDataHistory&) const = 0;

protected:

    Data(const ErrorReporterPtr&, bool);

    bool isCompatible(const Slice::TypePtr&, const Slice::TypePtr&, DataInterceptor&);
    bool checkClasses(const Slice::ClassDeclPtr&, const Slice::ClassDeclPtr&, DataInterceptor&);

    ErrorReporterPtr _errorReporter;
    bool _readOnly;
};

class PrimitiveData : public Data
{
public:

    virtual DataPtr getMember(const std::string&) const;
    virtual DataPtr getElement(const DataPtr&) const;

    virtual void registerObjects(ObjectDataMap&) const;
    virtual void destroy();

protected:

    PrimitiveData(const ErrorReporterPtr&, bool);
};
typedef IceUtil::Handle<PrimitiveData> PrimitiveDataPtr;

class BooleanData : public PrimitiveData
{
public:

    BooleanData(const Slice::BuiltinPtr&, const ErrorReporterPtr&, bool);
    BooleanData(const Slice::BuiltinPtr&, const ErrorReporterPtr&, bool, bool);

    virtual bool operator==(const Data&) const;
    virtual bool operator!=(const Data&) const;
    virtual bool operator<(const Data&) const;

    virtual void transform(const DataPtr&, DataInterceptor&);
    virtual void set(const DataPtr&, DataInterceptor&, bool);
    virtual DataPtr clone() const;

    virtual Slice::TypePtr getType() const;

    virtual void marshal(IceInternal::BasicStream&) const;
    virtual void unmarshal(IceInternal::BasicStream&);

    virtual bool booleanValue(bool = false) const;
    virtual Ice::Long integerValue(bool = false) const;
    virtual double doubleValue(bool = false) const;
    virtual std::string stringValue(bool = false) const;
    virtual std::string toString() const;

    virtual void printI(IceUtil::Output&, ObjectDataHistory&) const;

private:

    Slice::BuiltinPtr _type;
    bool _value;
};
typedef IceUtil::Handle<BooleanData> BooleanDataPtr;

class IntegerData : public PrimitiveData
{
public:

    IntegerData(const Slice::BuiltinPtr&, const ErrorReporterPtr&, bool);
    IntegerData(const Slice::BuiltinPtr&, const ErrorReporterPtr&, bool, Ice::Long);

    virtual bool operator==(const Data&) const;
    virtual bool operator!=(const Data&) const;
    virtual bool operator<(const Data&) const;

    virtual void transform(const DataPtr&, DataInterceptor&);
    virtual void set(const DataPtr&, DataInterceptor&, bool);
    virtual DataPtr clone() const;

    virtual Slice::TypePtr getType() const;

    virtual void marshal(IceInternal::BasicStream&) const;
    virtual void unmarshal(IceInternal::BasicStream&);

    virtual bool booleanValue(bool = false) const;
    virtual Ice::Long integerValue(bool = false) const;
    virtual double doubleValue(bool = false) const;
    virtual std::string stringValue(bool = false) const;
    virtual std::string toString() const;

    virtual void printI(IceUtil::Output&, ObjectDataHistory&) const;

private:

    bool rangeCheck(Ice::Long) const;
    static std::string toString(Ice::Long);

    Slice::BuiltinPtr _type;
    Ice::Long _value;
};
typedef IceUtil::Handle<IntegerData> IntegerDataPtr;

class DoubleData : public PrimitiveData
{
public:

    DoubleData(const Slice::BuiltinPtr&, const ErrorReporterPtr&, bool);
    DoubleData(const Slice::BuiltinPtr&, const ErrorReporterPtr&, bool, double);

    virtual bool operator==(const Data&) const;
    virtual bool operator!=(const Data&) const;
    virtual bool operator<(const Data&) const;

    virtual void transform(const DataPtr&, DataInterceptor&);
    virtual void set(const DataPtr&, DataInterceptor&, bool);
    virtual DataPtr clone() const;

    virtual Slice::TypePtr getType() const;

    virtual void marshal(IceInternal::BasicStream&) const;
    virtual void unmarshal(IceInternal::BasicStream&);

    virtual bool booleanValue(bool = false) const;
    virtual Ice::Long integerValue(bool = false) const;
    virtual double doubleValue(bool = false) const;
    virtual std::string stringValue(bool = false) const;
    virtual std::string toString() const;

    virtual void printI(IceUtil::Output&, ObjectDataHistory&) const;

private:

    Slice::BuiltinPtr _type;
    double _value;
};
typedef IceUtil::Handle<DoubleData> DoubleDataPtr;

class StringData : public PrimitiveData
{
public:

    StringData(const DataFactoryPtr&, const Slice::BuiltinPtr&, const ErrorReporterPtr&, bool);
    StringData(const DataFactoryPtr&, const Slice::BuiltinPtr&, const ErrorReporterPtr&, bool, const std::string&);

    virtual DataPtr getMember(const std::string&) const;

    virtual bool operator==(const Data&) const;
    virtual bool operator!=(const Data&) const;
    virtual bool operator<(const Data&) const;

    virtual void transform(const DataPtr&, DataInterceptor&);
    virtual void set(const DataPtr&, DataInterceptor&, bool);
    virtual DataPtr clone() const;

    virtual Slice::TypePtr getType() const;

    virtual void marshal(IceInternal::BasicStream&) const;
    virtual void unmarshal(IceInternal::BasicStream&);

    virtual bool booleanValue(bool = false) const;
    virtual Ice::Long integerValue(bool = false) const;
    virtual double doubleValue(bool = false) const;
    virtual std::string stringValue(bool = false) const;
    virtual std::string toString() const;

    virtual void printI(IceUtil::Output&, ObjectDataHistory&) const;

private:

    void setValue(const std::string&);

    DataFactoryPtr _factory;
    Slice::BuiltinPtr _type;
    std::string _value;
    DataPtr _length;
};
typedef IceUtil::Handle<StringData> StringDataPtr;

class ProxyData : public Data
{
public:

    ProxyData(const Slice::TypePtr&, const Ice::CommunicatorPtr&, const ErrorReporterPtr&, bool);
    ProxyData(const Slice::TypePtr&, const Ice::CommunicatorPtr&, const ErrorReporterPtr&, bool, const Ice::ObjectPrx&);

    virtual DataPtr getMember(const std::string&) const;
    virtual DataPtr getElement(const DataPtr&) const;

    virtual bool operator==(const Data&) const;
    virtual bool operator!=(const Data&) const;
    virtual bool operator<(const Data&) const;

    virtual void registerObjects(ObjectDataMap&) const;
    virtual void transform(const DataPtr&, DataInterceptor&);
    virtual void set(const DataPtr&, DataInterceptor&, bool);
    virtual DataPtr clone() const;

    virtual Slice::TypePtr getType() const;
    virtual void destroy();

    virtual void marshal(IceInternal::BasicStream&) const;
    virtual void unmarshal(IceInternal::BasicStream&);

    virtual bool booleanValue(bool = false) const;
    virtual Ice::Long integerValue(bool = false) const;
    virtual double doubleValue(bool = false) const;
    virtual std::string stringValue(bool = false) const;
    virtual std::string toString() const;

    Ice::ObjectPrx getValue() const;

    virtual void printI(IceUtil::Output&, ObjectDataHistory&) const;

private:

    void transformI(const DataPtr&);
    void setValue(const std::string&);

    Slice::TypePtr _type; // Can be Builtin or Proxy
    Ice::CommunicatorPtr _communicator;
    Ice::ObjectPrx _value;
    std::string _str;
};
typedef IceUtil::Handle<ProxyData> ProxyDataPtr;

class StructData : public Data
{
public:

    StructData(const DataFactoryPtr&, const Slice::StructPtr&, const ErrorReporterPtr&, bool);

    virtual DataPtr getMember(const std::string&) const;
    virtual DataPtr getElement(const DataPtr&) const;

    virtual bool operator==(const Data&) const;
    virtual bool operator!=(const Data&) const;
    virtual bool operator<(const Data&) const;

    virtual void registerObjects(ObjectDataMap&) const;
    virtual void transform(const DataPtr&, DataInterceptor&);
    virtual void set(const DataPtr&, DataInterceptor&, bool);
    virtual DataPtr clone() const;

    virtual Slice::TypePtr getType() const;

    virtual void marshal(IceInternal::BasicStream&) const;
    virtual void unmarshal(IceInternal::BasicStream&);

    virtual void destroy();

    virtual bool booleanValue(bool = false) const;
    virtual Ice::Long integerValue(bool = false) const;
    virtual double doubleValue(bool = false) const;
    virtual std::string stringValue(bool = false) const;
    virtual std::string toString() const;

    DataMemberMap getMembers() const;

    virtual void printI(IceUtil::Output&, ObjectDataHistory&) const;

private:

    StructData(const Slice::StructPtr&, const ErrorReporterPtr&, bool, const DataMemberMap&);

    void transformI(const DataPtr&, DataInterceptor&);

    Slice::StructPtr _type;
    DataMemberMap _members;
};
typedef IceUtil::Handle<StructData> StructDataPtr;

class SequenceData : public Data
{
public:

    SequenceData(const DataFactoryPtr&, const Slice::SequencePtr&, const ErrorReporterPtr&, bool);

    virtual DataPtr getMember(const std::string&) const;
    virtual DataPtr getElement(const DataPtr&) const;

    virtual bool operator==(const Data&) const;
    virtual bool operator!=(const Data&) const;
    virtual bool operator<(const Data&) const;

    virtual void registerObjects(ObjectDataMap&) const;
    virtual void transform(const DataPtr&, DataInterceptor&);
    virtual void set(const DataPtr&, DataInterceptor&, bool);
    virtual DataPtr clone() const;

    virtual Slice::TypePtr getType() const;
    virtual void destroy();

    virtual void marshal(IceInternal::BasicStream&) const;
    virtual void unmarshal(IceInternal::BasicStream&);

    virtual bool booleanValue(bool = false) const;
    virtual Ice::Long integerValue(bool = false) const;
    virtual double doubleValue(bool = false) const;
    virtual std::string stringValue(bool = false) const;
    virtual std::string toString() const;

    DataList getElements() const;
    void resize(const DataPtr&, const DataPtr&, DataInterceptor&, bool);

    virtual void printI(IceUtil::Output&, ObjectDataHistory&) const;

private:

    SequenceData(const DataFactoryPtr&, const Slice::SequencePtr&, const ErrorReporterPtr&, bool, const DataList&);

    void transformI(const DataPtr&, DataInterceptor&);

    DataFactoryPtr _factory;
    Slice::SequencePtr _type;
    DataList _elements;
    DataPtr _length;
};
typedef IceUtil::Handle<SequenceData> SequenceDataPtr;

class EnumData : public Data
{
public:

    EnumData(const Slice::EnumPtr&, const ErrorReporterPtr&, bool);
    EnumData(const Slice::EnumPtr&, const ErrorReporterPtr&, bool, Ice::Int);

    virtual DataPtr getMember(const std::string&) const;
    virtual DataPtr getElement(const DataPtr&) const;

    virtual bool operator==(const Data&) const;
    virtual bool operator!=(const Data&) const;
    virtual bool operator<(const Data&) const;

    virtual void registerObjects(ObjectDataMap&) const;
    virtual void transform(const DataPtr&, DataInterceptor&);
    virtual void set(const DataPtr&, DataInterceptor&, bool);
    virtual DataPtr clone() const;

    virtual Slice::TypePtr getType() const;
    virtual void destroy();

    virtual void marshal(IceInternal::BasicStream&) const;
    virtual void unmarshal(IceInternal::BasicStream&);

    virtual bool booleanValue(bool = false) const;
    virtual Ice::Long integerValue(bool = false) const;
    virtual double doubleValue(bool = false) const;
    virtual std::string stringValue(bool = false) const;
    virtual std::string toString() const;

    Ice::Int getValue() const;

    bool setValue(Ice::Int);
    bool setValueAsString(const std::string&);

    virtual void printI(IceUtil::Output&, ObjectDataHistory&) const;

private:

    void transformI(const DataPtr&, DataInterceptor&);

    Slice::EnumPtr _type;
    Ice::Int _value;
    std::string _name;
    Ice::Int _count;
};
typedef IceUtil::Handle<EnumData> EnumDataPtr;

class DictionaryData : public Data
{
public:

    DictionaryData(const DataFactoryPtr&, const Slice::DictionaryPtr&, const ErrorReporterPtr&, bool);

    virtual DataPtr getMember(const std::string&) const;
    virtual DataPtr getElement(const DataPtr&) const;

    virtual bool operator==(const Data&) const;
    virtual bool operator!=(const Data&) const;
    virtual bool operator<(const Data&) const;

    virtual void registerObjects(ObjectDataMap&) const;
    virtual void transform(const DataPtr&, DataInterceptor&);
    virtual void set(const DataPtr&, DataInterceptor&, bool);
    virtual DataPtr clone() const;

    virtual Slice::TypePtr getType() const;
    virtual void destroy();

    virtual void marshal(IceInternal::BasicStream&) const;
    virtual void unmarshal(IceInternal::BasicStream&);

    virtual bool booleanValue(bool = false) const;
    virtual Ice::Long integerValue(bool = false) const;
    virtual double doubleValue(bool = false) const;
    virtual std::string stringValue(bool = false) const;
    virtual std::string toString() const;

    DataMap getElements() const;
    void add(const DataPtr&, const DataPtr&);
    void remove(const DataPtr&);

    virtual void printI(IceUtil::Output&, ObjectDataHistory&) const;

private:

    DictionaryData(const DataFactoryPtr&, const Slice::DictionaryPtr&, const ErrorReporterPtr&, bool, const DataMap&);

    void transformI(const DataPtr&, DataInterceptor&);

    DataFactoryPtr _factory;
    Slice::DictionaryPtr _type;
    DataMap _map;
    DataPtr _length;
};
typedef IceUtil::Handle<DictionaryData> DictionaryDataPtr;

class ObjectData : public Data
{
public:

    ObjectData(const DataFactoryPtr&, const Slice::TypePtr&, bool);

    virtual DataPtr getMember(const std::string&) const;
    virtual DataPtr getElement(const DataPtr&) const;

    virtual bool operator==(const Data&) const;
    virtual bool operator!=(const Data&) const;
    virtual bool operator<(const Data&) const;

    virtual void registerObjects(ObjectDataMap&) const;
    virtual void transform(const DataPtr&, DataInterceptor&);
    virtual void set(const DataPtr&, DataInterceptor&, bool);
    virtual DataPtr clone() const;

    virtual Slice::TypePtr getType() const;
    virtual void destroy();

    virtual void marshal(IceInternal::BasicStream&) const;
    virtual void unmarshal(IceInternal::BasicStream&);

    virtual bool booleanValue(bool = false) const;
    virtual Ice::Long integerValue(bool = false) const;
    virtual double doubleValue(bool = false) const;
    virtual std::string stringValue(bool = false) const;
    virtual std::string toString() const;

    void incRef();
    void decRef();

    virtual void printI(IceUtil::Output&, ObjectDataHistory&) const;

private:

    void destroyI();

    Slice::TypePtr _type; // Can be Builtin (Object) or ClassDecl
    DataPtr _facetMap;
    DataMemberMap _members;
    Ice::ObjectPtr _marshaler;
    Ice::Int _refCount;
    DataPtr _id;

    friend class ObjectWriter;
    friend class ObjectReader;
};

class ObjectRef : public Data
{
public:

    ObjectRef(const DataFactoryPtr&, const Slice::TypePtr&, bool);
    ObjectRef(const DataFactoryPtr&, const Slice::TypePtr&, bool, const ObjectDataPtr&);

    virtual DataPtr getMember(const std::string&) const;
    virtual DataPtr getElement(const DataPtr&) const;

    virtual bool operator==(const Data&) const;
    virtual bool operator!=(const Data&) const;
    virtual bool operator<(const Data&) const;

    virtual void registerObjects(ObjectDataMap&) const;
    virtual void transform(const DataPtr&, DataInterceptor&);
    virtual void set(const DataPtr&, DataInterceptor&, bool);
    virtual DataPtr clone() const;

    virtual Slice::TypePtr getType() const;
    virtual void destroy();

    virtual void marshal(IceInternal::BasicStream&) const;
    virtual void unmarshal(IceInternal::BasicStream&);

    virtual bool booleanValue(bool = false) const;
    virtual Ice::Long integerValue(bool = false) const;
    virtual double doubleValue(bool = false) const;
    virtual std::string stringValue(bool = false) const;
    virtual std::string toString() const;

    void instantiate();
    ObjectDataPtr getValue() const;
    void setValue(const ObjectDataPtr&);

    virtual void printI(IceUtil::Output&, ObjectDataHistory&) const;

private:

    void transformI(const DataPtr&, DataInterceptor&);

    DataFactoryPtr _factory;
    Slice::TypePtr _type; // Formal type: can be Builtin (Object) or ClassDecl
    ObjectDataPtr _value;
};
typedef IceUtil::Handle<ObjectRef> ObjectRefPtr;

class ObjectFactory : public Ice::ObjectFactory
{
public:

    ObjectFactory(const DataFactoryPtr&, const Slice::UnitPtr&);

    virtual Ice::ObjectPtr create(const std::string&);
    virtual void destroy();

private:

    DataFactoryPtr _factory;
    Slice::UnitPtr _unit;
};

class ClassNotFoundException
{
public:

    ClassNotFoundException(const std::string& s) : id(s) {}

    std::string id;
};

} // End of namespace Transform

#endif
