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

#include "marshal.h"
#include "proxy.h"
#include "slice.h"
#include "util.h"

#include <IceUtil/InputUtil.h>

using namespace std;

class PrimitiveMarshaler : public Marshaler
{
public:
    PrimitiveMarshaler(const Slice::BuiltinPtr&);
    ~PrimitiveMarshaler();

    virtual bool marshal(zval*, IceInternal::BasicStream& TSRMLS_DC);
    virtual bool unmarshal(zval*, IceInternal::BasicStream& TSRMLS_DC);

private:
    Slice::BuiltinPtr _type;
};

class SequenceMarshaler : public Marshaler
{
public:
    SequenceMarshaler(const Slice::SequencePtr&);
    ~SequenceMarshaler();

    virtual bool marshal(zval*, IceInternal::BasicStream& TSRMLS_DC);
    virtual bool unmarshal(zval*, IceInternal::BasicStream& TSRMLS_DC);

private:
    Slice::SequencePtr _type;
    MarshalerPtr _elementMarshaler;
};

class ProxyMarshaler : public Marshaler
{
public:
    ProxyMarshaler(const Slice::TypePtr&);
    ~ProxyMarshaler();

    virtual bool marshal(zval*, IceInternal::BasicStream& TSRMLS_DC);
    virtual bool unmarshal(zval*, IceInternal::BasicStream& TSRMLS_DC);

private:
    Slice::TypePtr _type;
};

class MemberMarshaler : public Marshaler
{
public:
    MemberMarshaler(const string&, const MarshalerPtr&);
    ~MemberMarshaler();

    virtual bool marshal(zval*, IceInternal::BasicStream& TSRMLS_DC);
    virtual bool unmarshal(zval*, IceInternal::BasicStream& TSRMLS_DC);

private:
    string _name;
    MarshalerPtr _marshaler;
};

class StructMarshaler : public Marshaler
{
public:
    StructMarshaler(const Slice::StructPtr&);
    ~StructMarshaler();

    virtual bool marshal(zval*, IceInternal::BasicStream& TSRMLS_DC);
    virtual bool unmarshal(zval*, IceInternal::BasicStream& TSRMLS_DC);

private:
    Slice::StructPtr _type;
    zend_class_entry* _class;
    vector<MarshalerPtr> _members;
};

class EnumMarshaler : public Marshaler
{
public:
    EnumMarshaler(const Slice::EnumPtr&);
    ~EnumMarshaler();

    virtual bool marshal(zval*, IceInternal::BasicStream& TSRMLS_DC);
    virtual bool unmarshal(zval*, IceInternal::BasicStream& TSRMLS_DC);

private:
    zend_class_entry* _class;
    long _count;
};

class NativeDictionaryMarshaler : public Marshaler
{
public:
    NativeDictionaryMarshaler(const Slice::DictionaryPtr&);
    ~NativeDictionaryMarshaler();

    virtual bool marshal(zval*, IceInternal::BasicStream& TSRMLS_DC);
    virtual bool unmarshal(zval*, IceInternal::BasicStream& TSRMLS_DC);

private:
    Slice::DictionaryPtr _type;
    Slice::Builtin::Kind _keyKind;
    MarshalerPtr _keyMarshaler;
    MarshalerPtr _valueMarshaler;
};

class ExceptionMarshaler : public Marshaler
{
public:
    ExceptionMarshaler(const Slice::ExceptionPtr&);
    ~ExceptionMarshaler();

    virtual bool marshal(zval*, IceInternal::BasicStream& TSRMLS_DC);
    virtual bool unmarshal(zval*, IceInternal::BasicStream& TSRMLS_DC);

private:
    Slice::ExceptionPtr _ex;
    zend_class_entry* _class;
};

//
// Marshaler implementation.
//
Marshaler::Marshaler()
{
}

Marshaler::~Marshaler()
{
}

