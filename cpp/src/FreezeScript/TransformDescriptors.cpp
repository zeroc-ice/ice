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

#include <FreezeScript/TransformDescriptors.h>
#include <FreezeScript/AssignVisitor.h>
#include <FreezeScript/Print.h>
#include <FreezeScript/Functions.h>
#include <FreezeScript/Exception.h>
#include <FreezeScript/Util.h>
#include <db_cxx.h>

using namespace std;

namespace FreezeScript
{

class SymbolTableI;
typedef IceUtil::Handle<SymbolTableI> SymbolTableIPtr;

class SymbolTableI : public SymbolTable
{
public:

    SymbolTableI(const DataFactoryPtr&, const Slice::UnitPtr&, const Slice::UnitPtr&, const ErrorReporterPtr&,
                 TransformInfo*, const SymbolTablePtr& = SymbolTablePtr());

    virtual void add(const string&, const DataPtr&);

    virtual DataPtr getValue(const EntityNodePtr&) const;

    virtual DataPtr getConstantValue(const string&) const;

    virtual SymbolTablePtr createChild();

    virtual Slice::TypePtr lookupType(const string&);

    virtual DataPtr invokeFunction(const string&, const DataPtr&, const DataList&);

private:

    DataPtr findValue(const string&) const;

    class EntityVisitor : public EntityNodeVisitor
    {
    public:

        EntityVisitor(SymbolTableI*);

        virtual void visitIdentifier(const string&);
        virtual void visitElement(const NodePtr&);

        DataPtr getCurrent() const;

    private:

        SymbolTableI* _table;
        DataPtr _current;
        bool _error;
    };

    friend class EntityVisitor;

    DataFactoryPtr _factory;
    Slice::UnitPtr _old;
    Slice::UnitPtr _new;
    ErrorReporterPtr _errorReporter;
    TransformInfo* _info;
    SymbolTablePtr _parent;
    typedef map<string, DataPtr> DataMap;
    DataMap _dataMap;
    DataMap _constantCache;
};

class ObjectVisitor : public DataVisitor
{
public:

    ObjectVisitor(ObjectDataMap&);

    virtual void visitStruct(const StructDataPtr&);
    virtual void visitSequence(const SequenceDataPtr&);
    virtual void visitDictionary(const DictionaryDataPtr&);
    virtual void visitObject(const ObjectRefPtr&);

private:

    ObjectDataMap& _map;
};

typedef map<string, TransformDescriptorPtr> TransformMap;
typedef map<string, Slice::TypePtr> RenameMap;

class TransformInfoI : public TransformInfo
{
public:

    TransformInfoI(const DataFactoryPtr&, const ErrorReporterPtr&, const Slice::UnitPtr&, const Slice::UnitPtr&);

    virtual bool doDefaultTransform(const Slice::TypePtr&);
    virtual bool doBaseTransform(const Slice::ClassDefPtr&);
    virtual Slice::TypePtr getRenamedType(const Slice::TypePtr&);
    virtual void executeCustomTransform(const DataPtr&, const DataPtr&);
    virtual bool purgeObjects();
    virtual ObjectDataMap& getObjectDataMap();

    DataFactoryPtr factory;
    ErrorReporterPtr errorReporter;
    Slice::UnitPtr oldUnit;
    Slice::UnitPtr newUnit;
    SymbolTablePtr symbolTable;
    TransformMap transformMap;
    RenameMap renameMap;
    ObjectDataMap objectDataMap;
    Slice::TypePtr oldKeyType;
    Slice::TypePtr oldValueType;
    Slice::TypePtr newKeyType;
    Slice::TypePtr newValueType;
    Ice::CommunicatorPtr communicator;
    bool purge;
    Db* oldDb;
    DbTxn* oldDbTxn;
    Db* newDb;
    DbTxn* newDbTxn;
};

void assignOrTransform(const DataPtr&, const DataPtr&, bool, const DataFactoryPtr&, const ErrorReporterPtr&,
                       TransformInfo*);

} // End of namespace FreezeScript

//
// Descriptor
//
FreezeScript::Descriptor::Descriptor(const DescriptorPtr& parent, int line, const DataFactoryPtr& factory,
                                     const ErrorReporterPtr& errorReporter) :
    _parent(parent), _line(line), _factory(factory), _errorReporter(errorReporter)
{
}

FreezeScript::Descriptor::~Descriptor()
{
}

FreezeScript::DescriptorPtr
FreezeScript::Descriptor::parent() const
{
    return _parent;
}

FreezeScript::NodePtr
FreezeScript::Descriptor::parse(const string& expr) const
{
    return parseExpression(expr, _factory, _errorReporter);
}

Slice::TypePtr
FreezeScript::Descriptor::findType(const Slice::UnitPtr& u, const string& type)
{
    Slice::TypeList l;

    l = u->lookupType(type, false);
    if(l.empty())
    {
        _errorReporter->error("unknown type `" + type + "'");
    }

    return l.front();
}

//
// SetDescriptor
//
FreezeScript::SetDescriptor::SetDescriptor(const DescriptorPtr& parent, int line, const DataFactoryPtr& factory,
                                           const ErrorReporterPtr& errorReporter,
                                           const IceXML::Attributes& attributes) :
    Descriptor(parent, line, factory, errorReporter)
{
    DescriptorErrorContext ctx(_errorReporter, "set", _line);

    IceXML::Attributes::const_iterator p;

    string target;
    p = attributes.find("target");
    if(p == attributes.end())
    {
        _errorReporter->error("required attribute `target' is missing");
    }
    target = p->second;

    p = attributes.find("value");
    if(p != attributes.end())
    {
        _valueStr = p->second;
    }

    p = attributes.find("type");
    if(p != attributes.end())
    {
        _type = p->second;
    }

    p = attributes.find("length");
    if(p != attributes.end())
    {
        _lengthStr = p->second;
    }

    p = attributes.find("convert");
    if(p != attributes.end())
    {
        _convert = p->second == "true";
    }

    if(!_valueStr.empty() && !_type.empty())
    {
        _errorReporter->error("attributes `value' and 'type' are mutually exclusive");
    }

    if(_valueStr.empty() && _type.empty() && _lengthStr.empty())
    {
        _errorReporter->error("requires a value for attributes `value', 'type' or 'length'");
    }

    NodePtr node = parse(target);
    _target = EntityNodePtr::dynamicCast(node);
    if(!_target)
    {
        _errorReporter->error("`target' attribute is not an entity: `" + target + "'");
    }

    if(!_valueStr.empty())
    {
        _value = parse(_valueStr);
    }

    if(!_lengthStr.empty())
    {
        _length = parse(_lengthStr);
    }
}

void
FreezeScript::SetDescriptor::addChild(const DescriptorPtr&)
{
    DescriptorErrorContext ctx(_errorReporter, "set", _line);
    _errorReporter->error("child elements are not supported");
}

void
FreezeScript::SetDescriptor::validate()
{
}

