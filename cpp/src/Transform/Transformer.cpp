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

#include <Transform/Transformer.h>
#include <Transform/TransformUtil.h>
#include <Transform/Data.h>
#include <Transform/Parser.h>
#include <Transform/Error.h>
#include <Transform/Exception.h>
#include <Transform/Analyzer.h>
#include <IceXML/Parser.h>
#include <db_cxx.h>
#include <stack>

using namespace std;

namespace Transform
{

class Descriptor;
typedef IceUtil::Handle<Descriptor> DescriptorPtr;

class TransformDescriptor;
typedef IceUtil::Handle<TransformDescriptor> TransformDescriptorPtr;

typedef map<string, TransformDescriptorPtr> TransformMap;
typedef map<string, string> RenameMap;

class DeleteRecordException {};

class TransformSymbolTable : public SymbolTable
{
public:

    TransformSymbolTable(const DataFactoryPtr&, const Slice::UnitPtr&, const Slice::UnitPtr&, const ErrorReporterPtr&,
                         TransformSymbolTable* = 0);

    virtual DataPtr getValue(const EntityNodePtr&) const;
    virtual DataPtr getConstantValue(const string&) const;

    void add(const string&, const DataPtr&);

private:

    DataPtr findValue(const string&) const;

    class EntityVisitor : public EntityNodeVisitor
    {
    public:

        EntityVisitor(TransformSymbolTable*);

        virtual void visitIdentifier(const string&);
        virtual void visitElement(const NodePtr&);

        DataPtr getCurrent() const;

    private:

        TransformSymbolTable* _table;
        DataPtr _current;
        bool _error;
    };

    friend class EntityVisitor;

    DataFactoryPtr _factory;
    Slice::UnitPtr _old;
    Slice::UnitPtr _new;
    ErrorReporterPtr _errorReporter;
    TransformSymbolTable* _parent;
    typedef map<string, DataPtr> DataMap;
    DataMap _dataMap;
    DataMap _constantCache;
};

class Descriptor : virtual public IceUtil::SimpleShared
{
public:

    virtual ~Descriptor();

    virtual DescriptorPtr parent() const;
    virtual void addChild(const DescriptorPtr&) = 0;
    virtual void validate() = 0;
    virtual void execute(TransformSymbolTable&, DataInterceptor&) = 0;

    virtual DataFactoryPtr factory() const;
    virtual Slice::UnitPtr oldUnit() const;
    virtual Slice::UnitPtr newUnit() const;
    virtual ErrorReporterPtr errorReporter() const;

protected:

    Descriptor(const DescriptorPtr&, int);

    NodePtr parse(const string&) const;

    Slice::TypePtr findType(const Slice::UnitPtr&, const string&);

    DescriptorPtr _parent;
    int _line;
};

class SetDescriptor : public Descriptor
{
public:

    SetDescriptor(const DescriptorPtr&, int, const string&, const string&, const string&, const string&, bool);

    virtual void addChild(const DescriptorPtr&);
    virtual void validate();
    virtual void execute(TransformSymbolTable&, DataInterceptor&);

private:

    EntityNodePtr _target;
    NodePtr _value;
    string _valueStr;
    string _type;
    NodePtr _length;
    string _lengthStr;
    bool _convert;
};

class AddDescriptor : public Descriptor
{
public:

    AddDescriptor(const DescriptorPtr&, int, const string&, const string&, const string&, const string&, bool);

    virtual void addChild(const DescriptorPtr&);
    virtual void validate();
    virtual void execute(TransformSymbolTable&, DataInterceptor&);

private:

    EntityNodePtr _target;
    NodePtr _key;
    string _keyStr;
    NodePtr _value;
    string _valueStr;
    string _type;
    bool _convert;
};

class RemoveDescriptor : public Descriptor
{
public:

    RemoveDescriptor(const DescriptorPtr&, int, const string&, const string&);

    virtual void addChild(const DescriptorPtr&);
    virtual void validate();
    virtual void execute(TransformSymbolTable&, DataInterceptor&);

private:

    EntityNodePtr _target;
    NodePtr _key;
    string _keyStr;
};

class DeleteDescriptor : public Descriptor
{
public:

    DeleteDescriptor(const DescriptorPtr&, int);

    virtual void addChild(const DescriptorPtr&);
    virtual void validate();
    virtual void execute(TransformSymbolTable&, DataInterceptor&);
};

class FailDescriptor : public Descriptor
{
public:

    FailDescriptor(const DescriptorPtr&, int, const string&, const string&);

    virtual void addChild(const DescriptorPtr&);
    virtual void validate();
    virtual void execute(TransformSymbolTable&, DataInterceptor&);

private:

    NodePtr _test;
    string _testStr;
    string _message;
};

class EchoDescriptor : public Descriptor
{
public:

    EchoDescriptor(const DescriptorPtr&, int, const string&, const string&);

    virtual void addChild(const DescriptorPtr&);
    virtual void validate();
    virtual void execute(TransformSymbolTable&, DataInterceptor&);

private:

    string _message;
    string _value;
    NodePtr _node;
};

class ExecutableContainerDescriptor : virtual public Descriptor
{
public:

    ExecutableContainerDescriptor(const DescriptorPtr&, int, const string&);

    virtual void addChild(const DescriptorPtr&);
    virtual void validate();
    virtual void execute(TransformSymbolTable&, DataInterceptor&);

private:

    string _name;
    vector<DescriptorPtr> _children;
};

class ConditionalDescriptor : public ExecutableContainerDescriptor
{
public:

    ConditionalDescriptor(const DescriptorPtr&, int, const string&);

    virtual void execute(TransformSymbolTable&, DataInterceptor&);

private:

    string _test;
    NodePtr _node;
};

class IterateDescriptor : public ExecutableContainerDescriptor
{
public:

    IterateDescriptor(const DescriptorPtr&, int, const string&, const string&, const string&, const string&,
                      const string&);

    virtual void execute(TransformSymbolTable&, DataInterceptor&);

private:

    EntityNodePtr _target;
    string _key;
    string _value;
    string _element;
    string _index;
};

class TransformDescriptor : public ExecutableContainerDescriptor
{
public:

    TransformDescriptor(const DescriptorPtr&, int, const string&, bool, const string&);

    string type() const;
    string rename() const;
    bool doDefaultTransform() const;

private:

    bool _default;
    string _rename;
    Slice::TypePtr _oldType;
    Slice::TypePtr _newType;
};

class InitDescriptor : virtual public ExecutableContainerDescriptor, virtual public DataInitializer
{
public:

    InitDescriptor(const DescriptorPtr&, int, const string&);

    virtual void initialize(const DataFactoryPtr&, const DataPtr&, const Ice::CommunicatorPtr&);

    string type() const;

private:

    Slice::TypePtr _type;
};
typedef IceUtil::Handle<InitDescriptor> InitDescriptorPtr;

class RecordDescriptor : public ExecutableContainerDescriptor
{
public:

    RecordDescriptor(const DescriptorPtr&, int);
};
typedef IceUtil::Handle<RecordDescriptor> RecordDescriptorPtr;

class DatabaseDescriptor : public Descriptor
{
public:

    DatabaseDescriptor(const DescriptorPtr&, int, const string&, const string&);

    virtual void addChild(const DescriptorPtr&);
    virtual void validate();
    virtual void execute(TransformSymbolTable&, DataInterceptor&);

    bool transform(IceInternal::BasicStream&, IceInternal::BasicStream&, IceInternal::BasicStream&,
                   IceInternal::BasicStream&, const TransformMap&, const RenameMap&, bool);

private:

