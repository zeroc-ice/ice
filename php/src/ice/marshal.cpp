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

ZEND_EXTERN_MODULE_GLOBALS(ice)

//
// The marshaling implementation is fairly straightforward. The factory methods in the
// Marshaler base class examine the given Slice type and create a Marshaler subclass
// that is responsible for marshaling that type. Some caching is done for complex types
// such as struct and class; the cached Marshaler instance is stored as a member of the
// ice_class_entry struct. (Cached instances are destroyed by Slice_destroyClasses.)
//
// The implementation of Ice object marshaling is more complex. In order to interface
// with the C++ BasicStream class, we need to supply Ice::Object instances, and we must
// be able to properly handle object graphs and cycles. The solution is to wrap each
// PHP object with a temporary Ice::Object, and maintain a table that associates each PHP
// object to its wrapper, so that graphs work correctly.
//
// The ObjectMarshaler class doesn't actually marshal object instances. Rather, it
// represents the top-level marshaler for a particular formal type (i.e., the declared
// type of a data member or operation parameter). During marshaling, the ObjectMarshaler
// validates the type of the object to ensure it is compatible with the formal type,
// and then obtains or creates an ObjectWrapper instance for the object. Since each PHP
// object is represented by an unsigned integer handle, looking up the wrapper is simple.
//
// Once the wrapper is obtained, the marshaler gives it to the stream. Eventually, the
// stream will invoke __write on the wrapper, at which point the data members are
// marshaled. For efficiency, each "slice" of the object's state is marshaled by an
// ObjectSliceMarshaler, which is cached in the class entry for future reuse.
//
// Note that a graph of PHP objects does not result in an equivalent graph of wrappers.
// Links between objects exist only in the PHP object representation. Furthermore, the
// lifetime of the wrappers is bound to the operation, not to the PHP objects. Wrappers
// exist only as a bridge to the C++ marshaling facility.
//
// The table which associates a PHP object's handle to its wrapper is stored in a member
// of our "module globals" (see php_ice.h), which are similar to thread-specific storage.
// The object map is not created until it is first needed, and is destroyed after each
// operation.
//

//
// Marshaler subclass definitions.
//
class PrimitiveMarshaler : public Marshaler
{
public:
    PrimitiveMarshaler(const Slice::BuiltinPtr&);

    virtual bool marshal(zval*, IceInternal::BasicStream& TSRMLS_DC);
    virtual bool unmarshal(zval*, IceInternal::BasicStream& TSRMLS_DC);

    virtual void destroy();

private:
    Slice::BuiltinPtr _type;
};

class SequenceMarshaler : public Marshaler
{
public:
    SequenceMarshaler(const Slice::SequencePtr& TSRMLS_DC);

    virtual bool marshal(zval*, IceInternal::BasicStream& TSRMLS_DC);
    virtual bool unmarshal(zval*, IceInternal::BasicStream& TSRMLS_DC);

    virtual void destroy();

private:
    Slice::SequencePtr _type;
    MarshalerPtr _elementMarshaler;
};

class ProxyMarshaler : public Marshaler
{
public:
    ProxyMarshaler(const Slice::TypePtr&);

    virtual bool marshal(zval*, IceInternal::BasicStream& TSRMLS_DC);
    virtual bool unmarshal(zval*, IceInternal::BasicStream& TSRMLS_DC);

    virtual void destroy();

private:
    Slice::TypePtr _type;
};

class MemberMarshaler : public Marshaler
{
public:
    MemberMarshaler(const string&, const MarshalerPtr&);

    virtual bool marshal(zval*, IceInternal::BasicStream& TSRMLS_DC);
    virtual bool unmarshal(zval*, IceInternal::BasicStream& TSRMLS_DC);

    virtual void destroy();

private:
    string _name;
    MarshalerPtr _marshaler;
};

class StructMarshaler : public Marshaler
{
public:
    StructMarshaler(const Slice::StructPtr& TSRMLS_DC);

    virtual bool marshal(zval*, IceInternal::BasicStream& TSRMLS_DC);
    virtual bool unmarshal(zval*, IceInternal::BasicStream& TSRMLS_DC);

    virtual void destroy();

private:
    Slice::StructPtr _type;
    zend_class_entry* _class;
    vector<MarshalerPtr> _members;
};

class EnumMarshaler : public Marshaler
{
public:
    EnumMarshaler(const Slice::EnumPtr& TSRMLS_DC);

