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
#include "php_ice.h"

#include <Slice/Preprocessor.h>

using namespace std;

//
// Member validates a struct or class data member.
//
class Member : public IceUtil::SimpleShared
{
public:
    Member(const string&, const Slice::TypePtr&);
    ~Member();

    bool checkValue(zval* TSRMLS_DC) const;
    zval* getDefaultProperty() const;

private:
    void createDefaultProperty();

    string _name;
    Slice::TypePtr _type;
    zval* _defaultProperty;
};
typedef IceUtil::Handle<Member> MemberPtr;
typedef map<string, MemberPtr> MemberMap;

//
// Encapsulates a Slice type.
//
class TypeInfo : public IceUtil::SimpleShared
{
public:
    virtual ~TypeInfo();

    Slice::TypePtr getType() const;

protected:
    TypeInfo(const Slice::TypePtr&);

    Slice::TypePtr _type;
};
typedef IceUtil::Handle<TypeInfo> TypeInfoPtr;

typedef map<string, TypeInfoPtr> TypeMap; // Key is a flattened name
static TypeMap _typeMap;

class StructInfo : public TypeInfo
{
public:
    StructInfo(const Slice::StructPtr&);

    string getScoped() const;
    void addDefaultProperties(zend_class_entry*) const;
    bool hasMember(const string&) const;
    bool validateMember(const string&, zval* TSRMLS_DC) const;

private:
    string _scoped;
    MemberMap _members;
};
typedef IceUtil::Handle<StructInfo> StructInfoPtr;

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
// Convert a scoped name into a flattened name (e.g., "::MyModule::MyType" => "MyModule_MyType").
//
static string
flatten(const string& str)
{
    string result = str;
    if(result.find("::") == 0)
    {
        result.erase(0, 2);
    }

    string::size_type pos;
    while((pos = result.find("::")) != string::npos)
    {
        result.replace(pos, 2, "_");
    }

    return result;
}

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

Member::Member(const string& name, const Slice::TypePtr& type) :
    _name(name), _type(type)
{
    createDefaultProperty();
}

Member::~Member()
{
    //zval_dtor(_defaultProperty);
    //free(_defaultProperty);
}

