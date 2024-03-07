//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_PROXY_FUNCTIONS_H
#define ICE_PROXY_FUNCTIONS_H

#include "Ice/Proxy.h"
#include "Ice/Current.h"
#include "Ice/Communicator.h"
#include "Ice/ObjectAdapter.h"

namespace Ice
{

/**
 * Verifies that a proxy received from the client is not null, and throws a MarshalException if it is.
 * @param prx The proxy to check.
 * @param current The Current object for the invocation.
 * @throw MarshalException If the proxy is null.
 * */
template<typename Prx, std::enable_if_t<std::is_base_of<ObjectPrx, Prx>::value, bool> = true>
void checkNotNull(std::optional<Prx> prx, const Current& current)
{
    if (!prx)
    {
        // Will be reported back to the client as an UnknownLocalException with an error message.
        std::ostringstream os;
        os << "null proxy passed to " << current.operation << " on object "
            << current.adapter->getCommunicator()->identityToString(current.id);
        throw MarshalException {__FILE__, __LINE__, os.str()};
    }
}

/**
 * Downcasts a proxy without confirming the target object's type via a remote invocation.
 * @param proxy The source proxy.
 * @return A proxy with the requested type.
 * @remark The preferred syntax is to construct the proxy from another proxy using the explicit ObjectPrx
 * constructor.
 */
template<typename Prx, std::enable_if_t<std::is_base_of<ObjectPrx, Prx>::value, bool> = true>
Prx uncheckedCast(const ObjectPrx& proxy) { return Prx(proxy); }

/**
 * Downcasts a proxy without confirming the target object's type via a remote invocation.
 * @param proxy The source proxy (can be nullopt).
 * @return A proxy with the requested type.
 * @remark The preferred syntax is to construct the proxy from another proxy using the explicit ObjectPrx
 * constructor.
 */
template<typename Prx, std::enable_if_t<std::is_base_of<ObjectPrx, Prx>::value, bool> = true>
std::optional<Prx> uncheckedCast(const std::optional<ObjectPrx>& proxy)
{
    return proxy ? std::make_optional<Prx>(proxy.value()) : std::nullopt;
}

/**
 * Downcasts a proxy without confirming the target object's type via a remote invocation.
 * @param proxy The source proxy.
 * @param facet A facet name.
 * @return A proxy with the requested type and facet.
 */
template<typename Prx, std::enable_if_t<std::is_base_of<ObjectPrx, Prx>::value, bool> = true>
Prx uncheckedCast(const ObjectPrx& proxy, const std::string& facet)
{
    return uncheckedCast<Prx>(proxy->ice_facet(facet));
}

/**
 * Downcasts a proxy without confirming the target object's type via a remote invocation.
 * @param proxy The source proxy.
 * @param facet A facet name.
 * @return A proxy with the requested type and facet.
 */
template<typename Prx, std::enable_if_t<std::is_base_of<ObjectPrx, Prx>::value, bool> = true>
std::optional<Prx> uncheckedCast(const std::optional<ObjectPrx>& proxy, const std::string& facet)
{
    return proxy ? std::make_optional<Prx>(proxy->ice_facet(facet)) : std::nullopt;
}

/**
 * Downcasts a proxy after confirming the target object's type via a remote invocation.
 * @param proxy The source proxy.
 * @param context The context map for the invocation.
 * @return A proxy with the requested type, or nullopt if the target object does not support the requested type.
 */
template<typename Prx, std::enable_if_t<std::is_base_of<ObjectPrx, Prx>::value, bool> = true>
std::optional<Prx> checkedCast(const ObjectPrx& proxy, const Context& context = noExplicitContext)
{
    return proxy->ice_isA(Prx::ice_staticId(), context) ? std::make_optional<Prx>(proxy) : std::nullopt;
}

/**
 * Downcasts a proxy after confirming the target object's type via a remote invocation.
 * @param proxy The source proxy (can be nullopt).
 * @param context The context map for the invocation.
 * @return A proxy with the requested type, or nullopt if the source proxy is nullopt or if the target object does not
 * support the requested type.
 */
template<typename Prx, std::enable_if_t<std::is_base_of<ObjectPrx, Prx>::value, bool> = true>
std::optional<Prx> checkedCast(const std::optional<ObjectPrx>& proxy, const Context& context = noExplicitContext)
{
    return proxy ? checkedCast<Prx>(proxy.value(), context) : std::nullopt;
}

/**
 * Downcasts a proxy after confirming the target object's type via a remote invocation.
 * @param proxy The source proxy (can be nullopt).
 * @param facet A facet name.
 * @param context The context map for the invocation.
 * @return A proxy with the requested type and facet, or nullopt if the target object does not have the requested facet
 * or if the facet is not of the requested type.
 */
template<typename Prx, std::enable_if_t<std::is_base_of<ObjectPrx, Prx>::value, bool> = true>
std::optional<Prx> checkedCast(
    const ObjectPrx& proxy,
    const std::string& facet,
    const Context& context = noExplicitContext)
{
    try
    {
        return checkedCast<Prx>(proxy->ice_facet(facet), context);
    }
    catch (const Ice::FacetNotExistException&)
    {
        return std::nullopt;
    }
}

/**
 * Downcasts a proxy after confirming the target object's type via a remote invocation.
 * @param proxy The source proxy (can be nullopt).
 * @param facet A facet name.
 * @param context The context map for the invocation.
 * @return A proxy with the requested type and facet, or nullopt if the source proxy is nullopt, if the target object
 * does not have the requested facet, or if the facet is not of the requested type.
 */
template<typename Prx, std::enable_if_t<std::is_base_of<ObjectPrx, Prx>::value, bool> = true>
std::optional<Prx> checkedCast(
    const std::optional<ObjectPrx>& proxy,
    const std::string& facet,
    const Context& context = noExplicitContext)
{
    try
    {
        return proxy ? checkedCast<Prx>(proxy->ice_facet(facet), context) : std::nullopt;
    }
    catch (const Ice::FacetNotExistException&)
    {
        return std::nullopt;
    }
}

ICE_API bool operator<(const ObjectPrx&, const ObjectPrx&);
ICE_API bool operator==(const ObjectPrx&, const ObjectPrx&);

inline bool operator>(const ObjectPrx& lhs, const ObjectPrx& rhs) { return rhs < lhs; }
inline bool operator<=(const ObjectPrx& lhs, const ObjectPrx& rhs) { return !(lhs > rhs); }
inline bool operator>=(const ObjectPrx& lhs, const ObjectPrx& rhs) { return !(lhs < rhs); }
inline bool operator!=(const ObjectPrx& lhs, const ObjectPrx& rhs) { return !(lhs == rhs); }

ICE_API ::std::ostream& operator<<(::std::ostream&, const ObjectPrx&);

template<typename Prx, std::enable_if_t<std::is_base_of<ObjectPrx, Prx>::value, bool> = true>
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
ICE_API bool proxyIdentityLess(const std::optional<ObjectPrx>& lhs, const std::optional<ObjectPrx>& rhs);

/**
 * Compares the object identities of two proxies.
 * @param lhs A proxy.
 * @param rhs A proxy.
 * @return True if the identity in lhs compares equal to the identity in rhs, false otherwise.
 */
ICE_API bool proxyIdentityEqual(const std::optional<ObjectPrx>& lhs, const std::optional<ObjectPrx>& rhs);

/**
 * Compares the object identities and facets of two proxies.
 * @param lhs A proxy.
 * @param rhs A proxy.
 * @return True if the identity and facet in lhs compare less than the identity and facet
 * in rhs, false otherwise.
 */
ICE_API bool proxyIdentityAndFacetLess(const std::optional<ObjectPrx>& lhs, const std::optional<ObjectPrx>& rhs);

/**
 * Compares the object identities and facets of two proxies.
 * @param lhs A proxy.
 * @param rhs A proxy.
 * @return True if the identity and facet in lhs compare equal to the identity and facet
 * in rhs, false otherwise.
 */
ICE_API bool proxyIdentityAndFacetEqual(const std::optional<ObjectPrx>& lhs, const std::optional<ObjectPrx>& rhs);

/**
 * A functor that compares the object identities of two proxies. Evaluates true if the identity in lhs
 * compares less than the identity in rhs, false otherwise.
 * \headerfile Ice/Ice.h
 */

struct ProxyIdentityLess
{
    bool operator()(const std::optional<ObjectPrx>& lhs, const std::optional<ObjectPrx>& rhs) const
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
    bool operator()(const std::optional<ObjectPrx>& lhs, const std::optional<ObjectPrx>& rhs) const
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
    bool operator()(const std::optional<ObjectPrx>& lhs, const std::optional<ObjectPrx>& rhs) const
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
    bool operator()(const std::optional<ObjectPrx>& lhs, const std::optional<ObjectPrx>& rhs) const
    {
        return proxyIdentityAndFacetEqual(lhs, rhs);
    }
};

}

#endif
