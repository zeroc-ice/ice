// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef FREEZE_SCRIPT_DATA_H
#define FREEZE_SCRIPT_DATA_H

#include <Ice/Ice.h>
#include <Slice/Parser.h>
#include <FreezeScript/Error.h>

namespace FreezeScript
{

class Data;
typedef IceUtil::Handle<Data> DataPtr;

class DataVisitor;

typedef std::map<std::string, DataPtr> DataMemberMap;
typedef std::vector<DataPtr> DataList;
typedef std::map<DataPtr, DataPtr> DataMap;

class ObjectWriter;
class ObjectReader;

class ObjectData;
typedef IceUtil::Handle<ObjectData> ObjectDataPtr;

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

    Ice::CommunicatorPtr getCommunicator() const;
    ErrorReporterPtr getErrorReporter() const;

protected:

    DataPtr createImpl(const Slice::TypePtr&, bool);

    virtual void initialize(const DataPtr&);

    Ice::CommunicatorPtr _communicator;
    Slice::UnitPtr _unit; // Only used for creating builtin types.
    ErrorReporterPtr _errorReporter;
};
typedef IceUtil::Handle<DataFactory> DataFactoryPtr;

class Data : public IceUtil::SimpleShared
{
public:

    virtual ~Data();

    virtual DataPtr getMember(const std::string&) const = 0;
    virtual DataPtr getElement(const DataPtr&) const = 0;

    virtual bool operator==(const Data&) const = 0;
    virtual bool operator!=(const Data&) const = 0;
    virtual bool operator<(const Data&) const = 0;

    virtual DataPtr clone() const = 0;

    bool readOnly() const;
    virtual Slice::TypePtr getType() const = 0;
    virtual void destroy() = 0;

    virtual void marshal(const Ice::OutputStreamPtr&) const = 0;
    virtual void unmarshal(const Ice::InputStreamPtr&) = 0;

    virtual bool booleanValue(bool = false) const = 0;
    virtual Ice::Long integerValue(bool = false) const = 0;
    virtual double doubleValue(bool = false) const = 0;
    virtual std::string stringValue(bool = false) const = 0;
    virtual std::string toString() const = 0;

    virtual void visit(DataVisitor&) = 0;

protected:

    Data(const ErrorReporterPtr&, bool);

    ErrorReporterPtr _errorReporter;
    bool _readOnly;
};

class PrimitiveData : public Data
{
public:

    virtual DataPtr getMember(const std::string&) const;
    virtual DataPtr getElement(const DataPtr&) const;

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

    virtual DataPtr clone() const;

    virtual Slice::TypePtr getType() const;

    virtual void marshal(const Ice::OutputStreamPtr&) const;
    virtual void unmarshal(const Ice::InputStreamPtr&);

    virtual bool booleanValue(bool = false) const;
    virtual Ice::Long integerValue(bool = false) const;
    virtual double doubleValue(bool = false) const;
    virtual std::string stringValue(bool = false) const;
    virtual std::string toString() const;

    virtual void visit(DataVisitor&);

    bool getValue() const;
    void setValue(bool);

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

    virtual DataPtr clone() const;

    virtual Slice::TypePtr getType() const;

    virtual void marshal(const Ice::OutputStreamPtr&) const;
    virtual void unmarshal(const Ice::InputStreamPtr&);

    virtual bool booleanValue(bool = false) const;
    virtual Ice::Long integerValue(bool = false) const;
    virtual double doubleValue(bool = false) const;
    virtual std::string stringValue(bool = false) const;
    virtual std::string toString() const;

    virtual void visit(DataVisitor&);

    Ice::Long getValue() const;
    void setValue(Ice::Long, bool);

private:

    bool rangeCheck(Ice::Long, bool) const;
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

    virtual DataPtr clone() const;

    virtual Slice::TypePtr getType() const;

    virtual void marshal(const Ice::OutputStreamPtr&) const;
    virtual void unmarshal(const Ice::InputStreamPtr&);

    virtual bool booleanValue(bool = false) const;
    virtual Ice::Long integerValue(bool = false) const;
    virtual double doubleValue(bool = false) const;
    virtual std::string stringValue(bool = false) const;
    virtual std::string toString() const;

    virtual void visit(DataVisitor&);

    double getValue() const;
    void setValue(double);

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

    virtual DataPtr clone() const;

    virtual Slice::TypePtr getType() const;

    virtual void marshal(const Ice::OutputStreamPtr&) const;
    virtual void unmarshal(const Ice::InputStreamPtr&);

    virtual bool booleanValue(bool = false) const;
    virtual Ice::Long integerValue(bool = false) const;
    virtual double doubleValue(bool = false) const;
    virtual std::string stringValue(bool = false) const;
    virtual std::string toString() const;

    virtual void visit(DataVisitor&);

    std::string getValue() const;
    void setValue(const std::string&);

private:

    DataFactoryPtr _factory;
    Slice::BuiltinPtr _type;
    std::string _value;
    IntegerDataPtr _length;
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

    virtual DataPtr clone() const;

    virtual Slice::TypePtr getType() const;
    virtual void destroy();

    virtual void marshal(const Ice::OutputStreamPtr&) const;
    virtual void unmarshal(const Ice::InputStreamPtr&);

    virtual bool booleanValue(bool = false) const;
    virtual Ice::Long integerValue(bool = false) const;
    virtual double doubleValue(bool = false) const;
    virtual std::string stringValue(bool = false) const;
    virtual std::string toString() const;

    virtual void visit(DataVisitor&);

    Ice::ObjectPrx getValue() const;
    void setValue(const std::string&, bool);
    void setValue(const Ice::ObjectPrx&);

private:

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

    virtual DataPtr clone() const;

    virtual Slice::TypePtr getType() const;