MarshalerPtr
Marshaler::createMarshaler(const Slice::TypePtr& type)
{
    Slice::BuiltinPtr builtin = Slice::BuiltinPtr::dynamicCast(type);
    if(builtin)
    {
        switch(builtin->kind())
        {
        case Slice::Builtin::KindByte:
        case Slice::Builtin::KindBool:
        case Slice::Builtin::KindShort:
        case Slice::Builtin::KindInt:
        case Slice::Builtin::KindLong:
        case Slice::Builtin::KindFloat:
        case Slice::Builtin::KindDouble:
        case Slice::Builtin::KindString:
            return new PrimitiveMarshaler(builtin);

        case Slice::Builtin::KindObject:
            // TODO
            return 0;

        case Slice::Builtin::KindObjectProxy:
            return new ProxyMarshaler(type);

        case Slice::Builtin::KindLocalObject:
            zend_error(E_ERROR, "unexpected local type");
            return 0;
        }
    }

    Slice::SequencePtr seq = Slice::SequencePtr::dynamicCast(type);
    if(seq)
    {
        return new SequenceMarshaler(seq);
    }

    Slice::ProxyPtr proxy = Slice::ProxyPtr::dynamicCast(type);
    if(proxy)
    {
        return new ProxyMarshaler(type);
    }

    Slice::StructPtr st = Slice::StructPtr::dynamicCast(type);
    if(st)
    {
        return new StructMarshaler(st);
    }

    Slice::EnumPtr en = Slice::EnumPtr::dynamicCast(type);
    if(en)
    {
        return new EnumMarshaler(en);
    }

    Slice::DictionaryPtr dict = Slice::DictionaryPtr::dynamicCast(type);
    if(dict)
    {
        if(Slice_isNativeKey(dict->keyType()))
        {
            return new NativeDictionaryMarshaler(dict);
        }
    }

#if 0
    ClassDeclPtr cl = ClassDeclPtr::dynamicCast(type);
    if(cl)
    {
    }
#endif

    return 0;
}

MarshalerPtr
Marshaler::createMemberMarshaler(const string& name, const Slice::TypePtr& type)
{
    MarshalerPtr result;
    MarshalerPtr m = createMarshaler(type);
    if(m)
    {
        result = new MemberMarshaler(name, m);
    }
    return result;
}

MarshalerPtr
Marshaler::createExceptionMarshaler(const Slice::ExceptionPtr& ex)
{
   return new ExceptionMarshaler(ex);
}

std::string
Marshaler::zendTypeToString(int type)
{
    string result;

    switch(type)
    {
    case IS_NULL:
        result = "null";
        break;

    case IS_LONG:
        result = "long";
        break;

    case IS_DOUBLE:
        result = "double";
        break;

    case IS_STRING:
        result = "string";
        break;

    case IS_ARRAY:
        result = "array";
        break;

    case IS_OBJECT:
        result = "object";
        break;

    case IS_BOOL:
        result = "bool";
        break;

    default:
        result = "unknown";
        break;
    }

    return result;
}

//
// PrimitiveMarshaler implementation.
//
PrimitiveMarshaler::PrimitiveMarshaler(const Slice::BuiltinPtr& type) :
    _type(type)
{
}

PrimitiveMarshaler::~PrimitiveMarshaler()
{
}