    Slice::TypePtr _oldKey;
    Slice::TypePtr _newKey;
    Slice::TypePtr _oldValue;
    Slice::TypePtr _newValue;
    RecordDescriptorPtr _record;
};
typedef IceUtil::Handle<DatabaseDescriptor> DatabaseDescriptorPtr;

class TransformerDescriptor : public Descriptor
{
public:

    TransformerDescriptor(const DataFactoryPtr&, const Slice::UnitPtr&, const Slice::UnitPtr&, const ErrorReporterPtr&,
                          int);

    virtual void addChild(const DescriptorPtr&);
    virtual void validate();
    virtual void execute(TransformSymbolTable&, DataInterceptor&);

    virtual DataFactoryPtr factory() const;
    virtual Slice::UnitPtr oldUnit() const;
    virtual Slice::UnitPtr newUnit() const;
    virtual ErrorReporterPtr errorReporter() const;

    void transform(const Ice::CommunicatorPtr&, Db*, Db*, bool);

private:

    DataFactoryPtr _factory;
    Slice::UnitPtr _old;
    Slice::UnitPtr _new;
    ErrorReporterPtr _errorReporter;
    DatabaseDescriptorPtr _database;
    TransformMap _transformMap;
    RenameMap _renameMap;
    vector<DescriptorPtr> _children;
};
typedef IceUtil::Handle<TransformerDescriptor> TransformerDescriptorPtr;

class DescriptorHandler : public IceXML::Handler
{
public:

    DescriptorHandler(const DataFactoryPtr&, const Slice::UnitPtr&, const Slice::UnitPtr&, const ErrorReporterPtr&);

    virtual void startElement(const std::string&, const IceXML::Attributes&, int, int);
    virtual void endElement(const std::string&, int, int);
    virtual void characters(const std::string&, int, int);
    virtual void error(const std::string&, int, int);

    TransformerDescriptorPtr descriptor() const;

private:

    DataFactoryPtr _factory;
    Slice::UnitPtr _old;
    Slice::UnitPtr _new;
    ErrorReporterPtr _errorReporter;
    DescriptorPtr _current;
    TransformerDescriptorPtr _transformer;
};

class TransformInterceptor : public DataInterceptor
{
public:

    TransformInterceptor(const DataFactoryPtr&, const ErrorReporterPtr&, const Slice::UnitPtr&,
                         const Slice::UnitPtr&, const TransformMap&, const RenameMap&, bool);

    virtual bool preTransform(const DataPtr&, const DataPtr&);
    virtual void postTransform(const DataPtr&, const DataPtr&);
    virtual ObjectDataMap& getObjectMap();
    virtual bool purgeObjects() const;
    virtual Slice::TypePtr getRename(const Slice::TypePtr&) const;

private:

    DataFactoryPtr _factory;
    ErrorReporterPtr _errorReporter;
    Slice::UnitPtr _old;
    Slice::UnitPtr _new;
    const TransformMap& _transformMap;
    const RenameMap& _renameMap;
    ObjectDataMap _objectMap;
    bool _purgeObjects;
};

} // End of namespace Transform

//
// TransformSymbolTable
//
Transform::TransformSymbolTable::TransformSymbolTable(const DataFactoryPtr& factory, const Slice::UnitPtr& oldUnit,
                                                      const Slice::UnitPtr& newUnit,
                                                      const ErrorReporterPtr& errorReporter,
                                                      TransformSymbolTable* parent) :
    _factory(factory), _old(oldUnit), _new(newUnit), _errorReporter(errorReporter), _parent(parent)
{
}

Transform::DataPtr
Transform::TransformSymbolTable::getValue(const EntityNodePtr& entity) const
{
    EntityVisitor visitor(const_cast<TransformSymbolTable*>(this));
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

Transform::DataPtr
Transform::TransformSymbolTable::getConstantValue(const string& name) const
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
        throw TransformException(__FILE__, __LINE__, "invalid constant name `" + name + "'");
    }

    Slice::ContainedList l = unit->findContents(key);
    if(l.empty())
    {
        throw TransformException(__FILE__, __LINE__, "unknown constant `" + name + "'");
    }

    Slice::EnumeratorPtr e = Slice::EnumeratorPtr::dynamicCast(l.front());
    Slice::ConstPtr c = Slice::ConstPtr::dynamicCast(l.front());
    if(!e && !c)
    {
        throw TransformException(__FILE__, __LINE__, "`" + name + "' does not refer to a Slice constant or enumerator");
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
                bool success = IceUtil::stringToInt64(value, n, end);
                assert(success);
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

void
Transform::TransformSymbolTable::add(const string& name, const DataPtr& data)
{
    DataPtr d = findValue(name);
    if(d)
    {
        throw TransformException(__FILE__, __LINE__, "`" + name + "' is already defined");
    }

    _dataMap.insert(DataMap::value_type(name, data));
}

Transform::DataPtr
Transform::TransformSymbolTable::findValue(const string& name) const
{
    DataMap::const_iterator p = _dataMap.find(name);
    if(p != _dataMap.end())
    {
        return p->second;
    }

    if(_parent)
    {
        return _parent->findValue(name);
    }

    return 0;
}

Transform::TransformSymbolTable::EntityVisitor::EntityVisitor(TransformSymbolTable* table) :
    _table(table), _error(false)
{
}

void
Transform::TransformSymbolTable::EntityVisitor::visitIdentifier(const string& name)
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
Transform::TransformSymbolTable::EntityVisitor::visitElement(const NodePtr& value)
{
    if(!_error)
    {
        assert(_current);

        DataPtr val = value->evaluate(*_table);
        _current = _current->getElement(val);

        if(!_current)
        {
            _error = true;
        }
    }
}

Transform::DataPtr
Transform::TransformSymbolTable::EntityVisitor::getCurrent() const
{
    return _current;
}

//
// Descriptor
//
Transform::Descriptor::Descriptor(const DescriptorPtr& parent, int line) :
    _parent(parent), _line(line)
{
}

Transform::Descriptor::~Descriptor()
{
}

Transform::DescriptorPtr
Transform::Descriptor::parent() const
{
    return _parent;
}

Transform::DataFactoryPtr
Transform::Descriptor::factory() const
{
    assert(_parent);
    return _parent->factory();
}

Slice::UnitPtr
Transform::Descriptor::oldUnit() const
{
    assert(_parent);
    return _parent->oldUnit();
}

Slice::UnitPtr
Transform::Descriptor::newUnit() const
{
    assert(_parent);
    return _parent->newUnit();
}

Transform::ErrorReporterPtr
Transform::Descriptor::errorReporter() const
{
    assert(_parent);
    return _parent->errorReporter();
}

Transform::NodePtr
Transform::Descriptor::parse(const string& expr) const
{
    return Parser::parse(expr, factory(), errorReporter());
}

Slice::TypePtr
Transform::Descriptor::findType(const Slice::UnitPtr& u, const string& type)
{
    Slice::TypeList l;

    l = u->lookupType(type, false);
    if(l.empty())
    {
        errorReporter()->error("unknown type `" + type + "'");
    }

    return l.front();
}

//
// SetDescriptor
//
Transform::SetDescriptor::SetDescriptor(const DescriptorPtr& parent, int line, const string& target,
                                        const string& value, const string& type, const string& length, bool convert) :
    Descriptor(parent, line), _valueStr(value), _type(type), _lengthStr(length), _convert(convert)
{
    DescriptorErrorContext ctx(errorReporter(), "set", _line);

    NodePtr node = parse(target);
    _target = EntityNodePtr::dynamicCast(node);
    if(!_target)
    {
        errorReporter()->error("`target' attribute is not an entity: `" + target + "'");
    }

    if(!value.empty())
    {
        _value = parse(value);
    }

    if(!length.empty())
    {
        _length = parse(length);
    }
}

void
Transform::SetDescriptor::addChild(const DescriptorPtr&)
{
    DescriptorErrorContext ctx(errorReporter(), "set", _line);
    errorReporter()->error("<set> cannot have child elements");
}

void
Transform::SetDescriptor::validate()
{
}

void
Transform::SetDescriptor::execute(TransformSymbolTable& sym, DataInterceptor& interceptor)
{
    DescriptorErrorContext ctx(errorReporter(), "set", _line);

    DataPtr data = sym.getValue(_target);
    if(data->readOnly())
    {
        ostringstream ostr;
        ostr << _target;
        errorReporter()->error("target `" + ostr.str() + "' cannot be modified");
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
            errorReporter()->error("evaluation of value `" + _valueStr + "' failed:\n" + ex.reason());
        }
    }

    Destroyer<DataPtr> valueDestroyer;
    if(!_type.empty())
    {
        assert(!value);
        Slice::TypeList l = newUnit()->lookupType(_type, false);
        if(l.empty())
        {
            errorReporter()->error("type `" + _type + "' not found");
        }
        value = factory()->createObject(l.front(), false);
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
            errorReporter()->error("target `" + ostr.str() + "' is not a sequence");
        }
        try
        {
            length = _length->evaluate(sym);
        }
        catch(const EvaluateException& ex)
        {
            errorReporter()->error("evaluation of length `" + _lengthStr + "' failed:\n" + ex.reason());
        }
        seq->resize(length, value, interceptor, _convert);
    }
    else
    {
        data->set(value, interceptor, _convert);
    }
}