    virtual bool marshal(zval*, IceInternal::BasicStream& TSRMLS_DC);
    virtual bool unmarshal(zval*, IceInternal::BasicStream& TSRMLS_DC);

    virtual void destroy();

private:
    zend_class_entry* _class;
    long _count;
};

class NativeDictionaryMarshaler : public Marshaler
{
public:
    NativeDictionaryMarshaler(const Slice::TypePtr&, const Slice::TypePtr& TSRMLS_DC);

    virtual bool marshal(zval*, IceInternal::BasicStream& TSRMLS_DC);
    virtual bool unmarshal(zval*, IceInternal::BasicStream& TSRMLS_DC);

    virtual void destroy();

private:
    Slice::Builtin::Kind _keyKind;
    MarshalerPtr _keyMarshaler;
    MarshalerPtr _valueMarshaler;
};

class ExceptionMarshaler : public Marshaler
{
public:
    ExceptionMarshaler(const Slice::ExceptionPtr& TSRMLS_DC);

    virtual bool marshal(zval*, IceInternal::BasicStream& TSRMLS_DC);
    virtual bool unmarshal(zval*, IceInternal::BasicStream& TSRMLS_DC);

    virtual void destroy();

private:
    Slice::ExceptionPtr _ex;
    zend_class_entry* _class;
};

//
// Special marshaler just for the Ice::Object slice.
//
class IceObjectSliceMarshaler : public Marshaler
{
public:
    IceObjectSliceMarshaler(TSRMLS_D);

    virtual bool marshal(zval*, IceInternal::BasicStream& TSRMLS_DC);
    virtual bool unmarshal(zval*, IceInternal::BasicStream& TSRMLS_DC);

    virtual void destroy();

private:
    MarshalerPtr _facetMapMarshaler;
};

class ObjectSliceMarshaler : public Marshaler
{
public:
    ObjectSliceMarshaler(const string&, const Slice::DataMemberList& TSRMLS_DC);

    virtual bool marshal(zval*, IceInternal::BasicStream& TSRMLS_DC);
    virtual bool unmarshal(zval*, IceInternal::BasicStream& TSRMLS_DC);

    virtual void destroy();

private:
    string _scoped;
    vector<MarshalerPtr> _members;
};

class ObjectWrapper : public Ice::Object
{
public:
    ObjectWrapper(zval*, zend_object* TSRMLS_DC);

    virtual void __write(::IceInternal::BasicStream*) const;
    virtual void __read(::IceInternal::BasicStream*, bool = true);

    void setValue(zend_class_entry*, zval*);

private:
    zval* _value;
    zend_object* _obj;
#ifdef ZTS
    TSRMLS_D;
#endif
};
typedef IceUtil::Handle<ObjectWrapper> ObjectWrapperPtr;

class ObjectMarshaler : public Marshaler
{
public:
    ObjectMarshaler(const Slice::ClassDefPtr& TSRMLS_DC);

    virtual bool marshal(zval*, IceInternal::BasicStream& TSRMLS_DC);
    virtual bool unmarshal(zval*, IceInternal::BasicStream& TSRMLS_DC);

    virtual void destroy();

private:
    zend_class_entry* _class; // The static class type.
};

//
// PHPObjectFactory is an implementation of Ice::ObjectFactory that creates PHP objects.
// A single instance can be used for all types.
//
class PHPObjectFactory : public Ice::ObjectFactory
{
public:
    PHPObjectFactory(TypeMap& TSRMLS_DC);

    virtual Ice::ObjectPtr create(const string&);
    virtual void destroy();

private:
    TypeMap& _typeMap;
#ifdef ZTS
    TSRMLS_D;
#endif
};

//
// FactoryVisitor descends the Slice parse tree and registers an object factory for each
// non-abstract class.
//
class FactoryVisitor : public Slice::ParserVisitor
{
public:
    FactoryVisitor(const Ice::CommunicatorPtr&, TypeMap& TSRMLS_DC);

    virtual bool visitClassDefStart(const Slice::ClassDefPtr&);

private:
    Ice::CommunicatorPtr _communicator;
    Ice::ObjectFactoryPtr _factory;
};

//
// Typedef for the "global" object map.
//
typedef map<unsigned int, Ice::ObjectPtr> ObjectMap;

//
// Types for the "global" patch list.
//
struct PatchInfo
{
    zend_class_entry* ce; // The formal type
    zval* zv;             // The destination zval
};
typedef vector<PatchInfo*> PatchList;

