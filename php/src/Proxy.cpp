// Copyright (c) ZeroC, Inc.

#include "Proxy.h"
#include "Connection.h"
#include "Endpoint.h"
#include "Util.h"

using namespace std;
using namespace IcePHP;

ZEND_EXTERN_MODULE_GLOBALS(ice)

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
// a new proxy containing the interface definition. This proxy is considered
// to be narrowed to that interface and therefore supports user-defined operations.
//
// Naturally, there are many predefined proxy methods (e.g., ice_getIdentity, etc.), but
// the proxy also needs to support user-defined operations (if it has type information).
// We use a Zend API hook that allows us to intercept the invocation of unknown methods
// on the proxy object.

// Class entries represent the PHP class implementations we have registered.
namespace IcePHP
{
    zend_class_entry* proxyClassEntry = 0;
}

// Ice::ObjectPrx support.
static zend_object_handlers _handlers;

extern "C"
{
    static zend_object* handleAlloc(zend_class_entry*);
    static void handleFreeStorage(zend_object*);
    static zend_object* handleClone(zend_object*);
    static union _zend_function* handleGetMethod(zend_object**, zend_string*, const zval*);
    static int handleCompare(zval*, zval*);
}

namespace IcePHP
{
    // Encapsulates proxy and type information.
    class Proxy
    {
    public:
        Proxy(Ice::ObjectPrx, ProxyInfoPtr, CommunicatorInfoPtr);
        ~Proxy();

        bool clone(zval*, Ice::ObjectPrx);
        bool cloneUntyped(zval*, Ice::ObjectPrx);
        static bool create(zval*, Ice::ObjectPrx, ProxyInfoPtr, CommunicatorInfoPtr);

        Ice::ObjectPrx proxy;
        ProxyInfoPtr info;
        CommunicatorInfoPtr communicator;
        zval* connection;
        zval* cachedConnection;
    };
    using ProxyPtr = shared_ptr<Proxy>;

} // End of namespace IcePHP

ZEND_METHOD(Ice_ObjectPrx, __construct) { runtimeError("proxies cannot be instantiated, use stringToProxy()"); }

ZEND_METHOD(Ice_ObjectPrx, __toString)
{
    if (ZEND_NUM_ARGS() > 0)
    {
        WRONG_PARAM_COUNT;
    }

    ProxyPtr _this = Wrapper<ProxyPtr>::value(getThis());
    assert(_this);

    try
    {
        string str = _this->proxy->ice_toString();
        RETURN_STRINGL(str.c_str(), static_cast<int>(str.length()));
    }
    catch (...)
    {
        throwException(current_exception());
        RETURN_NULL();
    }
}

ZEND_METHOD(Ice_ObjectPrx, ice_getCommunicator)
{
    if (ZEND_NUM_ARGS() > 0)
    {
        WRONG_PARAM_COUNT;
    }

    ProxyPtr _this = Wrapper<ProxyPtr>::value(getThis());
    assert(_this);

    _this->communicator->getZval(return_value);
}

ZEND_METHOD(Ice_ObjectPrx, ice_toString) { ZEND_MN(Ice_ObjectPrx___toString)(INTERNAL_FUNCTION_PARAM_PASSTHRU); }

ZEND_METHOD(Ice_ObjectPrx, ice_getIdentity)
{
    if (ZEND_NUM_ARGS() != 0)
    {
        WRONG_PARAM_COUNT;
    }

    ProxyPtr _this = Wrapper<ProxyPtr>::value(getThis());
    assert(_this);

    createIdentity(return_value, _this->proxy->ice_getIdentity());
}

ZEND_BEGIN_ARG_INFO_EX(Ice_ObjectPrx_ice_identity_arginfo, 1, ZEND_RETURN_VALUE, static_cast<zend_ulong>(1))
ZEND_ARG_INFO(0, id)
ZEND_END_ARG_INFO()

ZEND_METHOD(Ice_ObjectPrx, ice_identity)
{
    ProxyPtr _this = Wrapper<ProxyPtr>::value(getThis());
    assert(_this);

    zend_class_entry* cls = idToClass("::Ice::Identity");
    assert(cls);

    zval* zid;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), const_cast<char*>("O"), &zid, cls) == FAILURE)
    {
        RETURN_NULL();
    }

    Ice::Identity id;
    if (extractIdentity(zid, id))
    {
        try
        {
            if (!_this->cloneUntyped(return_value, _this->proxy->ice_identity(id)))
            {
                RETURN_NULL();
            }
        }
        catch (...)
        {
            throwException(current_exception());
            RETURN_NULL();
        }
    }
}

ZEND_METHOD(Ice_ObjectPrx, ice_getContext)
{
    if (ZEND_NUM_ARGS() != 0)
    {
        WRONG_PARAM_COUNT;
    }

    ProxyPtr _this = Wrapper<ProxyPtr>::value(getThis());
    assert(_this);

    if (!createContext(return_value, _this->proxy->ice_getContext()))
    {
        RETURN_NULL();
    }
}

ZEND_BEGIN_ARG_INFO_EX(Ice_ObjectPrx_ice_context_arginfo, 1, ZEND_RETURN_VALUE, static_cast<zend_ulong>(1))
ZEND_ARG_INFO(0, ctx)
ZEND_END_ARG_INFO()

ZEND_METHOD(Ice_ObjectPrx, ice_context)
{
    zval* arr = 0;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), const_cast<char*>("a"), &arr) == FAILURE)
    {
        RETURN_NULL();
    }

    // Populate the context.
    Ice::Context ctx;
    if (arr && !extractContext(arr, ctx))
    {
        RETURN_NULL();
    }

    ProxyPtr _this = Wrapper<ProxyPtr>::value(getThis());
    assert(_this);
    try
    {
        if (!_this->clone(return_value, _this->proxy->ice_context(ctx)))
        {
            RETURN_NULL();
        }
    }
    catch (...)
    {
        throwException(current_exception());
        RETURN_NULL();
    }
}

ZEND_METHOD(Ice_ObjectPrx, ice_getFacet)
{
    if (ZEND_NUM_ARGS() != 0)
    {
        WRONG_PARAM_COUNT;
    }

    ProxyPtr _this = Wrapper<ProxyPtr>::value(getThis());
    assert(_this);

    try
    {
        string facet = _this->proxy->ice_getFacet();
        ZVAL_STRINGL(return_value, facet.c_str(), static_cast<int>(facet.length()));
    }
    catch (...)
    {
        throwException(current_exception());
        RETURN_NULL();
    }
}

ZEND_BEGIN_ARG_INFO_EX(Ice_ObjectPrx_ice_facet_arginfo, 1, ZEND_RETURN_VALUE, static_cast<zend_ulong>(1))
ZEND_ARG_INFO(0, facet)
ZEND_END_ARG_INFO()

ZEND_METHOD(Ice_ObjectPrx, ice_facet)
{
    char* name;
    size_t len;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), const_cast<char*>("s"), &name, &len) == FAILURE)
    {
        RETURN_NULL();
    }

    ProxyPtr _this = Wrapper<ProxyPtr>::value(getThis());
    assert(_this);

    try
    {
        if (!_this->cloneUntyped(return_value, _this->proxy->ice_facet(name)))
        {
            RETURN_NULL();
        }
    }
    catch (...)
    {
        throwException(current_exception());
        RETURN_NULL();
    }
}