bool
Member::checkValue(zval* zv TSRMLS_DC) const
{
    Slice::BuiltinPtr builtin = Slice::BuiltinPtr::dynamicCast(_type);
    if(builtin)
    {
        switch(builtin->kind())
        {
        case Slice::Builtin::KindByte:
        {
            if(Z_TYPE_P(zv) != IS_LONG)
            {
                string s = Marshaler::zendTypeToString(Z_TYPE_P(zv));
                zend_error(E_ERROR, "member %s expects type byte but received %s", _name.c_str(), s.c_str());
                return false;
            }
            long val = Z_LVAL_P(zv);
            if(val < 0 || val > 255)
            {
                zend_error(E_ERROR, "value %ld is out of range for byte member %s", val, _name.c_str());
                return false;
            }

            return true;
        }

        case Slice::Builtin::KindBool:
        {
            if(Z_TYPE_P(zv) != IS_BOOL)
            {
                string s = Marshaler::zendTypeToString(Z_TYPE_P(zv));
                zend_error(E_ERROR, "member %s expects type bool but received %s", _name.c_str(), s.c_str());
                return false;
            }

            return true;
        }

        case Slice::Builtin::KindShort:
        {
            if(Z_TYPE_P(zv) != IS_LONG)
            {
                string s = Marshaler::zendTypeToString(Z_TYPE_P(zv));
                zend_error(E_ERROR, "member %s expects type short but received %s", _name.c_str(), s.c_str());
                return false;
            }
            long val = Z_LVAL_P(zv);
            if(val < SHRT_MIN || val > SHRT_MAX)
            {
                zend_error(E_ERROR, "value %ld is out of range for short member %s", val, _name.c_str());
                return false;
            }

            return true;
        }

        case Slice::Builtin::KindInt:
        {
            if(Z_TYPE_P(zv) != IS_LONG)
            {
                string s = Marshaler::zendTypeToString(Z_TYPE_P(zv));
                zend_error(E_ERROR, "member %s expects type int but received %s", _name.c_str(), s.c_str());
                return false;
            }
            long val = Z_LVAL_P(zv);
            if(val < INT_MIN || val > INT_MAX)
            {
                zend_error(E_ERROR, "value %ld is out of range for int member %s", val, _name.c_str());
                return false;
            }

            return true;
        }

        case Slice::Builtin::KindLong:
        {
            //
            // The platform's 'long' type may not be 64 bits, so we also accept
            // a string argument for this type.
            //
            if(Z_TYPE_P(zv) != IS_LONG && Z_TYPE_P(zv) != IS_STRING)
            {
                string s = Marshaler::zendTypeToString(Z_TYPE_P(zv));
                zend_error(E_ERROR, "member %s expects type long but received %s", _name.c_str(), s.c_str());
                return false;
            }

            if(Z_TYPE_P(zv) == IS_STRING)
            {
                string sval(Z_STRVAL_P(zv), Z_STRLEN_P(zv));
                string::size_type pos;
                Ice::Long l;
                if(!IceUtil::stringToInt64(sval, l, pos))
                {
                    zend_error(E_ERROR, "invalid long value `%s' for member %s", Z_STRVAL_P(zv), _name.c_str());
                    return false;
                }
            }

            return true;
        }

        case Slice::Builtin::KindFloat:
        {
            if(Z_TYPE_P(zv) != IS_DOUBLE)
            {
                string s = Marshaler::zendTypeToString(Z_TYPE_P(zv));
                zend_error(E_ERROR, "member %s expects type float but received %s", _name.c_str(), s.c_str());
                return false;
            }

            return true;
        }

        case Slice::Builtin::KindDouble:
        {
            if(Z_TYPE_P(zv) != IS_DOUBLE)
            {
                string s = Marshaler::zendTypeToString(Z_TYPE_P(zv));
                zend_error(E_ERROR, "member %s expects type double but received %s", _name.c_str(), s.c_str());
                return false;
            }

            return true;
        }

        case Slice::Builtin::KindString:
        {
            if(Z_TYPE_P(zv) != IS_STRING && Z_TYPE_P(zv) != IS_NULL)
            {
                string s = Marshaler::zendTypeToString(Z_TYPE_P(zv));
                zend_error(E_ERROR, "member %s expects type string but received %s", _name.c_str(), s.c_str());
                return false;
            }

            return true;
        }

        case Slice::Builtin::KindObject:
        {
            // TODO: Further type checking required to ensure the value is actually an
            // instance of a class
            if(Z_TYPE_P(zv) != IS_OBJECT && Z_TYPE_P(zv) != IS_NULL)
            {
                string s = Marshaler::zendTypeToString(Z_TYPE_P(zv));
                zend_error(E_ERROR, "member %s expects type class but received %s", _name.c_str(), s.c_str());
                return false;
            }

            return true;
        }

        case Slice::Builtin::KindObjectProxy:
        {
            // TODO: Further type checking required to ensure the value is actually a proxy
            if(Z_TYPE_P(zv) != IS_OBJECT && Z_TYPE_P(zv) != IS_NULL)
            {
                string s = Marshaler::zendTypeToString(Z_TYPE_P(zv));
                zend_error(E_ERROR, "member %s expects type proxy but received %s", _name.c_str(), s.c_str());
                return false;
            }

            return true;
        }

        case Slice::Builtin::KindLocalObject:
        {
            zend_error(E_ERROR, "unexpected local type");
            return false;
        }
        }
    }

    Slice::SequencePtr seq = Slice::SequencePtr::dynamicCast(_type);
    if(seq)
    {
        if(Z_TYPE_P(zv) != IS_ARRAY && Z_TYPE_P(zv) != IS_NULL)
        {
            string s = Marshaler::zendTypeToString(Z_TYPE_P(zv));
            zend_error(E_ERROR, "member %s expects type array but received %s", _name.c_str(), s.c_str());
            return false;
        }

        return true;
    }

    Slice::ProxyPtr proxy = Slice::ProxyPtr::dynamicCast(_type);
    if(proxy)
    {
        // TODO: Compare types
        if(Z_TYPE_P(zv) != IS_OBJECT && Z_TYPE_P(zv) != IS_NULL)
        {
            string s = Marshaler::zendTypeToString(Z_TYPE_P(zv));
            zend_error(E_ERROR, "member %s expects type proxy but received %s", _name.c_str(), s.c_str());
            return false;
        }

        return true;
    }

    Slice::DictionaryPtr dict = Slice::DictionaryPtr::dynamicCast(_type);
    if(dict)
    {
        // TODO: Compare types
        if(Z_TYPE_P(zv) != IS_OBJECT && Z_TYPE_P(zv) != IS_NULL)
        {
            string s = Marshaler::zendTypeToString(Z_TYPE_P(zv));
            zend_error(E_ERROR, "member %s expects type dictionary but received %s", _name.c_str(), s.c_str());
            return false;
        }

        return true;
    }

    Slice::ClassDeclPtr decl = Slice::ClassDeclPtr::dynamicCast(_type);
    if(decl)
    {
        // TODO: Compare types
        if(Z_TYPE_P(zv) != IS_OBJECT && Z_TYPE_P(zv) != IS_NULL)
        {
            string s = Marshaler::zendTypeToString(Z_TYPE_P(zv));
            zend_error(E_ERROR, "member %s expects type class but received %s", _name.c_str(), s.c_str());
            return false;
        }

        return true;
    }

    return false;
}

