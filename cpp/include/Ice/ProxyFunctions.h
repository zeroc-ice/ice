//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_PROXY_FUNCTIONS_H
#define ICE_PROXY_FUNCTIONS_H

#include "Ice/Proxy.h"

namespace Ice
{

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