ZEND_METHOD(Ice_ObjectPrx, ice_getAdapterId)
{
    if (ZEND_NUM_ARGS() != 0)
    {
        WRONG_PARAM_COUNT;
    }

    ProxyPtr _this = Wrapper<ProxyPtr>::value(getThis());
    assert(_this);

    try
    {
        string id = _this->proxy->ice_getAdapterId();
        ZVAL_STRINGL(return_value, id.c_str(), static_cast<int>(id.length()));
    }
    catch (...)
    {
        throwException(current_exception());
        RETURN_NULL();
    }
}

ZEND_BEGIN_ARG_INFO_EX(Ice_ObjectPrx_ice_adapterId_arginfo, 1, ZEND_RETURN_VALUE, static_cast<zend_ulong>(1))
ZEND_ARG_INFO(0, id)
ZEND_END_ARG_INFO()

ZEND_METHOD(Ice_ObjectPrx, ice_adapterId)
{
    ProxyPtr _this = Wrapper<ProxyPtr>::value(getThis());
    assert(_this);

    char* id;
    size_t len;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), const_cast<char*>("s"), &id, &len) == FAILURE)
    {
        RETURN_NULL();
    }

    try
    {
        if (!_this->clone(return_value, _this->proxy->ice_adapterId(id)))
        {
            RETURN_NULL();
        }
    }
    catch (...)
    {
        throwException(current_exception());
        RETURN_NULL();
    }
}

ZEND_METHOD(Ice_ObjectPrx, ice_getEndpoints)
{
    if (ZEND_NUM_ARGS() != 0)
    {
        WRONG_PARAM_COUNT;
    }

    ProxyPtr _this = Wrapper<ProxyPtr>::value(getThis());
    assert(_this);

    try
    {
        Ice::EndpointSeq endpoints = _this->proxy->ice_getEndpoints();

        array_init(return_value);
        uint32_t idx = 0;
        for (const auto& p : endpoints)
        {
            zval elem;
            if (!createEndpoint(&elem, p))
            {
                zval_ptr_dtor(&elem);
                RETURN_NULL();
            }
            add_index_zval(return_value, idx++, &elem);
        }
    }
    catch (...)
    {
        throwException(current_exception());
        RETURN_NULL();
    }
}

ZEND_BEGIN_ARG_INFO_EX(Ice_ObjectPrx_ice_endpoints_arginfo, 1, ZEND_RETURN_VALUE, static_cast<zend_ulong>(1))
ZEND_ARG_INFO(0, endpoints)
ZEND_END_ARG_INFO()

ZEND_METHOD(Ice_ObjectPrx, ice_endpoints)
{
    ProxyPtr _this = Wrapper<ProxyPtr>::value(getThis());
    assert(_this);

    zval* zv;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), const_cast<char*>("a"), &zv) == FAILURE)
    {
        RETURN_NULL();
    }

    Ice::EndpointSeq seq;

    HashTable* arr = Z_ARRVAL_P(zv);
    zval* val;
    ZEND_HASH_FOREACH_VAL(arr, val)
    {
        if (Z_TYPE_P(val) != IS_OBJECT)
        {
            runtimeError("expected an element of type Ice::Endpoint");
            RETURN_NULL();
        }

        Ice::EndpointPtr endpoint;
        if (!fetchEndpoint(val, endpoint))
        {
            RETURN_NULL();
        }

        seq.push_back(endpoint);
    }
    ZEND_HASH_FOREACH_END();

    try
    {
        if (!_this->clone(return_value, _this->proxy->ice_endpoints(seq)))
        {
            RETURN_NULL();
        }
    }
    catch (...)
    {
        throwException(current_exception());
        RETURN_NULL();
    }
}

ZEND_METHOD(Ice_ObjectPrx, ice_getLocatorCacheTimeout)
{
    if (ZEND_NUM_ARGS() != 0)
    {
        WRONG_PARAM_COUNT;
    }

    ProxyPtr _this = Wrapper<ProxyPtr>::value(getThis());
    assert(_this);

    try
    {
        chrono::seconds timeout = chrono::duration_cast<chrono::seconds>(_this->proxy->ice_getLocatorCacheTimeout());
        ZVAL_LONG(return_value, static_cast<long>(timeout.count()));
    }
    catch (...)
    {
        throwException(current_exception());
        RETURN_NULL();
    }
}

ZEND_METHOD(Ice_ObjectPrx, ice_getConnectionId)
{
    if (ZEND_NUM_ARGS() != 0)
    {
        WRONG_PARAM_COUNT;
    }

    ProxyPtr _this = Wrapper<ProxyPtr>::value(getThis());
    assert(_this);

    try
    {
        string connectionId = _this->proxy->ice_getConnectionId();
        ZVAL_STRINGL(return_value, connectionId.c_str(), static_cast<int>(connectionId.length()));
    }
    catch (...)
    {
        throwException(current_exception());
        RETURN_NULL();
    }
}

ZEND_BEGIN_ARG_INFO_EX(Ice_ObjectPrx_ice_locatorCacheTimeout_arginfo, 1, ZEND_RETURN_VALUE, static_cast<zend_ulong>(1))
ZEND_ARG_INFO(0, timeout)
ZEND_END_ARG_INFO()

ZEND_METHOD(Ice_ObjectPrx, ice_locatorCacheTimeout)
{
    ProxyPtr _this = Wrapper<ProxyPtr>::value(getThis());
    assert(_this);

    zend_long l;
    if (zend_parse_parameters(ZEND_NUM_ARGS(), const_cast<char*>("l"), &l) != SUCCESS)
    {
        RETURN_NULL();
    }

    try
    {
        if (!_this->clone(return_value, _this->proxy->ice_locatorCacheTimeout(static_cast<int32_t>(l))))
        {
            RETURN_NULL();
        }
    }
    catch (...)
    {
        throwException(current_exception());
        RETURN_NULL();
    }
}

ZEND_METHOD(Ice_ObjectPrx, ice_isConnectionCached)
{
    if (ZEND_NUM_ARGS() != 0)
    {
        WRONG_PARAM_COUNT;
    }

    ProxyPtr _this = Wrapper<ProxyPtr>::value(getThis());
    assert(_this);

    try
    {
        bool b = _this->proxy->ice_isConnectionCached();
        ZVAL_BOOL(return_value, b ? 1 : 0);
    }
    catch (...)
    {
        throwException(current_exception());
        RETURN_NULL();
    }
}

ZEND_BEGIN_ARG_INFO_EX(Ice_ObjectPrx_ice_connectionCached_arginfo, 1, ZEND_RETURN_VALUE, static_cast<zend_ulong>(1))
ZEND_ARG_INFO(0, cached)
ZEND_END_ARG_INFO()

