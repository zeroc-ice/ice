// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/DisableWarnings.h>
#include <Proxy.h>
#include <Communicator.h>
#include <Marshal.h>
#include <Profile.h>
#include <Util.h>

using namespace std;
using namespace IcePHP;

ZEND_EXTERN_MODULE_GLOBALS(ice)

//
// Here's a brief description of how proxies are handled by this extension.
//
// A single PHP class, Ice_ObjectPrx, is registered. This is an "internal" class,
// i.e., implemented by this extension, and it is used to represent all proxies
// regardless of interface type.
//
// Like in C++, a proxy is only capable of invoking the Ice::ObjectPrx operations
// until it is narrowed with a checked or unchecked cast. Unlike C++, no PHP classes
// are created for proxies, because all marshaling activity is driven by the Slice
// definitions, not by statically-generated code.
//
// In order to perform a checked or unchecked cast, the user invokes ice_checkedCast
// or ice_uncheckedCast on the proxy to be narrowed, supplying a scoped name for the
// desired type. Internally, the proxy validates the scoped name and returns a new
// proxy containing the Slice class or interface definition. This proxy is considered
// to be narrowed to that interface and therefore supports user-defined operations.
//
// Naturally, there are many predefined proxy methods (e.g., ice_isA, etc.), but
// the proxy also needs to support user-defined operations (if it has type information).
// We use a Zend API hook that allows us to intercept the invocation of unknown methods
// on the proxy object. At this point, the proxy checks the interface definition for
// an operation with the given name, and then creates an Operation object (see below)
// that is responsible for invoking the operation. The proxy caches the Operation objects
// for future reuse.
//

//
// Class entries represent the PHP class implementations we have registered.
//
namespace IcePHP
{
zend_class_entry* proxyClassEntry = 0;
zend_class_entry* endpointClassEntry = 0;
zend_class_entry* connectionClassEntry = 0;
}

//
// Ice::ObjectPrx and Ice::Endpoint support.
//
static zend_object_handlers _proxyHandlers;
static zend_object_handlers _endpointHandlers;
static zend_object_handlers _connectionHandlers;

extern "C"
{
static zend_object_value handleProxyAlloc(zend_class_entry* TSRMLS_DC);
static void handleProxyFreeStorage(void* TSRMLS_DC);
static zend_object_value handleProxyClone(zval* TSRMLS_DC);
static union _zend_function* handleProxyGetMethod(zval**, char*, int TSRMLS_DC);
static int handleProxyCompare(zval*, zval* TSRMLS_DC);
ZEND_FUNCTION(Ice_ObjectPrx_call);

static zend_object_value handleEndpointAlloc(zend_class_entry* TSRMLS_DC);
static void handleEndpointFreeStorage(void* TSRMLS_DC);

static zend_object_value handleConnectionAlloc(zend_class_entry* TSRMLS_DC);
static void handleConnectionFreeStorage(void* TSRMLS_DC);
static int handleConnectionCompare(zval*, zval* TSRMLS_DC);
}

static bool lookupClass(const string&, Slice::ClassDefPtr& TSRMLS_DC);

namespace IcePHP
{

//
// Encapsulates an operation description.
//
class Operation : public IceUtil::SimpleShared
{
public:
    Operation(const Ice::ObjectPrx&, const string&, const Slice::OperationPtr&, const Ice::CommunicatorPtr&
              TSRMLS_DC);
    virtual ~Operation();

    zend_function* getZendFunction() const;
    void invoke(INTERNAL_FUNCTION_PARAMETERS);

private:
    void throwUserException(Ice::InputStreamPtr& TSRMLS_DC);

    Ice::ObjectPrx _proxy;
    string _name; // Local name, not the on-the-wire name
    Slice::OperationPtr _op;
    Ice::CommunicatorPtr _communicator;
#ifdef ZTS
    TSRMLS_D;
#endif
    vector<string> _paramNames;
    MarshalerPtr _result;
    vector<MarshalerPtr> _inParams;
    vector<MarshalerPtr> _outParams;
    zend_internal_function* _zendFunction;
};
typedef IceUtil::Handle<Operation> OperationPtr;

//
// Encapsulates proxy and type information.
//
class Proxy
{
public:
    Proxy(const Ice::ObjectPrx&, const Slice::ClassDefPtr& TSRMLS_DC);
    ~Proxy();

    const Ice::ObjectPrx& getProxy() const;
    const Slice::ClassDefPtr& getClass() const;

    OperationPtr getOperation(const string&);

    string toString() const;

private:
    Ice::ObjectPrx _proxy;
    Slice::ClassDefPtr _class;
#ifdef ZTS
    TSRMLS_D;
#endif
    zval _communicatorZval;
    Ice::CommunicatorPtr _communicator;
    Slice::OperationList _classOps;
    map<string, OperationPtr> _ops;
};

} // End of namespace IcePHP

//
// Predefined methods for Ice_ObjectPrx.
//
static function_entry _proxyMethods[] =
{
    {"__construct",                PHP_FN(Ice_ObjectPrx___construct),                0},
    {"__tostring",                 PHP_FN(Ice_ObjectPrx___tostring),                 0},
    {"ice_getCommunicator",        PHP_FN(Ice_ObjectPrx_ice_getCommunicator),        0},
    {"ice_toString",               PHP_FN(Ice_ObjectPrx_ice_toString),               0},
    {"ice_isA",                    PHP_FN(Ice_ObjectPrx_ice_isA),                    0},
    {"ice_ping",                   PHP_FN(Ice_ObjectPrx_ice_ping),                   0},
    {"ice_id",                     PHP_FN(Ice_ObjectPrx_ice_id),                     0},
    {"ice_ids",                    PHP_FN(Ice_ObjectPrx_ice_ids),                    0},
    {"ice_getIdentity",            PHP_FN(Ice_ObjectPrx_ice_getIdentity),            0},
    {"ice_newIdentity",            PHP_FN(Ice_ObjectPrx_ice_identity),               0},
    {"ice_identity",               PHP_FN(Ice_ObjectPrx_ice_identity),               0},
    {"ice_getContext",             PHP_FN(Ice_ObjectPrx_ice_getContext),             0},
    {"ice_newContext",             PHP_FN(Ice_ObjectPrx_ice_context),                0},
    {"ice_context",                PHP_FN(Ice_ObjectPrx_ice_context),                0},
    {"ice_defaultContext",         PHP_FN(Ice_ObjectPrx_ice_defaultContext),         0},
    {"ice_getFacet",               PHP_FN(Ice_ObjectPrx_ice_getFacet),               0},
    {"ice_newFacet",               PHP_FN(Ice_ObjectPrx_ice_facet),                  0},
    {"ice_facet",                  PHP_FN(Ice_ObjectPrx_ice_facet),                  0},
    {"ice_getAdapterId",           PHP_FN(Ice_ObjectPrx_ice_getAdapterId),           0},
    {"ice_newAdapterId",           PHP_FN(Ice_ObjectPrx_ice_adapterId),              0},
    {"ice_adapterId",              PHP_FN(Ice_ObjectPrx_ice_adapterId),              0},
    {"ice_getEndpoints",           PHP_FN(Ice_ObjectPrx_ice_getEndpoints),           0},
    {"ice_newEndpoints",           PHP_FN(Ice_ObjectPrx_ice_endpoints),              0},
    {"ice_endpoints",              PHP_FN(Ice_ObjectPrx_ice_endpoints),              0},
    {"ice_getLocatorCacheTimeout", PHP_FN(Ice_ObjectPrx_ice_getLocatorCacheTimeout), 0},
    {"ice_locatorCacheTimeout",    PHP_FN(Ice_ObjectPrx_ice_locatorCacheTimeout),    0},
    {"ice_isConnectionCached",     PHP_FN(Ice_ObjectPrx_ice_isConnectionCached),     0},
    {"ice_connectionCached",       PHP_FN(Ice_ObjectPrx_ice_connectionCached),       0},
    {"ice_getEndpointSelection",   PHP_FN(Ice_ObjectPrx_ice_getEndpointSelection),   0},
    {"ice_endpointSelection",      PHP_FN(Ice_ObjectPrx_ice_endpointSelection),      0},
    {"ice_isSecure",               PHP_FN(Ice_ObjectPrx_ice_isSecure),               0},
    {"ice_secure",                 PHP_FN(Ice_ObjectPrx_ice_secure),                 0},
    {"ice_isPreferSecure",         PHP_FN(Ice_ObjectPrx_ice_isPreferSecure),         0},
    {"ice_preferSecure",           PHP_FN(Ice_ObjectPrx_ice_preferSecure),           0},
    {"ice_getRouter",              PHP_FN(Ice_ObjectPrx_ice_getRouter),              0},
    {"ice_router",                 PHP_FN(Ice_ObjectPrx_ice_router),                 0},
    {"ice_getLocator",             PHP_FN(Ice_ObjectPrx_ice_getLocator),             0},
    {"ice_locator",                PHP_FN(Ice_ObjectPrx_ice_locator),                0},
    {"ice_twoway",                 PHP_FN(Ice_ObjectPrx_ice_twoway),                 0},
    {"ice_isTwoway",               PHP_FN(Ice_ObjectPrx_ice_isTwoway),               0},
    {"ice_oneway",                 PHP_FN(Ice_ObjectPrx_ice_oneway),                 0},
    {"ice_isOneway",               PHP_FN(Ice_ObjectPrx_ice_isOneway),               0},
    {"ice_batchOneway",            PHP_FN(Ice_ObjectPrx_ice_batchOneway),            0},
    {"ice_isBatchOneway",          PHP_FN(Ice_ObjectPrx_ice_isBatchOneway),          0},
    {"ice_datagram",               PHP_FN(Ice_ObjectPrx_ice_datagram),               0},
    {"ice_isDatagram",             PHP_FN(Ice_ObjectPrx_ice_isDatagram),             0},
    {"ice_batchDatagram",          PHP_FN(Ice_ObjectPrx_ice_batchDatagram),          0},
    {"ice_isBatchDatagram",        PHP_FN(Ice_ObjectPrx_ice_isBatchDatagram),        0},
    {"ice_compress",               PHP_FN(Ice_ObjectPrx_ice_compress),               0},
    {"ice_timeout",                PHP_FN(Ice_ObjectPrx_ice_timeout),                0},
    {"ice_connectionId",           PHP_FN(Ice_ObjectPrx_ice_connectionId),           0},
    {"ice_isThreadPerConnection",  PHP_FN(Ice_ObjectPrx_ice_isThreadPerConnection),  0},
    {"ice_threadPerConnection",    PHP_FN(Ice_ObjectPrx_ice_threadPerConnection),    0},
    {"ice_getConnection",          PHP_FN(Ice_ObjectPrx_ice_getConnection),          0},
    {"ice_getCachedConnection",    PHP_FN(Ice_ObjectPrx_ice_getCachedConnection),    0},
    {"ice_uncheckedCast",          PHP_FN(Ice_ObjectPrx_ice_uncheckedCast),          0},
    {"ice_checkedCast",            PHP_FN(Ice_ObjectPrx_ice_checkedCast),            0},
    {0, 0, 0}
};