zval*
Member::getDefaultProperty() const
{
    return _defaultProperty;
}

void
Member::createDefaultProperty()
{
    //_defaultProperty = (zval*)malloc(sizeof(zval));
    //INIT_PZVAL(_defaultProperty);
    MAKE_STD_ZVAL(_defaultProperty);

    Slice::BuiltinPtr builtin = Slice::BuiltinPtr::dynamicCast(_type);
    if(builtin)
    {
        switch(builtin->kind())
        {
        case Slice::Builtin::KindBool:
        {
            ZVAL_FALSE(_defaultProperty);
            return;
        }

        case Slice::Builtin::KindByte:
        case Slice::Builtin::KindShort:
        case Slice::Builtin::KindInt:
        case Slice::Builtin::KindLong:
        {
            ZVAL_LONG(_defaultProperty, 1);
            return;
        }

        case Slice::Builtin::KindFloat:
        case Slice::Builtin::KindDouble:
        {
            ZVAL_DOUBLE(_defaultProperty, 0);
            return;
        }

        case Slice::Builtin::KindString:
        {
            ZVAL_EMPTY_STRING(_defaultProperty);
            return;
        }

        case Slice::Builtin::KindObject:
        case Slice::Builtin::KindObjectProxy:
        {
            ZVAL_NULL(_defaultProperty);
            return;
        }

        case Slice::Builtin::KindLocalObject:
        {
            zend_error(E_ERROR, "unexpected local type");
            return;
        }
        }
    }

    Slice::SequencePtr seq = Slice::SequencePtr::dynamicCast(_type);
    if(seq)
    {
        array_init(_defaultProperty);
        return;
    }

    Slice::ProxyPtr proxy = Slice::ProxyPtr::dynamicCast(_type);
    if(proxy)
    {
        ZVAL_NULL(_defaultProperty);
        return;
    }

    Slice::DictionaryPtr dict = Slice::DictionaryPtr::dynamicCast(_type);
    if(dict)
    {
        ZVAL_NULL(_defaultProperty);
        return;
    }

    Slice::ClassDeclPtr decl = Slice::ClassDeclPtr::dynamicCast(_type);
    if(decl)
    {
        ZVAL_NULL(_defaultProperty);
        return;
    }
}

TypeInfo::TypeInfo(const Slice::TypePtr& type) :
    _type(type)
{
}

TypeInfo::~TypeInfo()
{
}

Slice::TypePtr
TypeInfo::getType() const
{
    return _type;
}

StructInfo::StructInfo(const Slice::StructPtr& type) :
    TypeInfo(type)
{
    _scoped = type->scoped();

    Slice::DataMemberList members = type->dataMembers();
    for(Slice::DataMemberList::iterator p = members.begin(); p != members.end(); ++p)
    {
        string name = (*p)->name();
        _members[ice_lowercase(name)] = new Member(name, (*p)->type());
    }
}

string
StructInfo::getScoped() const
{
    return _scoped;
}

void
StructInfo::addDefaultProperties(zend_class_entry* cls) const
{
    for(MemberMap::const_iterator p = _members.begin(); p != _members.end(); ++p)
    {
        zval* val = p->second->getDefaultProperty();
        if(zend_hash_update(&cls->default_properties, const_cast<char*>(p->first.c_str()), p->first.length() + 1,
                            &val, sizeof(zval*), NULL) == FAILURE)
        {
            // TODO
        }
    }
}

bool
StructInfo::hasMember(const string& name) const
{
    MemberMap::const_iterator p = _members.find(ice_lowercase(name));
    return(p != _members.end());
}