ZEND_METHOD(Ice_ObjectPrx, ice_connectionCached)
{
    ProxyPtr _this = Wrapper<ProxyPtr>::value(getThis());
    assert(_this);

    zend_bool b;
    if (zend_parse_parameters(ZEND_NUM_ARGS(), const_cast<char*>("b"), &b) != SUCCESS)
    {
        RETURN_NULL();
    }

    try
    {
        if (!_this->clone(return_value, _this->proxy->ice_connectionCached(b ? true : false)))
        {
            RETURN_NULL();
        }
    }
    catch (...)
    {
        throwException(current_exception());
        RETURN_NULL();
    }
}

ZEND_METHOD(Ice_ObjectPrx, ice_getEndpointSelection)
{
    if (ZEND_NUM_ARGS() != 0)
    {
        WRONG_PARAM_COUNT;
    }

    ProxyPtr _this = Wrapper<ProxyPtr>::value(getThis());
    assert(_this);

    try
    {
        Ice::EndpointSelectionType type = _this->proxy->ice_getEndpointSelection();
        ZVAL_LONG(return_value, type == Ice::EndpointSelectionType::Random ? 0 : 1);
    }
    catch (...)
    {
        throwException(current_exception());
        RETURN_NULL();
    }
}

ZEND_BEGIN_ARG_INFO_EX(Ice_ObjectPrx_ice_endpointSelection_arginfo, 1, ZEND_RETURN_VALUE, static_cast<zend_ulong>(1))
ZEND_ARG_INFO(0, selectionType)
ZEND_END_ARG_INFO()

ZEND_METHOD(Ice_ObjectPrx, ice_endpointSelection)
{
    ProxyPtr _this = Wrapper<ProxyPtr>::value(getThis());
    assert(_this);

    zend_long l;
    if (zend_parse_parameters(ZEND_NUM_ARGS(), const_cast<char*>("l"), &l) != SUCCESS)
    {
        RETURN_NULL();
    }

    if (l < 0 || l > 1)
    {
        runtimeError("expecting Random or Ordered");
        RETURN_NULL();
    }

    try
    {
        auto type = l == 0 ? Ice::EndpointSelectionType::Random : Ice::EndpointSelectionType::Ordered;
        if (!_this->clone(return_value, _this->proxy->ice_endpointSelection(type)))
        {
            RETURN_NULL();
        }
    }
    catch (...)
    {
        throwException(current_exception());
        RETURN_NULL();
    }
}

ZEND_METHOD(Ice_ObjectPrx, ice_isSecure)
{
    if (ZEND_NUM_ARGS() != 0)
    {
        WRONG_PARAM_COUNT;
    }

    ProxyPtr _this = Wrapper<ProxyPtr>::value(getThis());
    assert(_this);

    try
    {
        bool b = _this->proxy->ice_isSecure();
        RETURN_BOOL(b ? 1 : 0);
    }
    catch (...)
    {
        throwException(current_exception());
        RETURN_FALSE;
    }
}

ZEND_BEGIN_ARG_INFO_EX(Ice_ObjectPrx_ice_secure_arginfo, 1, ZEND_RETURN_VALUE, static_cast<zend_ulong>(1))
ZEND_ARG_INFO(0, secure)
ZEND_END_ARG_INFO()

ZEND_METHOD(Ice_ObjectPrx, ice_secure)
{
    ProxyPtr _this = Wrapper<ProxyPtr>::value(getThis());
    assert(_this);

    zend_bool b;
    if (zend_parse_parameters(ZEND_NUM_ARGS(), const_cast<char*>("b"), &b) != SUCCESS)
    {
        RETURN_NULL();
    }

    try
    {
        if (!_this->clone(return_value, _this->proxy->ice_secure(b ? true : false)))
        {
            RETURN_NULL();
        }
    }
    catch (...)
    {
        throwException(current_exception());
        RETURN_NULL();
    }
}

ZEND_METHOD(Ice_ObjectPrx, ice_getEncodingVersion)
{
    if (ZEND_NUM_ARGS() != 0)
    {
        WRONG_PARAM_COUNT;
    }

    ProxyPtr _this = Wrapper<ProxyPtr>::value(getThis());
    assert(_this);

    try
    {
        if (!createEncodingVersion(return_value, _this->proxy->ice_getEncodingVersion()))
        {
            RETURN_NULL();
        }
    }
    catch (...)
    {
        throwException(current_exception());
        RETURN_NULL();
    }
}

ZEND_BEGIN_ARG_INFO_EX(Ice_ObjectPrx_ice_encodingVersion_arginfo, 1, ZEND_RETURN_VALUE, static_cast<zend_ulong>(1))
ZEND_ARG_INFO(0, version)
ZEND_END_ARG_INFO()

ZEND_METHOD(Ice_ObjectPrx, ice_encodingVersion)
{
    ProxyPtr _this = Wrapper<ProxyPtr>::value(getThis());
    assert(_this);

    zend_class_entry* cls = idToClass("::Ice::EncodingVersion");
    assert(cls);

    zval* zv;
    if (zend_parse_parameters(ZEND_NUM_ARGS(), const_cast<char*>("O"), &zv, cls) == FAILURE)
    {
        RETURN_NULL();
    }

    Ice::EncodingVersion v;
    if (extractEncodingVersion(zv, v))
    {
        try
        {
            if (!_this->clone(return_value, _this->proxy->ice_encodingVersion(v)))
            {
                RETURN_NULL();
            }
        }
        catch (...)
        {
            throwException(current_exception());
            RETURN_NULL();
        }
    }
}

ZEND_METHOD(Ice_ObjectPrx, ice_isPreferSecure)
{
    if (ZEND_NUM_ARGS() != 0)
    {
        WRONG_PARAM_COUNT;
    }

    ProxyPtr _this = Wrapper<ProxyPtr>::value(getThis());
    assert(_this);

    try
    {
        bool b = _this->proxy->ice_isPreferSecure();
        RETURN_BOOL(b ? 1 : 0);
    }
    catch (...)
    {
        throwException(current_exception());
        RETURN_FALSE;
    }
}

ZEND_BEGIN_ARG_INFO_EX(Ice_ObjectPrx_ice_preferSecure_arginfo, 1, ZEND_RETURN_VALUE, static_cast<zend_ulong>(1))
ZEND_ARG_INFO(0, preferSecure)
ZEND_END_ARG_INFO()

ZEND_METHOD(Ice_ObjectPrx, ice_preferSecure)
{
    ProxyPtr _this = Wrapper<ProxyPtr>::value(getThis());
    assert(_this);

    zend_bool b;
    if (zend_parse_parameters(ZEND_NUM_ARGS(), const_cast<char*>("b"), &b) != SUCCESS)
    {
        RETURN_NULL();
    }

    try
    {
        if (!_this->clone(return_value, _this->proxy->ice_preferSecure(b ? true : false)))
        {
            RETURN_NULL();
        }
    }
    catch (...)
    {
        throwException(current_exception());
        RETURN_NULL();
    }
}

ZEND_METHOD(Ice_ObjectPrx, ice_getRouter)
{
    if (ZEND_NUM_ARGS() != 0)
    {
        WRONG_PARAM_COUNT;
    }

    ProxyPtr _this = Wrapper<ProxyPtr>::value(getThis());
    assert(_this);

    try
    {
        optional<Ice::RouterPrx> router = _this->proxy->ice_getRouter();
        if (router)
        {
            ProxyInfoPtr info = getProxyInfo("::Ice::Router");
            if (!info)
            {
                RETURN_NULL();
            }

            assert(info);

            if (!createProxy(return_value, std::move(router.value()), std::move(info), _this->communicator))
            {
                RETURN_NULL();
            }
        }
        else
        {
            RETURN_NULL();
        }
    }
    catch (...)
    {
        throwException(current_exception());
        RETURN_FALSE;
    }
}

