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
#include "objectprx.h"

#include <IceUtil/InputUtil.h>

using namespace std;

class PrimitiveMarshaler : public Marshaler
{
public:
    PrimitiveMarshaler(const Slice::BuiltinPtr&);
    ~PrimitiveMarshaler();

    virtual std::string getArgType() const;
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

    virtual std::string getArgType() const;
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

    virtual std::string getArgType() const;
    virtual bool marshal(zval*, IceInternal::BasicStream& TSRMLS_DC);
    virtual bool unmarshal(zval*, IceInternal::BasicStream& TSRMLS_DC);

private:
    Slice::TypePtr _type;
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
            php_error(E_ERROR, "unexpected local type");
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

#if 0
    ClassDeclPtr cl = ClassDeclPtr::dynamicCast(type);
    if(cl)
    {
    }

    DictionaryPtr dict = DictionaryPtr::dynamicCast(type);
    if(dict)
    {
    }
#endif

    return 0;
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

string
PrimitiveMarshaler::getArgType() const
{
    string result;

    switch(_type->kind())
    {
    case Slice::Builtin::KindBool:
        result = "b";
        break;
    case Slice::Builtin::KindByte:
    case Slice::Builtin::KindShort:
    case Slice::Builtin::KindInt:
    case Slice::Builtin::KindLong:
        result = "l";
        break;
    case Slice::Builtin::KindFloat:
    case Slice::Builtin::KindDouble:
        result = "d";
        break;
    case Slice::Builtin::KindString:
        result = "s";
        break;
    case Slice::Builtin::KindObject:
    case Slice::Builtin::KindObjectProxy:
    case Slice::Builtin::KindLocalObject:
        assert(false);
    }

    return result;
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
            php_error(E_ERROR, "expected boolean value but received %s", s.c_str());
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
            php_error(E_ERROR, "expected byte value but received %s", s.c_str());
            return false;
        }
        long val = Z_LVAL_P(zv);
        if(val < 0 || val > 255)
        {
            php_error(E_ERROR, "value %l is out of range for a byte", val);
            return false;
        }
        os.write((Ice::Byte)val);
        break;
    }
    case Slice::Builtin::KindShort:
    {
        if(Z_TYPE_P(zv) != IS_LONG)
        {
            string s = zendTypeToString(Z_TYPE_P(zv));
            php_error(E_ERROR, "expected short value but received %s", s.c_str());
            return false;
        }
        long val = Z_LVAL_P(zv);
        if(val < SHRT_MIN || val > SHRT_MAX)
        {
            php_error(E_ERROR, "value %l is out of range for a short", val);
            return false;
        }
        os.write((Ice::Short)val);
        break;
    }
    case Slice::Builtin::KindInt:
    {
        if(Z_TYPE_P(zv) != IS_LONG)
        {
            string s = zendTypeToString(Z_TYPE_P(zv));
            php_error(E_ERROR, "expected int value but received %s", s.c_str());
            return false;
        }
        long val = Z_LVAL_P(zv);
        if(val < INT_MIN || val > INT_MAX)
        {
            php_error(E_ERROR, "value %l is out of range for an int", val);
            return false;
        }
        os.write((Ice::Int)val);
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
            php_error(E_ERROR, "expected long value but received %s", s.c_str());
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
                php_error(E_ERROR, "invalid long value `%s'", Z_STRVAL_P(zv));
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
            php_error(E_ERROR, "expected float value but received %s", s.c_str());
            return false;
        }
        double val = Z_DVAL_P(zv);
        os.write((Ice::Float)val);
        break;
    }
    case Slice::Builtin::KindDouble:
    {
        if(Z_TYPE_P(zv) != IS_DOUBLE)
        {
            string s = zendTypeToString(Z_TYPE_P(zv));
            php_error(E_ERROR, "expected double value but received %s", s.c_str());
            return false;
        }
        double val = Z_DVAL_P(zv);
        os.write(val);
        break;
    }
    case Slice::Builtin::KindString:
    {
        if(Z_TYPE_P(zv) != IS_STRING)
        {
            string s = zendTypeToString(Z_TYPE_P(zv));
            php_error(E_ERROR, "expected string value but received %s", s.c_str());
            return false;
        }
        string val(Z_STRVAL_P(zv), Z_STRLEN_P(zv));
        os.write(val);
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

string
SequenceMarshaler::getArgType() const
{
    return "a";
}

bool
SequenceMarshaler::marshal(zval* zv, IceInternal::BasicStream& os TSRMLS_DC)
{
    if(Z_TYPE_P(zv) != IS_ARRAY)
    {
        string s = zendTypeToString(Z_TYPE_P(zv));
        php_error(E_ERROR, "expected array value but received %s", s.c_str());
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

string
ProxyMarshaler::getArgType() const
{
    return "o!";
}

bool
ProxyMarshaler::marshal(zval* zv, IceInternal::BasicStream& os TSRMLS_DC)
{
    if(Z_TYPE_P(zv) != IS_OBJECT && Z_TYPE_P(zv) != IS_NULL)
    {
        string s = zendTypeToString(Z_TYPE_P(zv));
        php_error(E_ERROR, "expected proxy value but received %s", s.c_str());
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
