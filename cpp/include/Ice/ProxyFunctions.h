//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_PROXY_FUNCTIONS_H
#define ICE_PROXY_FUNCTIONS_H

#include "Ice/Locator.h"
#include "Ice/Router.h"

namespace IceInternal
{

ICE_API ReferencePtr setLocator(const Ice::ObjectPrx& proxy, const std::optional<Ice::LocatorPrx>& locator);
ICE_API ReferencePtr setRouter(const Ice::ObjectPrx& proxy, const std::optional<Ice::RouterPrx>& router);

}

namespace Ice
{

/**
 * Obtains the locator configured on a proxy.
 * @param proxy The proxy.
 * @return The locator for the proxy. If no locator is configured, the return value is nullopt.
*/
ICE_API std::optional<LocatorPrx> ice_getLocator(const ObjectPrx& proxy);

/**
 * Obtains the router configured on a proxy.
 * @param proxy The proxy.
 * @return The router for the proxy. If no router is configured, the return value is nullopt.
*/
ICE_API std::optional<RouterPrx> ice_getRouter(const ObjectPrx& proxy);

/**
 * Obtains a proxy that is identical to the source proxy, except for the locator.
 * @param proxy The source proxy.
 * @param locator The locator for the new proxy.
 * @return A proxy with the specified locator.
*/
template<typename Prx,
         typename std::enable_if<std::is_base_of<ObjectPrx, Prx>::value>::type* = nullptr>
Prx ice_locator(const Prx& proxy, const std::optional<LocatorPrx>& locator)
{
    auto ref = IceInternal::setLocator(proxy, locator);
    return ref == proxy._getReference() ? proxy : Prx(ref);
}

/**
 * Obtains a proxy that is identical to the source proxy, except for the router.
 * @param proxy The source proxy.
 * @param router The router for the new proxy.
 * @return A proxy with the specified router.
*/
template<typename Prx,
         typename std::enable_if<std::is_base_of<ObjectPrx, Prx>::value>::type* = nullptr>
Prx ice_router(const Prx& proxy, const std::optional<RouterPrx>& router)
{
    auto ref = IceInternal::setRouter(proxy, router);
    return ref == proxy._getReference() ? proxy : Prx(ref);
}

/**
 * Downcasts a proxy without confirming the target object's type via a remote invocation.
 * @param proxy The source proxy.
 * @return A proxy with the requested type.
 * @remark The preferred syntax is to construct the proxy from another proxy using the explicit ObjectPrx
 * constructor.
 */
template<typename Prx,
         typename std::enable_if<std::is_base_of<ObjectPrx, Prx>::value>::type* = nullptr>
Prx uncheckedCast(const ObjectPrx& proxy) { return Prx(proxy); }

/**
 * Downcasts a proxy without confirming the target object's type via a remote invocation.
 * @param proxy The source proxy (can be nullopt).
 * @return A proxy with the requested type.
 * @remark The preferred syntax is to construct the proxy from another proxy using the explicit ObjectPrx
 * constructor.
 */
template<typename Prx,
         typename std::enable_if<std::is_base_of<ObjectPrx, Prx>::value>::type* = nullptr>
std::optional<Prx> uncheckedCast(const std::optional<ObjectPrx>& proxy)
{
    return proxy ? std::make_optional<Prx>(proxy.value()) : std::nullopt;
}

/**
 * Downcasts a proxy after confirming the target object's type via a remote invocation.
 * @param proxy The source proxy.
 * @param context The context map for the invocation.
 * @return A proxy with the requested type, or nullopt if the target object does not support the requested type.
 */
template<typename Prx,
         typename std::enable_if<std::is_base_of<ObjectPrx, Prx>::value>::type* = nullptr>
std::optional<Prx> checkedCast(const ObjectPrx& proxy, const Context& context = noExplicitContext)
{
    if (proxy->ice_isA(Prx::ice_staticId(), context))
    {
        return Prx(proxy);
    }
    else
    {
        return std::nullopt;
    }
}

/**
 * Downcasts a proxy after confirming the target object's type via a remote invocation.
 * @param proxy The source proxy (can be nullopt).
 * @param context The context map for the invocation.
 * @return A proxy with the requested type, or nullopt if the target object does not support the requested type.
 */
template<typename Prx,
         typename std::enable_if<std::is_base_of<ObjectPrx, Prx>::value>::type* = nullptr>
std::optional<Prx> checkedCast(const std::optional<ObjectPrx>& proxy, const Context& context = noExplicitContext)
{
    return (proxy && proxy->ice_isA(Prx::ice_staticId(), context)) ?
        std::make_optional<Prx>(proxy.value()) :
        std::nullopt;
}

}

#endif