void
Marshal_preOperation(TSRMLS_D)
{
    ICE_G(objectMap) = 0; // Lazy initialization - see ObjectWrapper::__write
    ICE_G(patchList) = 0; // Lazy initialization - see ObjectMarshaler::unmarshal
}

void
Marshal_postOperation(TSRMLS_D)
{
    if(ICE_G(objectMap))
    {
        delete static_cast<ObjectMap*>(ICE_G(objectMap));
        ICE_G(objectMap) = 0;
    }
    if(ICE_G(patchList))
    {
        PatchList* pl = static_cast<PatchList*>(ICE_G(patchList));
        for(PatchList::iterator p = pl->begin(); p != pl->end(); ++p)
        {
            delete (*p);
        }
        delete pl;
        ICE_G(patchList) = 0;
    }
}

void
Marshal_registerFactories(const Ice::CommunicatorPtr& communicator TSRMLS_DC)
{
    Slice::UnitPtr unit = Slice_getUnit();
    TypeMap* typeMap = static_cast<TypeMap*>(ICE_G(typeMap));
    FactoryVisitor visitor(communicator, *typeMap TSRMLS_CC);
    unit->visit(&visitor);
}

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
Marshaler::createMarshaler(const Slice::TypePtr& type TSRMLS_DC)
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
            return new ObjectMarshaler(0 TSRMLS_CC);

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
        return new SequenceMarshaler(seq TSRMLS_CC);
    }

    Slice::ProxyPtr proxy = Slice::ProxyPtr::dynamicCast(type);
    if(proxy)
    {
        return new ProxyMarshaler(type);
    }

    Slice::StructPtr st = Slice::StructPtr::dynamicCast(type);
    if(st)
    {
        //
        // Check to see if a marshaler is cached in the class entry. If not, create
        // one and cache it in the class entry.
        //
        TypeMap* typeMap = static_cast<TypeMap*>(ICE_G(typeMap));
        TypeMap::iterator p = typeMap->find(st->scoped());
        assert(p != typeMap->end());
        ice_class_entry* ce = (ice_class_entry*)p->second;
        if(ce->marshaler)
        {
            return *(static_cast<MarshalerPtr*>(ce->marshaler));
        }
        else
        {
            MarshalerPtr result = new StructMarshaler(st TSRMLS_CC);
            ce->marshaler = new MarshalerPtr(result);
            return result;
        }
    }

    Slice::EnumPtr en = Slice::EnumPtr::dynamicCast(type);
    if(en)
    {
        return new EnumMarshaler(en TSRMLS_CC);
    }

    Slice::DictionaryPtr dict = Slice::DictionaryPtr::dynamicCast(type);
    if(dict)
    {
        if(Slice_isNativeKey(dict->keyType()))
        {
            return new NativeDictionaryMarshaler(dict->keyType(), dict->valueType() TSRMLS_CC);
        }
    }

    Slice::ClassDeclPtr cl = Slice::ClassDeclPtr::dynamicCast(type);
    if(cl)
    {
        //
        // Don't cache ObjectMarshaler in the class entry - we cache ObjectSliceMarshaler instead.
        //
        Slice::ClassDefPtr def = cl->definition();
        if(!def)
        {
            string scoped = cl->scoped();
            zend_error(E_ERROR, "cannot use Slice %s %s because it has not been defined",
                       cl->isInterface() ? "interface" : "class", scoped.c_str());
            return 0;
        }
        return new ObjectMarshaler(def TSRMLS_CC);
    }

    return 0;
}

MarshalerPtr
Marshaler::createMemberMarshaler(const string& name, const Slice::TypePtr& type TSRMLS_DC)
{
    MarshalerPtr result;
    MarshalerPtr m = createMarshaler(type TSRMLS_CC);
    if(m)
    {
        result = new MemberMarshaler(name, m);
    }
    return result;
}

MarshalerPtr
Marshaler::createExceptionMarshaler(const Slice::ExceptionPtr& ex TSRMLS_DC)
{
   return new ExceptionMarshaler(ex TSRMLS_CC);
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

    return true;
}

void
PrimitiveMarshaler::destroy()
{
}

