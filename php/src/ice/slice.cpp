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
#include "marshal.h"
#include "util.h"

#include <Slice/Preprocessor.h>

using namespace std;

ZEND_EXTERN_MODULE_GLOBALS(ice)

#if 0
//
// We're using a little hack. For performance reasons, we want to parse the Slice files and create
// the corresponding PHP classes during module startup, not request startup. In order for these
// classes to persist across requests, we need to register them as "internal" classes, and not
// "user" classes. User classes are created when PHP scripts are parsed, and destroyed when the
// script terminates.
//
// The decision about which type of class to create influences memory management. If we use PHP's
// e* functions (e.g., emalloc), then that memory is reclaimed after each request. Therefore, we
// cannot use these functions to create internal classes.
//
// There are also two types of class methods: internal functions and user functions. Internal
// functions are implemented by the extension, whereas user functions are implemented in PHP.
// PHP assumes that a user function descriptor is allocated using the e* functions, because
// a user function is normally only created by the PHP parser and therefore the function must
// have been defined within a user class.
//
// This means that if we want to define a user function in an internal class, as is necessary for
// definining operation prototypes for Slice classes and interfaces, we have to be very careful
// about memory management. Therefore, when PHP is shutting down, we iterate over the classes
// we've created and manually free the user functions we created. If we didn't do this, then PHP
// would use the wrong memory management functions to free this data.
//
// There are two alternatives:
//
// 1. Create the PHP classes for Slice types for each request. These could then be created as
//    user classes, not internal classes.
//
// 2. Generate PHP classes from the Slice types. This avoids the need to create the classes
//    manually, but we would likely still need to parse the Slice files in the extension, which
//    means the user would need both the PHP classes and the Slice files.
//
#endif

//
// PHPVisitor descends the Slice parse tree and creates PHP classes for certain Slice types.
//
class PHPVisitor : public Slice::ParserVisitor
{
public:
    PHPVisitor(TypeMap&, int TSRMLS_DC);

    virtual bool visitClassDefStart(const Slice::ClassDefPtr&);
    virtual bool visitExceptionStart(const Slice::ExceptionPtr&);
    virtual bool visitStructStart(const Slice::StructPtr&);
    virtual void visitDictionary(const Slice::DictionaryPtr&);
    virtual void visitEnum(const Slice::EnumPtr&);
    virtual void visitConst(const Slice::ConstPtr&);

private:
    zend_class_entry* allocClass(const string&, const Slice::ContainedPtr&);
    zend_class_entry* findClass(const string&);

    TypeMap& _typeMap;
    int _module;
    zend_class_entry* _currentClass;
#ifdef ZTS
    TSRMLS_D;
#endif
};

//
// FactoryWrapper is an implementation of Ice::ObjectFactory that creates PHP objects.
// A single instance can be used for all types.
//
class FactoryWrapper : public Ice::ObjectFactory
{
public:
    FactoryWrapper(TypeMap&);

    virtual Ice::ObjectPtr create(const string&);
    virtual void destroy();

private:
    TypeMap& _typeMap;
};

//
// FactoryVisitor descends the Slice parse tree and registers an object factory for each
// non-abstract class.
//
class FactoryVisitor : public Slice::ParserVisitor
{
public:
    FactoryVisitor(const Ice::CommunicatorPtr&, TypeMap&);

    virtual bool visitClassDefStart(const Slice::ClassDefPtr&);

private:
    Ice::CommunicatorPtr _communicator;
    Ice::ObjectFactoryPtr _factory;
};

//
// The Slice parse tree. This is static because the Slice files are parsed only once,
// when the module is loaded.
//
static Slice::UnitPtr _unit;

//
// Parse the Slice files that define the types and operations available to a PHP script.
//
static bool
parseSlice(const string& argStr)
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
Slice_init(TSRMLS_D)
{
    //
    // Parse the Slice files.
    //
    char* parse = INI_STR("ice.parse");
    if(parse && strlen(parse) > 0)
    {
        if(!parseSlice(parse))
        {
            return false;
        }
    }

    return true;
}

