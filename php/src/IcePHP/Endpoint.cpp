// **********************************************************************
//
// Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Endpoint.h>
#include <Util.h>

using namespace std;
using namespace IcePHP;

ZEND_EXTERN_MODULE_GLOBALS(ice)

//
// Class entries represent the PHP class implementations we have registered.
//
namespace IcePHP
{
zend_class_entry* endpointClassEntry = 0;
}

static zend_class_entry* tcpEndpointClassEntry = 0;
static zend_class_entry* udpEndpointClassEntry = 0;
static zend_class_entry* opaqueEndpointClassEntry = 0;

//
// Ice::Endpoint support.
//
static zend_object_handlers _handlers;

extern "C"
{
static zend_object_value handleAlloc(zend_class_entry* TSRMLS_DC);
static void handleFreeStorage(void* TSRMLS_DC);
}

ZEND_METHOD(Ice_Endpoint, __construct)
{
    runtimeError("Endpoint cannot be instantiated" TSRMLS_CC);
}

ZEND_METHOD(Ice_Endpoint, __toString)
{
    if(ZEND_NUM_ARGS() > 0)
    {
        WRONG_PARAM_COUNT;
    }

    Ice::EndpointPtr _this = Wrapper<Ice::EndpointPtr>::value(getThis() TSRMLS_CC);
    assert(_this);

    try
    {
        string str = _this->toString();
        RETURN_STRINGL(STRCAST(str.c_str()), str.length(), 1);
    }
    catch(const IceUtil::Exception& ex)
    {
        throwException(ex TSRMLS_CC);
        RETURN_NULL();
    }
}

ZEND_METHOD(Ice_Endpoint, toString)
{
    ZEND_MN(Ice_Endpoint___toString)(INTERNAL_FUNCTION_PARAM_PASSTHRU);
}

ZEND_METHOD(Ice_Endpoint, timeout)
{
    if(ZEND_NUM_ARGS() > 0)
    {
        WRONG_PARAM_COUNT;
    }

    Ice::EndpointPtr _this = Wrapper<Ice::EndpointPtr>::value(getThis() TSRMLS_CC);
    assert(_this);

    try
    {
        long timeout = static_cast<long>(_this->timeout());
        RETURN_LONG(timeout);
    }
    catch(const IceUtil::Exception& ex)
    {
        throwException(ex TSRMLS_CC);
        RETURN_NULL();
    }
}

ZEND_METHOD(Ice_Endpoint, compress)
{
    if(ZEND_NUM_ARGS() > 0)
    {
        WRONG_PARAM_COUNT;
    }

    Ice::EndpointPtr _this = Wrapper<Ice::EndpointPtr>::value(getThis() TSRMLS_CC);
    assert(_this);

    try
    {
        RETURN_BOOL(_this->timeout() ? 1 : 0);
    }
    catch(const IceUtil::Exception& ex)
    {
        throwException(ex TSRMLS_CC);
        RETURN_NULL();
    }
}

ZEND_METHOD(Ice_TcpEndpoint, host)
{
    if(ZEND_NUM_ARGS() > 0)
    {
        WRONG_PARAM_COUNT;
    }

    Ice::EndpointPtr base = Wrapper<Ice::EndpointPtr>::value(getThis() TSRMLS_CC);
    assert(base);
    Ice::TcpEndpointPtr _this = Ice::TcpEndpointPtr::dynamicCast(base);
    assert(_this);

    try
    {
        string str = _this->host();
        RETURN_STRINGL(STRCAST(str.c_str()), str.length(), 1);
    }
    catch(const IceUtil::Exception& ex)
    {
        throwException(ex TSRMLS_CC);
        RETURN_NULL();
    }
}

ZEND_METHOD(Ice_TcpEndpoint, port)
{
    if(ZEND_NUM_ARGS() > 0)
    {
        WRONG_PARAM_COUNT;
    }

    Ice::EndpointPtr base = Wrapper<Ice::EndpointPtr>::value(getThis() TSRMLS_CC);
    assert(base);
    Ice::TcpEndpointPtr _this = Ice::TcpEndpointPtr::dynamicCast(base);
    assert(_this);

    try
    {
        long port = static_cast<long>(_this->port());
        RETURN_LONG(port);
    }
    catch(const IceUtil::Exception& ex)
    {
        throwException(ex TSRMLS_CC);
        RETURN_NULL();
    }
}