ZEND_BEGIN_ARG_INFO_EX(Ice_ObjectPrx_ice_router_arginfo, 1, ZEND_RETURN_VALUE, static_cast<zend_ulong>(1))
ZEND_ARG_INFO(0, router)
ZEND_END_ARG_INFO()

ZEND_METHOD(Ice_ObjectPrx, ice_router)
{
    ProxyPtr _this = Wrapper<ProxyPtr>::value(getThis());
    assert(_this);

    zval* zprx;
    if (zend_parse_parameters(ZEND_NUM_ARGS(), const_cast<char*>("O!"), &zprx, proxyClassEntry) != SUCCESS)
    {
        RETURN_NULL();
    }

    optional<Ice::ObjectPrx> proxy;
    ProxyInfoPtr def;
    if (zprx && !fetchProxy(zprx, proxy, def))
    {
        RETURN_NULL();
    }

    optional<Ice::RouterPrx> router;
    if (proxy)
    {
        if (!def || !def->isA("::Ice::Router"))
        {
            runtimeError("ice_router requires a proxy narrowed to Ice::Router");
            RETURN_NULL();
        }
        router = Ice::uncheckedCast<Ice::RouterPrx>(proxy);
    }

    try
    {
        if (!_this->clone(return_value, _this->proxy->ice_router(router)))
        {
            RETURN_NULL();
        }
    }
    catch (...)
    {
        throwException(current_exception());
        RETURN_NULL();
    }
}

ZEND_METHOD(Ice_ObjectPrx, ice_getLocator)
{
    if (ZEND_NUM_ARGS() != 0)
    {
        WRONG_PARAM_COUNT;
    }

    ProxyPtr _this = Wrapper<ProxyPtr>::value(getThis());
    assert(_this);

    try
    {
        optional<Ice::LocatorPrx> locator = _this->proxy->ice_getLocator();
        if (locator)
        {
            ProxyInfoPtr info = getProxyInfo("::Ice::Locator");
            if (!info)
            {
                RETURN_NULL();
            }

            if (!createProxy(return_value, std::move(locator).value(), std::move(info), _this->communicator))
            {
                RETURN_NULL();
            }
        }
        else
        {
            RETURN_NULL();
        }
    }
    catch (...)
    {
        throwException(current_exception());
        RETURN_FALSE;
    }
}

ZEND_BEGIN_ARG_INFO_EX(Ice_ObjectPrx_ice_locator_arginfo, 1, ZEND_RETURN_VALUE, static_cast<zend_ulong>(1))
ZEND_ARG_INFO(0, locator)
ZEND_END_ARG_INFO()

ZEND_METHOD(Ice_ObjectPrx, ice_locator)
{
    ProxyPtr _this = Wrapper<ProxyPtr>::value(getThis());
    assert(_this);

    zval* zprx;
    if (zend_parse_parameters(ZEND_NUM_ARGS(), const_cast<char*>("O!"), &zprx, proxyClassEntry) != SUCCESS)
    {
        RETURN_NULL();
    }

    optional<Ice::ObjectPrx> proxy;
    ProxyInfoPtr def;
    if (zprx && !fetchProxy(zprx, proxy, def))
    {
        RETURN_NULL();
    }

    optional<Ice::LocatorPrx> locator;
    if (proxy)
    {
        if (!def || !def->isA("::Ice::Locator"))
        {
            runtimeError("ice_locator requires a proxy narrowed to Ice::Locator");
            RETURN_NULL();
        }
        locator = Ice::uncheckedCast<Ice::LocatorPrx>(proxy);
    }

    try
    {
        if (!_this->clone(return_value, _this->proxy->ice_locator(std::move(locator))))
        {
            RETURN_NULL();
        }
    }
    catch (...)
    {
        throwException(current_exception());
        RETURN_NULL();
    }
}

ZEND_METHOD(Ice_ObjectPrx, ice_twoway)
{
    if (ZEND_NUM_ARGS() != 0)
    {
        WRONG_PARAM_COUNT;
    }

    ProxyPtr _this = Wrapper<ProxyPtr>::value(getThis());
    assert(_this);

    try
    {
        if (!_this->clone(return_value, _this->proxy->ice_twoway()))
        {
            RETURN_NULL();
        }
    }
    catch (...)
    {
        throwException(current_exception());
        RETURN_NULL();
    }
}

ZEND_METHOD(Ice_ObjectPrx, ice_isTwoway)
{
    if (ZEND_NUM_ARGS() != 0)
    {
        WRONG_PARAM_COUNT;
    }

    ProxyPtr _this = Wrapper<ProxyPtr>::value(getThis());
    assert(_this);

    try
    {
        bool b = _this->proxy->ice_isTwoway();
        RETURN_BOOL(b ? 1 : 0);
    }
    catch (...)
    {
        throwException(current_exception());
        RETURN_FALSE;
    }
}

ZEND_METHOD(Ice_ObjectPrx, ice_oneway)
{
    if (ZEND_NUM_ARGS() != 0)
    {
        WRONG_PARAM_COUNT;
    }

    ProxyPtr _this = Wrapper<ProxyPtr>::value(getThis());
    assert(_this);

    try
    {
        if (!_this->clone(return_value, _this->proxy->ice_oneway()))
        {
            RETURN_NULL();
        }
    }
    catch (...)
    {
        throwException(current_exception());
        RETURN_NULL();
    }
}

ZEND_METHOD(Ice_ObjectPrx, ice_isOneway)
{
    if (ZEND_NUM_ARGS() != 0)
    {
        WRONG_PARAM_COUNT;
    }

    ProxyPtr _this = Wrapper<ProxyPtr>::value(getThis());
    assert(_this);

    try
    {
        bool b = _this->proxy->ice_isOneway();
        RETURN_BOOL(b ? 1 : 0);
    }
    catch (...)
    {
        throwException(current_exception());
        RETURN_FALSE;
    }
}

ZEND_METHOD(Ice_ObjectPrx, ice_batchOneway)
{
    if (ZEND_NUM_ARGS() != 0)
    {
        WRONG_PARAM_COUNT;
    }

    ProxyPtr _this = Wrapper<ProxyPtr>::value(getThis());
    assert(_this);

    try
    {
        if (!_this->clone(return_value, _this->proxy->ice_batchOneway()))
        {
            RETURN_NULL();
        }
    }
    catch (...)
    {
        throwException(current_exception());
        RETURN_NULL();
    }
}

ZEND_METHOD(Ice_ObjectPrx, ice_isBatchOneway)
{
    if (ZEND_NUM_ARGS() != 0)
    {
        WRONG_PARAM_COUNT;
    }

    ProxyPtr _this = Wrapper<ProxyPtr>::value(getThis());
    assert(_this);

    try
    {
        bool b = _this->proxy->ice_isBatchOneway();
        RETURN_BOOL(b ? 1 : 0);
    }
    catch (...)
    {
        throwException(current_exception());
        RETURN_FALSE;
    }
}