//
// AddDescriptor
//
Transform::AddDescriptor::AddDescriptor(const DescriptorPtr& parent, int line, const string& target, const string& key,
                                        const string& value, const string& type, bool convert) :
    Descriptor(parent, line), _keyStr(key), _valueStr(value), _type(type), _convert(convert)
{
    DescriptorErrorContext ctx(errorReporter(), "add", _line);

    NodePtr node = parse(target);
    _target = EntityNodePtr::dynamicCast(node);
    if(!_target)
    {
        errorReporter()->error("`target' attribute is not an entity: `" + target + "'");
    }

    assert(!key.empty());
    _key = parse(key);

    if(!value.empty())
    {
        _value = parse(value);
    }
}

void
Transform::AddDescriptor::addChild(const DescriptorPtr&)
{
    DescriptorErrorContext ctx(errorReporter(), "add", _line);
    errorReporter()->error("<add> cannot have child elements");
}

void
Transform::AddDescriptor::validate()
{
}

void
Transform::AddDescriptor::execute(TransformSymbolTable& sym, DataInterceptor& interceptor)
{
    DescriptorErrorContext ctx(errorReporter(), "add", _line);

    DataPtr data = sym.getValue(_target);
    if(data->readOnly())
    {
        ostringstream ostr;
        ostr << _target;
        errorReporter()->error("target `" + ostr.str() + "' cannot be modified");
    }

    DictionaryDataPtr dict = DictionaryDataPtr::dynamicCast(data);
    if(!dict)
    {
        ostringstream ostr;
        ostr << _target;
        errorReporter()->error("target `" + ostr.str() + "' is not a dictionary");
    }

    Slice::DictionaryPtr type = Slice::DictionaryPtr::dynamicCast(dict->getType());
    assert(type);

    DataPtr key;
    Destroyer<DataPtr> keyDestroyer;
    try
    {
        DataPtr v = _key->evaluate(sym);
        key = factory()->create(type->keyType(), false);
        keyDestroyer.set(key);
        key->set(v, interceptor, _convert);
    }
    catch(const EvaluateException& ex)
    {
        errorReporter()->error("evaluation of key `" + _keyStr + "' failed:\n" + ex.reason());
    }

    if(dict->getElement(key))
    {
        ostringstream ostr;
        key->print(ostr);
        errorReporter()->error("key " + ostr.str() + " already exists in dictionary");
    }

    DataPtr elem = factory()->create(type->valueType(), false);
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
            errorReporter()->error("evaluation of value `" + _valueStr + "' failed:\n" + ex.reason());
        }
    }

    Destroyer<DataPtr> valueDestroyer;
    if(!_type.empty())
    {
        assert(!value);
        Slice::TypeList l = newUnit()->lookupType(_type, false);
        if(l.empty())
        {
            errorReporter()->error("type `" + _type + "' not found");
        }
        value = factory()->createObject(l.front(), false);
        valueDestroyer.set(value);
    }

    if(value)
    {
        elem->set(value, interceptor, _convert);
    }
    dict->add(key, elem);
    keyDestroyer.release();
    elemDestroyer.release();
}

//
// RemoveDescriptor
//
Transform::RemoveDescriptor::RemoveDescriptor(const DescriptorPtr& parent, int line, const string& target,
                                              const string& key) :
    Descriptor(parent, line), _keyStr(key)
{
    DescriptorErrorContext ctx(errorReporter(), "remove", _line);

    NodePtr node = parse(target);
    _target = EntityNodePtr::dynamicCast(node);
    if(!_target)
    {
        errorReporter()->error("`target' attribute is not an entity: `" + target + "'");
    }

    _key = parse(key);
}

void
Transform::RemoveDescriptor::addChild(const DescriptorPtr&)
{
    DescriptorErrorContext ctx(errorReporter(), "remove", _line);
    errorReporter()->error("<remove> cannot have child elements");
}

void
Transform::RemoveDescriptor::validate()
{
}

void
Transform::RemoveDescriptor::execute(TransformSymbolTable& sym, DataInterceptor& interceptor)
{
    DescriptorErrorContext ctx(errorReporter(), "remove", _line);

    DataPtr key;
    try
    {
        key = _key->evaluate(sym);
    }
    catch(const EvaluateException& ex)
    {
        errorReporter()->error("evaluation of key `" + _keyStr + "' failed:\n" + ex.reason());
    }

    DataPtr data = sym.getValue(_target);
    if(data->readOnly())
    {
        ostringstream ostr;
        ostr << _target;
        errorReporter()->error("target `" + ostr.str() + "' cannot be modified");
    }

    DictionaryDataPtr dict = DictionaryDataPtr::dynamicCast(data);
    if(!dict)
    {
        ostringstream ostr;
        ostr << _target;
        errorReporter()->error("target `" + ostr.str() + "' is not a dictionary");
    }
    dict->remove(key);
}

//
// DeleteDescriptor
//
Transform::DeleteDescriptor::DeleteDescriptor(const DescriptorPtr& parent, int line) :
    Descriptor(parent, line)
{
}

void
Transform::DeleteDescriptor::addChild(const DescriptorPtr&)
{
    DescriptorErrorContext ctx(errorReporter(), "delete", _line);
    errorReporter()->error("<delete> cannot have child elements");
}

void
Transform::DeleteDescriptor::validate()
{
}

void
Transform::DeleteDescriptor::execute(TransformSymbolTable&, DataInterceptor&)
{
    throw DeleteRecordException();
}

//
// FailDescriptor
//
Transform::FailDescriptor::FailDescriptor(const DescriptorPtr& parent, int line, const string& test,
                                          const string& message) :
    Descriptor(parent, line), _testStr(test), _message(message)
{
    if(!test.empty())
    {
        _test = parse(test);
    }

    if(_message.empty())
    {
        ostringstream ostr;
        ostr << "<fail> executed at line " << line << endl;
        _message = ostr.str();
    }
}

