// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Connection.h>
#include <Endpoint.h>
#include <Types.h>
#include <Util.h>
#include <IceSSL/IceSSL.h>

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
static zend_class_entry* wsConnectionInfoClassEntry = 0;
static zend_class_entry* sslConnectionInfoClassEntry = 0;

//
// Ice::Connection support.
//
static zend_object_handlers _connectionHandlers;
static zend_object_handlers _connectionInfoHandlers;

extern "C"
{
static zend_object* handleConnectionAlloc(zend_class_entry*);
static void handleConnectionFreeStorage(zend_object*);
static int handleConnectionCompare(zval*, zval*);

static zend_object* handleConnectionInfoAlloc(zend_class_entry*);
static void handleConnectionInfoFreeStorage(zend_object*);
}

ZEND_METHOD(Ice_Connection, __construct)
{
    runtimeError("Connection cannot be instantiated");
}

ZEND_METHOD(Ice_Connection, __toString)
{
    if(ZEND_NUM_ARGS() > 0)
    {
        WRONG_PARAM_COUNT;
    }

    Ice::ConnectionPtr _this = Wrapper<Ice::ConnectionPtr>::value(getThis());
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

ZEND_METHOD(Ice_Connection, close)
{
    Ice::ConnectionPtr _this = Wrapper<Ice::ConnectionPtr>::value(getThis());
    assert(_this);

    zval* mode;
    if(zend_parse_parameters(ZEND_NUM_ARGS(), const_cast<char*>("z"), &mode) != SUCCESS)
    {
        RETURN_NULL();
    }

    if(Z_TYPE_P(mode) != IS_LONG)
    {
        invalidArgument("value for 'mode' argument must be an enumerator of ConnectionClose");
        RETURN_NULL();
    }
    Ice::ConnectionClose cc = static_cast<Ice::ConnectionClose>(Z_LVAL_P(mode));

    try
    {
        _this->close(cc);
    }
    catch(const IceUtil::Exception& ex)
    {
        throwException(ex);
        RETURN_NULL();
    }
}

ZEND_METHOD(Ice_Connection, getEndpoint)
{
    if(ZEND_NUM_ARGS() > 0)
    {
        WRONG_PARAM_COUNT;
    }

    Ice::ConnectionPtr _this = Wrapper<Ice::ConnectionPtr>::value(getThis());
    assert(_this);

    try
    {
        if(!createEndpoint(return_value, _this->getEndpoint()))
        {
            RETURN_NULL();
        }
    }
    catch(const IceUtil::Exception& ex)
    {
        throwException(ex);
        RETURN_NULL();
    }
}

ZEND_METHOD(Ice_Connection, flushBatchRequests)
{
    zval* compress;
    if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, const_cast<char*>("z"), &compress TSRMLS_CC) != SUCCESS)
    {
        RETURN_NULL();
    }

    if(Z_TYPE_P(compress) != IS_LONG)
    {
        invalidArgument("value for 'compress' argument must be an enumerator of CompressBatch" TSRMLS_CC);
        RETURN_NULL();
    }
    Ice::CompressBatch cb = static_cast<Ice::CompressBatch>(Z_LVAL_P(compress));

    Ice::ConnectionPtr _this = Wrapper<Ice::ConnectionPtr>::value(getThis() TSRMLS_CC);
    assert(_this);

    try
    {
        _this->flushBatchRequests(cb);
    }
    catch(const IceUtil::Exception& ex)
    {
        throwException(ex TSRMLS_CC);
        RETURN_NULL();
    }
}

ZEND_METHOD(Ice_Connection, heartbeat)
{
    if(ZEND_NUM_ARGS() > 0)
    {
        WRONG_PARAM_COUNT;
    }

    Ice::ConnectionPtr _this = Wrapper<Ice::ConnectionPtr>::value(getThis());
    assert(_this);

    try
    {
        _this->heartbeat();
    }
    catch(const IceUtil::Exception& ex)
    {
        throwException(ex);
        RETURN_NULL();
    }
}

