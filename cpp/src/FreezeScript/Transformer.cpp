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

#include <FreezeScript/Transformer.h>
#include <FreezeScript/TransformDescriptors.h>
#include <FreezeScript/Exception.h>
#include <FreezeScript/TransformAnalyzer.h>
#include <FreezeScript/Util.h>
#include <db_cxx.h>

using namespace std;

namespace FreezeScript
{

class DescriptorHandler : public IceXML::Handler
{
public:

    DescriptorHandler(const DataFactoryPtr&, const Slice::UnitPtr&, const Slice::UnitPtr&, const ErrorReporterPtr&);

    virtual void startElement(const std::string&, const IceXML::Attributes&, int, int);
    virtual void endElement(const std::string&, int, int);
    virtual void characters(const std::string&, int, int);
    virtual void error(const std::string&, int, int);

    TransformDBDescriptorPtr descriptor() const;

private:

    DataFactoryPtr _factory;
    Slice::UnitPtr _old;
    Slice::UnitPtr _new;
    ErrorReporterPtr _errorReporter;
    DescriptorPtr _current;
    TransformDBDescriptorPtr _descriptor;
};

} // End of namespace FreezeScript

//
// DescriptorHandler
//
FreezeScript::DescriptorHandler::DescriptorHandler(const DataFactoryPtr& factory, const Slice::UnitPtr& oldUnit,
                                                   const Slice::UnitPtr& newUnit,
                                                   const ErrorReporterPtr& errorReporter) :
    _factory(factory), _old(oldUnit), _new(newUnit), _errorReporter(errorReporter)
{
}

void
FreezeScript::DescriptorHandler::startElement(const string& name, const IceXML::Attributes& attributes, int line,
                                              int column)
{
    DescriptorPtr d;

    if(name == "transformdb")
    {
        if(_current)
        {
            _errorReporter->descriptorError("<transformdb> must be the top-level element", line);
        }

        _descriptor = new TransformDBDescriptor(line, _factory, _errorReporter, attributes, _old, _new);
        d = _descriptor;
    }
    else if(name == "database")
    {
        if(!_current)
        {
            _errorReporter->descriptorError("<database> must be a child of <transformdb>", line);
        }

        d = new DatabaseDescriptor(_current, line, _factory, _errorReporter, attributes, _old, _new);
    }
    else if(name == "record")
    {
        if(!_current)
        {
            _errorReporter->descriptorError("<record> must be a child of <database>", line);
        }

        d = new RecordDescriptor(_current, line, _factory, _errorReporter, attributes, _old, _new);
    }
    else if(name == "transform")
    {
        if(!_current)
        {
            _errorReporter->descriptorError("<transform> must be a child of <transformdb>", line);
        }

        d = new TransformDescriptor(_current, line, _factory, _errorReporter, attributes, _old, _new);
    }
    else if(name == "init")
    {
        if(!_current)
        {
            _errorReporter->descriptorError("<init> must be a child of <transformdb>", line);
        }

        d = new InitDescriptor(_current, line, _factory, _errorReporter, attributes, _old, _new);
    }
    else if(name == "set")
    {
        if(!_current)
        {
            _errorReporter->descriptorError("<set> cannot be a top-level element", line);
        }

        d = new SetDescriptor(_current, line, _factory, _errorReporter, attributes);
    }
    else if(name == "define")
    {
        if(!_current)
        {
            _errorReporter->descriptorError("<define> cannot be a top-level element", line);
        }

        d = new DefineDescriptor(_current, line, _factory, _errorReporter, attributes, _old, _new);
    }
    else if(name == "add")
    {
        if(!_current)
        {
            _errorReporter->descriptorError("<add> cannot be a top-level element", line);
        }

        d = new AddDescriptor(_current, line, _factory, _errorReporter, attributes);
    }
    else if(name == "remove")
    {
        if(!_current)
        {
            _errorReporter->descriptorError("<remove> cannot be a top-level element", line);
        }

        d = new RemoveDescriptor(_current, line, _factory, _errorReporter, attributes);
    }
    else if(name == "delete")
    {
        if(!_current)
        {
            _errorReporter->descriptorError("<delete> cannot be a top-level element", line);
        }

        d = new DeleteDescriptor(_current, line, _factory, _errorReporter, attributes);
    }
    else if(name == "fail")
    {
        if(!_current)
        {
            _errorReporter->descriptorError("<fail> cannot be a top-level element", line);
        }

        d = new FailDescriptor(_current, line, _factory, _errorReporter, attributes);
    }
    else if(name == "echo")
    {
        if(!_current)
        {
            _errorReporter->descriptorError("<echo> cannot be a top-level element", line);
        }

        d = new EchoDescriptor(_current, line, _factory, _errorReporter, attributes);
    }
    else if(name == "if")
    {
        if(!_current)
        {
            _errorReporter->descriptorError("<if> cannot be a top-level element", line);
        }

        d = new IfDescriptor(_current, line, _factory, _errorReporter, attributes);
    }
    else if(name == "iterate")
    {
        if(!_current)
        {
            _errorReporter->descriptorError("<iterate> cannot be a top-level element", line);
        }

        d = new IterateDescriptor(_current, line, _factory, _errorReporter, attributes);
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
FreezeScript::DescriptorHandler::endElement(const std::string& name, int, int)
{
    assert(_current);
    _current = _current->parent();
}

void
FreezeScript::DescriptorHandler::characters(const std::string&, int, int)
{
}

void
FreezeScript::DescriptorHandler::error(const std::string& msg, int line, int col)
{
    _errorReporter->descriptorError(msg, line);
}

FreezeScript::TransformDBDescriptorPtr
FreezeScript::DescriptorHandler::descriptor() const
{
    return _descriptor;
}

//
// Transformer
//
FreezeScript::Transformer::Transformer(const Ice::CommunicatorPtr& communicator, const Slice::UnitPtr& oldUnit,
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
FreezeScript::Transformer::analyze(const string& oldKey, const string& newKey, const string& oldValue,
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

    TransformAnalyzer analyzer(_old, _new, _ignoreTypeChanges);
    analyzer.analyze(oldKeyType, newKeyType, oldValueType, newValueType, descriptors, missingTypes, errors);
}

void
FreezeScript::Transformer::analyze(ostream& descriptors, Ice::StringSeq& missingTypes, Ice::StringSeq& errors)
{
    const string keyType = "::Freeze::EvictorStorageKey";
    const string valueType = "::Freeze::ObjectRecord";
    analyze(keyType, keyType, valueType, valueType, descriptors, missingTypes, errors);
}

void
FreezeScript::Transformer::transform(istream& is, Db* db, DbTxn* txn, Db* dbNew, DbTxn* txnNew, ostream& errors,
                                     bool suppress)
{
    ErrorReporterPtr errorReporter = new ErrorReporter(errors, suppress);

    try
    {
        DataFactoryPtr factory = new DataFactory(_communicator, _new, errorReporter);
        DescriptorHandler dh(factory, _old, _new, errorReporter);
        IceXML::Parser::parse(is, dh);

        TransformDBDescriptorPtr descriptor = dh.descriptor();
        descriptor->validate();
        descriptor->transform(_communicator, db, txn, dbNew, txnNew, _purgeObjects);
    }
    catch(const IceXML::ParserException& ex)
    {
        errorReporter->error(ex.reason());
    }
}

Slice::TypePtr
FreezeScript::Transformer::findType(const Slice::UnitPtr& u, const string& type, Ice::StringSeq& errors)
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
