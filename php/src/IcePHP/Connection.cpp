// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Connection.h>
#include <Endpoint.h>
#include <Util.h>

using namespace std;
using namespace IcePHP;

ZEND_EXTERN_MODULE_GLOBALS(ice)

//
// Class entries represent the PHP class implementations we have registered.
//
static zend_class_entry* connectionClassEntry = 0;

static zend_class_entry* connectionInfoClassEntry = 0;
static zend_class_entry* ipConnectionInfoClassEntry = 0;
static zend_class_entry* tcpConnectionInfoClassEntry = 0;
static zend_class_entry* udpConnectionInfoClassEntry = 0;

//
// Ice::Connection support.
//
static zend_object_handlers _connectionHandlers;
static zend_object_handlers _connectionInfoHandlers;

extern "C"
{
static zend_object_value handleConnectionAlloc(zend_class_entry* TSRMLS_DC);
static void handleConnectionFreeStorage(void* TSRMLS_DC);
static int handleConnectionCompare(zval*, zval* TSRMLS_DC);

static zend_object_value handleConnectionInfoAlloc(zend_class_entry* TSRMLS_DC);
static void handleConnectionInfoFreeStorage(void* TSRMLS_DC);
}

ZEND_METHOD(Ice_Connection, __construct)
{
    runtimeError("Connection cannot be instantiated" TSRMLS_CC);
}

ZEND_METHOD(Ice_Connection, __toString)
{
    if(ZEND_NUM_ARGS() > 0)
    {
        WRONG_PARAM_COUNT;
    }

    Ice::ConnectionPtr _this = Wrapper<Ice::ConnectionPtr>::value(getThis() TSRMLS_CC);
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

ZEND_METHOD(Ice_Connection, close)
{
    Ice::ConnectionPtr _this = Wrapper<Ice::ConnectionPtr>::value(getThis() TSRMLS_CC);
    assert(_this);

    zend_bool b;
    if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, const_cast<char*>("b"), &b TSRMLS_CC) != SUCCESS)
    {
        RETURN_NULL();
    }

    try
    {
        _this->close(b ? true : false);
    }
    catch(const IceUtil::Exception& ex)
    {
        throwException(ex TSRMLS_CC);
        RETURN_NULL();
    }
}

ZEND_METHOD(Ice_Connection, getEndpoint)
{
    if(ZEND_NUM_ARGS() > 0)
    {
        WRONG_PARAM_COUNT;
    }

    Ice::ConnectionPtr _this = Wrapper<Ice::ConnectionPtr>::value(getThis() TSRMLS_CC);
    assert(_this);

    try
    {
        if(!createEndpoint(return_value, _this->getEndpoint() TSRMLS_CC))
        {
            RETURN_NULL();
        }
    }
    catch(const IceUtil::Exception& ex)
    {
        throwException(ex TSRMLS_CC);
        RETURN_NULL();
    }
}

ZEND_METHOD(Ice_Connection, flushBatchRequests)
{
    if(ZEND_NUM_ARGS() > 0)
    {
        WRONG_PARAM_COUNT;
    }

    Ice::ConnectionPtr _this = Wrapper<Ice::ConnectionPtr>::value(getThis() TSRMLS_CC);
    assert(_this);

    try
    {
        _this->flushBatchRequests();
    }
    catch(const IceUtil::Exception& ex)
    {
        throwException(ex TSRMLS_CC);
        RETURN_NULL();
    }
}

ZEND_METHOD(Ice_Connection, type)
{
    if(ZEND_NUM_ARGS() > 0)
    {
        WRONG_PARAM_COUNT;
    }

    Ice::ConnectionPtr _this = Wrapper<Ice::ConnectionPtr>::value(getThis() TSRMLS_CC);
    assert(_this);

    try
    {
        string str = _this->type();
        RETURN_STRINGL(STRCAST(str.c_str()), str.length(), 1);
    }
    catch(const IceUtil::Exception& ex)
    {
        throwException(ex TSRMLS_CC);
        RETURN_NULL();
    }
}

ZEND_METHOD(Ice_Connection, timeout)
{
    if(ZEND_NUM_ARGS() != 0)
    {
        WRONG_PARAM_COUNT;
    }

    Ice::ConnectionPtr _this = Wrapper<Ice::ConnectionPtr>::value(getThis() TSRMLS_CC);
    assert(_this);

    try
    {
        Ice::Int timeout = _this->timeout();
        ZVAL_LONG(return_value, static_cast<long>(timeout));
    }
    catch(const IceUtil::Exception& ex)
    {
        throwException(ex TSRMLS_CC);
        RETURN_NULL();
    }
}

ZEND_METHOD(Ice_Connection, toString)
{
    ZEND_MN(Ice_Connection___toString)(INTERNAL_FUNCTION_PARAM_PASSTHRU);
}