//
// Predefined methods for Ice_Endpoint.
//
static function_entry _endpointMethods[] =
{
    {"__construct", PHP_FN(Ice_Endpoint___construct), 0},
    {"__tostring",  PHP_FN(Ice_Endpoint___tostring),  0},
    {"toString",    PHP_FN(Ice_Endpoint_toString),    0},
    {0, 0, 0}
};

//
// Predefined methods for Ice_Connection.
//
static function_entry _connectionMethods[] =
{
    {"__construct",         PHP_FN(Ice_Connection___construct),         0},
    {"__tostring",          PHP_FN(Ice_Connection___tostring),          0},
    {"close",               PHP_FN(Ice_Connection_close),               0},
    {"flushBatchRequests",  PHP_FN(Ice_Connection_flushBatchRequests),  0},
    {"type",                PHP_FN(Ice_Connection_type),                0},
    {"timeout",             PHP_FN(Ice_Connection_timeout),             0},
    {"toString",            PHP_FN(Ice_Connection_toString),            0},
    {0, 0, 0}
};

bool
IcePHP::proxyInit(TSRMLS_D)
{
    //
    // Register the Ice_ObjectPrx class.
    //
    zend_class_entry ce;
    INIT_CLASS_ENTRY(ce, "Ice_ObjectPrx", _proxyMethods);
    ce.create_object = handleProxyAlloc;
    proxyClassEntry = zend_register_internal_class(&ce TSRMLS_CC);
    memcpy(&_proxyHandlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
    _proxyHandlers.clone_obj = handleProxyClone;
    _proxyHandlers.get_method = handleProxyGetMethod;
    _proxyHandlers.compare_objects = handleProxyCompare;

    //
    // Register the Ice_Endpoint class.
    //
    INIT_CLASS_ENTRY(ce, "Ice_Endpoint", _endpointMethods);
    ce.create_object = handleEndpointAlloc;
    endpointClassEntry = zend_register_internal_class(&ce TSRMLS_CC);
    memcpy(&_endpointHandlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));

    //
    // Register the Ice_Connection class.
    //
    INIT_CLASS_ENTRY(ce, "Ice_Connection", _connectionMethods);
    ce.create_object = handleConnectionAlloc;
    connectionClassEntry = zend_register_internal_class(&ce TSRMLS_CC);
    memcpy(&_connectionHandlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
    _connectionHandlers.compare_objects = handleConnectionCompare;

    return true;
}

bool
IcePHP::createProxy(zval* zv, const Ice::ObjectPrx& p TSRMLS_DC)
{
    return createProxy(zv, p, 0 TSRMLS_CC);
}

bool
IcePHP::createProxy(zval* zv, const Ice::ObjectPrx& p, const Slice::ClassDefPtr& def TSRMLS_DC)
{
    if(object_init_ex(zv, proxyClassEntry) != SUCCESS)
    {
        php_error_docref(0 TSRMLS_CC, E_ERROR, "unable to initialize proxy");
        return false;
    }

    ice_object* zprx = static_cast<ice_object*>(zend_object_store_get_object(zv TSRMLS_CC));
    assert(!zprx->ptr);
    zprx->ptr = new Proxy(p, def TSRMLS_CC);

    return true;
}

bool
IcePHP::fetchProxy(zval* zv, Ice::ObjectPrx& prx, Slice::ClassDefPtr& def TSRMLS_DC)
{
    if(!ZVAL_IS_NULL(zv))
    {
        void* p = zend_object_store_get_object(zv TSRMLS_CC);
        if(!p)
        {
            php_error_docref(0 TSRMLS_CC, E_ERROR, "unable to retrieve proxy object from object store");
            return false;
        }
        if(Z_OBJCE_P(zv) != proxyClassEntry)
        {
            php_error_docref(0 TSRMLS_CC, E_ERROR, "value is not a proxy");
            return false;
        }
        ice_object* obj = static_cast<ice_object*>(p);
        assert(obj->ptr);
        Proxy* proxy = static_cast<Proxy*>(obj->ptr);
        prx = proxy->getProxy();
        def = proxy->getClass();
    }
    return true;
}

static bool
createEndpoint(zval* zv, const Ice::EndpointPtr& p TSRMLS_DC)
{
    if(object_init_ex(zv, endpointClassEntry) != SUCCESS)
    {
        php_error_docref(0 TSRMLS_CC, E_ERROR, "unable to initialize endpoint");
        return false;
    }

    ice_object* ze = static_cast<ice_object*>(zend_object_store_get_object(zv TSRMLS_CC));
    assert(!ze->ptr);
    ze->ptr = new Ice::EndpointPtr(p);

    return true;
}

static bool
fetchEndpoint(zval* zv, Ice::EndpointPtr& endpoint TSRMLS_DC)
{
    if(!ZVAL_IS_NULL(zv))
    {
        void* p = zend_object_store_get_object(zv TSRMLS_CC);
        if(!p)
        {
            php_error_docref(0 TSRMLS_CC, E_ERROR, "unable to retrieve endpoint object from object store");
            return false;
        }
        if(Z_OBJCE_P(zv) != endpointClassEntry)
        {
            php_error_docref(0 TSRMLS_CC, E_ERROR, "value is not an endpoint");
            return false;
        }
        ice_object* obj = static_cast<ice_object*>(p);
        assert(obj->ptr);
        Ice::EndpointPtr* pe = static_cast<Ice::EndpointPtr*>(obj->ptr);
        endpoint = *pe;
    }
    return true;
}

static bool
createConnection(zval* zv, const Ice::ConnectionPtr& p TSRMLS_DC)
{
    if(object_init_ex(zv, connectionClassEntry) != SUCCESS)
    {
        php_error_docref(0 TSRMLS_CC, E_ERROR, "unable to initialize connection");
        return false;
    }

    ice_object* ze = static_cast<ice_object*>(zend_object_store_get_object(zv TSRMLS_CC));
    assert(!ze->ptr);
    ze->ptr = new Ice::ConnectionPtr(p);

    return true;
}

ZEND_FUNCTION(Ice_ObjectPrx___construct)
{
    php_error_docref(0 TSRMLS_CC, E_ERROR, "Ice_ObjectPrx cannot be instantiated, use $ICE->stringToProxy()");
}

ZEND_FUNCTION(Ice_ObjectPrx___tostring)
{
    if(ZEND_NUM_ARGS() > 0)
    {
        WRONG_PARAM_COUNT;
    }

    ice_object* obj = static_cast<ice_object*>(zend_object_store_get_object(getThis() TSRMLS_CC));
    assert(obj->ptr);
    Proxy* _this = static_cast<Proxy*>(obj->ptr);

    try
    {
        string str = _this->toString();
        RETURN_STRINGL(const_cast<char*>(str.c_str()), str.length(), 1);
    }
    catch(const IceUtil::Exception& ex)
    {
        throwException(ex TSRMLS_CC);
        RETURN_NULL();
    }
}

ZEND_FUNCTION(Ice_ObjectPrx_ice_getCommunicator)
{
    zval* zc = getCommunicatorZval(TSRMLS_C);

    Z_TYPE_P(return_value) = IS_OBJECT;
    return_value->value.obj = zc->value.obj;
    Z_OBJ_HT_P(return_value)->add_ref(return_value TSRMLS_CC);
}

ZEND_FUNCTION(Ice_ObjectPrx_ice_toString)
{
    ZEND_FN(Ice_ObjectPrx___tostring)(INTERNAL_FUNCTION_PARAM_PASSTHRU);
}

ZEND_FUNCTION(Ice_ObjectPrx_ice_isA)
{
    if(ZEND_NUM_ARGS() < 1 || ZEND_NUM_ARGS() > 2)
    {
        WRONG_PARAM_COUNT;
    }

    char* id;
    int len;
    zval* arr = 0;

    if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|a", &id, &len, &arr) == FAILURE)
    {
        RETURN_FALSE;
    }

    //
    // Populate the context (if necessary).
    //
    Ice::Context ctx;
    if(arr && !extractContext(arr, ctx TSRMLS_CC))
    {
        RETURN_FALSE;
    }

    ice_object* obj = static_cast<ice_object*>(zend_object_store_get_object(getThis() TSRMLS_CC));
    assert(obj->ptr);
    Proxy* _this = static_cast<Proxy*>(obj->ptr);

    try
    {
        bool b;
        if(arr)
        {
            b = _this->getProxy()->ice_isA(id, ctx);
        }
        else
        {
            b = _this->getProxy()->ice_isA(id);
        }

        RETURN_BOOL(b ? 1 : 0);
    }
    catch(const IceUtil::Exception& ex)
    {
        throwException(ex TSRMLS_CC);
        RETVAL_FALSE;
    }
}

ZEND_FUNCTION(Ice_ObjectPrx_ice_ping)
{
    if(ZEND_NUM_ARGS() > 1)
    {
        WRONG_PARAM_COUNT;
    }

    zval* arr = 0;

    if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|a", &arr) == FAILURE)
    {
        RETURN_NULL();
    }

    //
    // Populate the context (if necessary).
    //
    Ice::Context ctx;
    if(arr && !extractContext(arr, ctx TSRMLS_CC))
    {
        RETURN_NULL();
    }

    ice_object* obj = static_cast<ice_object*>(zend_object_store_get_object(getThis() TSRMLS_CC));
    assert(obj->ptr);
    Proxy* _this = static_cast<Proxy*>(obj->ptr);

    try
    {
        if(arr)
        {
            _this->getProxy()->ice_ping(ctx);
        }
        else
        {
            _this->getProxy()->ice_ping();
        }
    }
    catch(const IceUtil::Exception& ex)
    {
        throwException(ex TSRMLS_CC);
    }

    RETURN_NULL();
}

ZEND_FUNCTION(Ice_ObjectPrx_ice_id)
{
    if(ZEND_NUM_ARGS() > 1)
    {
        WRONG_PARAM_COUNT;
    }

    zval* arr = 0;

    if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|a", &arr) == FAILURE)
    {
        RETURN_NULL();
    }

    //
    // Populate the context (if necessary).
    //
    Ice::Context ctx;
    if(arr && !extractContext(arr, ctx TSRMLS_CC))
    {
        RETURN_NULL();
    }

    ice_object* obj = static_cast<ice_object*>(zend_object_store_get_object(getThis() TSRMLS_CC));
    assert(obj->ptr);
    Proxy* _this = static_cast<Proxy*>(obj->ptr);

    try
    {
        string id;
        if(arr)
        {
            id = _this->getProxy()->ice_id(ctx);
        }
        else
        {
            id = _this->getProxy()->ice_id();
        }
        RETURN_STRINGL(const_cast<char*>(id.c_str()), id.length(), 1);
    }
    catch(const IceUtil::Exception& ex)
    {
        throwException(ex TSRMLS_CC);
        RETURN_NULL();
    }
}

ZEND_FUNCTION(Ice_ObjectPrx_ice_ids)
{
    if(ZEND_NUM_ARGS() > 1)
    {
        WRONG_PARAM_COUNT;
    }

    zval* arr = 0;

    if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|a", &arr) == FAILURE)
    {
        RETURN_NULL();
    }

    //
    // Populate the context (if necessary).
    //
    Ice::Context ctx;
    if(arr && !extractContext(arr, ctx TSRMLS_CC))
    {
        RETURN_NULL();
    }

    ice_object* obj = static_cast<ice_object*>(zend_object_store_get_object(getThis() TSRMLS_CC));
    assert(obj->ptr);
    Proxy* _this = static_cast<Proxy*>(obj->ptr);

    try
    {
        vector<string> ids;
        if(arr)
        {
            ids = _this->getProxy()->ice_ids(ctx);
        }
        else
        {
            ids = _this->getProxy()->ice_ids();
        }

        array_init(return_value);
        uint idx = 0;
        for(vector<string>::const_iterator p = ids.begin(); p != ids.end(); ++p, ++idx)
        {
            add_index_stringl(return_value, idx, const_cast<char*>((*p).c_str()), (*p).length(), 1);
        }
    }
    catch(const IceUtil::Exception& ex)
    {
        throwException(ex TSRMLS_CC);
        RETURN_NULL();
    }
}

ZEND_FUNCTION(Ice_ObjectPrx_ice_getIdentity)
{
    if(ZEND_NUM_ARGS() != 0)
    {
        WRONG_PARAM_COUNT;
    }

    ice_object* obj = static_cast<ice_object*>(zend_object_store_get_object(getThis() TSRMLS_CC));
    assert(obj->ptr);
    Proxy* _this = static_cast<Proxy*>(obj->ptr);

    createIdentity(return_value, _this->getProxy()->ice_getIdentity() TSRMLS_CC);
}

ZEND_FUNCTION(Ice_ObjectPrx_ice_identity)
{
    if(ZEND_NUM_ARGS() != 1)
    {
        WRONG_PARAM_COUNT;
    }

    ice_object* obj = static_cast<ice_object*>(zend_object_store_get_object(getThis() TSRMLS_CC));
    assert(obj->ptr);
    Proxy* _this = static_cast<Proxy*>(obj->ptr);

    zend_class_entry* cls = findClass("Ice_Identity" TSRMLS_CC);
    assert(cls);

    zval *zid;

    if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "O", &zid, cls) == FAILURE)
    {
        RETURN_NULL();
    }

    Ice::Identity id;
    if(extractIdentity(zid, id TSRMLS_CC))
    {
        try
        {
            Ice::ObjectPrx prx = _this->getProxy()->ice_identity(id);
            if(!createProxy(return_value, prx TSRMLS_CC))
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
}

ZEND_FUNCTION(Ice_ObjectPrx_ice_getContext)
{
    if(ZEND_NUM_ARGS() != 0)
    {
        WRONG_PARAM_COUNT;
    }

    ice_object* obj = static_cast<ice_object*>(zend_object_store_get_object(getThis() TSRMLS_CC));
    assert(obj->ptr);
    Proxy* _this = static_cast<Proxy*>(obj->ptr);

    createContext(return_value, _this->getProxy()->ice_getContext() TSRMLS_CC);
}

ZEND_FUNCTION(Ice_ObjectPrx_ice_context)
{
    if(ZEND_NUM_ARGS() != 1)
    {
        WRONG_PARAM_COUNT;
    }

    zval* arr = 0;

    if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "a", &arr) == FAILURE)
    {
        RETURN_NULL();
    }

    //
    // Populate the context.
    //
    Ice::Context ctx;
    if(arr && !extractContext(arr, ctx TSRMLS_CC))
    {
        RETURN_NULL();
    }

    ice_object* obj = static_cast<ice_object*>(zend_object_store_get_object(getThis() TSRMLS_CC));
    assert(obj->ptr);
    Proxy* _this = static_cast<Proxy*>(obj->ptr);

    try
    {
        Ice::ObjectPrx prx = _this->getProxy()->ice_context(ctx);
        if(!createProxy(return_value, prx, _this->getClass() TSRMLS_CC))
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

ZEND_FUNCTION(Ice_ObjectPrx_ice_defaultContext)
{
    if(ZEND_NUM_ARGS() != 0)
    {
        WRONG_PARAM_COUNT;
    }

    ice_object* obj = static_cast<ice_object*>(zend_object_store_get_object(getThis() TSRMLS_CC));
    assert(obj->ptr);
    Proxy* _this = static_cast<Proxy*>(obj->ptr);

    try
    {
        Ice::ObjectPrx prx = _this->getProxy()->ice_defaultContext();
        if(!createProxy(return_value, prx, _this->getClass() TSRMLS_CC))
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

ZEND_FUNCTION(Ice_ObjectPrx_ice_getFacet)
{
    if(ZEND_NUM_ARGS() != 0)
    {
        WRONG_PARAM_COUNT;
    }

    ice_object* obj = static_cast<ice_object*>(zend_object_store_get_object(getThis() TSRMLS_CC));
    assert(obj->ptr);
    Proxy* _this = static_cast<Proxy*>(obj->ptr);

    try
    {
        string facet = _this->getProxy()->ice_getFacet();
        ZVAL_STRINGL(return_value, const_cast<char*>(facet.c_str()), facet.length(), 1);
    }
    catch(const IceUtil::Exception& ex)
    {
        throwException(ex TSRMLS_CC);
        RETURN_NULL();
    }
}

ZEND_FUNCTION(Ice_ObjectPrx_ice_facet)
{
    if(ZEND_NUM_ARGS() != 1)
    {
        WRONG_PARAM_COUNT;
    }

    char* name;
    int len;

    if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &name, &len) == FAILURE)
    {
        RETURN_NULL();
    }

    ice_object* obj = static_cast<ice_object*>(zend_object_store_get_object(getThis() TSRMLS_CC));
    assert(obj->ptr);
    Proxy* _this = static_cast<Proxy*>(obj->ptr);

    try
    {
        Ice::ObjectPrx prx = _this->getProxy()->ice_facet(name);
        if(!createProxy(return_value, prx TSRMLS_CC))
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

ZEND_FUNCTION(Ice_ObjectPrx_ice_getAdapterId)
{
    if(ZEND_NUM_ARGS() != 0)
    {
        WRONG_PARAM_COUNT;
    }

    ice_object* obj = static_cast<ice_object*>(zend_object_store_get_object(getThis() TSRMLS_CC));
    assert(obj->ptr);
    Proxy* _this = static_cast<Proxy*>(obj->ptr);

    try
    {
        string id = _this->getProxy()->ice_getAdapterId();
        ZVAL_STRINGL(return_value, const_cast<char*>(id.c_str()), id.length(), 1);
    }
    catch(const IceUtil::Exception& ex)
    {
        throwException(ex TSRMLS_CC);
        RETURN_NULL();
    }
}

ZEND_FUNCTION(Ice_ObjectPrx_ice_adapterId)
{
    if(ZEND_NUM_ARGS() != 1)
    {
        WRONG_PARAM_COUNT;
    }

    ice_object* obj = static_cast<ice_object*>(zend_object_store_get_object(getThis() TSRMLS_CC));
    assert(obj->ptr);
    Proxy* _this = static_cast<Proxy*>(obj->ptr);

    char* id;
    int len;

    if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &id, &len) == FAILURE)
    {
        RETURN_NULL();
    }

    try
    {
        Ice::ObjectPrx prx = _this->getProxy()->ice_adapterId(id);
        if(!createProxy(return_value, prx, _this->getClass() TSRMLS_CC))
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

ZEND_FUNCTION(Ice_ObjectPrx_ice_getEndpoints)
{
    if(ZEND_NUM_ARGS() != 0)
    {
        WRONG_PARAM_COUNT;
    }

    ice_object* obj = static_cast<ice_object*>(zend_object_store_get_object(getThis() TSRMLS_CC));
    assert(obj->ptr);
    Proxy* _this = static_cast<Proxy*>(obj->ptr);

    try
    {
        Ice::EndpointSeq endpoints = _this->getProxy()->ice_getEndpoints();

        array_init(return_value);
        uint idx = 0;
        for(Ice::EndpointSeq::const_iterator p = endpoints.begin(); p != endpoints.end(); ++p, ++idx)
        {
            zval* elem;
            MAKE_STD_ZVAL(elem);
            if(!createEndpoint(elem, *p TSRMLS_CC))
            {
                zval_ptr_dtor(&elem);
                RETURN_NULL();
            }
            add_index_zval(return_value, idx, elem);
        }
    }
    catch(const IceUtil::Exception& ex)
    {
        throwException(ex TSRMLS_CC);
        RETURN_NULL();
    }
}

ZEND_FUNCTION(Ice_ObjectPrx_ice_endpoints)
{
    if(ZEND_NUM_ARGS() != 1)
    {
        WRONG_PARAM_COUNT;
    }

    ice_object* obj = static_cast<ice_object*>(zend_object_store_get_object(getThis() TSRMLS_CC));
    assert(obj->ptr);
    Proxy* _this = static_cast<Proxy*>(obj->ptr);

    zval* zv;

    if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "a", &zv) == FAILURE)
    {
        RETURN_NULL();
    }

    Ice::EndpointSeq seq;

    HashTable* arr = Z_ARRVAL_P(zv);
    HashPosition pos;
    zval** val;

    zend_hash_internal_pointer_reset_ex(arr, &pos);
    while(zend_hash_get_current_data_ex(arr, reinterpret_cast<void**>(&val), &pos) != FAILURE)
    {
        if(Z_TYPE_PP(val) != IS_OBJECT)
        {
            php_error_docref(0 TSRMLS_CC, E_ERROR, "expected an element of type Ice_Endpoint");
            RETURN_NULL();
        }

        Ice::EndpointPtr endpoint;
        if(!fetchEndpoint(*val, endpoint TSRMLS_CC))
        {
            RETURN_NULL();
        }

        seq.push_back(endpoint);

        zend_hash_move_forward_ex(arr, &pos);
    }

    try
    {
        Ice::ObjectPrx prx = _this->getProxy()->ice_endpoints(seq);
        if(!createProxy(return_value, prx, _this->getClass() TSRMLS_CC))
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

ZEND_FUNCTION(Ice_ObjectPrx_ice_getLocatorCacheTimeout)
{
    if(ZEND_NUM_ARGS() != 0)
    {
        WRONG_PARAM_COUNT;
    }

    ice_object* obj = static_cast<ice_object*>(zend_object_store_get_object(getThis() TSRMLS_CC));
    assert(obj->ptr);
    Proxy* _this = static_cast<Proxy*>(obj->ptr);

    try
    {
        Ice::Int timeout = _this->getProxy()->ice_getLocatorCacheTimeout();
        ZVAL_LONG(return_value, static_cast<long>(timeout));
    }
    catch(const IceUtil::Exception& ex)
    {
        throwException(ex TSRMLS_CC);
        RETURN_NULL();
    }
}

ZEND_FUNCTION(Ice_ObjectPrx_ice_locatorCacheTimeout)
{
    if(ZEND_NUM_ARGS() != 1)
    {
        WRONG_PARAM_COUNT;
    }

    ice_object* obj = static_cast<ice_object*>(zend_object_store_get_object(getThis() TSRMLS_CC));
    assert(obj->ptr);
    Proxy* _this = static_cast<Proxy*>(obj->ptr);

    long l;
    if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &l) != SUCCESS)
    {
        RETURN_NULL();
    }

    try
    {
        Ice::ObjectPrx prx = _this->getProxy()->ice_locatorCacheTimeout(l);
        if(!createProxy(return_value, prx, _this->getClass() TSRMLS_CC))
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

ZEND_FUNCTION(Ice_ObjectPrx_ice_isConnectionCached)
{
    if(ZEND_NUM_ARGS() != 0)
    {
        WRONG_PARAM_COUNT;
    }

    ice_object* obj = static_cast<ice_object*>(zend_object_store_get_object(getThis() TSRMLS_CC));
    assert(obj->ptr);
    Proxy* _this = static_cast<Proxy*>(obj->ptr);

    try
    {
        bool b = _this->getProxy()->ice_isConnectionCached();
        ZVAL_BOOL(return_value, b ? 1 : 0);
    }
    catch(const IceUtil::Exception& ex)
    {
        throwException(ex TSRMLS_CC);
        RETURN_NULL();
    }
}

ZEND_FUNCTION(Ice_ObjectPrx_ice_connectionCached)
{
    if(ZEND_NUM_ARGS() != 1)
    {
        WRONG_PARAM_COUNT;
    }

    ice_object* obj = static_cast<ice_object*>(zend_object_store_get_object(getThis() TSRMLS_CC));
    assert(obj->ptr);
    Proxy* _this = static_cast<Proxy*>(obj->ptr);

    zend_bool b;
    if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "b", &b) != SUCCESS)
    {
        RETURN_NULL();
    }

    try
    {
        Ice::ObjectPrx prx = _this->getProxy()->ice_connectionCached(b ? true : false);
        if(!createProxy(return_value, prx, _this->getClass() TSRMLS_CC))
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

ZEND_FUNCTION(Ice_ObjectPrx_ice_getEndpointSelection)
{
    if(ZEND_NUM_ARGS() != 0)
    {
        WRONG_PARAM_COUNT;
    }

    ice_object* obj = static_cast<ice_object*>(zend_object_store_get_object(getThis() TSRMLS_CC));
    assert(obj->ptr);
    Proxy* _this = static_cast<Proxy*>(obj->ptr);

    try
    {
        Ice::EndpointSelectionType type = _this->getProxy()->ice_getEndpointSelection();
        ZVAL_LONG(return_value, type == Ice::Random ? 0 : 1);
    }
    catch(const IceUtil::Exception& ex)
    {
        throwException(ex TSRMLS_CC);
        RETURN_NULL();
    }
}

ZEND_FUNCTION(Ice_ObjectPrx_ice_endpointSelection)
{
    if(ZEND_NUM_ARGS() != 1)
    {
        WRONG_PARAM_COUNT;
    }

    ice_object* obj = static_cast<ice_object*>(zend_object_store_get_object(getThis() TSRMLS_CC));
    assert(obj->ptr);
    Proxy* _this = static_cast<Proxy*>(obj->ptr);

    long l;
    if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &l) != SUCCESS)
    {
        RETURN_NULL();
    }

    if(l < 0 || l > 1)
    {
        php_error_docref(0 TSRMLS_CC, E_ERROR, "expecting Random or Ordered");
        RETURN_NULL();
    }

    try
    {
        Ice::ObjectPrx prx = _this->getProxy()->ice_endpointSelection(l == 0 ? Ice::Random : Ice::Ordered);
        if(!createProxy(return_value, prx, _this->getClass() TSRMLS_CC))
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

ZEND_FUNCTION(Ice_ObjectPrx_ice_isSecure)
{
    if(ZEND_NUM_ARGS() != 0)
    {
        WRONG_PARAM_COUNT;
    }

    ice_object* obj = static_cast<ice_object*>(zend_object_store_get_object(getThis() TSRMLS_CC));
    assert(obj->ptr);
    Proxy* _this = static_cast<Proxy*>(obj->ptr);

    try
    {
        bool b = _this->getProxy()->ice_isSecure();
        RETURN_BOOL(b ? 1 : 0);
    }
    catch(const IceUtil::Exception& ex)
    {
        throwException(ex TSRMLS_CC);
        RETURN_FALSE;
    }
}

ZEND_FUNCTION(Ice_ObjectPrx_ice_secure)
{
    if(ZEND_NUM_ARGS() != 1)
    {
        WRONG_PARAM_COUNT;
    }

    ice_object* obj = static_cast<ice_object*>(zend_object_store_get_object(getThis() TSRMLS_CC));
    assert(obj->ptr);
    Proxy* _this = static_cast<Proxy*>(obj->ptr);

    zend_bool b;
    if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "b", &b TSRMLS_CC) != SUCCESS)
    {
        RETURN_NULL();
    }

    try
    {
        Ice::ObjectPrx prx = _this->getProxy()->ice_secure(b ? true : false);
        if(!createProxy(return_value, prx, _this->getClass() TSRMLS_CC))
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

ZEND_FUNCTION(Ice_ObjectPrx_ice_isPreferSecure)
{
    if(ZEND_NUM_ARGS() != 0)
    {
        WRONG_PARAM_COUNT;
    }

    ice_object* obj = static_cast<ice_object*>(zend_object_store_get_object(getThis() TSRMLS_CC));
    assert(obj->ptr);
    Proxy* _this = static_cast<Proxy*>(obj->ptr);

    try
    {
        bool b = _this->getProxy()->ice_isPreferSecure();
        RETURN_BOOL(b ? 1 : 0);
    }
    catch(const IceUtil::Exception& ex)
    {
        throwException(ex TSRMLS_CC);
        RETURN_FALSE;
    }
}

ZEND_FUNCTION(Ice_ObjectPrx_ice_preferSecure)
{
    if(ZEND_NUM_ARGS() != 1)
    {
        WRONG_PARAM_COUNT;
    }

    ice_object* obj = static_cast<ice_object*>(zend_object_store_get_object(getThis() TSRMLS_CC));
    assert(obj->ptr);
    Proxy* _this = static_cast<Proxy*>(obj->ptr);

    zend_bool b;
    if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "b", &b TSRMLS_CC) != SUCCESS)
    {
        RETURN_NULL();
    }

    try
    {
        Ice::ObjectPrx prx = _this->getProxy()->ice_preferSecure(b ? true : false);
        if(!createProxy(return_value, prx, _this->getClass() TSRMLS_CC))
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

ZEND_FUNCTION(Ice_ObjectPrx_ice_getRouter)
{
    if(ZEND_NUM_ARGS() != 0)
    {
        WRONG_PARAM_COUNT;
    }

    ice_object* obj = static_cast<ice_object*>(zend_object_store_get_object(getThis() TSRMLS_CC));
    assert(obj->ptr);
    Proxy* _this = static_cast<Proxy*>(obj->ptr);

    try
    {
        Ice::RouterPrx router = _this->getProxy()->ice_getRouter();
        if(router)
        {
            Slice::ClassDefPtr def;
            if(!lookupClass("Ice::Router", def TSRMLS_CC))
            {
                RETURN_NULL();
            }

            assert(def);

            if(!createProxy(return_value, router, def TSRMLS_CC))
            {
                RETURN_NULL();
            }
        }
        else
        {
            RETURN_NULL();
        }
    }
    catch(const IceUtil::Exception& ex)
    {
        throwException(ex TSRMLS_CC);
        RETURN_FALSE;
    }
}

ZEND_FUNCTION(Ice_ObjectPrx_ice_router)
{
    if(ZEND_NUM_ARGS() != 1)
    {
        WRONG_PARAM_COUNT;
    }

    ice_object* obj = static_cast<ice_object*>(zend_object_store_get_object(getThis() TSRMLS_CC));
    assert(obj->ptr);
    Proxy* _this = static_cast<Proxy*>(obj->ptr);

    zval* zprx;
    if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "O!", &zprx, proxyClassEntry TSRMLS_CC) != SUCCESS)
    {
        RETURN_NULL();
    }

    Ice::ObjectPrx proxy;
    Slice::ClassDefPtr def;
    if(!fetchProxy(zprx, proxy, def TSRMLS_CC))
    {
        RETURN_NULL();
    }

    Ice::RouterPrx router;
    if(proxy)
    {
        if(!def || !def->isA("Ice::Router"))
        {
            php_error_docref(0 TSRMLS_CC, E_ERROR, "ice_router requires a proxy narrowed to Ice::Router");
            RETURN_NULL();
        }
        router = Ice::RouterPrx::uncheckedCast(proxy);
    }

    try
    {
        Ice::ObjectPrx prx = _this->getProxy()->ice_router(router);
        if(!createProxy(return_value, prx, _this->getClass() TSRMLS_CC))
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

ZEND_FUNCTION(Ice_ObjectPrx_ice_getLocator)
{
    if(ZEND_NUM_ARGS() != 0)
    {
        WRONG_PARAM_COUNT;
    }

    ice_object* obj = static_cast<ice_object*>(zend_object_store_get_object(getThis() TSRMLS_CC));
    assert(obj->ptr);
    Proxy* _this = static_cast<Proxy*>(obj->ptr);

    try
    {
        Ice::LocatorPrx locator = _this->getProxy()->ice_getLocator();
        if(locator)
        {
            Slice::ClassDefPtr def;
            if(!lookupClass("Ice::Locator", def TSRMLS_CC))
            {
                RETURN_NULL();
            }

            assert(def);

            if(!createProxy(return_value, locator, def TSRMLS_CC))
            {
                RETURN_NULL();
            }
        }
        else
        {
            RETURN_NULL();
        }
    }
    catch(const IceUtil::Exception& ex)
    {
        throwException(ex TSRMLS_CC);
        RETURN_FALSE;
    }
}

ZEND_FUNCTION(Ice_ObjectPrx_ice_locator)
{
    if(ZEND_NUM_ARGS() != 1)
    {
        WRONG_PARAM_COUNT;
    }

    ice_object* obj = static_cast<ice_object*>(zend_object_store_get_object(getThis() TSRMLS_CC));
    assert(obj->ptr);
    Proxy* _this = static_cast<Proxy*>(obj->ptr);

    zval* zprx;
    if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "O!", &zprx, proxyClassEntry TSRMLS_CC) != SUCCESS)
    {
        RETURN_NULL();
    }

    Ice::ObjectPrx proxy;
    Slice::ClassDefPtr def;
    if(!fetchProxy(zprx, proxy, def TSRMLS_CC))
    {
        RETURN_NULL();
    }

    Ice::LocatorPrx locator;
    if(proxy)
    {
        if(!def || !def->isA("Ice::Locator"))
        {
            php_error_docref(0 TSRMLS_CC, E_ERROR, "ice_locator requires a proxy narrowed to Ice::Locator");
            RETURN_NULL();
        }
        locator = Ice::LocatorPrx::uncheckedCast(proxy);
    }

    try
    {
        Ice::ObjectPrx prx = _this->getProxy()->ice_locator(locator);
        if(!createProxy(return_value, prx, _this->getClass() TSRMLS_CC))
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

ZEND_FUNCTION(Ice_ObjectPrx_ice_twoway)
{
    if(ZEND_NUM_ARGS() != 0)
    {
        WRONG_PARAM_COUNT;
    }

    ice_object* obj = static_cast<ice_object*>(zend_object_store_get_object(getThis() TSRMLS_CC));
    assert(obj->ptr);
    Proxy* _this = static_cast<Proxy*>(obj->ptr);

    try
    {
        Ice::ObjectPrx prx = _this->getProxy()->ice_twoway();
        if(!createProxy(return_value, prx, _this->getClass() TSRMLS_CC))
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

ZEND_FUNCTION(Ice_ObjectPrx_ice_isTwoway)
{
    if(ZEND_NUM_ARGS() != 0)
    {
        WRONG_PARAM_COUNT;
    }

    ice_object* obj = static_cast<ice_object*>(zend_object_store_get_object(getThis() TSRMLS_CC));
    assert(obj->ptr);
    Proxy* _this = static_cast<Proxy*>(obj->ptr);

    try
    {
        bool b = _this->getProxy()->ice_isTwoway();
        RETURN_BOOL(b ? 1 : 0);
    }
    catch(const IceUtil::Exception& ex)
    {
        throwException(ex TSRMLS_CC);
        RETURN_FALSE;
    }
}

ZEND_FUNCTION(Ice_ObjectPrx_ice_oneway)
{
    if(ZEND_NUM_ARGS() != 0)
    {
        WRONG_PARAM_COUNT;
    }

    ice_object* obj = static_cast<ice_object*>(zend_object_store_get_object(getThis() TSRMLS_CC));
    assert(obj->ptr);
    Proxy* _this = static_cast<Proxy*>(obj->ptr);

    try
    {
        Ice::ObjectPrx prx = _this->getProxy()->ice_oneway();
        if(!createProxy(return_value, prx, _this->getClass() TSRMLS_CC))
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

ZEND_FUNCTION(Ice_ObjectPrx_ice_isOneway)
{
    if(ZEND_NUM_ARGS() != 0)
    {
        WRONG_PARAM_COUNT;
    }

    ice_object* obj = static_cast<ice_object*>(zend_object_store_get_object(getThis() TSRMLS_CC));
    assert(obj->ptr);
    Proxy* _this = static_cast<Proxy*>(obj->ptr);

    try
    {
        bool b = _this->getProxy()->ice_isOneway();
        RETURN_BOOL(b ? 1 : 0);
    }
    catch(const IceUtil::Exception& ex)
    {
        throwException(ex TSRMLS_CC);
        RETURN_FALSE;
    }
}

ZEND_FUNCTION(Ice_ObjectPrx_ice_batchOneway)
{
    if(ZEND_NUM_ARGS() != 0)
    {
        WRONG_PARAM_COUNT;
    }

    ice_object* obj = static_cast<ice_object*>(zend_object_store_get_object(getThis() TSRMLS_CC));
    assert(obj->ptr);
    Proxy* _this = static_cast<Proxy*>(obj->ptr);

    try
    {
        Ice::ObjectPrx prx = _this->getProxy()->ice_batchOneway();
        if(!createProxy(return_value, prx, _this->getClass() TSRMLS_CC))
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

ZEND_FUNCTION(Ice_ObjectPrx_ice_isBatchOneway)
{
    if(ZEND_NUM_ARGS() != 0)
    {
        WRONG_PARAM_COUNT;
    }

    ice_object* obj = static_cast<ice_object*>(zend_object_store_get_object(getThis() TSRMLS_CC));
    assert(obj->ptr);
    Proxy* _this = static_cast<Proxy*>(obj->ptr);

    try
    {
        bool b = _this->getProxy()->ice_isBatchOneway();
        RETURN_BOOL(b ? 1 : 0);
    }
    catch(const IceUtil::Exception& ex)
    {
        throwException(ex TSRMLS_CC);
        RETURN_FALSE;
    }
}

ZEND_FUNCTION(Ice_ObjectPrx_ice_datagram)
{
    if(ZEND_NUM_ARGS() != 0)
    {
        WRONG_PARAM_COUNT;
    }

    ice_object* obj = static_cast<ice_object*>(zend_object_store_get_object(getThis() TSRMLS_CC));
    assert(obj->ptr);
    Proxy* _this = static_cast<Proxy*>(obj->ptr);

    try
    {
        Ice::ObjectPrx prx = _this->getProxy()->ice_datagram();
        if(!createProxy(return_value, prx, _this->getClass() TSRMLS_CC))
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

ZEND_FUNCTION(Ice_ObjectPrx_ice_isDatagram)
{
    if(ZEND_NUM_ARGS() != 0)
    {
        WRONG_PARAM_COUNT;
    }

    ice_object* obj = static_cast<ice_object*>(zend_object_store_get_object(getThis() TSRMLS_CC));
    assert(obj->ptr);
    Proxy* _this = static_cast<Proxy*>(obj->ptr);

    try
    {
        bool b = _this->getProxy()->ice_isDatagram();
        RETURN_BOOL(b ? 1 : 0);
    }
    catch(const IceUtil::Exception& ex)
    {
        throwException(ex TSRMLS_CC);
        RETURN_FALSE;
    }
}

ZEND_FUNCTION(Ice_ObjectPrx_ice_batchDatagram)
{
    if(ZEND_NUM_ARGS() != 0)
    {
        WRONG_PARAM_COUNT;
    }

    ice_object* obj = static_cast<ice_object*>(zend_object_store_get_object(getThis() TSRMLS_CC));
    assert(obj->ptr);
    Proxy* _this = static_cast<Proxy*>(obj->ptr);

    try
    {
        Ice::ObjectPrx prx = _this->getProxy()->ice_batchDatagram();
        if(!createProxy(return_value, prx, _this->getClass() TSRMLS_CC))
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

ZEND_FUNCTION(Ice_ObjectPrx_ice_isBatchDatagram)
{
    if(ZEND_NUM_ARGS() != 0)
    {
        WRONG_PARAM_COUNT;
    }

    ice_object* obj = static_cast<ice_object*>(zend_object_store_get_object(getThis() TSRMLS_CC));
    assert(obj->ptr);
    Proxy* _this = static_cast<Proxy*>(obj->ptr);

    try
    {
        bool b = _this->getProxy()->ice_isBatchDatagram();
        RETURN_BOOL(b ? 1 : 0);
    }
    catch(const IceUtil::Exception& ex)
    {
        throwException(ex TSRMLS_CC);
        RETURN_FALSE;
    }
}

ZEND_FUNCTION(Ice_ObjectPrx_ice_compress)
{
    if(ZEND_NUM_ARGS() != 1)
    {
        WRONG_PARAM_COUNT;
    }

    ice_object* obj = static_cast<ice_object*>(zend_object_store_get_object(getThis() TSRMLS_CC));
    assert(obj->ptr);
    Proxy* _this = static_cast<Proxy*>(obj->ptr);

    zend_bool b;
    if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "b", &b) != SUCCESS)
    {
        RETURN_NULL();
    }

    try
    {
        Ice::ObjectPrx prx = _this->getProxy()->ice_compress(b ? true : false);
        if(!createProxy(return_value, prx, _this->getClass() TSRMLS_CC))
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

ZEND_FUNCTION(Ice_ObjectPrx_ice_timeout)
{
    if(ZEND_NUM_ARGS() != 1)
    {
        WRONG_PARAM_COUNT;
    }

    ice_object* obj = static_cast<ice_object*>(zend_object_store_get_object(getThis() TSRMLS_CC));
    assert(obj->ptr);
    Proxy* _this = static_cast<Proxy*>(obj->ptr);

    try
    {
        long l;
        if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &l) != SUCCESS)
        {
            RETURN_NULL();
        }
        // TODO: range check?
        Ice::ObjectPrx prx = _this->getProxy()->ice_timeout(l);
        if(!createProxy(return_value, prx, _this->getClass() TSRMLS_CC))
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

ZEND_FUNCTION(Ice_ObjectPrx_ice_connectionId)
{
    if(ZEND_NUM_ARGS() != 1)
    {
        WRONG_PARAM_COUNT;
    }

    ice_object* obj = static_cast<ice_object*>(zend_object_store_get_object(getThis() TSRMLS_CC));
    assert(obj->ptr);
    Proxy* _this = static_cast<Proxy*>(obj->ptr);

    try
    {
        char* id;
        int idLen;
        if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &id, &idLen) != SUCCESS)
        {
            RETURN_NULL();
        }
        Ice::ObjectPrx prx = _this->getProxy()->ice_connectionId(id);
        if(!createProxy(return_value, prx, _this->getClass() TSRMLS_CC))
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

ZEND_FUNCTION(Ice_ObjectPrx_ice_isThreadPerConnection)
{
    if(ZEND_NUM_ARGS() != 0)
    {
        WRONG_PARAM_COUNT;
    }

    ice_object* obj = static_cast<ice_object*>(zend_object_store_get_object(getThis() TSRMLS_CC));
    assert(obj->ptr);
    Proxy* _this = static_cast<Proxy*>(obj->ptr);

    try
    {
        bool b = _this->getProxy()->ice_isThreadPerConnection();
        RETURN_BOOL(b ? 1 : 0);
    }
    catch(const IceUtil::Exception& ex)
    {
        throwException(ex TSRMLS_CC);
        RETURN_FALSE;
    }
}

ZEND_FUNCTION(Ice_ObjectPrx_ice_threadPerConnection)
{
    if(ZEND_NUM_ARGS() != 1)
    {
        WRONG_PARAM_COUNT;
    }

    ice_object* obj = static_cast<ice_object*>(zend_object_store_get_object(getThis() TSRMLS_CC));
    assert(obj->ptr);
    Proxy* _this = static_cast<Proxy*>(obj->ptr);

    zend_bool b;
    if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "b", &b) != SUCCESS)
    {
        RETURN_NULL();
    }

    try
    {
        Ice::ObjectPrx prx = _this->getProxy()->ice_threadPerConnection(b ? true : false);
        if(!createProxy(return_value, prx, _this->getClass() TSRMLS_CC))
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

ZEND_FUNCTION(Ice_ObjectPrx_ice_getConnection)
{
    if(ZEND_NUM_ARGS() != 0)
    {
        WRONG_PARAM_COUNT;
    }

    ice_object* obj = static_cast<ice_object*>(zend_object_store_get_object(getThis() TSRMLS_CC));
    assert(obj->ptr);
    Proxy* _this = static_cast<Proxy*>(obj->ptr);

    try
    {
        Ice::ConnectionPtr con = _this->getProxy()->ice_getConnection();
        if(!createConnection(return_value, con TSRMLS_CC))
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

ZEND_FUNCTION(Ice_ObjectPrx_ice_getCachedConnection)
{
    if(ZEND_NUM_ARGS() != 0)
    {
        WRONG_PARAM_COUNT;
    }

    ice_object* obj = static_cast<ice_object*>(zend_object_store_get_object(getThis() TSRMLS_CC));
    assert(obj->ptr);
    Proxy* _this = static_cast<Proxy*>(obj->ptr);

    try
    {
        Ice::ConnectionPtr con = _this->getProxy()->ice_getCachedConnection();
        if(!con || !createConnection(return_value, con TSRMLS_CC))
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

static bool
lookupClass(const string& id, Slice::ClassDefPtr& def TSRMLS_DC)
{
    def = 0;

    try
    {
        Slice::TypeList l;
        Profile* profile = static_cast<Profile*>(ICE_G(profile));
        if(profile)
        {
            l = profile->unit->lookupType(id, false);
        }

        if(l.empty())
        {
            php_error_docref(0 TSRMLS_CC, E_ERROR, "no Slice definition found for type %s", id.c_str());
            return false;
        }

        Slice::BuiltinPtr b = Slice::BuiltinPtr::dynamicCast(l.front());
        if(b && b->kind() != Slice::Builtin::KindObject && b->kind() != Slice::Builtin::KindObjectProxy)
        {
            php_error_docref(0 TSRMLS_CC, E_ERROR, "type %s is not a class or interface", id.c_str());
            return false;
        }

        if(!b)
        {
            //
            // Allow the use of "::Type" (ClassDecl) or "::Type*" (Proxy).
            //
            Slice::ClassDeclPtr decl;
            Slice::TypePtr type = l.front();
            Slice::ProxyPtr proxy = Slice::ProxyPtr::dynamicCast(type);
            if(proxy)
            {
                decl = proxy->_class();
            }
            else
            {
                decl = Slice::ClassDeclPtr::dynamicCast(type);
            }

            if(!decl)
            {
                php_error_docref(0 TSRMLS_CC, E_ERROR, "type %s is not a class or interface", id.c_str());
                return false;
            }

            if(decl->isLocal())
            {
                php_error_docref(0 TSRMLS_CC, E_ERROR, "%s is a local type", id.c_str());
                return false;
            }

            def = decl->definition();
            if(!def)
            {
                php_error_docref(0 TSRMLS_CC, E_ERROR, "%s is declared but not defined", id.c_str());
                return false;
            }

            string scoped = decl->scoped();

            //
            // Verify that the script has compiled the Slice definition for this type.
            //
            if(findClassScoped(scoped TSRMLS_CC) == 0)
            {
                php_error_docref(0 TSRMLS_CC, E_ERROR, "the Slice definition for type %s has not been compiled",
                                 scoped.c_str());
                return false;
            }
        }
    }
    catch(const IceUtil::Exception& ex)
    {
        throwException(ex TSRMLS_CC);
        return false;
    }

    return true;
}

static void
do_cast(INTERNAL_FUNCTION_PARAMETERS, bool check)
{
    //
    // First argument is required and should be a scoped name. The second and third arguments
    // are optional and represent a facet name, a context, or a facet name followed by a context.
    //
    if(ZEND_NUM_ARGS() < 1 || ZEND_NUM_ARGS() > 3)
    {
        WRONG_PARAM_COUNT;
    }

    char* id;
    int idLen;
    char* facet = 0;
    int facetLen;
    zval* arr = 0;

    if(zend_parse_parameters_ex(ZEND_PARSE_PARAMS_QUIET, ZEND_NUM_ARGS() TSRMLS_CC, "s|sa", &id, &idLen, &facet,
                                &facetLen, &arr) == FAILURE)
    {
        facet = 0;
        if(zend_parse_parameters_ex(ZEND_PARSE_PARAMS_QUIET, ZEND_NUM_ARGS() TSRMLS_CC, "s|a", &id, &idLen, &arr) ==
                                    FAILURE)
        {
            php_error(E_ERROR, "%s() requires a type id followed by an optional facet and/or context",
                      get_active_function_name(TSRMLS_C));
            return;
        }
    }

    ice_object* obj = static_cast<ice_object*>(zend_object_store_get_object(getThis() TSRMLS_CC));
    assert(obj->ptr);
    Proxy* _this = static_cast<Proxy*>(obj->ptr);

    //
    // Populate the context.
    //
    Ice::Context ctx;
    if(arr && !extractContext(arr, ctx TSRMLS_CC))
    {
        RETURN_NULL();
    }

    try
    {
        Slice::ClassDefPtr def;
        if(!lookupClass(id, def TSRMLS_CC))
        {
            RETURN_NULL();
        }

        Ice::ObjectPrx prx = _this->getProxy();
        if(facet)
        {
            prx = prx->ice_facet(facet);
        }

        if(arr)
        {
            prx = prx->ice_context(ctx);
        }

        if(check)
        {
            string scoped = def ? def->declaration()->scoped() : "::Ice::Object";

            //
            // Verify that the object supports the requested type. We don't use id here,
            // because it might contain a proxy type (e.g., "::MyClass*").
            //
            if(!prx->ice_isA(scoped))
            {
                RETURN_NULL();
            }
        }

        if(!createProxy(return_value, prx, def TSRMLS_CC))
        {
            RETURN_NULL();
        }
    }
    catch(const IceUtil::Exception& ex)
    {
        throwException(ex TSRMLS_CC);
        RETVAL_FALSE;
    }
}

ZEND_FUNCTION(Ice_ObjectPrx_ice_uncheckedCast)
{
    do_cast(INTERNAL_FUNCTION_PARAM_PASSTHRU, false);
}

ZEND_FUNCTION(Ice_ObjectPrx_ice_checkedCast)
{
    do_cast(INTERNAL_FUNCTION_PARAM_PASSTHRU, true);
}

ZEND_FUNCTION(Ice_Endpoint___construct)
{
    php_error_docref(0 TSRMLS_CC, E_ERROR, "Ice_Endpoint cannot be instantiated");
}

ZEND_FUNCTION(Ice_Endpoint___tostring)
{
    if(ZEND_NUM_ARGS() > 0)
    {
        WRONG_PARAM_COUNT;
    }

    ice_object* obj = static_cast<ice_object*>(zend_object_store_get_object(getThis() TSRMLS_CC));
    assert(obj->ptr);
    Ice::EndpointPtr* _this = static_cast<Ice::EndpointPtr*>(obj->ptr);

    try
    {
        string str = (*_this)->toString();
        RETURN_STRINGL(const_cast<char*>(str.c_str()), str.length(), 1);
    }
    catch(const IceUtil::Exception& ex)
    {
        throwException(ex TSRMLS_CC);
        RETURN_NULL();
    }
}

ZEND_FUNCTION(Ice_Endpoint_toString)
{
    ZEND_FN(Ice_Endpoint___tostring)(INTERNAL_FUNCTION_PARAM_PASSTHRU);
}

ZEND_FUNCTION(Ice_Connection___construct)
{
    php_error_docref(0 TSRMLS_CC, E_ERROR, "Ice_Connection cannot be instantiated");
}

ZEND_FUNCTION(Ice_Connection___tostring)
{
    if(ZEND_NUM_ARGS() > 0)
    {
        WRONG_PARAM_COUNT;
    }

    ice_object* obj = static_cast<ice_object*>(zend_object_store_get_object(getThis() TSRMLS_CC));
    assert(obj->ptr);
    Ice::ConnectionPtr* _this = static_cast<Ice::ConnectionPtr*>(obj->ptr);

    try
    {
        string str = (*_this)->toString();
        RETURN_STRINGL(const_cast<char*>(str.c_str()), str.length(), 1);
    }
    catch(const IceUtil::Exception& ex)
    {
        throwException(ex TSRMLS_CC);
        RETURN_NULL();
    }
}

ZEND_FUNCTION(Ice_Connection_close)
{
    if(ZEND_NUM_ARGS() != 1)
    {
        WRONG_PARAM_COUNT;
    }

    ice_object* obj = static_cast<ice_object*>(zend_object_store_get_object(getThis() TSRMLS_CC));
    assert(obj->ptr);
    Ice::ConnectionPtr* _this = static_cast<Ice::ConnectionPtr*>(obj->ptr);

    zend_bool b;
    if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "b", &b TSRMLS_CC) != SUCCESS)
    {
        RETURN_NULL();
    }

    try
    {
        (*_this)->close(b ? true : false);
    }
    catch(const IceUtil::Exception& ex)
    {
        throwException(ex TSRMLS_CC);
        RETURN_NULL();
    }
}

ZEND_FUNCTION(Ice_Connection_flushBatchRequests)
{
    if(ZEND_NUM_ARGS() > 0)
    {
        WRONG_PARAM_COUNT;
    }

    ice_object* obj = static_cast<ice_object*>(zend_object_store_get_object(getThis() TSRMLS_CC));
    assert(obj->ptr);
    Ice::ConnectionPtr* _this = static_cast<Ice::ConnectionPtr*>(obj->ptr);

    try
    {
        (*_this)->flushBatchRequests();
    }
    catch(const IceUtil::Exception& ex)
    {
        throwException(ex TSRMLS_CC);
        RETURN_NULL();
    }
}

ZEND_FUNCTION(Ice_Connection_type)
{
    if(ZEND_NUM_ARGS() > 0)
    {
        WRONG_PARAM_COUNT;
    }

    ice_object* obj = static_cast<ice_object*>(zend_object_store_get_object(getThis() TSRMLS_CC));
    assert(obj->ptr);
    Ice::ConnectionPtr* _this = static_cast<Ice::ConnectionPtr*>(obj->ptr);

    try
    {
        string str = (*_this)->type();
        RETURN_STRINGL(const_cast<char*>(str.c_str()), str.length(), 1);
    }
    catch(const IceUtil::Exception& ex)
    {
        throwException(ex TSRMLS_CC);
        RETURN_NULL();
    }
}

ZEND_FUNCTION(Ice_Connection_timeout)
{
    if(ZEND_NUM_ARGS() != 0)
    {
        WRONG_PARAM_COUNT;
    }

    ice_object* obj = static_cast<ice_object*>(zend_object_store_get_object(getThis() TSRMLS_CC));
    assert(obj->ptr);
    Ice::ConnectionPtr* _this = static_cast<Ice::ConnectionPtr*>(obj->ptr);

    try
    {
        Ice::Int timeout = (*_this)->timeout();
        ZVAL_LONG(return_value, static_cast<long>(timeout));
    }
    catch(const IceUtil::Exception& ex)
    {
        throwException(ex TSRMLS_CC);
        RETURN_NULL();
    }
}

ZEND_FUNCTION(Ice_Connection_toString)
{
    ZEND_FN(Ice_Connection___tostring)(INTERNAL_FUNCTION_PARAM_PASSTHRU);
}

IcePHP::Operation::Operation(const Ice::ObjectPrx& proxy, const string& name, const Slice::OperationPtr& op,
                             const Ice::CommunicatorPtr& communicator TSRMLS_DC) :
    _proxy(proxy), _name(name), _op(op), _communicator(communicator), _zendFunction(0)
{
#ifdef ZTS
    this->TSRMLS_C = TSRMLS_C;
#endif
    //
    // Create Marshaler objects for return type and parameters.
    //
    Slice::TypePtr ret = op->returnType();
    if(ret)
    {
        _result = Marshaler::createMarshaler(ret TSRMLS_CC);
        if(!_result)
        {
            return;
        }
    }

    Slice::ParamDeclList params = op->parameters();

    //
    // Create an array that indicates how arguments are passed to the operation.
    //
    zend_arg_info* argInfo = new zend_arg_info[params.size()];

    int i;
    Slice::ParamDeclList::const_iterator p;
    for(p = params.begin(), i = 0; p != params.end(); ++p, ++i)
    {
        Slice::TypePtr paramType = (*p)->type();
        MarshalerPtr m = Marshaler::createMarshaler(paramType TSRMLS_CC);
        if(!m)
        {
            break;
        }
        _paramNames.push_back((*p)->name());
        argInfo[i].name = 0;
        argInfo[i].class_name = 0;
        argInfo[i].allow_null = 1;
        Slice::ContainedPtr cont = Slice::ContainedPtr::dynamicCast(paramType);
        if(cont)
        {
            argInfo[i].array_type_hint = ((cont->containedType() == Slice::Contained::ContainedTypeSequence ||
                                           cont->containedType() == Slice::Contained::ContainedTypeDictionary) ? 1 : 0);
        }
        else
        {
            argInfo[i].array_type_hint = 0;
        }
        argInfo[i].return_reference = 0;
        argInfo[i].required_num_args = static_cast<zend_uint>(params.size());
        if((*p)->isOutParam())
        {
            argInfo[i].pass_by_reference = 1;
            _outParams.push_back(m);
        }
        else
        {
            argInfo[i].pass_by_reference = 0;
            _inParams.push_back(m);
        }
    }

    _zendFunction = static_cast<zend_internal_function*>(emalloc(sizeof(zend_internal_function)));
    _zendFunction->type = ZEND_INTERNAL_FUNCTION;
    _zendFunction->function_name = estrndup(const_cast<char*>(name.c_str()), name.length());
    _zendFunction->scope = proxyClassEntry;
    _zendFunction->fn_flags = ZEND_ACC_PUBLIC;
    _zendFunction->prototype = 0;
    _zendFunction->num_args = static_cast<zend_uint>(params.size());
    _zendFunction->arg_info = argInfo;
    _zendFunction->pass_rest_by_reference = 0;
    _zendFunction->required_num_args = _zendFunction->num_args;
    _zendFunction->return_reference = 0;
    _zendFunction->handler = ZEND_FN(Ice_ObjectPrx_call);
}

IcePHP::Operation::~Operation()
{
    if(_zendFunction)
    {
        delete []_zendFunction->arg_info;
        efree(_zendFunction->function_name);
        efree(_zendFunction);
    }
}

zend_function*
IcePHP::Operation::getZendFunction() const
{
    return reinterpret_cast<zend_function*>(_zendFunction);
}

void
IcePHP::Operation::invoke(INTERNAL_FUNCTION_PARAMETERS)
{
    Ice::OperationMode mode = (Ice::OperationMode)_op->sendMode();
    int i;

    //
    // Verify that the expected number of arguments are supplied. The context argument is optional.
    //
    int numParams = static_cast<int>(_inParams.size() + _outParams.size());
    if(ZEND_NUM_ARGS() != numParams && ZEND_NUM_ARGS() != numParams + 1)
    {
        php_error_docref(0 TSRMLS_CC, E_ERROR, "incorrect number of parameters (%d)", numParams);
        return;
    }

    //
    // Retrieve the arguments.
    //
    zval*** args = static_cast<zval***>(emalloc(ZEND_NUM_ARGS() * sizeof(zval**)));
    AutoEfree autoArgs(args); // Call efree on return
    if(zend_get_parameters_array_ex(ZEND_NUM_ARGS(), args) == FAILURE)
    {
        php_error_docref(0 TSRMLS_CC, E_ERROR, "unable to get arguments");
        return;
    }

    //
    // Verify that the zvals for out parameters are passed by reference.
    //
    for(i = static_cast<int>(_inParams.size()); i < numParams; ++i)
    {
        if(!PZVAL_IS_REF(*args[i]))
        {
            php_error_docref(0 TSRMLS_CC, E_ERROR, "argument for out parameter %s must be passed by reference",
                             _paramNames[i].c_str());
            return;
        }
    }

    try
    {
        //
        // Marshal the arguments.
        //
        Ice::OutputStreamPtr os = Ice::createOutputStream(_communicator);
        ObjectMap objectMap;
        vector<MarshalerPtr>::iterator p;
        for(i = 0, p = _inParams.begin(); p != _inParams.end(); ++i, ++p)
        {
            if(!(*p)->marshal(*args[i], os, objectMap TSRMLS_CC))
            {
                return;
            }
        }

        if(_op->sendsClasses())
        {
            os->writePendingObjects();
        }

        Ice::ByteSeq params;
        os->finished(params);

        //
        // Populate the context (if necessary).
        //
        Ice::Context ctx;
        bool haveContext = false;
        if(ZEND_NUM_ARGS() == numParams + 1)
        {
            if(extractContext(*args[numParams], ctx TSRMLS_CC))
            {
                haveContext = true;
            }
            else
            {
                return;
            }
        }

        //
        // Invoke the operation. Don't use _name here.
        //
        Ice::ByteSeq result;
        bool status;
        if(haveContext)
        {
            status = _proxy->ice_invoke(_op->name(), mode, params, result, ctx);
        }
        else
        {
            status = _proxy->ice_invoke(_op->name(), mode, params, result);
        }

        //
        // Process the reply.
        //
        if(_proxy->ice_isTwoway())
        {
            Ice::InputStreamPtr is = Ice::createInputStream(_communicator, result);

            if(status)
            {
                //
                // Unmarshal the results.
                //
                // TODO: Check for oneway/datagram errors
                //
                for(i = _inParams.size(), p = _outParams.begin(); p != _outParams.end(); ++i, ++p)
                {
                    //
                    // We must explicitly destroy the existing contents of all zvals passed
                    // as out parameters, otherwise leaks occur.
                    //
                    zval_dtor(*args[i]);
                    if(!(*p)->unmarshal(*args[i], is TSRMLS_CC))
                    {
                        return;
                    }
                }
                if(_result)
                {
                    if(!_result->unmarshal(return_value, is TSRMLS_CC))
                    {
                        return;
                    }
                }
                if(_op->returnsClasses())
                {
                    is->readPendingObjects();
                }
            }
            else
            {
                //
                // Unmarshal and "throw" a user exception.
                //
                throwUserException(is TSRMLS_CC);
            }
        }
    }
    catch(const AbortMarshaling&)
    {
        //
        // We use AbortMarshaling to escape from any nesting depth if
        // a PHP exception has already been set.
        //
        assert(EG(exception));
    }
    catch(const IceUtil::Exception& ex)
    {
        throwException(ex TSRMLS_CC);
    }
}

void
IcePHP::Operation::throwUserException(Ice::InputStreamPtr& is TSRMLS_DC)
{
    Slice::UnitPtr unit = _op->unit();

    is->readBool(); // usesClasses

    string id = is->readString();
    while(!id.empty())
    {
        //
        // Look for a definition of this type.
        //
        Slice::ExceptionPtr ex = unit->lookupException(id, false);
        if(ex)
        {
            if(ex->isLocal())
            {
                php_error_docref(0 TSRMLS_CC, E_ERROR, "cannot unmarshal local exception %s", id.c_str());
                return;
            }

            MarshalerPtr m = Marshaler::createExceptionMarshaler(ex TSRMLS_CC);
            assert(m);

            zval* zex;
            MAKE_STD_ZVAL(zex);
            if(m->unmarshal(zex, is TSRMLS_CC))
            {
                if(ex->usesClasses())
                {
                    is->readPendingObjects();
                }
                zend_throw_exception_object(zex TSRMLS_CC);
            }
            else
            {
                zval_dtor(zex);
            }

            return;
        }
        else
        {
            is->skipSlice();
            id = is->readString();
        }
    }
    //
    // Getting here should be impossible: we can get here only if the
    // sender has marshaled a sequence of type IDs, none of which we
    // have factory for. This means that sender and receiver disagree
    // about the Slice definitions they use.
    //
    throw Ice::UnknownUserException(__FILE__, __LINE__);
}

IcePHP::Proxy::Proxy(const Ice::ObjectPrx& proxy, const Slice::ClassDefPtr& cls TSRMLS_DC) :
    _proxy(proxy), _class(cls)
{
#ifdef ZTS
    this->TSRMLS_C = TSRMLS_C;
#endif

    //
    // We want to ensure that the PHP object corresponding to the communicator is
    // not destroyed until after this proxy is destroyed. We keep a copy of the
    // communicator's zval because the symbol table holding the communicator's zval
    // may be destroyed before this proxy, therefore our destructor cannot rely on
    // symbol table lookup when it needs to decrement the reference count.
    //
    zval* zc = getCommunicatorZval(TSRMLS_C);
    _communicatorZval = *zc; // This is legal - it simply copies the object's handle
    Z_OBJ_HT(_communicatorZval)->add_ref(&_communicatorZval TSRMLS_CC);

    _communicator = getCommunicator(TSRMLS_C);

    if(cls)
    {
        _classOps = _class->allOperations();
    }
}

IcePHP::Proxy::~Proxy()
{
    //
    // In order to avoid the communicator's "leak warning", we have to ensure that we
    // remove any references to the communicator or its supporting objects. This must
    // be done prior to invoking del_ref(), because the C++ communicator object may
    // be destroyed during this call.
    // 
    _communicator = 0;
    _ops.clear();
    _proxy = 0;
    Z_OBJ_HT(_communicatorZval)->del_ref(&_communicatorZval TSRMLS_CC);
}

const Ice::ObjectPrx&
IcePHP::Proxy::getProxy() const
{
    return _proxy;
}

const Slice::ClassDefPtr&
IcePHP::Proxy::getClass() const
{
    return _class;
}

OperationPtr
IcePHP::Proxy::getOperation(const string& name)
{
    OperationPtr result;

    string n = lowerCase(name);
    map<string, OperationPtr>::const_iterator p = _ops.find(n);
    if(p == _ops.end())
    {
        for(Slice::OperationList::const_iterator q = _classOps.begin(); q != _classOps.end(); ++q)
        {
            string opName = lowerCase(fixIdent((*q)->name()));
            if(n == opName)
            {
                result = new Operation(_proxy, opName, *q, _communicator TSRMLS_CC);
                _ops[opName] = result;
                break;
            }
        }
    }
    else
    {
        result = p->second;
    }

    return result;
}

string
IcePHP::Proxy::toString() const
{
    return _communicator->proxyToString(_proxy);
}

#ifdef WIN32
extern "C"
#endif
static zend_object_value
handleProxyAlloc(zend_class_entry* ce TSRMLS_DC)
{
    zend_object_value result;

    ice_object* obj = newObject(ce TSRMLS_CC);
    assert(obj);

    result.handle = zend_objects_store_put(obj, 0, (zend_objects_free_object_storage_t)handleProxyFreeStorage,
                                           0 TSRMLS_CC);
    result.handlers = &_proxyHandlers;

    return result;
}

#ifdef WIN32
extern "C"
#endif
static void
handleProxyFreeStorage(void* p TSRMLS_DC)
{
    ice_object* obj = static_cast<ice_object*>(p);
    Proxy* _this = static_cast<Proxy*>(obj->ptr);

    delete _this;

    zend_objects_free_object_storage(static_cast<zend_object*>(p) TSRMLS_CC);
}

#ifdef WIN32
extern "C"
#endif
static zend_object_value
handleProxyClone(zval* zv TSRMLS_DC)
{
    //
    // Create a new object that shares a C++ proxy instance with this object.
    //

    zend_object_value result;
    memset(&result, 0, sizeof(zend_object_value));

    ice_object* obj = static_cast<ice_object*>(zend_object_store_get_object(zv TSRMLS_CC));
    assert(obj->ptr);
    Proxy* _this = static_cast<Proxy*>(obj->ptr);

    zval* clone;
    MAKE_STD_ZVAL(clone);
    if(object_init_ex(clone, IcePHP::proxyClassEntry) != SUCCESS)
    {
        php_error_docref(0 TSRMLS_CC, E_ERROR, "unable to initialize proxy");
        return result;
    }

    ice_object* cobj = static_cast<ice_object*>(zend_object_store_get_object(clone TSRMLS_CC));
    assert(!cobj->ptr);
    cobj->ptr = new Proxy(_this->getProxy(), _this->getClass() TSRMLS_CC);

    //
    // We only need to return the new object's handle, so we must destroy the zval containing
    // a reference to the new object. We increment the object's reference count to ensure it
    // does not get destroyed.
    //
    result = clone->value.obj;
    Z_OBJ_HT_P(clone)->add_ref(clone TSRMLS_CC);
    zval_dtor(clone);
    efree(clone);

    return result;
}

#ifdef WIN32
extern "C"
#endif
static union _zend_function*
handleProxyGetMethod(zval** zv, char* method, int len TSRMLS_DC)
{
    zend_function* result;

    //
    // First delegate to the standard implementation of get_method. This will find
    // any of our predefined proxy methods. If it returns 0, then we return a
    // function that will check the class definition.
    //
    result = zend_get_std_object_handlers()->get_method(zv, method, len TSRMLS_CC);
    if(!result)
    {
        ice_object* obj = static_cast<ice_object*>(zend_object_store_get_object(*zv TSRMLS_CC));
        assert(obj->ptr);
        Proxy* _this = static_cast<Proxy*>(obj->ptr);

        Slice::ClassDefPtr def = _this->getClass();
        if(!def)
        {
            php_error_docref(0 TSRMLS_CC, E_ERROR, "unknown method %s invoked on untyped proxy", method);
            return 0;
        }

        OperationPtr op = _this->getOperation(method);
        if(!op)
        {
            string scoped = def->scoped();
            php_error_docref(0 TSRMLS_CC, E_ERROR, "unknown operation %s invoked on proxy of type %s", method,
                             scoped.c_str());
            return 0;
        }

        result = op->getZendFunction();
    }

    return result;
}

#ifdef WIN32
extern "C"
#endif
static int
handleProxyCompare(zval* zobj1, zval* zobj2 TSRMLS_DC)
{
    //
    // PHP guarantees that the objects have the same class.
    //

    ice_object* obj1 = static_cast<ice_object*>(zend_object_store_get_object(zobj1 TSRMLS_CC));
    assert(obj1->ptr);
    Proxy* _this1 = static_cast<Proxy*>(obj1->ptr);
    Ice::ObjectPrx prx1 = _this1->getProxy();

    ice_object* obj2 = static_cast<ice_object*>(zend_object_store_get_object(zobj2 TSRMLS_CC));
    assert(obj2->ptr);
    Proxy* _this2 = static_cast<Proxy*>(obj2->ptr);
    Ice::ObjectPrx prx2 = _this2->getProxy();

    if(prx1 == prx2)
    {
        return 0;
    }
    else if(prx1 < prx2)
    {
        return -1;
    }
    else
    {
        return 1;
    }
}

ZEND_FUNCTION(Ice_ObjectPrx_call)
{
    ice_object* obj = static_cast<ice_object*>(zend_object_store_get_object(getThis() TSRMLS_CC));
    assert(obj->ptr);
    Proxy* _this = static_cast<Proxy*>(obj->ptr);

    OperationPtr op = _this->getOperation(get_active_function_name(TSRMLS_C));
    assert(op); // handleGetethod should have already verified the operation's existence.

    op->invoke(INTERNAL_FUNCTION_PARAM_PASSTHRU);
}

#ifdef WIN32
extern "C"
#endif
static zend_object_value
handleEndpointAlloc(zend_class_entry* ce TSRMLS_DC)
{
    zend_object_value result;

    ice_object* obj = newObject(ce TSRMLS_CC);
    assert(obj);

    result.handle = zend_objects_store_put(obj, 0, (zend_objects_free_object_storage_t)handleEndpointFreeStorage,
                                           0 TSRMLS_CC);
    result.handlers = &_endpointHandlers;

    return result;
}

#ifdef WIN32
extern "C"
#endif
static void
handleEndpointFreeStorage(void* p TSRMLS_DC)
{
    ice_object* obj = static_cast<ice_object*>(p);
    Ice::EndpointPtr* _this = static_cast<Ice::EndpointPtr*>(obj->ptr);

    delete _this;

    zend_objects_free_object_storage(static_cast<zend_object*>(p) TSRMLS_CC);
}

#ifdef WIN32
extern "C"
#endif
static zend_object_value
handleConnectionAlloc(zend_class_entry* ce TSRMLS_DC)
{
    zend_object_value result;

    ice_object* obj = newObject(ce TSRMLS_CC);
    assert(obj);

    result.handle = zend_objects_store_put(obj, 0, (zend_objects_free_object_storage_t)handleConnectionFreeStorage,
                                           0 TSRMLS_CC);
    result.handlers = &_connectionHandlers;

    return result;
}

#ifdef WIN32
extern "C"
#endif
static void
handleConnectionFreeStorage(void* p TSRMLS_DC)
{
    ice_object* obj = static_cast<ice_object*>(p);
    Ice::ConnectionPtr* _this = static_cast<Ice::ConnectionPtr*>(obj->ptr);

    delete _this;

    zend_objects_free_object_storage(static_cast<zend_object*>(p) TSRMLS_CC);
}

#ifdef WIN32
extern "C"
#endif
static int
handleConnectionCompare(zval* zobj1, zval* zobj2 TSRMLS_DC)
{
    //
    // PHP guarantees that the objects have the same class.
    //

    ice_object* obj1 = static_cast<ice_object*>(zend_object_store_get_object(zobj1 TSRMLS_CC));
    assert(obj1->ptr);
    Ice::ConnectionPtr* _this1 = static_cast<Ice::ConnectionPtr*>(obj1->ptr);
    Ice::ConnectionPtr con1 = *_this1;

    ice_object* obj2 = static_cast<ice_object*>(zend_object_store_get_object(zobj2 TSRMLS_CC));
    assert(obj2->ptr);
    Ice::ConnectionPtr* _this2 = static_cast<Ice::ConnectionPtr*>(obj2->ptr);
    Ice::ConnectionPtr con2 = *_this2;

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
