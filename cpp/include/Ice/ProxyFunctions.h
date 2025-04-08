// Copyright (c) ZeroC, Inc.

#ifndef ICE_PROXY_FUNCTIONS_H
#define ICE_PROXY_FUNCTIONS_H

#include "Ice/Proxy.h"

namespace IceInternal
{
    ICE_API void throwNullProxyMarshalException(const char* file, int line, const Ice::Current& current);
}

namespace Ice
{
    struct Current;

    /// Verifies that a proxy received from the client is not null, and throws a MarshalException if it is.
    /// @param prx The proxy to check.
    /// @param file The source file name.
    /// @param line The source line number.
    /// @param current The Current object of the incoming request.
    /// @throw MarshalException If the proxy is null.
    template<typename Prx, std::enable_if_t<std::is_base_of_v<ObjectPrx, Prx>, bool> = true>
    void checkNotNull(const std::optional<Prx>& prx, const char* file, int line, const Current& current)
    {
        if (!prx)
        {
            // Will be reported back to the client as an UnknownLocalException with an error message.
            IceInternal::throwNullProxyMarshalException(file, line, current);
        }
    }

    /// Downcasts a proxy without confirming the target object's type via a remote invocation.
    /// @param proxy The source proxy.
    /// @return A proxy with the requested type.
    /// @remark The preferred syntax is to construct the proxy from another proxy using the explicit proxy
    /// constructor.
    template<typename Prx, std::enable_if_t<std::is_base_of_v<ObjectPrx, Prx>, bool> = true>
    Prx uncheckedCast(const ObjectPrx& proxy)
    {
        return Prx::_fromReference(proxy._getReference());
    }

    /// Downcasts a proxy without confirming the target object's type via a remote invocation.
    /// @param proxy The source proxy (can be nullopt).
    /// @return A proxy with the requested type.
    /// @remark The preferred syntax is to construct the proxy from another proxy using the explicit ObjectPrx
    /// constructor.
    template<typename Prx, std::enable_if_t<std::is_base_of_v<ObjectPrx, Prx>, bool> = true>
    std::optional<Prx> uncheckedCast(const std::optional<ObjectPrx>& proxy)
    {
        if (proxy)
        {
            return uncheckedCast<Prx>(proxy.value());
        }
        else
        {
            return std::nullopt;
        }
    }

    /// Downcasts a proxy without confirming the target object's type via a remote invocation.
    /// @param proxy The source proxy.
    /// @param facet A facet name.
    /// @return A proxy with the requested type and facet.
    template<typename Prx, std::enable_if_t<std::is_base_of_v<ObjectPrx, Prx>, bool> = true>
    Prx uncheckedCast(const ObjectPrx& proxy, std::string facet)
    {
        return uncheckedCast<Prx>(proxy->ice_facet(std::move(facet)));
    }

    /// Downcasts a proxy without confirming the target object's type via a remote invocation.
    /// @param proxy The source proxy.
    /// @param facet A facet name.
    /// @return A proxy with the requested type and facet.
    template<typename Prx, std::enable_if_t<std::is_base_of_v<ObjectPrx, Prx>, bool> = true>
    std::optional<Prx> uncheckedCast(const std::optional<ObjectPrx>& proxy, std::string facet)
    {
        if (proxy)
        {
            return uncheckedCast<Prx>(proxy->ice_facet(std::move(facet)));
        }
        else
        {
            return std::nullopt;
        }
    }

    /// Downcasts a proxy after confirming the target object's type via a remote invocation.
    /// @param proxy The source proxy.
    /// @param context The request context.
    /// @return A proxy with the requested type, or nullopt if the target object does not support the requested type.
    template<typename Prx, std::enable_if_t<std::is_base_of_v<ObjectPrx, Prx>, bool> = true>
    std::optional<Prx> checkedCast(const ObjectPrx& proxy, const Context& context = noExplicitContext)
    {
        if (proxy->ice_isA(Prx::ice_staticId(), context))
        {
            return uncheckedCast<Prx>(proxy);
        }
        else
        {
            return std::nullopt;
        }
    }

    /// Downcasts a proxy after confirming the target object's type via a remote invocation.
    /// @param proxy The source proxy (can be nullopt).
    /// @param context The request context.
    /// @return A proxy with the requested type, or nullopt if the source proxy is nullopt or if the target object does
    /// not support the requested type.
    template<typename Prx, std::enable_if_t<std::is_base_of_v<ObjectPrx, Prx>, bool> = true>
    std::optional<Prx> checkedCast(const std::optional<ObjectPrx>& proxy, const Context& context = noExplicitContext)
    {
        return proxy ? checkedCast<Prx>(proxy.value(), context) : std::nullopt;
    }