ZEND_METHOD(Ice_Connection, setACM)
{
    Ice::ConnectionPtr _this = Wrapper<Ice::ConnectionPtr>::value(getThis());
    assert(_this);

    zval* t;
    zval* c;
    zval* h;
    if(zend_parse_parameters(ZEND_NUM_ARGS(), const_cast<char*>("zzz"), &t, &c, &h) != SUCCESS)
    {
        RETURN_NULL();
    }

    IceUtil::Optional<Ice::Int> timeout;
    IceUtil::Optional<Ice::ACMClose> close;
    IceUtil::Optional<Ice::ACMHeartbeat> heartbeat;

    if(!isUnset(t))
    {
        if(Z_TYPE_P(t) != IS_LONG)
        {
            invalidArgument("value for 'timeout' argument must be Unset or an integer");
            RETURN_NULL();
        }
        timeout = static_cast<Ice::Int>(Z_LVAL_P(t));
    }

    if(!isUnset(c))
    {
        if(Z_TYPE_P(c) != IS_LONG)
        {
            invalidArgument("value for 'close' argument must be Unset or an enumerator of ACMClose");
            RETURN_NULL();
        }
        close = static_cast<Ice::ACMClose>(Z_LVAL_P(c));
    }

    if(!isUnset(h))
    {
        if(Z_TYPE_P(h) != IS_LONG)
        {
            invalidArgument("value for 'heartbeat' argument must be Unset or an enumerator of ACMHeartbeat");
            RETURN_NULL();
        }
        heartbeat = static_cast<Ice::ACMHeartbeat>(Z_LVAL_P(h));
    }

    try
    {
        _this->setACM(timeout, close, heartbeat);
    }
    catch(const IceUtil::Exception& ex)
    {
        throwException(ex);
        RETURN_NULL();
    }
}

ZEND_METHOD(Ice_Connection, getACM)
{
    if(ZEND_NUM_ARGS() > 0)
    {
        WRONG_PARAM_COUNT;
    }

    Ice::ConnectionPtr _this = Wrapper<Ice::ConnectionPtr>::value(getThis());
    assert(_this);

    try
    {
        Ice::ACM acm = _this->getACM();

        zend_class_entry* acmClass = idToClass("::Ice::ACM");

        if(object_init_ex(return_value, const_cast<zend_class_entry*>(acmClass)) != SUCCESS)
        {
            runtimeError("unable to initialize object of type %s", acmClass->name);
            RETURN_NULL();
        }

        add_property_long(return_value, STRCAST("timeout"), static_cast<long>(acm.timeout));
        add_property_long(return_value, STRCAST("close"), static_cast<long>(acm.close));
        add_property_long(return_value, STRCAST("heartbeat"), static_cast<long>(acm.heartbeat));
    }
    catch(const IceUtil::Exception& ex)
    {
        throwException(ex);
        RETURN_NULL();
    }
}

ZEND_METHOD(Ice_Connection, type)
{
    if(ZEND_NUM_ARGS() > 0)
    {
        WRONG_PARAM_COUNT;
    }

    Ice::ConnectionPtr _this = Wrapper<Ice::ConnectionPtr>::value(getThis());
    assert(_this);

    try
    {
        string str = _this->type();
        RETURN_STRINGL(STRCAST(str.c_str()), str.length());
    }
    catch(const IceUtil::Exception& ex)
    {
        throwException(ex);
        RETURN_NULL();
    }
}