void
FreezeScript::SetDescriptor::execute(const SymbolTablePtr& sym, TransformInfo* info)
{
    DescriptorErrorContext ctx(_errorReporter, "set", _line);

    DataPtr data = sym->getValue(_target);
    if(data->readOnly())
    {
        ostringstream ostr;
        ostr << _target;
        _errorReporter->error("target `" + ostr.str() + "' cannot be modified");
    }

    DataPtr value;
    if(_value)
    {
        try
        {
            value = _value->evaluate(sym);
        }
        catch(const EvaluateException& ex)
        {
            _errorReporter->error("evaluation of value `" + _valueStr + "' failed:\n" + ex.reason());
        }
    }

    Destroyer<DataPtr> valueDestroyer;
    if(!_type.empty())
    {
        assert(!value);
        Slice::TypePtr type = sym->lookupType(_type);
        if(!type)
        {
            _errorReporter->error("type `" + _type + "' not found");
        }
        value = _factory->createObject(type, false);
        valueDestroyer.set(value);
    }

    DataPtr length;
    if(_length)
    {
        SequenceDataPtr seq = SequenceDataPtr::dynamicCast(data);
        if(!seq)
        {
            ostringstream ostr;
            ostr << _target;
            _errorReporter->error("target `" + ostr.str() + "' is not a sequence");
        }

        try
        {
            length = _length->evaluate(sym);
        }
        catch(const EvaluateException& ex)
        {
            _errorReporter->error("evaluation of length `" + _lengthStr + "' failed:\n" + ex.reason());
        }

        DataList& elements = seq->getElements();
        Ice::Long l = length->integerValue();
        if(l < 0 || l > INT_MAX)
        {
            _errorReporter->error("sequence length " + length->toString() + " is out of range");
        }

        DataList::size_type len = static_cast<DataList::size_type>(l);
        if(len < elements.size())
        {
            for(DataList::size_type i = len; i < elements.size(); ++i)
            {
                elements[i]->destroy();
            }
            elements.resize(len);
        }
        else if(len > elements.size())
        {
            Slice::SequencePtr seqType = Slice::SequencePtr::dynamicCast(seq->getType());
            assert(seqType);
            Slice::TypePtr elemType = seqType->type();
            for(DataList::size_type i = elements.size(); i < len; ++i)
            {
                DataPtr v = _factory->create(elemType, false);
                if(value)
                {
                    assignOrTransform(v, value, _convert, _factory, _errorReporter, info);
                }
                elements.push_back(v);
            }
        }
    }
    else
    {
        assignOrTransform(data, value, _convert, _factory, _errorReporter, info);
    }
}

//
// DefineDescriptor
//
FreezeScript::DefineDescriptor::DefineDescriptor(const DescriptorPtr& parent, int line, const DataFactoryPtr& factory,
                                                 const ErrorReporterPtr& errorReporter,
                                                 const IceXML::Attributes& attributes, const Slice::UnitPtr& oldUnit,
                                                 const Slice::UnitPtr& newUnit) :
    Descriptor(parent, line, factory, errorReporter)
{
    DescriptorErrorContext ctx(_errorReporter, "define", _line);

    IceXML::Attributes::const_iterator p;

    p = attributes.find("name");
    if(p == attributes.end())
    {
        _errorReporter->error("required attribute `name' is missing");
    }
    _name = p->second;

    p = attributes.find("type");
    if(p == attributes.end())
    {
        _errorReporter->error("required attribute `type' is missing");
    }
    if(p->second.find("::New") != string::npos)
    {
        Slice::TypeList l = newUnit->lookupType(p->second.substr(5), false);
        if(l.empty())
        {
            _errorReporter->error("type `" + p->second + "' not found");
        }
        _type = l.front();
    }
    else if(p->second.find("::Old") != string::npos)
    {
        Slice::TypeList l = oldUnit->lookupType(p->second.substr(5), false);
        if(l.empty())
        {
            _errorReporter->error("type `" + p->second + "' not found");
        }
        _type = l.front();
    }
    else
    {
        Slice::TypeList l = newUnit->lookupType(p->second, false);
        if(l.empty())
        {
            _errorReporter->error("type `" + p->second + "' not found");
        }
        _type = l.front();
    }

    p = attributes.find("value");
    if(p != attributes.end())
    {
        _valueStr = p->second;
    }

    p = attributes.find("convert");
    if(p != attributes.end())
    {
        _convert = p->second == "true";
    }

    if(!_valueStr.empty())
    {
        _value = parse(_valueStr);
    }
}

void
FreezeScript::DefineDescriptor::addChild(const DescriptorPtr&)
{
    DescriptorErrorContext ctx(_errorReporter, "define", _line);
    _errorReporter->error("child elements are not supported");
}

void
FreezeScript::DefineDescriptor::validate()
{
}

void
FreezeScript::DefineDescriptor::execute(const SymbolTablePtr& sym, TransformInfo* info)
{
    DescriptorErrorContext ctx(_errorReporter, "define", _line);

    DataPtr data = _factory->create(_type, false);

    DataPtr value;
    if(_value)
    {
        try
        {
            value = _value->evaluate(sym);
        }
        catch(const EvaluateException& ex)
        {
            _errorReporter->error("evaluation of value `" + _valueStr + "' failed:\n" + ex.reason());
        }
    }

    if(value)
    {
        assignOrTransform(data, value, _convert, _factory, _errorReporter, info);
    }

    sym->add(_name, data);
}

//
// AddDescriptor
//
FreezeScript::AddDescriptor::AddDescriptor(const DescriptorPtr& parent, int line, const DataFactoryPtr& factory,
                                           const ErrorReporterPtr& errorReporter,
                                           const IceXML::Attributes& attributes) :
    Descriptor(parent, line, factory, errorReporter)
{
    DescriptorErrorContext ctx(_errorReporter, "add", _line);

    IceXML::Attributes::const_iterator p;

    string target;
    p = attributes.find("target");
    if(p == attributes.end())
    {
        _errorReporter->error("required attribute `target' is missing");
    }
    target = p->second;

    p = attributes.find("key");
    if(p == attributes.end())
    {
        _errorReporter->error("required attribute `key' is missing");
    }
    _keyStr = p->second;

    p = attributes.find("value");
    if(p != attributes.end())
    {
        _valueStr = p->second;
    }

    p = attributes.find("type");
    if(p != attributes.end())
    {
        _type = p->second;
    }

    p = attributes.find("convert");
    if(p != attributes.end())
    {
        _convert = p->second == "true";
    }

    if(!_valueStr.empty() && !_type.empty())
    {
        _errorReporter->error("attributes `value' and 'type' are mutually exclusive");
    }

    NodePtr node = parse(target);
    _target = EntityNodePtr::dynamicCast(node);
    if(!_target)
    {
        _errorReporter->error("`target' attribute is not an entity: `" + target + "'");
    }

    assert(!_keyStr.empty());
    _key = parse(_keyStr);

    if(!_valueStr.empty())
    {
        _value = parse(_valueStr);
    }
}

void
FreezeScript::AddDescriptor::addChild(const DescriptorPtr&)
{
    DescriptorErrorContext ctx(_errorReporter, "add", _line);
    _errorReporter->error("child elements are not supported");
}

void
FreezeScript::AddDescriptor::validate()
{
}

