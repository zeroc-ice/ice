// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "ice_marshal.h"
#include "ice_profile.h"
#include "ice_proxy.h"
#include "ice_util.h"

#include <IceUtil/InputUtil.h>

using namespace std;
using namespace IcePHP;

ZEND_EXTERN_MODULE_GLOBALS(ice)

//
// The marshaling implementation is fairly straightforward. The factory methods in the
// Marshaler base class examine the given Slice type and create a Marshaler subclass
// that is responsible for marshaling that type. Some caching is done for complex types
// such as struct and class; the cached Marshaler instance is stored as a member of the
// ice_class_entry struct. (Cached instances are destroyed by Slice_destroyClasses.)
//
// The implementation of Ice object marshaling is more complex. In order to interface
// with the Ice stream interface, we need to supply Ice::Object instances, and we must
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
// stream will invoke write on the writer, at which point the data members are
// marshaled. For efficiency, each "slice" of the object's state is marshaled by an
// ObjectSliceMarshaler, which is cached for future reuse.
//
// Note that a graph of PHP objects does not result in an equivalent graph of writers.
// Links between objects exist only in the PHP object representation. Furthermore, the
// lifetime of the writers is bound to the operation, not to the PHP objects. Writers
// exist only as a bridge to the C++ marshaling facility.
//
// Unmarshaling of Ice objects works in a similar fashion. A default object factory
// is installed in the communicator that is capable of instantiating any concrete
// class type for which a definition is present, including the type "::Ice::Object".
// It returns an instance of ObjectReader, a subclass of Ice::Object that overrides
// the read method to unmarshal object state. The setValue method is eventually
// called on the ObjectReader in order to transfer its object handle to a different
// zval value.
//

namespace IcePHP
{

//
// Marshaler subclass definitions.
//
class PrimitiveMarshaler : public Marshaler
{
public:
    PrimitiveMarshaler(const Slice::BuiltinPtr&);

    virtual bool marshal(zval*, const Ice::OutputStreamPtr&, ObjectMap& TSRMLS_DC);
    virtual bool unmarshal(zval*, const Ice::InputStreamPtr& TSRMLS_DC);

    virtual void destroy();

private:
    Slice::BuiltinPtr _type;
};

class SequenceMarshaler : public Marshaler
{
public:
    SequenceMarshaler(const Slice::SequencePtr& TSRMLS_DC);

    virtual bool marshal(zval*, const Ice::OutputStreamPtr&, ObjectMap& TSRMLS_DC);
    virtual bool unmarshal(zval*, const Ice::InputStreamPtr& TSRMLS_DC);

    virtual void destroy();

private:
    Slice::SequencePtr _type;
    MarshalerPtr _elementMarshaler;
};

class ProxyMarshaler : public Marshaler
{
public:
    ProxyMarshaler(const Slice::ProxyPtr&);

    virtual bool marshal(zval*, const Ice::OutputStreamPtr&, ObjectMap& TSRMLS_DC);
    virtual bool unmarshal(zval*, const Ice::InputStreamPtr& TSRMLS_DC);

    virtual void destroy();

private:
    Slice::ProxyPtr _type;
};

class MemberMarshaler : public Marshaler
{
public:
    MemberMarshaler(const string&, const MarshalerPtr&);

    virtual bool marshal(zval*, const Ice::OutputStreamPtr&, ObjectMap& TSRMLS_DC);
    virtual bool unmarshal(zval*, const Ice::InputStreamPtr& TSRMLS_DC);

    virtual void destroy();

private:
    string _name;
    MarshalerPtr _marshaler;
};

class StructMarshaler : public Marshaler
{
public:
    StructMarshaler(const Slice::StructPtr& TSRMLS_DC);

    virtual bool marshal(zval*, const Ice::OutputStreamPtr&, ObjectMap& TSRMLS_DC);
    virtual bool unmarshal(zval*, const Ice::InputStreamPtr& TSRMLS_DC);

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

    virtual bool marshal(zval*, const Ice::OutputStreamPtr&, ObjectMap& TSRMLS_DC);
    virtual bool unmarshal(zval*, const Ice::InputStreamPtr& TSRMLS_DC);

    virtual void destroy();

private:
    zend_class_entry* _class;
    long _count;
};

class NativeDictionaryMarshaler : public Marshaler
{
public:
    NativeDictionaryMarshaler(const Slice::TypePtr&, const Slice::TypePtr& TSRMLS_DC);

    virtual bool marshal(zval*, const Ice::OutputStreamPtr&, ObjectMap& TSRMLS_DC);
    virtual bool unmarshal(zval*, const Ice::InputStreamPtr& TSRMLS_DC);

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

    virtual bool marshal(zval*, const Ice::OutputStreamPtr&, ObjectMap& TSRMLS_DC);
    virtual bool unmarshal(zval*, const Ice::InputStreamPtr& TSRMLS_DC);

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

    virtual bool marshal(zval*, const Ice::OutputStreamPtr&, ObjectMap& TSRMLS_DC);
    virtual bool unmarshal(zval*, const Ice::InputStreamPtr& TSRMLS_DC);