//
// SequenceMarshaler implementation.
//
SequenceMarshaler::SequenceMarshaler(const Slice::SequencePtr& type TSRMLS_DC) :
    _type(type)
{
    _elementMarshaler = createMarshaler(type->type() TSRMLS_CC);
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

void
SequenceMarshaler::destroy()
{
    _elementMarshaler->destroy();
    _elementMarshaler = 0;
}

//
// ProxyMarshaler implementation.
//
ProxyMarshaler::ProxyMarshaler(const Slice::TypePtr& type) :
    _type(type)
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

void
ProxyMarshaler::destroy()
{
}

//
// MemberMarshaler implementation.
//
MemberMarshaler::MemberMarshaler(const string& name, const MarshalerPtr& marshaler) :
    _name(name), _marshaler(marshaler)
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

void
MemberMarshaler::destroy()
{
    _marshaler->destroy();
    _marshaler = 0;
}

//
// StructMarshaler implementation.
//
StructMarshaler::StructMarshaler(const Slice::StructPtr& type TSRMLS_DC) :
    _type(type)
{
    TypeMap* typeMap = static_cast<TypeMap*>(ICE_G(typeMap));
    TypeMap::iterator p = typeMap->find(type->scoped());
    assert(p != typeMap->end());
    _class = p->second;

    Slice::DataMemberList members = type->dataMembers();
    for(Slice::DataMemberList::iterator p = members.begin(); p != members.end(); ++p)
    {
        MarshalerPtr marshaler = createMemberMarshaler((*p)->name(), (*p)->type() TSRMLS_CC);
        assert(marshaler);
        _members.push_back(marshaler);
    }
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
        if(!(*p)->marshal(zv, os TSRMLS_CC))
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
        if(!(*p)->unmarshal(zv, is TSRMLS_CC))
        {
            return false;
        }
    }

    return true;
}

void
StructMarshaler::destroy()
{
    vector<MarshalerPtr> members = _members;
    _members.clear();
    for(vector<MarshalerPtr>::iterator p = members.begin(); p != members.end(); ++p)
    {
        (*p)->destroy();
    }
}