bool
Slice_shutdown(TSRMLS_D)
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
Slice_createClasses(int module TSRMLS_DC)
{
    TypeMap* typeMap = static_cast<TypeMap*>(ICE_G(typeMap));
    assert(typeMap->empty());

    if(_unit)
    {
        PHPVisitor visitor(*typeMap, module TSRMLS_CC);
        _unit->visit(&visitor);
    }

    return true;
}

bool
Slice_destroyClasses(TSRMLS_D)
{
    TypeMap* typeMap = static_cast<TypeMap*>(ICE_G(typeMap));

    for(TypeMap::iterator p = typeMap->begin(); p != typeMap->end(); ++p)
    {
        ice_class_entry* ice = (ice_class_entry*)p->second;
        delete static_cast<Slice::ContainedPtr*>(ice->contained);
        delete static_cast<MarshalerPtr*>(ice->marshaler);
    }

    typeMap->clear();

    return true;
}

bool
Slice_registerFactories(const Ice::CommunicatorPtr& communicator TSRMLS_DC)
{
    if(_unit)
    {
        TypeMap* typeMap = static_cast<TypeMap*>(ICE_G(typeMap));
        FactoryVisitor visitor(communicator, *typeMap);
        _unit->visit(&visitor);
    }

    return true;
}

Slice::UnitPtr
Slice_getUnit()
{
    return _unit;
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

PHPVisitor::PHPVisitor(TypeMap& typeMap, int module TSRMLS_DC) :
    _typeMap(typeMap), _module(module), _currentClass(0)
{
#ifdef ZTS
    this->TSRMLS_C = TSRMLS_C;
#endif
}

bool
PHPVisitor::visitClassDefStart(const Slice::ClassDefPtr& p)
{
    //
    // Collect the base class and interfaces.
    //
    zend_class_entry* parent = NULL;
    zend_class_entry** interfaces = NULL;
    zend_uint numInterfaces = 0;
    Slice::ClassList bases = p->bases();
    if(!bases.empty())
    {
        if(!bases.front()->isInterface())
        {
            parent = findClass(bases.front()->scoped());
            if(parent == NULL)
            {
                return false;
            }
            bases.pop_front();
            assert((parent->type & ZEND_ACC_INTERFACE) == 0);
        }

        if(!bases.empty())
        {
            interfaces = static_cast<zend_class_entry**>(emalloc(bases.size() * sizeof(zend_class_entry*)));
            Slice::ClassList::iterator q;
            for(Slice::ClassList::iterator q = bases.begin(); q != bases.end(); ++q, ++numInterfaces)
            {
                interfaces[numInterfaces] = findClass((*q)->scoped());
                if(interfaces[numInterfaces] == NULL)
                {
                    return false;
                }
                assert(interfaces[numInterfaces]->type & ZEND_ACC_INTERFACE);
            }
        }
    }

    string scoped = p->scoped();
    zend_class_entry* cls = allocClass(scoped, p);
    if(p->isInterface())
    {
        cls->ce_flags |= ZEND_ACC_INTERFACE;
    }
    else if(p->isAbstract())
    {
        cls->ce_flags |= ZEND_ACC_ABSTRACT;
    }
    if(parent)
    {
        zend_do_inheritance(cls, parent);
    }
    cls->interfaces = interfaces;
    cls->num_interfaces = numInterfaces;
    for(zend_uint i = 0; i < numInterfaces; ++i)
    {
        zend_do_implement_interface(cls, interfaces[i]);
    }
    zend_hash_update(CG(class_table), cls->name, cls->name_length + 1, &cls, sizeof(zend_class_entry*), NULL);
    _typeMap[scoped] = cls;
    _currentClass = cls;

    return false;
}

bool
PHPVisitor::visitExceptionStart(const Slice::ExceptionPtr& p)
{
    //
    // Get the class entry for the base exception (if any).
    //
    zend_class_entry* parent = NULL;
    Slice::ExceptionPtr base = p->base();
    if(base)
    {
        parent = findClass(base->scoped());
        if(parent == NULL)
        {
            return false;
        }
    }

    string scoped = p->scoped();
    zend_class_entry* cls = allocClass(scoped, p);
    if(parent)
    {
        zend_do_inheritance(cls, parent);
    }
    zend_hash_update(CG(class_table), cls->name, cls->name_length + 1, &cls, sizeof(zend_class_entry*), NULL);
    _typeMap[scoped] = cls;

    return false;
}

bool
PHPVisitor::visitStructStart(const Slice::StructPtr& p)
{
    string scoped = p->scoped();

    //
    // Special case for Ice::Identity, which is predefined.
    //
    if(scoped == "::Ice::Identity")
    {
        return false;
    }

    zend_class_entry* cls = allocClass(scoped, p);
    cls->ce_flags |= ZEND_ACC_FINAL;
    zend_hash_update(CG(class_table), cls->name, cls->name_length + 1, &cls, sizeof(zend_class_entry*), NULL);
    _typeMap[scoped] = cls;

    return false;
}

void
PHPVisitor::visitDictionary(const Slice::DictionaryPtr& p)
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
PHPVisitor::visitEnum(const Slice::EnumPtr& p)
{
    string scoped = p->scoped();

    //
    // Enum values are represented as integers, however we define a class in order
    // to hold constants that symbolically identify the enumerators.
    //
    zend_class_entry* cls = allocClass(scoped, p);

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
        MAKE_STD_ZVAL(en);
        ZVAL_LONG(en, i);
        zend_hash_update(&cls->constants_table, const_cast<char*>(name.c_str()), name.length() + 1, &en,
                         sizeof(zval*), NULL);
    }
    zend_hash_update(CG(class_table), cls->name, cls->name_length + 1, &cls, sizeof(zend_class_entry*), NULL);
    _typeMap[scoped] = cls;
}

