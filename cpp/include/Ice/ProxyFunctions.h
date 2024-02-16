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

ICE_API bool operator<(const ObjectPrx&, const ObjectPrx&);
ICE_API bool operator==(const ObjectPrx&, const ObjectPrx&);

inline bool
operator>(const ObjectPrx& lhs, const ObjectPrx& rhs)
{
    return rhs < lhs;
}

inline bool
operator<=(const ObjectPrx& lhs, const ObjectPrx& rhs)
{
    return !(lhs > rhs);
}

inline bool
operator>=(const ObjectPrx& lhs, const ObjectPrx& rhs)
{
    return !(lhs < rhs);
}

inline bool
operator!=(const ObjectPrx& lhs, const ObjectPrx& rhs)
{
    return !(lhs == rhs);
}

ICE_API ::std::ostream& operator<<(::std::ostream&, const ObjectPrx&);

template<typename Prx,
         typename std::enable_if<std::is_base_of<ObjectPrx, Prx>::value>::type* = nullptr>
inline ::std::ostream& operator<<(::std::ostream& os, const std::optional<Prx>& proxy)
{
    if (proxy)
    {
        os << proxy.value();
    }
    else
    {
        os << "";
    }
    return os;
}

/**
 * Compares the object identities of two proxies.
 * @param lhs A proxy.
 * @param rhs A proxy.
 * @return True if the identity in lhs compares less than the identity in rhs, false otherwise.
 */
ICE_API bool proxyIdentityLess(const ObjectPrx& lhs, const ObjectPrx& rhs);

/**
 * Compares the object identities of two proxies.
 * @param lhs A proxy.
 * @param rhs A proxy.
 * @return True if the identity in lhs compares equal to the identity in rhs, false otherwise.
 */
ICE_API bool proxyIdentityEqual(const ObjectPrx& lhs, const ObjectPrx& rhs);

/**
 * Compares the object identities and facets of two proxies.
 * @param lhs A proxy.
 * @param rhs A proxy.
 * @return True if the identity and facet in lhs compare less than the identity and facet
 * in rhs, false otherwise.
 */
ICE_API bool proxyIdentityAndFacetLess(const ObjectPrx& lhs, const ObjectPrx& rhs);

/**
 * Compares the object identities and facets of two proxies.
 * @param lhs A proxy.
 * @param rhs A proxy.
 * @return True if the identity and facet in lhs compare equal to the identity and facet
 * in rhs, false otherwise.
 */
ICE_API bool proxyIdentityAndFacetEqual(const ObjectPrx& lhs, const ObjectPrx& rhs);

/**
 * A functor that compares the object identities of two proxies. Evaluates true if the identity in lhs
 * compares less than the identity in rhs, false otherwise.
 * \headerfile Ice/Ice.h
 */

struct ProxyIdentityLess
{
    bool operator()(const ObjectPrx& lhs, const ObjectPrx& rhs) const
    {
        return proxyIdentityLess(lhs, rhs);
    }
};

/**
 * A functor that compares the object identities of two proxies. Evaluates true if the identity in lhs
 * compares equal to the identity in rhs, false otherwise.
 * \headerfile Ice/Ice.h
 */
struct ProxyIdentityEqual
{
    bool operator()(const ObjectPrx& lhs, const ObjectPrx& rhs) const
    {
        return proxyIdentityEqual(lhs, rhs);
    }
};

/**
 * A functor that compares the object identities and facets of two proxies. Evaluates true if the identity
 * and facet in lhs compare less than the identity and facet in rhs, false otherwise.
 * \headerfile Ice/Ice.h
 */
struct ProxyIdentityAndFacetLess
{
    bool operator()(const ObjectPrx& lhs, const ObjectPrx& rhs) const
    {
        return proxyIdentityAndFacetLess(lhs, rhs);
    }
};

/**
 * A functor that compares the object identities and facets of two proxies. Evaluates true if the identity
 * and facet in lhs compare equal to the identity and facet in rhs, false otherwise.
 * \headerfile Ice/Ice.h
 */
struct ProxyIdentityAndFacetEqual
{
    bool operator()(const ObjectPrx& lhs, const ObjectPrx& rhs) const
    {
        return proxyIdentityAndFacetEqual(lhs, rhs);
    }
};

}

#endif