    /// Downcasts a proxy after confirming the target object's type via a remote invocation.
    /// @param proxy The source proxy.
    /// @param facet A facet name.
    /// @param context The request context.
    /// @return A proxy with the requested type and facet, or nullopt if the target facet is not of the requested
    /// type.
    template<typename Prx, std::enable_if_t<std::is_base_of_v<ObjectPrx, Prx>, bool> = true>
    std::optional<Prx>
    checkedCast(const ObjectPrx& proxy, std::string facet, const Context& context = noExplicitContext)
    {
        return checkedCast<Prx>(proxy->ice_facet(std::move(facet)), context);
    }

    /// Downcasts a proxy after confirming the target object's type via a remote invocation.
    /// @param proxy The source proxy (can be nullopt).
    /// @param facet A facet name.
    /// @param context The request context.
    /// @return A proxy with the requested type and facet, or nullopt if the source proxy is nullopt, or if the target
    /// facet is not of the requested type.
    template<typename Prx, std::enable_if_t<std::is_base_of_v<ObjectPrx, Prx>, bool> = true>
    std::optional<Prx>
    checkedCast(const std::optional<ObjectPrx>& proxy, std::string facet, const Context& context = noExplicitContext)
    {
        return proxy ? checkedCast<Prx>(proxy->ice_facet(std::move(facet)), context) : std::nullopt;
    }

    /// Operator less-than.
    /// @param lhs The left-hand side proxy.
    /// @param rhs The right-hand side proxy.
    /// @return `true` if @p lhs is less than @p rhs, `false` otherwise.
    ICE_API bool operator<(const ObjectPrx& lhs, const ObjectPrx& rhs) noexcept;

    /// Operator equal to.
    /// @param lhs The left-hand side proxy.
    /// @param rhs The right-hand side proxy.
    /// @return `true` if @p lhs is equal to @p rhs, `false` otherwise.
    ICE_API bool operator==(const ObjectPrx& lhs, const ObjectPrx& rhs) noexcept;

    /// Operator greater-than.
    /// @param lhs The left-hand side proxy.
    /// @param rhs The right-hand side proxy.
    /// @return `true` if @p lhs is greater than @p rhs, `false` otherwise.
    inline bool operator>(const ObjectPrx& lhs, const ObjectPrx& rhs) noexcept { return rhs < lhs; }

    /// Operator less-than or equal to.
    /// @param lhs The left-hand side proxy.
    /// @param rhs The right-hand side proxy.
    /// @return `true` if @p lhs is less than or equal to @p rhs, `false` otherwise.
    inline bool operator<=(const ObjectPrx& lhs, const ObjectPrx& rhs) noexcept { return !(lhs > rhs); }

    /// Operator greater-than or equal to.
    /// @param lhs The left-hand side proxy.
    /// @param rhs The right-hand side proxy.
    /// @return `true` if @p lhs is greater than or equal to @p rhs, `false` otherwise.
    inline bool operator>=(const ObjectPrx& lhs, const ObjectPrx& rhs) noexcept { return !(lhs < rhs); }

    /// Operator not equal to.
    /// @param lhs The left-hand side proxy.
    /// @param rhs The right-hand side proxy.
    /// @return `true` if @p lhs is not equal to @p rhs, `false` otherwise.
    inline bool operator!=(const ObjectPrx& lhs, const ObjectPrx& rhs) noexcept { return !(lhs == rhs); }

    /// Compares the object identities of two proxies.
    /// @param lhs A proxy.
    /// @param rhs A proxy.
    /// @return `true` if the identity in lhs compares less than the identity in rhs, `false` otherwise.
    ICE_API bool proxyIdentityLess(const std::optional<ObjectPrx>& lhs, const std::optional<ObjectPrx>& rhs) noexcept;

    /// Compares the object identities of two proxies.
    /// @param lhs A proxy.
    /// @param rhs A proxy.
    /// @return `true` if the identity in lhs compares equal to the identity in rhs, `false` otherwise.
    ICE_API bool proxyIdentityEqual(const std::optional<ObjectPrx>& lhs, const std::optional<ObjectPrx>& rhs) noexcept;

    /// Compares the object identities and facets of two proxies.
    /// @param lhs A proxy.
    /// @param rhs A proxy.
    /// @return `true` if the identity and facet in lhs compare less than the identity and facet
    /// in rhs, `false` otherwise.
    ICE_API bool
    proxyIdentityAndFacetLess(const std::optional<ObjectPrx>& lhs, const std::optional<ObjectPrx>& rhs) noexcept;

    /// Compares the object identities and facets of two proxies.
    /// @param lhs A proxy.
    /// @param rhs A proxy.
    /// @return `true` if the identity and facet in lhs compare equal to the identity and facet
    /// in rhs, `false` otherwise.
    ICE_API bool
    proxyIdentityAndFacetEqual(const std::optional<ObjectPrx>& lhs, const std::optional<ObjectPrx>& rhs) noexcept;
}

#endif
