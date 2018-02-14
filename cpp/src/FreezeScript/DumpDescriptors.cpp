// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <FreezeScript/DumpDescriptors.h>
#include <FreezeScript/AssignVisitor.h>
#include <FreezeScript/Print.h>
#include <FreezeScript/Functions.h>
#include <FreezeScript/Exception.h>
#include <FreezeScript/Util.h>
#include <IceUtil/InputUtil.h>
#include <db_cxx.h>
#include <set>
#include <climits>

using namespace std;

namespace FreezeScript
{

class SymbolTableI;
typedef IceUtil::Handle<SymbolTableI> SymbolTableIPtr;

class SymbolTableI : public SymbolTable
{
public:

    SymbolTableI(const DataFactoryPtr&, const Slice::UnitPtr&, const ErrorReporterPtr&, ExecuteInfo*,
                 const SymbolTablePtr& = SymbolTablePtr());

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
    Slice::UnitPtr _unit;
    ErrorReporterPtr _errorReporter;
    ExecuteInfo* _info;
    SymbolTablePtr _parent;
    typedef map<string, DataPtr> DataMap;
    DataMap _dataMap;
    DataMap _constantCache;
};

typedef map<string, DumpDescriptorPtr> DumpMap;

struct ExecuteInfo
{
    Ice::CommunicatorPtr communicator;
    Db* db;
    DbTxn* txn;
    string facet;
    SymbolTablePtr symbolTable;
    DumpMap dumpMap;
    Slice::TypePtr keyType;
    Slice::TypePtr valueType;
};

class DumpVisitor : public DataVisitor
{
public:

    DumpVisitor(const DataFactoryPtr&, const Slice::UnitPtr&, const ErrorReporterPtr&, ExecuteInfo*);

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

private:

    //
    // Returns true if the contents of the data should be visited.
    //
    bool dump(const DataPtr&);

    DataFactoryPtr _factory;
    Slice::UnitPtr _unit;
    ErrorReporterPtr _errorReporter;
    ExecuteInfo* _info;
    set<const ObjectData*> _objectHistory;
};

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

    p = attributes.find("target");
    if(p == attributes.end())
    {
        _errorReporter->error("required attribute `target' is missing");
    }
    NodePtr node = parse(p->second);
    _target = EntityNodePtr::dynamicCast(node);
    if(!_target)
    {
        _errorReporter->error("`target' attribute is not an entity: `" + p->second + "'");
    }

    p = attributes.find("value");
    if(p != attributes.end())
    {
        _valueStr = p->second;
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

    if(_valueStr.empty() && _lengthStr.empty())
    {
        _errorReporter->error("requires a value for attributes `value' or 'length'");
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
FreezeScript::SetDescriptor::execute(const SymbolTablePtr& sym, ExecuteInfo*)
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

    DataPtr length;
    if(_length)
    {
        SequenceDataPtr seq = SequenceDataPtr::dynamicCast(data);
        if(!seq)
        {
            ostringstream ostr;
            ostr << _target;
            _errorReporter->error("value `" + ostr.str() + "' is not a sequence");
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
                    AssignVisitor visitor(value, _factory, _errorReporter, _convert);
                    v->visit(visitor);
                }
                elements.push_back(v);
            }
        }
    }
    else
    {
        AssignVisitor visitor(value, _factory, _errorReporter, _convert);
        data->visit(visitor);
    }
}