ZEND_METHOD(Ice_UdpEndpoint, host)
{
    if(ZEND_NUM_ARGS() > 0)
    {
        WRONG_PARAM_COUNT;
    }

    Ice::EndpointPtr base = Wrapper<Ice::EndpointPtr>::value(getThis() TSRMLS_CC);
    assert(base);
    Ice::UdpEndpointPtr _this = Ice::UdpEndpointPtr::dynamicCast(base);
    assert(_this);

    try
    {
        string str = _this->host();
        RETURN_STRINGL(STRCAST(str.c_str()), str.length(), 1);
    }
    catch(const IceUtil::Exception& ex)
    {
        throwException(ex TSRMLS_CC);
        RETURN_NULL();
    }
}

ZEND_METHOD(Ice_UdpEndpoint, port)
{
    if(ZEND_NUM_ARGS() > 0)
    {
        WRONG_PARAM_COUNT;
    }

    Ice::EndpointPtr base = Wrapper<Ice::EndpointPtr>::value(getThis() TSRMLS_CC);
    assert(base);
    Ice::UdpEndpointPtr _this = Ice::UdpEndpointPtr::dynamicCast(base);
    assert(_this);

    try
    {
        long port = static_cast<long>(_this->port());
        RETURN_LONG(port);
    }
    catch(const IceUtil::Exception& ex)
    {
        throwException(ex TSRMLS_CC);
        RETURN_NULL();
    }
}

ZEND_METHOD(Ice_UdpEndpoint, mcastInterface)
{
    if(ZEND_NUM_ARGS() > 0)
    {
        WRONG_PARAM_COUNT;
    }

    Ice::EndpointPtr base = Wrapper<Ice::EndpointPtr>::value(getThis() TSRMLS_CC);
    assert(base);
    Ice::UdpEndpointPtr _this = Ice::UdpEndpointPtr::dynamicCast(base);
    assert(_this);

    try
    {
        string str = _this->mcastInterface();
        RETURN_STRINGL(STRCAST(str.c_str()), str.length(), 1);
    }
    catch(const IceUtil::Exception& ex)
    {
        throwException(ex TSRMLS_CC);
        RETURN_NULL();
    }
}

ZEND_METHOD(Ice_UdpEndpoint, mcastTtl)
{
    if(ZEND_NUM_ARGS() > 0)
    {
        WRONG_PARAM_COUNT;
    }

    Ice::EndpointPtr base = Wrapper<Ice::EndpointPtr>::value(getThis() TSRMLS_CC);
    assert(base);
    Ice::UdpEndpointPtr _this = Ice::UdpEndpointPtr::dynamicCast(base);
    assert(_this);

    try
    {
        long port = static_cast<long>(_this->mcastTtl());
        RETURN_LONG(port);
    }
    catch(const IceUtil::Exception& ex)
    {
        throwException(ex TSRMLS_CC);
        RETURN_NULL();
    }
}

ZEND_METHOD(Ice_OpaqueEndpoint, rawBytes)
{
    if(ZEND_NUM_ARGS() > 0)
    {
        WRONG_PARAM_COUNT;
    }

    Ice::EndpointPtr base = Wrapper<Ice::EndpointPtr>::value(getThis() TSRMLS_CC);
    assert(base);
    Ice::OpaqueEndpointPtr _this = Ice::OpaqueEndpointPtr::dynamicCast(base);
    assert(_this);

    try
    {
        Ice::ByteSeq seq = _this->rawBytes();
        array_init(return_value);
        for(Ice::ByteSeq::iterator p = seq.begin(); p != seq.end(); ++p)
        {
            add_next_index_long(return_value, static_cast<long>(*p));
        }
    }
    catch(const IceUtil::Exception& ex)
    {
        throwException(ex TSRMLS_CC);
        RETURN_NULL();
    }
}

#ifdef _WIN32
extern "C"
#endif
static zend_object_value
handleAlloc(zend_class_entry* ce TSRMLS_DC)
{
    zend_object_value result;

    Wrapper<Ice::EndpointPtr>* obj = Wrapper<Ice::EndpointPtr>::create(ce TSRMLS_CC);
    assert(obj);

    result.handle = zend_objects_store_put(obj, 0, (zend_objects_free_object_storage_t)handleFreeStorage, 0 TSRMLS_CC);
    result.handlers = &_handlers;

    return result;
}