bool
PrimitiveMarshaler::marshal(zval* zv, IceInternal::BasicStream& os TSRMLS_DC)
{
    //
    // TODO: Use convert_to_XXX functions? For example, this would allow users to provide a stringified 
    // bool or number which is converted into the required type (if possible). This might result in
    // some unusual behavior, so this is not currently supported.
    //
    // See http://www.php.net/manual/en/zend.arguments.access.php
    //
    switch(_type->kind())
    {
    case Slice::Builtin::KindBool:
    {
        if(Z_TYPE_P(zv) != IS_BOOL)
        {
            string s = zendTypeToString(Z_TYPE_P(zv));
            zend_error(E_ERROR, "expected boolean value but received %s", s.c_str());
            return false;
        }
        os.write(Z_BVAL_P(zv) ? true : false);
        break;
    }
    case Slice::Builtin::KindByte:
    {
        if(Z_TYPE_P(zv) != IS_LONG)
        {
            string s = zendTypeToString(Z_TYPE_P(zv));
            zend_error(E_ERROR, "expected byte value but received %s", s.c_str());
            return false;
        }
        long val = Z_LVAL_P(zv);
        if(val < 0 || val > 255)
        {
            zend_error(E_ERROR, "value %ld is out of range for a byte", val);
            return false;
        }
        os.write(static_cast<Ice::Byte>(val));
        break;
    }
    case Slice::Builtin::KindShort:
    {
        if(Z_TYPE_P(zv) != IS_LONG)
        {
            string s = zendTypeToString(Z_TYPE_P(zv));
            zend_error(E_ERROR, "expected short value but received %s", s.c_str());
            return false;
        }
        long val = Z_LVAL_P(zv);
        if(val < SHRT_MIN || val > SHRT_MAX)
        {
            zend_error(E_ERROR, "value %ld is out of range for a short", val);
            return false;
        }
        os.write(static_cast<Ice::Short>(val));
        break;
    }
    case Slice::Builtin::KindInt:
    {
        if(Z_TYPE_P(zv) != IS_LONG)
        {
            string s = zendTypeToString(Z_TYPE_P(zv));
            zend_error(E_ERROR, "expected int value but received %s", s.c_str());
            return false;
        }
        long val = Z_LVAL_P(zv);
        if(val < INT_MIN || val > INT_MAX)
        {
            zend_error(E_ERROR, "value %ld is out of range for an int", val);
            return false;
        }
        os.write(static_cast<Ice::Int>(val));
        break;
    }
    case Slice::Builtin::KindLong:
    {
        //
        // The platform's 'long' type may not be 64 bits, so we also accept
        // a string argument for this type.
        //
        if(Z_TYPE_P(zv) != IS_LONG && Z_TYPE_P(zv) != IS_STRING)
        {
            string s = zendTypeToString(Z_TYPE_P(zv));
            zend_error(E_ERROR, "expected long value but received %s", s.c_str());
            return false;
        }
        Ice::Long val;
        if(Z_TYPE_P(zv) == IS_LONG)
        {
            val = Z_LVAL_P(zv);
        }
        else
        {
            string sval(Z_STRVAL_P(zv), Z_STRLEN_P(zv));
            string::size_type pos;
            if(!IceUtil::stringToInt64(sval, val, pos))
            {
                zend_error(E_ERROR, "invalid long value `%s'", Z_STRVAL_P(zv));
                return false;
            }
        }
        os.write(val);
        break;
    }
    case Slice::Builtin::KindFloat:
    {
        if(Z_TYPE_P(zv) != IS_DOUBLE)
        {
            string s = zendTypeToString(Z_TYPE_P(zv));
            zend_error(E_ERROR, "expected float value but received %s", s.c_str());
            return false;
        }
        double val = Z_DVAL_P(zv);
        os.write(static_cast<Ice::Float>(val));
        break;
    }
    case Slice::Builtin::KindDouble:
    {
        if(Z_TYPE_P(zv) != IS_DOUBLE)
        {
            string s = zendTypeToString(Z_TYPE_P(zv));
            zend_error(E_ERROR, "expected double value but received %s", s.c_str());
            return false;
        }
        double val = Z_DVAL_P(zv);
        os.write(val);
        break;
    }
    case Slice::Builtin::KindString:
    {
        if(Z_TYPE_P(zv) == IS_STRING)
        {
            string val(Z_STRVAL_P(zv), Z_STRLEN_P(zv));
            os.write(val);
        }
        else if(Z_TYPE_P(zv) == IS_NULL)
        {
            os.write("");
        }
        else
        {
            string s = zendTypeToString(Z_TYPE_P(zv));
            zend_error(E_ERROR, "expected string value but received %s", s.c_str());
            return false;
        }
        break;
    }

    case Slice::Builtin::KindObject:
    case Slice::Builtin::KindObjectProxy:
    case Slice::Builtin::KindLocalObject:
        assert(false);
    }
    return true;
}