ZEND_METHOD(Ice_Connection, getInfo)
{
    if(ZEND_NUM_ARGS() != 0)
    {
        WRONG_PARAM_COUNT;
    }

    Ice::ConnectionPtr _this = Wrapper<Ice::ConnectionPtr>::value(getThis() TSRMLS_CC);
    assert(_this);

    try
    {
        Ice::ConnectionInfoPtr info = _this->getInfo();
        if(!createConnectionInfo(return_value, _this->getInfo() TSRMLS_CC))
        {
            RETURN_NULL();
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
handleConnectionAlloc(zend_class_entry* ce TSRMLS_DC)
{
    zend_object_value result;

    Wrapper<Ice::ConnectionPtr>* obj = Wrapper<Ice::ConnectionPtr>::create(ce TSRMLS_CC);
    assert(obj);

    result.handle = zend_objects_store_put(obj, 0, (zend_objects_free_object_storage_t)handleConnectionFreeStorage,
                                           0 TSRMLS_CC);
    result.handlers = &_connectionHandlers;

    return result;
}

#ifdef _WIN32
extern "C"
#endif
static void
handleConnectionFreeStorage(void* p TSRMLS_DC)
{
    Wrapper<Ice::ConnectionPtr>* obj = static_cast<Wrapper<Ice::ConnectionPtr>*>(p);
    delete obj->ptr;
    zend_object_std_dtor(static_cast<zend_object*>(p) TSRMLS_CC);
    efree(p);
}

#ifdef _WIN32
extern "C"
#endif
static int
handleConnectionCompare(zval* zobj1, zval* zobj2 TSRMLS_DC)
{
    //
    // PHP guarantees that the objects have the same class.
    //

    Ice::ConnectionPtr con1 = Wrapper<Ice::ConnectionPtr>::value(zobj1 TSRMLS_CC);
    assert(con1);
    Ice::ConnectionPtr con2 = Wrapper<Ice::ConnectionPtr>::value(zobj2 TSRMLS_CC);
    assert(con2);

    if(con1 == con2)
    {
        return 0;
    }
    else if(con1 < con2)
    {
        return -1;
    }
    else
    {
        return 1;
    }
}

//
// Necessary to suppress warnings from zend_function_entry in php-5.2.
//
#if defined(__GNUC__)
#  pragma GCC diagnostic ignored "-Wwrite-strings"
#endif

//
// Predefined methods for Connection.
//
static zend_function_entry _interfaceMethods[] =
{
    {0, 0, 0}
};
static zend_function_entry _connectionClassMethods[] =
{
    ZEND_ME(Ice_Connection, __construct, NULL, ZEND_ACC_PRIVATE|ZEND_ACC_CTOR)
    ZEND_ME(Ice_Connection, __toString, NULL, ZEND_ACC_PUBLIC)
    ZEND_ME(Ice_Connection, close, NULL, ZEND_ACC_PUBLIC)
    ZEND_ME(Ice_Connection, getEndpoint, NULL, ZEND_ACC_PUBLIC)
    ZEND_ME(Ice_Connection, flushBatchRequests, NULL, ZEND_ACC_PUBLIC)
    ZEND_ME(Ice_Connection, type, NULL, ZEND_ACC_PUBLIC)
    ZEND_ME(Ice_Connection, timeout, NULL, ZEND_ACC_PUBLIC)
    ZEND_ME(Ice_Connection, toString, NULL, ZEND_ACC_PUBLIC)
    ZEND_ME(Ice_Connection, getInfo, NULL, ZEND_ACC_PUBLIC)
    {0, 0, 0}
};

ZEND_METHOD(Ice_ConnectionInfo, __construct)
{
    runtimeError("ConnectionInfo cannot be instantiated" TSRMLS_CC);
}

//
// Predefined methods for ConnectionInfo.
//
static zend_function_entry _connectionInfoClassMethods[] =
{
    ZEND_ME(Ice_ConnectionInfo, __construct, NULL, ZEND_ACC_PRIVATE|ZEND_ACC_CTOR)
    {0, 0, 0}
};
//
// enable warning again
//
#if defined(__GNUC__)
#  pragma GCC diagnostic error "-Wwrite-strings"
#endif

#ifdef _WIN32
extern "C"
#endif
static zend_object_value
handleConnectionInfoAlloc(zend_class_entry* ce TSRMLS_DC)
{
    zend_object_value result;

    Wrapper<Ice::ConnectionInfoPtr>* obj = Wrapper<Ice::ConnectionInfoPtr>::create(ce TSRMLS_CC);
    assert(obj);

    result.handle = zend_objects_store_put(obj, 0, (zend_objects_free_object_storage_t)handleConnectionInfoFreeStorage,
                                           0 TSRMLS_CC);
    result.handlers = &_connectionInfoHandlers;

    return result;
}

#ifdef _WIN32
extern "C"
#endif
static void
handleConnectionInfoFreeStorage(void* p TSRMLS_DC)
{
    Wrapper<Ice::ConnectionInfoPtr>* obj = static_cast<Wrapper<Ice::ConnectionInfoPtr>*>(p);
    delete obj->ptr;
    zend_object_std_dtor(static_cast<zend_object*>(p) TSRMLS_CC);
    efree(p);
}

bool
IcePHP::connectionInit(TSRMLS_D)
{
    //
    // We register an interface and a class that implements the interface. This allows
    // applications to safely include the Slice-generated code for the type.
    //

    //
    // Register the Connection interface.
    //
    zend_class_entry ce;
#ifdef ICEPHP_USE_NAMESPACES
    INIT_NS_CLASS_ENTRY(ce, "Ice", "Connection", _interfaceMethods);
#else
    INIT_CLASS_ENTRY(ce, "Ice_Connection", _interfaceMethods);
#endif
    zend_class_entry* interface = zend_register_internal_interface(&ce TSRMLS_CC);

    //
    // Register the Connection class.
    //
    INIT_CLASS_ENTRY(ce, "IcePHP_Connection", _connectionClassMethods);
    ce.create_object = handleConnectionAlloc;
    connectionClassEntry = zend_register_internal_class(&ce TSRMLS_CC);
    memcpy(&_connectionHandlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
    _connectionHandlers.compare_objects = handleConnectionCompare;
    zend_class_implements(connectionClassEntry TSRMLS_CC, 1, interface);

    //
    // Register the ConnectionInfo class.
    //
#ifdef ICEPHP_USE_NAMESPACES
    INIT_NS_CLASS_ENTRY(ce, "Ice", "ConnectionInfo", _connectionInfoClassMethods);
#else
    INIT_CLASS_ENTRY(ce, "Ice_ConnectionInfo", _connectionInfoClassMethods);
#endif
    ce.create_object = handleConnectionInfoAlloc;
    connectionInfoClassEntry = zend_register_internal_class(&ce TSRMLS_CC);
    memcpy(&_connectionInfoHandlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
    zend_declare_property_bool(connectionInfoClassEntry, STRCAST("incoming"), sizeof("incoming") - 1, 0,
                               ZEND_ACC_PUBLIC TSRMLS_CC);
    zend_declare_property_string(connectionInfoClassEntry, STRCAST("adapterName"), sizeof("adapterName") - 1,
                                 STRCAST(""), ZEND_ACC_PUBLIC TSRMLS_CC);

    //
    // Register the IPConnectionInfo class.
    //
#ifdef ICEPHP_USE_NAMESPACES
    INIT_NS_CLASS_ENTRY(ce, "Ice", "IPConnectionInfo", NULL);
#else
    INIT_CLASS_ENTRY(ce, "Ice_IPConnectionInfo", NULL);
#endif
    ce.create_object = handleConnectionInfoAlloc;
    ipConnectionInfoClassEntry = zend_register_internal_class_ex(&ce, connectionInfoClassEntry, NULL TSRMLS_CC);
    zend_declare_property_string(ipConnectionInfoClassEntry, STRCAST("localAddress"), sizeof("localAddress") - 1,
                                 STRCAST(""), ZEND_ACC_PUBLIC TSRMLS_CC);
    zend_declare_property_long(ipConnectionInfoClassEntry, STRCAST("localPort"), sizeof("localPort") - 1, 0,
                               ZEND_ACC_PUBLIC TSRMLS_CC);
    zend_declare_property_string(ipConnectionInfoClassEntry, STRCAST("remoteAddress"), sizeof("remoteAddress") - 1,
                                 STRCAST(""), ZEND_ACC_PUBLIC TSRMLS_CC);
    zend_declare_property_long(ipConnectionInfoClassEntry, STRCAST("remotePort"), sizeof("remotePort") - 1, 0,
                               ZEND_ACC_PUBLIC TSRMLS_CC);

    //
    // Register the TCPConnectionInfo class.
    //
#ifdef ICEPHP_USE_NAMESPACES
    INIT_NS_CLASS_ENTRY(ce, "Ice", "TCPConnectionInfo", NULL);
#else
    INIT_CLASS_ENTRY(ce, "Ice_TCPConnectionInfo", NULL);
#endif
    ce.create_object = handleConnectionInfoAlloc;
    tcpConnectionInfoClassEntry = zend_register_internal_class_ex(&ce, ipConnectionInfoClassEntry, NULL TSRMLS_CC);

    //
    // Register the UDPConnectionInfo class.
    //
#ifdef ICEPHP_USE_NAMESPACES
    INIT_NS_CLASS_ENTRY(ce, "Ice", "UDPConnectionInfo", NULL);
#else
    INIT_CLASS_ENTRY(ce, "Ice_UDPConnectionInfo", NULL);
#endif
    ce.create_object = handleConnectionInfoAlloc;
    udpConnectionInfoClassEntry = zend_register_internal_class_ex(&ce, ipConnectionInfoClassEntry, NULL TSRMLS_CC);
    zend_declare_property_string(udpConnectionInfoClassEntry, STRCAST("mcastAddress"), sizeof("mcastAddress") - 1,
                                 STRCAST(""), ZEND_ACC_PUBLIC TSRMLS_CC);
    zend_declare_property_long(udpConnectionInfoClassEntry, STRCAST("mcastPort"), sizeof("mcastPort") - 1, 0,
                               ZEND_ACC_PUBLIC TSRMLS_CC);

    return true;
}

bool
IcePHP::createConnection(zval* zv, const Ice::ConnectionPtr& p TSRMLS_DC)
{
    if(object_init_ex(zv, connectionClassEntry) != SUCCESS)
    {
        runtimeError("unable to initialize connection" TSRMLS_CC);
        return false;
    }

    Wrapper<Ice::ConnectionPtr>* obj = Wrapper<Ice::ConnectionPtr>::extract(zv TSRMLS_CC);
    assert(obj);
    assert(!obj->ptr);
    obj->ptr = new Ice::ConnectionPtr(p);

    return true;
}

bool
IcePHP::fetchConnection(zval* zv, Ice::ConnectionPtr& connection TSRMLS_DC)
{
    if(ZVAL_IS_NULL(zv))
    {
        connection = 0;
    }
    else
    {
        if(Z_TYPE_P(zv) != IS_OBJECT || Z_OBJCE_P(zv) != connectionClassEntry)
        {
            invalidArgument("value is not a connection" TSRMLS_CC);
            return false;
        }
        Wrapper<Ice::ConnectionPtr>* obj = Wrapper<Ice::ConnectionPtr>::extract(zv TSRMLS_CC);
        if(!obj)
        {
            return false;
        }
        connection = *obj->ptr;
    }
    return true;
}

bool
IcePHP::createConnectionInfo(zval* zv, const Ice::ConnectionInfoPtr& p TSRMLS_DC)
{
    int status;
    if(Ice::TCPConnectionInfoPtr::dynamicCast(p))
    {
        status = object_init_ex(zv, tcpConnectionInfoClassEntry);
    }
    else if(Ice::UDPConnectionInfoPtr::dynamicCast(p))
    {
        Ice::UDPConnectionInfoPtr info = Ice::UDPConnectionInfoPtr::dynamicCast(p);
        if((status = object_init_ex(zv, udpConnectionInfoClassEntry)) == SUCCESS)
        {
            add_property_string(zv, STRCAST("mcastAddress"), const_cast<char*>(info->mcastAddress.c_str()), 1);
            add_property_long(zv, STRCAST("mcastPort"), static_cast<long>(info->mcastPort));
        }
    }
    else if(Ice::IPConnectionInfoPtr::dynamicCast(p))
    {
        status = object_init_ex(zv, ipConnectionInfoClassEntry);
    }
    else
    {
        status = object_init_ex(zv, connectionInfoClassEntry);
    }

    if(status != SUCCESS)
    {
        runtimeError("unable to initialize connection info" TSRMLS_CC);
        return false;
    }

    if(Ice::IPConnectionInfoPtr::dynamicCast(p))
    {
        Ice::IPConnectionInfoPtr info = Ice::IPConnectionInfoPtr::dynamicCast(p);
        add_property_string(zv, STRCAST("localAddress"), const_cast<char*>(info->localAddress.c_str()), 1);
        add_property_long(zv, STRCAST("localPort"), static_cast<long>(info->localPort));
        add_property_string(zv, STRCAST("remoteAddress"), const_cast<char*>(info->remoteAddress.c_str()), 1);
        add_property_long(zv, STRCAST("remotePort"), static_cast<long>(info->remotePort));
    }

    add_property_bool(zv, STRCAST("incoming"), p->incoming ? 1 : 0);
    add_property_string(zv, STRCAST("adapterName"), const_cast<char*>(p->adapterName.c_str()), 1);

    Wrapper<Ice::ConnectionInfoPtr>* obj = Wrapper<Ice::ConnectionInfoPtr>::extract(zv TSRMLS_CC);
    assert(obj);
    assert(!obj->ptr);
    obj->ptr = new Ice::ConnectionInfoPtr(p);

    return true;
}
