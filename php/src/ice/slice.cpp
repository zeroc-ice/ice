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

//
// PHP has two types of classes: internal and user. Internal classes are intended to be
// implemented by extensions, whereas user classes are generally created while parsing
// class definitions in a script. The lifetimes of these two types of classes are different:
// user classes are destroyed after each request, whereas internal classes survive until
// module shutdown. PHP also requires the use of different allocation schemes for creating
// internal and user classes, which has additional implications.
//
// In order for a script to have access to Slice types, the classes for those types must be
// defined prior to script execution. We initially tried creating internal classes for
// the Slice types, because we would only have to create them once, during module startup.
// However, we eventually realized that these classes should not be internal classes - the
// extension is not implementing these classes, it is simply defining them. In other words,
// the extension needs to create classes just like PHP's parser does, as if the Slice
// definitions had first been translated into PHP code, and then parsed by the interpreter.
//
// Therefore, we now create user classes from the Slice definitions for every request.
// Note that we encountered problems when trying to create function prototypes for class or
// interface operations, so we do not attempt that.
//
// For structs, classes and exceptions, we create "default properties" in the class
// definition taht correspond to the type's data members. When a new instance of the class
// is created, the interpreter copies these default properties to the new instance so that
// the data members have default values. It's not technically necessary to do this, but it
// does resolve a usability issue. Namely, the marshaling code expects all data members to
// be defined. If we did not supply default properties, then the user would be required to
// ensure that each data member was defined before attempting to marshal it.
//

//
// PHPVisitor descends the Slice parse tree and creates PHP classes for certain Slice types.
//
class PHPVisitor : public Slice::ParserVisitor
{
public:
    PHPVisitor(TypeMap&, int TSRMLS_DC);

    virtual void visitClassDecl(const Slice::ClassDeclPtr&);
    virtual bool visitClassDefStart(const Slice::ClassDefPtr&);
    virtual bool visitExceptionStart(const Slice::ExceptionPtr&);
    virtual bool visitStructStart(const Slice::StructPtr&);
    virtual void visitDataMember(const Slice::DataMemberPtr&);
    virtual void visitDictionary(const Slice::DictionaryPtr&);
    virtual void visitEnum(const Slice::EnumPtr&);
    virtual void visitConst(const Slice::ConstPtr&);

private:
    zend_class_entry* findClass(const string&);
    bool getDefaultValue(zval*, const Slice::TypePtr&);

