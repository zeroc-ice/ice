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
    Visitor(int TSRMLS_DC);

    virtual bool visitClassDefStart(const Slice::ClassDefPtr&);
    virtual void visitClassDefEnd(const Slice::ClassDefPtr&);
    virtual bool visitExceptionStart(const Slice::ExceptionPtr&);
    virtual bool visitStructStart(const Slice::StructPtr&);
    virtual void visitOperation(const Slice::OperationPtr&);
    virtual void visitDataMember(const Slice::DataMemberPtr&);
    virtual void visitDictionary(const Slice::DictionaryPtr&);
    virtual void visitEnum(const Slice::EnumPtr&);
    virtual void visitConst(const Slice::ConstPtr&);

private:
    int _module;
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
// Map from flattened, lowercase name to Slice information.
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
    if(!ice_splitString(argStr, args))
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
Slice_init(int module TSRMLS_DC)
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

        Visitor visitor(module TSRMLS_CC);
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
Slice_getClass(const string& scoped)
{
    zend_class_entry* result = NULL;

    string flat = ice_lowerCase(ice_flatten(scoped));
    TypeMap::iterator p = _typeMap.find(flat);
    if(p != _typeMap.end())
    {
        result = p->second->entry;
    }

    return result;
}

bool
Slice_isNativeKey(const Slice::TypePtr& type)
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

Visitor::Visitor(int module TSRMLS_DC) :
    _module(module)
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
Visitor::visitExceptionStart(const Slice::ExceptionPtr& p)
{
    //
    // Get the class entry for the base exception (if any).
    //
    zend_class_entry* parent = NULL;
    Slice::ExceptionPtr base = p->base();
    if(base)
    {
        string s = base->scoped();
        string f = ice_lowerCase(ice_flatten(s));
        TypeMap::iterator q = _typeMap.find(f);
        if(q != _typeMap.end())
        {
            parent = q->second->entry;
        }
        else
        {
            zend_error(E_ERROR, "base exception %s not found", f.c_str());
            return false;
        }
    }

    string scoped = p->scoped();
    string flat = ice_lowerCase(ice_flatten(scoped));

    zend_class_entry ce;
    INIT_CLASS_ENTRY(ce, flat.c_str(), NULL);
    //
    // We have to reset name_length because the INIT_CLASS_ENTRY macro assumes the class name
    // is a string constant.
    //
    ce.name_length = flat.length();
    // TODO: Check for conflicts with existing symbols?
    zend_class_entry* cls = zend_register_internal_class_ex(&ce, parent, NULL TSRMLS_CC);

    if(cls == NULL)
    {
        zend_error(E_ERROR, "unable to create class for type %s", scoped.c_str());
    }
    else
    {
        _typeMap[flat] = new TypeInfo(p, cls);
    }

    return false;
}

bool
Visitor::visitStructStart(const Slice::StructPtr& p)
{
    //
    // Special case for Ice::Identity, which is predefined.
    //
    if(p->scoped() == "::Ice::Identity")
    {
        return false;
    }

    createZendClass(p);
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
    if(!Slice_isNativeKey(keyType))
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
Visitor::visitConst(const Slice::ConstPtr& p)
{
    string name = ice_flatten(p->scoped()); // Don't convert to lower case; constant names are case sensitive.
    char* cname = const_cast<char*>(name.c_str());
    uint cnameLen = name.length() + 1;
    string value = p->value();
    const int flags = CONST_PERSISTENT|CONST_CS; // Persistent and case sensitive

    Slice::TypePtr type = p->type();
    Slice::BuiltinPtr b = Slice::BuiltinPtr::dynamicCast(type);
    if(b)
    {
        switch(b->kind())
        {
        case Slice::Builtin::KindBool:
        {
            long l = (value == "true" ? 1 : 0);
            zend_register_long_constant(cname, cnameLen, l, flags, _module TSRMLS_CC);
            break;
        }

        case Slice::Builtin::KindByte:
        case Slice::Builtin::KindShort:
        case Slice::Builtin::KindInt:
        case Slice::Builtin::KindLong:
        {
            Ice::Long l;
            string::size_type pos;
            IceUtil::stringToInt64(value, l, pos);
            //
            // The platform's 'long' type may not be 64 bits, so we store 64-bit
            // values as a string.
            //
            if(sizeof(Ice::Long) > sizeof(long) && (l < LONG_MIN || l > LONG_MAX))
            {
                zend_register_stringl_constant(cname, cnameLen, const_cast<char*>(value.c_str()), value.length(),
                                               flags, _module TSRMLS_CC);
            }
            else
            {
                zend_register_long_constant(cname, cnameLen, static_cast<long>(l), flags, _module TSRMLS_CC);
            }
            break;
        }

        case Slice::Builtin::KindString:
        {
            zend_register_stringl_constant(cname, cnameLen, const_cast<char*>(value.c_str()), value.length(), flags,
                                           _module TSRMLS_CC);
            break;
        }

        case Slice::Builtin::KindFloat:
        case Slice::Builtin::KindDouble:
        {
            double d = atof(value.c_str());
            zend_register_double_constant(cname, cnameLen, d, flags, _module TSRMLS_CC);
            break;
        }

        case Slice::Builtin::KindObject:
        case Slice::Builtin::KindObjectProxy:
        case Slice::Builtin::KindLocalObject:
            assert(false);
        }

        return;
    }

    Slice::EnumPtr en = Slice::EnumPtr::dynamicCast(type);
    if(en)
    {
        Slice::EnumeratorList l = en->getEnumerators();
        Slice::EnumeratorList::iterator q;
        long i;
        for(q = l.begin(), i = 0; q != l.end(); ++q, ++i)
        {
            if((*q)->name() == value)
            {
                zend_register_long_constant(cname, cnameLen, i, flags, _module TSRMLS_CC);
                return;
            }
        }
        assert(false); // No match found.
    }

    //
    // No other constant types are allowed.
    //
    assert(false);
}

zend_class_entry*
Visitor::createZendClass(const Slice::ContainedPtr& type)
{
    string scoped = type->scoped();
    string flat = ice_lowerCase(ice_flatten(scoped));

    zend_class_entry ce;
    INIT_CLASS_ENTRY(ce, flat.c_str(), NULL);
    //
    // We have to reset name_length because the INIT_CLASS_ENTRY macro assumes the class name
    // is a string constant.
    //
    ce.name_length = flat.length();
    // TODO: Check for conflicts with existing symbols?
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