bool
StructInfo::validateMember(const string& name, zval* zv TSRMLS_DC) const
{
    MemberMap::const_iterator p = _members.find(name);
    if(p == _members.end())
    {
        string flat = flatten(_scoped);
        zend_error(E_ERROR, "%s does not have a member `%s'", flat.c_str(), name.c_str());
        return false;
    }

    return p->second->checkValue(zv);
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

static zend_object_handlers struct_handlers;
static void struct_dtor(void*, zend_object_handle TSRMLS_DC);
static zval* struct_read_prop(zval*, zval* TSRMLS_DC);
static void struct_write_prop(zval*, zval*, zval* TSRMLS_DC);

static zend_object_value
struct_alloc(zend_class_entry* ce TSRMLS_DC)
{
    TypeMap::iterator p = _typeMap.find(ce->name);
    assert(p != _typeMap.end());

    zend_object_value result;

    ice_object* obj = ice_object_new(ce TSRMLS_CC);
    assert(obj);

    result.handle = zend_objects_store_put(obj, struct_dtor, NULL TSRMLS_CC);
    result.handlers = &struct_handlers;

    StructInfoPtr st = StructInfoPtr::dynamicCast(p->second);
    assert(st);
    obj->ptr = new StructInfoPtr(st);

    return result;
}

static void
struct_dtor(void* p, zend_object_handle handle TSRMLS_DC)
{
    ice_object* obj = static_cast<ice_object*>(p);
    StructInfoPtr* _this = static_cast<StructInfoPtr*>(obj->ptr);

    delete _this;

    zend_objects_destroy_object(static_cast<zend_object*>(p), handle TSRMLS_CC);
}

static zval*
struct_read_prop(zval* object, zval* member TSRMLS_DC)
{
    zval tmp;
    zval* result = NULL;

    //
    // TODO: Do we really need to handle this case?
    //
    if(Z_TYPE_P(member) != IS_STRING)
    {
        tmp = *member;
        zval_copy_ctor(&tmp);
        convert_to_string(&tmp);
        member = &tmp;
    }

    ice_object* obj = static_cast<ice_object*>(zend_object_store_get_object(object TSRMLS_CC));
    assert(obj->ptr);
    StructInfoPtr* _this = static_cast<StructInfoPtr*>(obj->ptr);

    if(!(*_this)->hasMember(Z_STRVAL_P(member)))
    {
        string flat = flatten((*_this)->getScoped());
        zend_error(E_ERROR, "member %s not found in type %s", Z_STRVAL_P(member), flat.c_str());
    }
    else
    {
        //
        // Now that we've done some error checking, we can delegate to the standard handler.
        //
        zend_object_handlers* handlers = zend_get_std_object_handlers();
        result = handlers->read_property(object, member TSRMLS_CC);
    }

    if(member == &tmp)
    {
        zval_dtor(member);
    }

    return result;
}

static void
struct_write_prop(zval* object, zval* member, zval* value TSRMLS_DC)
{
    zval tmp;

    //
    // TODO: Do we really need to handle this case?
    //
    if(Z_TYPE_P(member) != IS_STRING)
    {
        tmp = *member;
        zval_copy_ctor(&tmp);
        convert_to_string(&tmp);
        member = &tmp;
    }

    ice_object* obj = static_cast<ice_object*>(zend_object_store_get_object(object TSRMLS_CC));
    assert(obj->ptr);
    StructInfoPtr* _this = static_cast<StructInfoPtr*>(obj->ptr);

    if((*_this)->validateMember(Z_STRVAL_P(member), value TSRMLS_CC))
    {
        //
        // Now that we've done some type checking, we can delegate to the standard handler.
        //
        zend_object_handlers* handlers = zend_get_std_object_handlers();
        handlers->write_property(object, member, value TSRMLS_CC);
    }

    if(member == &tmp)
    {
        zval_dtor(member);
    }
}

#if 0
static void
dump_class_table()
{
    HashPosition pos;
    zend_hash_internal_pointer_reset_ex(CG(class_table), &pos);
    zval* val;
    char* key;
    uint len;
    ulong index;
    while(zend_hash_get_current_key_ex(CG(class_table), &key, &len, &index, 0, &pos) == HASH_KEY_IS_STRING)
    {
        zend_hash_move_forward_ex(CG(class_table), &pos);
    }
}
#endif

bool
Visitor::visitStructStart(const Slice::StructPtr& p)
{
    string flat = flatten(p->scoped());
    zend_class_entry ce;
    INIT_CLASS_ENTRY(ce, flat.c_str(), NULL);
    //
    // We have to reset name_length because the INIT_CLASS_ENTRY macro assumes the class name
    // is a string constant.
    //
    ce.name_length = flat.length();
    ce.create_object = struct_alloc;
    // TODO: Check for conflicts with existing symbols
    zend_class_entry* cls = zend_register_internal_class(&ce TSRMLS_CC);
    memcpy(&struct_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
    struct_handlers.read_property = struct_read_prop;
    struct_handlers.write_property = struct_write_prop;

    StructInfoPtr info = new StructInfo(p);
    // TODO: This causes a segfault on PHP shutdown
    //info->addDefaultProperties(cls);
    _typeMap[flat] = info;

    return false;
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
