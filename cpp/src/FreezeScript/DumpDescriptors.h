// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef FREEZE_SCRIPT_DUMP_DESCRIPTORS_H
#define FREEZE_SCRIPT_DUMP_DESCRIPTORS_H

#include <FreezeScript/Parser.h>
#include <IceXML/Parser.h>

class Db;
class DbTxn;

namespace FreezeScript
{

class Descriptor;
typedef IceUtil::Handle<Descriptor> DescriptorPtr;

struct ExecuteInfo;

class Descriptor : virtual public IceUtil::SimpleShared
{
public:

    virtual ~Descriptor();

    virtual DescriptorPtr parent() const;
    virtual void addChild(const DescriptorPtr&) = 0;
    virtual void validate() = 0;
    virtual void execute(const SymbolTablePtr&, ExecuteInfo*) = 0;

protected:

    Descriptor(const DescriptorPtr&, int, const DataFactoryPtr&, const ErrorReporterPtr&);

    NodePtr parse(const std::string&) const;

    Slice::TypePtr findType(const Slice::UnitPtr&, const std::string&);

    DescriptorPtr _parent;
    int _line;
    DataFactoryPtr _factory;
    ErrorReporterPtr _errorReporter;
};

class SetDescriptor : public Descriptor
{
public:

    SetDescriptor(const DescriptorPtr&, int, const DataFactoryPtr&, const ErrorReporterPtr&,
                  const IceXML::Attributes&);

    virtual void addChild(const DescriptorPtr&);
    virtual void validate();
    virtual void execute(const SymbolTablePtr&, ExecuteInfo*);

private:

    EntityNodePtr _target;
    NodePtr _value;
    std::string _valueStr;
    NodePtr _length;
    std::string _lengthStr;
    bool _convert;
};

class DefineDescriptor : public Descriptor
{
public:

    DefineDescriptor(const DescriptorPtr&, int, const DataFactoryPtr&, const ErrorReporterPtr&,
                     const IceXML::Attributes&, const Slice::UnitPtr&);

    virtual void addChild(const DescriptorPtr&);
    virtual void validate();
    virtual void execute(const SymbolTablePtr&, ExecuteInfo*);

private:

    std::string _name;
    Slice::TypePtr _type;
    NodePtr _value;
    std::string _valueStr;
    bool _convert;
};

class AddDescriptor : public Descriptor
{
public:

    AddDescriptor(const DescriptorPtr&, int, const DataFactoryPtr&, const ErrorReporterPtr&,
                  const IceXML::Attributes&);

    virtual void addChild(const DescriptorPtr&);
    virtual void validate();
    virtual void execute(const SymbolTablePtr&, ExecuteInfo*);

private:

    EntityNodePtr _target;
    NodePtr _key;
    std::string _keyStr;
    NodePtr _index;
    std::string _indexStr;
    NodePtr _value;
    std::string _valueStr;
    bool _convert;
};

class RemoveDescriptor : public Descriptor
{
public:

    RemoveDescriptor(const DescriptorPtr&, int, const DataFactoryPtr&, const ErrorReporterPtr&,
                     const IceXML::Attributes&);

    virtual void addChild(const DescriptorPtr&);
    virtual void validate();
    virtual void execute(const SymbolTablePtr&, ExecuteInfo*);

private:

    EntityNodePtr _target;
    NodePtr _key;
    std::string _keyStr;
    NodePtr _index;
    std::string _indexStr;
};

class FailDescriptor : public Descriptor
{
public:

    FailDescriptor(const DescriptorPtr&, int, const DataFactoryPtr&, const ErrorReporterPtr&,
                   const IceXML::Attributes&);

    virtual void addChild(const DescriptorPtr&);
    virtual void validate();
    virtual void execute(const SymbolTablePtr&, ExecuteInfo*);

private:

    NodePtr _test;
    std::string _testStr;
    std::string _message;
};

class EchoDescriptor : public Descriptor
{
public:

    EchoDescriptor(const DescriptorPtr&, int, const DataFactoryPtr&, const ErrorReporterPtr&,
                   const IceXML::Attributes&, std::ostream&);

