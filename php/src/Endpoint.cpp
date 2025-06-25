// Copyright (c) ZeroC, Inc.

#include "Endpoint.h"
#include "Ice/Ice.h"
#include "Util.h"

using namespace std;
using namespace IcePHP;

ZEND_EXTERN_MODULE_GLOBALS(ice)

// Class entries representing the PHP class implementations we have registered.
static zend_class_entry* endpointClassEntry = 0;

static zend_class_entry* endpointInfoClassEntry = 0;
static zend_class_entry* ipEndpointInfoClassEntry = 0;
static zend_class_entry* tcpEndpointInfoClassEntry = 0;
static zend_class_entry* udpEndpointInfoClassEntry = 0;
static zend_class_entry* wsEndpointInfoClassEntry = 0;
static zend_class_entry* opaqueEndpointInfoClassEntry = 0;
static zend_class_entry* sslEndpointInfoClassEntry = 0;

// Ice::Endpoint support.
static zend_object_handlers _endpointHandlers;
static zend_object_handlers _endpointInfoHandlers;

extern "C"
{
    static zend_object* handleEndpointAlloc(zend_class_entry*);
    static void handleEndpointFreeStorage(zend_object*);

    static zend_object* handleEndpointInfoAlloc(zend_class_entry*);
    static void handleEndpointInfoFreeStorage(zend_object*);
}

ZEND_METHOD(Ice_Endpoint, __construct) { runtimeError("Endpoint cannot be instantiated"); }

ZEND_METHOD(Ice_Endpoint, __toString)
{
    if (ZEND_NUM_ARGS() > 0)
    {
        WRONG_PARAM_COUNT;
    }

    Ice::EndpointPtr _this = Wrapper<Ice::EndpointPtr>::value(getThis());
    assert(_this);

    try
    {
        string str = _this->toString();
        RETURN_STRINGL(str.c_str(), static_cast<int>(str.length()));
    }
    catch (...)
    {
        throwException(current_exception());
        RETURN_NULL();
    }
}

ZEND_METHOD(Ice_Endpoint, toString) { ZEND_MN(Ice_Endpoint___toString)(INTERNAL_FUNCTION_PARAM_PASSTHRU); }

ZEND_METHOD(Ice_Endpoint, getInfo)
{
    if (ZEND_NUM_ARGS() > 0)
    {
        WRONG_PARAM_COUNT;
    }

    Ice::EndpointPtr _this = Wrapper<Ice::EndpointPtr>::value(getThis());
    assert(_this);

    if (!createEndpointInfo(return_value, _this->getInfo()))
    {
        RETURN_NULL();
    }
}

static zend_object*
handleEndpointAlloc(zend_class_entry* ce)
{
    Wrapper<Ice::EndpointPtr>* obj = Wrapper<Ice::EndpointPtr>::create(ce);
    assert(obj);

    obj->zobj.handlers = &_endpointHandlers;

    return &obj->zobj;
}

static void
handleEndpointFreeStorage(zend_object* object)
{
    Wrapper<Ice::EndpointPtr>* obj = Wrapper<Ice::EndpointPtr>::fetch(object);
    delete obj->ptr;
    zend_object_std_dtor(object);
}

ZEND_METHOD(Ice_EndpointInfo, __construct) { runtimeError("EndpointInfo cannot be instantiated"); }

ZEND_METHOD(Ice_EndpointInfo, type)
{
    if (ZEND_NUM_ARGS() > 0)
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
    catch (...)
    {
        throwException(current_exception());
        RETURN_NULL();
    }
}

ZEND_METHOD(Ice_EndpointInfo, datagram)
{
    if (ZEND_NUM_ARGS() > 0)
    {
        WRONG_PARAM_COUNT;
    }

    Ice::EndpointInfoPtr _this = Wrapper<Ice::EndpointInfoPtr>::value(getThis());
    assert(_this);

    try
    {
        RETURN_BOOL(_this->datagram() ? 1 : 0);
    }
    catch (...)
    {
        throwException(current_exception());
        RETURN_NULL();
    }
}