bool
PrimitiveMarshaler::unmarshal(zval* zv, IceInternal::BasicStream& is TSRMLS_DC)
{
    switch(_type->kind())
    {
    case Slice::Builtin::KindBool:
    {
        bool val;
        is.read(val);
        ZVAL_BOOL(zv, val ? 1 : 0);
        break;
    }
    case Slice::Builtin::KindByte:
    {
        Ice::Byte val;
        is.read(val);
        ZVAL_LONG(zv, val);
        break;
    }
    case Slice::Builtin::KindShort:
    {
        Ice::Short val;
        is.read(val);
        ZVAL_LONG(zv, val);
        break;
    }
    case Slice::Builtin::KindInt:
    {
        Ice::Int val;
        is.read(val);
        ZVAL_LONG(zv, val);
        break;
    }
    case Slice::Builtin::KindLong:
    {
        Ice::Long val;
        is.read(val);

        //
        // The platform's 'long' type may not be 64 bits, so we store 64-bit
        // values as a string.
        //
        if(sizeof(Ice::Long) > sizeof(long) && (val < LONG_MIN || val > LONG_MAX))
        {
            char buf[64];
#ifdef WIN32
            sprintf(buf, "%I64d", val);
#else
            sprintf(buf, "%lld", val);
#endif
            ZVAL_STRING(zv, buf, 1);
        }
        else
        {
            ZVAL_LONG(zv, val);
        }
        break;
    }
    case Slice::Builtin::KindFloat:
    {
        Ice::Float val;
        is.read(val);
        ZVAL_DOUBLE(zv, val);
        break;
    }
    case Slice::Builtin::KindDouble:
    {
        Ice::Double val;
        is.read(val);
        ZVAL_DOUBLE(zv, val);
        break;
    }
    case Slice::Builtin::KindString:
    {
        string val;
        is.read(val);
        ZVAL_STRINGL(zv, const_cast<char*>(val.c_str()), val.length(), 1);
        break;
    }

    case Slice::Builtin::KindObject:
    case Slice::Builtin::KindObjectProxy:
    case Slice::Builtin::KindLocalObject:
        assert(false);
    }

#if 0
    switch(builtin->kind())
    {
        case Builtin::KindBool:
            result = "b";
            break;
        case Builtin::KindByte:
        case Builtin::KindShort:
        case Builtin::KindInt:
        case Builtin::KindLong:
            result = "l";
            break;
        case Builtin::KindFloat:
        case Builtin::KindDouble:
            result = "d";
            break;
        case Builtin::KindString:
            result = "s";
            break;

        case Builtin::KindObject:
        case Builtin::KindObjectProxy:
        case Builtin::KindLocalObject:
            assert(false);
    }
#endif
    return true;
}

//
// SequenceMarshaler implementation.
//
SequenceMarshaler::SequenceMarshaler(const Slice::SequencePtr& type) :
    _type(type)
{
    _elementMarshaler = createMarshaler(type->type());
}

SequenceMarshaler::~SequenceMarshaler()
{
}

bool
SequenceMarshaler::marshal(zval* zv, IceInternal::BasicStream& os TSRMLS_DC)
{
    if(Z_TYPE_P(zv) != IS_ARRAY)
    {
        string s = zendTypeToString(Z_TYPE_P(zv));
        zend_error(E_ERROR, "expected array value but received %s", s.c_str());
        return false;
    }

    HashTable* arr = Z_ARRVAL_P(zv);
    HashPosition pos;
    zval** val;

    os.writeSize(zend_hash_num_elements(arr));

    zend_hash_internal_pointer_reset_ex(arr, &pos);
    while(zend_hash_get_current_data_ex(arr, (void**)&val, &pos) != FAILURE)
    {
        if(!_elementMarshaler->marshal(*val, os TSRMLS_CC))
        {
            return false;
        }
        zend_hash_move_forward_ex(arr, &pos);
    }

    return true;
}

bool
SequenceMarshaler::unmarshal(zval* zv, IceInternal::BasicStream& is TSRMLS_DC)
{
    array_init(zv);

    Ice::Int sz;
    is.readSize(sz);

    // TODO: Optimize for certain sequence types (e.g., bytes)?

    for(Ice::Int i = 0; i < sz; ++i)
    {
        zval* val;
        MAKE_STD_ZVAL(val);
        if(!_elementMarshaler->unmarshal(val, is TSRMLS_CC))
        {
            return false;
        }
        add_index_zval(zv, i, val);
    }

    return true;
}