ZEND_METHOD(Ice_Connection, timeout)
{
    if(ZEND_NUM_ARGS() != 0)
    {
        WRONG_PARAM_COUNT;
    }

    Ice::ConnectionPtr _this = Wrapper<Ice::ConnectionPtr>::value(getThis());
    assert(_this);

    try
    {
        Ice::Int timeout = _this->timeout();
        ZVAL_LONG(return_value, static_cast<long>(timeout));
    }
    catch(const IceUtil::Exception& ex)
    {
        throwException(ex);
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

    Ice::ConnectionPtr _this = Wrapper<Ice::ConnectionPtr>::value(getThis());
    assert(_this);

    try
    {
        Ice::ConnectionInfoPtr info = _this->getInfo();
        if(!createConnectionInfo(return_value, _this->getInfo()))
        {
            RETURN_NULL();
        }
    }
    catch(const IceUtil::Exception& ex)
    {
        throwException(ex);
        RETURN_NULL();
    }
}

ZEND_METHOD(Ice_Connection, setBufferSize)
{
    Ice::ConnectionPtr _this = Wrapper<Ice::ConnectionPtr>::value(getThis());
    assert(_this);

    zval* r;
    zval* s;
    if(zend_parse_parameters(ZEND_NUM_ARGS(), const_cast<char*>("zz"), &r, &s) != SUCCESS)
    {
        RETURN_NULL();
    }

    int rcvSize = static_cast<int>(Z_LVAL_P(r));
    int sndSize = static_cast<int>(Z_LVAL_P(s));

    try
    {
        _this->setBufferSize(rcvSize, sndSize);
    }
    catch(const IceUtil::Exception& ex)
    {
        throwException(ex);
        RETURN_NULL();
    }
}

ZEND_METHOD(Ice_Connection, throwException)
{
    if(ZEND_NUM_ARGS() > 0)
    {
        WRONG_PARAM_COUNT;
    }

    Ice::ConnectionPtr _this = Wrapper<Ice::ConnectionPtr>::value(getThis());
    assert(_this);

    try
    {
        _this->throwException();
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
handleConnectionAlloc(zend_class_entry* ce)
{
    Wrapper<Ice::ConnectionPtr>* obj = Wrapper<Ice::ConnectionPtr>::create(ce);
    assert(obj);

    obj->zobj.handlers = &_connectionHandlers;

    return &obj->zobj;
}

#ifdef _WIN32
extern "C"
#endif
static void
handleConnectionFreeStorage(zend_object* object)
{
    Wrapper<Ice::ConnectionPtr>* obj = Wrapper<Ice::ConnectionPtr>::fetch(object);
    delete obj->ptr;
    zend_object_std_dtor(object);
}

#ifdef _WIN32
extern "C"
#endif
static int
handleConnectionCompare(zval* zobj1, zval* zobj2)
{
    //
    // PHP guarantees that the objects have the same class.
    //

    Ice::ConnectionPtr con1 = Wrapper<Ice::ConnectionPtr>::value(zobj1);
    assert(con1);
    Ice::ConnectionPtr con2 = Wrapper<Ice::ConnectionPtr>::value(zobj2);
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
    ZEND_ME(Ice_Connection, __construct, ICE_NULLPTR, ZEND_ACC_PRIVATE|ZEND_ACC_CTOR)
    ZEND_ME(Ice_Connection, __toString, ICE_NULLPTR, ZEND_ACC_PUBLIC)
    ZEND_ME(Ice_Connection, close, ICE_NULLPTR, ZEND_ACC_PUBLIC)
    ZEND_ME(Ice_Connection, getEndpoint, ICE_NULLPTR, ZEND_ACC_PUBLIC)
    ZEND_ME(Ice_Connection, flushBatchRequests, ICE_NULLPTR, ZEND_ACC_PUBLIC)
    ZEND_ME(Ice_Connection, heartbeat, ICE_NULLPTR, ZEND_ACC_PUBLIC)
    ZEND_ME(Ice_Connection, setACM, ICE_NULLPTR, ZEND_ACC_PUBLIC)
    ZEND_ME(Ice_Connection, getACM, ICE_NULLPTR, ZEND_ACC_PUBLIC)
    ZEND_ME(Ice_Connection, type, ICE_NULLPTR, ZEND_ACC_PUBLIC)
    ZEND_ME(Ice_Connection, timeout, ICE_NULLPTR, ZEND_ACC_PUBLIC)
    ZEND_ME(Ice_Connection, toString, ICE_NULLPTR, ZEND_ACC_PUBLIC)
    ZEND_ME(Ice_Connection, getInfo, ICE_NULLPTR, ZEND_ACC_PUBLIC)
    ZEND_ME(Ice_Connection, setBufferSize, ICE_NULLPTR, ZEND_ACC_PUBLIC)
    ZEND_ME(Ice_Connection, throwException, ICE_NULLPTR, ZEND_ACC_PUBLIC)
    {0, 0, 0}
};

ZEND_METHOD(Ice_ConnectionInfo, __construct)
{
    runtimeError("ConnectionInfo cannot be instantiated");
}

//
// Predefined methods for ConnectionInfo.
//
static zend_function_entry _connectionInfoClassMethods[] =
{
    ZEND_ME(Ice_ConnectionInfo, __construct, ICE_NULLPTR, ZEND_ACC_PRIVATE|ZEND_ACC_CTOR)
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
static zend_object*
handleConnectionInfoAlloc(zend_class_entry* ce)
{
    Wrapper<Ice::ConnectionInfoPtr>* obj = Wrapper<Ice::ConnectionInfoPtr>::create(ce);
    assert(obj);

    obj->zobj.handlers = &_connectionInfoHandlers;

    return &obj->zobj;
}

#ifdef _WIN32
extern "C"
#endif
static void
handleConnectionInfoFreeStorage(zend_object* object)
{
    Wrapper<Ice::ConnectionInfoPtr>* obj = Wrapper<Ice::ConnectionInfoPtr>::fetch(object);
    delete obj->ptr;
    zend_object_std_dtor(object);
}

bool
IcePHP::connectionInit(void)
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
    zend_class_entry* interface = zend_register_internal_interface(&ce);

    //
    // Register the Connection class.
    //
    INIT_CLASS_ENTRY(ce, "IcePHP_Connection", _connectionClassMethods);
    ce.create_object = handleConnectionAlloc;
    connectionClassEntry = zend_register_internal_class(&ce);
    memcpy(&_connectionHandlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
    _connectionHandlers.compare_objects = handleConnectionCompare;
    _connectionHandlers.free_obj = handleConnectionFreeStorage;
    _connectionHandlers.offset = XtOffsetOf(Wrapper<Ice::ConnectionPtr>, zobj);
    zend_class_implements(connectionClassEntry, 1, interface);

    //
    // Register the ConnectionInfo class.
    //
#ifdef ICEPHP_USE_NAMESPACES
    INIT_NS_CLASS_ENTRY(ce, "Ice", "ConnectionInfo", _connectionInfoClassMethods);
#else
    INIT_CLASS_ENTRY(ce, "Ice_ConnectionInfo", _connectionInfoClassMethods);
#endif
    ce.create_object = handleConnectionInfoAlloc;
    connectionInfoClassEntry = zend_register_internal_class(&ce);
    memcpy(&_connectionInfoHandlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
    _connectionInfoHandlers.free_obj = handleConnectionInfoFreeStorage;
    _connectionInfoHandlers.offset = XtOffsetOf(Wrapper<Ice::ConnectionInfoPtr>, zobj);
    zend_declare_property_bool(connectionInfoClassEntry, STRCAST("incoming"), sizeof("incoming") - 1, 0,
                               ZEND_ACC_PUBLIC);
    zend_declare_property_string(connectionInfoClassEntry, STRCAST("adapterName"), sizeof("adapterName") - 1,
                                 STRCAST(""), ZEND_ACC_PUBLIC);

    //
    // Register the IPConnectionInfo class.
    //
#ifdef ICEPHP_USE_NAMESPACES
    INIT_NS_CLASS_ENTRY(ce, "Ice", "IPConnectionInfo", ICE_NULLPTR);
#else
    INIT_CLASS_ENTRY(ce, "Ice_IPConnectionInfo", ICE_NULLPTR);
#endif
    ce.create_object = handleConnectionInfoAlloc;
    ipConnectionInfoClassEntry = zend_register_internal_class_ex(&ce, connectionInfoClassEntry);
    zend_declare_property_string(ipConnectionInfoClassEntry, STRCAST("localAddress"), sizeof("localAddress") - 1,
                                 STRCAST(""), ZEND_ACC_PUBLIC);
    zend_declare_property_long(ipConnectionInfoClassEntry, STRCAST("localPort"), sizeof("localPort") - 1, 0,
                               ZEND_ACC_PUBLIC);
    zend_declare_property_string(ipConnectionInfoClassEntry, STRCAST("remoteAddress"), sizeof("remoteAddress") - 1,
                                 STRCAST(""), ZEND_ACC_PUBLIC);
    zend_declare_property_long(ipConnectionInfoClassEntry, STRCAST("remotePort"), sizeof("remotePort") - 1, 0,
                               ZEND_ACC_PUBLIC);

    //
    // Register the TCPConnectionInfo class.
    //
#ifdef ICEPHP_USE_NAMESPACES
    INIT_NS_CLASS_ENTRY(ce, "Ice", "TCPConnectionInfo", ICE_NULLPTR);
#else
    INIT_CLASS_ENTRY(ce, "Ice_TCPConnectionInfo", ICE_NULLPTR);
#endif
    ce.create_object = handleConnectionInfoAlloc;
    tcpConnectionInfoClassEntry = zend_register_internal_class_ex(&ce, ipConnectionInfoClassEntry);

    //
    // Register the UDPConnectionInfo class.
    //
#ifdef ICEPHP_USE_NAMESPACES
    INIT_NS_CLASS_ENTRY(ce, "Ice", "UDPConnectionInfo", ICE_NULLPTR);
#else
    INIT_CLASS_ENTRY(ce, "Ice_UDPConnectionInfo", ICE_NULLPTR);
#endif
    ce.create_object = handleConnectionInfoAlloc;
    udpConnectionInfoClassEntry = zend_register_internal_class_ex(&ce, ipConnectionInfoClassEntry);
    zend_declare_property_string(udpConnectionInfoClassEntry, STRCAST("mcastAddress"), sizeof("mcastAddress") - 1,
                                 STRCAST(""), ZEND_ACC_PUBLIC);
    zend_declare_property_long(udpConnectionInfoClassEntry, STRCAST("mcastPort"), sizeof("mcastPort") - 1, 0,
                               ZEND_ACC_PUBLIC);

    //
    // Register the WSConnectionInfo class.
    //
#ifdef ICEPHP_USE_NAMESPACES
    INIT_NS_CLASS_ENTRY(ce, "Ice", "WSConnectionInfo", ICE_NULLPTR);
#else
    INIT_CLASS_ENTRY(ce, "Ice_WSConnectionInfo", ICE_NULLPTR);
#endif
    ce.create_object = handleConnectionInfoAlloc;
    wsConnectionInfoClassEntry = zend_register_internal_class_ex(&ce, connectionInfoClassEntry);
    zend_declare_property_string(wsConnectionInfoClassEntry, STRCAST("headers"), sizeof("headers") - 1,
                                 STRCAST(""), ZEND_ACC_PUBLIC);

    //
    // Register the SSLConnectionInfo class.
    //
#ifdef ICEPHP_USE_NAMESPACES
    INIT_NS_CLASS_ENTRY(ce, "Ice", "SSLConnectionInfo", ICE_NULLPTR);
#else
    INIT_CLASS_ENTRY(ce, "Ice_SSLConnectionInfo", ICE_NULLPTR);
#endif
    ce.create_object = handleConnectionInfoAlloc;
    sslConnectionInfoClassEntry = zend_register_internal_class_ex(&ce, connectionInfoClassEntry);
    zend_declare_property_string(sslConnectionInfoClassEntry, STRCAST("cipher"), sizeof("cipher") - 1,
                                 STRCAST(""), ZEND_ACC_PUBLIC);
    zend_declare_property_string(sslConnectionInfoClassEntry, STRCAST("certs"), sizeof("certs") - 1,
                                 STRCAST(""), ZEND_ACC_PUBLIC);
    zend_declare_property_bool(sslConnectionInfoClassEntry, STRCAST("verified"), sizeof("verified") - 1, 0,
                               ZEND_ACC_PUBLIC);

    return true;
}

bool
IcePHP::createConnection(zval* zv, const Ice::ConnectionPtr& p)
{
    if(object_init_ex(zv, connectionClassEntry) != SUCCESS)
    {
        runtimeError("unable to initialize connection");
        return false;
    }

    Wrapper<Ice::ConnectionPtr>* obj = Wrapper<Ice::ConnectionPtr>::extract(zv);
    assert(obj);
    assert(!obj->ptr);
    obj->ptr = new Ice::ConnectionPtr(p);

    return true;
}

bool
IcePHP::fetchConnection(zval* zv, Ice::ConnectionPtr& connection)
{
    if(ZVAL_IS_NULL(zv))
    {
        connection = 0;
    }
    else
    {
        if(Z_TYPE_P(zv) != IS_OBJECT || Z_OBJCE_P(zv) != connectionClassEntry)
        {
            invalidArgument("value is not a connection");
            return false;
        }
        Wrapper<Ice::ConnectionPtr>* obj = Wrapper<Ice::ConnectionPtr>::extract(zv);
        if(!obj)
        {
            return false;
        }
        connection = *obj->ptr;
    }
    return true;
}

bool
IcePHP::createConnectionInfo(zval* zv, const Ice::ConnectionInfoPtr& p)
{
    if(!p)
    {
        ZVAL_NULL(zv);
        return true;
    }

    int status;
    if(Ice::WSConnectionInfoPtr::dynamicCast(p))
    {
        Ice::WSConnectionInfoPtr info = Ice::WSConnectionInfoPtr::dynamicCast(p);
        if((status = object_init_ex(zv, wsConnectionInfoClassEntry)) == SUCCESS)
        {
            zval zmap;
            AutoDestroy mapDestroyer(&zmap);
            if(createStringMap(&zmap, info->headers))
            {
                add_property_zval(zv, STRCAST("headers"), &zmap);
            }
            else
            {
                return false;
            }
        }
    }
    else if(Ice::TCPConnectionInfoPtr::dynamicCast(p))
    {
        Ice::TCPConnectionInfoPtr info = Ice::TCPConnectionInfoPtr::dynamicCast(p);
        if((status = object_init_ex(zv, tcpConnectionInfoClassEntry)) == SUCCESS)
        {
            add_property_long(zv, STRCAST("rcvSize"), static_cast<long>(info->rcvSize));
            add_property_long(zv, STRCAST("sndSize"), static_cast<long>(info->sndSize));
        }
    }
    else if(Ice::UDPConnectionInfoPtr::dynamicCast(p))
    {
        Ice::UDPConnectionInfoPtr info = Ice::UDPConnectionInfoPtr::dynamicCast(p);
        if((status = object_init_ex(zv, udpConnectionInfoClassEntry)) == SUCCESS)
        {
            add_property_string(zv, STRCAST("mcastAddress"), const_cast<char*>(info->mcastAddress.c_str()));
            add_property_long(zv, STRCAST("mcastPort"), static_cast<long>(info->mcastPort));
            add_property_long(zv, STRCAST("rcvSize"), static_cast<long>(info->rcvSize));
            add_property_long(zv, STRCAST("sndSize"), static_cast<long>(info->sndSize));
        }
    }
    else if(IceSSL::ConnectionInfoPtr::dynamicCast(p))
    {
        status = object_init_ex(zv, sslConnectionInfoClassEntry);
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
        runtimeError("unable to initialize connection info");
        return false;
    }

    if(IceSSL::ConnectionInfoPtr::dynamicCast(p))
    {
        IceSSL::ConnectionInfoPtr info = IceSSL::ConnectionInfoPtr::dynamicCast(p);
        add_property_string(zv, STRCAST("cipher"), const_cast<char*>(info->cipher.c_str()));
        add_property_bool(zv, STRCAST("verified"), info->verified ? 1 : 0);

        zval zarr;
        AutoDestroy listDestroyer(&zarr);

        Ice::StringSeq encoded;
        for(vector<IceSSL::CertificatePtr>::const_iterator i = info->certs.begin(); i != info->certs.end(); ++i)
        {
            encoded.push_back((*i)->encode());
        }

        if(createStringArray(&zarr, encoded))
        {
            add_property_zval(zv, STRCAST("certs"), &zarr);
        }
        else
        {
            return false;
        }
    }

    if(Ice::IPConnectionInfoPtr::dynamicCast(p))
    {
        Ice::IPConnectionInfoPtr info = Ice::IPConnectionInfoPtr::dynamicCast(p);
        add_property_string(zv, STRCAST("localAddress"), const_cast<char*>(info->localAddress.c_str()));
        add_property_long(zv, STRCAST("localPort"), static_cast<long>(info->localPort));
        add_property_string(zv, STRCAST("remoteAddress"), const_cast<char*>(info->remoteAddress.c_str()));
        add_property_long(zv, STRCAST("remotePort"), static_cast<long>(info->remotePort));
    }

    zval underlying;
    if(!createConnectionInfo(&underlying, p->underlying TSRMLS_CC))
    {
        runtimeError("unable to initialize connection info" TSRMLS_CC);
        return false;
    }
    add_property_zval(zv, STRCAST("underlying"), &underlying);
    zval_ptr_dtor(&underlying); // add_property_zval increased the refcount of underlying
    add_property_bool(zv, STRCAST("incoming"), p->incoming ? 1 : 0);
    add_property_string(zv, STRCAST("adapterName"), const_cast<char*>(p->adapterName.c_str()));

    Wrapper<Ice::ConnectionInfoPtr>* obj = Wrapper<Ice::ConnectionInfoPtr>::extract(zv);
    assert(obj);
    assert(!obj->ptr);
    obj->ptr = new Ice::ConnectionInfoPtr(p);

    return true;
}