void
Transform::FailDescriptor::addChild(const DescriptorPtr&)
{
    DescriptorErrorContext ctx(errorReporter(), "fail", _line);
    errorReporter()->error("<fail> cannot have child elements");
}

void
Transform::FailDescriptor::validate()
{
}

void
Transform::FailDescriptor::execute(TransformSymbolTable& sym, DataInterceptor&)
{
    DescriptorErrorContext ctx(errorReporter(), "fail", _line);

    if(_test)
    {
        try
        {
            DataPtr b = _test->evaluate(sym);
            BooleanDataPtr bd = BooleanDataPtr::dynamicCast(b);
            if(!bd)
            {
                errorReporter()->error("expression `" + _testStr + "' does not evaluate to a boolean");
            }
            if(!bd->booleanValue())
            {
                return;
            }
        }
        catch(const EvaluateException& ex)
        {
            errorReporter()->error("evaluation of expression `" + _testStr + "' failed:\n" + ex.reason());
        }
    }

    throw TransformException(__FILE__, __LINE__, _message);
}

//
// EchoDescriptor
//
Transform::EchoDescriptor::EchoDescriptor(const DescriptorPtr& parent, int line, const string& message,
                                          const string& value) :
    Descriptor(parent, line), _message(message), _value(value)
{
    if(!value.empty())
    {
        _node = parse(value);
    }
}

void
Transform::EchoDescriptor::addChild(const DescriptorPtr&)
{
    DescriptorErrorContext ctx(errorReporter(), "echo", _line);
    errorReporter()->error("<echo> cannot have child elements");
}

void
Transform::EchoDescriptor::validate()
{
}

void
Transform::EchoDescriptor::execute(TransformSymbolTable& sym, DataInterceptor&)
{
    DescriptorErrorContext ctx(errorReporter(), "echo", _line);

    ostream& out = errorReporter()->stream();

    if(!_message.empty())
    {
        out << _message;
    }

    if(_node)
    {
        try
        {
            DataPtr v = _node->evaluate(sym);
            v->print(out);
        }
        catch(const EvaluateException& ex)
        {
            errorReporter()->error("evaluation of value `" + _value + "' failed:\n" + ex.reason());
        }
    }

    out << endl;
}

//
// ExecutableContainerDescriptor
//
Transform::ExecutableContainerDescriptor::ExecutableContainerDescriptor(const DescriptorPtr& parent, int line,
                                                                        const string& name) :
    Descriptor(parent, line), _name(name)
{
}

void
Transform::ExecutableContainerDescriptor::addChild(const DescriptorPtr& child)
{
    _children.push_back(child);
}

void
Transform::ExecutableContainerDescriptor::validate()
{
    for(vector<DescriptorPtr>::iterator p = _children.begin(); p != _children.end(); ++p)
    {
        (*p)->validate();
    }
}

void
Transform::ExecutableContainerDescriptor::execute(TransformSymbolTable& sym, DataInterceptor& interceptor)
{
    for(vector<DescriptorPtr>::iterator p = _children.begin(); p != _children.end(); ++p)
    {
        (*p)->execute(sym, interceptor);
    }
}

//
// ConditionalDescriptor
//
Transform::ConditionalDescriptor::ConditionalDescriptor(const DescriptorPtr& parent, int line, const string& test) :
    ExecutableContainerDescriptor(parent, line, "if"), Descriptor(parent, line), _test(test)
{
    DescriptorErrorContext ctx(errorReporter(), "if", _line);
    _node = parse(test);
}

void
Transform::ConditionalDescriptor::execute(TransformSymbolTable& sym, DataInterceptor& interceptor)
{
    DescriptorErrorContext ctx(errorReporter(), "if", _line);

    try
    {
        DataPtr b = _node->evaluate(sym);
        BooleanDataPtr bd = BooleanDataPtr::dynamicCast(b);
        if(!bd)
        {
            errorReporter()->error("expression `" + _test + "' does not evaluate to a boolean");
        }
        if(bd->booleanValue())
        {
            ExecutableContainerDescriptor::execute(sym, interceptor);
        }
    }
    catch(const EvaluateException& ex)
    {
        errorReporter()->error("evaluation of conditional expression `" + _test + "' failed:\n" + ex.reason());
    }
}

//
// IterateDescriptor
//
Transform::IterateDescriptor::IterateDescriptor(const DescriptorPtr& parent, int line, const string& target,
                                                const string& key, const string& value, const string& element,
                                                const string& index) :
    ExecutableContainerDescriptor(parent, line, "iterate"), Descriptor(parent, line), _key(key), _value(value),
    _element(element), _index(index)
{
    DescriptorErrorContext ctx(errorReporter(), "iterate", _line);

    NodePtr node = parse(target);
    _target = EntityNodePtr::dynamicCast(node);
    if(!_target)
    {
        errorReporter()->error("`target' attribute is not an entity: `" + target + "'");
    }
}