void
FreezeScript::AddDescriptor::execute(const SymbolTablePtr& sym, TransformInfo* info)
{
    DescriptorErrorContext ctx(_errorReporter, "add", _line);

    DataPtr data = sym->getValue(_target);
    if(data->readOnly())
    {
        ostringstream ostr;
        ostr << _target;
        _errorReporter->error("target `" + ostr.str() + "' cannot be modified");
    }

    DictionaryDataPtr dict = DictionaryDataPtr::dynamicCast(data);
    if(!dict)
    {
        ostringstream ostr;
        ostr << _target;
        _errorReporter->error("target `" + ostr.str() + "' is not a dictionary");
    }

    Slice::DictionaryPtr type = Slice::DictionaryPtr::dynamicCast(dict->getType());
    assert(type);

    DataPtr key;
    Destroyer<DataPtr> keyDestroyer;
    try
    {
        DataPtr v = _key->evaluate(sym);
        key = _factory->create(type->keyType(), false);
        keyDestroyer.set(key);
        assignOrTransform(key, v, _convert, _factory, _errorReporter, info);
    }
    catch(const EvaluateException& ex)
    {
        _errorReporter->error("evaluation of key `" + _keyStr + "' failed:\n" + ex.reason());
    }

    if(dict->getElement(key))
    {
        ostringstream ostr;
        printData(key, ostr);
        _errorReporter->error("key " + ostr.str() + " already exists in dictionary");
    }

    DataPtr elem = _factory->create(type->valueType(), false);
    Destroyer<DataPtr> elemDestroyer(elem);

    DataPtr value;
    if(_value)
    {
        try
        {
            value = _value->evaluate(sym);
        }
        catch(const EvaluateException& ex)
        {
            _errorReporter->error("evaluation of value `" + _valueStr + "' failed:\n" + ex.reason());
        }
    }

    Destroyer<DataPtr> valueDestroyer;
    if(!_type.empty())
    {
        assert(!value);
        Slice::TypePtr type = sym->lookupType(_type);
        if(!type)
        {
            _errorReporter->error("type `" + _type + "' not found");
        }
        value = _factory->createObject(type, false);
        valueDestroyer.set(value);
    }

    if(value)
    {
        assignOrTransform(elem, value, _convert, _factory, _errorReporter, info);
    }
    DataMap& map = dict->getElements();
    map.insert(DataMap::value_type(key, elem));
    keyDestroyer.release();
    elemDestroyer.release();
}

//
// RemoveDescriptor
//
FreezeScript::RemoveDescriptor::RemoveDescriptor(const DescriptorPtr& parent, int line, const DataFactoryPtr& factory,
                                                 const ErrorReporterPtr& errorReporter,
                                                 const IceXML::Attributes& attributes) :
    Descriptor(parent, line, factory, errorReporter)
{
    DescriptorErrorContext ctx(_errorReporter, "remove", _line);

    IceXML::Attributes::const_iterator p;

    string target;
    p = attributes.find("target");
    if(p == attributes.end())
    {
        _errorReporter->error("required attribute `target' is missing");
    }
    target = p->second;

    p = attributes.find("key");
    if(p == attributes.end())
    {
        _errorReporter->error("required attribute `key' is missing");
    }
    _keyStr = p->second;

    NodePtr node = parse(target);
    _target = EntityNodePtr::dynamicCast(node);
    if(!_target)
    {
        _errorReporter->error("`target' attribute is not an entity: `" + target + "'");
    }

    _key = parse(_keyStr);
}

void
FreezeScript::RemoveDescriptor::addChild(const DescriptorPtr&)
{
    DescriptorErrorContext ctx(_errorReporter, "remove", _line);
    _errorReporter->error("child elements are not supported");
}

void
FreezeScript::RemoveDescriptor::validate()
{
}

void
FreezeScript::RemoveDescriptor::execute(const SymbolTablePtr& sym, TransformInfo*)
{
    DescriptorErrorContext ctx(_errorReporter, "remove", _line);

    DataPtr key;
    try
    {
        key = _key->evaluate(sym);
    }
    catch(const EvaluateException& ex)
    {
        _errorReporter->error("evaluation of key `" + _keyStr + "' failed:\n" + ex.reason());
    }

    DataPtr data = sym->getValue(_target);
    if(data->readOnly())
    {
        ostringstream ostr;
        ostr << _target;
        _errorReporter->error("target `" + ostr.str() + "' cannot be modified");
    }

    DictionaryDataPtr dict = DictionaryDataPtr::dynamicCast(data);
    if(!dict)
    {
        ostringstream ostr;
        ostr << _target;
        _errorReporter->error("target `" + ostr.str() + "' is not a dictionary");
    }

    DataMap& map = dict->getElements();
    DataMap::iterator p = map.find(key);
    if(p != map.end())
    {
        p->first->destroy();
        p->second->destroy();
        map.erase(p);
    }
}

//
// DeleteDescriptor
//
FreezeScript::DeleteDescriptor::DeleteDescriptor(const DescriptorPtr& parent, int line, const DataFactoryPtr& factory,
                                                 const ErrorReporterPtr& errorReporter, const IceXML::Attributes&) :
    Descriptor(parent, line, factory, errorReporter)
{
}

void
FreezeScript::DeleteDescriptor::addChild(const DescriptorPtr&)
{
    DescriptorErrorContext ctx(_errorReporter, "delete", _line);
    _errorReporter->error("child elements are not supported");
}

void
FreezeScript::DeleteDescriptor::validate()
{
}

void
FreezeScript::DeleteDescriptor::execute(const SymbolTablePtr&, TransformInfo*)
{
    throw DeleteRecordException();
}

//
// FailDescriptor
//
FreezeScript::FailDescriptor::FailDescriptor(const DescriptorPtr& parent, int line, const DataFactoryPtr& factory,
                                             const ErrorReporterPtr& errorReporter,
                                             const IceXML::Attributes& attributes) :
    Descriptor(parent, line, factory, errorReporter)
{
    IceXML::Attributes::const_iterator p;

    p = attributes.find("test");
    if(p != attributes.end())
    {
        _testStr = p->second;
    }

    p = attributes.find("message");
    if(p != attributes.end())
    {
        _message = p->second;
    }

    if(!_testStr.empty())
    {
        _test = parse(_testStr);
    }

    if(_message.empty())
    {
        ostringstream ostr;
        ostr << "<fail> executed at line " << line << endl;
        _message = ostr.str();
    }
}

void
FreezeScript::FailDescriptor::addChild(const DescriptorPtr&)
{
    DescriptorErrorContext ctx(_errorReporter, "fail", _line);
    _errorReporter->error("child elements are not supported");
}

void
FreezeScript::FailDescriptor::validate()
{
}

void
FreezeScript::FailDescriptor::execute(const SymbolTablePtr& sym, TransformInfo*)
{
    DescriptorErrorContext ctx(_errorReporter, "fail", _line);

    if(_test)
    {
        try
        {
            DataPtr b = _test->evaluate(sym);
            BooleanDataPtr bd = BooleanDataPtr::dynamicCast(b);
            if(!bd)
            {
                _errorReporter->error("expression `" + _testStr + "' does not evaluate to a boolean");
            }
            if(!bd->booleanValue())
            {
                return;
            }
        }
        catch(const EvaluateException& ex)
        {
            _errorReporter->error("evaluation of expression `" + _testStr + "' failed:\n" + ex.reason());
        }
    }

    throw Exception(__FILE__, __LINE__, _message);
}

//
// EchoDescriptor
//
FreezeScript::EchoDescriptor::EchoDescriptor(const DescriptorPtr& parent, int line, const DataFactoryPtr& factory,
                                             const ErrorReporterPtr& errorReporter,
                                             const IceXML::Attributes& attributes) :
    Descriptor(parent, line, factory, errorReporter)
{
    IceXML::Attributes::const_iterator p;

    p = attributes.find("message");
    if(p != attributes.end())
    {
        _message = p->second;
    }

    p = attributes.find("value");
    if(p != attributes.end())
    {
        _valueStr = p->second;
    }

    if(!_valueStr.empty())
    {
        _value = parse(_valueStr);
    }
}

void
FreezeScript::EchoDescriptor::addChild(const DescriptorPtr&)
{
    DescriptorErrorContext ctx(_errorReporter, "echo", _line);
    _errorReporter->error("child elements are not supported");
}

void
FreezeScript::EchoDescriptor::validate()
{
}

