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

#include <IceUtil/InputUtil.h>

using namespace std;

class PrimitiveMarshaler : public Marshaler
{
public:
    PrimitiveMarshaler(const Slice::BuiltinPtr&);
    ~PrimitiveMarshaler();

    virtual std::string getArgType() const;
    virtual bool marshal(zval*, IceInternal::BasicStream&);
    virtual bool unmarshal(zval*, IceInternal::BasicStream&);

private:
    Slice::BuiltinPtr _type;
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
        case Slice::Builtin::KindObjectProxy:
            // TODO
            return 0;

        case Slice::Builtin::KindLocalObject:
            php_error(E_ERROR, "unexpected local type");
            return 0;
        }
    }

#if 0
    ClassDeclPtr cl = ClassDeclPtr::dynamicCast(type);
    if(cl)
    {
    }

    ProxyPtr proxy = ProxyPtr::dynamicCast(type);
    if(proxy)
    {
    }

    DictionaryPtr dict = DictionaryPtr::dynamicCast(type);
    if(dict)
    {
    }

    SequencePtr seq = SequencePtr::dynamicCast(type);
    if(seq)
    {
    }
#endif

    return 0;
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
PrimitiveMarshaler::marshal(zval* zv, IceInternal::BasicStream& os)
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
            php_error(E_ERROR, "expected boolean value");
            return false;
        }
        os.write(Z_BVAL_P(zv) ? true : false);
        break;
    }
    case Slice::Builtin::KindByte:
    {
        if(Z_TYPE_P(zv) != IS_LONG)
        {
            php_error(E_ERROR, "expected byte value");
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
            php_error(E_ERROR, "expected short value");
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
            php_error(E_ERROR, "expected int value");
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
            php_error(E_ERROR, "expected long value");
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
            php_error(E_ERROR, "expected float value");
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
            php_error(E_ERROR, "expected double value");
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
            php_error(E_ERROR, "expected string value");
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
PrimitiveMarshaler::unmarshal(zval* zv, IceInternal::BasicStream& is)
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
