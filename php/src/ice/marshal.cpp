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

#include "ice_marshal.h"
#include "ice_proxy.h"
#include "ice_slice.h"
#include "ice_util.h"

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
// and then obtains or creates an ObjectWriter instance for the object. Since each PHP
// object is represented by an unsigned integer handle, looking up the wrapper is simple.
//
// Once the writer is obtained, the marshaler gives it to the stream. Eventually, the
// stream will invoke __write on the writer, at which point the data members are
// marshaled. For efficiency, each "slice" of the object's state is marshaled by an
// ObjectSliceMarshaler, which is cached in the class entry for future reuse.
//
// Note that a graph of PHP objects does not result in an equivalent graph of writers.
// Links between objects exist only in the PHP object representation. Furthermore, the
// lifetime of the writers is bound to the operation, not to the PHP objects. Writers
// exist only as a bridge to the C++ marshaling facility.
//
// The table which associates a PHP object's handle to its writer is stored in a member
// of our "module globals" (see php_ice.h), which are similar to thread-specific storage.
// The object map is not created until it is first needed, and is destroyed after each
// operation.
//
// Unmarshaling of Ice objects works in a similar fashion. Before each request, the
// extension descends the Slice parse tree and installs an object factory for each
// concrete class type. The same factory instance is registered for all relevant
// types, including the type "::Ice::Object".
//
// When the factory is invoked, it uses the type map to look up the zend_class_entry
// associated with the given type id, therefore it is capable of creating any object.
// It returns an instance of ObjectReader, a subclass of Ice::Object that overrides
// the __read method to unmarshal object state. The setValue method is eventually
// called on the ObjectReader in order to transfer its object handle to a different
// zval value.
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

class ObjectWriter : public Ice::Object
{
public:
    ObjectWriter(zval*, const Slice::SyntaxTreeBasePtr& TSRMLS_DC);
    ~ObjectWriter();

    virtual void __write(::IceInternal::BasicStream*) const;
    virtual void __read(::IceInternal::BasicStream*, bool = true);

private:
    zval* _value;
    Slice::ClassDefPtr _type; // nil if type is ::Ice::Object
#ifdef ZTS
    TSRMLS_D;
#endif
};

class ObjectReader : public Ice::Object
{
public:
    ObjectReader(zend_class_entry*, const Slice::ClassDefPtr& TSRMLS_DC);
    ~ObjectReader();

    virtual void __write(::IceInternal::BasicStream*) const;
    virtual void __read(::IceInternal::BasicStream*, bool = true);

    void setValue(zend_class_entry*, zval*);

private:
    zend_class_entry* _class;
    Slice::ClassDefPtr _type; // nil if type is ::Ice::Object
#ifdef ZTS
    TSRMLS_D;
#endif
    zval* _value;
};
typedef IceUtil::Handle<ObjectReader> ObjectReaderPtr;

class ObjectMarshaler : public Marshaler
{
public:
    ObjectMarshaler(const Slice::ClassDefPtr& TSRMLS_DC);

    virtual bool marshal(zval*, IceInternal::BasicStream& TSRMLS_DC);
    virtual bool unmarshal(zval*, IceInternal::BasicStream& TSRMLS_DC);

    virtual void destroy();

private:
    Slice::ClassDefPtr _def;
    zend_class_entry* _class; // The static class type.
};

//
// PHPObjectFactory is an implementation of Ice::ObjectFactory that creates PHP objects.
// A single instance can be used for all types.
//
class PHPObjectFactory : public Ice::ObjectFactory
{
public:
    PHPObjectFactory(TSRMLS_D);

    virtual Ice::ObjectPtr create(const string&);
    virtual void destroy();

private:
#ifdef ZTS
    TSRMLS_D;
#endif
};