void
FreezeScript::EchoDescriptor::execute(const SymbolTablePtr& sym, TransformInfo*)
{
    DescriptorErrorContext ctx(_errorReporter, "echo", _line);

    ostream& out = _errorReporter->stream();

    if(!_message.empty())
    {
        out << _message;
    }

    if(_value)
    {
        DataPtr v;
        try
        {
            v = _value->evaluate(sym);
        }
        catch(const EvaluateException& ex)
        {
            _errorReporter->error("evaluation of value `" + _valueStr + "' failed:\n" + ex.reason());
        }
        printData(v, out);
    }

    out << endl;
}

//
// ExecutableContainerDescriptor
//
FreezeScript::ExecutableContainerDescriptor::ExecutableContainerDescriptor(const DescriptorPtr& parent, int line,
                                                                           const DataFactoryPtr& factory,
                                                                           const ErrorReporterPtr& errorReporter,
                                                                           const IceXML::Attributes&,
                                                                           const string& name) :
    Descriptor(parent, line, factory, errorReporter), _name(name)
{
}

void
FreezeScript::ExecutableContainerDescriptor::addChild(const DescriptorPtr& child)
{
    _children.push_back(child);
}

void
FreezeScript::ExecutableContainerDescriptor::validate()
{
    for(vector<DescriptorPtr>::iterator p = _children.begin(); p != _children.end(); ++p)
    {
        (*p)->validate();
    }
}

void
FreezeScript::ExecutableContainerDescriptor::execute(const SymbolTablePtr& sym, TransformInfo* info)
{
    for(vector<DescriptorPtr>::iterator p = _children.begin(); p != _children.end(); ++p)
    {
        (*p)->execute(sym, info);
    }
}

//
// IfDescriptor
//
FreezeScript::IfDescriptor::IfDescriptor(const DescriptorPtr& parent, int line, const DataFactoryPtr& factory,
                                         const ErrorReporterPtr& errorReporter,
                                         const IceXML::Attributes& attributes) :
    ExecutableContainerDescriptor(parent, line, factory, errorReporter, attributes, "if"),
    Descriptor(parent, line, factory, errorReporter)
{
    DescriptorErrorContext ctx(_errorReporter, "if", _line);

    IceXML::Attributes::const_iterator p = attributes.find("test");
    if(p == attributes.end())
    {
        _errorReporter->error("required attribute `test' is missing");
    }
    _testStr = p->second;

    _test = parse(_testStr);
}

void
FreezeScript::IfDescriptor::execute(const SymbolTablePtr& sym, TransformInfo* info)
{
    DescriptorErrorContext ctx(_errorReporter, "if", _line);

    try
    {
        DataPtr b = _test->evaluate(sym);
        BooleanDataPtr bd = BooleanDataPtr::dynamicCast(b);
        if(!bd)
        {
            _errorReporter->error("expression `" + _testStr + "' does not evaluate to a boolean");
        }
        if(bd->booleanValue())
        {
            ExecutableContainerDescriptor::execute(sym, info);
        }
    }
    catch(const EvaluateException& ex)
    {
        _errorReporter->error("evaluation of conditional expression `" + _testStr + "' failed:\n" + ex.reason());
    }
}

//
// IterateDescriptor
//
FreezeScript::IterateDescriptor::IterateDescriptor(const DescriptorPtr& parent, int line,
                                                   const DataFactoryPtr& factory,
                                                   const ErrorReporterPtr& errorReporter,
                                                   const IceXML::Attributes& attributes) :
    ExecutableContainerDescriptor(parent, line, factory, errorReporter, attributes, "iterate"),
    Descriptor(parent, line, factory, errorReporter)
{
    DescriptorErrorContext ctx(_errorReporter, "iterate", _line);

    IceXML::Attributes::const_iterator p;
    string target;

    p = attributes.find("target");
    if(p == attributes.end())
    {
        _errorReporter->error("required attribute `target' is missing");
    }
    target = p->second;

    p = attributes.find("key");
    if(p != attributes.end())
    {
        _key = p->second;
    }

    p = attributes.find("value");
    if(p != attributes.end())
    {
        _value = p->second;
    }

    p = attributes.find("element");
    if(p != attributes.end())
    {
        _element = p->second;
    }

    p = attributes.find("index");
    if(p != attributes.end())
    {
        _index = p->second;
    }

    NodePtr node = parse(target);
    _target = EntityNodePtr::dynamicCast(node);
    if(!_target)
    {
        _errorReporter->error("`target' attribute is not an entity: `" + target + "'");
    }
}

void
FreezeScript::IterateDescriptor::execute(const SymbolTablePtr& sym, TransformInfo* info)
{
    DescriptorErrorContext ctx(_errorReporter, "iterate", _line);

    DataPtr data = sym->getValue(_target);

    DictionaryDataPtr dict = DictionaryDataPtr::dynamicCast(data);
    SequenceDataPtr seq = SequenceDataPtr::dynamicCast(data);
    if(!dict && !seq)
    {
        ostringstream ostr;
        ostr << _target;
        _errorReporter->error("target `" + ostr.str() + "' is not a dictionary or sequence");
    }

    if(dict)
    {
        if(!_element.empty())
        {
            _errorReporter->error("attribute `element' specified for dictionary target");
        }
        if(!_index.empty())
        {
            _errorReporter->error("attribute `index' specified for dictionary target");
        }

        string key = _key;
        if(key.empty())
        {
            key = "key";
        }

        string value = _value;
        if(value.empty())
        {
            value = "value";
        }

        DataMap& map = dict->getElements();
        for(DataMap::iterator p = map.begin(); p != map.end(); ++p)
        {
            SymbolTablePtr elemSym = sym->createChild();
            elemSym->add(key, p->first);
            elemSym->add(value, p->second);
            ExecutableContainerDescriptor::execute(elemSym, info);
        }
    }
    else
    {
        if(!_key.empty())
        {
            _errorReporter->error("attribute `key' specified for sequence target");
        }
        if(!_value.empty())
        {
            _errorReporter->error("attribute `value' specified for sequence target");
        }

        string element = _element;
        if(element.empty())
        {
            element = "elem";
        }

        string index = _index;
        if(index.empty())
        {
            index = "i";
        }

        DataList& l = seq->getElements();
        Ice::Long i = 0;
        for(DataList::iterator p = l.begin(); p != l.end(); ++p, ++i)
        {
            SymbolTablePtr elemSym = sym->createChild();
            elemSym->add(element, *p);
            elemSym->add(index, _factory->createInteger(i, true));
            ExecutableContainerDescriptor::execute(elemSym, info);
        }
    }
}

//
// TransformDescriptor
//
FreezeScript::TransformDescriptor::TransformDescriptor(const DescriptorPtr& parent, int line,
                                                       const DataFactoryPtr& factory,
                                                       const ErrorReporterPtr& errorReporter,
                                                       const IceXML::Attributes& attributes,
                                                       const Slice::UnitPtr& oldUnit, const Slice::UnitPtr& newUnit) :
    ExecutableContainerDescriptor(parent, line, factory, errorReporter, attributes, "transform"),
    Descriptor(parent, line, factory, errorReporter), _default(true), _base(true)
{
    DescriptorErrorContext ctx(_errorReporter, "transform", _line);

    IceXML::Attributes::const_iterator p;

    string type, rename;

    p = attributes.find("type");
    if(p == attributes.end())
    {
        _errorReporter->error("required attribute `type' is missing");
    }
    type = p->second;

    p = attributes.find("default");
    if(p != attributes.end())
    {
        if(p->second == "false")
        {
            _default = false;
        }
    }

    p = attributes.find("base");
    if(p != attributes.end())
    {
        if(p->second == "false")
        {
            _base = false;
        }
    }

    p = attributes.find("rename");
    if(p != attributes.end())
    {
        rename = p->second;
    }

    Slice::TypeList l;

    l = newUnit->lookupType(type, false);
    if(l.empty())
    {
        _errorReporter->error("unable to find type `" + type + "' in new Slice definitions");
    }
    else
    {
        _newType = l.front();
    }

    if(!rename.empty())
    {
        l = oldUnit->lookupType(rename, false);
        if(l.empty())
        {
            _errorReporter->error("unable to find type `" + rename + "' in old Slice definitions");
        }
        _renameType = l.front();
    }
}

