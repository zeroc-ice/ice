// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <FreezeScript/Transformer.h>
#include <FreezeScript/Parser.h>
#include <FreezeScript/TransformVisitor.h>
#include <FreezeScript/AssignVisitor.h>
#include <FreezeScript/Print.h>
#include <FreezeScript/Functions.h>
#include <FreezeScript/Exception.h>
#include <FreezeScript/Util.h>
#include <Freeze/Catalog.h>
#include <Freeze/Initialize.h>
#include <IceXML/Parser.h>
#include <IceUtil/InputUtil.h>
#include <db_cxx.h>
#include <climits>

using namespace std;

namespace FreezeScript
{

class DeleteRecordException {};

class Descriptor;
typedef IceUtil::Handle<Descriptor> DescriptorPtr;

class InitDescriptor;
typedef IceUtil::Handle<InitDescriptor> InitDescriptorPtr;

class TransformDescriptor;
typedef IceUtil::Handle<TransformDescriptor> TransformDescriptorPtr;

class TransformDataFactory;
typedef IceUtil::Handle<TransformDataFactory> TransformDataFactoryPtr;

typedef map<string, TransformDescriptorPtr> TransformMap;
typedef map<string, Slice::TypePtr> RenameMap;

//
// TransformInfoI
//
struct TransformInfoI : public TransformInfo
{
    virtual DataFactoryPtr getDataFactory();
    virtual ErrorReporterPtr getErrorReporter();
    virtual bool doDefaultTransform(const Slice::TypePtr&);
    virtual bool doBaseTransform(const Slice::ClassDefPtr&);
    virtual Slice::TypePtr getRenamedType(const Slice::TypePtr&);
    virtual void executeCustomTransform(const DataPtr&, const DataPtr&);
    virtual string facetName();
    virtual bool purgeObjects();
    virtual ObjectDataMap& getObjectDataMap();

    Ice::CommunicatorPtr communicator;
    FreezeScript::ObjectFactoryPtr objectFactory;
    Slice::UnitPtr oldUnit;
    Slice::UnitPtr newUnit;
    Db* oldDb;
    Db* newDb;
    DbTxn* newDbTxn;

    //
    // Connection to the new environment; not null only if we want to write into the catalog
    //
    Freeze::ConnectionPtr connection;
    string newDbName;

    string facet;
    bool purge;
    ErrorReporterPtr errorReporter;
    TransformDataFactoryPtr factory;
    SymbolTablePtr symbolTable;
    Slice::TypePtr oldKeyType;
    Slice::TypePtr oldValueType;
    Slice::TypePtr newKeyType;
    Slice::TypePtr newValueType;
    TransformMap transformMap;
    RenameMap renameMap;
    ObjectDataMap objectDataMap;
};
typedef IceUtil::Handle<TransformInfoI> TransformInfoIPtr;

//
// TransformDataFactory
//
class TransformDataFactory : public DataFactory
{
public:

    TransformDataFactory(const Ice::CommunicatorPtr&, const Slice::UnitPtr&, const ErrorReporterPtr&);

    void addInitDescriptor(const InitDescriptorPtr&);
    void disableInitializers();
    void enableInitializers();

protected:

    virtual void initialize(const DataPtr&);

private:

    typedef map<string, InitDescriptorPtr> InitMap;
    InitMap _initMap;
    bool _enabled;
};

//
// Descriptor
//
class Descriptor : public IceUtil::SimpleShared
{
public:

    virtual ~Descriptor();

    virtual DescriptorPtr parent() const;
    virtual void addChild(const DescriptorPtr&) = 0;
    virtual void validate() = 0;
    virtual void execute(const SymbolTablePtr&) = 0;

protected:

    Descriptor(const DescriptorPtr&, int, const TransformInfoIPtr&);

    NodePtr parse(const string&) const;

    Slice::TypePtr findType(const Slice::UnitPtr&, const string&);

    DescriptorPtr _parent;
    int _line;
    TransformInfoIPtr _info;
};

//
// SetDescriptor
//
class SetDescriptor : public Descriptor
{
public:

    SetDescriptor(const DescriptorPtr&, int, const TransformInfoIPtr&, const IceXML::Attributes&);

    virtual void addChild(const DescriptorPtr&);
    virtual void validate();
    virtual void execute(const SymbolTablePtr&);

private:

    EntityNodePtr _target;
    NodePtr _value;
    string _valueStr;
    string _type;
    NodePtr _length;
    string _lengthStr;
    bool _convert;
};

//
// DefineDescriptor
//
class DefineDescriptor : public Descriptor
{
public:

    DefineDescriptor(const DescriptorPtr&, int, const TransformInfoIPtr&, const IceXML::Attributes&);

    virtual void addChild(const DescriptorPtr&);
    virtual void validate();
    virtual void execute(const SymbolTablePtr&);

private:

    string _name;
    NodePtr _value;
    string _valueStr;
    Slice::TypePtr _type;
    bool _convert;
};

//
// AddDescriptor
//
class AddDescriptor : public Descriptor
{
public:

    AddDescriptor(const DescriptorPtr&, int, const TransformInfoIPtr&, const IceXML::Attributes&);

    virtual void addChild(const DescriptorPtr&);
    virtual void validate();
    virtual void execute(const SymbolTablePtr&);

private:

    EntityNodePtr _target;
    NodePtr _key;
    string _keyStr;
    NodePtr _index;
    string _indexStr;
    NodePtr _value;
    string _valueStr;
    string _type;
    bool _convert;
};

//
// RemoveDescriptor
//
class RemoveDescriptor : public Descriptor
{
public:

    RemoveDescriptor(const DescriptorPtr&, int, const TransformInfoIPtr&, const IceXML::Attributes&);

    virtual void addChild(const DescriptorPtr&);
    virtual void validate();
    virtual void execute(const SymbolTablePtr&);

private:

    EntityNodePtr _target;
    NodePtr _key;
    string _keyStr;
    NodePtr _index;
    string _indexStr;
};

//
// DeleteDescriptor
//
class DeleteDescriptor : public Descriptor
{
public:

    DeleteDescriptor(const DescriptorPtr&, int, const TransformInfoIPtr&, const IceXML::Attributes&);

    virtual void addChild(const DescriptorPtr&);
    virtual void validate();
    virtual void execute(const SymbolTablePtr&);
};

//
// FailDescriptor
//
class FailDescriptor : public Descriptor
{
public:

    FailDescriptor(const DescriptorPtr&, int, const TransformInfoIPtr&, const IceXML::Attributes&);

    virtual void addChild(const DescriptorPtr&);
    virtual void validate();
    virtual void execute(const SymbolTablePtr&);

private:

    NodePtr _test;
    string _testStr;
    string _message;
};

//
// EchoDescriptor
//
class EchoDescriptor : public Descriptor
{
public:

    EchoDescriptor(const DescriptorPtr&, int, const TransformInfoIPtr&, const IceXML::Attributes&);

    virtual void addChild(const DescriptorPtr&);
    virtual void validate();
    virtual void execute(const SymbolTablePtr&);

private:

    string _message;
    NodePtr _value;
    string _valueStr;
};

//
// ExecutableContainerDescriptor
//
class ExecutableContainerDescriptor : public Descriptor
{
public:

    ExecutableContainerDescriptor(const DescriptorPtr&, int, const TransformInfoIPtr&, const IceXML::Attributes&,
                                  const string&);

    virtual void addChild(const DescriptorPtr&);
    virtual void validate();
    virtual void execute(const SymbolTablePtr&);

protected:

    vector<DescriptorPtr> _children;

private:

    string _name;
};

//
// IfDescriptor
//
class IfDescriptor : public ExecutableContainerDescriptor
{
public:

    IfDescriptor(const DescriptorPtr&, int, const TransformInfoIPtr&, const IceXML::Attributes&);

    virtual void execute(const SymbolTablePtr&);

private:

    NodePtr _test;
    string _testStr;
};

//
// IterateDescriptor
//
class IterateDescriptor : public ExecutableContainerDescriptor
{
public:

    IterateDescriptor(const DescriptorPtr&, int, const TransformInfoIPtr&, const IceXML::Attributes&);

    virtual void execute(const SymbolTablePtr&);

private:

    EntityNodePtr _target;
    string _key;
    string _value;
    string _element;
    string _index;
};

//
// TransformDescriptor
//
class TransformDescriptor : public ExecutableContainerDescriptor
{
public:

    TransformDescriptor(const DescriptorPtr&, int, const TransformInfoIPtr&, const IceXML::Attributes&);

    Slice::TypePtr type() const;
    string typeName() const;
    Slice::TypePtr renameType() const;
    string renameTypeName() const;
    bool doDefaultTransform() const;
    bool doBaseTransform() const;

private:

    bool _default;
    bool _base;
    Slice::TypePtr _newType;
    Slice::TypePtr _renameType;
};
typedef IceUtil::Handle<TransformDescriptor> TransformDescriptorPtr;

//
// InitDescriptor
//
class InitDescriptor : public ExecutableContainerDescriptor
{
public:

    InitDescriptor(const DescriptorPtr&, int, const TransformInfoIPtr&, const IceXML::Attributes&);

    void initialize(const DataPtr&);

    string typeName() const;

private:

    Slice::TypePtr _type;
};

//
// RecordDescriptor
//
class RecordDescriptor : public ExecutableContainerDescriptor
{
public:

    RecordDescriptor(const DescriptorPtr&, int, const TransformInfoIPtr&, const IceXML::Attributes&);

    virtual void execute(const SymbolTablePtr&);

private:

    void transformRecord(const Ice::ByteSeq&, const Ice::ByteSeq&, Ice::ByteSeq&, Ice::ByteSeq&);

    Slice::UnitPtr _old;
    Slice::UnitPtr _new;
};
typedef IceUtil::Handle<RecordDescriptor> RecordDescriptorPtr;

//
// DatabaseDescriptor
//
class DatabaseDescriptor : public ExecutableContainerDescriptor
{
public:

    DatabaseDescriptor(const DescriptorPtr&, int, const TransformInfoIPtr&, const IceXML::Attributes&);

    virtual void addChild(const DescriptorPtr&);
    virtual void execute(const SymbolTablePtr&);

    string name() const;

private:

    string _name;
    string _oldKeyName;
    string _oldValueName;
    string _newKeyName;
    string _newValueName;
    RecordDescriptorPtr _record;
};
typedef IceUtil::Handle<DatabaseDescriptor> DatabaseDescriptorPtr;

//
// TransformDBDescriptor
//
class TransformDBDescriptor : public Descriptor
{
public:

    TransformDBDescriptor(int, const TransformInfoIPtr&, const IceXML::Attributes&);
    ~TransformDBDescriptor();

    virtual void addChild(const DescriptorPtr&);
    virtual void validate();
    virtual void execute(const SymbolTablePtr&);

private:

    map<string, DatabaseDescriptorPtr> _databases;
    vector<DescriptorPtr> _children;
};
typedef IceUtil::Handle<TransformDBDescriptor> TransformDBDescriptorPtr;

//
// SymbolTableI
//
class SymbolTableI : public SymbolTable
{
public:

    SymbolTableI(const TransformInfoIPtr&, const SymbolTablePtr& = SymbolTablePtr());

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

    TransformInfoIPtr _info;
    SymbolTablePtr _parent;
    typedef map<string, DataPtr> DataMap;
    DataMap _dataMap;
    DataMap _constantCache;
};
typedef IceUtil::Handle<SymbolTableI> SymbolTableIPtr;

//
// ObjectVisitor
//
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

//
// DescriptorHandler
//
class DescriptorHandler : public IceXML::Handler
{
public:

    DescriptorHandler(const TransformInfoIPtr&);

    virtual void startElement(const string&, const IceXML::Attributes&, int, int);
    virtual void endElement(const string&, int, int);
    virtual void characters(const string&, int, int);
    virtual void error(const string&, int, int);

    TransformDBDescriptorPtr descriptor() const;

private:

    TransformInfoIPtr _info;
    DescriptorPtr _current;
    TransformDBDescriptorPtr _descriptor;
};

void assignOrTransform(const DataPtr&, const DataPtr&, bool, const TransformInfoPtr&);

} // End of namespace FreezeScript

//
// TransformDataFactory
//
FreezeScript::TransformDataFactory::TransformDataFactory(const Ice::CommunicatorPtr& communicator,
                                                         const Slice::UnitPtr& unit,
                                                         const ErrorReporterPtr& errorReporter) :
    DataFactory(communicator, unit, errorReporter), _enabled(true)
{
}

void
FreezeScript::TransformDataFactory::addInitDescriptor(const InitDescriptorPtr& init)
{
    string typeName = init->typeName();
    assert(_initMap.find(typeName) == _initMap.end());
    _initMap.insert(InitMap::value_type(typeName, init));
}

void
FreezeScript::TransformDataFactory::initialize(const DataPtr& data)
{
    if(_enabled)
    {
        string typeName = typeToString(data->getType());
        InitMap::iterator p = _initMap.find(typeName);
        if(p != _initMap.end())
        {
            p->second->initialize(data);
        }
    }
}

void
FreezeScript::TransformDataFactory::disableInitializers()
{
    _enabled = false;
}

void
FreezeScript::TransformDataFactory::enableInitializers()
{
    _enabled = true;
}

//
// Descriptor
//
FreezeScript::Descriptor::Descriptor(const DescriptorPtr& parent, int line, const TransformInfoIPtr& info) :
    _parent(parent), _line(line), _info(info)
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
    return parseExpression(expr, _info->factory, _info->errorReporter);
}

Slice::TypePtr
FreezeScript::Descriptor::findType(const Slice::UnitPtr& u, const string& type)
{
    Slice::TypeList l;

    l = u->lookupType(type, false);
    if(l.empty())
    {
        _info->errorReporter->error("unknown type `" + type + "'");
    }

    return l.front();
}

