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

#include "struct.h"
#include "util.h"

using namespace std;

//
// Struct support.
//
static zend_object_handlers struct_handlers;
static zend_object_value struct_alloc(zend_class_entry* TSRMLS_DC);
static void struct_dtor(void*, zend_object_handle TSRMLS_DC);
static zval* struct_read_prop(zval*, zval* TSRMLS_DC);
static void struct_write_prop(zval*, zval*, zval* TSRMLS_DC);

//
// Encapsulates a Slice struct.
//
class StructType;
typedef IceUtil::Handle<StructType> StructTypePtr;

class StructType : public Marshaler
{
public:
    static StructTypePtr create(const Slice::StructPtr&);
    static StructTypePtr find(const string&);

    virtual bool marshal(zval*, IceInternal::BasicStream& TSRMLS_DC);
    virtual bool unmarshal(zval*, IceInternal::BasicStream& TSRMLS_DC);

    Slice::TypePtr getType() const;
    bool hasMember(const string&) const;

private:
    StructType(const Slice::StructPtr&, zend_class_entry*);

    Slice::StructPtr _type;
    zend_class_entry* _entry;
    map<string, bool> _memberNames; // Key is a lowercase name
    vector<MarshalerPtr> _members; // In order of declaration

    //
    // Maintain a cache of the StructType objects we've created.
    //
    typedef map<string, StructTypePtr> TypeMap; // Key is a lowercase flattened name
    static TypeMap _typeMap;
};

StructType::TypeMap StructType::_typeMap;

StructType::StructType(const Slice::StructPtr& type, zend_class_entry* entry) :
    _type(type), _entry(entry)
{
    Slice::DataMemberList members = type->dataMembers();
    for(Slice::DataMemberList::iterator p = members.begin(); p != members.end(); ++p)
    {
        string name = ice_lowercase((*p)->name());
        _memberNames[name] = true;
        MarshalerPtr marshaler = createMemberMarshaler(name, (*p)->type());
        assert(marshaler);
        _members.push_back(marshaler);
    }
}

StructTypePtr
StructType::create(const Slice::StructPtr& type)
{
    StructTypePtr result;

    //
    // PHP symbol names are case-insensitive.
    //
    string flat = ice_lowercase(ice_flatten(type->scoped()));

    //
    // Check the cache.
    //
    TypeMap::iterator p = _typeMap.find(flat);
    if(p == _typeMap.end())
    {
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

        result = new StructType(type, cls);
        _typeMap[flat] = result;
    }
    else
    {
        result = p->second;
    }

    return result;
}

StructTypePtr
StructType::find(const string& flat)
{
    StructTypePtr result;

    TypeMap::iterator p = _typeMap.find(flat);
    if(p != _typeMap.end())
    {
        result = p->second;
    }

    return result;
}

bool
StructType::marshal(zval* zv, IceInternal::BasicStream& os TSRMLS_DC)
{
    if(Z_TYPE_P(zv) != IS_OBJECT)
    {
        string s = zendTypeToString(Z_TYPE_P(zv));
        zend_error(E_ERROR, "expected struct value of type %s but received %s", _entry->name, s.c_str());
        return false;
    }

    //
    // Compare class entries.
    //
    ice_object* obj = ice_object_get(zv TSRMLS_CC);
    assert(obj);
    if(obj->zobj.ce != _entry)
    {
        zend_error(E_ERROR, "expected struct value of type %s but received %s", _entry->name, obj->zobj.ce->name);
        return false;
    }

    for(vector<MarshalerPtr>::iterator p = _members.begin(); p != _members.end(); ++p)
    {
        if(!(*p)->marshal(zv, os))
        {
            return false;
        }
    }

    return true;
}

bool
StructType::unmarshal(zval* zv, IceInternal::BasicStream& is TSRMLS_DC)
{
    if(object_init_ex(zv, _entry) != SUCCESS)
    {
        zend_error(E_ERROR, "unable to initialize object of type %s", _entry->name);
        return false;
    }

    for(vector<MarshalerPtr>::iterator p = _members.begin(); p != _members.end(); ++p)
    {
        if(!(*p)->unmarshal(zv, is))
        {
            return false;
        }
    }

    return true;
}

Slice::TypePtr
StructType::getType() const
{
    return _type;
}

bool
StructType::hasMember(const string& name) const
{
    map<string, bool>::const_iterator p = _memberNames.find(ice_lowercase(name));
    return(p != _memberNames.end());
}

static zend_object_value
struct_alloc(zend_class_entry* ce TSRMLS_DC)
{
    StructTypePtr type = StructType::find(ce->name);
    assert(type);

    zend_object_value result;

    ice_object* obj = ice_object_new(ce TSRMLS_CC);
    assert(obj);

    result.handle = zend_objects_store_put(obj, struct_dtor, NULL TSRMLS_CC);
    result.handlers = &struct_handlers;

    obj->ptr = new StructTypePtr(type);

    return result;
}

static void
struct_dtor(void* p, zend_object_handle handle TSRMLS_DC)
{
    ice_object* obj = static_cast<ice_object*>(p);
    StructTypePtr* _this = static_cast<StructTypePtr*>(obj->ptr);

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
    StructTypePtr* _this = static_cast<StructTypePtr*>(obj->ptr);

    if(!(*_this)->hasMember(Z_STRVAL_P(member)))
    {
        zend_error(E_ERROR, "member %s not found in type %s", Z_STRVAL_P(member), obj->zobj.ce->name);
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
    StructTypePtr* _this = static_cast<StructTypePtr*>(obj->ptr);

    if(!(*_this)->hasMember(Z_STRVAL_P(member)))
    {
        zend_error(E_ERROR, "member %s not found in type %s", Z_STRVAL_P(member), obj->zobj.ce->name);
    }
    else
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

bool
Struct_register(const Slice::StructPtr& p)
{
    StructTypePtr type = StructType::create(p);
    return p != 0;
}

MarshalerPtr
Struct_create_marshaler(const Slice::StructPtr& p)
{
    return StructType::create(p);
}
