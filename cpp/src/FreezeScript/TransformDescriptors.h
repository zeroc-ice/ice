// **********************************************************************
//
// Copyright (c) 2004
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

#ifndef FREEZE_SCRIPT_TRANSFORM_DESCRIPTORS_H
#define FREEZE_SCRIPT_TRANSFORM_DESCRIPTORS_H

#include <FreezeScript/Parser.h>
#include <FreezeScript/TransformVisitor.h>
#include <IceXML/Parser.h>

class Db;
class DbTxn;

namespace FreezeScript
{

class Descriptor;
typedef IceUtil::Handle<Descriptor> DescriptorPtr;

class DeleteRecordException {};

class Descriptor : virtual public IceUtil::SimpleShared
{
public:

    virtual ~Descriptor();

    virtual DescriptorPtr parent() const;
    virtual void addChild(const DescriptorPtr&) = 0;
    virtual void validate() = 0;
    virtual void execute(const SymbolTablePtr&, TransformInfo*) = 0;

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
    virtual void execute(const SymbolTablePtr&, TransformInfo*);

private:

    EntityNodePtr _target;
    NodePtr _value;
    std::string _valueStr;
    std::string _type;
    NodePtr _length;
    std::string _lengthStr;
    bool _convert;
};

class DefineDescriptor : public Descriptor
{
public:

    DefineDescriptor(const DescriptorPtr&, int, const DataFactoryPtr&, const ErrorReporterPtr&,
                     const IceXML::Attributes&, const Slice::UnitPtr&, const Slice::UnitPtr&);

    virtual void addChild(const DescriptorPtr&);
    virtual void validate();
    virtual void execute(const SymbolTablePtr&, TransformInfo*);

private:

    std::string _name;
    NodePtr _value;
    std::string _valueStr;
    Slice::TypePtr _type;
    NodePtr _length;
    std::string _lengthStr;
    bool _convert;
};

class AddDescriptor : public Descriptor
{
public:

    AddDescriptor(const DescriptorPtr&, int, const DataFactoryPtr&, const ErrorReporterPtr&,
                  const IceXML::Attributes&);

    virtual void addChild(const DescriptorPtr&);
    virtual void validate();
    virtual void execute(const SymbolTablePtr&, TransformInfo*);

private:

    EntityNodePtr _target;
    NodePtr _key;
    std::string _keyStr;
    NodePtr _value;
    std::string _valueStr;
    std::string _type;
    bool _convert;
};

class RemoveDescriptor : public Descriptor
{
public:

    RemoveDescriptor(const DescriptorPtr&, int, const DataFactoryPtr&, const ErrorReporterPtr&,
                     const IceXML::Attributes&);

    virtual void addChild(const DescriptorPtr&);
    virtual void validate();
    virtual void execute(const SymbolTablePtr&, TransformInfo*);

private:

    EntityNodePtr _target;
    NodePtr _key;
    std::string _keyStr;
};

class DeleteDescriptor : public Descriptor
{
public:

    DeleteDescriptor(const DescriptorPtr&, int, const DataFactoryPtr&, const ErrorReporterPtr&,
                     const IceXML::Attributes&);

    virtual void addChild(const DescriptorPtr&);
    virtual void validate();
    virtual void execute(const SymbolTablePtr&, TransformInfo*);
};

class FailDescriptor : public Descriptor
{
public:

    FailDescriptor(const DescriptorPtr&, int, const DataFactoryPtr&, const ErrorReporterPtr&,
                   const IceXML::Attributes&);

    virtual void addChild(const DescriptorPtr&);
    virtual void validate();
    virtual void execute(const SymbolTablePtr&, TransformInfo*);

private:

    NodePtr _test;
    std::string _testStr;
    std::string _message;
};

class EchoDescriptor : public Descriptor
{
public:

    EchoDescriptor(const DescriptorPtr&, int, const DataFactoryPtr&, const ErrorReporterPtr&,
                   const IceXML::Attributes&);

    virtual void addChild(const DescriptorPtr&);
    virtual void validate();
    virtual void execute(const SymbolTablePtr&, TransformInfo*);

private:

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
    virtual void execute(const SymbolTablePtr&, TransformInfo*);

protected:

    std::vector<DescriptorPtr> _children;

private:

    std::string _name;
};

class IfDescriptor : public ExecutableContainerDescriptor
{
public:

    IfDescriptor(const DescriptorPtr&, int, const DataFactoryPtr&, const ErrorReporterPtr&, const IceXML::Attributes&);