//
// SetDescriptor
//
FreezeScript::SetDescriptor::SetDescriptor(const DescriptorPtr& parent, int line, const TransformInfoIPtr& info,
                                           const IceXML::Attributes& attributes) :
    Descriptor(parent, line, info)
{
    DescriptorErrorContext ctx(_info->errorReporter, "set", _line);

    IceXML::Attributes::const_iterator p;

    string target;
    p = attributes.find("target");
    if(p == attributes.end())
    {
        _info->errorReporter->error("required attribute `target' is missing");
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
        _info->errorReporter->error("attributes `value' and 'type' are mutually exclusive");
    }

    if(_valueStr.empty() && _type.empty() && _lengthStr.empty())
    {
        _info->errorReporter->error("requires a value for attributes `value', 'type' or 'length'");
    }

    NodePtr node = parse(target);
    _target = EntityNodePtr::dynamicCast(node);
    if(!_target)
    {
        _info->errorReporter->error("`target' attribute is not an entity: `" + target + "'");
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
    DescriptorErrorContext ctx(_info->errorReporter, "set", _line);
    _info->errorReporter->error("child elements are not supported");
}

void
FreezeScript::SetDescriptor::validate()
{
}

void
FreezeScript::SetDescriptor::execute(const SymbolTablePtr& sym)
{
    DescriptorErrorContext ctx(_info->errorReporter, "set", _line);

    DataPtr data = sym->getValue(_target);
    if(data->readOnly())
    {
        ostringstream ostr;
        ostr << _target;
        _info->errorReporter->error("target `" + ostr.str() + "' cannot be modified");
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
            _info->errorReporter->error("evaluation of value `" + _valueStr + "' failed:\n" + ex.reason());
        }
    }

    Destroyer<DataPtr> valueDestroyer;
    if(!_type.empty())
    {
        assert(!value);
        Slice::TypePtr type = sym->lookupType(_type);
        if(!type)
        {
            _info->errorReporter->error("type `" + _type + "' not found");
        }
        value = _info->factory->createObject(type, false);
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
            _info->errorReporter->error("target `" + ostr.str() + "' is not a sequence");
        }

        try
        {
            length = _length->evaluate(sym);
        }
        catch(const EvaluateException& ex)
        {
            _info->errorReporter->error("evaluation of length `" + _lengthStr + "' failed:\n" + ex.reason());
        }

        DataList& elements = seq->getElements();
        Ice::Long l = length->integerValue();
        if(l < 0 || l > INT_MAX)
        {
            _info->errorReporter->error("sequence length " + length->toString() + " is out of range");
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
                DataPtr v = _info->factory->create(elemType, false);
                if(value)
                {
                    assignOrTransform(v, value, _convert, _info);
                }
                elements.push_back(v);
            }
        }
    }
    else
    {
        assignOrTransform(data, value, _convert, _info);
    }
}

//
// DefineDescriptor
//
FreezeScript::DefineDescriptor::DefineDescriptor(const DescriptorPtr& parent, int line, const TransformInfoIPtr& info,
                                                 const IceXML::Attributes& attributes) :
    Descriptor(parent, line, info)
{
    DescriptorErrorContext ctx(_info->errorReporter, "define", _line);

    IceXML::Attributes::const_iterator p;

    p = attributes.find("name");
    if(p == attributes.end())
    {
        _info->errorReporter->error("required attribute `name' is missing");
    }
    _name = p->second;

    p = attributes.find("type");
    if(p == attributes.end())
    {
        _info->errorReporter->error("required attribute `type' is missing");
    }
    if(p->second.find("::New") != string::npos)
    {
        Slice::TypeList l = _info->newUnit->lookupType(p->second.substr(5), false);
        if(l.empty())
        {
            _info->errorReporter->error("type `" + p->second + "' not found");
        }
        _type = l.front();
    }
    else if(p->second.find("::Old") != string::npos)
    {
        Slice::TypeList l = _info->oldUnit->lookupType(p->second.substr(5), false);
        if(l.empty())
        {
            _info->errorReporter->error("type `" + p->second + "' not found");
        }
        _type = l.front();
    }
    else
    {
        Slice::TypeList l = _info->newUnit->lookupType(p->second, false);
        if(l.empty())
        {
            _info->errorReporter->error("type `" + p->second + "' not found");
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
    DescriptorErrorContext ctx(_info->errorReporter, "define", _line);
    _info->errorReporter->error("child elements are not supported");
}

void
FreezeScript::DefineDescriptor::validate()
{
}

void
FreezeScript::DefineDescriptor::execute(const SymbolTablePtr& sym)
{
    DescriptorErrorContext ctx(_info->errorReporter, "define", _line);

    DataPtr data = _info->factory->create(_type, false);

    DataPtr value;
    if(_value)
    {
        try
        {
            value = _value->evaluate(sym);
        }
        catch(const EvaluateException& ex)
        {
            _info->errorReporter->error("evaluation of value `" + _valueStr + "' failed:\n" + ex.reason());
        }
    }

    if(value)
    {
        assignOrTransform(data, value, _convert, _info);
    }

    sym->add(_name, data);
}

//
// AddDescriptor
//
FreezeScript::AddDescriptor::AddDescriptor(const DescriptorPtr& parent, int line, const TransformInfoIPtr& info,
                                           const IceXML::Attributes& attributes) :
    Descriptor(parent, line, info)
{
    DescriptorErrorContext ctx(_info->errorReporter, "add", _line);

    IceXML::Attributes::const_iterator p;

    string target;
    p = attributes.find("target");
    if(p == attributes.end())
    {
        _info->errorReporter->error("required attribute `target' is missing");
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

    if(!_keyStr.empty() && !_indexStr.empty())
    {
        _info->errorReporter->error("attributes `key' and `index' are mutually exclusive");
    }

    if(_keyStr.empty() && _indexStr.empty())
    {
        _info->errorReporter->error("one of attributes `key' or `index' is required");
    }

    if(!_valueStr.empty() && !_type.empty())
    {
        _info->errorReporter->error("attributes `value' and `type' are mutually exclusive");
    }

    NodePtr node = parse(target);
    _target = EntityNodePtr::dynamicCast(node);
    if(!_target)
    {
        _info->errorReporter->error("`target' attribute is not an entity: `" + target + "'");
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
    DescriptorErrorContext ctx(_info->errorReporter, "add", _line);
    _info->errorReporter->error("child elements are not supported");
}

void
FreezeScript::AddDescriptor::validate()
{
}

void
FreezeScript::AddDescriptor::execute(const SymbolTablePtr& sym)
{
    DescriptorErrorContext ctx(_info->errorReporter, "add", _line);

    DataPtr data = sym->getValue(_target);
    if(data->readOnly())
    {
        ostringstream ostr;
        ostr << _target;
        _info->errorReporter->error("target `" + ostr.str() + "' cannot be modified");
    }

    if(_key)
    {
        DictionaryDataPtr dict = DictionaryDataPtr::dynamicCast(data);
        if(!dict)
        {
            ostringstream ostr;
            ostr << _target;
            _info->errorReporter->error("target `" + ostr.str() + "' is not a dictionary");
        }

        Slice::DictionaryPtr type = Slice::DictionaryPtr::dynamicCast(dict->getType());
        assert(type);

        DataPtr key;
        Destroyer<DataPtr> keyDestroyer;
        try
        {
            DataPtr v = _key->evaluate(sym);
            key = _info->factory->create(type->keyType(), false);
            keyDestroyer.set(key);
            assignOrTransform(key, v, _convert, _info);
        }
        catch(const EvaluateException& ex)
        {
            _info->errorReporter->error("evaluation of key `" + _keyStr + "' failed:\n" + ex.reason());
        }

        if(dict->getElement(key))
        {
            ostringstream ostr;
            printData(key, ostr);
            _info->errorReporter->error("key " + ostr.str() + " already exists in dictionary");
        }

        DataPtr elem = _info->factory->create(type->valueType(), false);
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
                _info->errorReporter->error("evaluation of value `" + _valueStr + "' failed:\n" + ex.reason());
            }
        }

        Destroyer<DataPtr> valueDestroyer;
        if(!_type.empty())
        {
            assert(!value);
            Slice::TypePtr type = sym->lookupType(_type);
            if(!type)
            {
                _info->errorReporter->error("type `" + _type + "' not found");
            }
            value = _info->factory->createObject(type, false);
            valueDestroyer.set(value);
        }

        if(value)
        {
            assignOrTransform(elem, value, _convert, _info);
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
            _info->errorReporter->error("target `" + ostr.str() + "' is not a sequence");
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
            _info->errorReporter->error("evaluation of index `" + _indexStr + "' failed:\n" + ex.reason());
        }

        DataList& elements = seq->getElements();
        Ice::Long l = index->integerValue();
        DataList::size_type i = static_cast<DataList::size_type>(l);
        if(l < 0 || l > INT_MAX || i > elements.size())
        {
            _info->errorReporter->error("sequence index " + index->toString() + " is out of range");
        }

        DataPtr elem = _info->factory->create(type->type(), false);
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
                _info->errorReporter->error("evaluation of value `" + _valueStr + "' failed:\n" + ex.reason());
            }
        }

        Destroyer<DataPtr> valueDestroyer;
        if(!_type.empty())
        {
            assert(!value);
            Slice::TypePtr type = sym->lookupType(_type);
            if(!type)
            {
                _info->errorReporter->error("type `" + _type + "' not found");
            }
            value = _info->factory->createObject(type, false);
            valueDestroyer.set(value);
        }

        if(value)
        {
            assignOrTransform(elem, value, _convert, _info);
        }

        elements.insert(elements.begin() + i, elem);
        indexDestroyer.release();
        elemDestroyer.release();
    }
}