Slice::TypePtr
FreezeScript::TransformDescriptor::type() const
{
    return _newType;
}

string
FreezeScript::TransformDescriptor::typeName() const
{
    return typeToString(_newType);
}

Slice::TypePtr
FreezeScript::TransformDescriptor::renameType() const
{
    return _renameType;
}

string
FreezeScript::TransformDescriptor::renameTypeName() const
{
    string result;
    if(_renameType)
    {
        result = typeToString(_renameType);
    }
    return result;
}

bool
FreezeScript::TransformDescriptor::doDefaultTransform() const
{
    return _default;
}

bool
FreezeScript::TransformDescriptor::doBaseTransform() const
{
    return _base;
}

//
// InitDescriptor
//
FreezeScript::InitDescriptor::InitDescriptor(const DescriptorPtr& parent, int line, const DataFactoryPtr& factory,
                                             const ErrorReporterPtr& errorReporter,
                                             const IceXML::Attributes& attributes,
                                             const Slice::UnitPtr& oldUnit, const Slice::UnitPtr& newUnit) :
    ExecutableContainerDescriptor(parent, line, factory, errorReporter, attributes, "init"),
    Descriptor(parent, line, factory, errorReporter), _old(oldUnit), _new(newUnit)
{
    DescriptorErrorContext ctx(_errorReporter, "init", _line);

    IceXML::Attributes::const_iterator p = attributes.find("type");

    string type;

    if(p == attributes.end())
    {
        _errorReporter->error("required attribute `type' is missing");
    }
    type = p->second;

    Slice::TypeList l = _new->lookupType(type, false);
    if(l.empty())
    {
        _errorReporter->error("unable to find type `" + type + "' in new Slice definitions");
    }
    else
    {
        _type = l.front();
    }
}

void
FreezeScript::InitDescriptor::initialize(const DataFactoryPtr& factory, const DataPtr& data,
                                         const Ice::CommunicatorPtr& communicator)
{
    //
    // Create a new symbol table for the initializer and add the value to be initialized
    // as the symbol "value".
    //
    TransformInfo* info = 0;

    // TODO: Initialializers should have access to the main symbol table.
    SymbolTablePtr sym = new SymbolTableI(factory, _old, _new, _errorReporter, info);
    sym->add("value", data);

    execute(sym, info);
}

string
FreezeScript::InitDescriptor::typeName() const
{
    return typeToString(_type);
}

//
// RecordDescriptor
//
FreezeScript::RecordDescriptor::RecordDescriptor(const DescriptorPtr& parent, int line,
                                                 const DataFactoryPtr& factory,
                                                 const ErrorReporterPtr& errorReporter,
                                                 const IceXML::Attributes& attributes,
                                                 const Slice::UnitPtr& oldUnit, const Slice::UnitPtr& newUnit) :
    ExecutableContainerDescriptor(parent, line, factory, errorReporter, attributes, "record"),
    Descriptor(parent, line, factory, errorReporter), _old(oldUnit), _new(newUnit)
{
}

void
FreezeScript::RecordDescriptor::execute(const SymbolTablePtr& sym, TransformInfo* info)
{
    TransformInfoI* infoI = dynamic_cast<TransformInfoI*>(info);
    assert(infoI);

    //
    // We need the Instance in order to use BasicStream.
    //
    IceInternal::InstancePtr instance = IceInternal::getInstance(infoI->communicator);

    //
    // Temporarily add an object factory.
    //
    infoI->communicator->addObjectFactory(new FreezeScript::ObjectFactory(_factory, _old), "");

    //
    // Iterate over the database.
    //
    Dbc* dbc = 0;
    infoI->oldDb->cursor(infoI->oldDbTxn, &dbc, 0);
    try
    {
        Dbt dbKey, dbValue;
        while(dbc->get(&dbKey, &dbValue, DB_NEXT) == 0)
        {
            IceInternal::BasicStream inKey(instance.get());
            inKey.b.resize(dbKey.get_size());
            memcpy(&inKey.b[0], dbKey.get_data(), dbKey.get_size());
            inKey.i = inKey.b.begin();

            IceInternal::BasicStream inValue(instance.get());
            inValue.b.resize(dbValue.get_size());
            memcpy(&inValue.b[0], dbValue.get_data(), dbValue.get_size());
            inValue.i = inValue.b.begin();
            inValue.startReadEncaps();

            IceInternal::BasicStream outKey(instance.get());
            IceInternal::BasicStream outValue(instance.get());
            outValue.startWriteEncaps();
            try
            {
                transformRecord(inKey, inValue, outKey, outValue, info);
                outValue.endWriteEncaps();
                Dbt dbNewKey(&outKey.b[0], outKey.b.size()), dbNewValue(&outValue.b[0], outValue.b.size());
                if(infoI->newDb->put(infoI->newDbTxn, &dbNewKey, &dbNewValue, DB_NOOVERWRITE) == DB_KEYEXIST)
                {
                    _errorReporter->error("duplicate key encountered");
                }
            }
            catch(const DeleteRecordException&)
            {
                // The record is deleted simply by not adding it to the new database.
            }
            catch(const ClassNotFoundException& ex)
            {
                if(!infoI->purge)
                {
                    _errorReporter->error("class " + ex.id + " not found in new Slice definitions");
                }
                else
                {
                    // The record is deleted simply by not adding it to the new database.
                    _errorReporter->warning("purging database record due to missing class type " + ex.id);
                }
            }
        }
    }
    catch(...)
    {
        if(dbc)
        {
            dbc->close();
        }
        infoI->communicator->removeObjectFactory("");
        throw;
    }

    infoI->communicator->removeObjectFactory("");

    if(dbc)
    {
        dbc->close();
    }
}

void
FreezeScript::RecordDescriptor::transformRecord(IceInternal::BasicStream& inKey, IceInternal::BasicStream& inValue,
                                                IceInternal::BasicStream& outKey, IceInternal::BasicStream& outValue,
                                                TransformInfo* info)
{
    TransformInfoI* infoI = dynamic_cast<TransformInfoI*>(info);
    assert(infoI);

    //
    // Create data representations of the old key and value types.
    //
    _factory->disableInitializers();
    DataPtr oldKeyData = _factory->create(infoI->oldKeyType, true);
    Destroyer<DataPtr> oldKeyDataDestroyer(oldKeyData);
    DataPtr oldValueData = _factory->create(infoI->oldValueType, true);
    Destroyer<DataPtr> oldValueDataDestroyer(oldValueData);

    //
    // Unmarshal the old key and value.
    //
    oldKeyData->unmarshal(inKey);
    oldValueData->unmarshal(inValue);
    infoI->objectDataMap.clear();
    if(infoI->oldValueType->usesClasses())
    {
        inValue.readPendingObjects();
        ObjectVisitor visitor(infoI->objectDataMap);
        oldValueData->visit(visitor);
    }
    _factory->enableInitializers();

    //
    // Create data representations of the new key and value types.
    //
    DataPtr newKeyData = _factory->create(infoI->newKeyType, false);
    Destroyer<DataPtr> newKeyDataDestroyer(newKeyData);
    DataPtr newValueData = _factory->create(infoI->newValueType, false);
    Destroyer<DataPtr> newValueDataDestroyer(newValueData);

    //
    // Copy the data from the old key and value to the new key and value, if possible.
    //
    TransformVisitor keyVisitor(oldKeyData, _factory, _errorReporter, info);
    newKeyData->visit(keyVisitor);
    TransformVisitor valueVisitor(oldValueData, _factory, _errorReporter, info);
    newValueData->visit(valueVisitor);

    if(!_children.empty())
    {
        //
        // Execute the child descriptors.
        //
        // TODO: Revisit identifiers.
        //
        SymbolTablePtr st = new SymbolTableI(_factory, _old, _new, _errorReporter, info, infoI->symbolTable);
        st->add("oldkey", oldKeyData);
        st->add("newkey", newKeyData);
        st->add("oldvalue", oldValueData);
        st->add("newvalue", newValueData);
        ExecutableContainerDescriptor::execute(st, info);
    }

    newKeyData->marshal(outKey);
    newValueData->marshal(outValue);
    if(infoI->newValueType->usesClasses())
    {
        outValue.writePendingObjects();
    }
}