ZEND_METHOD(Ice_EndpointInfo, secure)
{
    if (ZEND_NUM_ARGS() > 0)
    {
        WRONG_PARAM_COUNT;
    }

    Ice::EndpointInfoPtr _this = Wrapper<Ice::EndpointInfoPtr>::value(getThis());
    assert(_this);

    try
    {
        RETURN_BOOL(_this->secure() ? 1 : 0);
    }
    catch (...)
    {
        throwException(current_exception());
        RETURN_NULL();
    }
}

static zend_object*
handleEndpointInfoAlloc(zend_class_entry* ce)
{
    Wrapper<Ice::EndpointInfoPtr>* obj = Wrapper<Ice::EndpointInfoPtr>::create(ce);
    assert(obj);
    obj->zobj.handlers = &_endpointInfoHandlers;
    return &obj->zobj;
}

static void
handleEndpointInfoFreeStorage(zend_object* object)
{
    Wrapper<Ice::EndpointInfoPtr>* obj = Wrapper<Ice::EndpointInfoPtr>::fetch(object);
    delete obj->ptr;
    zend_object_std_dtor(object);
}

static zend_function_entry _interfaceMethods[] = {{0, 0, 0}};

// Predefined methods for Endpoint.
static zend_function_entry _endpointMethods[] = {
    // _construct
    ZEND_ME(Ice_Endpoint, __construct, ice_void_arginfo, ZEND_ACC_PRIVATE | ZEND_ACC_CTOR)
    // __toString
    ZEND_ME(Ice_Endpoint, __toString, ice_to_string_arginfo, ZEND_ACC_PUBLIC)
    // toString
    ZEND_ME(Ice_Endpoint, toString, ice_void_arginfo, ZEND_ACC_PUBLIC)
    // getInfo
    ZEND_ME(Ice_Endpoint, getInfo, ice_void_arginfo, ZEND_ACC_PUBLIC){0, 0, 0}};

// Predefined methods for EndpointInfo.
static zend_function_entry _endpointInfoMethods[] = {
    // _construct
    ZEND_ME(Ice_EndpointInfo, __construct, ice_void_arginfo, ZEND_ACC_PRIVATE | ZEND_ACC_CTOR)
    // type
    ZEND_ME(Ice_EndpointInfo, type, ice_void_arginfo, ZEND_ACC_PUBLIC)
    // datagram
    ZEND_ME(Ice_EndpointInfo, datagram, ice_void_arginfo, ZEND_ACC_PUBLIC)
    // secure
    ZEND_ME(Ice_EndpointInfo, secure, ice_void_arginfo, ZEND_ACC_PUBLIC){0, 0, 0}};