//
// RemoveDescriptor
//
FreezeScript::RemoveDescriptor::RemoveDescriptor(const DescriptorPtr& parent, int line, const TransformInfoIPtr& info,
                                                 const IceXML::Attributes& attributes) :
    Descriptor(parent, line, info)
{
    DescriptorErrorContext ctx(_info->errorReporter, "remove", _line);

    IceXML::Attributes::const_iterator p;

    string target;
    p = attributes.find("target");
    if(p == attributes.end())
    {
        _info->errorReporter->error("required attribute `target' is missing");
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

    NodePtr node = parse(target);
    _target = EntityNodePtr::dynamicCast(node);
    if(!_target)
    {
        _info->errorReporter->error("`target' attribute is not an entity: `" + target + "'");
    }

    if(!_keyStr.empty() && !_indexStr.empty())
    {
        _info->errorReporter->error("attributes `key' and `index' are mutually exclusive");
    }

    if(_keyStr.empty() && _indexStr.empty())
    {
        _info->errorReporter->error("one of attributes `key' or `index' is required");
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
    DescriptorErrorContext ctx(_info->errorReporter, "remove", _line);
    _info->errorReporter->error("child elements are not supported");
}

void
FreezeScript::RemoveDescriptor::validate()
{
}

void
FreezeScript::RemoveDescriptor::execute(const SymbolTablePtr& sym)
{
    DescriptorErrorContext ctx(_info->errorReporter, "remove", _line);

    DataPtr data = sym->getValue(_target);
    if(data->readOnly())
    {
        ostringstream ostr;
        ostr << _target;
        _info->errorReporter->error("target `" + ostr.str() + "' cannot be modified");
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
            _info->errorReporter->error("evaluation of key `" + _keyStr + "' failed:\n" + ex.reason());
        }

        DictionaryDataPtr dict = DictionaryDataPtr::dynamicCast(data);
        if(!dict)
        {
            ostringstream ostr;
            ostr << _target;
            _info->errorReporter->error("target `" + ostr.str() + "' is not a dictionary");
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
            _info->errorReporter->error("evaluation of index `" + _indexStr + "' failed:\n" + ex.reason());
        }

        SequenceDataPtr seq = SequenceDataPtr::dynamicCast(data);
        if(!seq)
        {
            ostringstream ostr;
            ostr << _target;
            _info->errorReporter->error("target `" + ostr.str() + "' is not a sequence");
        }

        DataList& elements = seq->getElements();
        Ice::Long l = index->integerValue();
        DataList::size_type i = static_cast<DataList::size_type>(l);
        if(l < 0 || l > INT_MAX || i >= elements.size())
        {
            _info->errorReporter->error("sequence index " + index->toString() + " is out of range");
        }

        elements[i]->destroy();
        elements.erase(elements.begin() + i);
    }
}

//
// DeleteDescriptor
//
FreezeScript::DeleteDescriptor::DeleteDescriptor(const DescriptorPtr& parent, int line, const TransformInfoIPtr& info,
                                                 const IceXML::Attributes&) :
    Descriptor(parent, line, info)
{
}

void
FreezeScript::DeleteDescriptor::addChild(const DescriptorPtr&)
{
    DescriptorErrorContext ctx(_info->errorReporter, "delete", _line);
    _info->errorReporter->error("child elements are not supported");
}

void
FreezeScript::DeleteDescriptor::validate()
{
}

void
FreezeScript::DeleteDescriptor::execute(const SymbolTablePtr&)
{
    throw DeleteRecordException();
}

//
// FailDescriptor
//
FreezeScript::FailDescriptor::FailDescriptor(const DescriptorPtr& parent, int line, const TransformInfoIPtr& info,
                                             const IceXML::Attributes& attributes) :
    Descriptor(parent, line, info)
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
    DescriptorErrorContext ctx(_info->errorReporter, "fail", _line);
    _info->errorReporter->error("child elements are not supported");
}

void
FreezeScript::FailDescriptor::validate()
{
}

void
FreezeScript::FailDescriptor::execute(const SymbolTablePtr& sym)
{
    DescriptorErrorContext ctx(_info->errorReporter, "fail", _line);

    if(_test)
    {
        try
        {
            DataPtr b = _test->evaluate(sym);
            BooleanDataPtr bd = BooleanDataPtr::dynamicCast(b);
            if(!bd)
            {
                _info->errorReporter->error("expression `" + _testStr + "' does not evaluate to a boolean");
            }
            if(!bd->booleanValue())
            {
                return;
            }
        }
        catch(const EvaluateException& ex)
        {
            _info->errorReporter->error("evaluation of expression `" + _testStr + "' failed:\n" + ex.reason());
        }
    }

    throw FailureException(__FILE__, __LINE__, _message);
}

//
// EchoDescriptor
//
FreezeScript::EchoDescriptor::EchoDescriptor(const DescriptorPtr& parent, int line, const TransformInfoIPtr& info,
                                             const IceXML::Attributes& attributes) :
    Descriptor(parent, line, info)
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
    DescriptorErrorContext ctx(_info->errorReporter, "echo", _line);
    _info->errorReporter->error("child elements are not supported");
}

void
FreezeScript::EchoDescriptor::validate()
{
}