ZEND_METHOD(Ice_ObjectPrx, ice_datagram)
{
    if (ZEND_NUM_ARGS() != 0)
    {
        WRONG_PARAM_COUNT;
    }

    ProxyPtr _this = Wrapper<ProxyPtr>::value(getThis());
    assert(_this);

    try
    {
        if (!_this->clone(return_value, _this->proxy->ice_datagram()))
        {
            RETURN_NULL();
        }
    }
    catch (...)
    {
        throwException(current_exception());
        RETURN_NULL();
    }
}

ZEND_METHOD(Ice_ObjectPrx, ice_isDatagram)
{
    if (ZEND_NUM_ARGS() != 0)
    {
        WRONG_PARAM_COUNT;
    }

    ProxyPtr _this = Wrapper<ProxyPtr>::value(getThis());
    assert(_this);

    try
    {
        bool b = _this->proxy->ice_isDatagram();
        RETURN_BOOL(b ? 1 : 0);
    }
    catch (...)
    {
        throwException(current_exception());
        RETURN_FALSE;
    }
}

ZEND_METHOD(Ice_ObjectPrx, ice_batchDatagram)
{
    if (ZEND_NUM_ARGS() != 0)
    {
        WRONG_PARAM_COUNT;
    }

    ProxyPtr _this = Wrapper<ProxyPtr>::value(getThis());
    assert(_this);

    try
    {
        if (!_this->clone(return_value, _this->proxy->ice_batchDatagram()))
        {
            RETURN_NULL();
        }
    }
    catch (...)
    {
        throwException(current_exception());
        RETURN_NULL();
    }
}

ZEND_METHOD(Ice_ObjectPrx, ice_isBatchDatagram)
{
    if (ZEND_NUM_ARGS() != 0)
    {
        WRONG_PARAM_COUNT;
    }

    ProxyPtr _this = Wrapper<ProxyPtr>::value(getThis());
    assert(_this);

    try
    {
        bool b = _this->proxy->ice_isBatchDatagram();
        RETURN_BOOL(b ? 1 : 0);
    }
    catch (...)
    {
        throwException(current_exception());
        RETURN_FALSE;
    }
}

ZEND_BEGIN_ARG_INFO_EX(Ice_ObjectPrx_ice_compress_arginfo, 1, ZEND_RETURN_VALUE, static_cast<zend_ulong>(1))
ZEND_ARG_INFO(0, compress)
ZEND_END_ARG_INFO()

ZEND_METHOD(Ice_ObjectPrx, ice_compress)
{
    ProxyPtr _this = Wrapper<ProxyPtr>::value(getThis());
    assert(_this);

    zend_bool b;
    if (zend_parse_parameters(ZEND_NUM_ARGS(), const_cast<char*>("b"), &b) != SUCCESS)
    {
        RETURN_NULL();
    }

    try
    {
        if (!_this->clone(return_value, _this->proxy->ice_compress(b ? true : false)))
        {
            RETURN_NULL();
        }
    }
    catch (...)
    {
        throwException(current_exception());
        RETURN_NULL();
    }
}

ZEND_METHOD(Ice_ObjectPrx, ice_getCompress)
{
    if (ZEND_NUM_ARGS() != 0)
    {
        WRONG_PARAM_COUNT;
    }

    ProxyPtr _this = Wrapper<ProxyPtr>::value(getThis());
    assert(_this);

    try
    {
        optional<bool> compress = _this->proxy->ice_getCompress();
        if (compress)
        {
            RETURN_BOOL(*compress ? 1 : 0);
        }
        else
        {
            assignUnset(return_value);
        }
    }
    catch (...)
    {
        throwException(current_exception());
        RETURN_NULL();
    }
}

ZEND_BEGIN_ARG_INFO_EX(Ice_ObjectPrx_ice_invocationTimeout_arginfo, 1, ZEND_RETURN_VALUE, static_cast<zend_ulong>(1))
ZEND_ARG_INFO(0, invocationTiemout)
ZEND_END_ARG_INFO()

ZEND_METHOD(Ice_ObjectPrx, ice_invocationTimeout)
{
    ProxyPtr _this = Wrapper<ProxyPtr>::value(getThis());
    assert(_this);

    try
    {
        zend_long l;
        if (zend_parse_parameters(ZEND_NUM_ARGS(), const_cast<char*>("l"), &l) != SUCCESS)
        {
            RETURN_NULL();
        }
        // TODO: range check?
        if (!_this->clone(return_value, _this->proxy->ice_invocationTimeout(static_cast<int32_t>(l))))
        {
            RETURN_NULL();
        }
    }
    catch (...)
    {
        throwException(current_exception());
        RETURN_NULL();
    }
}

ZEND_METHOD(Ice_ObjectPrx, ice_getInvocationTimeout)
{
    if (ZEND_NUM_ARGS() != 0)
    {
        WRONG_PARAM_COUNT;
    }

    ProxyPtr _this = Wrapper<ProxyPtr>::value(getThis());
    assert(_this);

    try
    {
        chrono::milliseconds timeout = _this->proxy->ice_getInvocationTimeout();
        ZVAL_LONG(return_value, static_cast<long>(timeout.count()));
    }
    catch (...)
    {
        throwException(current_exception());
        RETURN_NULL();
    }
}

ZEND_BEGIN_ARG_INFO_EX(Ice_ObjectPrx_ice_connectionId_arginfo, 1, ZEND_RETURN_VALUE, static_cast<zend_ulong>(1))
ZEND_ARG_INFO(0, connectionId)
ZEND_END_ARG_INFO()

ZEND_METHOD(Ice_ObjectPrx, ice_connectionId)
{
    ProxyPtr _this = Wrapper<ProxyPtr>::value(getThis());
    assert(_this);

    try
    {
        char* id;
        size_t idLen;
        if (zend_parse_parameters(ZEND_NUM_ARGS(), const_cast<char*>("s"), &id, &idLen) != SUCCESS)
        {
            RETURN_NULL();
        }
        if (!_this->clone(return_value, _this->proxy->ice_connectionId(id)))
        {
            RETURN_NULL();
        }
    }
    catch (...)
    {
        throwException(current_exception());
        RETURN_NULL();
    }
}

ZEND_BEGIN_ARG_INFO_EX(Ice_ObjectPrx_ice_fixed_arginfo, 1, ZEND_RETURN_VALUE, static_cast<zend_ulong>(1))
ZEND_ARG_INFO(0, connection)
ZEND_END_ARG_INFO()

ZEND_METHOD(Ice_ObjectPrx, ice_fixed)
{
    ProxyPtr _this = Wrapper<ProxyPtr>::value(getThis());
    assert(_this);

    zval* zcon;
    if (zend_parse_parameters(ZEND_NUM_ARGS(), const_cast<char*>("O!"), &zcon, connectionClassEntry) != SUCCESS)
    {
        RETURN_NULL();
    }

    Ice::ConnectionPtr connection;
    if (zcon && !fetchConnection(zcon, connection))
    {
        RETURN_NULL();
    }

    try
    {
        if (!_this->clone(return_value, _this->proxy->ice_fixed(connection)))
        {
            RETURN_NULL();
        }
    }
    catch (...)
    {
        throwException(current_exception());
        RETURN_NULL();
    }
}