void
Marshal_initCommunicator(const Ice::CommunicatorPtr& communicator TSRMLS_DC)
{
    //
    // Register our default object factory with the communicator.
    //
    Ice::ObjectFactoryPtr factory = new PHPObjectFactory(TSRMLS_C);
    communicator->addObjectFactory(factory, "");
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
        // Check to see if a marshaler for this type has already been created. If not, create
        // one and cache it in the marshaler map for future use.
        //
        string scoped = st->scoped();
        MarshalerMap* marshalerMap = static_cast<MarshalerMap*>(ICE_G(marshalerMap));
        MarshalerMap::iterator p = marshalerMap->find(scoped);
        if(p != marshalerMap->end())
        {
            return p->second;
        }
        else
        {
            MarshalerPtr result = new StructMarshaler(st TSRMLS_CC);
            marshalerMap->insert(pair<string, MarshalerPtr>(scoped, result));
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
        // Don't cache ObjectMarshaler - we cache ObjectSliceMarshaler instead.
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
    switch(_type->kind())
    {
    case Slice::Builtin::KindBool:
    {
        if(Z_TYPE_P(zv) != IS_BOOL)
        {
            string s = ice_zendTypeToString(Z_TYPE_P(zv));
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
            string s = ice_zendTypeToString(Z_TYPE_P(zv));
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
            string s = ice_zendTypeToString(Z_TYPE_P(zv));
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
            string s = ice_zendTypeToString(Z_TYPE_P(zv));
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
            string s = ice_zendTypeToString(Z_TYPE_P(zv));
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
            string s = ice_zendTypeToString(Z_TYPE_P(zv));
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
            string s = ice_zendTypeToString(Z_TYPE_P(zv));
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
            string s = ice_zendTypeToString(Z_TYPE_P(zv));
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
            ZVAL_LONG(zv, static_cast<long>(val));
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
        string s = ice_zendTypeToString(Z_TYPE_P(zv));
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
        string s = ice_zendTypeToString(Z_TYPE_P(zv));
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
        zend_error(E_ERROR, "member `%s' is not defined", _name.c_str());
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
    zval_ptr_dtor(&val); // add_property_zval increments the refcount

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
    _class = ice_findClassScoped(type->scoped(), "" TSRMLS_CC);
    assert(_class);

    Slice::DataMemberList members = type->dataMembers();
    for(Slice::DataMemberList::iterator q = members.begin(); q != members.end(); ++q)
    {
        MarshalerPtr marshaler = createMemberMarshaler((*q)->name(), (*q)->type() TSRMLS_CC);
        assert(marshaler);
        _members.push_back(marshaler);
    }
}

bool
StructMarshaler::marshal(zval* zv, IceInternal::BasicStream& os TSRMLS_DC)
{
    if(Z_TYPE_P(zv) != IS_OBJECT)
    {
        string s = ice_zendTypeToString(Z_TYPE_P(zv));
        zend_error(E_ERROR, "expected struct value of type %s but received %s", _class->name, s.c_str());
        return false;
    }

    //
    // Compare class entries.
    //
    zend_class_entry* ce = Z_OBJCE_P(zv);
    if(ce != _class)
    {
        zend_error(E_ERROR, "expected struct value of type %s but received %s", _class->name, ce->name);
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
    _class = ice_findClassScoped(type->scoped(), "" TSRMLS_CC);
    assert(_class);
    _count = static_cast<long>(type->getEnumerators().size());
}

bool
EnumMarshaler::marshal(zval* zv, IceInternal::BasicStream& os TSRMLS_DC)
{
    if(Z_TYPE_P(zv) != IS_LONG)
    {
        string s = ice_zendTypeToString(Z_TYPE_P(zv));
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
        string s = ice_zendTypeToString(Z_TYPE_P(zv));
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
            ZVAL_STRINGL(&zkey, keyStr, keyLen - 1, 1);
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
        zval key;
        zval* val;
        INIT_ZVAL(key);
        MAKE_STD_ZVAL(val);

        if(!_keyMarshaler->unmarshal(&key, is TSRMLS_CC))
        {
            return false;
        }
        if(!_valueMarshaler->unmarshal(val, is TSRMLS_CC))
        {
            return false;
        }

        switch(Z_TYPE(key))
        {
        case IS_LONG:
            add_index_zval(zv, Z_LVAL(key), val);
            break;
        case IS_BOOL:
            add_index_zval(zv, Z_BVAL(key) ? 1 : 0, val);
            break;
        case IS_STRING:
            add_assoc_zval_ex(zv, Z_STRVAL(key), Z_STRLEN(key) + 1, val);
            break;
        default:
            assert(false);
            return false;
        }
        zval_dtor(&key);
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
    _class = ice_findClassScoped(ex->scoped(), "" TSRMLS_CC);
    assert(_class);
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
    //
    // Create a marshaler for the ice_facets member.
    //
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
    // Do not read type id here - see ObjectReader::__read().
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
    // Do not read type id here - see ObjectReader::__read().
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
    PHPStream::PatchInfo* info = static_cast<PHPStream::PatchInfo*>(addr);

    if(v)
    {
        ObjectReaderPtr reader = ObjectReaderPtr::dynamicCast(v);
        assert(reader);

        reader->setValue(info->ce, info->zv);
    }
    else
    {
        ZVAL_NULL(info->zv);
    }
}

//
// ObjectWriter implementation.
//
ObjectWriter::ObjectWriter(zval* value, const Slice::SyntaxTreeBasePtr& type TSRMLS_DC) :
    _value(value)
{
    _type = Slice::ClassDefPtr::dynamicCast(type);

#ifdef ZTS
    this->TSRMLS_C = TSRMLS_C;
#endif

    Z_OBJ_HT_P(_value)->add_ref(_value TSRMLS_CC);
}

ObjectWriter::~ObjectWriter()
{
    Z_OBJ_HT_P(_value)->del_ref(_value TSRMLS_CC);
}

void
ObjectWriter::__write(IceInternal::BasicStream* os) const
{
    MarshalerMap* marshalerMap = static_cast<MarshalerMap*>(ICE_G(marshalerMap));

    Slice::ClassDefPtr def = _type;
    while(true)
    {
        string scoped = def->scoped();
        MarshalerPtr slice;
        MarshalerMap::iterator p = marshalerMap->find(scoped);
        if(p != marshalerMap->end())
        {
            slice = p->second;
        }
        else
        {
            slice = new ObjectSliceMarshaler(scoped, def->dataMembers() TSRMLS_CC);
            marshalerMap->insert(pair<string, MarshalerPtr>(scoped, slice));
        }

        if(!slice->marshal(_value, *os TSRMLS_CC))
        {
            throw AbortMarshaling();
        }

        Slice::ClassList bases = def->bases();
        if(!bases.empty() && !bases.front()->isInterface())
        {
            def = bases.front();
        }
        else
        {
            break;
        }
    }

    //
    // Marshal the Ice::Object slice.
    //
    MarshalerPtr slice;
    MarshalerMap::iterator p = marshalerMap->find(Ice::Object::ice_staticId());
    if(p != marshalerMap->end())
    {
        slice = p->second;
    }
    else
    {
        slice = new IceObjectSliceMarshaler(TSRMLS_C);
        marshalerMap->insert(pair<string, MarshalerPtr>(Ice::Object::ice_staticId(), slice));
    }

    if(!slice->marshal(_value, *os TSRMLS_CC))
    {
        throw AbortMarshaling();
    }
}

void
ObjectWriter::__read(IceInternal::BasicStream* is, bool rid)
{
    zend_error(E_ERROR, "ObjectWriter::__read should never be called");
}

//
// ObjectReader implementation.
//
ObjectReader::ObjectReader(zend_class_entry* cls, const Slice::ClassDefPtr& type TSRMLS_DC) :
    _class(cls), _type(type)
{
#ifdef ZTS
    this->TSRMLS_C = TSRMLS_C;
#endif

    //
    // Create a zval to hold the new object.
    //
    MAKE_STD_ZVAL(_value);

    //
    // Instantiate the new object.
    //
    object_init_ex(_value, _class);
}

ObjectReader::~ObjectReader()
{
    Z_OBJ_HT_P(_value)->del_ref(_value TSRMLS_CC);
    efree(_value);
}

void
ObjectReader::__write(IceInternal::BasicStream* os) const
{
    zend_error(E_ERROR, "ObjectReader::__write should never be called");
}

void
ObjectReader::__read(IceInternal::BasicStream* is, bool rid)
{
    MarshalerMap* marshalerMap = static_cast<MarshalerMap*>(ICE_G(marshalerMap));

    //
    // Unmarshal the slices of a user-defined type.
    //
    if(_type)
    {
        Slice::ClassDefPtr def = _type;
        while(true)
        {
            string scoped;
            if(rid)
            {
                is->readTypeId(scoped);
            }
            else
            {
                scoped = def->scoped();
            }

            MarshalerPtr slice;
            MarshalerMap::iterator p = marshalerMap->find(scoped);
            if(p != marshalerMap->end())
            {
                slice = p->second;
            }
            else
            {
                slice = new ObjectSliceMarshaler(scoped, def->dataMembers() TSRMLS_CC);
                marshalerMap->insert(pair<string, MarshalerPtr>(scoped, slice));
            }

            if(!slice->unmarshal(_value, *is TSRMLS_CC))
            {
                throw AbortMarshaling();
            }

            rid = true;

            Slice::ClassList bases = def->bases();
            if(!bases.empty() && !bases.front()->isInterface())
            {
                def = bases.front();
            }
            else
            {
                break;
            }
        }
    }

    //
    // Unmarshal the Ice::Object slice.
    //
    if(rid)
    {
        string myId;
        is->readTypeId(myId);
    }

    MarshalerPtr slice;
    MarshalerMap::iterator p = marshalerMap->find(Ice::Object::ice_staticId());
    if(p != marshalerMap->end())
    {
        slice = p->second;
    }
    else
    {
        slice = new IceObjectSliceMarshaler(TSRMLS_C);
        marshalerMap->insert(pair<string, MarshalerPtr>(Ice::Object::ice_staticId(), slice));
    }

    if(!slice->unmarshal(_value, *is TSRMLS_CC))
    {
        throw AbortMarshaling();
    }
}

void
ObjectReader::setValue(zend_class_entry* ce, zval* zv)
{
    //
    // Compare the class entries. The argument "ce" represents the formal type.
    //
    if(_class != ce)
    {
        //
        // Check for inheritance.
        //
        zend_class_entry* c = _class->parent;
        while(c && c != ce)
        {
            c = c->parent;
        }

        if(c == NULL)
        {
            zend_error(E_ERROR, "expected object value of type %s but received %s", ce->name, _class->name);
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
ObjectMarshaler::ObjectMarshaler(const Slice::ClassDefPtr& def TSRMLS_DC) :
    _def(def)
{
    //
    // Find the class entry for this type.
    //
    if(def)
    {
        string scoped = def->scoped();
        _class = ice_findClassScoped(scoped, "" TSRMLS_CC);
    }
    else
    {
        _class = ice_findClass("Ice_ObjectImpl" TSRMLS_CC);
    }

    assert(_class);
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
        string s = ice_zendTypeToString(Z_TYPE_P(zv));
        zend_error(E_ERROR, "expected object value of type %s but received %s", _class ? _class->name : "ice_object",
                   s.c_str());
        return false;
    }

    //
    // Compare the class entries.
    //
    zend_class_entry* ce = Z_OBJCE_P(zv);
    if(ce != _class)
    {
        //
        // Check for inheritance.
        //
        zend_class_entry* parent = ce->parent;
        while(parent && parent != _class)
        {
            parent = parent->parent;
        }

        if(parent == NULL)
        {
            zend_error(E_ERROR, "expected object value of type %s but received %s", _class->name, ce->name);
            return false;
        }
    }

    PHPStream* po = static_cast<PHPStream*>(&os);

    //
    // ObjectWriter is a subclass of Ice::Object that wraps a PHP object for marshaling. It is
    // possible that this PHP object has already been marshaled, therefore we first must check
    // the object map to see if this object is present. If so, we use the existing ObjectWriter,
    // otherwise we create a new one. The key of the map is the object's handle.
    //
    Ice::ObjectPtr writer;

    //
    // Retrieve the ClassDef for the actual type. This may fail if the type is Ice::Object.
    //
    Slice::ClassDefPtr def = Slice_getClassDef(ce->name);

    //
    // Initialize the object map if necessary.
    //
    if(po->objectMap == 0)
    {
        po->objectMap = new PHPStream::ObjectMap;
        writer = new ObjectWriter(zv, def TSRMLS_CC);
        po->objectMap->insert(pair<unsigned int, Ice::ObjectPtr>(Z_OBJ_HANDLE_P(zv), writer));
    }
    else
    {
        PHPStream::ObjectMap::iterator p = po->objectMap->find(Z_OBJ_HANDLE_P(zv));
        if(p == po->objectMap->end())
        {
            writer = new ObjectWriter(zv, def TSRMLS_CC);
            po->objectMap->insert(pair<unsigned int, Ice::ObjectPtr>(Z_OBJ_HANDLE_P(zv), writer));
        }
        else
        {
            writer = p->second;
        }
    }

    //
    // Give the writer to the stream. The stream will eventually call __write on it.
    //
    os.write(writer);

    return true;
}

bool
ObjectMarshaler::unmarshal(zval* zv, IceInternal::BasicStream& is TSRMLS_DC)
{
    PHPStream* pi = static_cast<PHPStream*>(&is);

    //
    // Allocate patch information and store it in the patch list for later destruction.
    // We cannot simply destroy this in the patch callback function because it might
    // never be called if an exception occurs.
    //
    PHPStream::PatchInfo* info = new PHPStream::PatchInfo;
    info->ce = _class;
    info->zv = zv;

    if(!pi->patchList)
    {
        pi->patchList = new PHPStream::PatchInfoList;
    }

    pi->patchList->push_back(info);

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
PHPObjectFactory::PHPObjectFactory(TSRMLS_D)
{
#ifdef ZTS
    this->TSRMLS_C = TSRMLS_C;
#endif
}

Ice::ObjectPtr
PHPObjectFactory::create(const string& scoped)
{
    Ice::ObjectPtr result;

    //
    // Attempt to find a class entry for the given type id. If no class entry is
    // found, or the class is abstract, then we return nil and the stream will skip
    // the slice and try again.
    //
    zend_class_entry* cls = NULL;
    Slice::ClassDefPtr def;
    if(scoped == Ice::Object::ice_staticId())
    {
        cls = ice_findClass("Ice_ObjectImpl" TSRMLS_CC);
    }
    else
    {
        cls = ice_findClassScoped(scoped, "" TSRMLS_CC);
    }
    if(cls && (cls->ce_flags & ZEND_CE_ABSTRACT) == 0)
    {
        def = Slice_getClassDef(cls->name);
        result = new ObjectReader(cls, def TSRMLS_CC);
    }

    return result;
}

void
PHPObjectFactory::destroy()
{
}

//
// PHPStream implementation.
//
PHPStream::PHPStream(IceInternal::Instance* instance) :
    IceInternal::BasicStream(instance), objectMap(0), patchList(0)
{
}

PHPStream::~PHPStream()
{
    delete objectMap;
    if(patchList)
    {
        for(PatchInfoList::iterator p = patchList->begin(); p != patchList->end(); ++p)
        {
            delete (*p);
        }
        delete patchList;
    }
}