    virtual void destroy();
};

class ObjectSliceMarshaler : public Marshaler
{
public:
    ObjectSliceMarshaler(const string&, const Slice::DataMemberList& TSRMLS_DC);

    virtual bool marshal(zval*, const Ice::OutputStreamPtr&, ObjectMap& TSRMLS_DC);
    virtual bool unmarshal(zval*, const Ice::InputStreamPtr& TSRMLS_DC);

    virtual void destroy();

private:
    string _scoped;
    vector<MarshalerPtr> _members;
};

class ObjectWriter : public Ice::ObjectWriter
{
public:
    ObjectWriter(zval*, const Slice::SyntaxTreeBasePtr&, ObjectMap& TSRMLS_DC);
    ~ObjectWriter();

    virtual void ice_preMarshal();

    virtual void write(const Ice::OutputStreamPtr&) const;

private:
    zval* _value;
    Slice::ClassDefPtr _type; // nil if type is ::Ice::Object
    ObjectMap& _map;
#ifdef ZTS
    TSRMLS_D;
#endif
};

class ReadObjectCallback : public Ice::ReadObjectCallback
{
public:

    virtual void invoke(const ::Ice::ObjectPtr&);

    zend_class_entry* ce; // The formal type
    string scoped;
    zval* zv;             // The destination zval
};
typedef IceUtil::Handle<ReadObjectCallback> ReadObjectCallbackPtr;

class ObjectReader : public Ice::ObjectReader
{
public:
    ObjectReader(zval*, const Slice::ClassDefPtr& TSRMLS_DC);
    ~ObjectReader();

    virtual void ice_postUnmarshal();

    virtual void read(const Ice::InputStreamPtr&, bool);

    void setValue(zend_class_entry*, const string&, zval*);

private:
    zval* _value;
    Slice::ClassDefPtr _type; // nil if type is ::Ice::Object
#ifdef ZTS
    TSRMLS_D;
#endif
    zend_class_entry* _class;
};
typedef IceUtil::Handle<ObjectReader> ObjectReaderPtr;

class ObjectMarshaler : public Marshaler
{
public:
    ObjectMarshaler(const Slice::ClassDefPtr& TSRMLS_DC);

    virtual bool marshal(zval*, const Ice::OutputStreamPtr&, ObjectMap& TSRMLS_DC);
    virtual bool unmarshal(zval*, const Ice::InputStreamPtr& TSRMLS_DC);

    virtual void destroy();

private:
    Slice::ClassDefPtr _def;
    zend_class_entry* _class; // The static class type.
    string _scoped;
};

} // End of namespace IcePHP

//
// Marshaler implementation.
//
IcePHP::Marshaler::Marshaler()
{
}

IcePHP::Marshaler::~Marshaler()
{
}

MarshalerPtr
IcePHP::Marshaler::createMarshaler(const Slice::TypePtr& type TSRMLS_DC)
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
            return new ProxyMarshaler(0);

        case Slice::Builtin::KindLocalObject:
            zend_error(E_ERROR, "%s(): unexpected local type", get_active_function_name(TSRMLS_C));
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
        return new ProxyMarshaler(proxy);
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
        if(isNativeKey(dict->keyType()))
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
            zend_error(E_ERROR, "%s(): cannot use Slice %s %s because it has not been defined",
                       get_active_function_name(TSRMLS_C), cl->isInterface() ? "interface" : "class", scoped.c_str());
            return 0;
        }
        return new ObjectMarshaler(def TSRMLS_CC);
    }

    return 0;
}

MarshalerPtr
IcePHP::Marshaler::createMemberMarshaler(const string& name, const Slice::TypePtr& type TSRMLS_DC)
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
IcePHP::Marshaler::createExceptionMarshaler(const Slice::ExceptionPtr& ex TSRMLS_DC)
{
   return new ExceptionMarshaler(ex TSRMLS_CC);
}

//
// PrimitiveMarshaler implementation.
//
IcePHP::PrimitiveMarshaler::PrimitiveMarshaler(const Slice::BuiltinPtr& type) :
    _type(type)
{
}

