// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Proxy.h>
#include <Connection.h>
#include <Endpoint.h>
#include <Util.h>

using namespace std;
using namespace IcePHP;

ZEND_EXTERN_MODULE_GLOBALS(ice)

//
// Here's a brief description of how proxies are handled by this extension.
//
// A single PHP class, ObjectPrx, is registered. This is an "internal" class,
// i.e., implemented by this extension, and it is used to represent all proxies
// regardless of interface type.
//
// Like in C++, a proxy is only capable of invoking the Ice::ObjectPrx operations
// until it is narrowed with a checked or unchecked cast. Unlike C++, no PHP classes
// are created for proxies, because all marshaling activity is driven by the type
// definitions, not by statically-generated code.
//
// In order to perform a checked or unchecked cast, the generated code invokes
// ice_checkedCast or ice_uncheckedCast on the proxy to be narrowed, supplying a scoped
// name for the desired type. Internally, the proxy validates the scoped name and returns
// a new proxy containing the class or interface definition. This proxy is considered
// to be narrowed to that interface and therefore supports user-defined operations.
//
// Naturally, there are many predefined proxy methods (e.g., ice_getIdentity, etc.), but
// the proxy also needs to support user-defined operations (if it has type information).
// We use a Zend API hook that allows us to intercept the invocation of unknown methods
// on the proxy object.
//

//
// Class entries represent the PHP class implementations we have registered.
//
namespace IcePHP
{
zend_class_entry* proxyClassEntry = 0;
}

//
// Ice::ObjectPrx support.
//
static zend_object_handlers _handlers;

extern "C"
{
static zend_object_value handleAlloc(zend_class_entry* TSRMLS_DC);
static void handleFreeStorage(void* TSRMLS_DC);
static zend_object_value handleClone(zval* TSRMLS_DC);
#if PHP_VERSION_ID < 50400
static union _zend_function* handleGetMethod(zval**, char*, int TSRMLS_DC);
#else
static union _zend_function* handleGetMethod(zval**, char*, int, const _zend_literal* TSRMLS_DC);
#endif
static int handleCompare(zval*, zval* TSRMLS_DC);
}

namespace IcePHP
{

//
// Encapsulates proxy and type information.
//
class Proxy : public IceUtil::Shared
{
public:

    Proxy(const Ice::ObjectPrx&, const ProxyInfoPtr&, const CommunicatorInfoPtr& TSRMLS_DC);
    ~Proxy();

    bool clone(zval*, const Ice::ObjectPrx& TSRMLS_DC);
    bool cloneUntyped(zval*, const Ice::ObjectPrx& TSRMLS_DC);
    static bool create(zval*, const Ice::ObjectPrx&, const ProxyInfoPtr&, const CommunicatorInfoPtr& TSRMLS_DC);

    Ice::ObjectPrx proxy;
    ProxyInfoPtr info;
    CommunicatorInfoPtr communicator;
    zval* connection;
    zval* cachedConnection;
#if ZTS
    TSRMLS_D;
#endif
};
typedef IceUtil::Handle<Proxy> ProxyPtr;

} // End of namespace IcePHP

ZEND_METHOD(Ice_ObjectPrx, __construct)
{
    runtimeError("proxies cannot be instantiated, use stringToProxy()" TSRMLS_CC);
}

ZEND_METHOD(Ice_ObjectPrx, __toString)
{
    if(ZEND_NUM_ARGS() > 0)
    {
        WRONG_PARAM_COUNT;
    }

    ProxyPtr _this = Wrapper<ProxyPtr>::value(getThis() TSRMLS_CC);
    assert(_this);

    try
    {
        string str = _this->proxy->ice_toString();
        RETURN_STRINGL(STRCAST(str.c_str()), static_cast<int>(str.length()), 1);
    }
    catch(const IceUtil::Exception& ex)
    {
        throwException(ex TSRMLS_CC);
        RETURN_NULL();
    }
}

ZEND_METHOD(Ice_ObjectPrx, ice_getCommunicator)
{
    if(ZEND_NUM_ARGS() > 0)
    {
        WRONG_PARAM_COUNT;
    }

    ProxyPtr _this = Wrapper<ProxyPtr>::value(getThis() TSRMLS_CC);
    assert(_this);

    _this->communicator->getZval(return_value TSRMLS_CC);
}

ZEND_METHOD(Ice_ObjectPrx, ice_toString)
{
    ZEND_MN(Ice_ObjectPrx___toString)(INTERNAL_FUNCTION_PARAM_PASSTHRU);
}

ZEND_METHOD(Ice_ObjectPrx, ice_getIdentity)
{
    if(ZEND_NUM_ARGS() != 0)
    {
        WRONG_PARAM_COUNT;
    }

    ProxyPtr _this = Wrapper<ProxyPtr>::value(getThis() TSRMLS_CC);
    assert(_this);

    createIdentity(return_value, _this->proxy->ice_getIdentity() TSRMLS_CC);
}