void
PHPVisitor::visitConst(const Slice::ConstPtr& p)
{
    string name = ice_flatten(p->scoped()); // Don't convert to lower case; constant names are case sensitive.
    char* cname = const_cast<char*>(name.c_str());
    uint cnameLen = name.length() + 1;
    string value = p->value();
    const int flags = CONST_CS; // Case sensitive

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
PHPVisitor::allocClass(const string& scoped, const Slice::ContainedPtr& contained)
{
    ice_class_entry* result = static_cast<ice_class_entry*>(emalloc(sizeof(ice_class_entry)));

    string flat = ice_lowerCase(ice_flatten(scoped));

    result->ce.type = ZEND_USER_CLASS;
    result->ce.name = estrndup(const_cast<char*>(flat.c_str()), flat.length());
    result->ce.name_length = flat.length();

    zend_initialize_class_data(&result->ce, 1 TSRMLS_CC);

    result->ce.ce_flags |= ZEND_ACC_PUBLIC;
    result->scoped = estrndup(const_cast<char*>(scoped.c_str()), scoped.length());
    result->contained = new Slice::ContainedPtr(contained);
    result->marshaler = 0;

    return (zend_class_entry*)result;
}

zend_class_entry*
PHPVisitor::findClass(const string& scoped)
{
    zend_class_entry* result = NULL;

    TypeMap::iterator p = _typeMap.find(scoped);
    if(p != _typeMap.end())
    {
        result = static_cast<zend_class_entry*>(p->second);
    }
    else
    {
        zend_error(E_ERROR, "no class found for type %s", scoped.c_str());
    }

    return result;
}

//
// FactoryWrapper
//
FactoryWrapper::FactoryWrapper(TypeMap& typeMap) :
    _typeMap(typeMap)
{
}

Ice::ObjectPtr
FactoryWrapper::create(const string& scoped)
{
    TypeMap::iterator p = _typeMap.find(scoped);
    assert(p != _typeMap.end());
    Ice::ObjectPtr result;
    // TODO
    return result;
}

void
FactoryWrapper::destroy()
{
}

//
// FactoryVisitor
//
FactoryVisitor::FactoryVisitor(const Ice::CommunicatorPtr& communicator, TypeMap& typeMap) :
    _communicator(communicator), _factory(new FactoryWrapper(typeMap))
{
}

bool
FactoryVisitor::visitClassDefStart(const Slice::ClassDefPtr& p)
{
    if(!p->isAbstract())
    {
        _communicator->addObjectFactory(_factory, p->scoped());
    }

    return false;
}