//
// ProxyMarshaler implementation.
//
ProxyMarshaler::ProxyMarshaler(const Slice::TypePtr& type) :
    _type(type)
{
}

ProxyMarshaler::~ProxyMarshaler()
{
}

bool
ProxyMarshaler::marshal(zval* zv, IceInternal::BasicStream& os TSRMLS_DC)
{
    if(Z_TYPE_P(zv) != IS_OBJECT && Z_TYPE_P(zv) != IS_NULL)
    {
        string s = zendTypeToString(Z_TYPE_P(zv));
        zend_error(E_ERROR, "expected proxy value but received %s", s.c_str());
        return false;
    }

    Ice::ObjectPrx proxy;
    if(!ZVAL_IS_NULL(zv))
    {
        if(!Ice_ObjectPrx_fetch(zv, proxy TSRMLS_CC))
        {
            return false;
        }
    }
    os.write(proxy);

    return true;
}

bool
ProxyMarshaler::unmarshal(zval* zv, IceInternal::BasicStream& is TSRMLS_DC)
{
    Ice::ObjectPrx proxy;
    is.read(proxy);

    if(!proxy)
    {
        ZVAL_NULL(zv);
        return true;
    }

    //
    // If _type is not a primitive proxy (i.e., Builtin::KindObjectProxy), then we
    // want to associate our class with the proxy so that it is considered to be
    // "narrowed".
    //
    Slice::ClassDeclPtr decl;
    Slice::ProxyPtr type = Slice::ProxyPtr::dynamicCast(_type);
    if(type)
    {
        decl = type->_class();
    }

    if(!Ice_ObjectPrx_create(zv, proxy, decl TSRMLS_CC))
    {
        return false;
    }

    return true;
}

//
// MemberMarshaler implementation.
//
MemberMarshaler::MemberMarshaler(const string& name, const MarshalerPtr& marshaler) :
    _name(name), _marshaler(marshaler)
{
}

MemberMarshaler::~MemberMarshaler()
{
}

bool
MemberMarshaler::marshal(zval* zv, IceInternal::BasicStream& os TSRMLS_DC)
{
    zval** val;
    if(zend_hash_find(Z_OBJPROP_P(zv), const_cast<char*>(_name.c_str()), _name.length() + 1, (void**)&val) == FAILURE)
    {
        zend_error(E_ERROR, "unable to find member `%s'", _name.c_str());
        return false;
    }

    return _marshaler->marshal(*val, os TSRMLS_CC);;
}

bool
MemberMarshaler::unmarshal(zval* zv, IceInternal::BasicStream& is TSRMLS_DC)
{
    zval* val;
    MAKE_STD_ZVAL(val);

    if(!_marshaler->unmarshal(val, is TSRMLS_CC))
    {
        return false;
    }

    if(add_property_zval(zv, const_cast<char*>(_name.c_str()), val) == FAILURE)
    {
        zend_error(E_ERROR, "unable to set member `%s'", _name.c_str());
        return false;
    }

    return true;
}

//
// StructMarshaler implementation.
//
StructMarshaler::StructMarshaler(const Slice::StructPtr& type) :
    _type(type)
{
    _class = Slice_getClass(type->scoped());
    assert(_class);

    Slice::DataMemberList members = type->dataMembers();
    for(Slice::DataMemberList::iterator p = members.begin(); p != members.end(); ++p)
    {
        string name = ice_lowerCase((*p)->name());
        MarshalerPtr marshaler = createMemberMarshaler(name, (*p)->type());
        assert(marshaler);
        _members.push_back(marshaler);
    }
}

StructMarshaler::~StructMarshaler()
{
}