    virtual void execute(const SymbolTablePtr&, TransformInfo*);

private:

    NodePtr _test;
    std::string _testStr;
};

class IterateDescriptor : public ExecutableContainerDescriptor
{
public:

    IterateDescriptor(const DescriptorPtr&, int, const DataFactoryPtr&, const ErrorReporterPtr&,
                      const IceXML::Attributes&);

    virtual void execute(const SymbolTablePtr&, TransformInfo*);

private:

    EntityNodePtr _target;
    std::string _key;
    std::string _value;
    std::string _element;
    std::string _index;
};

class TransformDescriptor : public ExecutableContainerDescriptor
{
public:

    TransformDescriptor(const DescriptorPtr&, int, const DataFactoryPtr&, const ErrorReporterPtr&,
                        const IceXML::Attributes&, const Slice::UnitPtr&, const Slice::UnitPtr&);

    Slice::TypePtr type() const;
    std::string typeName() const;
    Slice::TypePtr renameType() const;
    std::string renameTypeName() const;
    bool doDefaultTransform() const;
    bool doBaseTransform() const;

private:

    bool _default;
    bool _base;
    Slice::TypePtr _newType;
    Slice::TypePtr _renameType;
};
typedef IceUtil::Handle<TransformDescriptor> TransformDescriptorPtr;

class InitDescriptor : virtual public ExecutableContainerDescriptor, virtual public DataInitializer
{
public:

    InitDescriptor(const DescriptorPtr&, int, const DataFactoryPtr&, const ErrorReporterPtr&,
                   const IceXML::Attributes&, const Slice::UnitPtr&, const Slice::UnitPtr&);

    virtual void initialize(const DataFactoryPtr&, const DataPtr&, const Ice::CommunicatorPtr&);

    std::string typeName() const;

private:

    Slice::UnitPtr _old;
    Slice::UnitPtr _new;
    Slice::TypePtr _type;
};
typedef IceUtil::Handle<InitDescriptor> InitDescriptorPtr;

class RecordDescriptor : public ExecutableContainerDescriptor
{
public:

    RecordDescriptor(const DescriptorPtr&, int, const DataFactoryPtr&, const ErrorReporterPtr&,
                     const IceXML::Attributes&, const Slice::UnitPtr&, const Slice::UnitPtr&);

    virtual void execute(const SymbolTablePtr&, TransformInfo*);

private:

    void transformRecord(IceInternal::BasicStream&, IceInternal::BasicStream&, IceInternal::BasicStream&,
                         IceInternal::BasicStream&, TransformInfo*);

    Slice::UnitPtr _old;
    Slice::UnitPtr _new;
};
typedef IceUtil::Handle<RecordDescriptor> RecordDescriptorPtr;

class DatabaseDescriptor : public ExecutableContainerDescriptor
{
public:

    DatabaseDescriptor(const DescriptorPtr&, int, const DataFactoryPtr&, const ErrorReporterPtr&,
                       const IceXML::Attributes&, const Slice::UnitPtr&, const Slice::UnitPtr&);

    virtual void addChild(const DescriptorPtr&);
    virtual void execute(const SymbolTablePtr&, TransformInfo*);

private:

    Slice::UnitPtr _old;
    Slice::UnitPtr _new;
    Slice::TypePtr _oldKey;
    Slice::TypePtr _newKey;
    Slice::TypePtr _oldValue;
    Slice::TypePtr _newValue;
    RecordDescriptorPtr _record;
};
typedef IceUtil::Handle<DatabaseDescriptor> DatabaseDescriptorPtr;

class TransformInfoI;

class TransformDBDescriptor : public Descriptor
{
public:

    TransformDBDescriptor(int, const DataFactoryPtr&, const ErrorReporterPtr&, const IceXML::Attributes&,
                          const Slice::UnitPtr&, const Slice::UnitPtr&);
    ~TransformDBDescriptor();

    virtual void addChild(const DescriptorPtr&);
    virtual void validate();
    virtual void execute(const SymbolTablePtr&, TransformInfo*);

    void transform(const Ice::CommunicatorPtr&, Db*, DbTxn*, Db*, DbTxn*, bool);

private:

    Slice::UnitPtr _old;
    Slice::UnitPtr _new;
    DatabaseDescriptorPtr _database;
    std::vector<DescriptorPtr> _children;
    TransformInfoI* _info;
};
typedef IceUtil::Handle<TransformDBDescriptor> TransformDBDescriptorPtr;

} // End of namespace FreezeScript

#endif