bool
IcePHP::PrimitiveMarshaler::marshal(zval* zv, const Ice::OutputStreamPtr& os, ObjectMap& TSRMLS_DC)
{
    switch(_type->kind())
    {
    case Slice::Builtin::KindBool:
    {
        if(Z_TYPE_P(zv) != IS_BOOL)
        {
            string s = zendTypeToString(Z_TYPE_P(zv));
            zend_error(E_ERROR, "%s(): expected boolean value but received %s", get_active_function_name(TSRMLS_C),
                       s.c_str());
            return false;
        }
        os->writeBool(Z_BVAL_P(zv) ? true : false);
        break;
    }
    case Slice::Builtin::KindByte:
    {
        if(Z_TYPE_P(zv) != IS_LONG)
        {
            string s = zendTypeToString(Z_TYPE_P(zv));
            zend_error(E_ERROR, "%s(): expected byte value but received %s", get_active_function_name(TSRMLS_C),
                       s.c_str());
            return false;
        }
        long val = Z_LVAL_P(zv);
        if(val < 0 || val > 255)
        {
            zend_error(E_ERROR, "%s(): value %ld is out of range for a byte", get_active_function_name(TSRMLS_C), val);
            return false;
        }
        os->writeByte(static_cast<Ice::Byte>(val));
        break;
    }
    case Slice::Builtin::KindShort:
    {
        if(Z_TYPE_P(zv) != IS_LONG)
        {
            string s = zendTypeToString(Z_TYPE_P(zv));
            zend_error(E_ERROR, "%s(): expected short value but received %s", get_active_function_name(TSRMLS_C),
                       s.c_str());
            return false;
        }
        long val = Z_LVAL_P(zv);
        if(val < SHRT_MIN || val > SHRT_MAX)
        {
            zend_error(E_ERROR, "%s(): value %ld is out of range for a short", get_active_function_name(TSRMLS_C), val);
            return false;
        }
        os->writeShort(static_cast<Ice::Short>(val));
        break;
    }
    case Slice::Builtin::KindInt:
    {
        if(Z_TYPE_P(zv) != IS_LONG)
        {
            string s = zendTypeToString(Z_TYPE_P(zv));
            zend_error(E_ERROR, "%s(): expected int value but received %s", get_active_function_name(TSRMLS_C),
                       s.c_str());
            return false;
        }
        long val = Z_LVAL_P(zv);
        if(val < INT_MIN || val > INT_MAX)
        {
            zend_error(E_ERROR, "%s(): value %ld is out of range for an int", get_active_function_name(TSRMLS_C), val);
            return false;
        }
        os->writeInt(static_cast<Ice::Int>(val));
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
            zend_error(E_ERROR, "%s(): expected long value but received %s", get_active_function_name(TSRMLS_C),
                       s.c_str());
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
                zend_error(E_ERROR, "%s(): invalid long value `%s'", get_active_function_name(TSRMLS_C),
                           Z_STRVAL_P(zv));
                return false;
            }
        }
        os->writeLong(val);
        break;
    }
    case Slice::Builtin::KindFloat:
    {
        if(Z_TYPE_P(zv) != IS_DOUBLE)
        {
            string s = zendTypeToString(Z_TYPE_P(zv));
            zend_error(E_ERROR, "%s(): expected float value but received %s", get_active_function_name(TSRMLS_C),
                       s.c_str());
            return false;
        }
        double val = Z_DVAL_P(zv);
        os->writeFloat(static_cast<Ice::Float>(val));
        break;
    }
    case Slice::Builtin::KindDouble:
    {
        if(Z_TYPE_P(zv) != IS_DOUBLE)
        {
            string s = zendTypeToString(Z_TYPE_P(zv));
            zend_error(E_ERROR, "%s(): expected double value but received %s", get_active_function_name(TSRMLS_C),
                       s.c_str());
            return false;
        }
        double val = Z_DVAL_P(zv);
        os->writeDouble(val);
        break;
    }
    case Slice::Builtin::KindString:
    {
        if(Z_TYPE_P(zv) == IS_STRING)
        {
            string val(Z_STRVAL_P(zv), Z_STRLEN_P(zv));
            os->writeString(val);
        }
        else if(Z_TYPE_P(zv) == IS_NULL)
        {
            os->writeString(string());
        }
        else
        {
            string s = zendTypeToString(Z_TYPE_P(zv));
            zend_error(E_ERROR, "%s(): expected string value but received %s", get_active_function_name(TSRMLS_C),
                       s.c_str());
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
IcePHP::PrimitiveMarshaler::unmarshal(zval* zv, const Ice::InputStreamPtr& is TSRMLS_DC)
{
    switch(_type->kind())
    {
    case Slice::Builtin::KindBool:
    {
        bool val = is->readBool();
        ZVAL_BOOL(zv, val ? 1 : 0);
        break;
    }
    case Slice::Builtin::KindByte:
    {
        Ice::Byte val = is->readByte();
        ZVAL_LONG(zv, val & 0xff);
        break;
    }
    case Slice::Builtin::KindShort:
    {
        Ice::Short val = is->readShort();
        ZVAL_LONG(zv, val);
        break;
    }
    case Slice::Builtin::KindInt:
    {
        Ice::Int val = is->readInt();
        ZVAL_LONG(zv, val);
        break;
    }
    case Slice::Builtin::KindLong:
    {
        Ice::Long val = is->readLong();

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
        Ice::Float val = is->readFloat();
        ZVAL_DOUBLE(zv, val);
        break;
    }
    case Slice::Builtin::KindDouble:
    {
        Ice::Double val = is->readDouble();
        ZVAL_DOUBLE(zv, val);
        break;
    }
    case Slice::Builtin::KindString:
    {
        string val = is->readString();
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
IcePHP::PrimitiveMarshaler::destroy()
{
}

//
// SequenceMarshaler implementation.
//
IcePHP::SequenceMarshaler::SequenceMarshaler(const Slice::SequencePtr& type TSRMLS_DC) :
    _type(type)
{
    _elementMarshaler = createMarshaler(type->type() TSRMLS_CC);
}

bool
IcePHP::SequenceMarshaler::marshal(zval* zv, const Ice::OutputStreamPtr& os, ObjectMap& m TSRMLS_DC)
{
    if(Z_TYPE_P(zv) != IS_ARRAY)
    {
        string s = zendTypeToString(Z_TYPE_P(zv));
        zend_error(E_ERROR, "%s(): expected array value but received %s", get_active_function_name(TSRMLS_C),
                   s.c_str());
        return false;
    }

    HashTable* arr = Z_ARRVAL_P(zv);
    HashPosition pos;
    zval** val;

    os->writeSize(zend_hash_num_elements(arr));

    zend_hash_internal_pointer_reset_ex(arr, &pos);
    while(zend_hash_get_current_data_ex(arr, (void**)&val, &pos) != FAILURE)
    {
        if(!_elementMarshaler->marshal(*val, os, m TSRMLS_CC))
        {
            return false;
        }
        zend_hash_move_forward_ex(arr, &pos);
    }

    return true;
}

bool
IcePHP::SequenceMarshaler::unmarshal(zval* zv, const Ice::InputStreamPtr& is TSRMLS_DC)
{
    array_init(zv);

    Ice::Int sz = is->readSize();

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
IcePHP::SequenceMarshaler::destroy()
{
    _elementMarshaler->destroy();
    _elementMarshaler = 0;
}

//
// ProxyMarshaler implementation.
//
IcePHP::ProxyMarshaler::ProxyMarshaler(const Slice::ProxyPtr& type) :
    _type(type)
{
}

bool
IcePHP::ProxyMarshaler::marshal(zval* zv, const Ice::OutputStreamPtr& os, ObjectMap& TSRMLS_DC)
{
    if(Z_TYPE_P(zv) != IS_OBJECT && Z_TYPE_P(zv) != IS_NULL)
    {
        string s = zendTypeToString(Z_TYPE_P(zv));
        zend_error(E_ERROR, "%s(): expected proxy value but received %s", get_active_function_name(TSRMLS_C),
                   s.c_str());
        return false;
    }

    Ice::ObjectPrx proxy;
    Slice::ClassDefPtr def;
    if(!ZVAL_IS_NULL(zv))
    {
        if(!fetchProxy(zv, proxy, def TSRMLS_CC))
        {
            return false;
        }

        if(_type)
        {
            string scoped = _type->_class()->scoped();
            if(def)
            {
                if(!def->isA(scoped))
                {
                    string s = def->scoped();
                    zend_error(E_ERROR, "%s(): expected a proxy of type %s but received %s",
                               get_active_function_name(TSRMLS_C), scoped.c_str(), s.c_str());
                    return false;
                }
            }
            else
            {
                zend_error(E_ERROR, "%s(): expected a proxy of type %s", get_active_function_name(TSRMLS_C),
                           scoped.c_str());
                return false;
            }
        }
    }
    os->writeProxy(proxy);

    return true;
}

bool
IcePHP::ProxyMarshaler::unmarshal(zval* zv, const Ice::InputStreamPtr& is TSRMLS_DC)
{
    Ice::ObjectPrx proxy = is->readProxy();

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
    Slice::ClassDefPtr def;
    if(_type)
    {
        def = _type->_class()->definition();
    }

    if(!createProxy(zv, proxy, def TSRMLS_CC))
    {
        return false;
    }

    return true;
}

void
IcePHP::ProxyMarshaler::destroy()
{
}

//
// MemberMarshaler implementation.
//
IcePHP::MemberMarshaler::MemberMarshaler(const string& name, const MarshalerPtr& marshaler) :
    _name(name), _marshaler(marshaler)
{
}

bool
IcePHP::MemberMarshaler::marshal(zval* zv, const Ice::OutputStreamPtr& os, ObjectMap& m TSRMLS_DC)
{
    zval** val;
    if(zend_hash_find(Z_OBJPROP_P(zv), const_cast<char*>(_name.c_str()), _name.length() + 1, (void**)&val) == FAILURE)
    {
        zend_error(E_ERROR, "%s(): member `%s' is not defined", get_active_function_name(TSRMLS_C), _name.c_str());
        return false;
    }

    return _marshaler->marshal(*val, os, m TSRMLS_CC);;
}

bool
IcePHP::MemberMarshaler::unmarshal(zval* zv, const Ice::InputStreamPtr& is TSRMLS_DC)
{
    zval* val;
    MAKE_STD_ZVAL(val);

    if(!_marshaler->unmarshal(val, is TSRMLS_CC))
    {
        return false;
    }

    if(add_property_zval(zv, const_cast<char*>(_name.c_str()), val) == FAILURE)
    {
        zend_error(E_ERROR, "%s(): unable to set member `%s'", get_active_function_name(TSRMLS_C), _name.c_str());
        return false;
    }
    zval_ptr_dtor(&val); // add_property_zval increments the refcount

    return true;
}

void
IcePHP::MemberMarshaler::destroy()
{
    _marshaler->destroy();
    _marshaler = 0;
}

//
// StructMarshaler implementation.
//
IcePHP::StructMarshaler::StructMarshaler(const Slice::StructPtr& type TSRMLS_DC) :
    _type(type)
{
    _class = findClassScoped(type->scoped() TSRMLS_CC);
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
IcePHP::StructMarshaler::marshal(zval* zv, const Ice::OutputStreamPtr& os, ObjectMap& m TSRMLS_DC)
{
    if(Z_TYPE_P(zv) != IS_OBJECT)
    {
        string s = zendTypeToString(Z_TYPE_P(zv));
        zend_error(E_ERROR, "%s(): expected struct value of type %s but received %s",
                   get_active_function_name(TSRMLS_C), _class->name, s.c_str());
        return false;
    }

    //
    // Compare class entries.
    //
    zend_class_entry* ce = Z_OBJCE_P(zv);
    if(ce != _class)
    {
        zend_error(E_ERROR, "%s(): expected struct value of type %s but received %s",
                   get_active_function_name(TSRMLS_C), _class->name, ce->name);
        return false;
    }

    for(vector<MarshalerPtr>::iterator p = _members.begin(); p != _members.end(); ++p)
    {
        if(!(*p)->marshal(zv, os, m TSRMLS_CC))
        {
            return false;
        }
    }

    return true;
}

bool
IcePHP::StructMarshaler::unmarshal(zval* zv, const Ice::InputStreamPtr& is TSRMLS_DC)
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
IcePHP::StructMarshaler::destroy()
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
IcePHP::EnumMarshaler::EnumMarshaler(const Slice::EnumPtr& type TSRMLS_DC)
{
    _class = findClassScoped(type->scoped() TSRMLS_CC);
    assert(_class);
    _count = static_cast<long>(type->getEnumerators().size());
}

bool
IcePHP::EnumMarshaler::marshal(zval* zv, const Ice::OutputStreamPtr& os, ObjectMap& TSRMLS_DC)
{
    if(Z_TYPE_P(zv) != IS_LONG)
    {
        string s = zendTypeToString(Z_TYPE_P(zv));
        zend_error(E_ERROR, "%s(): expected long value for enum %s but received %s",
                   get_active_function_name(TSRMLS_C), _class->name, s.c_str());
        return false;
    }

    //
    // Validate value.
    //
    long val = Z_LVAL_P(zv);
    if(val < 0 || val >= _count)
    {
        zend_error(E_ERROR, "%s(): value %ld is out of range for enum %s", get_active_function_name(TSRMLS_C),
                   val, _class->name);
        return false;
    }

    if(_count <= 127)
    {
        os->writeByte(static_cast<Ice::Byte>(val));
    }
    else if(_count <= 32767)
    {
        os->writeShort(static_cast<Ice::Short>(val));
    }
    else
    {
        os->writeInt(static_cast<Ice::Int>(val));
    }

    return true;
}

bool
IcePHP::EnumMarshaler::unmarshal(zval* zv, const Ice::InputStreamPtr& is TSRMLS_DC)
{
    if(_count <= 127)
    {
        Ice::Byte val = is->readByte();
        ZVAL_LONG(zv, val);
    }
    else if(_count <= 32767)
    {
        Ice::Short val = is->readShort();
        ZVAL_LONG(zv, val);
    }
    else
    {
        Ice::Int val = is->readInt();
        ZVAL_LONG(zv, val);
    }

    return true;
}

void
IcePHP::EnumMarshaler::destroy()
{
}

//
// NativeDictionaryMarshaler implementation.
//
IcePHP::NativeDictionaryMarshaler::NativeDictionaryMarshaler(const Slice::TypePtr& keyType,
                                                             const Slice::TypePtr& valueType TSRMLS_DC)
{
    Slice::BuiltinPtr b = Slice::BuiltinPtr::dynamicCast(keyType);
    assert(b);
    _keyKind = b->kind();
    _keyMarshaler = createMarshaler(keyType TSRMLS_CC);
    _valueMarshaler = createMarshaler(valueType TSRMLS_CC);
}

bool
IcePHP::NativeDictionaryMarshaler::marshal(zval* zv, const Ice::OutputStreamPtr& os, ObjectMap& m TSRMLS_DC)
{
    if(Z_TYPE_P(zv) != IS_ARRAY)
    {
        string s = zendTypeToString(Z_TYPE_P(zv));
        zend_error(E_ERROR, "%s: expected array value but received %s", get_active_function_name(TSRMLS_C), s.c_str());
        return false;
    }

    HashTable* arr = Z_ARRVAL_P(zv);
    HashPosition pos;
    zval** val;

    os->writeSize(zend_hash_num_elements(arr));

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
        if(!_keyMarshaler->marshal(&zkey, os, m TSRMLS_CC))
        {
            zval_dtor(&zkey);
            return false;
        }

        zval_dtor(&zkey);

        //
        // Marshal the value.
        //
        if(!_valueMarshaler->marshal(*val, os, m TSRMLS_CC))
        {
            return false;
        }

        zend_hash_move_forward_ex(arr, &pos);
    }

    return true;
}

bool
IcePHP::NativeDictionaryMarshaler::unmarshal(zval* zv, const Ice::InputStreamPtr& is TSRMLS_DC)
{
    array_init(zv);

    Ice::Int sz = is->readSize();

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
IcePHP::NativeDictionaryMarshaler::destroy()
{
    _keyMarshaler->destroy();
    _keyMarshaler = 0;
    _valueMarshaler->destroy();
    _valueMarshaler = 0;
}

//
// ExceptionMarshaler implementation.
//
IcePHP::ExceptionMarshaler::ExceptionMarshaler(const Slice::ExceptionPtr& ex TSRMLS_DC) :
    _ex(ex)
{
    _class = findClassScoped(ex->scoped() TSRMLS_CC);
    assert(_class);
}

bool
IcePHP::ExceptionMarshaler::marshal(zval*, const Ice::OutputStreamPtr&, ObjectMap& TSRMLS_DC)
{
    //
    // We never need to marshal an exception.
    //
    zend_error(E_ERROR, "exception marshaling is not supported");
    return false;
}

bool
IcePHP::ExceptionMarshaler::unmarshal(zval* zv, const Ice::InputStreamPtr& is TSRMLS_DC)
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
        is->startSlice();
        for(Slice::DataMemberList::iterator p = members.begin(); p != members.end(); ++p)
        {
            MarshalerPtr member = createMemberMarshaler((*p)->name(), (*p)->type() TSRMLS_CC);
            if(!member->unmarshal(zv, is TSRMLS_CC))
            {
                return false;
            }
        }
        is->endSlice();
        ex = ex->base();
        if(ex)
        {
            is->readString(); // Skip id.
        }
    }

    return true;
}

void
IcePHP::ExceptionMarshaler::destroy()
{
}

//
// IceObjectSliceMarshaler implementation.
//
IcePHP::IceObjectSliceMarshaler::IceObjectSliceMarshaler(TSRMLS_D)
{
}

bool
IcePHP::IceObjectSliceMarshaler::marshal(zval* zv, const Ice::OutputStreamPtr& os, ObjectMap& TSRMLS_DC)
{
    assert(Z_TYPE_P(zv) == IS_OBJECT);

    os->writeTypeId(Ice::Object::ice_staticId());
    os->startSlice();
    os->writeSize(0); // For compatibility with the old AFM.
    os->endSlice();

    return true;
}

bool
IcePHP::IceObjectSliceMarshaler::unmarshal(zval* zv, const Ice::InputStreamPtr& is TSRMLS_DC)
{
    assert(Z_TYPE_P(zv) == IS_OBJECT);

    //
    // Do not read type id here - see ObjectReader::__read().
    //
    //is->readTypeId()

    is->startSlice();

    // For compatibility with the old AFM.
    Ice::Int sz = is->readSize();
    if(sz != 0)
    {
        throw Ice::MarshalException(__FILE__, __LINE__);
    }

    is->endSlice();

    return true;
}

void
IcePHP::IceObjectSliceMarshaler::destroy()
{
}

//
// ObjectSliceMarshaler implementation.
//
IcePHP::ObjectSliceMarshaler::ObjectSliceMarshaler(const string& scoped,
                                                   const Slice::DataMemberList& members TSRMLS_DC) :
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
IcePHP::ObjectSliceMarshaler::marshal(zval* zv, const Ice::OutputStreamPtr& os, ObjectMap& m TSRMLS_DC)
{
    assert(Z_TYPE_P(zv) == IS_OBJECT);

    os->writeTypeId(_scoped);
    os->startSlice();
    for(vector<MarshalerPtr>::iterator p = _members.begin(); p != _members.end(); ++p)
    {
        if(!(*p)->marshal(zv, os, m TSRMLS_CC))
        {
            return false;
        }
    }
    os->endSlice();

    return true;
}

bool
IcePHP::ObjectSliceMarshaler::unmarshal(zval* zv, const Ice::InputStreamPtr& is TSRMLS_DC)
{
    assert(Z_TYPE_P(zv) == IS_OBJECT);

    //
    // Do not read type id here - see ObjectReader::__read().
    //
    //is->readTypeId()

    is->startSlice();
    for(vector<MarshalerPtr>::iterator p = _members.begin(); p != _members.end(); ++p)
    {
        if(!(*p)->unmarshal(zv, is TSRMLS_CC))
        {
            return false;
        }
    }
    is->endSlice();

    return true;
}

void
IcePHP::ObjectSliceMarshaler::destroy()
{
    vector<MarshalerPtr> members = _members;
    _members.clear();
    for(vector<MarshalerPtr>::iterator p = members.begin(); p != members.end(); ++p)
    {
        (*p)->destroy();
    }
}

//
// ObjectWriter implementation.
//
IcePHP::ObjectWriter::ObjectWriter(zval* value, const Slice::SyntaxTreeBasePtr& type, ObjectMap& m TSRMLS_DC) :
    _value(value), _map(m)
{
#if defined(__SUNPRO_CC) && (__SUNPRO_CC <= 0x530)
// Strange Sun C++ 5.3 bug.
    const IceUtil::HandleBase<Slice::SyntaxTreeBase>& hb = type;
    _type = Slice::ClassDefPtr::dynamicCast(hb);
#else
    _type = Slice::ClassDefPtr::dynamicCast(type);
#endif

#ifdef ZTS
    this->TSRMLS_C = TSRMLS_C;
#endif

    Z_OBJ_HT_P(_value)->add_ref(_value TSRMLS_CC);
}

IcePHP::ObjectWriter::~ObjectWriter()
{
    Z_OBJ_HT_P(_value)->del_ref(_value TSRMLS_CC);
}

void
IcePHP::ObjectWriter::ice_preMarshal()
{
    zend_call_method_with_0_params(&_value, NULL, NULL, "ice_preMarshal", NULL);
}

void
IcePHP::ObjectWriter::write(const Ice::OutputStreamPtr& os) const
{
    MarshalerMap* marshalerMap = static_cast<MarshalerMap*>(ICE_G(marshalerMap));
    ObjectMap& objectMap = const_cast<ObjectMap&>(_map);
    zval* value = const_cast<zval*>(_value);

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

        if(!slice->marshal(value, os, objectMap TSRMLS_CC))
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

    if(!slice->marshal(value, os, objectMap TSRMLS_CC))
    {
        throw AbortMarshaling();
    }
}

//
// ReadObjectCallback implementation.
//
void
IcePHP::ReadObjectCallback::invoke(const Ice::ObjectPtr& v)
{
    ObjectReaderPtr p = ObjectReaderPtr::dynamicCast(v);
    if(p)
    {
        p->setValue(ce, scoped, zv);
    }
    else
    {
        ZVAL_NULL(zv);
    }
}

//
// ObjectReader implementation.
//
IcePHP::ObjectReader::ObjectReader(zval* val, const Slice::ClassDefPtr& type TSRMLS_DC) :
    _value(val), _type(type)
{
#ifdef ZTS
    this->TSRMLS_C = TSRMLS_C;
#endif

    ZVAL_ADDREF(_value);

    _class = Z_OBJCE_P(_value);
}

IcePHP::ObjectReader::~ObjectReader()
{
    zval_ptr_dtor(&_value);
}

void
IcePHP::ObjectReader::ice_postUnmarshal()
{
    zend_call_method_with_0_params(&_value, NULL, NULL, "ice_postUnmarshal", NULL);
}

void
IcePHP::ObjectReader::read(const Ice::InputStreamPtr& is, bool rid)
{
    MarshalerMap* marshalerMap = static_cast<MarshalerMap*>(ICE_G(marshalerMap));

    //
    // Unmarshal the slices of a user-defined class.
    //
    if(_type)
    {
        Slice::ClassDefPtr def = _type;
        while(true)
        {
            string scoped;
            if(rid)
            {
                scoped = is->readTypeId();
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

            if(!slice->unmarshal(_value, is TSRMLS_CC))
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
        is->readTypeId();
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

    if(!slice->unmarshal(_value, is TSRMLS_CC))
    {
        throw AbortMarshaling();
    }
}

void
IcePHP::ObjectReader::setValue(zend_class_entry* ce, const string& scoped, zval* zv)
{
    //
    // Compare the class entries. The argument "ce" represents the formal type.
    //
    if(!checkClass(_class, ce))
    {
        Ice::NoObjectFactoryException ex(__FILE__, __LINE__);
        ex.type = scoped;
        throw ex;
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
IcePHP::ObjectMarshaler::ObjectMarshaler(const Slice::ClassDefPtr& def TSRMLS_DC) :
    _def(def)
{
    //
    // Find the class entry for this type.
    //
    if(def)
    {
        _scoped = def->scoped();
        _class = findClassScoped(_scoped TSRMLS_CC);
    }
    else
    {
        _scoped = "::Ice::Object";
        _class = findClass("Ice_ObjectImpl" TSRMLS_CC);
    }

    assert(_class);
}

bool
IcePHP::ObjectMarshaler::marshal(zval* zv, const Ice::OutputStreamPtr& os, ObjectMap& m TSRMLS_DC)
{
    if(Z_TYPE_P(zv) == IS_NULL)
    {
        os->writeObject(0);
        return true;
    }

    if(Z_TYPE_P(zv) != IS_OBJECT)
    {
        string s = zendTypeToString(Z_TYPE_P(zv));
        zend_error(E_ERROR, "%s(): expected object value of type %s but received %s",
                   get_active_function_name(TSRMLS_C), _class ? _class->name : "ice_object",
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
            zend_error(E_ERROR, "%s(): expected object value of type %s but received %s",
                       get_active_function_name(TSRMLS_C), _class->name, ce->name);
            return false;
        }
    }

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
    Profile* profile = static_cast<Profile*>(ICE_G(profile));
    assert(profile);
    Slice::ClassDefPtr def;
    Profile::ClassMap::iterator p = profile->classes.find(ce->name);
    if(p != profile->classes.end())
    {
        def = p->second;
    }

    ObjectMap::iterator q = m.find(Z_OBJ_HANDLE_P(zv));
    if(q == m.end())
    {
        writer = new ObjectWriter(zv, def, m TSRMLS_CC);
        m.insert(pair<unsigned int, Ice::ObjectPtr>(Z_OBJ_HANDLE_P(zv), writer));
    }
    else
    {
        writer = q->second;
    }

    //
    // Give the writer to the stream. The stream will eventually call write() on it.
    //
    os->writeObject(writer);

    return true;
}

bool
IcePHP::ObjectMarshaler::unmarshal(zval* zv, const Ice::InputStreamPtr& is TSRMLS_DC)
{
    ReadObjectCallbackPtr cb = new ReadObjectCallback;
    cb->ce = _class;
    cb->scoped = _scoped;
    cb->zv = zv;

    //
    // Invoke readObject(), passing our callback object. When the object is eventually unmarshaled,
    // our callback will be invoked and we will assign a value to zv.
    //
    is->readObject(cb);

    return true;
}

void
IcePHP::ObjectMarshaler::destroy()
{
}

//
// PHPObjectFactory implementation.
//
IcePHP::PHPObjectFactory::PHPObjectFactory(TSRMLS_D)
{
#ifdef ZTS
    this->TSRMLS_C = TSRMLS_C;
#endif
}

Ice::ObjectPtr
IcePHP::PHPObjectFactory::create(const string& scoped)
{
    Ice::ObjectPtr result;

    Profile* profile = static_cast<Profile*>(ICE_G(profile));
    assert(profile);

    ObjectFactoryMap* ofm = static_cast<ObjectFactoryMap*>(ICE_G(objectFactoryMap));
    assert(ofm);

    //
    // First check our map for a factory registered for this type.
    //
    ObjectFactoryMap::iterator p;
    p = ofm->find(scoped);
    if(p == ofm->end())
    {
        //
        // Next, check for a default factory.
        //
        p = ofm->find("");
    }

    //
    // If we found a factory, invoke create() on the object.
    //
    if(p != ofm->end())
    {
        zval* id;
        MAKE_STD_ZVAL(id);
        ZVAL_STRINGL(id, const_cast<char*>(scoped.c_str()), scoped.length(), 1);

        zval* zresult = NULL;

        zend_call_method_with_1_params(&p->second, NULL, NULL, "create", &zresult, id);

        zval_ptr_dtor(&id);

        AutoDestroy destroyResult(zresult);

        //
        // Bail out if an exception has been thrown.
        //
        if(EG(exception))
        {
            throw AbortMarshaling();
        }

        if(zresult)
        {
            //
            // If the factory returned a non-null value, verify that it is an object, and that it
            // inherits from Ice_ObjectImpl.
            //
            if(!ZVAL_IS_NULL(zresult))
            {
                if(Z_TYPE_P(zresult) != IS_OBJECT)
                {
                    zend_error(E_ERROR, "object factory did not return an object");
                    throw AbortMarshaling();
                }

                zend_class_entry* ce = Z_OBJCE_P(zresult);
                zend_class_entry* base = findClass("Ice_ObjectImpl" TSRMLS_CC);
                if(!checkClass(ce, base))
                {
                    zend_error(E_ERROR, "object returned by factory does not implement Ice_ObjectImpl");
                    throw AbortMarshaling();
                }

                //
                // Attempt to find a class definition for the object.
                //
                Profile::ClassMap::iterator p;
                while(ce != NULL && (p = profile->classes.find(ce->name)) == profile->classes.end())
                {
                    ce = ce->parent;
                }

                Slice::ClassDefPtr def;
                if(ce != NULL)
                {
                    assert(p != profile->classes.end());
                    def = p->second;
                }

                return new ObjectReader(zresult, def TSRMLS_CC);
            }
        }
    }

    //
    // Attempt to find a class entry for the given type id. If no class entry is
    // found, or the class is abstract, then we return nil and the stream will skip
    // the slice and try again.
    //
    zend_class_entry* cls = NULL;
    Slice::ClassDefPtr def;
    if(scoped == Ice::Object::ice_staticId())
    {
        cls = findClass("Ice_ObjectImpl" TSRMLS_CC);
    }
    else
    {
        cls = findClassScoped(scoped TSRMLS_CC);
    }

    //
    // Instantiate the class if it's not abstract.
    //
    const int abstractFlags = ZEND_ACC_INTERFACE | ZEND_ACC_EXPLICIT_ABSTRACT_CLASS;
    if(cls && (cls->ce_flags & abstractFlags) == 0)
    {
        Profile::ClassMap::iterator p = profile->classes.find(cls->name);
        if(p != profile->classes.end())
        {
            def = p->second;
        }
        zval* obj;
        MAKE_STD_ZVAL(obj);
        object_init_ex(obj, cls);
        result = new ObjectReader(obj, def TSRMLS_CC);
        zval_ptr_dtor(&obj);
    }

    return result;
}

void
IcePHP::PHPObjectFactory::destroy()
{
}
