//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include "Connection.h"
#include "Endpoint.h"
#include "Types.h"
#include "Util.h"

#include <IceSSL/IceSSL.h>

using namespace std;
using namespace IcePHP;

ZEND_EXTERN_MODULE_GLOBALS(ice)

// Class entries represent the PHP class implementations we have registered.
namespace IcePHP
{

zend_class_entry* connectionClassEntry = 0;

}

static zend_class_entry* connectionInfoClassEntry = 0;
static zend_class_entry* ipConnectionInfoClassEntry = 0;
static zend_class_entry* tcpConnectionInfoClassEntry = 0;
static zend_class_entry* udpConnectionInfoClassEntry = 0;
static zend_class_entry* wsConnectionInfoClassEntry = 0;
static zend_class_entry* sslConnectionInfoClassEntry = 0;

// Ice::Connection support.
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
        RETURN_STRINGL(str.c_str(), static_cast<int>(str.length()));
    }
    catch(const IceUtil::Exception& ex)
    {
        throwException(ex);
        RETURN_NULL();
    }
}

ZEND_BEGIN_ARG_INFO_EX(Ice_Connection_close_arginfo, 1, ZEND_RETURN_VALUE, static_cast<zend_ulong>(1))
    ZEND_ARG_INFO(0, mode)
ZEND_END_ARG_INFO()

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

ZEND_BEGIN_ARG_INFO_EX(Ice_Connection_flushBatchRequests_arginfo, 1, ZEND_RETURN_VALUE, static_cast<zend_ulong>(1))
    ZEND_ARG_INFO(0, compress)
ZEND_END_ARG_INFO()