ZEND_METHOD(Ice_ObjectPrx, ice_isFixed)
{
    if (ZEND_NUM_ARGS() != 0)
    {
        WRONG_PARAM_COUNT;
    }

    ProxyPtr _this = Wrapper<ProxyPtr>::value(getThis());
    assert(_this);

    try
    {
        bool b = _this->proxy->ice_isFixed();
        RETURN_BOOL(b ? 1 : 0);
    }
    catch (...)
    {
        throwException(current_exception());
        RETURN_FALSE;
    }
}

ZEND_METHOD(Ice_ObjectPrx, ice_getConnection)
{
    if (ZEND_NUM_ARGS() != 0)
    {
        WRONG_PARAM_COUNT;
    }

    ProxyPtr _this = Wrapper<ProxyPtr>::value(getThis());
    assert(_this);

    try
    {
        Ice::ConnectionPtr con = _this->proxy->ice_getConnection();
        if (!createConnection(return_value, con))
        {
            RETURN_NULL();
        }
    }
    catch (...)
    {
        throwException(current_exception());
        RETURN_NULL();
    }
}

ZEND_METHOD(Ice_ObjectPrx, ice_getCachedConnection)
{
    if (ZEND_NUM_ARGS() != 0)
    {
        WRONG_PARAM_COUNT;
    }

    ProxyPtr _this = Wrapper<ProxyPtr>::value(getThis());
    assert(_this);

    try
    {
        Ice::ConnectionPtr con = _this->proxy->ice_getCachedConnection();
        if (!con || !createConnection(return_value, con))
        {
            RETURN_NULL();
        }
    }
    catch (...)
    {
        throwException(current_exception());
        RETURN_NULL();
    }
}

ZEND_METHOD(Ice_ObjectPrx, ice_flushBatchRequests)
{
    if (ZEND_NUM_ARGS() != 0)
    {
        WRONG_PARAM_COUNT;
    }

    ProxyPtr _this = Wrapper<ProxyPtr>::value(getThis());
    assert(_this);

    try
    {
        _this->proxy->ice_flushBatchRequests();
    }
    catch (...)
    {
        throwException(current_exception());
        RETURN_NULL();
    }
}

ZEND_BEGIN_ARG_INFO_EX(Ice_Proxy_do_cast_arginfo, 1, ZEND_RETURN_VALUE, static_cast<zend_ulong>(1))
ZEND_ARG_INFO(0, id)
ZEND_ARG_INFO(0, facet)
ZEND_ARG_INFO(0, ctx)
ZEND_END_ARG_INFO()

static void
do_cast(INTERNAL_FUNCTION_PARAMETERS, bool check)
{
    // First argument is required and should be a scoped name. The second and third arguments are optional and
    // represent a facet name, a context, or a facet name followed by a context.
    if (ZEND_NUM_ARGS() < 1 || ZEND_NUM_ARGS() > 3)
    {
        WRONG_PARAM_COUNT;
    }

    char* id;
    size_t idLen;
    char* facet = 0;
    size_t facetLen;
    zval* arr = 0;

    if (zend_parse_parameters_ex(
            ZEND_PARSE_PARAMS_QUIET,
            ZEND_NUM_ARGS(),
            const_cast<char*>("s|s!a!"),
            &id,
            &idLen,
            &facet,
            &facetLen,
            &arr) == FAILURE)
    {
        facet = 0;
        if (zend_parse_parameters_ex(
                ZEND_PARSE_PARAMS_QUIET,
                ZEND_NUM_ARGS(),
                const_cast<char*>("s|a!"),
                &id,
                &idLen,
                &arr) == FAILURE)
        {
            php_error(
                E_ERROR,
                "%s() requires a type id followed by an optional facet and/or context",
                get_active_function_name());
            return;
        }
    }

    ProxyPtr _this = Wrapper<ProxyPtr>::value(getThis());
    assert(_this);

    // Populate the context.
    Ice::Context ctx;
    if (arr && !extractContext(arr, ctx))
    {
        RETURN_NULL();
    }

    try
    {
        ProxyInfoPtr info = getProxyInfo(id);
        if (!info)
        {
            RETURN_NULL();
        }

        Ice::ObjectPrx prx = _this->proxy;
        if (facet)
        {
            prx = prx->ice_facet(facet);
        }

        if (arr)
        {
            prx = prx->ice_context(ctx);
        }

        if (check)
        {
            // Verify that the object supports the requested type.
            if (!prx->ice_isA(info->id))
            {
                RETURN_NULL();
            }
        }

        if (!createProxy(return_value, std::move(prx), std::move(info), _this->communicator))
        {
            RETURN_NULL();
        }
    }
    catch (...)
    {
        throwException(current_exception());
        RETVAL_FALSE;
    }
}

ZEND_METHOD(Ice_ObjectPrx, ice_uncheckedCast) { do_cast(INTERNAL_FUNCTION_PARAM_PASSTHRU, false); }

ZEND_METHOD(Ice_ObjectPrx, ice_checkedCast) { do_cast(INTERNAL_FUNCTION_PARAM_PASSTHRU, true); }

IcePHP::Proxy::Proxy(Ice::ObjectPrx p, ProxyInfoPtr i, CommunicatorInfoPtr comm)
    : proxy(std::move(p)),
      info(std::move(i)),
      communicator(std::move(comm)),
      connection(0),
      cachedConnection(0)
{
}

IcePHP::Proxy::~Proxy()
{
    if (connection)
    {
        zval_ptr_dtor(connection);
    }
    if (cachedConnection)
    {
        zval_ptr_dtor(cachedConnection);
    }
}

bool
IcePHP::Proxy::clone(zval* zv, Ice::ObjectPrx p)
{
    return create(zv, std::move(p), info, communicator);
}

bool
IcePHP::Proxy::cloneUntyped(zval* zv, Ice::ObjectPrx p)
{
    return create(zv, std::move(p), nullptr, communicator);
}

bool
IcePHP::Proxy::create(zval* zv, Ice::ObjectPrx p, ProxyInfoPtr info, CommunicatorInfoPtr comm)
{
    ProxyInfoPtr prxInfo = std::move(info);
    if (!prxInfo)
    {
        prxInfo = getProxyInfo("::Ice::Object");
        assert(prxInfo);
    }

    if (object_init_ex(zv, proxyClassEntry) != SUCCESS)
    {
        runtimeError("unable to initialize proxy");
        return false;
    }

    Wrapper<ProxyPtr>* obj = Wrapper<ProxyPtr>::extract(zv);
    assert(!obj->ptr);
    obj->ptr = new ProxyPtr(new Proxy(std::move(p), std::move(prxInfo), std::move(comm)));
    return true;
}

static zend_object*
handleAlloc(zend_class_entry* ce)
{
    Wrapper<ProxyPtr>* obj = Wrapper<ProxyPtr>::create(ce);
    assert(obj);
    obj->zobj.handlers = &_handlers;
    return &obj->zobj;
}

