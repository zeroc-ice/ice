// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Endpoint.h>
#include <Util.h>
#include <IceSSL/EndpointInfo.h>

using namespace std;
using namespace IcePHP;

ZEND_EXTERN_MODULE_GLOBALS(ice)

//
// Class entries representing the PHP class implementations we have registered.
//
static zend_class_entry* endpointClassEntry = 0;

static zend_class_entry* endpointInfoClassEntry = 0;
static zend_class_entry* ipEndpointInfoClassEntry = 0;
static zend_class_entry* tcpEndpointInfoClassEntry = 0;
static zend_class_entry* udpEndpointInfoClassEntry = 0;
static zend_class_entry* wsEndpointInfoClassEntry = 0;
static zend_class_entry* opaqueEndpointInfoClassEntry = 0;
static zend_class_entry* sslEndpointInfoClassEntry = 0;

//
// Ice::Endpoint support.
//
static zend_object_handlers _endpointHandlers;
static zend_object_handlers _endpointInfoHandlers;

extern "C"
{
static zend_object* handleEndpointAlloc(zend_class_entry*);
static void handleEndpointFreeStorage(zend_object*);

static zend_object* handleEndpointInfoAlloc(zend_class_entry*);
static void handleEndpointInfoFreeStorage(zend_object*);
}

ZEND_METHOD(Ice_Endpoint, __construct)
{
    runtimeError("Endpoint cannot be instantiated");
}

ZEND_METHOD(Ice_Endpoint, __toString)
{
    if(ZEND_NUM_ARGS() > 0)
    {
        WRONG_PARAM_COUNT;
    }

    Ice::EndpointPtr _this = Wrapper<Ice::EndpointPtr>::value(getThis());
    assert(_this);

    try
    {
        string str = _this->toString();
        RETURN_STRINGL(STRCAST(str.c_str()), static_cast<int>(str.length()));
    }
    catch(const IceUtil::Exception& ex)
    {
        throwException(ex);
        RETURN_NULL();
    }
}

ZEND_METHOD(Ice_Endpoint, toString)
{
    ZEND_MN(Ice_Endpoint___toString)(INTERNAL_FUNCTION_PARAM_PASSTHRU);
}

ZEND_METHOD(Ice_Endpoint, getInfo)
{
    if(ZEND_NUM_ARGS() > 0)
    {
        WRONG_PARAM_COUNT;
    }

    Ice::EndpointPtr _this = Wrapper<Ice::EndpointPtr>::value(getThis());
    assert(_this);

    if(!createEndpointInfo(return_value, _this->getInfo()))
    {
        RETURN_NULL();
    }
}

#ifdef _WIN32
extern "C"
#endif
static zend_object*
handleEndpointAlloc(zend_class_entry* ce)
{
    Wrapper<Ice::EndpointPtr>* obj = Wrapper<Ice::EndpointPtr>::create(ce);
    assert(obj);

    obj->zobj.handlers = &_endpointHandlers;

    return &obj->zobj;
}

#ifdef _WIN32
extern "C"
#endif
static void
handleEndpointFreeStorage(zend_object* object)
{
    Wrapper<Ice::EndpointPtr>* obj = Wrapper<Ice::EndpointPtr>::fetch(object);
    delete obj->ptr;
    zend_object_std_dtor(object);
}

ZEND_METHOD(Ice_EndpointInfo, __construct)
{
    runtimeError("EndpointInfo cannot be instantiated");
}

ZEND_METHOD(Ice_EndpointInfo, type)
{
    if(ZEND_NUM_ARGS() > 0)
    {
        WRONG_PARAM_COUNT;
    }

    Ice::EndpointInfoPtr _this = Wrapper<Ice::EndpointInfoPtr>::value(getThis());
    assert(_this);

    try
    {
        short type = static_cast<short>(_this->type());
        RETURN_LONG(type);
    }
    catch(const IceUtil::Exception& ex)
    {
        throwException(ex);
        RETURN_NULL();
    }
}