    virtual void addChild(const DescriptorPtr&);
    virtual void validate();
    virtual void execute(const SymbolTablePtr&, ExecuteInfo*);

private:

    std::ostream& _os;
    std::string _message;
    NodePtr _value;
    std::string _valueStr;
};

class ExecutableContainerDescriptor : virtual public Descriptor
{
public:

    ExecutableContainerDescriptor(const DescriptorPtr&, int, const DataFactoryPtr&, const ErrorReporterPtr&,
                                  const IceXML::Attributes&, const std::string&);

    virtual void addChild(const DescriptorPtr&);
    virtual void validate();
    virtual void execute(const SymbolTablePtr&, ExecuteInfo*);

protected:

    std::vector<DescriptorPtr> _children;

private:

    std::string _name;
};

class IfDescriptor : public ExecutableContainerDescriptor
{
public:

    IfDescriptor(const DescriptorPtr&, int, const DataFactoryPtr&, const ErrorReporterPtr&, const IceXML::Attributes&);

    virtual void execute(const SymbolTablePtr&, ExecuteInfo*);

private:

    NodePtr _test;
    std::string _testStr;
};

class IterateDescriptor : public ExecutableContainerDescriptor
{
public:

    IterateDescriptor(const DescriptorPtr&, int, const DataFactoryPtr&, const ErrorReporterPtr&,
                      const IceXML::Attributes&);

    virtual void execute(const SymbolTablePtr&, ExecuteInfo*);

private:

    EntityNodePtr _target;
    std::string _key;
    std::string _value;
    std::string _element;
    std::string _index;
};

class DumpDescriptor : public ExecutableContainerDescriptor
{
public:

    DumpDescriptor(const DescriptorPtr&, int, const DataFactoryPtr&, const ErrorReporterPtr&,
                   const IceXML::Attributes&, const Slice::UnitPtr&);

    Slice::TypePtr type() const;
    std::string typeName() const;
    bool base() const;
    bool contents() const;

private:

    Slice::TypePtr _type;
    bool _base;
    bool _contents;
};
typedef IceUtil::Handle<DumpDescriptor> DumpDescriptorPtr;

class RecordDescriptor : public ExecutableContainerDescriptor
{
public:

    RecordDescriptor(const DescriptorPtr&, int, const DataFactoryPtr&, const ErrorReporterPtr&,
                     const IceXML::Attributes&, const Slice::UnitPtr&, 
                     const FreezeScript::ObjectFactoryPtr&);

    virtual void execute(const SymbolTablePtr&, ExecuteInfo*);

private:

    Slice::UnitPtr _unit;
    FreezeScript::ObjectFactoryPtr _objectFactory;
};
typedef IceUtil::Handle<RecordDescriptor> RecordDescriptorPtr;

class DatabaseDescriptor : public ExecutableContainerDescriptor
{
public:

    DatabaseDescriptor(const DescriptorPtr&, int, const DataFactoryPtr&, const ErrorReporterPtr&,
                       const IceXML::Attributes&, const Slice::UnitPtr&);

    virtual void addChild(const DescriptorPtr&);
    virtual void execute(const SymbolTablePtr&, ExecuteInfo*);

private:

    Slice::UnitPtr _unit;
    Slice::TypePtr _key;
    Slice::TypePtr _value;
    RecordDescriptorPtr _record;
};
typedef IceUtil::Handle<DatabaseDescriptor> DatabaseDescriptorPtr;

class DumpDBDescriptor : public Descriptor
{
public:

    DumpDBDescriptor(int, const DataFactoryPtr&, const ErrorReporterPtr&, const IceXML::Attributes&,
                     const Slice::UnitPtr&);
    ~DumpDBDescriptor();

    virtual void addChild(const DescriptorPtr&);
    virtual void validate();
    virtual void execute(const SymbolTablePtr&, ExecuteInfo*);

    void dump(const Ice::CommunicatorPtr&, Db*, DbTxn*, const std::string&);

private:

    Slice::UnitPtr _unit;
    DatabaseDescriptorPtr _database;
    std::vector<DescriptorPtr> _children;
    ExecuteInfo* _info;
};
typedef IceUtil::Handle<DumpDBDescriptor> DumpDBDescriptorPtr;

} // End of namespace FreezeScript

#endif
