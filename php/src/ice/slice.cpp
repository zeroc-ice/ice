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
    Visitor(TSRMLS_D);

    virtual bool visitClassDefStart(const Slice::ClassDefPtr&);
    virtual void visitClassDefEnd(const Slice::ClassDefPtr&);
    virtual bool visitExceptionStart(const Slice::ExceptionPtr&);
    virtual void visitExceptionEnd(const Slice::ExceptionPtr&);
    virtual bool visitStructStart(const Slice::StructPtr&);
    virtual void visitOperation(const Slice::OperationPtr&);
    virtual void visitDataMember(const Slice::DataMemberPtr&);
    virtual void visitDictionary(const Slice::DictionaryPtr&);
    virtual void visitEnum(const Slice::EnumPtr&);
    virtual void visitConst(const Slice::ConstPtr&);

private:
    zend_class_entry* createZendClass(const Slice::ContainedPtr&);
#ifdef ZTS
    TSRMLS_D;
#endif
};

//
// The Slice parse tree.
//
static Slice::UnitPtr _unit;

//
// Map from flattened, lowercase type name to type information.
//
struct TypeInfo : public IceUtil::SimpleShared
{
    TypeInfo(const Slice::ContainedPtr& t, zend_class_entry* e) : type(t), entry(e) {}

    Slice::ContainedPtr type;
    zend_class_entry* entry;
};
typedef IceUtil::Handle<TypeInfo> TypeInfoPtr;
typedef map<string, TypeInfoPtr> TypeMap;
static TypeMap _typeMap;

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

#ifdef ZTS
        Visitor visitor(TSRMLS_C);
#else
        Visitor visitor;
#endif
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

zend_class_entry*
Slice_get_class(const string& scoped)
{
    zend_class_entry* result = NULL;

    string flat = ice_lowercase(ice_flatten(scoped));
    TypeMap::iterator p = _typeMap.find(flat);
    if(p != _typeMap.end())
    {
        result = p->second->entry;
    }

    return result;
}

bool
Slice_is_native_key(const Slice::TypePtr& type)
{
    //
    // PHP's native associative array supports only integer and string types for the key.
    // For Slice dictionaries that meet this criteria, we use the native array type.
    //
    Slice::BuiltinPtr b = Slice::BuiltinPtr::dynamicCast(type);
    if(b)
    {
        switch(b->kind())
        {
        case Slice::Builtin::KindByte:
        case Slice::Builtin::KindBool: // We allow bool even though PHP doesn't support it directly.
        case Slice::Builtin::KindShort:
        case Slice::Builtin::KindInt:
        case Slice::Builtin::KindLong:
        case Slice::Builtin::KindString:
            return true;

        case Slice::Builtin::KindFloat:
        case Slice::Builtin::KindDouble:
        case Slice::Builtin::KindObject:
        case Slice::Builtin::KindObjectProxy:
        case Slice::Builtin::KindLocalObject:
            break;
        }
    }

    return false;
}

Visitor::Visitor(TSRMLS_D)
{
#ifdef ZTS
    this->TSRMLS_C = TSRMLS_C;
#endif
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
    zend_class_entry* cls = createZendClass(p);
    if(cls != NULL)
    {
        // TODO: Add default properties?
    }

    return false;
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
Visitor::visitDictionary(const Slice::DictionaryPtr& p)
{
    Slice::TypePtr keyType = p->keyType();
    if(!Slice_is_native_key(keyType))
    {
        //
        // TODO: Generate class.
        //
        string scoped = p->scoped();
        zend_error(E_WARNING, "dictionary %s uses an unsupported key type", scoped.c_str());
    }
}

void
Visitor::visitEnum(const Slice::EnumPtr& p)
{
    //
    // Enum values are represented as integers, however we define a class in order
    // to hold constants that symbolically identify the enumerators.
    //
    zend_class_entry* cls = createZendClass(p);
    if(cls != NULL)
    {
        //
        // Create a class constant for each enumerator.
        //
        Slice::EnumeratorList l = p->getEnumerators();
        Slice::EnumeratorList::const_iterator q;
        long i;
        for(q = l.begin(), i = 0; q != l.end(); ++q, ++i)
        {
            string name = (*q)->name();
            zval* en;
            ALLOC_ZVAL(en);
            ZVAL_LONG(en, i);
            zend_hash_update(&cls->constants_table, const_cast<char*>(name.c_str()), name.length() + 1, &en,
                             sizeof(zval*), NULL);
        }
    }
}

void
Visitor::visitConst(const Slice::ConstPtr&)
{
}

zend_class_entry*
Visitor::createZendClass(const Slice::ContainedPtr& type)
{
    string scoped = type->scoped();
    string flat = ice_lowercase(ice_flatten(scoped));

    zend_class_entry ce;
    INIT_CLASS_ENTRY(ce, flat.c_str(), NULL);
    //
    // We have to reset name_length because the INIT_CLASS_ENTRY macro assumes the class name
    // is a string constant.
    //
    ce.name_length = flat.length();
    // TODO: Check for conflicts with existing symbols
    zend_class_entry* result = zend_register_internal_class(&ce TSRMLS_CC);

    if(result == NULL)
    {
        zend_error(E_ERROR, "unable to create class for type %s", scoped.c_str());
    }
    else
    {
        _typeMap[flat] = new TypeInfo(type, result);
    }

    return result;
}