static void
handleFreeStorage(zend_object* object)
{
    Wrapper<ProxyPtr>* obj = Wrapper<ProxyPtr>::fetch(object);
    delete obj->ptr;
    zend_object_std_dtor(object);
}

static zend_object*
handleClone(zend_object* zobj)
{
    // Create a new object that shares a C++ proxy instance with this object.
    ProxyPtr obj = *Wrapper<ProxyPtr>::fetch(zobj)->ptr;
    assert(obj);
    zval clone;
    if (!obj->clone(&clone, obj->proxy))
    {
        return 0;
    }
    return Z_OBJ(clone);
}

static union _zend_function*
handleGetMethod(zend_object** object, zend_string* name, const zval* key)
{
    zend_function* result;
    // First delegate to the standard implementation of get_method. This will find any of our predefined proxy methods.
    // If it returns 0, then we return a function that will check the class definition.
    result = zend_get_std_object_handlers()->get_method(object, name, key);
    if (!result)
    {
        Wrapper<ProxyPtr>* obj = Wrapper<ProxyPtr>::fetch(*object);
        assert(obj->ptr);
        ProxyPtr _this = *obj->ptr;

        ProxyInfoPtr info = _this->info;
        assert(info);

        OperationPtr op = info->getOperation(name->val);
        if (!op)
        {
            // Returning 0 causes PHP to report an "undefined method" error.
            return 0;
        }

        result = op->function();
    }

    return result;
}

static int
handleCompare(zval* zobj1, zval* zobj2)
{
    // PHP guarantees that the objects have the same class.
    Wrapper<ProxyPtr>* obj1 = Wrapper<ProxyPtr>::extract(zobj1);
    assert(obj1->ptr);
    ProxyPtr _this1 = *obj1->ptr;
    Ice::ObjectPrx prx1 = _this1->proxy;

    Wrapper<ProxyPtr>* obj2 = Wrapper<ProxyPtr>::extract(zobj2);
    assert(obj2->ptr);
    ProxyPtr _this2 = *obj2->ptr;
    Ice::ObjectPrx prx2 = _this2->proxy;

    if (prx1 == prx2)
    {
        return 0;
    }
    else if (prx1 < prx2)
    {
        return -1;
    }
    else
    {
        return 1;
    }
}

// Predefined methods for ObjectPrx.
static zend_function_entry _proxyMethods[] = {
    // _construct
    ZEND_ME(Ice_ObjectPrx, __construct, ice_void_arginfo, ZEND_ACC_PRIVATE | ZEND_ACC_CTOR)
    // __toString
    ZEND_ME(Ice_ObjectPrx, __toString, ice_to_string_arginfo, ZEND_ACC_PUBLIC)
    // ice_getCommunicator
    ZEND_ME(Ice_ObjectPrx, ice_getCommunicator, ice_void_arginfo, ZEND_ACC_PUBLIC)
    // ice_toString
    ZEND_ME(Ice_ObjectPrx, ice_toString, ice_void_arginfo, ZEND_ACC_PUBLIC)
    // ice_getIdentity
    ZEND_ME(Ice_ObjectPrx, ice_getIdentity, ice_void_arginfo, ZEND_ACC_PUBLIC)
    // ice_identity
    ZEND_ME(Ice_ObjectPrx, ice_identity, Ice_ObjectPrx_ice_identity_arginfo, ZEND_ACC_PUBLIC)
    // ice_getContext
    ZEND_ME(Ice_ObjectPrx, ice_getContext, ice_void_arginfo, ZEND_ACC_PUBLIC)
    // ice_context
    ZEND_ME(Ice_ObjectPrx, ice_context, Ice_ObjectPrx_ice_context_arginfo, ZEND_ACC_PUBLIC)
    // ice_getFacet
    ZEND_ME(Ice_ObjectPrx, ice_getFacet, ice_void_arginfo, ZEND_ACC_PUBLIC)
    // ice_facet
    ZEND_ME(Ice_ObjectPrx, ice_facet, Ice_ObjectPrx_ice_facet_arginfo, ZEND_ACC_PUBLIC)
    // ice_getAdapterId
    ZEND_ME(Ice_ObjectPrx, ice_getAdapterId, ice_void_arginfo, ZEND_ACC_PUBLIC)
    // ice_adapterId
    ZEND_ME(Ice_ObjectPrx, ice_adapterId, Ice_ObjectPrx_ice_adapterId_arginfo, ZEND_ACC_PUBLIC)
    // ice_getEndpoints
    ZEND_ME(Ice_ObjectPrx, ice_getEndpoints, ice_void_arginfo, ZEND_ACC_PUBLIC)
    // ice_endpoints
    ZEND_ME(Ice_ObjectPrx, ice_endpoints, Ice_ObjectPrx_ice_endpoints_arginfo, ZEND_ACC_PUBLIC)
    // ice_getLocatorCacheTimeout
    ZEND_ME(Ice_ObjectPrx, ice_getLocatorCacheTimeout, ice_void_arginfo, ZEND_ACC_PUBLIC)
    // ice_getConnectionId
    ZEND_ME(Ice_ObjectPrx, ice_getConnectionId, ice_void_arginfo, ZEND_ACC_PUBLIC)
    // ice_locatorCacheTimeout
    ZEND_ME(Ice_ObjectPrx, ice_locatorCacheTimeout, Ice_ObjectPrx_ice_locatorCacheTimeout_arginfo, ZEND_ACC_PUBLIC)
    // ice_isConnectionCached
    ZEND_ME(Ice_ObjectPrx, ice_isConnectionCached, ice_void_arginfo, ZEND_ACC_PUBLIC)
    // ice_connectionCached
    ZEND_ME(Ice_ObjectPrx, ice_connectionCached, Ice_ObjectPrx_ice_connectionCached_arginfo, ZEND_ACC_PUBLIC)
    // ice_getEndpointSelection
    ZEND_ME(Ice_ObjectPrx, ice_getEndpointSelection, ice_void_arginfo, ZEND_ACC_PUBLIC)
    // ice_endpointSelection
    ZEND_ME(Ice_ObjectPrx, ice_endpointSelection, Ice_ObjectPrx_ice_endpointSelection_arginfo, ZEND_ACC_PUBLIC)
    // ice_isSecure
    ZEND_ME(Ice_ObjectPrx, ice_isSecure, ice_void_arginfo, ZEND_ACC_PUBLIC)
    // ice_secure
    ZEND_ME(Ice_ObjectPrx, ice_secure, Ice_ObjectPrx_ice_secure_arginfo, ZEND_ACC_PUBLIC)
    // ice_getEncodingVersion
    ZEND_ME(Ice_ObjectPrx, ice_getEncodingVersion, ice_void_arginfo, ZEND_ACC_PUBLIC)
    // ice_encodingVersion
    ZEND_ME(Ice_ObjectPrx, ice_encodingVersion, Ice_ObjectPrx_ice_encodingVersion_arginfo, ZEND_ACC_PUBLIC)
    // ice_isPreferSecure
    ZEND_ME(Ice_ObjectPrx, ice_isPreferSecure, ice_void_arginfo, ZEND_ACC_PUBLIC)
    // ice_preferSecure
    ZEND_ME(Ice_ObjectPrx, ice_preferSecure, Ice_ObjectPrx_ice_preferSecure_arginfo, ZEND_ACC_PUBLIC)
    // ice_getRouter
    ZEND_ME(Ice_ObjectPrx, ice_getRouter, ice_void_arginfo, ZEND_ACC_PUBLIC)
    // ice_router
    ZEND_ME(Ice_ObjectPrx, ice_router, Ice_ObjectPrx_ice_router_arginfo, ZEND_ACC_PUBLIC)
    // ice_getLocator
    ZEND_ME(Ice_ObjectPrx, ice_getLocator, ice_void_arginfo, ZEND_ACC_PUBLIC)
    // ice_locator
    ZEND_ME(Ice_ObjectPrx, ice_locator, Ice_ObjectPrx_ice_locator_arginfo, ZEND_ACC_PUBLIC)
    // ice_twoway
    ZEND_ME(Ice_ObjectPrx, ice_twoway, ice_void_arginfo, ZEND_ACC_PUBLIC)
    // ice_isTwoway
    ZEND_ME(Ice_ObjectPrx, ice_isTwoway, ice_void_arginfo, ZEND_ACC_PUBLIC)
    // ice_oneway
    ZEND_ME(Ice_ObjectPrx, ice_oneway, ice_void_arginfo, ZEND_ACC_PUBLIC)
    // ice_isOneway
    ZEND_ME(Ice_ObjectPrx, ice_isOneway, ice_void_arginfo, ZEND_ACC_PUBLIC)
    // ice_batchOneway
    ZEND_ME(Ice_ObjectPrx, ice_batchOneway, ice_void_arginfo, ZEND_ACC_PUBLIC)
    // ice_isBatchOneway
    ZEND_ME(Ice_ObjectPrx, ice_isBatchOneway, ice_void_arginfo, ZEND_ACC_PUBLIC)
    // ice_datagram
    ZEND_ME(Ice_ObjectPrx, ice_datagram, ice_void_arginfo, ZEND_ACC_PUBLIC)
    // ice_isDatagram
    ZEND_ME(Ice_ObjectPrx, ice_isDatagram, ice_void_arginfo, ZEND_ACC_PUBLIC)
    // ice_batchDatagram
    ZEND_ME(Ice_ObjectPrx, ice_batchDatagram, ice_void_arginfo, ZEND_ACC_PUBLIC)
    // ice_isBatchDatagram
    ZEND_ME(Ice_ObjectPrx, ice_isBatchDatagram, ice_void_arginfo, ZEND_ACC_PUBLIC)
    // ice_compress
    ZEND_ME(Ice_ObjectPrx, ice_compress, Ice_ObjectPrx_ice_compress_arginfo, ZEND_ACC_PUBLIC)
    // ice_getCompress
    ZEND_ME(Ice_ObjectPrx, ice_getCompress, ice_void_arginfo, ZEND_ACC_PUBLIC)
    // ice_invocationTimeout
    ZEND_ME(Ice_ObjectPrx, ice_invocationTimeout, Ice_ObjectPrx_ice_invocationTimeout_arginfo, ZEND_ACC_PUBLIC)
    // ice_getInvocationTimeout
    ZEND_ME(Ice_ObjectPrx, ice_getInvocationTimeout, ice_void_arginfo, ZEND_ACC_PUBLIC)
    // ice_connectionId
    ZEND_ME(Ice_ObjectPrx, ice_connectionId, Ice_ObjectPrx_ice_connectionId_arginfo, ZEND_ACC_PUBLIC)
    // ice_fixed
    ZEND_ME(Ice_ObjectPrx, ice_fixed, Ice_ObjectPrx_ice_fixed_arginfo, ZEND_ACC_PUBLIC)
    // ice_isFixed
    ZEND_ME(Ice_ObjectPrx, ice_isFixed, ice_void_arginfo, ZEND_ACC_PUBLIC)
    // ice_getConnection
    ZEND_ME(Ice_ObjectPrx, ice_getConnection, ice_void_arginfo, ZEND_ACC_PUBLIC)
    // ice_getCachedConnection
    ZEND_ME(Ice_ObjectPrx, ice_getCachedConnection, ice_void_arginfo, ZEND_ACC_PUBLIC)
    // ice_flushBatchRequests
    ZEND_ME(Ice_ObjectPrx, ice_flushBatchRequests, ice_void_arginfo, ZEND_ACC_PUBLIC)
    // ice_uncheckedCast
    ZEND_ME(Ice_ObjectPrx, ice_uncheckedCast, Ice_Proxy_do_cast_arginfo, ZEND_ACC_PUBLIC)
    // ice_checkedCast
    ZEND_ME(Ice_ObjectPrx, ice_checkedCast, Ice_Proxy_do_cast_arginfo, ZEND_ACC_PUBLIC){0, 0, 0}};