//
// DefineDescriptor
//
FreezeScript::DefineDescriptor::DefineDescriptor(const DescriptorPtr& parent, int line, const DataFactoryPtr& factory,
                                                 const ErrorReporterPtr& errorReporter,
                                                 const IceXML::Attributes& attributes, const Slice::UnitPtr& unit) :
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
    _type = findType(unit, p->second);

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
FreezeScript::DefineDescriptor::execute(const SymbolTablePtr& sym, ExecuteInfo*)
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
        AssignVisitor visitor(value, _factory, _errorReporter, _convert);
        data->visit(visitor);
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
    if(p != attributes.end())
    {
        _keyStr = p->second;
    }

    p = attributes.find("index");
    if(p != attributes.end())
    {
        _indexStr = p->second;
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

    NodePtr node = parse(target);
    _target = EntityNodePtr::dynamicCast(node);
    if(!_target)
    {
        _errorReporter->error("`target' attribute is not an entity: `" + target + "'");
    }

    if(!_keyStr.empty() && !_indexStr.empty())
    {
        _errorReporter->error("attributes `key' and `index' are mutually exclusive");
    }

    if(_keyStr.empty() && _indexStr.empty())
    {
        _errorReporter->error("one of attributes `key' or `index' is required");
    }

    if(!_keyStr.empty())
    {
        _key = parse(_keyStr);
    }

    if(!_indexStr.empty())
    {
        _index = parse(_indexStr);
    }

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
FreezeScript::AddDescriptor::execute(const SymbolTablePtr& sym, ExecuteInfo*)
{
    DescriptorErrorContext ctx(_errorReporter, "add", _line);

    DataPtr data = sym->getValue(_target);
    if(data->readOnly())
    {
        ostringstream ostr;
        ostr << _target;
        _errorReporter->error("target `" + ostr.str() + "' cannot be modified");
    }

    if(_key)
    {
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
            AssignVisitor visitor(v, _factory, _errorReporter, _convert);
            key->visit(visitor);
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

        if(value)
        {
            AssignVisitor visitor(value, _factory, _errorReporter, _convert);
            elem->visit(visitor);
        }
        DataMap& map = dict->getElements();
        map.insert(DataMap::value_type(key, elem));
        keyDestroyer.release();
        elemDestroyer.release();
    }
    else
    {
        assert(_index);

        SequenceDataPtr seq = SequenceDataPtr::dynamicCast(data);
        if(!seq)
        {
            ostringstream ostr;
            ostr << _target;
            _errorReporter->error("target `" + ostr.str() + "' is not a sequence");
        }

        Slice::SequencePtr type = Slice::SequencePtr::dynamicCast(seq->getType());
        assert(type);

        DataPtr index;
        Destroyer<DataPtr> indexDestroyer;
        try
        {
            index = _index->evaluate(sym);
            indexDestroyer.set(index);
        }
        catch(const EvaluateException& ex)
        {
            _errorReporter->error("evaluation of index `" + _indexStr + "' failed:\n" + ex.reason());
        }

        DataList& elements = seq->getElements();
        Ice::Long l = index->integerValue();
        DataList::size_type i = static_cast<DataList::size_type>(l);
        if(l < 0 || l > INT_MAX || i > elements.size())
        {
            _errorReporter->error("sequence index " + index->toString() + " is out of range");
        }

        DataPtr elem = _factory->create(type->type(), false);
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

        if(value)
        {
            AssignVisitor visitor(value, _factory, _errorReporter, _convert);
            elem->visit(visitor);
        }

        elements.insert(elements.begin() + i, elem);
        indexDestroyer.release();
        elemDestroyer.release();
    }
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
    if(p != attributes.end())
    {
        _keyStr = p->second;
    }

    p = attributes.find("index");
    if(p != attributes.end())
    {
        _indexStr = p->second;
    }

    if(!_keyStr.empty() && !_indexStr.empty())
    {
        _errorReporter->error("attributes `key' and `index' are mutually exclusive");
    }

    if(_keyStr.empty() && _indexStr.empty())
    {
        _errorReporter->error("one of attributes `key' or `index' is required");
    }

    NodePtr node = parse(target);
    _target = EntityNodePtr::dynamicCast(node);
    if(!_target)
    {
        _errorReporter->error("`target' attribute is not an entity: `" + target + "'");
    }

    if(!_keyStr.empty())
    {
        _key = parse(_keyStr);
    }

    if(!_indexStr.empty())
    {
        _index = parse(_indexStr);
    }
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
FreezeScript::RemoveDescriptor::execute(const SymbolTablePtr& sym, ExecuteInfo*)
{
    DescriptorErrorContext ctx(_errorReporter, "remove", _line);

    DataPtr data = sym->getValue(_target);
    if(data->readOnly())
    {
        ostringstream ostr;
        ostr << _target;
        _errorReporter->error("target `" + ostr.str() + "' cannot be modified");
    }

    if(_key)
    {
        DataPtr key;
        try
        {
            key = _key->evaluate(sym);
        }
        catch(const EvaluateException& ex)
        {
            _errorReporter->error("evaluation of key `" + _keyStr + "' failed:\n" + ex.reason());
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
    else
    {
        assert(_index);

        DataPtr index;
        try
        {
            index = _index->evaluate(sym);
        }
        catch(const EvaluateException& ex)
        {
            _errorReporter->error("evaluation of index `" + _indexStr + "' failed:\n" + ex.reason());
        }

        SequenceDataPtr seq = SequenceDataPtr::dynamicCast(data);
        if(!seq)
        {
            ostringstream ostr;
            ostr << _target;
            _errorReporter->error("target `" + ostr.str() + "' is not a sequence");
        }

        DataList& elements = seq->getElements();
        Ice::Long l = index->integerValue();
        DataList::size_type i = static_cast<DataList::size_type>(l);
        if(l < 0 || l > INT_MAX || i >= elements.size())
        {
            _errorReporter->error("sequence index " + index->toString() + " is out of range");
        }

        elements[i]->destroy();
        elements.erase(elements.begin() + i);
    }
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
FreezeScript::FailDescriptor::execute(const SymbolTablePtr& sym, ExecuteInfo*)
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

    throw FailureException(__FILE__, __LINE__, _message);
}

//
// EchoDescriptor
//
FreezeScript::EchoDescriptor::EchoDescriptor(const DescriptorPtr& parent, int line, const DataFactoryPtr& factory,
                                             const ErrorReporterPtr& errorReporter,
                                             const IceXML::Attributes& attributes,
                                             ostream& os) :
    Descriptor(parent, line, factory, errorReporter), _os(os)
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
FreezeScript::EchoDescriptor::execute(const SymbolTablePtr& sym, ExecuteInfo*)
{
    DescriptorErrorContext ctx(_errorReporter, "echo", _line);

    if(!_message.empty())
    {
        _os << _message;
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
        printData(v, _os);
    }

    _os << endl;
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
FreezeScript::ExecutableContainerDescriptor::execute(const SymbolTablePtr& sym, ExecuteInfo* info)
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
    Descriptor(parent, line, factory, errorReporter),
    ExecutableContainerDescriptor(parent, line, factory, errorReporter, attributes, "if")
   
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
FreezeScript::IfDescriptor::execute(const SymbolTablePtr& sym, ExecuteInfo* info)
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
    Descriptor(parent, line, factory, errorReporter),
    ExecutableContainerDescriptor(parent, line, factory, errorReporter, attributes, "iterate")
  
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
FreezeScript::IterateDescriptor::execute(const SymbolTablePtr& sym, ExecuteInfo* info)
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

        //
        // NOTE: Don't use iterator to traverse vector because child descriptors might remove elements.
        //
        DataList& l = seq->getElements();
        DataList::size_type i = 0;
        while(i < l.size())
        {
            SymbolTablePtr elemSym = sym->createChild();
            elemSym->add(element, l[i]);
            elemSym->add(index, _factory->createInteger(i, true));
            ExecutableContainerDescriptor::execute(elemSym, info);
            ++i;
        }
    }
}

//
// DumpDescriptor
//
FreezeScript::DumpDescriptor::DumpDescriptor(const DescriptorPtr& parent, int line,
                                             const DataFactoryPtr& factory,
                                             const ErrorReporterPtr& errorReporter,
                                             const IceXML::Attributes& attributes,
                                             const Slice::UnitPtr& unit) :
    Descriptor(parent, line, factory, errorReporter),
    ExecutableContainerDescriptor(parent, line, factory, errorReporter, attributes, "dump"),
    _base(true), 
    _contents(true)
{
    DescriptorErrorContext ctx(_errorReporter, "dump", _line);

    for(IceXML::Attributes::const_iterator p = attributes.begin(); p != attributes.end(); ++p)
    {
        if(p->first == "type")
        {
            if(_type)
            {
                _errorReporter->error("duplicate attribute `type'");
            }
            _type = findType(unit, p->second);
        }
        else if(p->first == "base")
        {
            if(p->second == "false")
            {
                _base = false;
            }
            else if(p->second != "true")
            {
                _errorReporter->error("invalid value `" + p->second + "' for attribute `base'");
            }
        }
        else if(p->first == "contents")
        {
            if(p->second == "false")
            {
                _contents = false;
            }
            else if(p->second != "true")
            {
                _errorReporter->error("invalid value `" + p->second + "' for attribute `contents'");
            }
        }
        else
        {
            _errorReporter->error("unknown attribute `" + p->first + "'");
        }
    }

    if(!_type)
    {
        _errorReporter->error("required attribute `type' is missing");
    }
}

Slice::TypePtr
FreezeScript::DumpDescriptor::type() const
{
    return _type;
}

string
FreezeScript::DumpDescriptor::typeName() const
{
    return typeToString(_type);
}

bool
FreezeScript::DumpDescriptor::base() const
{
    return _base;
}

bool
FreezeScript::DumpDescriptor::contents() const
{
    return _contents;
}

//
// RecordDescriptor
//
FreezeScript::RecordDescriptor::RecordDescriptor(const DescriptorPtr& parent, int line,
                                                 const DataFactoryPtr& factory,
                                                 const ErrorReporterPtr& errorReporter,
                                                 const IceXML::Attributes& attributes,
                                                 const Slice::UnitPtr& unit,
                                                 const FreezeScript::ObjectFactoryPtr& objectFactory) :
    Descriptor(parent, line, factory, errorReporter), 
    ExecutableContainerDescriptor(parent, line, factory, errorReporter, attributes, "record"),
    _unit(unit),
    _objectFactory(objectFactory)
{
}

void
FreezeScript::RecordDescriptor::execute(const SymbolTablePtr& /*sym*/, ExecuteInfo* info)
{
    //
    // Temporarily add an object factory.
    //
    _objectFactory->activate(_factory, _unit);

    //
    // Iterate over the database.
    //
    Dbc* dbc = 0;
    info->db->cursor(info->txn, &dbc, 0);
    try
    {
        Dbt dbKey, dbValue;
        while(dbc->get(&dbKey, &dbValue, DB_NEXT) == 0)
        {
            Ice::ByteSeq keyBytes;
            keyBytes.resize(dbKey.get_size());
            memcpy(&keyBytes[0], dbKey.get_data(), dbKey.get_size());
            Ice::InputStreamPtr inKey = Ice::wrapInputStream(info->communicator, keyBytes);

            Ice::ByteSeq valueBytes;
            valueBytes.resize(dbValue.get_size());
            memcpy(&valueBytes[0], dbValue.get_data(), dbValue.get_size());
            Ice::InputStreamPtr inValue = Ice::wrapInputStream(info->communicator, valueBytes);
            inValue->startEncapsulation();

            //
            // Create data representations of the key and value types.
            //
            DataPtr keyData = _factory->create(info->keyType, true);
            Destroyer<DataPtr> keyDataDestroyer(keyData);
            DataPtr valueData = _factory->create(info->valueType, true);
            Destroyer<DataPtr> valueDataDestroyer(valueData);
            DataPtr facetData = _factory->createString(info->facet, true);
            Destroyer<DataPtr> facetDataDestroyer(facetData);

            //
            // Unmarshal the key and value.
            //
            keyData->unmarshal(inKey);
            valueData->unmarshal(inValue);
            if(info->valueType->usesClasses())
            {
                inValue->readPendingObjects();
            }

            //
            // Visit the key and value.
            //
            DumpVisitor visitor(_factory, _unit, _errorReporter, info);
            keyData->visit(visitor);
            valueData->visit(visitor);

            if(!_children.empty())
            {
                //
                // Execute the child descriptors.
                //
                SymbolTablePtr st = new SymbolTableI(_factory, _unit, _errorReporter, info, info->symbolTable);
                st->add("key", keyData);
                st->add("value", valueData);
                st->add("facet", facetData);
                ExecutableContainerDescriptor::execute(st, info);
            }
        }
    }
    catch(...)
    {
        if(dbc)
        {
            dbc->close();
        }
        _objectFactory->deactivate();
        throw;
    }

    if(dbc)
    {
        dbc->close();
    }
    _objectFactory->deactivate();
}

//
// DatabaseDescriptor
//
FreezeScript::DatabaseDescriptor::DatabaseDescriptor(const DescriptorPtr& parent, int line,
                                                     const DataFactoryPtr& factory,
                                                     const ErrorReporterPtr& errorReporter,
                                                     const IceXML::Attributes& attributes,
                                                     const Slice::UnitPtr& unit) :
    Descriptor(parent, line, factory, errorReporter), 
    ExecutableContainerDescriptor(parent, line, factory, errorReporter, attributes, "database"),
    _unit(unit)
{
    DescriptorErrorContext ctx(_errorReporter, "database", _line);

    IceXML::Attributes::const_iterator p = attributes.find("key");
    if(p == attributes.end())
    {
        _errorReporter->error("required attribute `key' is missing");
    }
    _key = findType(unit, p->second);

    p = attributes.find("value");
    if(p == attributes.end())
    {
        _errorReporter->error("required attribute `value' is missing");
    }
    _value = findType(unit, p->second);
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
FreezeScript::DatabaseDescriptor::execute(const SymbolTablePtr&, ExecuteInfo* info)
{
    //
    // Store the key and value types.
    //
    info->keyType = _key;
    info->valueType = _value;

    ExecutableContainerDescriptor::execute(info->symbolTable, info);
}

//
// DumpDBDescriptor
//
FreezeScript::DumpDBDescriptor::DumpDBDescriptor(int line, const DataFactoryPtr& factory,
                                                 const ErrorReporterPtr& errorReporter,
                                                 const IceXML::Attributes& /*attributes*/, const Slice::UnitPtr& unit) :
    Descriptor(0, line, factory, errorReporter), _unit(unit), _info(new ExecuteInfo)
{
    _info->symbolTable = new SymbolTableI(factory, unit, errorReporter, _info);
}

FreezeScript::DumpDBDescriptor::~DumpDBDescriptor()
{
    delete _info;
}

void
FreezeScript::DumpDBDescriptor::addChild(const DescriptorPtr& child)
{
    DescriptorErrorContext ctx(_errorReporter, "dumpdb", _line);

    DatabaseDescriptorPtr db = DatabaseDescriptorPtr::dynamicCast(child);
    DumpDescriptorPtr dump = DumpDescriptorPtr::dynamicCast(child);

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
    else if(dump)
    {
        string name = dump->typeName();
        DumpMap::iterator p = _info->dumpMap.find(name);
        if(p != _info->dumpMap.end())
        {
            _errorReporter->error("<dump> descriptor for `" + name + "' specified more than once");
        }
        _info->dumpMap.insert(DumpMap::value_type(name, dump));

        _children.push_back(dump);
    }
    else
    {
        _errorReporter->error("invalid child element");
    }
}

void
FreezeScript::DumpDBDescriptor::validate()
{
    DescriptorErrorContext ctx(_errorReporter, "dumpdb", _line);

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
FreezeScript::DumpDBDescriptor::execute(const SymbolTablePtr&, ExecuteInfo*)
{
    assert(false);
}

void
FreezeScript::DumpDBDescriptor::dump(const Ice::CommunicatorPtr& communicator, Db* db, DbTxn* txn, const string& facet)
{
    _info->communicator = communicator;
    _info->db = db;
    _info->txn = txn;
    _info->facet = facet;

    try
    {
        _database->execute(0, _info);
    }
    catch(...)
    {
        _info->communicator = 0;
        _info->db = 0;
        _info->txn = 0;
        throw;
    }

    _info->communicator = 0;
    _info->db = 0;
    _info->txn = 0;
}

//
// SymbolTableI
//
FreezeScript::SymbolTableI::SymbolTableI(const DataFactoryPtr& factory, const Slice::UnitPtr& unit,
                                         const ErrorReporterPtr& errorReporter, ExecuteInfo* info,
                                         const SymbolTablePtr& parent) :
    _factory(factory), _unit(unit), _errorReporter(errorReporter), _info(info), _parent(parent)
{
}

void
FreezeScript::SymbolTableI::add(const string& name, const DataPtr& data)
{
    DataMap::const_iterator p = _dataMap.find(name);
    if(p != _dataMap.end())
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
    DataMap::const_iterator p = _constantCache.find(name);
    if(p != _constantCache.end())
    {
        return p->second;
    }

    if(_parent)
    {
        return _parent->getConstantValue(name);
    }

    Slice::ContainedList l = _unit->findContents(name);
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
                Ice::Long n;
                if(!IceUtilInternal::stringToInt64(value, n))
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
    return new SymbolTableI(_factory, _unit, _errorReporter, _info, this);
}

Slice::TypePtr
FreezeScript::SymbolTableI::lookupType(const string& name)
{
    Slice::TypeList l = _unit->lookupType(name, false);
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
            AssignVisitor visitor(args[0], _factory, _errorReporter, false);
            key->visit(visitor);
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
        if(invokeGlobalFunction(_info->communicator, name, args, result, _factory, _errorReporter))
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
// DumpVisitor
//
FreezeScript::DumpVisitor::DumpVisitor(const DataFactoryPtr& factory, const Slice::UnitPtr& unit,
                                       const ErrorReporterPtr& errorReporter, ExecuteInfo* info) :
    _factory(factory), _unit(unit), _errorReporter(errorReporter), _info(info)
{
}

void
FreezeScript::DumpVisitor::visitBoolean(const BooleanDataPtr& data)
{
    dump(data);
}

void
FreezeScript::DumpVisitor::visitInteger(const IntegerDataPtr& data)
{
    dump(data);
}

void
FreezeScript::DumpVisitor::visitDouble(const DoubleDataPtr& data)
{
    dump(data);
}

void
FreezeScript::DumpVisitor::visitString(const StringDataPtr& data)
{
    dump(data);
}

void
FreezeScript::DumpVisitor::visitProxy(const ProxyDataPtr& data)
{
    dump(data);
}

void
FreezeScript::DumpVisitor::visitStruct(const StructDataPtr& data)
{
    if(dump(data))
    {
        DataMemberMap& members = data->getMembers();
        for(DataMemberMap::iterator p = members.begin(); p != members.end(); ++p)
        {
            p->second->visit(*this);
        }
    }
}

void
FreezeScript::DumpVisitor::visitSequence(const SequenceDataPtr& data)
{
    if(dump(data))
    {
        DataList& elements = data->getElements();
        for(DataList::iterator p = elements.begin(); p != elements.end(); ++p)
        {
            (*p)->visit(*this);
        }
    }
}

void
FreezeScript::DumpVisitor::visitEnum(const EnumDataPtr& data)
{
    dump(data);
}

void
FreezeScript::DumpVisitor::visitDictionary(const DictionaryDataPtr& data)
{
    if(dump(data))
    {
        DataMap& elements = data->getElements();
        for(DataMap::iterator p = elements.begin(); p != elements.end(); ++p)
        {
            p->first->visit(*this);
            p->second->visit(*this);
        }
    }
}

void
FreezeScript::DumpVisitor::visitObject(const ObjectRefPtr& data)
{
    if(dump(data))
    {
        ObjectDataPtr value = data->getValue();
        if(value)
        {
            set<const ObjectData*>::iterator p = _objectHistory.find(value.get());
            if(p == _objectHistory.end())
            {
                _objectHistory.insert(value.get());
                DataMemberMap& members = value->getMembers();
                for(DataMemberMap::iterator q = members.begin(); q != members.end(); ++q)
                {
                    q->second->visit(*this);
                }
            }
        }
    }
}

bool
FreezeScript::DumpVisitor::dump(const DataPtr& data)
{
    bool result = true;
    ObjectRefPtr obj = ObjectRefPtr::dynamicCast(data);
    if(obj && obj->getValue())
    {
        //
        // When the value is an object, attempt to find a <dump> descriptor for the
        // object's actual type. If no descriptor is found, attempt to find a
        // descriptor for the object's base types (including Ice::Object).
        //
        ObjectDataPtr objData = obj->getValue();
        Slice::TypePtr cls = objData->getType(); // Actual type
        bool checkContents = true;
        while(cls)
        {
            string type = typeToString(cls);
            bool base = true;
            DumpMap::const_iterator p = _info->dumpMap.find(type);
            if(p != _info->dumpMap.end())
            {
                SymbolTablePtr sym = new SymbolTableI(_factory, _unit, _errorReporter, _info, _info->symbolTable);
                sym->add("value", objData);
                p->second->execute(sym, _info);
                base = p->second->base();
                if(checkContents)
                {
                    result = p->second->contents();
                    checkContents = false;
                }
            }
            Slice::ClassDeclPtr decl = Slice::ClassDeclPtr::dynamicCast(cls);
            cls = 0;
            if(base && decl)
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
                    cls = _unit->builtin(Slice::Builtin::KindObject);
                }
            }
        }
    }
    else
    {
        string typeName = typeToString(data->getType());
        DumpMap::iterator p = _info->dumpMap.find(typeName);
        if(p != _info->dumpMap.end())
        {
            SymbolTablePtr st = new SymbolTableI(_factory, _unit, _errorReporter, _info, _info->symbolTable);
            st->add("value", data);
            p->second->execute(st, _info);
            result = p->second->contents();
        }

    }

    return result;
}