void
FreezeScript::EchoDescriptor::execute(const SymbolTablePtr& sym)
{
    DescriptorErrorContext ctx(_info->errorReporter, "echo", _line);

    ostream& out = _info->errorReporter->stream();

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
            _info->errorReporter->error("evaluation of value `" + _valueStr + "' failed:\n" + ex.reason());
        }
        printData(v, out);
    }

    out << endl;
}

//
// ExecutableContainerDescriptor
//
FreezeScript::ExecutableContainerDescriptor::ExecutableContainerDescriptor(const DescriptorPtr& parent, int line,
                                                                           const TransformInfoIPtr& info,
                                                                           const IceXML::Attributes&,
                                                                           const string& name) :
    Descriptor(parent, line, info), _name(name)
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
FreezeScript::ExecutableContainerDescriptor::execute(const SymbolTablePtr& sym)
{
    for(vector<DescriptorPtr>::iterator p = _children.begin(); p != _children.end(); ++p)
    {
        (*p)->execute(sym);
    }
}

//
// IfDescriptor
//
FreezeScript::IfDescriptor::IfDescriptor(const DescriptorPtr& parent, int line, const TransformInfoIPtr& info,
                                         const IceXML::Attributes& attributes) :
    ExecutableContainerDescriptor(parent, line, info, attributes, "if")
{
    DescriptorErrorContext ctx(_info->errorReporter, "if", _line);

    IceXML::Attributes::const_iterator p = attributes.find("test");
    if(p == attributes.end())
    {
        _info->errorReporter->error("required attribute `test' is missing");
    }
    _testStr = p->second;

    _test = parse(_testStr);
}

void
FreezeScript::IfDescriptor::execute(const SymbolTablePtr& sym)
{
    DescriptorErrorContext ctx(_info->errorReporter, "if", _line);

    try
    {
        DataPtr b = _test->evaluate(sym);
        BooleanDataPtr bd = BooleanDataPtr::dynamicCast(b);
        if(!bd)
        {
            _info->errorReporter->error("expression `" + _testStr + "' does not evaluate to a boolean");
        }
        if(bd->booleanValue())
        {
            ExecutableContainerDescriptor::execute(sym);
        }
    }
    catch(const EvaluateException& ex)
    {
        _info->errorReporter->error("evaluation of conditional expression `" + _testStr + "' failed:\n" + ex.reason());
    }
}

//
// IterateDescriptor
//
FreezeScript::IterateDescriptor::IterateDescriptor(const DescriptorPtr& parent, int line,
                                                   const TransformInfoIPtr& info,
                                                   const IceXML::Attributes& attributes) :
    ExecutableContainerDescriptor(parent, line, info, attributes, "iterate")
{
    DescriptorErrorContext ctx(_info->errorReporter, "iterate", _line);

    IceXML::Attributes::const_iterator p;
    string target;

    p = attributes.find("target");
    if(p == attributes.end())
    {
        _info->errorReporter->error("required attribute `target' is missing");
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
        _info->errorReporter->error("`target' attribute is not an entity: `" + target + "'");
    }
}

void
FreezeScript::IterateDescriptor::execute(const SymbolTablePtr& sym)
{
    DescriptorErrorContext ctx(_info->errorReporter, "iterate", _line);

    DataPtr data = sym->getValue(_target);

    DictionaryDataPtr dict = DictionaryDataPtr::dynamicCast(data);
    SequenceDataPtr seq = SequenceDataPtr::dynamicCast(data);
    if(!dict && !seq)
    {
        ostringstream ostr;
        ostr << _target;
        _info->errorReporter->error("target `" + ostr.str() + "' is not a dictionary or sequence");
    }

    if(dict)
    {
        if(!_element.empty())
        {
            _info->errorReporter->error("attribute `element' specified for dictionary target");
        }
        if(!_index.empty())
        {
            _info->errorReporter->error("attribute `index' specified for dictionary target");
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
            ExecutableContainerDescriptor::execute(elemSym);
        }
    }
    else
    {
        if(!_key.empty())
        {
            _info->errorReporter->error("attribute `key' specified for sequence target");
        }
        if(!_value.empty())
        {
            _info->errorReporter->error("attribute `value' specified for sequence target");
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
            elemSym->add(index, _info->factory->createInteger(i, true));
            ExecutableContainerDescriptor::execute(elemSym);
            ++i;
        }
    }
}