bool
StructMarshaler::marshal(zval* zv, IceInternal::BasicStream& os TSRMLS_DC)
{
    if(Z_TYPE_P(zv) != IS_OBJECT)
    {
        string s = zendTypeToString(Z_TYPE_P(zv));
        zend_error(E_ERROR, "expected struct value of type %s but received %s", _class->name, s.c_str());
        return false;
    }

    //
    // Compare class entries.
    //
    zend_object* obj = static_cast<zend_object*>(zend_object_store_get_object(zv TSRMLS_CC));
    if(!obj)
    {
        zend_error(E_ERROR, "object not found in object store");
        return false;
    }

    if(obj->ce != _class)
    {
        zend_error(E_ERROR, "expected struct value of type %s but received %s", _class->name, obj->ce->name);
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
StructMarshaler::unmarshal(zval* zv, IceInternal::BasicStream& is TSRMLS_DC)
{
    if(object_init_ex(zv, _class) != SUCCESS)
    {
        zend_error(E_ERROR, "unable to initialize object of type %s", _class->name);
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

//
// EnumMarshaler implementation.
//
EnumMarshaler::EnumMarshaler(const Slice::EnumPtr& type)
{
    _class = Slice_getClass(type->scoped());
    _count = static_cast<long>(type->getEnumerators().size());
}

EnumMarshaler::~EnumMarshaler()
{
}

bool
EnumMarshaler::marshal(zval* zv, IceInternal::BasicStream& os TSRMLS_DC)
{
    if(Z_TYPE_P(zv) != IS_LONG)
    {
        string s = zendTypeToString(Z_TYPE_P(zv));
        zend_error(E_ERROR, "expected long value for enum %s but received %s", _class->name, s.c_str());
        return false;
    }

    //
    // Validate value.
    //
    long val = Z_LVAL_P(zv);
    if(val < 0 || val >= _count)
    {
        zend_error(E_ERROR, "value %ld is out of range for enum %s", val, _class->name);
        return false;
    }

    if(_count <= 127)
    {
        os.write(static_cast<Ice::Byte>(val));
    }
    else if(_count <= 32767)
    {
        os.write(static_cast<Ice::Short>(val));
    }
    else
    {
        os.write(static_cast<Ice::Int>(val));
    }

    return true;
}

bool
EnumMarshaler::unmarshal(zval* zv, IceInternal::BasicStream& is TSRMLS_DC)
{
    if(_count <= 127)
    {
        Ice::Byte val;
        is.read(val);
        ZVAL_LONG(zv, val);
    }
    else if(_count <= 32767)
    {
        Ice::Short val;
        is.read(val);
        ZVAL_LONG(zv, val);
    }
    else
    {
        Ice::Int val;
        is.read(val);
        ZVAL_LONG(zv, val);
    }

    return true;
}

//
// NativeDictionaryMarshaler implementation.
//
NativeDictionaryMarshaler::NativeDictionaryMarshaler(const Slice::DictionaryPtr& type) :
    _type(type)
{
    Slice::TypePtr keyType = type->keyType();
    Slice::BuiltinPtr b = Slice::BuiltinPtr::dynamicCast(keyType);
    assert(b);
    _keyKind = b->kind();
    _keyMarshaler = createMarshaler(keyType);
    _valueMarshaler = createMarshaler(type->valueType());
}

NativeDictionaryMarshaler::~NativeDictionaryMarshaler()
{
}

bool
NativeDictionaryMarshaler::marshal(zval* zv, IceInternal::BasicStream& os TSRMLS_DC)
{
    if(Z_TYPE_P(zv) != IS_ARRAY)
    {
        string s = zendTypeToString(Z_TYPE_P(zv));
        zend_error(E_ERROR, "expected array value but received %s", s.c_str());
        return false;
    }

    HashTable* arr = Z_ARRVAL_P(zv);
    HashPosition pos;
    zval** val;

    os.writeSize(zend_hash_num_elements(arr));

    zend_hash_internal_pointer_reset_ex(arr, &pos);
    while(zend_hash_get_current_data_ex(arr, (void**)&val, &pos) != FAILURE)
    {
        //
        // Get the key (which can be a long or a string).
        //
        char* keyStr;
        uint keyLen;
        ulong keyNum;
        int keyType = zend_hash_get_current_key_ex(arr, &keyStr, &keyLen, &keyNum, 0, &pos);

        //
        // Store the key in a zval, so that we can reuse the PrimitiveMarshaler logic.
        //
        zval zkey;
        if(keyType == HASH_KEY_IS_LONG)
        {
            ZVAL_LONG(&zkey, keyNum);
        }
        else
        {
            ZVAL_STRINGL(&zkey, keyStr, keyLen, 1);
        }

        //
        // Convert the zval to the key type required by Slice, if necessary.
        //
        switch(_keyKind)
        {
        case Slice::Builtin::KindBool:
        {
            convert_to_boolean(&zkey);
            break;
        }

        case Slice::Builtin::KindByte:
        case Slice::Builtin::KindShort:
        case Slice::Builtin::KindInt:
        case Slice::Builtin::KindLong:
        {
            if(keyType == HASH_KEY_IS_STRING)
            {
                convert_to_long(&zkey);
            }
            break;
        }

        case Slice::Builtin::KindString:
        {
            if(keyType == HASH_KEY_IS_LONG)
            {
                convert_to_string(&zkey);
            }
            break;
        }

        case Slice::Builtin::KindFloat:
        case Slice::Builtin::KindDouble:
        case Slice::Builtin::KindObject:
        case Slice::Builtin::KindObjectProxy:
        case Slice::Builtin::KindLocalObject:
            assert(false);
        }

        //
        // Marshal the key.
        //
        if(!_keyMarshaler->marshal(&zkey, os TSRMLS_CC))
        {
            zval_dtor(&zkey);
            return false;
        }

        zval_dtor(&zkey);

        //
        // Marshal the value.
        //
        if(!_valueMarshaler->marshal(*val, os TSRMLS_CC))
        {
            return false;
        }

        zend_hash_move_forward_ex(arr, &pos);
    }

    return true;
}

bool
NativeDictionaryMarshaler::unmarshal(zval* zv, IceInternal::BasicStream& is TSRMLS_DC)
{
    array_init(zv);

    Ice::Int sz;
    is.readSize(sz);

    for(Ice::Int i = 0; i < sz; ++i)
    {
        zval* key;
        zval* val;
        MAKE_STD_ZVAL(key);
        MAKE_STD_ZVAL(val);

        if(!_keyMarshaler->unmarshal(key, is TSRMLS_CC))
        {
            return false;
        }
        if(!_valueMarshaler->unmarshal(val, is TSRMLS_CC))
        {
            return false;
        }

        switch(Z_TYPE_P(key))
        {
        case IS_LONG:
            add_index_zval(zv, Z_LVAL_P(key), val);
            break;
        case IS_BOOL:
            add_index_zval(zv, Z_BVAL_P(key) ? 1 : 0, val);
            break;
        case IS_STRING:
            add_assoc_zval_ex(zv, Z_STRVAL_P(key), Z_STRLEN_P(key) + 1, val);
            break;
        default:
            assert(false);
            return false;
        }
    }

    return true;
}

ExceptionMarshaler::ExceptionMarshaler(const Slice::ExceptionPtr& ex) :
    _ex(ex)
{
    _class = Slice_getClass(ex->scoped());
    assert(_class);
}

ExceptionMarshaler::~ExceptionMarshaler()
{
}

bool
ExceptionMarshaler::marshal(zval*, IceInternal::BasicStream& TSRMLS_DC)
{
    //
    // We never need to marshal an exception.
    //
    zend_error(E_ERROR, "exception marshaling is not supported");
    return false;
}

bool
ExceptionMarshaler::unmarshal(zval* zv, IceInternal::BasicStream& is TSRMLS_DC)
{
    if(object_init_ex(zv, _class) != SUCCESS)
    {
        zend_error(E_ERROR, "unable to initialize exception %s", _class->name);
        return false;
    }

    //
    // NOTE: The type id for the first slice has already been read.
    //

    Slice::ExceptionPtr ex = _ex;
    while(ex)
    {
        Slice::DataMemberList members = ex->dataMembers();
        is.startReadSlice();
        for(Slice::DataMemberList::iterator p = members.begin(); p != members.end(); ++p)
        {
            MarshalerPtr member = createMemberMarshaler((*p)->name(), (*p)->type());
            if(!member->unmarshal(zv, is TSRMLS_CC))
            {
                return false;
            }
        }
        is.endReadSlice();
        ex = ex->base();
        if(ex)
        {
            string id;
            is.read(id);
        }
    }

    return true;
}