ZEND_METHOD(Ice_Connection, flushBatchRequests)
{
    zval* compress;
    if(zend_parse_parameters(ZEND_NUM_ARGS(), const_cast<char*>("z"), &compress) != SUCCESS)
    {
        RETURN_NULL();
    }

    if(Z_TYPE_P(compress) != IS_LONG)
    {
        invalidArgument("value for 'compress' argument must be an enumerator of CompressBatch");
        RETURN_NULL();
    }
    Ice::CompressBatch cb = static_cast<Ice::CompressBatch>(Z_LVAL_P(compress));

    Ice::ConnectionPtr _this = Wrapper<Ice::ConnectionPtr>::value(getThis());
    assert(_this);

    try
    {
        _this->flushBatchRequests(cb);
    }
    catch(const IceUtil::Exception& ex)
    {
        throwException(ex);
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

ZEND_BEGIN_ARG_INFO_EX(Ice_Connection_setACM_arginfo, 1, ZEND_RETURN_VALUE, static_cast<zend_ulong>(3))
    ZEND_ARG_INFO(0, timeout)
    ZEND_ARG_INFO(0, close)
    ZEND_ARG_INFO(0, heartbeat)
ZEND_END_ARG_INFO()

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

    optional<Ice::Int> timeout;
    optional<Ice::ACMClose> close;
    optional<Ice::ACMHeartbeat> heartbeat;

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

        add_property_long(return_value, "timeout", static_cast<long>(acm.timeout));
        add_property_long(return_value, "close", static_cast<long>(acm.close));
        add_property_long(return_value, "heartbeat", static_cast<long>(acm.heartbeat));
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
        RETURN_STRINGL(str.c_str(), str.length());
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

ZEND_BEGIN_ARG_INFO_EX(Ice_Connection_setBufferSize_arginfo, 1, ZEND_RETURN_VALUE, static_cast<zend_ulong>(2))
    ZEND_ARG_INFO(0, rcvSize)
    ZEND_ARG_INFO(0, sndSize)
ZEND_END_ARG_INFO()

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

static zend_object*
handleConnectionAlloc(zend_class_entry* ce)
{
    Wrapper<Ice::ConnectionPtr>* obj = Wrapper<Ice::ConnectionPtr>::create(ce);
    assert(obj);
    obj->zobj.handlers = &_connectionHandlers;
    return &obj->zobj;
}

static void
handleConnectionFreeStorage(zend_object* object)
{
    Wrapper<Ice::ConnectionPtr>* obj = Wrapper<Ice::ConnectionPtr>::fetch(object);
    delete obj->ptr;
    zend_object_std_dtor(object);
}

static int
handleConnectionCompare(zval* zobj1, zval* zobj2)
{
    // PHP guarantees that the objects have the same class.
    Ice::ConnectionPtr con1 = Wrapper<Ice::ConnectionPtr>::value(zobj1);
    assert(con1);
    Ice::ConnectionPtr con2 = Wrapper<Ice::ConnectionPtr>::value(zobj2);
    assert(con2);

    if (con1 == con2)
    {
        return 0;
    }
    else if (con1 < con2)
    {
        return -1;
    }
    else
    {
        return 1;
    }
}

// Predefined methods for Connection.
static zend_function_entry _interfaceMethods[] =
{
    {0, 0, 0}
};

static zend_function_entry _connectionClassMethods[] =
{
    ZEND_ME(Ice_Connection, __construct, ice_void_arginfo, ZEND_ACC_PRIVATE|ZEND_ACC_CTOR)
    ZEND_ME(Ice_Connection, __toString, ice_to_string_arginfo, ZEND_ACC_PUBLIC)
    ZEND_ME(Ice_Connection, close, Ice_Connection_close_arginfo, ZEND_ACC_PUBLIC)
    ZEND_ME(Ice_Connection, getEndpoint, ice_void_arginfo, ZEND_ACC_PUBLIC)
    ZEND_ME(Ice_Connection, flushBatchRequests, Ice_Connection_flushBatchRequests_arginfo, ZEND_ACC_PUBLIC)
    ZEND_ME(Ice_Connection, heartbeat, ice_void_arginfo, ZEND_ACC_PUBLIC)
    ZEND_ME(Ice_Connection, setACM, Ice_Connection_setACM_arginfo, ZEND_ACC_PUBLIC)
    ZEND_ME(Ice_Connection, getACM, ice_void_arginfo, ZEND_ACC_PUBLIC)
    ZEND_ME(Ice_Connection, type, ice_void_arginfo, ZEND_ACC_PUBLIC)
    ZEND_ME(Ice_Connection, timeout, ice_void_arginfo, ZEND_ACC_PUBLIC)
    ZEND_ME(Ice_Connection, toString, ice_void_arginfo, ZEND_ACC_PUBLIC)
    ZEND_ME(Ice_Connection, getInfo, ice_void_arginfo, ZEND_ACC_PUBLIC)
    ZEND_ME(Ice_Connection, setBufferSize, Ice_Connection_setBufferSize_arginfo, ZEND_ACC_PUBLIC)
    ZEND_ME(Ice_Connection, throwException, ice_void_arginfo, ZEND_ACC_PUBLIC)
    {0, 0, 0}
};

ZEND_METHOD(Ice_ConnectionInfo, __construct)
{
    runtimeError("ConnectionInfo cannot be instantiated");
}

// Predefined methods for ConnectionInfo.
static zend_function_entry _connectionInfoClassMethods[] =
{
    ZEND_ME(Ice_ConnectionInfo, __construct, ice_void_arginfo, ZEND_ACC_PRIVATE|ZEND_ACC_CTOR)
    {0, 0, 0}
};

static zend_object*
handleConnectionInfoAlloc(zend_class_entry* ce)
{
    Wrapper<Ice::ConnectionInfoPtr>* obj = Wrapper<Ice::ConnectionInfoPtr>::create(ce);
    assert(obj);

    obj->zobj.handlers = &_connectionInfoHandlers;

    return &obj->zobj;
}

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
    // We register an interface and a class that implements the interface. This allows  applications to safely include
    // the Slice-generated code for the type.

    // Register the Connection interface.
    zend_class_entry ce;
    INIT_NS_CLASS_ENTRY(ce, "Ice", "Connection", _interfaceMethods);
    zend_class_entry* interface = zend_register_internal_interface(&ce);

    // Register the Connection class.
    INIT_CLASS_ENTRY(ce, "IcePHP_Connection", _connectionClassMethods);
    ce.create_object = handleConnectionAlloc;
    connectionClassEntry = zend_register_internal_class(&ce);
    memcpy(&_connectionHandlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
    _connectionHandlers.compare = handleConnectionCompare;
    _connectionHandlers.free_obj = handleConnectionFreeStorage;
    _connectionHandlers.offset = XtOffsetOf(Wrapper<Ice::ConnectionPtr>, zobj);
    zend_class_implements(connectionClassEntry, 1, interface);

    // Register the ConnectionInfo class.
    INIT_NS_CLASS_ENTRY(ce, "Ice", "ConnectionInfo", _connectionInfoClassMethods);
    ce.create_object = handleConnectionInfoAlloc;
    connectionInfoClassEntry = zend_register_internal_class(&ce);
    memcpy(&_connectionInfoHandlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
    _connectionInfoHandlers.free_obj = handleConnectionInfoFreeStorage;
    _connectionInfoHandlers.offset = XtOffsetOf(Wrapper<Ice::ConnectionInfoPtr>, zobj);

    zend_declare_property_bool(connectionInfoClassEntry, "incoming", sizeof("incoming") - 1, 0, ZEND_ACC_PUBLIC);
    zend_declare_property_string(
        connectionInfoClassEntry,
        "adapterName",
        sizeof("adapterName") - 1,
        "",
        ZEND_ACC_PUBLIC);
    zend_declare_property_null(connectionInfoClassEntry, "underlying", sizeof("underlying") - 1, ZEND_ACC_PUBLIC);

    // Register the IPConnectionInfo class.
    INIT_NS_CLASS_ENTRY(ce, "Ice", "IPConnectionInfo", nullptr);
    ce.create_object = handleConnectionInfoAlloc;
    ipConnectionInfoClassEntry = zend_register_internal_class_ex(&ce, connectionInfoClassEntry);
    zend_declare_property_string(
        ipConnectionInfoClassEntry,
        "localAddress",
        sizeof("localAddress") - 1,
        "",
        ZEND_ACC_PUBLIC);
    zend_declare_property_long(ipConnectionInfoClassEntry, "localPort", sizeof("localPort") - 1, 0, ZEND_ACC_PUBLIC);
    zend_declare_property_string(
        ipConnectionInfoClassEntry,
        "remoteAddress",
        sizeof("remoteAddress") - 1,
        "",
        ZEND_ACC_PUBLIC);
    zend_declare_property_long(ipConnectionInfoClassEntry, "remotePort", sizeof("remotePort") - 1, 0, ZEND_ACC_PUBLIC);

    // Register the TCPConnectionInfo class.
    INIT_NS_CLASS_ENTRY(ce, "Ice", "TCPConnectionInfo", nullptr);
    ce.create_object = handleConnectionInfoAlloc;
    tcpConnectionInfoClassEntry = zend_register_internal_class_ex(&ce, ipConnectionInfoClassEntry);
    zend_declare_property_long(tcpConnectionInfoClassEntry, "rcvSize", sizeof("rcvSize") - 1, 0, ZEND_ACC_PUBLIC);
    zend_declare_property_long(tcpConnectionInfoClassEntry, "sndSize", sizeof("sndSize") - 1, 0, ZEND_ACC_PUBLIC);

    // Register the UDPConnectionInfo class.
    INIT_NS_CLASS_ENTRY(ce, "Ice", "UDPConnectionInfo", nullptr);
    ce.create_object = handleConnectionInfoAlloc;
    udpConnectionInfoClassEntry = zend_register_internal_class_ex(&ce, ipConnectionInfoClassEntry);
    zend_declare_property_string(
        udpConnectionInfoClassEntry,
        "mcastAddress",
        sizeof("mcastAddress") - 1,
        "",
        ZEND_ACC_PUBLIC);
    zend_declare_property_long(udpConnectionInfoClassEntry, "mcastPort", sizeof("mcastPort") - 1, 0, ZEND_ACC_PUBLIC);

    // Register the WSConnectionInfo class.
    INIT_NS_CLASS_ENTRY(ce, "Ice", "WSConnectionInfo", nullptr);
    ce.create_object = handleConnectionInfoAlloc;
    wsConnectionInfoClassEntry = zend_register_internal_class_ex(&ce, connectionInfoClassEntry);
    zend_declare_property_string(wsConnectionInfoClassEntry, "headers", sizeof("headers") - 1, "", ZEND_ACC_PUBLIC);

    // Register the SSLConnectionInfo class.
    INIT_NS_CLASS_ENTRY(ce, "Ice", "SSLConnectionInfo", nullptr);
    ce.create_object = handleConnectionInfoAlloc;
    sslConnectionInfoClassEntry = zend_register_internal_class_ex(&ce, connectionInfoClassEntry);
    zend_declare_property_string(sslConnectionInfoClassEntry, "cipher", sizeof("cipher") - 1, "", ZEND_ACC_PUBLIC);
    zend_declare_property_string(sslConnectionInfoClassEntry, "certs", sizeof("certs") - 1, "", ZEND_ACC_PUBLIC);
    zend_declare_property_bool(sslConnectionInfoClassEntry, "verified", sizeof("verified") - 1, 0, ZEND_ACC_PUBLIC);

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
    if(dynamic_pointer_cast<Ice::WSConnectionInfo>(p))
    {
        auto info = dynamic_pointer_cast<Ice::WSConnectionInfo>(p);
        if((status = object_init_ex(zv, wsConnectionInfoClassEntry)) == SUCCESS)
        {
            zval zmap;
            AutoDestroy mapDestroyer(&zmap);
            if(createStringMap(&zmap, info->headers))
            {
                add_property_zval(zv, "headers", &zmap);
            }
            else
            {
                return false;
            }
        }
    }
    else if(dynamic_pointer_cast<Ice::TCPConnectionInfo>(p))
    {
        auto info = dynamic_pointer_cast<Ice::TCPConnectionInfo>(p);
        if((status = object_init_ex(zv, tcpConnectionInfoClassEntry)) == SUCCESS)
        {
            add_property_long(zv, "rcvSize", static_cast<long>(info->rcvSize));
            add_property_long(zv, "sndSize", static_cast<long>(info->sndSize));
        }
    }
    else if(dynamic_pointer_cast<Ice::UDPConnectionInfo>(p))
    {
        auto info = dynamic_pointer_cast<Ice::UDPConnectionInfo>(p);
        if((status = object_init_ex(zv, udpConnectionInfoClassEntry)) == SUCCESS)
        {
            add_property_string(zv, "mcastAddress", const_cast<char*>(info->mcastAddress.c_str()));
            add_property_long(zv, "mcastPort", static_cast<long>(info->mcastPort));
            add_property_long(zv, "rcvSize", static_cast<long>(info->rcvSize));
            add_property_long(zv, "sndSize", static_cast<long>(info->sndSize));
        }
    }
    else if(dynamic_pointer_cast<IceSSL::ConnectionInfo>(p))
    {
        status = object_init_ex(zv, sslConnectionInfoClassEntry);
    }
    else if(dynamic_pointer_cast<Ice::IPConnectionInfo>(p))
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

    if(dynamic_pointer_cast<IceSSL::ConnectionInfo>(p))
    {
        auto info = dynamic_pointer_cast<IceSSL::ConnectionInfo>(p);
        add_property_string(zv, "cipher", const_cast<char*>(info->cipher.c_str()));
        add_property_bool(zv, "verified", info->verified ? 1 : 0);

        zval zarr;
        AutoDestroy listDestroyer(&zarr);

        Ice::StringSeq encoded;
        transform(
            info->certs.cbegin(),
            info->certs.cend(),
            back_inserter(encoded),
            [](const auto& cert) { return cert->encode(); });

        if(createStringArray(&zarr, encoded))
        {
            add_property_zval(zv, "certs", &zarr);
        }
        else
        {
            return false;
        }
    }

    if(dynamic_pointer_cast<Ice::IPConnectionInfo>(p))
    {
        auto info = dynamic_pointer_cast<Ice::IPConnectionInfo>(p);
        add_property_string(zv, "localAddress", const_cast<char*>(info->localAddress.c_str()));
        add_property_long(zv, "localPort", static_cast<long>(info->localPort));
        add_property_string(zv, "remoteAddress", const_cast<char*>(info->remoteAddress.c_str()));
        add_property_long(zv, "remotePort", static_cast<long>(info->remotePort));
    }

    zval underlying;
    if(!createConnectionInfo(&underlying, p->underlying))
    {
        runtimeError("unable to initialize connection info");
        return false;
    }
    add_property_zval(zv, "underlying", &underlying);
    zval_ptr_dtor(&underlying); // add_property_zval increased the refcount of underlying
    add_property_bool(zv, "incoming", p->incoming ? 1 : 0);
    add_property_string(zv, "adapterName", const_cast<char*>(p->adapterName.c_str()));

    Wrapper<Ice::ConnectionInfoPtr>* obj = Wrapper<Ice::ConnectionInfoPtr>::extract(zv);
    assert(obj);
    assert(!obj->ptr);
    obj->ptr = new Ice::ConnectionInfoPtr(p);

    return true;
}