    virtual void marshal(const Ice::OutputStreamPtr&) const;
    virtual void unmarshal(const Ice::InputStreamPtr&);

    virtual void destroy();

    virtual bool booleanValue(bool = false) const;
    virtual Ice::Long integerValue(bool = false) const;
    virtual double doubleValue(bool = false) const;
    virtual std::string stringValue(bool = false) const;
    virtual std::string toString() const;

    virtual void visit(DataVisitor&);

    DataMemberMap& getMembers();

private:

    StructData(const Slice::StructPtr&, const ErrorReporterPtr&, bool, const DataMemberMap&);

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

    virtual DataPtr clone() const;

    virtual Slice::TypePtr getType() const;
    virtual void destroy();

    virtual void marshal(const Ice::OutputStreamPtr&) const;
    virtual void unmarshal(const Ice::InputStreamPtr&);

    virtual bool booleanValue(bool = false) const;
    virtual Ice::Long integerValue(bool = false) const;
    virtual double doubleValue(bool = false) const;
    virtual std::string stringValue(bool = false) const;
    virtual std::string toString() const;

    virtual void visit(DataVisitor&);

    DataList& getElements();

private:

    SequenceData(const DataFactoryPtr&, const Slice::SequencePtr&, const ErrorReporterPtr&, bool, const DataList&);

    DataFactoryPtr _factory;
    Slice::SequencePtr _type;
    DataList _elements;
    IntegerDataPtr _length;
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

    virtual DataPtr clone() const;

    virtual Slice::TypePtr getType() const;
    virtual void destroy();

    virtual void marshal(const Ice::OutputStreamPtr&) const;
    virtual void unmarshal(const Ice::InputStreamPtr&);

    virtual bool booleanValue(bool = false) const;
    virtual Ice::Long integerValue(bool = false) const;
    virtual double doubleValue(bool = false) const;
    virtual std::string stringValue(bool = false) const;
    virtual std::string toString() const;

    virtual void visit(DataVisitor&);

    Ice::Int getValue() const;
    bool setValue(Ice::Int);
    bool setValueAsString(const std::string&);

private:

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

    virtual DataPtr clone() const;

    virtual Slice::TypePtr getType() const;
    virtual void destroy();

    virtual void marshal(const Ice::OutputStreamPtr&) const;
    virtual void unmarshal(const Ice::InputStreamPtr&);

    virtual bool booleanValue(bool = false) const;
    virtual Ice::Long integerValue(bool = false) const;
    virtual double doubleValue(bool = false) const;
    virtual std::string stringValue(bool = false) const;
    virtual std::string toString() const;

    virtual void visit(DataVisitor&);

    DataMap& getElements();

private:

    DictionaryData(const DataFactoryPtr&, const Slice::DictionaryPtr&, const ErrorReporterPtr&, bool, const DataMap&);

    DataFactoryPtr _factory;
    Slice::DictionaryPtr _type;
    DataMap _map;
    IntegerDataPtr _length;
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

    virtual DataPtr clone() const;

    virtual Slice::TypePtr getType() const;
    virtual void destroy();

    virtual void marshal(const Ice::OutputStreamPtr&) const;
    virtual void unmarshal(const Ice::InputStreamPtr&);

    virtual bool booleanValue(bool = false) const;
    virtual Ice::Long integerValue(bool = false) const;
    virtual double doubleValue(bool = false) const;
    virtual std::string stringValue(bool = false) const;
    virtual std::string toString() const;

    virtual void visit(DataVisitor&);

    DataMemberMap& getMembers();

    void incRef();
    void decRef();

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

    virtual DataPtr clone() const;

    virtual Slice::TypePtr getType() const;
    virtual void destroy();

    virtual void marshal(const Ice::OutputStreamPtr&) const;
    virtual void unmarshal(const Ice::InputStreamPtr&);

    virtual bool booleanValue(bool = false) const;
    virtual Ice::Long integerValue(bool = false) const;
    virtual double doubleValue(bool = false) const;
    virtual std::string stringValue(bool = false) const;
    virtual std::string toString() const;

    virtual void visit(DataVisitor&);

    void instantiate();
    ObjectDataPtr getValue() const;
    void setValue(const ObjectDataPtr&);

private:

    DataFactoryPtr _factory;
    Slice::TypePtr _type; // Formal type: can be Builtin (Object) or ClassDecl
    ObjectDataPtr _value;
};
typedef IceUtil::Handle<ObjectRef> ObjectRefPtr;

class ObjectFactory : public Ice::ObjectFactory
{
public:

    virtual Ice::ObjectPtr create(const std::string&);
    virtual void destroy();

    void activate(const DataFactoryPtr&, const Slice::UnitPtr&);
    void deactivate();

private:

    DataFactoryPtr _factory;
    Slice::UnitPtr _unit;
};
typedef IceUtil::Handle<ObjectFactory> ObjectFactoryPtr;

class ClassNotFoundException
{
public:

    ClassNotFoundException(const std::string& s) : id(s) {}

    std::string id;
};

class DataVisitor
{
public:

    virtual ~DataVisitor();

    virtual void visitBoolean(const BooleanDataPtr&);
    virtual void visitInteger(const IntegerDataPtr&);
    virtual void visitDouble(const DoubleDataPtr&);
    virtual void visitString(const StringDataPtr&);
    virtual void visitProxy(const ProxyDataPtr&);
    virtual void visitStruct(const StructDataPtr&);
    virtual void visitSequence(const SequenceDataPtr&);
    virtual void visitEnum(const EnumDataPtr&);
    virtual void visitDictionary(const DictionaryDataPtr&);
    virtual void visitObject(const ObjectRefPtr&);
};

} // End of namespace FreezeScript

#endif