bool
IcePHP::proxyInit(void)
{
    // Register the ObjectPrx class.
    zend_class_entry ce;
    INIT_NS_CLASS_ENTRY(ce, "Ice", "ObjectPrx", _proxyMethods);
    ce.create_object = handleAlloc;
    proxyClassEntry = zend_register_internal_class(&ce);
    // proxyClassEntry->ce_flags |= ZEND_ACC_EXPLICIT_ABSTRACT_CLASS;
    memcpy(&_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
    _handlers.clone_obj = handleClone;
    _handlers.get_method = handleGetMethod;
    _handlers.compare = handleCompare;
    _handlers.free_obj = handleFreeStorage;
    _handlers.offset = XtOffsetOf(Wrapper<ProxyPtr>, zobj);
    return true;
}

bool
IcePHP::createProxy(zval* zv, Ice::ObjectPrx proxy, CommunicatorInfoPtr communicatorInfo)
{
    return Proxy::create(zv, std::move(proxy), nullptr, std::move(communicatorInfo));
}

bool
IcePHP::createProxy(zval* zv, Ice::ObjectPrx proxy, ProxyInfoPtr proxyInfo, CommunicatorInfoPtr communicatorInfo)
{
    return Proxy::create(zv, std::move(proxy), std::move(proxyInfo), std::move(communicatorInfo));
}

bool
IcePHP::fetchProxy(zval* zv, optional<Ice::ObjectPrx>& prx, ProxyInfoPtr& info)
{
    CommunicatorInfoPtr comm;
    return fetchProxy(zv, prx, info, comm);
}

bool
IcePHP::fetchProxy(zval* zv, optional<Ice::ObjectPrx>& prx, ProxyInfoPtr& info, CommunicatorInfoPtr& comm)
{
    if (!ZVAL_IS_NULL(zv))
    {
        if (Z_TYPE_P(zv) != IS_OBJECT || Z_OBJCE_P(zv) != proxyClassEntry)
        {
            invalidArgument("value is not a proxy");
            return false;
        }
        Wrapper<ProxyPtr>* obj = Wrapper<ProxyPtr>::extract(zv);
        if (!obj)
        {
            runtimeError("unable to retrieve proxy object from object store");
            return false;
        }
        assert(obj->ptr);
        prx = (*obj->ptr)->proxy;
        info = (*obj->ptr)->info;
        comm = (*obj->ptr)->communicator;
    }
    return true;
}