ZEND_METHOD(Ice_EndpointInfo, datagram)
{
    if(ZEND_NUM_ARGS() > 0)
    {
        WRONG_PARAM_COUNT;
    }

    Ice::EndpointInfoPtr _this = Wrapper<Ice::EndpointInfoPtr>::value(getThis());
    assert(_this);

    try
    {
        RETURN_BOOL(_this->datagram() ? 1 : 0);
    }
    catch(const IceUtil::Exception& ex)
    {
        throwException(ex);
        RETURN_NULL();
    }
}

ZEND_METHOD(Ice_EndpointInfo, secure)
{
    if(ZEND_NUM_ARGS() > 0)
    {
        WRONG_PARAM_COUNT;
    }

    Ice::EndpointInfoPtr _this = Wrapper<Ice::EndpointInfoPtr>::value(getThis());
    assert(_this);

    try
    {
        RETURN_BOOL(_this->secure() ? 1 : 0);
    }
    catch(const IceUtil::Exception& ex)
    {
        throwException(ex);
        RETURN_NULL();
    }
}

#ifdef _WIN32
extern "C"
#endif
static zend_object*
handleEndpointInfoAlloc(zend_class_entry* ce)
{
    Wrapper<Ice::EndpointInfoPtr>* obj = Wrapper<Ice::EndpointInfoPtr>::create(ce);
    assert(obj);

    obj->zobj.handlers = &_endpointInfoHandlers;

    return &obj->zobj;
}

#ifdef _WIN32
extern "C"
#endif
static void
handleEndpointInfoFreeStorage(zend_object* object)
{
    Wrapper<Ice::EndpointInfoPtr>* obj = Wrapper<Ice::EndpointInfoPtr>::fetch(object);
    delete obj->ptr;
    zend_object_std_dtor(object);
}

static zend_function_entry _interfaceMethods[] =
{
    {0, 0, 0}
};

//
// Necessary to suppress warnings from zend_function_entry in php-5.2.
//
#if defined(__GNUC__)
#  pragma GCC diagnostic ignored "-Wwrite-strings"
#endif

//
// Predefined methods for Endpoint.
//
static zend_function_entry _endpointMethods[] =
{
    ZEND_ME(Ice_Endpoint, __construct, ICE_NULLPTR, ZEND_ACC_PRIVATE|ZEND_ACC_CTOR)
    ZEND_ME(Ice_Endpoint, __toString, ICE_NULLPTR, ZEND_ACC_PUBLIC)
    ZEND_ME(Ice_Endpoint, toString, ICE_NULLPTR, ZEND_ACC_PUBLIC)
    ZEND_ME(Ice_Endpoint, getInfo, ICE_NULLPTR, ZEND_ACC_PUBLIC)
    {0, 0, 0}
};

//
// Predefined methods for EndpointInfo.
//
static zend_function_entry _endpointInfoMethods[] =
{
    ZEND_ME(Ice_EndpointInfo, __construct, ICE_NULLPTR, ZEND_ACC_PRIVATE|ZEND_ACC_CTOR)
    ZEND_ME(Ice_EndpointInfo, type, ICE_NULLPTR, ZEND_ACC_PUBLIC)
    ZEND_ME(Ice_EndpointInfo, datagram, ICE_NULLPTR, ZEND_ACC_PUBLIC)
    ZEND_ME(Ice_EndpointInfo, secure, ICE_NULLPTR, ZEND_ACC_PUBLIC)
    {0, 0, 0}
};

//
// enable warning again
//
#if defined(__GNUC__)
#  pragma GCC diagnostic error "-Wwrite-strings"
#endif

