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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "slice.h"
#include "struct.h"
#include "util.h"
#include "php_ice.h"

#include <Slice/Preprocessor.h>

using namespace std;

//
// Visitor descends the Slice parse tree and creates PHP classes for certain Slice types.
//
class Visitor : public Slice::ParserVisitor
{
public:
    virtual bool visitClassDefStart(const Slice::ClassDefPtr&);
    virtual void visitClassDefEnd(const Slice::ClassDefPtr&);
    virtual bool visitExceptionStart(const Slice::ExceptionPtr&);
    virtual void visitExceptionEnd(const Slice::ExceptionPtr&);
    virtual bool visitStructStart(const Slice::StructPtr&);
    virtual void visitStructEnd(const Slice::StructPtr&);
    virtual void visitOperation(const Slice::OperationPtr&);
    virtual void visitDataMember(const Slice::DataMemberPtr&);
    virtual void visitDictionary(const Slice::DictionaryPtr&);
    virtual void visitEnum(const Slice::EnumPtr&);
    virtual void visitConst(const Slice::ConstPtr&);
};

//
// The Slice parse tree.
//
static Slice::UnitPtr _unit;

//
// Parse the Slice files that define the types and operations available to a PHP script.
//
static bool
parse_slice(const string& argStr)
{
    vector<string> args;
    if(!ice_split_string(argStr, args))
    {
        return false;
    }

    string cppArgs;
    vector<string> files;
    bool debug = false;
    bool ice = false;
    bool caseSensitive = false;

    vector<string>::const_iterator p;
    for(p = args.begin(); p != args.end(); ++p)
    {
        string arg = *p;
        if(arg.substr(0, 2) == "-I" || arg.substr(0, 2) == "-D" || arg.substr(0, 2) == "-U")
        {
            cppArgs += ' ';
            if(arg.find(' ') != string::npos)
            {
                cppArgs += "'";
                cppArgs += arg;
                cppArgs += "'";
            }
            else
            {
                cppArgs += arg;
            }
        }
        else if(arg == "--ice")
        {
            ice = true;
        }
        else if(arg == "--case-sensitive")
        {
            caseSensitive = true;
        }
        else if(arg[0] == '-')
        {
            zend_error(E_ERROR, "unknown option `%s' in ice.parse", arg.c_str());
            return false;
        }
        else
        {
            files.push_back(arg);
        }
    }

    if(files.empty())
    {
        zend_error(E_ERROR, "no Slice files specified in ice.parse");
        return false;
    }

    bool ignoreRedefs = false;
    bool all = false;
    _unit = Slice::Unit::createUnit(ignoreRedefs, all, ice, caseSensitive);
    bool status = true;

    for(p = files.begin(); p != files.end(); ++p)
    {
        Slice::Preprocessor icecpp("icecpp", *p, cppArgs);
        FILE* cppHandle = icecpp.preprocess(false);

        if(cppHandle == 0)
        {
            status = false;
            break;
        }

        int parseStatus = _unit->parse(cppHandle, debug);

        if(!icecpp.close())
        {
            status = false;
            break;
        }

        if(parseStatus == EXIT_FAILURE)
        {
            status = false;
            break;
        }
    }

    return status;
}

bool
Slice_init(TSRMLS_DC)
{
    //
    // Parse the Slice files.
    //
    char* parse = INI_STR("ice.parse");
    if(parse && strlen(parse) > 0)
    {
        if(!parse_slice(parse))
        {
            return false;
        }

        Visitor visitor;
        _unit->visit(&visitor);
    }

    return true;
}

Slice::UnitPtr
Slice_getUnit(TSRMLS_DC)
{
    return _unit;
}

bool
Slice_shutdown(TSRMLS_DC)
{
    if(_unit)
    {
        try
        {
            _unit->destroy();
            _unit = 0;
        }
        catch(const IceUtil::Exception& ex)
        {
            ostringstream ostr;
            ex.ice_print(ostr);
            zend_error(E_ERROR, "unable to destroy Slice parse tree:\n%s", ostr.str().c_str());
            return false;
        }
    }

    return true;
}

bool
Visitor::visitClassDefStart(const Slice::ClassDefPtr&)
{
    return false;
}

void
Visitor::visitClassDefEnd(const Slice::ClassDefPtr&)
{
}

bool
Visitor::visitExceptionStart(const Slice::ExceptionPtr&)
{
    return false;
}

void
Visitor::visitExceptionEnd(const Slice::ExceptionPtr&)
{
}

bool
Visitor::visitStructStart(const Slice::StructPtr& p)
{
    return Struct_register(p);
}

void
Visitor::visitStructEnd(const Slice::StructPtr&)
{
}

void
Visitor::visitOperation(const Slice::OperationPtr&)
{
}

void
Visitor::visitDataMember(const Slice::DataMemberPtr&)
{
}

void
Visitor::visitDictionary(const Slice::DictionaryPtr&)
{
}

void
Visitor::visitEnum(const Slice::EnumPtr&)
{
}

void
Visitor::visitConst(const Slice::ConstPtr&)
{
}