//
// DatabaseDescriptor
//
FreezeScript::DatabaseDescriptor::DatabaseDescriptor(const DescriptorPtr& parent, int line,
                                                     const DataFactoryPtr& factory,
                                                     const ErrorReporterPtr& errorReporter,
                                                     const IceXML::Attributes& attributes,
                                                     const Slice::UnitPtr& oldUnit, const Slice::UnitPtr& newUnit) :
    ExecutableContainerDescriptor(parent, line, factory, errorReporter, attributes, "database"),
    Descriptor(parent, line, factory, errorReporter), _old(oldUnit), _new(newUnit)
{
    DescriptorErrorContext ctx(_errorReporter, "database", _line);

    IceXML::Attributes::const_iterator p = attributes.find("key");
    if(p == attributes.end())
    {
        _errorReporter->error("required attribute `key' is missing");
    }
    string keyTypes = p->second;

    p = attributes.find("value");
    if(p == attributes.end())
    {
        _errorReporter->error("required attribute `value' is missing");
    }
    string valueTypes = p->second;

    string oldKeyName, newKeyName;
    string oldValueName, newValueName;
    string::size_type pos;

    pos = keyTypes.find(',');
    if(pos == 0 || pos == keyTypes.size())
    {
        _errorReporter->error("invalid key type specification `" + keyTypes +"'");
    }
    if(pos == string::npos)
    {
        oldKeyName = keyTypes;
        newKeyName = keyTypes;
    }
    else
    {
        oldKeyName = keyTypes.substr(0, pos);
        newKeyName = keyTypes.substr(pos + 1);
    }

    pos = valueTypes.find(',');
    if(pos == 0 || pos == valueTypes.size())
    {
        _errorReporter->error("invalid value type specification `" + valueTypes +"'");
    }
    if(pos == string::npos)
    {
        oldValueName = valueTypes;
        newValueName = valueTypes;
    }
    else
    {
        oldValueName = valueTypes.substr(0, pos);
        newValueName = valueTypes.substr(pos + 1);
    }

    //
    // Look up the Slice definitions for the key and value types.
    //
    _oldKey = findType(_old, oldKeyName);
    _newKey = findType(_new, newKeyName);
    _oldValue = findType(_old, oldValueName);
    _newValue = findType(_new, newValueName);
}

void
FreezeScript::DatabaseDescriptor::addChild(const DescriptorPtr& child)
{
    DescriptorErrorContext ctx(_errorReporter, "database", _line);

    RecordDescriptorPtr rec = RecordDescriptorPtr::dynamicCast(child);
    if(rec)
    {
        if(_record)
        {
            _errorReporter->error("only one <record> element can be specified");
        }
        _record = rec;
    }

    ExecutableContainerDescriptor::addChild(child);
}

void
FreezeScript::DatabaseDescriptor::execute(const SymbolTablePtr&, TransformInfo* info)
{
    TransformInfoI* infoI = dynamic_cast<TransformInfoI*>(info);
    assert(infoI);

    //
    // Store the key and value types.
    //
    infoI->oldKeyType = _oldKey;
    infoI->oldValueType = _oldValue;
    infoI->newKeyType = _newKey;
    infoI->newValueType = _newValue;

    ExecutableContainerDescriptor::execute(infoI->symbolTable, info);
}

//
// TransformDBDescriptor
//
FreezeScript::TransformDBDescriptor::TransformDBDescriptor(int line, const DataFactoryPtr& factory,
                                                           const ErrorReporterPtr& errorReporter,
                                                           const IceXML::Attributes& attributes,
                                                           const Slice::UnitPtr& oldUnit,
                                                           const Slice::UnitPtr& newUnit) :
    Descriptor(0, line, factory, errorReporter), _old(oldUnit), _new(newUnit),
    _info(new TransformInfoI(factory, errorReporter, oldUnit, newUnit))
{
}

FreezeScript::TransformDBDescriptor::~TransformDBDescriptor()
{
    delete _info;
}

void
FreezeScript::TransformDBDescriptor::addChild(const DescriptorPtr& child)
{
    DescriptorErrorContext ctx(_errorReporter, "transformdb", _line);

    DatabaseDescriptorPtr db = DatabaseDescriptorPtr::dynamicCast(child);
    TransformDescriptorPtr transform = TransformDescriptorPtr::dynamicCast(child);
    InitDescriptorPtr init = InitDescriptorPtr::dynamicCast(child);

    if(db)
    {
        if(_database)
        {
            _errorReporter->error("only one <database> element can be specified");
        }
        else
        {
            _database = db;
            _children.push_back(db);
        }
    }
    else if(transform)
    {
        string name = transform->typeName();
        TransformMap::iterator p = _info->transformMap.find(name);
        if(p != _info->transformMap.end())
        {
            _errorReporter->error("transform `" + name + "' specified more than once");
        }
        _info->transformMap.insert(TransformMap::value_type(name, transform));

        string renameTypeName = transform->renameTypeName();
        if(!renameTypeName.empty())
        {
            RenameMap::iterator q = _info->renameMap.find(renameTypeName);
            if(q != _info->renameMap.end())
            {
                _errorReporter->error("multiple transform descriptors specify the rename value `" + renameTypeName +
                                       "'");
            }
            _info->renameMap.insert(RenameMap::value_type(renameTypeName, transform->type()));
        }

        _children.push_back(transform);
    }
    else if(init)
    {
        string name = init->typeName();
        _factory->addInitializer(name, init);
        _children.push_back(init);
    }
    else
    {
        _errorReporter->error("invalid child element");
    }
}

void
FreezeScript::TransformDBDescriptor::validate()
{
    DescriptorErrorContext ctx(_errorReporter, "transformdb", _line);

    if(!_database)
    {
        _errorReporter->error("no <database> element specified");
    }

    for(vector<DescriptorPtr>::iterator p = _children.begin(); p != _children.end(); ++p)
    {
        (*p)->validate();
    }
}

void
FreezeScript::TransformDBDescriptor::execute(const SymbolTablePtr&, TransformInfo*)
{
    assert(false);
}