ZEND_METHOD(Ice_ObjectPrx, ice_identity)
{
    ProxyPtr _this = Wrapper<ProxyPtr>::value(getThis() TSRMLS_CC);
    assert(_this);

    zend_class_entry* cls = idToClass("::Ice::Identity" TSRMLS_CC);
    assert(cls);

    zval *zid;

    if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, const_cast<char*>("O"), &zid, cls) == FAILURE)
    {
        RETURN_NULL();
    }

    Ice::Identity id;
    if(extractIdentity(zid, id TSRMLS_CC))
    {
        try
        {
            if(!_this->cloneUntyped(return_value, _this->proxy->ice_identity(id) TSRMLS_CC))
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

ZEND_METHOD(Ice_ObjectPrx, ice_getContext)
{
    if(ZEND_NUM_ARGS() != 0)
    {
        WRONG_PARAM_COUNT;
    }

    ProxyPtr _this = Wrapper<ProxyPtr>::value(getThis() TSRMLS_CC);
    assert(_this);

    if(!createStringMap(return_value, _this->proxy->ice_getContext() TSRMLS_CC))
    {
        RETURN_NULL();
    }
}

ZEND_METHOD(Ice_ObjectPrx, ice_context)
{
    zval* arr = 0;

    if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, const_cast<char*>("a"), &arr) == FAILURE)
    {
        RETURN_NULL();
    }

    //
    // Populate the context.
    //
    Ice::Context ctx;
    if(arr && !extractStringMap(arr, ctx TSRMLS_CC))
    {
        RETURN_NULL();
    }

    ProxyPtr _this = Wrapper<ProxyPtr>::value(getThis() TSRMLS_CC);
    assert(_this);

    try
    {
        if(!_this->clone(return_value, _this->proxy->ice_context(ctx) TSRMLS_CC))
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

ZEND_METHOD(Ice_ObjectPrx, ice_getFacet)
{
    if(ZEND_NUM_ARGS() != 0)
    {
        WRONG_PARAM_COUNT;
    }

    ProxyPtr _this = Wrapper<ProxyPtr>::value(getThis() TSRMLS_CC);
    assert(_this);

    try
    {
        string facet = _this->proxy->ice_getFacet();
        ZVAL_STRINGL(return_value, STRCAST(facet.c_str()), static_cast<int>(facet.length()), 1);
    }
    catch(const IceUtil::Exception& ex)
    {
        throwException(ex TSRMLS_CC);
        RETURN_NULL();
    }
}

ZEND_METHOD(Ice_ObjectPrx, ice_facet)
{
    char* name;
    int len;

    if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, const_cast<char*>("s"), &name, &len) == FAILURE)
    {
        RETURN_NULL();
    }

    ProxyPtr _this = Wrapper<ProxyPtr>::value(getThis() TSRMLS_CC);
    assert(_this);

    try
    {
        if(!_this->cloneUntyped(return_value, _this->proxy->ice_facet(name) TSRMLS_CC))
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

ZEND_METHOD(Ice_ObjectPrx, ice_getAdapterId)
{
    if(ZEND_NUM_ARGS() != 0)
    {
        WRONG_PARAM_COUNT;
    }

    ProxyPtr _this = Wrapper<ProxyPtr>::value(getThis() TSRMLS_CC);
    assert(_this);

    try
    {
        string id = _this->proxy->ice_getAdapterId();
        ZVAL_STRINGL(return_value, STRCAST(id.c_str()), static_cast<int>(id.length()), 1);
    }
    catch(const IceUtil::Exception& ex)
    {
        throwException(ex TSRMLS_CC);
        RETURN_NULL();
    }
}

ZEND_METHOD(Ice_ObjectPrx, ice_adapterId)
{
    ProxyPtr _this = Wrapper<ProxyPtr>::value(getThis() TSRMLS_CC);
    assert(_this);

    char* id;
    int len;

    if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, const_cast<char*>("s"), &id, &len) == FAILURE)
    {
        RETURN_NULL();
    }

    try
    {
        if(!_this->clone(return_value, _this->proxy->ice_adapterId(id) TSRMLS_CC))
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

ZEND_METHOD(Ice_ObjectPrx, ice_getEndpoints)
{
    if(ZEND_NUM_ARGS() != 0)
    {
        WRONG_PARAM_COUNT;
    }

    ProxyPtr _this = Wrapper<ProxyPtr>::value(getThis() TSRMLS_CC);
    assert(_this);

    try
    {
        Ice::EndpointSeq endpoints = _this->proxy->ice_getEndpoints();

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

ZEND_METHOD(Ice_ObjectPrx, ice_endpoints)
{
    ProxyPtr _this = Wrapper<ProxyPtr>::value(getThis() TSRMLS_CC);
    assert(_this);

    zval* zv;

    if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, const_cast<char*>("a"), &zv) == FAILURE)
    {
        RETURN_NULL();
    }

    Ice::EndpointSeq seq;

    HashTable* arr = Z_ARRVAL_P(zv);
    HashPosition pos;
    void* data;

    zend_hash_internal_pointer_reset_ex(arr, &pos);
    while(zend_hash_get_current_data_ex(arr, &data, &pos) != FAILURE)
    {
        zval** val = reinterpret_cast<zval**>(data);

        if(Z_TYPE_PP(val) != IS_OBJECT)
        {
            runtimeError("expected an element of type Ice::Endpoint" TSRMLS_CC);
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
        if(!_this->clone(return_value, _this->proxy->ice_endpoints(seq) TSRMLS_CC))
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

ZEND_METHOD(Ice_ObjectPrx, ice_getLocatorCacheTimeout)
{
    if(ZEND_NUM_ARGS() != 0)
    {
        WRONG_PARAM_COUNT;
    }

    ProxyPtr _this = Wrapper<ProxyPtr>::value(getThis() TSRMLS_CC);
    assert(_this);

    try
    {
        Ice::Int timeout = _this->proxy->ice_getLocatorCacheTimeout();
        ZVAL_LONG(return_value, static_cast<long>(timeout));
    }
    catch(const IceUtil::Exception& ex)
    {
        throwException(ex TSRMLS_CC);
        RETURN_NULL();
    }
}

ZEND_METHOD(Ice_ObjectPrx, ice_getConnectionId)
{
    if(ZEND_NUM_ARGS() != 0)
    {
        WRONG_PARAM_COUNT;
    }

    ProxyPtr _this = Wrapper<ProxyPtr>::value(getThis() TSRMLS_CC);
    assert(_this);

    try
    {
        string connectionId = _this->proxy->ice_getConnectionId();
        ZVAL_STRINGL(return_value, STRCAST(connectionId.c_str()), static_cast<int>(connectionId.length()), 1);
    }
    catch(const IceUtil::Exception& ex)
    {
        throwException(ex TSRMLS_CC);
        RETURN_NULL();
    }
}

ZEND_METHOD(Ice_ObjectPrx, ice_locatorCacheTimeout)
{
    ProxyPtr _this = Wrapper<ProxyPtr>::value(getThis() TSRMLS_CC);
    assert(_this);

    long l;
    if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, const_cast<char*>("l"), &l) != SUCCESS)
    {
        RETURN_NULL();
    }

    try
    {
        if(!_this->clone(return_value, _this->proxy->ice_locatorCacheTimeout(l) TSRMLS_CC))
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

ZEND_METHOD(Ice_ObjectPrx, ice_isConnectionCached)
{
    if(ZEND_NUM_ARGS() != 0)
    {
        WRONG_PARAM_COUNT;
    }

    ProxyPtr _this = Wrapper<ProxyPtr>::value(getThis() TSRMLS_CC);
    assert(_this);

    try
    {
        bool b = _this->proxy->ice_isConnectionCached();
        ZVAL_BOOL(return_value, b ? 1 : 0);
    }
    catch(const IceUtil::Exception& ex)
    {
        throwException(ex TSRMLS_CC);
        RETURN_NULL();
    }
}

ZEND_METHOD(Ice_ObjectPrx, ice_connectionCached)
{
    ProxyPtr _this = Wrapper<ProxyPtr>::value(getThis() TSRMLS_CC);
    assert(_this);

    zend_bool b;
    if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, const_cast<char*>("b"), &b) != SUCCESS)
    {
        RETURN_NULL();
    }

    try
    {
        if(!_this->clone(return_value, _this->proxy->ice_connectionCached(b ? true : false) TSRMLS_CC))
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

ZEND_METHOD(Ice_ObjectPrx, ice_getEndpointSelection)
{
    if(ZEND_NUM_ARGS() != 0)
    {
        WRONG_PARAM_COUNT;
    }

    ProxyPtr _this = Wrapper<ProxyPtr>::value(getThis() TSRMLS_CC);
    assert(_this);

    try
    {
        Ice::EndpointSelectionType type = _this->proxy->ice_getEndpointSelection();
        ZVAL_LONG(return_value, type == Ice::Random ? 0 : 1);
    }
    catch(const IceUtil::Exception& ex)
    {
        throwException(ex TSRMLS_CC);
        RETURN_NULL();
    }
}

ZEND_METHOD(Ice_ObjectPrx, ice_endpointSelection)
{
    ProxyPtr _this = Wrapper<ProxyPtr>::value(getThis() TSRMLS_CC);
    assert(_this);

    long l;
    if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, const_cast<char*>("l"), &l) != SUCCESS)
    {
        RETURN_NULL();
    }

    if(l < 0 || l > 1)
    {
        runtimeError("expecting Random or Ordered" TSRMLS_CC);
        RETURN_NULL();
    }

    try
    {
        Ice::EndpointSelectionType type = l == 0 ? Ice::Random : Ice::Ordered;
        if(!_this->clone(return_value, _this->proxy->ice_endpointSelection(type) TSRMLS_CC))
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

ZEND_METHOD(Ice_ObjectPrx, ice_isSecure)
{
    if(ZEND_NUM_ARGS() != 0)
    {
        WRONG_PARAM_COUNT;
    }

    ProxyPtr _this = Wrapper<ProxyPtr>::value(getThis() TSRMLS_CC);
    assert(_this);

    try
    {
        bool b = _this->proxy->ice_isSecure();
        RETURN_BOOL(b ? 1 : 0);
    }
    catch(const IceUtil::Exception& ex)
    {
        throwException(ex TSRMLS_CC);
        RETURN_FALSE;
    }
}

ZEND_METHOD(Ice_ObjectPrx, ice_secure)
{
    ProxyPtr _this = Wrapper<ProxyPtr>::value(getThis() TSRMLS_CC);
    assert(_this);

    zend_bool b;
    if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, const_cast<char*>("b"), &b TSRMLS_CC) != SUCCESS)
    {
        RETURN_NULL();
    }

    try
    {
        if(!_this->clone(return_value, _this->proxy->ice_secure(b ? true : false) TSRMLS_CC))
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

ZEND_METHOD(Ice_ObjectPrx, ice_getEncodingVersion)
{
    if(ZEND_NUM_ARGS() != 0)
    {
        WRONG_PARAM_COUNT;
    }

    ProxyPtr _this = Wrapper<ProxyPtr>::value(getThis() TSRMLS_CC);
    assert(_this);

    try
    {
        if(!createEncodingVersion(return_value, _this->proxy->ice_getEncodingVersion() TSRMLS_CC))
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

ZEND_METHOD(Ice_ObjectPrx, ice_encodingVersion)
{
    ProxyPtr _this = Wrapper<ProxyPtr>::value(getThis() TSRMLS_CC);
    assert(_this);

    zend_class_entry* cls = idToClass("::Ice::EncodingVersion" TSRMLS_CC);
    assert(cls);

    zval *zv;
    if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, const_cast<char*>("O"), &zv, cls) == FAILURE)
    {
        RETURN_NULL();
    }

    Ice::EncodingVersion v;
    if(extractEncodingVersion(zv, v TSRMLS_CC))
    {
        try
        {
            if(!_this->clone(return_value, _this->proxy->ice_encodingVersion(v) TSRMLS_CC))
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

ZEND_METHOD(Ice_ObjectPrx, ice_isPreferSecure)
{
    if(ZEND_NUM_ARGS() != 0)
    {
        WRONG_PARAM_COUNT;
    }

    ProxyPtr _this = Wrapper<ProxyPtr>::value(getThis() TSRMLS_CC);
    assert(_this);

    try
    {
        bool b = _this->proxy->ice_isPreferSecure();
        RETURN_BOOL(b ? 1 : 0);
    }
    catch(const IceUtil::Exception& ex)
    {
        throwException(ex TSRMLS_CC);
        RETURN_FALSE;
    }
}

ZEND_METHOD(Ice_ObjectPrx, ice_preferSecure)
{
    ProxyPtr _this = Wrapper<ProxyPtr>::value(getThis() TSRMLS_CC);
    assert(_this);

    zend_bool b;
    if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, const_cast<char*>("b"), &b TSRMLS_CC) != SUCCESS)
    {
        RETURN_NULL();
    }

    try
    {
        if(!_this->clone(return_value, _this->proxy->ice_preferSecure(b ? true : false) TSRMLS_CC))
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

ZEND_METHOD(Ice_ObjectPrx, ice_getRouter)
{
    if(ZEND_NUM_ARGS() != 0)
    {
        WRONG_PARAM_COUNT;
    }

    ProxyPtr _this = Wrapper<ProxyPtr>::value(getThis() TSRMLS_CC);
    assert(_this);

    try
    {
        Ice::RouterPrx router = _this->proxy->ice_getRouter();
        if(router)
        {
            ProxyInfoPtr info = getProxyInfo("::Ice::Router" TSRMLS_CC);
            if(!info)
            {
                RETURN_NULL();
            }

            assert(info);

            if(!createProxy(return_value, router, info, _this->communicator TSRMLS_CC))
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

ZEND_METHOD(Ice_ObjectPrx, ice_router)
{
    ProxyPtr _this = Wrapper<ProxyPtr>::value(getThis() TSRMLS_CC);
    assert(_this);

    zval* zprx;
    if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, const_cast<char*>("O!"), &zprx, proxyClassEntry TSRMLS_CC) !=
        SUCCESS)
    {
        RETURN_NULL();
    }

    Ice::ObjectPrx proxy;
    ProxyInfoPtr def;
    if(zprx && !fetchProxy(zprx, proxy, def TSRMLS_CC))
    {
        RETURN_NULL();
    }

    Ice::RouterPrx router;
    if(proxy)
    {
        if(!def || !def->isA("::Ice::Router"))
        {
            runtimeError("ice_router requires a proxy narrowed to Ice::Router" TSRMLS_CC);
            RETURN_NULL();
        }
        router = Ice::RouterPrx::uncheckedCast(proxy);
    }

    try
    {
        if(!_this->clone(return_value, _this->proxy->ice_router(router) TSRMLS_CC))
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

ZEND_METHOD(Ice_ObjectPrx, ice_getLocator)
{
    if(ZEND_NUM_ARGS() != 0)
    {
        WRONG_PARAM_COUNT;
    }

    ProxyPtr _this = Wrapper<ProxyPtr>::value(getThis() TSRMLS_CC);
    assert(_this);

    try
    {
        Ice::LocatorPrx locator = _this->proxy->ice_getLocator();
        if(locator)
        {
            ProxyInfoPtr info = getProxyInfo("::Ice::Locator" TSRMLS_CC);
            if(!info)
            {
                RETURN_NULL();
            }

            if(!createProxy(return_value, locator, info, _this->communicator TSRMLS_CC))
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

ZEND_METHOD(Ice_ObjectPrx, ice_locator)
{
    ProxyPtr _this = Wrapper<ProxyPtr>::value(getThis() TSRMLS_CC);
    assert(_this);

    zval* zprx;
    if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, const_cast<char*>("O!"), &zprx, proxyClassEntry TSRMLS_CC) !=
        SUCCESS)
    {
        RETURN_NULL();
    }

    Ice::ObjectPrx proxy;
    ProxyInfoPtr def;
    if(zprx && !fetchProxy(zprx, proxy, def TSRMLS_CC))
    {
        RETURN_NULL();
    }

    Ice::LocatorPrx locator;
    if(proxy)
    {
        if(!def || !def->isA("::Ice::Locator"))
        {
            runtimeError("ice_locator requires a proxy narrowed to Ice::Locator" TSRMLS_CC);
            RETURN_NULL();
        }
        locator = Ice::LocatorPrx::uncheckedCast(proxy);
    }

    try
    {
        if(!_this->clone(return_value, _this->proxy->ice_locator(locator) TSRMLS_CC))
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

ZEND_METHOD(Ice_ObjectPrx, ice_twoway)
{
    if(ZEND_NUM_ARGS() != 0)
    {
        WRONG_PARAM_COUNT;
    }

    ProxyPtr _this = Wrapper<ProxyPtr>::value(getThis() TSRMLS_CC);
    assert(_this);

    try
    {
        if(!_this->clone(return_value, _this->proxy->ice_twoway() TSRMLS_CC))
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

ZEND_METHOD(Ice_ObjectPrx, ice_isTwoway)
{
    if(ZEND_NUM_ARGS() != 0)
    {
        WRONG_PARAM_COUNT;
    }

    ProxyPtr _this = Wrapper<ProxyPtr>::value(getThis() TSRMLS_CC);
    assert(_this);

    try
    {
        bool b = _this->proxy->ice_isTwoway();
        RETURN_BOOL(b ? 1 : 0);
    }
    catch(const IceUtil::Exception& ex)
    {
        throwException(ex TSRMLS_CC);
        RETURN_FALSE;
    }
}

ZEND_METHOD(Ice_ObjectPrx, ice_oneway)
{
    if(ZEND_NUM_ARGS() != 0)
    {
        WRONG_PARAM_COUNT;
    }

    ProxyPtr _this = Wrapper<ProxyPtr>::value(getThis() TSRMLS_CC);
    assert(_this);

    try
    {
        if(!_this->clone(return_value, _this->proxy->ice_oneway() TSRMLS_CC))
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

ZEND_METHOD(Ice_ObjectPrx, ice_isOneway)
{
    if(ZEND_NUM_ARGS() != 0)
    {
        WRONG_PARAM_COUNT;
    }

    ProxyPtr _this = Wrapper<ProxyPtr>::value(getThis() TSRMLS_CC);
    assert(_this);

    try
    {
        bool b = _this->proxy->ice_isOneway();
        RETURN_BOOL(b ? 1 : 0);
    }
    catch(const IceUtil::Exception& ex)
    {
        throwException(ex TSRMLS_CC);
        RETURN_FALSE;
    }
}

ZEND_METHOD(Ice_ObjectPrx, ice_batchOneway)
{
    if(ZEND_NUM_ARGS() != 0)
    {
        WRONG_PARAM_COUNT;
    }

    ProxyPtr _this = Wrapper<ProxyPtr>::value(getThis() TSRMLS_CC);
    assert(_this);

    try
    {
        if(!_this->clone(return_value, _this->proxy->ice_batchOneway() TSRMLS_CC))
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

ZEND_METHOD(Ice_ObjectPrx, ice_isBatchOneway)
{
    if(ZEND_NUM_ARGS() != 0)
    {
        WRONG_PARAM_COUNT;
    }

    ProxyPtr _this = Wrapper<ProxyPtr>::value(getThis() TSRMLS_CC);
    assert(_this);

    try
    {
        bool b = _this->proxy->ice_isBatchOneway();
        RETURN_BOOL(b ? 1 : 0);
    }
    catch(const IceUtil::Exception& ex)
    {
        throwException(ex TSRMLS_CC);
        RETURN_FALSE;
    }
}

ZEND_METHOD(Ice_ObjectPrx, ice_datagram)
{
    if(ZEND_NUM_ARGS() != 0)
    {
        WRONG_PARAM_COUNT;
    }

    ProxyPtr _this = Wrapper<ProxyPtr>::value(getThis() TSRMLS_CC);
    assert(_this);

    try
    {
        if(!_this->clone(return_value, _this->proxy->ice_datagram() TSRMLS_CC))
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

ZEND_METHOD(Ice_ObjectPrx, ice_isDatagram)
{
    if(ZEND_NUM_ARGS() != 0)
    {
        WRONG_PARAM_COUNT;
    }

    ProxyPtr _this = Wrapper<ProxyPtr>::value(getThis() TSRMLS_CC);
    assert(_this);

    try
    {
        bool b = _this->proxy->ice_isDatagram();
        RETURN_BOOL(b ? 1 : 0);
    }
    catch(const IceUtil::Exception& ex)
    {
        throwException(ex TSRMLS_CC);
        RETURN_FALSE;
    }
}

ZEND_METHOD(Ice_ObjectPrx, ice_batchDatagram)
{
    if(ZEND_NUM_ARGS() != 0)
    {
        WRONG_PARAM_COUNT;
    }

    ProxyPtr _this = Wrapper<ProxyPtr>::value(getThis() TSRMLS_CC);
    assert(_this);

    try
    {
        if(!_this->clone(return_value, _this->proxy->ice_batchDatagram() TSRMLS_CC))
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

ZEND_METHOD(Ice_ObjectPrx, ice_isBatchDatagram)
{
    if(ZEND_NUM_ARGS() != 0)
    {
        WRONG_PARAM_COUNT;
    }

    ProxyPtr _this = Wrapper<ProxyPtr>::value(getThis() TSRMLS_CC);
    assert(_this);

    try
    {
        bool b = _this->proxy->ice_isBatchDatagram();
        RETURN_BOOL(b ? 1 : 0);
    }
    catch(const IceUtil::Exception& ex)
    {
        throwException(ex TSRMLS_CC);
        RETURN_FALSE;
    }
}

ZEND_METHOD(Ice_ObjectPrx, ice_compress)
{
    ProxyPtr _this = Wrapper<ProxyPtr>::value(getThis() TSRMLS_CC);
    assert(_this);

    zend_bool b;
    if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, const_cast<char*>("b"), &b) != SUCCESS)
    {
        RETURN_NULL();
    }

    try
    {
        if(!_this->clone(return_value, _this->proxy->ice_compress(b ? true : false) TSRMLS_CC))
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

ZEND_METHOD(Ice_ObjectPrx, ice_timeout)
{
    ProxyPtr _this = Wrapper<ProxyPtr>::value(getThis() TSRMLS_CC);
    assert(_this);

    try
    {
        long l;
        if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, const_cast<char*>("l"), &l) != SUCCESS)
        {
            RETURN_NULL();
        }
        // TODO: range check?
        if(!_this->clone(return_value, _this->proxy->ice_timeout(l) TSRMLS_CC))
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

ZEND_METHOD(Ice_ObjectPrx, ice_connectionId)
{
    ProxyPtr _this = Wrapper<ProxyPtr>::value(getThis() TSRMLS_CC);
    assert(_this);

    try
    {
        char* id;
        int idLen;
        if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, const_cast<char*>("s"), &id, &idLen) != SUCCESS)
        {
            RETURN_NULL();
        }
        if(!_this->clone(return_value, _this->proxy->ice_connectionId(id) TSRMLS_CC))
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

ZEND_METHOD(Ice_ObjectPrx, ice_getConnection)
{
    if(ZEND_NUM_ARGS() != 0)
    {
        WRONG_PARAM_COUNT;
    }

    ProxyPtr _this = Wrapper<ProxyPtr>::value(getThis() TSRMLS_CC);
    assert(_this);

    try
    {
        Ice::ConnectionPtr con = _this->proxy->ice_getConnection();
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

ZEND_METHOD(Ice_ObjectPrx, ice_getCachedConnection)
{
    if(ZEND_NUM_ARGS() != 0)
    {
        WRONG_PARAM_COUNT;
    }

    ProxyPtr _this = Wrapper<ProxyPtr>::value(getThis() TSRMLS_CC);
    assert(_this);

    try
    {
        Ice::ConnectionPtr con = _this->proxy->ice_getCachedConnection();
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

ZEND_METHOD(Ice_ObjectPrx, ice_flushBatchRequests)
{
    if(ZEND_NUM_ARGS() != 0)
    {
        WRONG_PARAM_COUNT;
    }

    ProxyPtr _this = Wrapper<ProxyPtr>::value(getThis() TSRMLS_CC);
    assert(_this);

    try
    {
        _this->proxy->ice_flushBatchRequests();
    }
    catch(const IceUtil::Exception& ex)
    {
        throwException(ex TSRMLS_CC);
        RETURN_NULL();
    }
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

    if(zend_parse_parameters_ex(ZEND_PARSE_PARAMS_QUIET, ZEND_NUM_ARGS() TSRMLS_CC, const_cast<char*>("s|s!a!"), &id,
                                &idLen, &facet, &facetLen, &arr) == FAILURE)
    {
        facet = 0;
        if(zend_parse_parameters_ex(ZEND_PARSE_PARAMS_QUIET, ZEND_NUM_ARGS() TSRMLS_CC, const_cast<char*>("s|a!"), &id,
                                    &idLen, &arr) == FAILURE)
        {
            php_error(E_ERROR, "%s() requires a type id followed by an optional facet and/or context",
                      get_active_function_name(TSRMLS_C));
            return;
        }
    }

    ProxyPtr _this = Wrapper<ProxyPtr>::value(getThis() TSRMLS_CC);
    assert(_this);

    //
    // Populate the context.
    //
    Ice::Context ctx;
    if(arr && !extractStringMap(arr, ctx TSRMLS_CC))
    {
        RETURN_NULL();
    }

    try
    {
        ProxyInfoPtr info = getProxyInfo(id TSRMLS_CC);
        if(!info)
        {
            RETURN_NULL();
        }

        Ice::ObjectPrx prx = _this->proxy;
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
            //
            // Verify that the object supports the requested type.
            //
            if(!prx->ice_isA(info->id))
            {
                RETURN_NULL();
            }
        }

        if(!createProxy(return_value, prx, info, _this->communicator TSRMLS_CC))
        {
            RETURN_NULL();
        }
    }
    catch(const Ice::FacetNotExistException&)
    {
        // Ignore.
    }
    catch(const IceUtil::Exception& ex)
    {
        throwException(ex TSRMLS_CC);
        RETVAL_FALSE;
    }
}

ZEND_METHOD(Ice_ObjectPrx, ice_uncheckedCast)
{
    do_cast(INTERNAL_FUNCTION_PARAM_PASSTHRU, false);
}

ZEND_METHOD(Ice_ObjectPrx, ice_checkedCast)
{
    do_cast(INTERNAL_FUNCTION_PARAM_PASSTHRU, true);
}

IcePHP::Proxy::Proxy(const Ice::ObjectPrx& p, const ProxyInfoPtr& i, const CommunicatorInfoPtr& comm TSRMLS_DC) :
    proxy(p), info(i), communicator(comm), connection(0), cachedConnection(0)
{
#ifdef ZTS
    this->TSRMLS_C = TSRMLS_C;
#endif

    //
    // We want to ensure that the PHP object corresponding to the communicator is
    // not destroyed until after this proxy is destroyed.
    //
    communicator->addRef(TSRMLS_C);
}

IcePHP::Proxy::~Proxy()
{
    communicator->decRef(TSRMLS_C);
    if(connection)
    {
        zval_ptr_dtor(&connection);
    }
    if(cachedConnection)
    {
        zval_ptr_dtor(&cachedConnection);
    }
}

bool
IcePHP::Proxy::clone(zval* zv, const Ice::ObjectPrx& p TSRMLS_DC)
{
    return create(zv, p, info, communicator TSRMLS_CC);
}

bool
IcePHP::Proxy::cloneUntyped(zval* zv, const Ice::ObjectPrx& p TSRMLS_DC)
{
    return create(zv, p, 0, communicator TSRMLS_CC);
}

bool
IcePHP::Proxy::create(zval* zv, const Ice::ObjectPrx& p, const ProxyInfoPtr& info, const CommunicatorInfoPtr& comm
                      TSRMLS_DC)
{
    ProxyInfoPtr prx = info;
    if(!prx)
    {
        prx = getProxyInfo("::Ice::Object" TSRMLS_CC);
        assert(prx);
    }

    if(object_init_ex(zv, proxyClassEntry) != SUCCESS)
    {
        runtimeError("unable to initialize proxy" TSRMLS_CC);
        return false;
    }

    Wrapper<ProxyPtr>* obj = Wrapper<ProxyPtr>::extract(zv TSRMLS_CC);
    ProxyPtr proxy = new Proxy(p, prx, comm TSRMLS_CC);
    assert(!obj->ptr);
    obj->ptr = new ProxyPtr(proxy);

    return true;
}

#ifdef _WIN32
extern "C"
#endif
static zend_object_value
handleAlloc(zend_class_entry* ce TSRMLS_DC)
{
    zend_object_value result;

    Wrapper<ProxyPtr>* obj = Wrapper<ProxyPtr>::create(ce TSRMLS_CC);
    assert(obj);

    result.handle = zend_objects_store_put(obj, 0, (zend_objects_free_object_storage_t)handleFreeStorage,
                                           0 TSRMLS_CC);
    result.handlers = &_handlers;

    return result;
}

#ifdef _WIN32
extern "C"
#endif
static void
handleFreeStorage(void* p TSRMLS_DC)
{
    Wrapper<ProxyPtr>* obj = static_cast<Wrapper<ProxyPtr>*>(p);
    delete obj->ptr;
    zend_object_std_dtor(static_cast<zend_object*>(p) TSRMLS_CC);
    efree(p);
}

#ifdef _WIN32
extern "C"
#endif
static zend_object_value
handleClone(zval* zv TSRMLS_DC)
{
    //
    // Create a new object that shares a C++ proxy instance with this object.
    //

    zend_object_value result;
    memset(&result, 0, sizeof(zend_object_value));

    ProxyPtr obj = Wrapper<ProxyPtr>::value(zv TSRMLS_CC);
    assert(obj);

    zval* clone;
    MAKE_STD_ZVAL(clone);
    if(!obj->clone(clone, obj->proxy TSRMLS_CC))
    {
        return result;
    }

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

#ifdef _WIN32
extern "C"
#endif
static union _zend_function*
#if PHP_VERSION_ID < 50400
handleGetMethod(zval** zv, char* method, int len TSRMLS_DC)
#else
handleGetMethod(zval** zv, char* method, int len, const _zend_literal* key TSRMLS_DC)
#endif
{
    zend_function* result;

    //
    // First delegate to the standard implementation of get_method. This will find
    // any of our predefined proxy methods. If it returns 0, then we return a
    // function that will check the class definition.
    //
#if PHP_VERSION_ID < 50400
    result = zend_get_std_object_handlers()->get_method(zv, method, len TSRMLS_CC);
#else
    result = zend_get_std_object_handlers()->get_method(zv, method, len, key TSRMLS_CC);
#endif
    if(!result)
    {
        Wrapper<ProxyPtr>* obj = Wrapper<ProxyPtr>::extract(*zv TSRMLS_CC);
        assert(obj->ptr);
        ProxyPtr _this = *obj->ptr;

        ProxyInfoPtr info = _this->info;
        assert(info);

        OperationPtr op = info->getOperation(method);
        if(!op)
        {
            //
            // Returning 0 causes PHP to report an "undefined method" error.
            //
            return 0;
        }

        result = op->function();
    }

    return result;
}

#ifdef _WIN32
extern "C"
#endif
static int
handleCompare(zval* zobj1, zval* zobj2 TSRMLS_DC)
{
    //
    // PHP guarantees that the objects have the same class.
    //

    Wrapper<ProxyPtr>* obj1 = Wrapper<ProxyPtr>::extract(zobj1 TSRMLS_CC);
    assert(obj1->ptr);
    ProxyPtr _this1 = *obj1->ptr;
    Ice::ObjectPrx prx1 = _this1->proxy;

    Wrapper<ProxyPtr>* obj2 = Wrapper<ProxyPtr>::extract(zobj2 TSRMLS_CC);
    assert(obj2->ptr);
    ProxyPtr _this2 = *obj2->ptr;
    Ice::ObjectPrx prx2 = _this2->proxy;

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

//
// Necessary to suppress warnings from zend_function_entry in php-5.2.
//
#if defined(__GNUC__)
#  pragma GCC diagnostic ignored "-Wwrite-strings"
#endif

//
// Predefined methods for ObjectPrx.
//
static zend_function_entry _proxyMethods[] =
{
    ZEND_ME(Ice_ObjectPrx, __construct, ICE_NULLPTR, ZEND_ACC_PRIVATE|ZEND_ACC_CTOR)
    ZEND_ME(Ice_ObjectPrx, __toString, ICE_NULLPTR, ZEND_ACC_PUBLIC)
    ZEND_ME(Ice_ObjectPrx, ice_getCommunicator, ICE_NULLPTR, ZEND_ACC_PUBLIC)
    ZEND_ME(Ice_ObjectPrx, ice_toString, ICE_NULLPTR, ZEND_ACC_PUBLIC)
    ZEND_ME(Ice_ObjectPrx, ice_getIdentity, ICE_NULLPTR, ZEND_ACC_PUBLIC)
    ZEND_ME(Ice_ObjectPrx, ice_identity, ICE_NULLPTR, ZEND_ACC_PUBLIC)
    ZEND_ME(Ice_ObjectPrx, ice_getContext, ICE_NULLPTR, ZEND_ACC_PUBLIC)
    ZEND_ME(Ice_ObjectPrx, ice_context, ICE_NULLPTR, ZEND_ACC_PUBLIC)
    ZEND_ME(Ice_ObjectPrx, ice_getFacet, ICE_NULLPTR, ZEND_ACC_PUBLIC)
    ZEND_ME(Ice_ObjectPrx, ice_facet, ICE_NULLPTR, ZEND_ACC_PUBLIC)
    ZEND_ME(Ice_ObjectPrx, ice_getAdapterId, ICE_NULLPTR, ZEND_ACC_PUBLIC)
    ZEND_ME(Ice_ObjectPrx, ice_adapterId, ICE_NULLPTR, ZEND_ACC_PUBLIC)
    ZEND_ME(Ice_ObjectPrx, ice_getEndpoints, ICE_NULLPTR, ZEND_ACC_PUBLIC)
    ZEND_ME(Ice_ObjectPrx, ice_endpoints, ICE_NULLPTR, ZEND_ACC_PUBLIC)
    ZEND_ME(Ice_ObjectPrx, ice_getLocatorCacheTimeout, ICE_NULLPTR, ZEND_ACC_PUBLIC)
    ZEND_ME(Ice_ObjectPrx, ice_getConnectionId, ICE_NULLPTR, ZEND_ACC_PUBLIC)
    ZEND_ME(Ice_ObjectPrx, ice_locatorCacheTimeout, ICE_NULLPTR, ZEND_ACC_PUBLIC)
    ZEND_ME(Ice_ObjectPrx, ice_isConnectionCached, ICE_NULLPTR, ZEND_ACC_PUBLIC)
    ZEND_ME(Ice_ObjectPrx, ice_connectionCached, ICE_NULLPTR, ZEND_ACC_PUBLIC)
    ZEND_ME(Ice_ObjectPrx, ice_getEndpointSelection, ICE_NULLPTR, ZEND_ACC_PUBLIC)
    ZEND_ME(Ice_ObjectPrx, ice_endpointSelection, ICE_NULLPTR, ZEND_ACC_PUBLIC)
    ZEND_ME(Ice_ObjectPrx, ice_isSecure, ICE_NULLPTR, ZEND_ACC_PUBLIC)
    ZEND_ME(Ice_ObjectPrx, ice_secure, ICE_NULLPTR, ZEND_ACC_PUBLIC)
    ZEND_ME(Ice_ObjectPrx, ice_getEncodingVersion, ICE_NULLPTR, ZEND_ACC_PUBLIC)
    ZEND_ME(Ice_ObjectPrx, ice_encodingVersion, ICE_NULLPTR, ZEND_ACC_PUBLIC)
    ZEND_ME(Ice_ObjectPrx, ice_isPreferSecure, ICE_NULLPTR, ZEND_ACC_PUBLIC)
    ZEND_ME(Ice_ObjectPrx, ice_preferSecure, ICE_NULLPTR, ZEND_ACC_PUBLIC)
    ZEND_ME(Ice_ObjectPrx, ice_getRouter, ICE_NULLPTR, ZEND_ACC_PUBLIC)
    ZEND_ME(Ice_ObjectPrx, ice_router, ICE_NULLPTR, ZEND_ACC_PUBLIC)
    ZEND_ME(Ice_ObjectPrx, ice_getLocator, ICE_NULLPTR, ZEND_ACC_PUBLIC)
    ZEND_ME(Ice_ObjectPrx, ice_locator, ICE_NULLPTR, ZEND_ACC_PUBLIC)
    ZEND_ME(Ice_ObjectPrx, ice_twoway, ICE_NULLPTR, ZEND_ACC_PUBLIC)
    ZEND_ME(Ice_ObjectPrx, ice_isTwoway, ICE_NULLPTR, ZEND_ACC_PUBLIC)
    ZEND_ME(Ice_ObjectPrx, ice_oneway, ICE_NULLPTR, ZEND_ACC_PUBLIC)
    ZEND_ME(Ice_ObjectPrx, ice_isOneway, ICE_NULLPTR, ZEND_ACC_PUBLIC)
    ZEND_ME(Ice_ObjectPrx, ice_batchOneway, ICE_NULLPTR, ZEND_ACC_PUBLIC)
    ZEND_ME(Ice_ObjectPrx, ice_isBatchOneway, ICE_NULLPTR, ZEND_ACC_PUBLIC)
    ZEND_ME(Ice_ObjectPrx, ice_datagram, ICE_NULLPTR, ZEND_ACC_PUBLIC)
    ZEND_ME(Ice_ObjectPrx, ice_isDatagram, ICE_NULLPTR, ZEND_ACC_PUBLIC)
    ZEND_ME(Ice_ObjectPrx, ice_batchDatagram, ICE_NULLPTR, ZEND_ACC_PUBLIC)
    ZEND_ME(Ice_ObjectPrx, ice_isBatchDatagram, ICE_NULLPTR, ZEND_ACC_PUBLIC)
    ZEND_ME(Ice_ObjectPrx, ice_compress, ICE_NULLPTR, ZEND_ACC_PUBLIC)
    ZEND_ME(Ice_ObjectPrx, ice_timeout, ICE_NULLPTR, ZEND_ACC_PUBLIC)
    ZEND_ME(Ice_ObjectPrx, ice_connectionId, ICE_NULLPTR, ZEND_ACC_PUBLIC)
    ZEND_ME(Ice_ObjectPrx, ice_getConnection, ICE_NULLPTR, ZEND_ACC_PUBLIC)
    ZEND_ME(Ice_ObjectPrx, ice_getCachedConnection, ICE_NULLPTR, ZEND_ACC_PUBLIC)
    ZEND_ME(Ice_ObjectPrx, ice_flushBatchRequests, ICE_NULLPTR, ZEND_ACC_PUBLIC)
    ZEND_ME(Ice_ObjectPrx, ice_uncheckedCast, ICE_NULLPTR, ZEND_ACC_PUBLIC)
    ZEND_ME(Ice_ObjectPrx, ice_checkedCast, ICE_NULLPTR, ZEND_ACC_PUBLIC)
    {0, 0, 0}
};

//
// enable warning again
//
#if defined(__GNUC__)
#  pragma GCC diagnostic error "-Wwrite-strings"
#endif

bool
IcePHP::proxyInit(TSRMLS_D)
{
    //
    // Register the ObjectPrx class.
    //
    zend_class_entry ce;
#ifdef ICEPHP_USE_NAMESPACES
    INIT_NS_CLASS_ENTRY(ce, "Ice", "ObjectPrx", _proxyMethods);
#else
    INIT_CLASS_ENTRY(ce, "Ice_ObjectPrx", _proxyMethods);
#endif
    ce.create_object = handleAlloc;
    proxyClassEntry = zend_register_internal_class(&ce TSRMLS_CC);
    //proxyClassEntry->ce_flags |= ZEND_ACC_EXPLICIT_ABSTRACT_CLASS;
    memcpy(&_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
    _handlers.clone_obj = handleClone;
    _handlers.get_method = handleGetMethod;
    _handlers.compare_objects = handleCompare;

    return true;
}

bool
IcePHP::createProxy(zval* zv, const Ice::ObjectPrx& p, const CommunicatorInfoPtr& comm TSRMLS_DC)
{
    return Proxy::create(zv, p, 0, comm TSRMLS_CC);
}

bool
IcePHP::createProxy(zval* zv, const Ice::ObjectPrx& p, const ProxyInfoPtr& info, const CommunicatorInfoPtr& comm
                    TSRMLS_DC)
{
    return Proxy::create(zv, p, info, comm TSRMLS_CC);
}

bool
IcePHP::fetchProxy(zval* zv, Ice::ObjectPrx& prx, ProxyInfoPtr& info TSRMLS_DC)
{
    CommunicatorInfoPtr comm;
    return fetchProxy(zv, prx, info, comm TSRMLS_CC);
}

bool
IcePHP::fetchProxy(zval* zv, Ice::ObjectPrx& prx, ProxyInfoPtr& info, CommunicatorInfoPtr& comm TSRMLS_DC)
{
    if(!ZVAL_IS_NULL(zv))
    {
        if(Z_TYPE_P(zv) != IS_OBJECT || Z_OBJCE_P(zv) != proxyClassEntry)
        {
            invalidArgument("value is not a proxy" TSRMLS_CC);
            return false;
        }
        Wrapper<ProxyPtr>* obj = Wrapper<ProxyPtr>::extract(zv TSRMLS_CC);
        if(!obj)
        {
            runtimeError("unable to retrieve proxy object from object store" TSRMLS_CC);
            return false;
        }
        assert(obj->ptr);
        prx = (*obj->ptr)->proxy;
        info = (*obj->ptr)->info;
        comm = (*obj->ptr)->communicator;
    }
    return true;
}