    TypeMap& _typeMap;
    int _module;
#ifdef ZTS
    TSRMLS_D;
#endif
    zend_class_entry* _iceObjectClass;
    zend_class_entry* _currentClass;
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
    bool ice = true; // This must be true so that we can create Ice::Identity when necessary
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
    bool all = true;
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

static zend_class_entry*
allocClass(const string& scoped, const Slice::SyntaxTreeBasePtr& p TSRMLS_DC)
{
    ice_class_entry* result = static_cast<ice_class_entry*>(emalloc(sizeof(ice_class_entry)));

    string flat = ice_lowerCase(ice_flatten(scoped));

    result->ce.type = ZEND_USER_CLASS;
    result->ce.name = estrndup(const_cast<char*>(flat.c_str()), flat.length());
    result->ce.name_length = flat.length();

    zend_initialize_class_data((zend_class_entry*)result, 1 TSRMLS_CC);

    result->ce.ce_flags |= ZEND_ACC_PUBLIC;
    result->scoped = estrndup(const_cast<char*>(scoped.c_str()), scoped.length());
    result->syntaxTreeBase = new Slice::SyntaxTreeBasePtr(p);
    result->marshaler = 0;

    return (zend_class_entry*)result;
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
    else
    {
        //
        // We must be allowed to obtain builtin types, as well as create Ice::Identity if necessary.
        //
        _unit = Slice::Unit::createUnit(false, false, true, false);
    }

    //
    // Create the Slice definition for Ice::Identity if it doesn't exist. The PHP class will
    // be created automatically by PHPVisitor.
    //
    string scoped = "::Ice::Identity";
    Slice::TypeList l = _unit->lookupTypeNoBuiltin(scoped, false);
    if(l.empty())
    {
        Slice::ContainedList c = _unit->lookupContained("Ice", false);
        Slice::ModulePtr module;
        if(c.empty())
        {
            module = _unit->createModule("Ice");
        }
        else
        {
            module = Slice::ModulePtr::dynamicCast(c.front());
            if(!module)
            {
                zend_error(E_ERROR, "the symbol `::Ice' is defined in Slice but is not a module");
                return false;
            }
        }
        Slice::StructPtr identity = module->createStruct("Identity", false);
        Slice::TypePtr str = _unit->builtin(Slice::Builtin::KindString);
        identity->createDataMember("category", str);
        identity->createDataMember("name", str);
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

    //
    // Create the class Ice_Object.
    //
    {
        string scoped = Ice::Object::ice_staticId();
        Slice::TypePtr p = _unit->builtin(Slice::Builtin::KindObject);
        zend_class_entry* cls = allocClass(scoped, p TSRMLS_CC);
        zval* facetMap;
        MAKE_STD_ZVAL(facetMap);
        array_init(facetMap);
        zend_hash_add(&cls->default_properties, "ice_facets", sizeof("ice_facets"), (void**)&facetMap, sizeof(zval*),
                      NULL);
        zend_hash_update(CG(class_table), cls->name, cls->name_length + 1, (void**)&cls, sizeof(zend_class_entry*),
                         NULL);
        (*typeMap)[scoped] = cls;
    }

    //
    // Descend the parse tree and create PHP classes.
    //
    PHPVisitor visitor(*typeMap, module TSRMLS_CC);
    _unit->visit(&visitor);

    return true;
}

bool
Slice_destroyClasses(TSRMLS_D)
{
    TypeMap* typeMap = static_cast<TypeMap*>(ICE_G(typeMap));

    for(TypeMap::iterator p = typeMap->begin(); p != typeMap->end(); ++p)
    {
        ice_class_entry* ice = (ice_class_entry*)p->second;
        efree(ice->scoped);
        delete static_cast<Slice::SyntaxTreeBasePtr*>(ice->syntaxTreeBase);
        delete static_cast<MarshalerPtr*>(ice->marshaler);
    }

    typeMap->clear();

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
    _iceObjectClass = findClass("::Ice::Object");
    assert(_iceObjectClass);
}

void
PHPVisitor::visitClassDecl(const Slice::ClassDeclPtr& p)
{
    Slice::ClassDefPtr def = p->definition();
    if(!def)
    {
        string scoped = p->scoped();
        zend_error(E_WARNING, "%s %s declared but not defined", p->isInterface() ? "interface" : "class",
                   scoped.c_str());
    }
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
            assert((parent->ce_flags & ZEND_ACC_INTERFACE) == 0);
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
                assert(interfaces[numInterfaces]->ce_flags & ZEND_ACC_INTERFACE);
            }
        }
    }

    if(parent == NULL && !p->isInterface())
    {
        parent = _iceObjectClass;
    }

    string scoped = p->scoped();
    zend_class_entry* cls = allocClass(scoped, p TSRMLS_CC);
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
    zend_hash_update(CG(class_table), cls->name, cls->name_length + 1, (void**)&cls, sizeof(zend_class_entry*), NULL);
    _typeMap[scoped] = cls;
    _currentClass = cls;

    return true;
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
    zend_class_entry* cls = allocClass(scoped, p TSRMLS_CC);
    if(parent)
    {
        zend_do_inheritance(cls, parent);
    }
    zend_hash_update(CG(class_table), cls->name, cls->name_length + 1, (void**)&cls, sizeof(zend_class_entry*), NULL);
    _typeMap[scoped] = cls;
    _currentClass = cls;

    return true;
}