//
// TransformDescriptor
//
FreezeScript::TransformDescriptor::TransformDescriptor(const DescriptorPtr& parent, int line,
                                                       const TransformInfoIPtr& info,
                                                       const IceXML::Attributes& attributes) :
    ExecutableContainerDescriptor(parent, line, info, attributes, "transform"), _default(true), _base(true)
{
    DescriptorErrorContext ctx(_info->errorReporter, "transform", _line);

    IceXML::Attributes::const_iterator p;

    string type, rename;

    p = attributes.find("type");
    if(p == attributes.end())
    {
        _info->errorReporter->error("required attribute `type' is missing");
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

    l = _info->newUnit->lookupType(type, false);
    if(l.empty())
    {
        _info->errorReporter->error("unable to find type `" + type + "' in new Slice definitions");
    }
    else
    {
        _newType = l.front();
    }

    if(!rename.empty())
    {
        l = _info->oldUnit->lookupType(rename, false);
        if(l.empty())
        {
            _info->errorReporter->error("unable to find type `" + rename + "' in old Slice definitions");
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
FreezeScript::InitDescriptor::InitDescriptor(const DescriptorPtr& parent, int line, const TransformInfoIPtr& info,
                                             const IceXML::Attributes& attributes) :
    ExecutableContainerDescriptor(parent, line, info, attributes, "init")
{
    DescriptorErrorContext ctx(_info->errorReporter, "init", _line);

    IceXML::Attributes::const_iterator p = attributes.find("type");

    string type;

    if(p == attributes.end())
    {
        _info->errorReporter->error("required attribute `type' is missing");
    }
    type = p->second;

    Slice::TypeList l = _info->newUnit->lookupType(type, false);
    if(l.empty())
    {
        _info->errorReporter->error("unable to find type `" + type + "' in new Slice definitions");
    }
    else
    {
        _type = l.front();
    }
}

void
FreezeScript::InitDescriptor::initialize(const DataPtr& data)
{
    SymbolTablePtr sym = new SymbolTableI(_info, _info->symbolTable);
    sym->add("value", data);
    execute(sym);
}

string
FreezeScript::InitDescriptor::typeName() const
{
    return typeToString(_type);
}

//
// RecordDescriptor
//
FreezeScript::RecordDescriptor::RecordDescriptor(const DescriptorPtr& parent, int line, const TransformInfoIPtr& info,
                                                 const IceXML::Attributes& attributes) :
    ExecutableContainerDescriptor(parent, line, info, attributes, "record")
{
}

void
FreezeScript::RecordDescriptor::execute(const SymbolTablePtr& /*sym*/)
{
    //
    // Temporarily add an object factory.
    //
    _info->objectFactory->activate(_info->factory, _info->oldUnit);

    //
    // Iterate over the database.
    //
    Dbc* dbc = 0;
    _info->oldDb->cursor(0, &dbc, 0);
    try
    {
        Dbt dbKey, dbValue;
        while(dbc->get(&dbKey, &dbValue, DB_NEXT) == 0)
        {
            Ice::ByteSeq inKeyBytes;
            inKeyBytes.resize(dbKey.get_size());
            memcpy(&inKeyBytes[0], dbKey.get_data(), dbKey.get_size());

            Ice::ByteSeq inValueBytes;
            inValueBytes.resize(dbValue.get_size());
            memcpy(&inValueBytes[0], dbValue.get_data(), dbValue.get_size());

            try
            {
                Ice::ByteSeq outKeyBytes, outValueBytes;
                transformRecord(inKeyBytes, inValueBytes, outKeyBytes, outValueBytes);
                Dbt dbNewKey(&outKeyBytes[0], static_cast<unsigned>(outKeyBytes.size())),
                             dbNewValue(&outValueBytes[0], static_cast<unsigned>(outValueBytes.size()));
                if(_info->newDb->put(_info->newDbTxn, &dbNewKey, &dbNewValue, DB_NOOVERWRITE) == DB_KEYEXIST)
                {
                    _info->errorReporter->error("duplicate key encountered");
                }
            }
            catch(const DeleteRecordException&)
            {
                // The record is deleted simply by not adding it to the new database.
            }
            catch(const ClassNotFoundException& ex)
            {
                if(!_info->purge)
                {
                    _info->errorReporter->error("class " + ex.id + " not found in new Slice definitions");
                }
                else
                {
                    // The record is deleted simply by not adding it to the new database.
                    _info->errorReporter->warning("purging database record due to missing class type " + ex.id);
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
        _info->objectFactory->deactivate();
        throw;
    }

    if(dbc)
    {
        dbc->close();
    }
    _info->objectFactory->deactivate();
}

void
FreezeScript::RecordDescriptor::transformRecord(const Ice::ByteSeq& inKeyBytes,
                                                const Ice::ByteSeq& inValueBytes,
                                                Ice::ByteSeq& outKeyBytes,
                                                Ice::ByteSeq& outValueBytes)
{
    Ice::InputStreamPtr inKey = Ice::wrapInputStream(_info->communicator, inKeyBytes);
    Ice::InputStreamPtr inValue = Ice::wrapInputStream(_info->communicator, inValueBytes);
    inValue->startEncapsulation();

    Ice::OutputStreamPtr outKey = Ice::createOutputStream(_info->communicator);
    Ice::OutputStreamPtr outValue = Ice::createOutputStream(_info->communicator);
    outValue->startEncapsulation();

    //
    // Create data representations of the old key and value types.
    //
    _info->factory->disableInitializers();
    DataPtr oldKeyData = _info->factory->create(_info->oldKeyType, true);
    Destroyer<DataPtr> oldKeyDataDestroyer(oldKeyData);
    DataPtr oldValueData = _info->factory->create(_info->oldValueType, true);
    Destroyer<DataPtr> oldValueDataDestroyer(oldValueData);

    //
    // Unmarshal the old key and value.
    //
    oldKeyData->unmarshal(inKey);
    oldValueData->unmarshal(inValue);
    _info->objectDataMap.clear();
    if(_info->oldValueType->usesClasses())
    {
        inValue->readPendingObjects();
        ObjectVisitor visitor(_info->objectDataMap);
        oldValueData->visit(visitor);
    }
    _info->factory->enableInitializers();

    //
    // Create data representations of the new key and value types.
    //
    DataPtr newKeyData = _info->factory->create(_info->newKeyType, false);
    Destroyer<DataPtr> newKeyDataDestroyer(newKeyData);
    DataPtr newValueData = _info->factory->create(_info->newValueType, false);
    Destroyer<DataPtr> newValueDataDestroyer(newValueData);
    DataPtr facetData = _info->factory->createString(_info->facet, true);
    Destroyer<DataPtr> facetDataDestroyer(facetData);

    //
    // Copy the data from the old key and value to the new key and value, if possible.
    //
    TransformVisitor keyVisitor(oldKeyData, _info);
    newKeyData->visit(keyVisitor);
    TransformVisitor valueVisitor(oldValueData, _info);
    newValueData->visit(valueVisitor);

    if(!_children.empty())
    {
        //
        // Execute the child descriptors.
        //
        // TODO: Revisit identifiers.
        //
        SymbolTablePtr st = new SymbolTableI(_info, _info->symbolTable);
        st->add("oldkey", oldKeyData);
        st->add("newkey", newKeyData);
        st->add("oldvalue", oldValueData);
        st->add("newvalue", newValueData);
        st->add("facet", facetData);
        ExecutableContainerDescriptor::execute(st);
    }

    newKeyData->marshal(outKey);
    newValueData->marshal(outValue);

    outKey->finished(outKeyBytes);

    if(_info->newValueType->usesClasses())
    {
        outValue->writePendingObjects();
    }
    outValue->endEncapsulation();
    outValue->finished(outValueBytes);
}

//
// DatabaseDescriptor
//
FreezeScript::DatabaseDescriptor::DatabaseDescriptor(const DescriptorPtr& parent, int line,
                                                     const TransformInfoIPtr& info,
                                                     const IceXML::Attributes& attributes) :
    ExecutableContainerDescriptor(parent, line, info, attributes, "database")
{
    DescriptorErrorContext ctx(_info->errorReporter, "database", _line);

    IceXML::Attributes::const_iterator p;

    p = attributes.find("name");
    if(p != attributes.end())
    {
        _name = p->second;
    }

    p = attributes.find("key");
    if(p == attributes.end())
    {
        _info->errorReporter->error("required attribute `key' is missing");
    }
    string keyTypes = p->second;

    p = attributes.find("value");
    if(p == attributes.end())
    {
        _info->errorReporter->error("required attribute `value' is missing");
    }
    string valueTypes = p->second;

    string::size_type pos;

    pos = keyTypes.find(',');
    if(pos == 0 || pos == keyTypes.size())
    {
        _info->errorReporter->error("invalid key type specification `" + keyTypes +"'");
    }
    if(pos == string::npos)
    {
        _oldKeyName = keyTypes;
        _newKeyName = keyTypes;
    }
    else
    {
        _oldKeyName = keyTypes.substr(0, pos);
        _newKeyName = keyTypes.substr(pos + 1);
    }

    pos = valueTypes.find(',');
    if(pos == 0 || pos == valueTypes.size())
    {
        _info->errorReporter->error("invalid value type specification `" + valueTypes +"'");
    }
    if(pos == string::npos)
    {
        _oldValueName = valueTypes;
        _newValueName = valueTypes;
    }
    else
    {
        _oldValueName = valueTypes.substr(0, pos);
        _newValueName = valueTypes.substr(pos + 1);
    }
}

void
FreezeScript::DatabaseDescriptor::addChild(const DescriptorPtr& child)
{
    DescriptorErrorContext ctx(_info->errorReporter, "database", _line);

    RecordDescriptorPtr rec = RecordDescriptorPtr::dynamicCast(child);
    if(rec)
    {
        if(_record)
        {
            _info->errorReporter->error("only one <record> element can be specified");
        }
        _record = rec;
    }

    ExecutableContainerDescriptor::addChild(child);
}

void
FreezeScript::DatabaseDescriptor::execute(const SymbolTablePtr& st)
{
    DescriptorErrorContext ctx(_info->errorReporter, "database", _line);

    //
    // Look up the Slice definitions for the key and value types.
    //
    _info->oldKeyType = findType(_info->oldUnit, _oldKeyName);
    _info->newKeyType = findType(_info->newUnit, _newKeyName);
    _info->oldValueType = findType(_info->oldUnit, _oldValueName);
    _info->newValueType = findType(_info->newUnit, _newValueName);

    if(_info->connection != 0)
    {
        Freeze::Catalog catalog(_info->connection, Freeze::catalogName());
        Freeze::CatalogData catalogData;
        catalogData.evictor = false;
        catalogData.key = _info->newKeyType->typeId(); 
        catalogData.value = _info->newValueType->typeId();
        catalog.put(Freeze::Catalog::value_type(_info->newDbName, catalogData));
    }

    //
    // TODO: it looks like _info is not destroyed before the new dbEnv is closed.
    //
    _info->connection = 0;

    ExecutableContainerDescriptor::execute(st);
}

string
FreezeScript::DatabaseDescriptor::name() const
{
    return _name;
}

//
// TransformDBDescriptor
//
FreezeScript::TransformDBDescriptor::TransformDBDescriptor(int line, const TransformInfoIPtr& info,
                                                           const IceXML::Attributes&) :
    Descriptor(0, line, info)
{
}

FreezeScript::TransformDBDescriptor::~TransformDBDescriptor()
{
}

void
FreezeScript::TransformDBDescriptor::addChild(const DescriptorPtr& child)
{
    DescriptorErrorContext ctx(_info->errorReporter, "transformdb", _line);

    DatabaseDescriptorPtr db = DatabaseDescriptorPtr::dynamicCast(child);
    TransformDescriptorPtr transform = TransformDescriptorPtr::dynamicCast(child);
    InitDescriptorPtr init = InitDescriptorPtr::dynamicCast(child);

    if(db)
    {
        string name = db->name();
        map<string, DatabaseDescriptorPtr>::iterator p = _databases.find(name);
        if(p != _databases.end())
        {
            if(name.empty())
            {
                _info->errorReporter->error("duplicate <database> element");
            }
            else
            {
                _info->errorReporter->error(string("duplicate <database> element for ") + name);
            }
        }
        else
        {
            _databases[name] = db;
            _children.push_back(db);
        }
    }
    else if(transform)
    {
        string name = transform->typeName();
        TransformMap::iterator p = _info->transformMap.find(name);
        if(p != _info->transformMap.end())
        {
            _info->errorReporter->error("transform `" + name + "' specified more than once");
        }
        _info->transformMap.insert(TransformMap::value_type(name, transform));

        string renameTypeName = transform->renameTypeName();
        if(!renameTypeName.empty())
        {
            RenameMap::iterator q = _info->renameMap.find(renameTypeName);
            if(q != _info->renameMap.end())
            {
                _info->errorReporter->error("multiple transform descriptors specify the rename value `" +
                                            renameTypeName + "'");
            }
            _info->renameMap.insert(RenameMap::value_type(renameTypeName, transform->type()));
        }

        _children.push_back(transform);
    }
    else if(init)
    {
        _info->factory->addInitDescriptor(init);
        _children.push_back(init);
    }
    else
    {
        _info->errorReporter->error("invalid child element");
    }
}

void
FreezeScript::TransformDBDescriptor::validate()
{
    DescriptorErrorContext ctx(_info->errorReporter, "transformdb", _line);

    if(_databases.empty())
    {
        _info->errorReporter->error("no <database> element defined");
    }

    for(vector<DescriptorPtr>::iterator p = _children.begin(); p != _children.end(); ++p)
    {
        (*p)->validate();
    }
}

void
FreezeScript::TransformDBDescriptor::execute(const SymbolTablePtr& sym)
{
    map<string, DatabaseDescriptorPtr>::iterator p = _databases.find(_info->newDbName);
    if(p == _databases.end())
    {
        p = _databases.find("");
    }

    if(p == _databases.end())
    {
        _info->errorReporter->error("no <database> element found for `" + _info->newDbName + "'");
    }

    p->second->execute(sym);
}

//
// SymbolTableI
//
FreezeScript::SymbolTableI::SymbolTableI(const TransformInfoIPtr& info, const SymbolTablePtr& parent) :
    _info(info), _parent(parent)
{
}

void
FreezeScript::SymbolTableI::add(const string& name, const DataPtr& data)
{
    DataMap::const_iterator p = _dataMap.find(name);
    if(p != _dataMap.end())
    {
        _info->errorReporter->error("`" + name + "' is already defined");
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
        _info->errorReporter->error(ostr.str());
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
            unit = _info->oldUnit;
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
                unit = _info->newUnit;
            }
        }
    }

    if(key.empty())
    {
        _info->errorReporter->error("invalid constant name `" + name + "'");
    }

    Slice::ContainedList l = unit->findContents(key);
    if(l.empty())
    {
        _info->errorReporter->error("unknown constant `" + name + "'");
    }

    Slice::EnumeratorPtr e = Slice::EnumeratorPtr::dynamicCast(l.front());
    Slice::ConstPtr c = Slice::ConstPtr::dynamicCast(l.front());
    if(!e && !c)
    {
        _info->errorReporter->error("`" + name + "' does not refer to a Slice constant or enumerator");
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
                result = _info->factory->createInteger(n, true);
                break;
            }

            case Slice::Builtin::KindBool:
            {
                result = _info->factory->createBoolean(value == "true", true);
                break;
            }

            case Slice::Builtin::KindFloat:
            case Slice::Builtin::KindDouble:
            {
                double v = strtod(value.c_str(), 0);
                result = _info->factory->createDouble(v, true);
                break;
            }

            case Slice::Builtin::KindString:
            {
                result = _info->factory->createString(value, true);
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
        result = _info->factory->create(e->type(), true);
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
    return new SymbolTableI(_info, this);
}

Slice::TypePtr
FreezeScript::SymbolTableI::lookupType(const string& name)
{
    Slice::TypeList l = _info->newUnit->lookupType(name, false);
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
                _info->errorReporter->error("containsKey() requires one argument");
            }
            Slice::DictionaryPtr dictType = Slice::DictionaryPtr::dynamicCast(targetDict->getType());
            assert(dictType);
            DataPtr key = _info->factory->create(dictType->keyType(), false);
            assignOrTransform(key, args[0], false, _info);
            DataPtr value = targetDict->getElement(key);
            return _info->factory->createBoolean(value ? true : false, false);
        }

        DataPtr result;
        if(invokeMemberFunction(name, target, args, result, _info->factory, _info->errorReporter))
        {
            return result;
        }

        _info->errorReporter->error("unknown function `" + name + "' invoked on type " +
                                    typeToString(target->getType()));
    }
    else
    {
        //
        // Global function.
        //
        DataPtr result;
        if(invokeGlobalFunction(_info->communicator, name, args, result, _info->factory, _info->errorReporter))
        {
            return result;
        }
        else
        {
            _info->errorReporter->error("unknown global function `" + name + "'");
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
#if (defined(_MSC_VER) && (_MSC_VER >= 1600))
            _map.insert(ObjectDataMap::value_type(value.get(), static_cast<ObjectDataPtr>(nullptr)));
#else
            _map.insert(ObjectDataMap::value_type(value.get(), 0));
#endif
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
FreezeScript::DataFactoryPtr
FreezeScript::TransformInfoI::getDataFactory()
{
    return factory;
}

FreezeScript::ErrorReporterPtr
FreezeScript::TransformInfoI::getErrorReporter()
{
    return errorReporter;
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
        Slice::TypePtr cls = data->getType(); // Actual type: may be Builtin (Object) or ClassDecl
        bool transformBase = true;
        while(cls)
        {
            string type = typeToString(cls);
            TransformMap::const_iterator p = transformMap.find(type);
            if(p != transformMap.end())
            {
                SymbolTablePtr sym = new SymbolTableI(this, symbolTable);
                sym->add("new", dest);
                sym->add("old", src);
                p->second->execute(sym);
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
            SymbolTablePtr sym = new SymbolTableI(this, symbolTable);
            sym->add("new", dest);
            sym->add("old", src);
            p->second->execute(sym);
        }
    }
}

string
FreezeScript::TransformInfoI::facetName()
{
    return facet;
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
// DescriptorHandler
//
FreezeScript::DescriptorHandler::DescriptorHandler(const TransformInfoIPtr& info) :
    _info(info)
{
}

void
FreezeScript::DescriptorHandler::startElement(const string& name, const IceXML::Attributes& attributes, int line,
                                              int /*column*/)
{
    DescriptorPtr d;

    if(name == "transformdb")
    {
        if(_current)
        {
            _info->errorReporter->descriptorError("<transformdb> must be the top-level element", line);
        }

        _descriptor = new TransformDBDescriptor(line, _info, attributes);
        d = _descriptor;
    }
    else if(name == "database")
    {
        if(!_current)
        {
            _info->errorReporter->descriptorError("<database> must be a child of <transformdb>", line);
        }

        d = new DatabaseDescriptor(_current, line, _info, attributes);
    }
    else if(name == "record")
    {
        if(!_current)
        {
            _info->errorReporter->descriptorError("<record> must be a child of <database>", line);
        }

        d = new RecordDescriptor(_current, line, _info, attributes);
    }
    else if(name == "transform")
    {
        if(!_current)
        {
            _info->errorReporter->descriptorError("<transform> must be a child of <transformdb>", line);
        }

        d = new TransformDescriptor(_current, line, _info, attributes);
    }
    else if(name == "init")
    {
        if(!_current)
        {
            _info->errorReporter->descriptorError("<init> must be a child of <transformdb>", line);
        }

        d = new InitDescriptor(_current, line, _info, attributes);
    }
    else if(name == "set")
    {
        if(!_current)
        {
            _info->errorReporter->descriptorError("<set> cannot be a top-level element", line);
        }

        d = new SetDescriptor(_current, line, _info, attributes);
    }
    else if(name == "define")
    {
        if(!_current)
        {
            _info->errorReporter->descriptorError("<define> cannot be a top-level element", line);
        }

        d = new DefineDescriptor(_current, line, _info, attributes);
    }
    else if(name == "add")
    {
        if(!_current)
        {
            _info->errorReporter->descriptorError("<add> cannot be a top-level element", line);
        }

        d = new AddDescriptor(_current, line, _info, attributes);
    }
    else if(name == "remove")
    {
        if(!_current)
        {
            _info->errorReporter->descriptorError("<remove> cannot be a top-level element", line);
        }

        d = new RemoveDescriptor(_current, line, _info, attributes);
    }
    else if(name == "delete")
    {
        if(!_current)
        {
            _info->errorReporter->descriptorError("<delete> cannot be a top-level element", line);
        }

        d = new DeleteDescriptor(_current, line, _info, attributes);
    }
    else if(name == "fail")
    {
        if(!_current)
        {
            _info->errorReporter->descriptorError("<fail> cannot be a top-level element", line);
        }

        d = new FailDescriptor(_current, line, _info, attributes);
    }
    else if(name == "echo")
    {
        if(!_current)
        {
            _info->errorReporter->descriptorError("<echo> cannot be a top-level element", line);
        }

        d = new EchoDescriptor(_current, line, _info, attributes);
    }
    else if(name == "if")
    {
        if(!_current)
        {
            _info->errorReporter->descriptorError("<if> cannot be a top-level element", line);
        }

        d = new IfDescriptor(_current, line, _info, attributes);
    }
    else if(name == "iterate")
    {
        if(!_current)
        {
            _info->errorReporter->descriptorError("<iterate> cannot be a top-level element", line);
        }

        d = new IterateDescriptor(_current, line, _info, attributes);
    }
    else
    {
        _info->errorReporter->descriptorError("unknown descriptor `" + name + "'", line);
    }

    if(_current)
    {
        _current->addChild(d);
    }
    _current = d;
}

void
FreezeScript::DescriptorHandler::endElement(const string&, int, int)
{
    assert(_current);
    _current = _current->parent();
}

void
FreezeScript::DescriptorHandler::characters(const string&, int, int)
{
}

void
FreezeScript::DescriptorHandler::error(const string& msg, int line, int)
{
    _info->errorReporter->descriptorError(msg, line);
}

FreezeScript::TransformDBDescriptorPtr
FreezeScript::DescriptorHandler::descriptor() const
{
    return _descriptor;
}

//
// assignOrTransform
//
void
FreezeScript::assignOrTransform(const DataPtr& dest, const DataPtr& src, bool convert, const TransformInfoPtr& info)
{
    Slice::TypePtr destType = dest->getType();
    Slice::TypePtr srcType = src->getType();
    Slice::BuiltinPtr b1 = Slice::BuiltinPtr::dynamicCast(destType);
    Slice::BuiltinPtr b2 = Slice::BuiltinPtr::dynamicCast(srcType);

    if(!b1 && !b2 && dest->getType()->unit().get() != src->getType()->unit().get())
    {
        TransformVisitor v(src, info);
        dest->visit(v);
    }
    else
    {
        AssignVisitor v(src, info->getDataFactory(), info->getErrorReporter(), convert);
        dest->visit(v);
    }
}

void
FreezeScript::transformDatabase(const Ice::CommunicatorPtr& communicator,
                                const FreezeScript::ObjectFactoryPtr& objectFactory,
                                const Slice::UnitPtr& oldUnit, const Slice::UnitPtr& newUnit,
                                Db* oldDb, Db* newDb, DbTxn* newDbTxn, const Freeze::ConnectionPtr& connection,
                                const string& newDbName, const string& facetName, bool purgeObjects, ostream& errors,
                                bool suppress, istream& is)
{

    TransformInfoIPtr info = new TransformInfoI;
    info->communicator = communicator;
    info->objectFactory = objectFactory;
    info->oldUnit = oldUnit;
    info->newUnit = newUnit;
    info->oldDb = oldDb;
    info->newDb = newDb;
    info->newDbTxn = newDbTxn;
    info->connection = connection;
    info->newDbName = newDbName;
    info->facet = facetName;
    info->purge = purgeObjects;
    info->errorReporter = new ErrorReporter(errors, suppress);
    info->factory = new TransformDataFactory(communicator, newUnit, info->errorReporter);
    info->symbolTable = new SymbolTableI(info);

    try
    {
        DescriptorHandler dh(info);
        IceXML::Parser::parse(is, dh);
        TransformDBDescriptorPtr descriptor = dh.descriptor();
        descriptor->validate();
        descriptor->execute(info->symbolTable);
    }
    catch(const IceXML::ParserException& ex)
    {
        info->errorReporter->error(ex.reason());
    }
}