void
Transform::IterateDescriptor::execute(TransformSymbolTable& sym, DataInterceptor& interceptor)
{
    DescriptorErrorContext ctx(errorReporter(), "iterate", _line);

    DataPtr data = sym.getValue(_target);

    DictionaryDataPtr dict = DictionaryDataPtr::dynamicCast(data);
    SequenceDataPtr seq = SequenceDataPtr::dynamicCast(data);
    if(!dict && !seq)
    {
        ostringstream ostr;
        ostr << _target;
        errorReporter()->error("target `" + ostr.str() + "' is not a dictionary or sequence");
    }

    if(dict)
    {
        if(!_element.empty())
        {
            errorReporter()->error("attribute `element' specified for dictionary target");
        }
        if(!_index.empty())
        {
            errorReporter()->error("attribute `index' specified for dictionary target");
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

        DataMap map = dict->getElements();
        for(DataMap::iterator p = map.begin(); p != map.end(); ++p)
        {
            TransformSymbolTable elemSym(factory(), oldUnit(), newUnit(), errorReporter(), &sym);
            elemSym.add(key, p->first);
            elemSym.add(value, p->second);
            ExecutableContainerDescriptor::execute(elemSym, interceptor);
        }
    }
    else
    {
        if(!_key.empty())
        {
            errorReporter()->error("attribute `key' specified for sequence target");
        }
        if(!_value.empty())
        {
            errorReporter()->error("attribute `value' specified for sequence target");
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

        DataList l = seq->getElements();
        Ice::Long i = 0;
        for(DataList::iterator p = l.begin(); p != l.end(); ++p, ++i)
        {
            TransformSymbolTable elemSym(factory(), oldUnit(), newUnit(), errorReporter(), &sym);
            elemSym.add(element, *p);
            elemSym.add(index, factory()->createInteger(i, true));
            ExecutableContainerDescriptor::execute(elemSym, interceptor);
        }
    }
}

//
// TransformDescriptor
//
Transform::TransformDescriptor::TransformDescriptor(const DescriptorPtr& parent, int line, const string& type,
                                                    bool def, const string& rename) :
    ExecutableContainerDescriptor(parent, line, "transform"), Descriptor(parent, line), _default(def), _rename(rename)
{
    DescriptorErrorContext ctx(errorReporter(), "transform", _line);

    Slice::TypeList l;

    l = oldUnit()->lookupType(type, false);
    if(!l.empty())
    {
        _oldType = l.front();
    }

    l = newUnit()->lookupType(type, false);
    if(l.empty())
    {
        errorReporter()->error("unable to find type `" + type + "' in new Slice definitions");
    }
    else
    {
        _newType = l.front();
    }

    if(!rename.empty())
    {
        l = oldUnit()->lookupType(rename, false);
        if(l.empty())
        {
            errorReporter()->error("unable to find type `" + rename + "' in old Slice definitions");
        }
    }
}

string
Transform::TransformDescriptor::type() const
{
    return typeName(_newType);
}

string
Transform::TransformDescriptor::rename() const
{
    return _rename;
}

bool
Transform::TransformDescriptor::doDefaultTransform() const
{
    return _default;
}

//
// InitDescriptor
//
Transform::InitDescriptor::InitDescriptor(const DescriptorPtr& parent, int line, const string& type) :
    ExecutableContainerDescriptor(parent, line, "init"), Descriptor(parent, line)
{
    DescriptorErrorContext ctx(errorReporter(), "init", _line);

    Slice::TypeList l = newUnit()->lookupType(type, false);
    if(l.empty())
    {
        errorReporter()->error("unable to find type `" + type + "' in new Slice definitions");
    }
    else
    {
        _type = l.front();
    }
}

void
Transform::InitDescriptor::initialize(const DataFactoryPtr& factory, const DataPtr& data,
                                      const Ice::CommunicatorPtr& communicator)
{
    //
    // Create a new symbol table for the initializer and add the value to be initialized
    // as the symbol "value".
    //
    TransformSymbolTable sym(factory, oldUnit(), newUnit(), errorReporter());
    sym.add("value", data);
    errorReporter()->raise(true);
    //
    // Also need an interceptor in order to call execute.
    //
    TransformMap transformMap;
    RenameMap renameMap;
    TransformInterceptor interceptor(factory, errorReporter(), oldUnit(), newUnit(), transformMap, renameMap, false);
    execute(sym, interceptor);
}

string
Transform::InitDescriptor::type() const
{
    return typeName(_type);
}

//
// RecordDescriptor
//
Transform::RecordDescriptor::RecordDescriptor(const DescriptorPtr& parent, int line) :
    ExecutableContainerDescriptor(parent, line, "record"), Descriptor(parent, line)
{
}

//
// DatabaseDescriptor
//
Transform::DatabaseDescriptor::DatabaseDescriptor(const DescriptorPtr& parent, int line, const string& keyTypes,
                                                  const string& valueTypes) :
    Descriptor(parent, line)
{
    DescriptorErrorContext ctx(errorReporter(), "database", _line);

    string oldKeyName, newKeyName;
    string oldValueName, newValueName;
    string::size_type pos;

    pos = keyTypes.find(',');
    if(pos == 0 || pos == keyTypes.size())
    {
        errorReporter()->error("invalid key type specification `" + keyTypes +"'");
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
        errorReporter()->error("invalid value type specification `" + valueTypes +"'");
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
    _oldKey = findType(oldUnit(), oldKeyName);
    _newKey = findType(newUnit(), newKeyName);
    _oldValue = findType(oldUnit(), oldValueName);
    _newValue = findType(newUnit(), newValueName);
}

void
Transform::DatabaseDescriptor::addChild(const DescriptorPtr& child)
{
    DescriptorErrorContext ctx(errorReporter(), "database", _line);

    RecordDescriptorPtr rec = RecordDescriptorPtr::dynamicCast(child);
    if(!rec)
    {
        errorReporter()->error("invalid child for <database> descriptor");
    }
    if(_record)
    {
        errorReporter()->error("only one <record> element can be specified");
    }
    _record = rec;
}

void
Transform::DatabaseDescriptor::validate()
{
    if(!_record)
    {
        _record = new RecordDescriptor(this, _line);
    }
    _record->validate();
}

void
Transform::DatabaseDescriptor::execute(TransformSymbolTable&, DataInterceptor&)
{
    assert(false);
}

bool
Transform::DatabaseDescriptor::transform(IceInternal::BasicStream& inKey, IceInternal::BasicStream& inValue,
                                         IceInternal::BasicStream& outKey, IceInternal::BasicStream& outValue,
                                         const TransformMap& transformMap, const RenameMap& renameMap,
                                         bool purgeObjects)
{
    errorReporter()->raise(false);

    TransformInterceptor interceptor(factory(), errorReporter(), oldUnit(), newUnit(), transformMap, renameMap,
                                     purgeObjects);

    //
    // Create data representations of the old key and value types.
    //
    factory()->disableInitializers();
    DataPtr oldKeyData = factory()->create(_oldKey, true);
    Destroyer<DataPtr> oldKeyDataDestroyer(oldKeyData);
    DataPtr oldValueData = factory()->create(_oldValue, true);
    Destroyer<DataPtr> oldValueDataDestroyer(oldValueData);

    //
    // Unmarshal the old key and value.
    //
    oldKeyData->unmarshal(inKey);
    oldValueData->unmarshal(inValue);
    if(_oldValue->usesClasses())
    {
        inValue.readPendingObjects();
        oldValueData->registerObjects(interceptor.getObjectMap());
    }
    factory()->enableInitializers();

    //
    // Create data representations of the new key and value types.
    //
    DataPtr newKeyData = factory()->create(_newKey, false);
    Destroyer<DataPtr> newKeyDataDestroyer(newKeyData);
    DataPtr newValueData = factory()->create(_newValue, false);
    Destroyer<DataPtr> newValueDataDestroyer(newValueData);

    //
    // Copy the data from the old key and value to the new key and value, if possible.
    //
    newKeyData->transform(oldKeyData, interceptor);
    newValueData->transform(oldValueData, interceptor);

    //
    // Execute the <record> descriptor.
    //
    // TODO: Revisit identifiers.
    //
    TransformSymbolTable sym(factory(), oldUnit(), newUnit(), errorReporter());
    sym.add("oldkey", oldKeyData);
    sym.add("newkey", newKeyData);
    sym.add("oldvalue", oldValueData);
    sym.add("newvalue", newValueData);
    errorReporter()->raise(true);
    _record->execute(sym, interceptor);

    newKeyData->marshal(outKey);
    newValueData->marshal(outValue);
    if(_newValue->usesClasses())
    {
        outValue.writePendingObjects();
    }

    return true;
}

//
// TransformerDescriptor
//
Transform::TransformerDescriptor::TransformerDescriptor(const DataFactoryPtr& factory, const Slice::UnitPtr& oldUnit,
                                                        const Slice::UnitPtr& newUnit,
                                                        const ErrorReporterPtr& errorReporter, int line) :
    Descriptor(0, line), _factory(factory), _old(oldUnit), _new(newUnit), _errorReporter(errorReporter)
{
}

void
Transform::TransformerDescriptor::addChild(const DescriptorPtr& child)
{
    DescriptorErrorContext ctx(errorReporter(), "transformer", _line);

    DatabaseDescriptorPtr db = DatabaseDescriptorPtr::dynamicCast(child);
    TransformDescriptorPtr transform = TransformDescriptorPtr::dynamicCast(child);
    InitDescriptorPtr init = InitDescriptorPtr::dynamicCast(child);

    if(db)
    {
        if(_database)
        {
            errorReporter()->error("only one <database> element can be specified");
        }
        else
        {
            _database = db;
            _children.push_back(db);
        }
    }
    else if(transform)
    {
        string name = transform->type();
        TransformMap::iterator p = _transformMap.find(name);
        if(p != _transformMap.end())
        {
            errorReporter()->error("transform `" + name + "' specified more than once");
        }
        _transformMap.insert(TransformMap::value_type(name, transform));
        string rename = transform->rename();
        if(!rename.empty())
        {
            RenameMap::iterator q = _renameMap.find(rename);
            if(q != _renameMap.end())
            {
                errorReporter()->error("multiple transform descriptors specify the rename value `" + rename + "'");
            }
            _renameMap.insert(RenameMap::value_type(rename, name));
        }
        _children.push_back(transform);
    }
    else if(init)
    {
        string name = init->type();
        _factory->addInitializer(init->type(), init);
        _children.push_back(init);
    }
    else
    {
        errorReporter()->error("invalid child element");
    }
}

void
Transform::TransformerDescriptor::validate()
{
    DescriptorErrorContext ctx(errorReporter(), "transformer", _line);

    if(!_database)
    {
        errorReporter()->error("no <database> element specified");
    }

    for(vector<DescriptorPtr>::iterator p = _children.begin(); p != _children.end(); ++p)
    {
        (*p)->validate();
    }
}

void
Transform::TransformerDescriptor::execute(TransformSymbolTable&, DataInterceptor&)
{
    assert(false);
}

Transform::DataFactoryPtr
Transform::TransformerDescriptor::factory() const
{
    return _factory;
}

Slice::UnitPtr
Transform::TransformerDescriptor::oldUnit() const
{
    return _old;
}

Slice::UnitPtr
Transform::TransformerDescriptor::newUnit() const
{
    return _new;
}

Transform::ErrorReporterPtr
Transform::TransformerDescriptor::errorReporter() const
{
    return _errorReporter;
}

void
Transform::TransformerDescriptor::transform(const Ice::CommunicatorPtr& communicator, Db* db, Db* dbNew,
                                            bool purgeObjects)
{
    Dbc* dbc = 0;

    IceInternal::InstancePtr instance = IceInternal::getInstance(communicator);
    db->cursor(0, &dbc, 0);

    communicator->addObjectFactory(new Transform::ObjectFactory(_factory, _old), "");

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
                if(_database->transform(inKey, inValue, outKey, outValue, _transformMap, _renameMap, purgeObjects))
                {
                    outValue.endWriteEncaps();
                    Dbt dbNewKey(&outKey.b[0], outKey.b.size()), dbNewValue(&outValue.b[0], outValue.b.size());
                    if(dbNew->put(0, &dbNewKey, &dbNewValue, DB_NOOVERWRITE) == DB_KEYEXIST)
                    {
                        errorReporter()->error("duplicate key encountered");
                    }
                }
            }
            catch(const DeleteRecordException&)
            {
                // The record is deleted simply by not adding it to the new database.
            }
            catch(const ClassNotFoundException& ex)
            {
                if(!purgeObjects)
                {
                    errorReporter()->error("class " + ex.id + " not found in new Slice definitions");
                }
                else
                {
                    // The record is deleted simply by not adding it to the new database.
                    errorReporter()->warning("purging database record due to missing class type " + ex.id);
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
        communicator->removeObjectFactory("");
        throw;
    }

    communicator->removeObjectFactory("");

    if(dbc)
    {
        dbc->close();
    }
}

//
// DescriptorHandler
//
Transform::DescriptorHandler::DescriptorHandler(const DataFactoryPtr& factory, const Slice::UnitPtr& oldUnit,
                                                const Slice::UnitPtr& newUnit, const ErrorReporterPtr& errorReporter) :
    _factory(factory), _old(oldUnit), _new(newUnit), _errorReporter(errorReporter)
{
}

void
Transform::DescriptorHandler::startElement(const string& name, const IceXML::Attributes& attributes, int line,
                                           int column)
{
    DescriptorPtr d;

    if(name == "transformer")
    {
        if(_current)
        {
            _errorReporter->descriptorError("<transformer> must be the top-level element", line);
        }

        _transformer = new TransformerDescriptor(_factory, _old, _new, _errorReporter, line);
        d = _transformer;
    }
    else if(name == "database")
    {
        if(!_current)
        {
            _errorReporter->descriptorError("<database> must be a child of <transformer>", line);
        }

        IceXML::Attributes::const_iterator p = attributes.find("key");
        if(p == attributes.end())
        {
            _errorReporter->descriptorError("required attribute `key' is missing from <database>", line);
        }
        string keyTypes = p->second;

        p = attributes.find("value");
        if(p == attributes.end())
        {
            _errorReporter->descriptorError("required attribute `value' is missing from <database>", line);
        }
        string valueTypes = p->second;

        d = new DatabaseDescriptor(_current, line, keyTypes, valueTypes);
    }
    else if(name == "record")
    {
        if(!_current)
        {
            _errorReporter->descriptorError("<record> must be a child of <database>", line);
        }

        d = new RecordDescriptor(_current, line);
    }
    else if(name == "transform")
    {
        if(!_current)
        {
            _errorReporter->descriptorError("<transform> must be a child of <transformer>", line);
        }

        IceXML::Attributes::const_iterator p;

        string type, rename;
        bool def = true;

        p = attributes.find("type");
        if(p == attributes.end())
        {
            _errorReporter->descriptorError("required attribute `type' is missing from <transform>", line);
        }
        type = p->second;

        p = attributes.find("default");
        if(p != attributes.end())
        {
            if(p->second == "false")
            {
                def = false;
            }
        }

        p = attributes.find("rename");
        if(p != attributes.end())
        {
            rename = p->second;
        }

        d = new TransformDescriptor(_current, line, type, def, rename);
    }
    else if(name == "init")
    {
        if(!_current)
        {
            _errorReporter->descriptorError("<init> must be a child of <transformer>", line);
        }

        IceXML::Attributes::const_iterator p = attributes.find("type");
        if(p == attributes.end())
        {
            _errorReporter->descriptorError("required attribute `type' is missing from <init>", line);
        }

        d = new InitDescriptor(_current, line, p->second);
    }
    else if(name == "set")
    {
        if(!_current)
        {
            _errorReporter->descriptorError("<set> cannot be a top-level element", line);
        }

        IceXML::Attributes::const_iterator p;

        string target, value, type, length;
        bool convert = false;
        p = attributes.find("target");
        if(p == attributes.end())
        {
            _errorReporter->descriptorError("required attribute `target' is missing from <set>", line);
        }
        target = p->second;

        p = attributes.find("value");
        if(p != attributes.end())
        {
            value = p->second;
        }

        p = attributes.find("type");
        if(p != attributes.end())
        {
            type = p->second;
        }

        p = attributes.find("length");
        if(p != attributes.end())
        {
            length = p->second;
        }

        p = attributes.find("convert");
        if(p != attributes.end())
        {
            convert = p->second == "true";
        }

        if(!value.empty() && !type.empty())
        {
            _errorReporter->descriptorError("<set> attributes `value' and 'type' are mutually exclusive", line);
        }

        if(value.empty() && type.empty() && length.empty())
        {
            _errorReporter->descriptorError("<set> requires a value for attributes `value', 'type' or 'length'", line);
        }

        d = new SetDescriptor(_current, line, target, value, type, length, convert);
    }
    else if(name == "add")
    {
        if(!_current)
        {
            _errorReporter->descriptorError("<add> cannot be a top-level element", line);
        }

        IceXML::Attributes::const_iterator p;

        string target, key, value, type;
        bool convert = false;
        p = attributes.find("target");
        if(p == attributes.end())
        {
            _errorReporter->descriptorError("required attribute `target' is missing from <add>", line);
        }
        target = p->second;

        p = attributes.find("key");
        if(p == attributes.end())
        {
            _errorReporter->descriptorError("required attribute `key' is missing from <add>", line);
        }
        key = p->second;

        p = attributes.find("value");
        if(p != attributes.end())
        {
            value = p->second;
        }

        p = attributes.find("type");
        if(p != attributes.end())
        {
            type = p->second;
        }

        p = attributes.find("convert");
        if(p != attributes.end())
        {
            convert = p->second == "true";
        }

        if(!value.empty() && !type.empty())
        {
            _errorReporter->descriptorError("<add> attributes `value' and 'type' are mutually exclusive", line);
        }

        d = new AddDescriptor(_current, line, target, key, value, type, convert);
    }
    else if(name == "remove")
    {
        if(!_current)
        {
            _errorReporter->descriptorError("<remove> cannot be a top-level element", line);
        }

        IceXML::Attributes::const_iterator p;

        string target, key;
        p = attributes.find("target");
        if(p == attributes.end())
        {
            _errorReporter->descriptorError("required attribute `target' is missing from <remove>", line);
        }
        target = p->second;

        p = attributes.find("key");
        if(p == attributes.end())
        {
            _errorReporter->descriptorError("required attribute `key' is missing from <remove>", line);
        }
        key = p->second;

        d = new RemoveDescriptor(_current, line, target, key);
    }
    else if(name == "delete")
    {
        if(!_current)
        {
            _errorReporter->descriptorError("<delete> cannot be a top-level element", line);
        }

        d = new DeleteDescriptor(_current, line);
    }
    else if(name == "fail")
    {
        if(!_current)
        {
            _errorReporter->descriptorError("<fail> cannot be a top-level element", line);
        }

        IceXML::Attributes::const_iterator p;
        string test, message;

        p = attributes.find("test");
        if(p != attributes.end())
        {
            test = p->second;
        }

        p = attributes.find("message");
        if(p != attributes.end())
        {
            message = p->second;
        }

        d = new FailDescriptor(_current, line, test, message);
    }
    else if(name == "echo")
    {
        if(!_current)
        {
            _errorReporter->descriptorError("<echo> cannot be a top-level element", line);
        }

        IceXML::Attributes::const_iterator p;
        string message, value;

        p = attributes.find("message");
        if(p != attributes.end())
        {
            message = p->second;
        }

        p = attributes.find("value");
        if(p != attributes.end())
        {
            value = p->second;
        }

        d = new EchoDescriptor(_current, line, message, value);
    }
    else if(name == "if")
    {
        if(!_current)
        {
            _errorReporter->descriptorError("<if> cannot be a top-level element", line);
        }

        IceXML::Attributes::const_iterator p = attributes.find("test");
        if(p == attributes.end())
        {
            _errorReporter->descriptorError("required attribute `test' is missing from <if>", line);
        }

        d = new ConditionalDescriptor(_current, line, p->second);
    }
    else if(name == "iterate")
    {
        if(!_current)
        {
            _errorReporter->descriptorError("<iterate> cannot be a top-level element", line);
        }

        IceXML::Attributes::const_iterator p;
        string target, key, value, element, index;

        p = attributes.find("target");
        if(p == attributes.end())
        {
            _errorReporter->descriptorError("required attribute `target' is missing from <iterate>", line);
        }
        target = p->second;

        p = attributes.find("key");
        if(p != attributes.end())
        {
            key = p->second;
        }

        p = attributes.find("value");
        if(p != attributes.end())
        {
            value = p->second;
        }

        p = attributes.find("element");
        if(p != attributes.end())
        {
            element = p->second;
        }

        p = attributes.find("index");
        if(p != attributes.end())
        {
            index = p->second;
        }

        d = new IterateDescriptor(_current, line, target, key, value, element, index);
    }
    else
    {
        _errorReporter->descriptorError("unknown descriptor `" + name + "'", line);
    }

    if(_current)
    {
        _current->addChild(d);
    }
    _current = d;
}

void
Transform::DescriptorHandler::endElement(const std::string& name, int, int)
{
    assert(_current);
    _current = _current->parent();
}

void
Transform::DescriptorHandler::characters(const std::string&, int, int)
{
}

void
Transform::DescriptorHandler::error(const std::string& msg, int line, int col)
{
    _errorReporter->descriptorError(msg, line);
}

Transform::TransformerDescriptorPtr
Transform::DescriptorHandler::descriptor() const
{
    return _transformer;
}

//
// TransformInterceptor
//
Transform::TransformInterceptor::TransformInterceptor(const DataFactoryPtr& factory,
                                                      const ErrorReporterPtr& errorReporter,
                                                      const Slice::UnitPtr& oldUnit,
                                                      const Slice::UnitPtr& newUnit,
                                                      const TransformMap& transformMap,
                                                      const RenameMap& renameMap,
                                                      bool purgeObjects) :
    _factory(factory), _errorReporter(errorReporter), _old(oldUnit), _new(newUnit), _transformMap(transformMap),
    _renameMap(renameMap), _purgeObjects(purgeObjects)
{
}

bool
Transform::TransformInterceptor::preTransform(const DataPtr& dest, const DataPtr&)
{
    //
    // Allow a transform descriptor to disable the default transformation.
    //
    string type = typeName(dest->getType());
    TransformMap::const_iterator p = _transformMap.find(type);
    if(p != _transformMap.end())
    {
        return p->second->doDefaultTransform();
    }

    return true;
}

void
Transform::TransformInterceptor::postTransform(const DataPtr& dest, const DataPtr& src)
{
    //
    // Execute the type's transform (if any).
    //
    string type = typeName(dest->getType());
    TransformMap::const_iterator p = _transformMap.find(type);
    if(p != _transformMap.end())
    {
        bool raise = _errorReporter->raise();
        _errorReporter->raise(true);
        TransformSymbolTable sym(_factory, _old, _new, _errorReporter);
        sym.add("new", dest);
        sym.add("old", src);
        try
        {
            p->second->execute(sym, *this);
            _errorReporter->raise(raise);
        }
        catch(...)
        {
            _errorReporter->raise(raise);
            throw;
        }
    }
}

Transform::ObjectDataMap&
Transform::TransformInterceptor::getObjectMap()
{
    return _objectMap;
}

bool
Transform::TransformInterceptor::purgeObjects() const
{
    return _purgeObjects;
}

Slice::TypePtr
Transform::TransformInterceptor::getRename(const Slice::TypePtr& oldType) const
{
    Slice::TypePtr result;

    if(oldType->unit().get() == _old.get())
    {
        RenameMap::const_iterator p = _renameMap.find(typeName(oldType));
        if(p != _renameMap.end())
        {
            Slice::TypeList l = _new->lookupType(p->second, false);
            assert(!l.empty());
            return l.front();
        }
    }

    return result;
}

//
// Transformer
//
Transform::Transformer::Transformer(const Ice::CommunicatorPtr& communicator, const Slice::UnitPtr& oldUnit,
                                    const Slice::UnitPtr& newUnit, bool ignoreTypeChanges, bool purgeObjects) :
    _communicator(communicator), _old(oldUnit), _new(newUnit), _ignoreTypeChanges(ignoreTypeChanges),
    _purgeObjects(purgeObjects)
{
    createCoreSliceTypes(_old);
    createCoreSliceTypes(_new);

    createEvictorSliceTypes(_old);
    createEvictorSliceTypes(_new);
}

void
Transform::Transformer::analyze(const string& oldKey, const string& newKey, const string& oldValue,
                                const string& newValue, ostream& descriptors, Ice::StringSeq& missingTypes,
                                Ice::StringSeq& errors)
{
    //
    // Look up the Slice definitions for the key and value types.
    //
    Slice::TypePtr oldKeyType = findType(_old, oldKey, errors);
    Slice::TypePtr newKeyType = findType(_new, newKey, errors);
    Slice::TypePtr oldValueType = findType(_old, oldValue, errors);
    Slice::TypePtr newValueType = findType(_new, newValue, errors);
    if(!oldKeyType || !newKeyType || !oldValueType || !newValueType)
    {
        return;
    }

    Analyzer analyzer(_old, _new, _ignoreTypeChanges);
    analyzer.analyze(oldKeyType, newKeyType, oldValueType, newValueType, descriptors, missingTypes, errors);
}

void
Transform::Transformer::analyze(ostream& descriptors, Ice::StringSeq& missingTypes, Ice::StringSeq& errors)
{
    const string keyType = "::Freeze::EvictorStorageKey";
    const string valueType = "::Freeze::ObjectRecord";
    analyze(keyType, keyType, valueType, valueType, descriptors, missingTypes, errors);
}

void
Transform::Transformer::transform(istream& is, Db* db, Db* dbNew, ostream& errors)
{
    ErrorReporterPtr errorReporter = new ErrorReporter(errors);

    try
    {
        DataFactoryPtr factory = new DataFactory(_communicator, _new, errorReporter);
        DescriptorHandler dh(factory, _old, _new, errorReporter);
        IceXML::Parser::parse(is, dh);

        TransformerDescriptorPtr descriptor = dh.descriptor();
        descriptor->validate();
        descriptor->transform(_communicator, db, dbNew, _purgeObjects);
    }
    catch(const IceXML::ParserException& ex)
    {
        errorReporter->error(ex.reason());
    }
}

void
Transform::Transformer::createCoreSliceTypes(const Slice::UnitPtr& unit)
{
    string scoped;
    Slice::TypeList l;
    Slice::ContainedList c;

    //
    // Create the Slice definition for _FacetMap if it doesn't exist. This type is
    // necessary for marshaling an object's facet map.
    //
    l = unit->lookupTypeNoBuiltin("::_FacetMap", false);
    if(l.empty())
    {
        Slice::TypePtr str = unit->builtin(Slice::Builtin::KindString);
        Slice::TypePtr obj = unit->builtin(Slice::Builtin::KindObject);
        unit->createDictionary("_FacetMap", str, obj, false);
    }
    else
    {
        assert(Slice::DictionaryPtr::dynamicCast(l.front()));
    }
}

void
Transform::Transformer::createEvictorSliceTypes(const Slice::UnitPtr& unit)
{
    string scoped;
    Slice::TypeList l;
    Slice::ContainedList c;

    //
    // Create the Ice module if necessary.
    //
    c = unit->lookupContained("Ice", false);
    Slice::ModulePtr ice;
    if(c.empty())
    {
        ice = unit->createModule("Ice");
    }
    else
    {
        ice = Slice::ModulePtr::dynamicCast(c.front());
        if(!ice)
        {
            throw TransformException(__FILE__, __LINE__,
                                     "the symbol `::Ice' is defined in Slice but is not a module");
        }
    }

    //
    // Create the Slice definition for Ice::Identity if it doesn't exist.
    //
    scoped = "::Ice::Identity";
    l = unit->lookupTypeNoBuiltin(scoped, false);
    Slice::StructPtr identity;
    if(l.empty())
    {
        identity = ice->createStruct("Identity", false);
        Slice::TypePtr str = unit->builtin(Slice::Builtin::KindString);
        identity->createDataMember("category", str);
        identity->createDataMember("name", str);
    }
    else
    {
        identity = Slice::StructPtr::dynamicCast(l.front());
        if(!identity)
        {
            throw TransformException(__FILE__, __LINE__,
                                     "the symbol `::Ice::Identity' is defined in Slice but is not a struct");
        }
    }

    //
    // Create the Slice definition for Ice::FacetPath if it doesn't exist.
    //
    scoped = "::Ice::FacetPath";
    l = unit->lookupTypeNoBuiltin(scoped, false);
    Slice::SequencePtr facetPath;
    if(l.empty())
    {
        Slice::TypePtr str = unit->builtin(Slice::Builtin::KindString);
        facetPath = ice->createSequence("FacetPath", str, false);
    }
    else
    {
        facetPath = Slice::SequencePtr::dynamicCast(l.front());
        if(!facetPath)
        {
            throw TransformException(__FILE__, __LINE__,
                                     "the symbol `::Ice::FacetPath' is defined in Slice but is not a sequence");
        }
    }

    //
    // Create the Freeze module if necessary.
    //
    c = unit->lookupContained("Freeze", false);
    Slice::ModulePtr freeze;
    if(c.empty())
    {
        freeze = unit->createModule("Freeze");
    }
    else
    {
        freeze = Slice::ModulePtr::dynamicCast(c.front());
        if(!freeze)
        {
            throw TransformException(__FILE__, __LINE__,
                                     "the symbol `::Freeze' is defined in Slice but is not a module");
        }
    }

    //
    // Create the Slice definition for Freeze::EvictorStorageKey if it doesn't exist.
    //
    scoped = "::Freeze::EvictorStorageKey";
    l = unit->lookupTypeNoBuiltin(scoped, false);
    if(l.empty())
    {
        Slice::StructPtr esk = freeze->createStruct("EvictorStorageKey", false);
        esk->createDataMember("identity", identity);
        esk->createDataMember("facet", facetPath);
    }
    else
    {
        if(!Slice::StructPtr::dynamicCast(l.front()))
        {
            throw TransformException(__FILE__, __LINE__, "the symbol `::Freeze::EvictorStorageKey' is defined in "
                                     "Slice but is not a struct");
        }
    }

    //
    // Create the Slice definition for Freeze::Statistics if it doesn't exist.
    //
    scoped = "::Freeze::Statistics";
    l = unit->lookupTypeNoBuiltin(scoped, false);
    Slice::StructPtr stats;
    if(l.empty())
    {
        stats = freeze->createStruct("Statistics", false);
        Slice::TypePtr tl = unit->builtin(Slice::Builtin::KindLong);
        stats->createDataMember("creationTime", tl);
        stats->createDataMember("lastSaveTime", tl);
        stats->createDataMember("avgSaveTime", tl);
    }
    else
    {
        stats = Slice::StructPtr::dynamicCast(l.front());
        if(!stats)
        {
            throw TransformException(__FILE__, __LINE__, "the symbol `::Freeze::Statistics' is defined in "
                                     "Slice but is not a struct");
        }
    }

    //
    // Create the Slice definition for Freeze::ObjectRecord if it doesn't exist.
    //
    scoped = "::Freeze::ObjectRecord";
    l = unit->lookupTypeNoBuiltin(scoped, false);
    if(l.empty())
    {
        Slice::StructPtr rec = freeze->createStruct("ObjectRecord", false);
        Slice::TypePtr obj = unit->builtin(Slice::Builtin::KindObject);
        rec->createDataMember("servant", obj);
        rec->createDataMember("stats", stats);
    }
    else
    {
        if(!Slice::StructPtr::dynamicCast(l.front()))
        {
            throw TransformException(__FILE__, __LINE__, "the symbol `::Freeze::ObjectRecord' is defined in "
                                     "Slice but is not a struct");
        }
    }
}

Slice::TypePtr
Transform::Transformer::findType(const Slice::UnitPtr& u, const string& type, Ice::StringSeq& errors)
{
    Slice::TypeList l;

    l = u->lookupType(type, false);
    if(l.empty())
    {
        errors.push_back("error: unknown type `" + type + "'");
        return 0;
    }

    return l.front();
}