#ifdef _WIN32
extern "C"
#endif
static void
handleFreeStorage(void* p TSRMLS_DC)
{
    Wrapper<Ice::EndpointPtr>* obj = static_cast<Wrapper<Ice::EndpointPtr>*>(p);
    delete obj->ptr;
    zend_objects_free_object_storage(static_cast<zend_object*>(p) TSRMLS_CC);
}

//
// Predefined methods for Endpoint.
//
static function_entry _interfaceMethods[] =
{
    {0, 0, 0}
};
static function_entry _endpointMethods[] =
{
    ZEND_ME(Ice_Endpoint, __construct, NULL, ZEND_ACC_PRIVATE|ZEND_ACC_CTOR)
    ZEND_ME(Ice_Endpoint, __toString, NULL, ZEND_ACC_PUBLIC)
    ZEND_ME(Ice_Endpoint, toString, NULL, ZEND_ACC_PUBLIC)
    ZEND_ME(Ice_Endpoint, timeout, NULL, ZEND_ACC_PUBLIC)
    ZEND_ME(Ice_Endpoint, compress, NULL, ZEND_ACC_PUBLIC)
    {0, 0, 0}
};
static function_entry _tcpEndpointMethods[] =
{
    ZEND_ME(Ice_TcpEndpoint, host, NULL, ZEND_ACC_PUBLIC)
    ZEND_ME(Ice_TcpEndpoint, port, NULL, ZEND_ACC_PUBLIC)
    {0, 0, 0}
};
static function_entry _udpEndpointMethods[] =
{
    ZEND_ME(Ice_UdpEndpoint, host, NULL, ZEND_ACC_PUBLIC)
    ZEND_ME(Ice_UdpEndpoint, port, NULL, ZEND_ACC_PUBLIC)
    ZEND_ME(Ice_UdpEndpoint, mcastInterface, NULL, ZEND_ACC_PUBLIC)
    ZEND_ME(Ice_UdpEndpoint, mcastTtl, NULL, ZEND_ACC_PUBLIC)
    {0, 0, 0}
};
static function_entry _opaqueEndpointMethods[] =
{
    ZEND_ME(Ice_OpaqueEndpoint, rawBytes, NULL, ZEND_ACC_PUBLIC)
    {0, 0, 0}
};