bool
PHPVisitor::visitStructStart(const Slice::StructPtr& p)
{
    string scoped = p->scoped();

    zend_class_entry* cls = allocClass(scoped, p TSRMLS_CC);
    cls->ce_flags |= ZEND_ACC_FINAL;
    zend_hash_update(CG(class_table), cls->name, cls->name_length + 1, (void**)&cls, sizeof(zend_class_entry*), NULL);
    _typeMap[scoped] = cls;
    _currentClass = cls;

    return true;
}

void
PHPVisitor::visitDataMember(const Slice::DataMemberPtr& p)
{
    assert(_currentClass);

    zval* zv;
    MAKE_STD_ZVAL(zv);
    if(getDefaultValue(zv, p->type()))
    {
        string name = ice_fixIdent(p->name());
        zend_hash_add(&_currentClass->default_properties, const_cast<char*>(name.c_str()), name.length() + 1,
                      (void**)&zv, sizeof(zval*), NULL);
    }
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
        zend_error(E_WARNING, "skipping dictionary %s - unsupported key type", scoped.c_str());
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
    zend_class_entry* cls = allocClass(scoped, p TSRMLS_CC);

    //
    // Create a class constant for each enumerator.
    //
    Slice::EnumeratorList l = p->getEnumerators();
    Slice::EnumeratorList::const_iterator q;
    long i;
    for(q = l.begin(), i = 0; q != l.end(); ++q, ++i)
    {
        string name = ice_fixIdent((*q)->name());
        zval* en;
        MAKE_STD_ZVAL(en);
        ZVAL_LONG(en, i);
        zend_hash_update(&cls->constants_table, const_cast<char*>(name.c_str()), name.length() + 1, (void**)&en,
                         sizeof(zval*), NULL);
    }
    zend_hash_update(CG(class_table), cls->name, cls->name_length + 1, (void**)&cls, sizeof(zend_class_entry*), NULL);
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
                char* cvalue = estrndup(const_cast<char*>(value.c_str()), value.length());
                zend_register_stringl_constant(cname, cnameLen, cvalue, value.length(), flags, _module TSRMLS_CC);
            }
            else
            {
                zend_register_long_constant(cname, cnameLen, static_cast<long>(l), flags, _module TSRMLS_CC);
            }
            break;
        }

        case Slice::Builtin::KindString:
        {
            char* cvalue = estrndup(const_cast<char*>(value.c_str()), value.length());
            zend_register_stringl_constant(cname, cnameLen, cvalue, value.length(), flags, _module TSRMLS_CC);
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

bool
PHPVisitor::getDefaultValue(zval* zv, const Slice::TypePtr& type)
{
    Slice::BuiltinPtr b = Slice::BuiltinPtr::dynamicCast(type);
    if(b)
    {
        switch(b->kind())
        {
        case Slice::Builtin::KindBool:
            ZVAL_FALSE(zv);
            return true;

        case Slice::Builtin::KindByte:
        case Slice::Builtin::KindShort:
        case Slice::Builtin::KindInt:
        case Slice::Builtin::KindLong:
            ZVAL_LONG(zv, 0);
            return true;

        case Slice::Builtin::KindString:
            ZVAL_EMPTY_STRING(zv);
            return true;

        case Slice::Builtin::KindFloat:
        case Slice::Builtin::KindDouble:
            ZVAL_DOUBLE(zv, 0.0);
            return true;

        case Slice::Builtin::KindObject:
        case Slice::Builtin::KindObjectProxy:
        case Slice::Builtin::KindLocalObject:
            ZVAL_NULL(zv);
            return true;
        }
    }

    Slice::EnumPtr en = Slice::EnumPtr::dynamicCast(type);
    if(en)
    {
        ZVAL_LONG(zv, 0);
        return true;
    }

    Slice::SequencePtr seq = Slice::SequencePtr::dynamicCast(type);
    if(seq)
    {
        array_init(zv);
        return true;
    }

    Slice::DictionaryPtr dict = Slice::DictionaryPtr::dynamicCast(type);
    if(dict)
    {
        array_init(zv);
        return true;
    }

    ZVAL_NULL(zv);
    return true;
}