void
FreezeScript::TransformDBDescriptor::transform(const Ice::CommunicatorPtr& communicator, Db* oldDb, DbTxn* oldDbTxn,
                                               Db* newDb, DbTxn* newDbTxn, bool purgeObjects)
{
    _info->communicator = communicator;
    _info->oldDb = oldDb;
    _info->oldDbTxn = oldDbTxn;
    _info->newDb = newDb;
    _info->newDbTxn = newDbTxn;
    _info->purge = purgeObjects;

    try
    {
        _database->execute(0, _info);
    }
    catch(...)
    {
        _info->communicator = 0;
        _info->oldDb = 0;
        _info->oldDbTxn = 0;
        _info->newDb = 0;
        _info->newDbTxn = 0;
        throw;
    }

    _info->communicator = 0;
    _info->oldDb = 0;
    _info->oldDbTxn = 0;
    _info->newDb = 0;
    _info->newDbTxn = 0;
}

//
// SymbolTableI
//
FreezeScript::SymbolTableI::SymbolTableI(const DataFactoryPtr& factory, const Slice::UnitPtr& oldUnit,
                                         const Slice::UnitPtr& newUnit,
                                         const ErrorReporterPtr& errorReporter, TransformInfo* info,
                                         const SymbolTablePtr& parent) :
    _factory(factory), _old(oldUnit), _new(newUnit), _errorReporter(errorReporter), _info(info), _parent(parent)
{
}

void
FreezeScript::SymbolTableI::add(const string& name, const DataPtr& data)
{
    DataPtr d = findValue(name);
    if(d)
    {
        _errorReporter->error("`" + name + "' is already defined");
    }

    _dataMap.insert(DataMap::value_type(name, data));
}

FreezeScript::DataPtr
FreezeScript::SymbolTableI::getValue(const EntityNodePtr& entity) const
{
    EntityVisitor visitor(const_cast<SymbolTableI*>(this));
    entity->visit(visitor);
    DataPtr result = visitor.getCurrent();
    if(!result)
    {
        ostringstream ostr;
        ostr << "invalid entity `" << entity << "'";
        _errorReporter->error(ostr.str());
    }

    return result;
}

FreezeScript::DataPtr
FreezeScript::SymbolTableI::getConstantValue(const string& name) const
{
    string::size_type pos;
    Slice::UnitPtr unit;
    string key;

    DataMap::const_iterator p = _constantCache.find(name);
    if(p != _constantCache.end())
    {
        return p->second;
    }

    if(_parent)
    {
        return _parent->getConstantValue(name);
    }

    //
    // Name must start with "::Old" or "::New" to indicate the Slice unit.
    //
    pos = name.find("::Old");
    if(pos == 0)
    {
        if(name.length() > 5)
        {
            key = name.substr(5);
            unit = _old;
        }
    }
    else
    {
        pos = name.find("::New");
        if(pos == 0)
        {
            if(name.length() > 5)
            {
                key = name.substr(5);
                unit = _new;
            }
        }
    }

    if(key.empty())
    {
        _errorReporter->error("invalid constant name `" + name + "'");
    }

    Slice::ContainedList l = unit->findContents(key);
    if(l.empty())
    {
        _errorReporter->error("unknown constant `" + name + "'");
    }

    Slice::EnumeratorPtr e = Slice::EnumeratorPtr::dynamicCast(l.front());
    Slice::ConstPtr c = Slice::ConstPtr::dynamicCast(l.front());
    if(!e && !c)
    {
        _errorReporter->error("`" + name + "' does not refer to a Slice constant or enumerator");
    }

    DataPtr result;

    if(c)
    {
        Slice::TypePtr type = c->type();
        string value = c->value();
        Slice::BuiltinPtr b = Slice::BuiltinPtr::dynamicCast(type);
        if(b)
        {
            switch(b->kind())
            {
            case Slice::Builtin::KindByte:
            case Slice::Builtin::KindShort:
            case Slice::Builtin::KindInt:
            case Slice::Builtin::KindLong:
            {
                string::size_type end;
                Ice::Long n;
                if(!IceUtil::stringToInt64(value, n, end))
                {
                    assert(false);
                }
                result = _factory->createInteger(n, true);
                break;
            }

            case Slice::Builtin::KindBool:
            {
                result = _factory->createBoolean(value == "true", true);
                break;
            }

            case Slice::Builtin::KindFloat:
            case Slice::Builtin::KindDouble:
            {
                double v = strtod(value.c_str(), 0);
                result = _factory->createDouble(v, true);
                break;
            }

            case Slice::Builtin::KindString:
            {
                result = _factory->createString(value, true);
                break;
            }

            case Slice::Builtin::KindObject:
            case Slice::Builtin::KindObjectProxy:
            case Slice::Builtin::KindLocalObject:
                assert(false);
            }
        }
        else
        {
            Slice::EnumPtr en = Slice::EnumPtr::dynamicCast(type);
            assert(en);
            Slice::EnumeratorList el = en->getEnumerators();
            for(Slice::EnumeratorList::iterator q = el.begin(); q != el.end(); ++q)
            {
                if((*q)->name() == value)
                {
                    e = *q;
                    break;
                }
            }
            assert(e);
        }
    }

    if(!result)
    {
        assert(e);
        result = _factory->create(e->type(), true);
        EnumDataPtr ed = EnumDataPtr::dynamicCast(result);
        assert(ed);
        ed->setValueAsString(e->name());
    }

    //
    // Cache the value.
    //
    const_cast<DataMap&>(_constantCache).insert(DataMap::value_type(name, result));

    return result;
}

FreezeScript::SymbolTablePtr
FreezeScript::SymbolTableI::createChild()
{
    return new SymbolTableI(_factory, _old, _new, _errorReporter, _info, this);
}

Slice::TypePtr
FreezeScript::SymbolTableI::lookupType(const string& name)
{
    Slice::TypeList l = _new->lookupType(name, false);
    Slice::TypePtr result;
    if(!l.empty())
    {
        result = l.front();
    }
    return result;
}

FreezeScript::DataPtr
FreezeScript::SymbolTableI::invokeFunction(const string& name, const DataPtr& target, const DataList& args)
{
    if(target)
    {
        DictionaryDataPtr targetDict = DictionaryDataPtr::dynamicCast(target);
        if(targetDict && name == "containsKey")
        {
            if(args.size() != 1)
            {
                _errorReporter->error("containsKey() requires one argument");
            }
            Slice::DictionaryPtr dictType = Slice::DictionaryPtr::dynamicCast(targetDict->getType());
            assert(dictType);
            DataPtr key = _factory->create(dictType->keyType(), false);
            assignOrTransform(key, args[0], false, _factory, _errorReporter, _info);
            DataPtr value = targetDict->getElement(key);
            return _factory->createBoolean(value ? true : false, false);
        }

        DataPtr result;
        if(invokeMemberFunction(name, target, args, result, _factory, _errorReporter))
        {
            return result;
        }

        _errorReporter->error("unknown function `" + name + "' invoked on type " + typeToString(target->getType()));
    }
    else
    {
        //
        // Global function.
        //
        DataPtr result;
        if(invokeGlobalFunction(name, args, result, _factory, _errorReporter))
        {
            return result;
        }
        else
        {
            _errorReporter->error("unknown global function `" + name + "'");
        }
    }

    return 0;
}

FreezeScript::DataPtr
FreezeScript::SymbolTableI::findValue(const string& name) const
{
    DataMap::const_iterator p = _dataMap.find(name);
    if(p != _dataMap.end())
    {
        return p->second;
    }

    if(_parent)
    {
        SymbolTableIPtr parentI = SymbolTableIPtr::dynamicCast(_parent);
        assert(parentI);
        return parentI->findValue(name);
    }

    return 0;
}

FreezeScript::SymbolTableI::EntityVisitor::EntityVisitor(SymbolTableI* table) :
    _table(table), _error(false)
{
}