//
// EnumMarshaler implementation.
//
EnumMarshaler::EnumMarshaler(const Slice::EnumPtr& type TSRMLS_DC)
{
    TypeMap* typeMap = static_cast<TypeMap*>(ICE_G(typeMap));
    TypeMap::iterator p = typeMap->find(type->scoped());
    assert(p != typeMap->end());
    _class = p->second;
    _count = static_cast<long>(type->getEnumerators().size());
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

void
EnumMarshaler::destroy()
{
}

//
// NativeDictionaryMarshaler implementation.
//
NativeDictionaryMarshaler::NativeDictionaryMarshaler(const Slice::TypePtr& keyType, const Slice::TypePtr& valueType
                                                     TSRMLS_DC)
{
    Slice::BuiltinPtr b = Slice::BuiltinPtr::dynamicCast(keyType);
    assert(b);
    _keyKind = b->kind();
    _keyMarshaler = createMarshaler(keyType TSRMLS_CC);
    _valueMarshaler = createMarshaler(valueType TSRMLS_CC);
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

void
NativeDictionaryMarshaler::destroy()
{
    _keyMarshaler->destroy();
    _keyMarshaler = 0;
    _valueMarshaler->destroy();
    _valueMarshaler = 0;
}

//
// ExceptionMarshaler implementation.
//
ExceptionMarshaler::ExceptionMarshaler(const Slice::ExceptionPtr& ex TSRMLS_DC) :
    _ex(ex)
{
    TypeMap* typeMap = static_cast<TypeMap*>(ICE_G(typeMap));
    TypeMap::iterator p = typeMap->find(ex->scoped());
    assert(p != typeMap->end());
    _class = p->second;
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
            MarshalerPtr member = createMemberMarshaler((*p)->name(), (*p)->type() TSRMLS_CC);
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

void
ExceptionMarshaler::destroy()
{
}

//
// IceObjectSliceMarshaler implementation.
//
IceObjectSliceMarshaler::IceObjectSliceMarshaler(TSRMLS_D)
{
    Slice::UnitPtr unit = Slice_getUnit();
    Slice::TypePtr keyType = unit->builtin(Slice::Builtin::KindString);
    Slice::TypePtr valueType = unit->builtin(Slice::Builtin::KindObject);
    MarshalerPtr dict = new NativeDictionaryMarshaler(keyType, valueType TSRMLS_CC);
    _facetMapMarshaler = new MemberMarshaler("ice_facets", dict);
}

bool
IceObjectSliceMarshaler::marshal(zval* zv, IceInternal::BasicStream& os TSRMLS_DC)
{
    assert(Z_TYPE_P(zv) == IS_OBJECT);

    os.writeTypeId(Ice::Object::ice_staticId());
    os.startWriteSlice();
    if(!_facetMapMarshaler->marshal(zv, os TSRMLS_CC))
    {
        return false;
    }
    os.endWriteSlice();

    return true;
}

bool
IceObjectSliceMarshaler::unmarshal(zval* zv, IceInternal::BasicStream& is TSRMLS_DC)
{
    assert(Z_TYPE_P(zv) == IS_OBJECT);

    //
    // Do not read type id here - see ObjectWrapper::__read().
    //
    //is.readTypeId()

    is.startReadSlice();
    if(!_facetMapMarshaler->unmarshal(zv, is TSRMLS_CC))
    {
        return false;
    }
    is.endReadSlice();

    return true;
}

void
IceObjectSliceMarshaler::destroy()
{
    _facetMapMarshaler->destroy();
    _facetMapMarshaler = 0;
}

//
// ObjectSliceMarshaler implementation.
//
ObjectSliceMarshaler::ObjectSliceMarshaler(const string& scoped, const Slice::DataMemberList& members TSRMLS_DC) :
    _scoped(scoped)
{
    for(Slice::DataMemberList::const_iterator p = members.begin(); p != members.end(); ++p)
    {
        MarshalerPtr marshaler = createMemberMarshaler((*p)->name(), (*p)->type() TSRMLS_CC);
        assert(marshaler);
        _members.push_back(marshaler);
    }
}

bool
ObjectSliceMarshaler::marshal(zval* zv, IceInternal::BasicStream& os TSRMLS_DC)
{
    assert(Z_TYPE_P(zv) == IS_OBJECT);

    os.writeTypeId(_scoped);
    os.startWriteSlice();
    for(vector<MarshalerPtr>::iterator p = _members.begin(); p != _members.end(); ++p)
    {
        if(!(*p)->marshal(zv, os TSRMLS_CC))
        {
            return false;
        }
    }
    os.endWriteSlice();

    return true;
}

bool
ObjectSliceMarshaler::unmarshal(zval* zv, IceInternal::BasicStream& is TSRMLS_DC)
{
    assert(Z_TYPE_P(zv) == IS_OBJECT);

    //
    // Do not read type id here - see ObjectWrapper::__read().
    //
    //is.readTypeId()

    is.startReadSlice();
    for(vector<MarshalerPtr>::iterator p = _members.begin(); p != _members.end(); ++p)
    {
        if(!(*p)->unmarshal(zv, is TSRMLS_CC))
        {
            return false;
        }
    }
    is.endReadSlice();

    return true;
}

void
ObjectSliceMarshaler::destroy()
{
    vector<MarshalerPtr> members = _members;
    _members.clear();
    for(vector<MarshalerPtr>::iterator p = members.begin(); p != members.end(); ++p)
    {
        (*p)->destroy();
    }
}

static void
patchObject(void* addr, Ice::ObjectPtr& v)
{
    PatchInfo* info = static_cast<PatchInfo*>(addr);

    if(v)
    {
        ObjectWrapperPtr wrapper = ObjectWrapperPtr::dynamicCast(v);
        assert(wrapper);

        wrapper->setValue(info->ce, info->zv);
    }
    else
    {
        ZVAL_NULL(info->zv);
    }
}

//
// ObjectWrapper implementation.
//
ObjectWrapper::ObjectWrapper(zval* value, zend_object* obj TSRMLS_DC) :
    _value(value), _obj(obj)
{
#ifdef ZTS
    this->TSRMLS_C = TSRMLS_C;
#endif
}

void
ObjectWrapper::__write(IceInternal::BasicStream* os) const
{
    ice_class_entry* ce = (ice_class_entry*)_obj->ce;

    while(ce)
    {
        //
        // For class types, the marshaler member of ice_class_entry must contain an instance
        // of ObjectSliceMarshaler or IceObjectSliceMarshaler. If the class entry does not
        // have a value, we create one and cache it in the class entry.
        //
        MarshalerPtr slice;
        if(ce->marshaler)
        {
            slice = *(static_cast<MarshalerPtr*>(ce->marshaler));
        }
        else
        {
            Slice::SyntaxTreeBasePtr* base = static_cast<Slice::SyntaxTreeBasePtr*>(ce->syntaxTreeBase);
            Slice::ClassDefPtr def = Slice::ClassDefPtr::dynamicCast(*base);
            if(def)
            {
                slice = new ObjectSliceMarshaler(ce->scoped, def->dataMembers() TSRMLS_CC);
            }
            else
            {
                slice = new IceObjectSliceMarshaler(TSRMLS_C); // Type must be ::Ice::Object
            }
            ce->marshaler = new MarshalerPtr(slice);
        }

        if(!slice->marshal(_value, *os TSRMLS_CC))
        {
            return;
        }

        ce = (ice_class_entry*)ce->ce.parent;
    }
}

void
ObjectWrapper::__read(IceInternal::BasicStream* is, bool rid)
{
    ice_class_entry* ce = (ice_class_entry*)_obj->ce;

    while(ce)
    {
        if(rid)
        {
            string myId;
            is->readTypeId(myId);
        }

        //
        // For class types, the marshaler member of ice_class_entry must contain an instance
        // of ObjectSliceMarshaler or IceObjectSliceMarshaler. If the class entry does not
        // have a value, we create one and cache it in the class entry.
        //
        MarshalerPtr slice;
        if(ce->marshaler)
        {
            slice = *(static_cast<MarshalerPtr*>(ce->marshaler));
        }
        else
        {
            Slice::SyntaxTreeBasePtr* base = static_cast<Slice::SyntaxTreeBasePtr*>(ce->syntaxTreeBase);
            Slice::ClassDefPtr def = Slice::ClassDefPtr::dynamicCast(*base);
            if(def)
            {
                slice = new ObjectSliceMarshaler(ce->scoped, def->dataMembers() TSRMLS_CC);
            }
            else
            {
                slice = new IceObjectSliceMarshaler(TSRMLS_C); // Type must be ::Ice::Object
            }
            ce->marshaler = new MarshalerPtr(slice);
        }

        if(!slice->unmarshal(_value, *is TSRMLS_CC))
        {
            return;
        }

        ce = (ice_class_entry*)ce->ce.parent;

        rid = true;
    }
}

void
ObjectWrapper::setValue(zend_class_entry* ce, zval* zv)
{
    //
    // Compare the class entries. The argument "ce" represents the formal type.
    //
    if(_obj->ce != ce)
    {
        //
        // Check for inheritance.
        //
        zend_class_entry* c = _obj->ce->parent;
        while(c && c != ce)
        {
            c = c->parent;
        }

        if(c == NULL)
        {
            zend_error(E_ERROR, "expected object value of type %s but received %s", ce->name, _obj->ce->name);
            return;
        }
    }

    //
    // Now both zvals have the same object handle (they point at the same object). We need to
    // increment the object's reference count accordingly.
    //
    Z_TYPE_P(zv) = IS_OBJECT;
    zv->value.obj = _value->value.obj;
    Z_OBJ_HT_P(_value)->add_ref(_value TSRMLS_CC);
}

//
// ObjectMarshaler implementation.
//
ObjectMarshaler::ObjectMarshaler(const Slice::ClassDefPtr& type TSRMLS_DC)
{
    //
    // Find the class entry for this type.
    //
    string scoped;
    if(type)
    {
        scoped = type->scoped();
    }
    else
    {
        scoped = Ice::Object::ice_staticId();
    }

    TypeMap* typeMap = static_cast<TypeMap*>(ICE_G(typeMap));
    TypeMap::iterator p = typeMap->find(scoped);
    assert(p != typeMap->end());
    _class = p->second;
}

bool
ObjectMarshaler::marshal(zval* zv, IceInternal::BasicStream& os TSRMLS_DC)
{
    if(Z_TYPE_P(zv) == IS_NULL)
    {
        os.write(Ice::ObjectPtr());
        return true;
    }

    if(Z_TYPE_P(zv) != IS_OBJECT)
    {
        string s = zendTypeToString(Z_TYPE_P(zv));
        zend_error(E_ERROR, "expected object value of type %s but received %s", _class ? _class->name : "ice_object",
                   s.c_str());
        return false;
    }

    //
    // Retrieve the Zend object from the object store.
    //
    zend_object* obj = static_cast<zend_object*>(zend_object_store_get_object(zv TSRMLS_CC));
    if(!obj)
    {
        zend_error(E_ERROR, "object not found in object store");
        return false;
    }

    //
    // Compare the class entries.
    //
    if(obj->ce != _class)
    {
        //
        // Check for inheritance.
        //
        zend_class_entry* ce = obj->ce->parent;
        while(ce && ce != _class)
        {
            ce = ce->parent;
        }

        if(ce == NULL)
        {
            zend_error(E_ERROR, "expected object value of type %s but received %s", _class->name, obj->ce->name);
            return false;
        }
    }

    //
    // We need a C++ wrapper for the PHP object to be marshaled. It's possible that this object
    // has already been marshaled, therefore we first must check the object map to see if this
    // object is present. If so, we use the existing wrapper, otherwise we create a new one.
    // The key of the map is the object's handle.
    //
    Ice::ObjectPtr wrapper;

    //
    // Initialize the object map if necessary.
    //
    ObjectMap* objectMap = static_cast<ObjectMap*>(ICE_G(objectMap));
    if(objectMap == 0)
    {
        objectMap = new ObjectMap;
        ICE_G(objectMap) = objectMap;
        wrapper = new ObjectWrapper(zv, obj TSRMLS_CC);
        objectMap->insert(pair<unsigned int, Ice::ObjectPtr>(Z_OBJ_HANDLE_P(zv), wrapper));
    }
    else
    {
        ObjectMap::iterator p = objectMap->find(Z_OBJ_HANDLE_P(zv));
        if(p == objectMap->end())
        {
            wrapper = new ObjectWrapper(zv, obj TSRMLS_CC);
            objectMap->insert(pair<unsigned int, Ice::ObjectPtr>(Z_OBJ_HANDLE_P(zv), wrapper));
        }
        else
        {
            wrapper = p->second;
        }
    }

    //
    // Give the C++ wrapper object to the stream. The stream will eventually call __write on the wrapper.
    //
    os.write(wrapper);

    return true;
}

bool
ObjectMarshaler::unmarshal(zval* zv, IceInternal::BasicStream& is TSRMLS_DC)
{
    //
    // Allocate patch information and store it in the patch list for later destruction.
    // We cannot simply destroy this in the patch callback function because it might
    // never be called if an exception occurs.
    //
    PatchInfo* info = new PatchInfo;
    info->ce = _class;
    info->zv = zv;

    PatchList* pl = static_cast<PatchList*>(ICE_G(patchList));
    if(!pl)
    {
        pl = new PatchList;
        ICE_G(patchList) = pl;
    }

    pl->push_back(info);

    //
    // Invoke read(), passing our patch callback function and the patch information. When
    // the object is eventually unmarshaled, our callback function will be invoked and
    // we will assign a value to zv.
    //
    is.read(patchObject, info);

    return true;
}

void
ObjectMarshaler::destroy()
{
}

//
// PHPObjectFactory implementation.
//
PHPObjectFactory::PHPObjectFactory(TypeMap& typeMap TSRMLS_DC) :
    _typeMap(typeMap)
{
#ifdef ZTS
    this->TSRMLS_C = TSRMLS_C;
#endif
}

Ice::ObjectPtr
PHPObjectFactory::create(const string& scoped)
{
    //
    // Lookup the type in the type map in order to obtain the class entry.
    //
    TypeMap::iterator p = _typeMap.find(scoped);
    assert(p != _typeMap.end());

    //
    // Create a zval to hold the new object.
    //
    zval* zv;
    MAKE_STD_ZVAL(zv);

    //
    // Instantiate the new object.
    //
    if(object_init_ex(zv, p->second) != SUCCESS)
    {
        zend_error(E_ERROR, "unable to initialize object of type %s", p->second->name);
        return 0;
    }

    //
    // Retrieve the object from the object store.
    //
    zend_object* obj = static_cast<zend_object*>(zend_object_store_get_object(zv TSRMLS_CC));
    assert(obj);

    //
    // Return the object wrapper.
    //
    return new ObjectWrapper(zv, obj TSRMLS_CC);
}

void
PHPObjectFactory::destroy()
{
}

//
// FactoryVisitor registers the same PHPObjectFactory instance for each non-abstract class.
//
FactoryVisitor::FactoryVisitor(const Ice::CommunicatorPtr& communicator, TypeMap& typeMap TSRMLS_DC) :
    _communicator(communicator), _factory(new PHPObjectFactory(typeMap TSRMLS_CC))
{
    //
    // Intercept the creation of Ice::Object.
    //
    _communicator->addObjectFactory(_factory, Ice::Object::ice_staticId());
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