bool
IcePHP::endpointInit(void)
{
    //
    // Although the Endpoint and EndpointInfo types are defined in Slice, we need to
    // define implementations at the time the PHP extension is loaded; we can't wait
    // to do this until after the generated code has been loaded. Consequently, we
    // define our own placeholder versions of the Slice types so that we can subclass
    // them. This essentially means that the generated code for these types is ignored.
    //

    //
    // Define the Endpoint interface.
    //
    zend_class_entry ce;
#ifdef ICEPHP_USE_NAMESPACES
    INIT_NS_CLASS_ENTRY(ce, "Ice", "Endpoint", _interfaceMethods);
#else
    INIT_CLASS_ENTRY(ce, "Ice_Endpoint", _interfaceMethods);
#endif
    zend_class_entry* endpointInterface = zend_register_internal_interface(&ce);

    //
    // Define a concrete Endpoint implementation class.
    //
    INIT_CLASS_ENTRY(ce, "IcePHP_Endpoint", _endpointMethods);
    ce.create_object = handleEndpointAlloc;
    endpointClassEntry = zend_register_internal_class(&ce);
    memcpy(&_endpointHandlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
    _endpointHandlers.free_obj = handleEndpointFreeStorage;
    _endpointHandlers.offset   = XtOffsetOf(Wrapper<Ice::EndpointPtr>, zobj);
    zend_class_implements(endpointClassEntry, 1, endpointInterface);

    //
    // Define the EndpointInfo class.
    //
#ifdef ICEPHP_USE_NAMESPACES
    INIT_NS_CLASS_ENTRY(ce, "Ice", "EndpointInfo", _endpointInfoMethods);
#else
    INIT_CLASS_ENTRY(ce, "Ice_EndpointInfo", _endpointInfoMethods);
#endif
    ce.create_object = handleEndpointInfoAlloc;
    endpointInfoClassEntry = zend_register_internal_class(&ce);
    memcpy(&_endpointInfoHandlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
    _endpointInfoHandlers.free_obj = handleEndpointInfoFreeStorage;
    _endpointInfoHandlers.offset   = XtOffsetOf(Wrapper<Ice::EndpointInfoPtr>, zobj);
    zend_declare_property_long(endpointInfoClassEntry, STRCAST("timeout"), sizeof("timeout") - 1, 0,
                               ZEND_ACC_PUBLIC);
    zend_declare_property_bool(endpointInfoClassEntry, STRCAST("compress"), sizeof("compress") - 1, 0,
                               ZEND_ACC_PUBLIC);

    //
    // Define the IPEndpointInfo class.
    //
#ifdef ICEPHP_USE_NAMESPACES
    INIT_NS_CLASS_ENTRY(ce, "Ice", "IPEndpointInfo", ICE_NULLPTR);
#else
    INIT_CLASS_ENTRY(ce, "Ice_IPEndpointInfo", ICE_NULLPTR);
#endif
    ce.create_object = handleEndpointInfoAlloc;
    ipEndpointInfoClassEntry = zend_register_internal_class_ex(&ce, endpointInfoClassEntry);
    zend_declare_property_string(ipEndpointInfoClassEntry, STRCAST("host"), sizeof("host") - 1, STRCAST(""),
                                 ZEND_ACC_PUBLIC);
    zend_declare_property_long(ipEndpointInfoClassEntry, STRCAST("port"), sizeof("port") - 1, 0,
                               ZEND_ACC_PUBLIC);
    zend_declare_property_string(ipEndpointInfoClassEntry, STRCAST("sourceAddress"), sizeof("sourceAddress") - 1,
                                 STRCAST(""), ZEND_ACC_PUBLIC);

    //
    // Define the TCPEndpointInfo class.
    //
#ifdef ICEPHP_USE_NAMESPACES
    INIT_NS_CLASS_ENTRY(ce, "Ice", "TCPEndpointInfo", ICE_NULLPTR);
#else
    INIT_CLASS_ENTRY(ce, "Ice_TCPEndpointInfo", ICE_NULLPTR);
#endif
    ce.create_object = handleEndpointInfoAlloc;
    tcpEndpointInfoClassEntry = zend_register_internal_class_ex(&ce, ipEndpointInfoClassEntry);

    //
    // Define the UDPEndpointInfo class.
    //
#ifdef ICEPHP_USE_NAMESPACES
    INIT_NS_CLASS_ENTRY(ce, "Ice", "UDPEndpointInfo", ICE_NULLPTR);
#else
    INIT_CLASS_ENTRY(ce, "Ice_UDPEndpointInfo", ICE_NULLPTR);
#endif
    ce.create_object = handleEndpointInfoAlloc;
    udpEndpointInfoClassEntry = zend_register_internal_class_ex(&ce, ipEndpointInfoClassEntry);
    zend_declare_property_string(udpEndpointInfoClassEntry, STRCAST("mcastInterface"), sizeof("mcastInterface") - 1,
                                 STRCAST(""), ZEND_ACC_PUBLIC);
    zend_declare_property_long(udpEndpointInfoClassEntry, STRCAST("mcastTtl"), sizeof("mcastTtl") - 1, 0,
                               ZEND_ACC_PUBLIC);

    //
    // Define the WSEndpointInfo class.
    //
#ifdef ICEPHP_USE_NAMESPACES
    INIT_NS_CLASS_ENTRY(ce, "Ice", "WSEndpointInfo", ICE_NULLPTR);
#else
    INIT_CLASS_ENTRY(ce, "Ice_WSEndpointInfo", ICE_NULLPTR);
#endif
    ce.create_object = handleEndpointInfoAlloc;
    wsEndpointInfoClassEntry = zend_register_internal_class_ex(&ce, endpointInfoClassEntry);
    zend_declare_property_string(wsEndpointInfoClassEntry, STRCAST("resource"), sizeof("resource") - 1,
                                 STRCAST(""), ZEND_ACC_PUBLIC);

    //
    // Define the OpaqueEndpointInfo class.
    //
#ifdef ICEPHP_USE_NAMESPACES
    INIT_NS_CLASS_ENTRY(ce, "Ice", "OpaqueEndpointInfo", ICE_NULLPTR);
#else
    INIT_CLASS_ENTRY(ce, "Ice_OpaqueEndpointInfo", ICE_NULLPTR);
#endif
    ce.create_object = handleEndpointInfoAlloc;
    opaqueEndpointInfoClassEntry = zend_register_internal_class_ex(&ce, endpointInfoClassEntry);
    zend_declare_property_null(opaqueEndpointInfoClassEntry, STRCAST("rawEncoding"), sizeof("rawEncoding") - 1,
                               ZEND_ACC_PUBLIC);
    zend_declare_property_null(opaqueEndpointInfoClassEntry, STRCAST("rawBytes"), sizeof("rawBytes") - 1,
                               ZEND_ACC_PUBLIC);

    //
    // Define the SSLEndpointInfo class.
    //
#ifdef ICEPHP_USE_NAMESPACES
    INIT_NS_CLASS_ENTRY(ce, "Ice", "SSLEndpointInfo", ICE_NULLPTR);
#else
    INIT_CLASS_ENTRY(ce, "Ice_SSLEndpointInfo", ICE_NULLPTR);
#endif
    ce.create_object = handleEndpointInfoAlloc;
    sslEndpointInfoClassEntry = zend_register_internal_class_ex(&ce, endpointInfoClassEntry);

    return true;
}

bool
IcePHP::createEndpoint(zval* zv, const Ice::EndpointPtr& p)
{
    if(object_init_ex(zv, endpointClassEntry) != SUCCESS)
    {
        runtimeError("unable to initialize endpoint");
        return false;
    }

    Wrapper<Ice::EndpointPtr>* obj = Wrapper<Ice::EndpointPtr>::extract(zv);
    assert(obj);
    assert(!obj->ptr);
    obj->ptr = new Ice::EndpointPtr(p);

    return true;
}

bool
IcePHP::fetchEndpoint(zval* zv, Ice::EndpointPtr& endpoint)
{
    if(ZVAL_IS_NULL(zv))
    {
        endpoint = 0;
    }
    else
    {
        if(Z_TYPE_P(zv) != IS_OBJECT || !checkClass(Z_OBJCE_P(zv), endpointClassEntry))
        {
            invalidArgument("value is not an endpoint");
            return false;
        }
        Wrapper<Ice::EndpointPtr>* obj = Wrapper<Ice::EndpointPtr>::extract(zv);
        if(!obj)
        {
            return false;
        }
        endpoint = *obj->ptr;
    }
    return true;
}

bool
IcePHP::createEndpointInfo(zval* zv, const Ice::EndpointInfoPtr& p)
{
    if(!p)
    {
        ZVAL_NULL(zv);
        return true;
    }

    int status;
    if(Ice::WSEndpointInfoPtr::dynamicCast(p))
    {
        Ice::WSEndpointInfoPtr info = Ice::WSEndpointInfoPtr::dynamicCast(p);
        if((status = object_init_ex(zv, wsEndpointInfoClassEntry)) == SUCCESS)
        {
            add_property_string(zv, STRCAST("resource"), const_cast<char*>(info->resource.c_str()));
        }
    }
    else if(Ice::TCPEndpointInfoPtr::dynamicCast(p))
    {
        status = object_init_ex(zv, tcpEndpointInfoClassEntry);
    }
    else if(Ice::UDPEndpointInfoPtr::dynamicCast(p))
    {
        Ice::UDPEndpointInfoPtr info = Ice::UDPEndpointInfoPtr::dynamicCast(p);
        if((status = object_init_ex(zv, udpEndpointInfoClassEntry)) == SUCCESS)
        {
            add_property_string(zv, STRCAST("mcastInterface"), const_cast<char*>(info->mcastInterface.c_str()));
            add_property_long(zv, STRCAST("mcastTtl"), static_cast<long>(info->mcastTtl));
        }
    }
    else if(Ice::OpaqueEndpointInfoPtr::dynamicCast(p))
    {
        Ice::OpaqueEndpointInfoPtr info = Ice::OpaqueEndpointInfoPtr::dynamicCast(p);
        if((status = object_init_ex(zv, opaqueEndpointInfoClassEntry)) == SUCCESS)
        {
            zval rawEncoding;
            createEncodingVersion(&rawEncoding, info->rawEncoding);
            add_property_zval(zv, STRCAST("rawEncoding"), &rawEncoding);
            zval_ptr_dtor(&rawEncoding); // add_property_zval increased the refcount of rawEncoding

            zval rawBytes;
            array_init(&rawBytes);
            for(Ice::ByteSeq::iterator i = info->rawBytes.begin(); i != info->rawBytes.end(); ++i)
            {
                add_next_index_long(&rawBytes, *i & 0xff);
            }
            add_property_zval(zv, STRCAST("rawBytes"), &rawBytes);
            zval_ptr_dtor(&rawBytes); // add_property_zval increased the refcount of rawBytes
        }
    }
    else if(IceSSL::EndpointInfoPtr::dynamicCast(p))
    {
        status = object_init_ex(zv, sslEndpointInfoClassEntry);
    }
    else if(Ice::IPEndpointInfoPtr::dynamicCast(p))
    {
        status = object_init_ex(zv, ipEndpointInfoClassEntry);
    }
    else
    {
        status = object_init_ex(zv, endpointInfoClassEntry);
    }

    if(status != SUCCESS)
    {
        runtimeError("unable to initialize endpoint info");
        return false;
    }

    if(Ice::IPEndpointInfoPtr::dynamicCast(p))
    {
        Ice::IPEndpointInfoPtr info = Ice::IPEndpointInfoPtr::dynamicCast(p);
        add_property_string(zv, STRCAST("host"), const_cast<char*>(info->host.c_str()));
        add_property_long(zv, STRCAST("port"), static_cast<long>(info->port));
        add_property_string(zv, STRCAST("sourceAddress"), const_cast<char*>(info->sourceAddress.c_str()));
    }

    zval underlying;
    if(!createEndpointInfo(&underlying, p->underlying TSRMLS_CC))
    {
        runtimeError("unable to initialize endpoint info" TSRMLS_CC);
        return false;
    }
    add_property_zval(zv, STRCAST("underlying"), &underlying);
    zval_ptr_dtor(&underlying); // add_property_zval increased the refcount of underlying
    add_property_long(zv, STRCAST("timeout"), static_cast<long>(p->timeout));
    add_property_bool(zv, STRCAST("compress"), static_cast<long>(p->compress));

    Wrapper<Ice::EndpointInfoPtr>* obj = Wrapper<Ice::EndpointInfoPtr>::extract(zv);
    assert(obj);
    assert(!obj->ptr);
    obj->ptr = new Ice::EndpointInfoPtr(p);

    return true;
}