void
FreezeScript::SymbolTableI::EntityVisitor::visitIdentifier(const string& name)
{
    if(!_error)
    {
        if(!_current)
        {
            _current = _table->findValue(name);
        }
        else
        {
            _current = _current->getMember(name);
        }

        if(!_current)
        {
            _error = true;
        }
    }
}

void
FreezeScript::SymbolTableI::EntityVisitor::visitElement(const NodePtr& value)
{
    if(!_error)
    {
        assert(_current);

        DataPtr val = value->evaluate(_table);
        _current = _current->getElement(val);

        if(!_current)
        {
            _error = true;
        }
    }
}

FreezeScript::DataPtr
FreezeScript::SymbolTableI::EntityVisitor::getCurrent() const
{
    return _current;
}

//
// ObjectVisitor
//
FreezeScript::ObjectVisitor::ObjectVisitor(ObjectDataMap& objectDataMap) :
    _map(objectDataMap)
{
}

void
FreezeScript::ObjectVisitor::visitStruct(const StructDataPtr& data)
{
    Slice::TypePtr type = data->getType();
    if(type->usesClasses())
    {
        DataMemberMap& members = data->getMembers();
        for(DataMemberMap::iterator p = members.begin(); p != members.end(); ++p)
        {
            p->second->visit(*this);
        }
    }
}

void
FreezeScript::ObjectVisitor::visitSequence(const SequenceDataPtr& data)
{
    Slice::TypePtr type = data->getType();
    if(type->usesClasses())
    {
        DataList& elements = data->getElements();
        for(DataList::iterator p = elements.begin(); p != elements.end(); ++p)
        {
            (*p)->visit(*this);
        }
    }
}

void
FreezeScript::ObjectVisitor::visitDictionary(const DictionaryDataPtr& data)
{
    Slice::TypePtr type = data->getType();
    if(type->usesClasses())
    {
        DataMap& elements = data->getElements();
        for(DataMap::iterator p = elements.begin(); p != elements.end(); ++p)
        {
            //
            // NOTE: There's no need to visit the key.
            //
            p->second->visit(*this);
        }
    }
}

void
FreezeScript::ObjectVisitor::visitObject(const ObjectRefPtr& data)
{
    ObjectDataPtr value = data->getValue();
    if(value)
    {
        ObjectDataMap::iterator p = _map.find(value.get());
        if(p == _map.end())
        {
            _map.insert(ObjectDataMap::value_type(value.get(), 0));
            DataMemberMap& members = value->getMembers();
            for(DataMemberMap::iterator q = members.begin(); q != members.end(); ++q)
            {
                q->second->visit(*this);
            }
        }
    }
}

//
// TransformInfoI
//
FreezeScript::TransformInfoI::TransformInfoI(const DataFactoryPtr& fact, const ErrorReporterPtr& err,
                                             const Slice::UnitPtr& oldU, const Slice::UnitPtr& newU) :
    factory(fact), errorReporter(err), oldUnit(oldU), newUnit(newU), purge(false), oldDb(0), oldDbTxn(0), newDb(0),
    newDbTxn(0)
{
    symbolTable = new SymbolTableI(factory, oldUnit, newUnit, errorReporter, this);
}

bool
FreezeScript::TransformInfoI::doDefaultTransform(const Slice::TypePtr& type)
{
    TransformMap::const_iterator p = transformMap.find(typeToString(type));
    if(p != transformMap.end())
    {
        return p->second->doDefaultTransform();
    }
    return true;
}

bool
FreezeScript::TransformInfoI::doBaseTransform(const Slice::ClassDefPtr& type)
{
    TransformMap::const_iterator p = transformMap.find(type->scoped());
    if(p != transformMap.end())
    {
        return p->second->doBaseTransform();
    }
    return true;
}

Slice::TypePtr
FreezeScript::TransformInfoI::getRenamedType(const Slice::TypePtr& old)
{
    RenameMap::iterator p = renameMap.find(typeToString(old));
    if(p != renameMap.end())
    {
        return p->second;
    }
    return 0;
}

void
FreezeScript::TransformInfoI::executeCustomTransform(const DataPtr& dest, const DataPtr& src)
{
    //
    // Execute the type's transform (if any). Non-nil objects need special consideration,
    // for two reasons:
    //
    // 1. The dest and src arguments are ObjectRef instances whose getType()
    //    function returns the formal type, which may not match the actual type
    //    if inheritance is being used. Therefore, we need to look for the
    //    transform of the actual type of the object.
    //
    // 2. It's not sufficient to execute only the transform for the actual type;
    //    the transform descriptors for base types must also be executed (if not
    //    explicitly precluded).
    //
    // The algorithm goes like this:
    //
    // 1. If a transform exists for the actual type, execute it.
    // 2. If the transform doesn't exist, or if it does exist and does not preclude
    //    the execution of the base transform, then obtain the base type. If the
    //    type has no user-defined base class, use Object.
    // 3. If a base type was found and a transform exists for the base type, execute it.
    // 4. Repeat step 2.
    //
    ObjectRefPtr obj = ObjectRefPtr::dynamicCast(dest);
    if(obj && obj->getValue())
    {
        ObjectDataPtr data = obj->getValue();
        Slice::TypePtr cls = data->getType(); // Actual type
        bool transformBase = true;
        while(cls)
        {
            string type = typeToString(cls);
            TransformMap::const_iterator p = transformMap.find(type);
            if(p != transformMap.end())
            {
                SymbolTablePtr sym = new SymbolTableI(factory, oldUnit, newUnit, errorReporter, this, symbolTable);
                sym->add("new", dest);
                sym->add("old", src);
                p->second->execute(sym, this);
                transformBase = p->second->doBaseTransform();
            }
            Slice::ClassDeclPtr decl = Slice::ClassDeclPtr::dynamicCast(cls);
            cls = 0;
            if(transformBase && decl)
            {
                Slice::ClassDefPtr def = decl->definition();
                assert(def);
                Slice::ClassList bases = def->bases();
                if(!bases.empty() && !bases.front()->isInterface())
                {
                    cls = bases.front()->declaration();
                }
                else
                {
                    cls = newUnit->builtin(Slice::Builtin::KindObject);
                }
            }
        }
    }
    else
    {
        string type = typeToString(dest->getType());
        TransformMap::const_iterator p = transformMap.find(type);
        if(p != transformMap.end())
        {
            SymbolTablePtr sym = new SymbolTableI(factory, oldUnit, newUnit, errorReporter, this, symbolTable);
            sym->add("new", dest);
            sym->add("old", src);
            p->second->execute(sym, this);
        }
    }
}

bool
FreezeScript::TransformInfoI::purgeObjects()
{
    return purge;
}

FreezeScript::ObjectDataMap&
FreezeScript::TransformInfoI::getObjectDataMap()
{
    return objectDataMap;
}

//
// assignOrTransform
//
void
FreezeScript::assignOrTransform(const DataPtr& dest, const DataPtr& src, bool convert, const DataFactoryPtr& factory,
                                const ErrorReporterPtr& errorReporter, TransformInfo* info)
{
    Slice::TypePtr destType = dest->getType();
    Slice::TypePtr srcType = src->getType();
    Slice::BuiltinPtr b1 = Slice::BuiltinPtr::dynamicCast(destType);
    Slice::BuiltinPtr b2 = Slice::BuiltinPtr::dynamicCast(srcType);

    if(!b1 && !b2 && dest->getType()->unit().get() != src->getType()->unit().get())
    {
        TransformVisitor v(src, factory, errorReporter, info);
        dest->visit(v);
    }
    else
    {
        AssignVisitor v(src, factory, errorReporter, convert);
        dest->visit(v);
    }
}