bool
IcePHP::endpointInit(void)
{
    // TODO review the comment is no longer accurate, the code is no longer generated in Slice
    // Although the Endpoint and EndpointInfo types are defined in Slice, we need to
    // define implementations at the time the PHP extension is loaded; we can't wait
    // to do this until after the generated code has been loaded. Consequently, we
    // define our own placeholder versions of the Slice types so that we can subclass
    // them. This essentially means that the generated code for these types is ignored.
    //

    // Define the Endpoint interface.
    zend_class_entry ce;
    INIT_NS_CLASS_ENTRY(ce, "Ice", "Endpoint", _interfaceMethods);
    zend_class_entry* endpointInterface = zend_register_internal_interface(&ce);

    // Define a concrete Endpoint implementation class.
    INIT_CLASS_ENTRY(ce, "IcePHP_Endpoint", _endpointMethods);
    ce.create_object = handleEndpointAlloc;
    endpointClassEntry = zend_register_internal_class(&ce);
    memcpy(&_endpointHandlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
    _endpointHandlers.free_obj = handleEndpointFreeStorage;
    _endpointHandlers.offset = XtOffsetOf(Wrapper<Ice::EndpointPtr>, zobj);
    zend_class_implements(endpointClassEntry, 1, endpointInterface);

    // Define the EndpointInfo class.
    INIT_NS_CLASS_ENTRY(ce, "Ice", "EndpointInfo", _endpointInfoMethods);
    ce.create_object = handleEndpointInfoAlloc;
    endpointInfoClassEntry = zend_register_internal_class(&ce);
    memcpy(&_endpointInfoHandlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
    _endpointInfoHandlers.free_obj = handleEndpointInfoFreeStorage;
    _endpointInfoHandlers.offset = XtOffsetOf(Wrapper<Ice::EndpointInfoPtr>, zobj);
    zend_declare_property_bool(endpointInfoClassEntry, "compress", sizeof("compress") - 1, 0, ZEND_ACC_PUBLIC);
    zend_declare_property_null(endpointInfoClassEntry, "underlying", sizeof("underlying") - 1, ZEND_ACC_PUBLIC);

    // Define the IPEndpointInfo class.
    INIT_NS_CLASS_ENTRY(ce, "Ice", "IPEndpointInfo", nullptr);
    ce.create_object = handleEndpointInfoAlloc;
    ipEndpointInfoClassEntry = zend_register_internal_class_ex(&ce, endpointInfoClassEntry);
    zend_declare_property_string(ipEndpointInfoClassEntry, "host", sizeof("host") - 1, "", ZEND_ACC_PUBLIC);
    zend_declare_property_long(ipEndpointInfoClassEntry, "port", sizeof("port") - 1, 0, ZEND_ACC_PUBLIC);
    zend_declare_property_string(
        ipEndpointInfoClassEntry,
        "sourceAddress",
        sizeof("sourceAddress") - 1,
        "",
        ZEND_ACC_PUBLIC);

    // Define the TCPEndpointInfo class.
    INIT_NS_CLASS_ENTRY(ce, "Ice", "TCPEndpointInfo", nullptr);
    ce.create_object = handleEndpointInfoAlloc;
    tcpEndpointInfoClassEntry = zend_register_internal_class_ex(&ce, ipEndpointInfoClassEntry);

    // Define the UDPEndpointInfo class.
    INIT_NS_CLASS_ENTRY(ce, "Ice", "UDPEndpointInfo", nullptr);
    ce.create_object = handleEndpointInfoAlloc;
    udpEndpointInfoClassEntry = zend_register_internal_class_ex(&ce, ipEndpointInfoClassEntry);
    zend_declare_property_string(
        udpEndpointInfoClassEntry,
        "mcastInterface",
        sizeof("mcastInterface") - 1,
        "",
        ZEND_ACC_PUBLIC);
    zend_declare_property_long(udpEndpointInfoClassEntry, "mcastTtl", sizeof("mcastTtl") - 1, 0, ZEND_ACC_PUBLIC);

    // Define the WSEndpointInfo class.
    INIT_NS_CLASS_ENTRY(ce, "Ice", "WSEndpointInfo", nullptr);
    ce.create_object = handleEndpointInfoAlloc;
    wsEndpointInfoClassEntry = zend_register_internal_class_ex(&ce, endpointInfoClassEntry);
    zend_declare_property_string(wsEndpointInfoClassEntry, "resource", sizeof("resource") - 1, "", ZEND_ACC_PUBLIC);

    // Define the OpaqueEndpointInfo class.
    INIT_NS_CLASS_ENTRY(ce, "Ice", "OpaqueEndpointInfo", nullptr);
    ce.create_object = handleEndpointInfoAlloc;
    opaqueEndpointInfoClassEntry = zend_register_internal_class_ex(&ce, endpointInfoClassEntry);
    zend_declare_property_null(opaqueEndpointInfoClassEntry, "rawEncoding", sizeof("rawEncoding") - 1, ZEND_ACC_PUBLIC);
    zend_declare_property_null(opaqueEndpointInfoClassEntry, "rawBytes", sizeof("rawBytes") - 1, ZEND_ACC_PUBLIC);

    // Define the SSLEndpointInfo class.
    INIT_NS_CLASS_ENTRY(ce, "Ice", "SSLEndpointInfo", nullptr);
    ce.create_object = handleEndpointInfoAlloc;
    sslEndpointInfoClassEntry = zend_register_internal_class_ex(&ce, endpointInfoClassEntry);

    return true;
}

bool
IcePHP::createEndpoint(zval* zv, const Ice::EndpointPtr& p)
{
    if (object_init_ex(zv, endpointClassEntry) != SUCCESS)
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
    if (ZVAL_IS_NULL(zv))
    {
        endpoint = 0;
    }
    else
    {
        if (Z_TYPE_P(zv) != IS_OBJECT || !checkClass(Z_OBJCE_P(zv), endpointClassEntry))
        {
            invalidArgument("value is not an endpoint");
            return false;
        }
        Wrapper<Ice::EndpointPtr>* obj = Wrapper<Ice::EndpointPtr>::extract(zv);
        if (!obj)
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
    if (!p)
    {
        ZVAL_NULL(zv);
        return true;
    }

    int status;
    if (dynamic_pointer_cast<Ice::WSEndpointInfo>(p))
    {
        auto info = dynamic_pointer_cast<Ice::WSEndpointInfo>(p);
        if ((status = object_init_ex(zv, wsEndpointInfoClassEntry)) == SUCCESS)
        {
            add_property_string(zv, "resource", const_cast<char*>(info->resource.c_str()));
        }
    }
    else if (dynamic_pointer_cast<Ice::TCPEndpointInfo>(p))
    {
        status = object_init_ex(zv, tcpEndpointInfoClassEntry);
    }
    else if (dynamic_pointer_cast<Ice::UDPEndpointInfo>(p))
    {
        auto info = dynamic_pointer_cast<Ice::UDPEndpointInfo>(p);
        if ((status = object_init_ex(zv, udpEndpointInfoClassEntry)) == SUCCESS)
        {
            add_property_string(zv, "mcastInterface", const_cast<char*>(info->mcastInterface.c_str()));
            add_property_long(zv, "mcastTtl", static_cast<long>(info->mcastTtl));
        }
    }
    else if (dynamic_pointer_cast<Ice::OpaqueEndpointInfo>(p))
    {
        auto info = dynamic_pointer_cast<Ice::OpaqueEndpointInfo>(p);
        if ((status = object_init_ex(zv, opaqueEndpointInfoClassEntry)) == SUCCESS)
        {
            zval rawEncoding;
            createEncodingVersion(&rawEncoding, info->rawEncoding);
            add_property_zval(zv, "rawEncoding", &rawEncoding);
            zval_ptr_dtor(&rawEncoding); // add_property_zval increased the refcount of rawEncoding

            zval rawBytes;
            array_init(&rawBytes);
            for (const auto& i : info->rawBytes)
            {
                add_next_index_long(&rawBytes, static_cast<zend_long>(i & byte{0xff}));
            }
            add_property_zval(zv, "rawBytes", &rawBytes);
            zval_ptr_dtor(&rawBytes); // add_property_zval increased the refcount of rawBytes
        }
    }
    else if (dynamic_pointer_cast<Ice::SSL::EndpointInfo>(p))
    {
        status = object_init_ex(zv, sslEndpointInfoClassEntry);
    }
    else if (dynamic_pointer_cast<Ice::IPEndpointInfo>(p))
    {
        status = object_init_ex(zv, ipEndpointInfoClassEntry);
    }
    else
    {
        status = object_init_ex(zv, endpointInfoClassEntry);
    }

    if (status != SUCCESS)
    {
        runtimeError("unable to initialize endpoint info");
        return false;
    }

    if (dynamic_pointer_cast<Ice::IPEndpointInfo>(p))
    {
        auto info = dynamic_pointer_cast<Ice::IPEndpointInfo>(p);
        add_property_string(zv, "host", const_cast<char*>(info->host.c_str()));
        add_property_long(zv, "port", static_cast<long>(info->port));
        add_property_string(zv, "sourceAddress", const_cast<char*>(info->sourceAddress.c_str()));
    }

    zval underlying;
    if (!createEndpointInfo(&underlying, p->underlying))
    {
        runtimeError("unable to initialize endpoint info");
        return false;
    }
    add_property_zval(zv, "underlying", &underlying);
    zval_ptr_dtor(&underlying); // add_property_zval increased the refcount of underlying
    add_property_bool(zv, "compress", static_cast<long>(p->compress));

    Wrapper<Ice::EndpointInfoPtr>* obj = Wrapper<Ice::EndpointInfoPtr>::extract(zv);
    assert(obj);
    assert(!obj->ptr);
    obj->ptr = new Ice::EndpointInfoPtr(p);

    return true;
}