bool
IcePHP::endpointInit(TSRMLS_D)
{
    //
    // We register an interface and a class that implements the interface. This allows
    // applications to safely include the Slice-generated code for the type.
    //

    //
    // Register the Endpoint interface.
    //
    zend_class_entry ce;
#ifdef ICEPHP_USE_NAMESPACES
    INIT_NS_CLASS_ENTRY(ce, STRCAST("Ice"), STRCAST("Endpoint"), _interfaceMethods);
#else
    INIT_CLASS_ENTRY(ce, "Ice_Endpoint", _interfaceMethods);
#endif
    zend_class_entry* endpointInterface = zend_register_internal_interface(&ce TSRMLS_CC);

    //
    // Register the TcpEndpoint interface.
    //
#ifdef ICEPHP_USE_NAMESPACES
    INIT_NS_CLASS_ENTRY(ce, STRCAST("Ice"), STRCAST("TcpEndpoint"), _interfaceMethods);
#else
    INIT_CLASS_ENTRY(ce, "Ice_TcpEndpoint", _interfaceMethods);
#endif
    zend_class_entry* tcpEndpointInterface = zend_register_internal_interface(&ce TSRMLS_CC);
    zend_class_implements(tcpEndpointInterface TSRMLS_CC, 1, endpointInterface);

    //
    // Register the UdpEndpoint interface.
    //
#ifdef ICEPHP_USE_NAMESPACES
    INIT_NS_CLASS_ENTRY(ce, STRCAST("Ice"), STRCAST("UdpEndpoint"), _interfaceMethods);
#else
    INIT_CLASS_ENTRY(ce, "Ice_UdpEndpoint", _interfaceMethods);
#endif
    zend_class_entry* udpEndpointInterface = zend_register_internal_interface(&ce TSRMLS_CC);
    zend_class_implements(udpEndpointInterface TSRMLS_CC, 1, endpointInterface);

    //
    // Register the OpaqueEndpoint interface.
    //
#ifdef ICEPHP_USE_NAMESPACES
    INIT_NS_CLASS_ENTRY(ce, STRCAST("Ice"), STRCAST("OpaqueEndpoint"), _interfaceMethods);
#else
    INIT_CLASS_ENTRY(ce, "Ice_OpaqueEndpoint", _interfaceMethods);
#endif
    zend_class_entry* opaqueEndpointInterface = zend_register_internal_interface(&ce TSRMLS_CC);
    zend_class_implements(opaqueEndpointInterface TSRMLS_CC, 1, endpointInterface);

    //
    // Register the Endpoint class.
    //
    INIT_CLASS_ENTRY(ce, "IcePHP_Endpoint", _endpointMethods);
    ce.create_object = handleAlloc;
    endpointClassEntry = zend_register_internal_class(&ce TSRMLS_CC);
    memcpy(&_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
    zend_class_implements(endpointClassEntry TSRMLS_CC, 1, endpointInterface);

    //
    // Register the TcpEndpoint class.
    //
    INIT_CLASS_ENTRY(ce, "IcePHP_TcpEndpoint", _tcpEndpointMethods);
    ce.create_object = handleAlloc;
    tcpEndpointClassEntry = zend_register_internal_class_ex(&ce, endpointClassEntry, NULL TSRMLS_CC);
    memcpy(&_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
    zend_class_implements(tcpEndpointClassEntry TSRMLS_CC, 1, tcpEndpointInterface);

    //
    // Register the UdpEndpoint class.
    //
    INIT_CLASS_ENTRY(ce, "IcePHP_UdpEndpoint", _udpEndpointMethods);
    ce.create_object = handleAlloc;
    udpEndpointClassEntry = zend_register_internal_class_ex(&ce, endpointClassEntry, NULL TSRMLS_CC);
    memcpy(&_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
    zend_class_implements(udpEndpointClassEntry TSRMLS_CC, 1, udpEndpointInterface);

    //
    // Register the OpaqueEndpoint class.
    //
    INIT_CLASS_ENTRY(ce, "IcePHP_OpaqueEndpoint", _opaqueEndpointMethods);
    ce.create_object = handleAlloc;
    opaqueEndpointClassEntry = zend_register_internal_class_ex(&ce, endpointClassEntry, NULL TSRMLS_CC);
    memcpy(&_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
    zend_class_implements(opaqueEndpointClassEntry TSRMLS_CC, 1, opaqueEndpointInterface);

    return true;
}

bool
IcePHP::createEndpoint(zval* zv, const Ice::EndpointPtr& p TSRMLS_DC)
{
    zend_class_entry* ce;
    if(Ice::TcpEndpointPtr::dynamicCast(p))
    {
        ce = tcpEndpointClassEntry;
    }
    else if(Ice::UdpEndpointPtr::dynamicCast(p))
    {
        ce = udpEndpointClassEntry;
    }
    else if(Ice::OpaqueEndpointPtr::dynamicCast(p))
    {
        ce = opaqueEndpointClassEntry;
    }
    else
    {
        ce = endpointClassEntry;
    }

    if(object_init_ex(zv, ce) != SUCCESS)
    {
        runtimeError("unable to initialize endpoint" TSRMLS_CC);
        return false;
    }

    Wrapper<Ice::EndpointPtr>* obj = Wrapper<Ice::EndpointPtr>::extract(zv TSRMLS_CC);
    assert(obj);
    assert(!obj->ptr);
    obj->ptr = new Ice::EndpointPtr(p);

    return true;
}

bool
IcePHP::fetchEndpoint(zval* zv, Ice::EndpointPtr& endpoint TSRMLS_DC)
{
    if(ZVAL_IS_NULL(zv))
    {
        endpoint = 0;
    }
    else
    {
        if(Z_TYPE_P(zv) != IS_OBJECT || !checkClass(Z_OBJCE_P(zv), endpointClassEntry))
        {
            invalidArgument("value is not an endpoint" TSRMLS_CC);
            return false;
        }
        Wrapper<Ice::EndpointPtr>* obj = Wrapper<Ice::EndpointPtr>::extract(zv TSRMLS_CC);
        if(!obj)
        {
            return false;
        }
        endpoint = *obj->ptr;
    }
    return true;
}
