// Copyright (c) ZeroC, Inc.

#pragma once

[["cpp:dll-export:ICE_API"]]
[["cpp:doxygen:include:Ice/Ice.h"]]
[["cpp:header-ext:h"]]

[["js:module:@zeroc/ice"]]
[["python:pkgdir:Ice"]]

#include "BuiltinSequences.ice"

["java:identifier:com.zeroc.Ice"]
module Ice
{
    /// Represents an intermediary object that routes requests and replies between clients and Ice objects that are not
    /// directly reachable from these clients.
    interface Router
    {
        /// Gets the router's client proxy, i.e., the proxy to use for forwarding requests from the client to the
        /// router. If a null proxy is returned, the client will forward requests to the router's endpoints.
        /// @param hasRoutingTable Indicates whether or not the router supports a routing table. If `true`, the Ice
        /// runtime will call {@link addProxies} to populate the routing table. The Ice runtime assumes the router has
        /// a routing table when @p hasRoutingTable is not set. Introduced in Ice 3.7.
        /// @return The router's client proxy.
        ["cpp:const"]
        idempotent Object* getClientProxy(out optional(1) bool hasRoutingTable);

        /// Gets the router's server proxy, i.e., the proxy to use for forwarding requests from the server to the
        /// router. The Ice runtime uses the endpoints of this proxy as the published endpoints of bi-dir object
        /// adapters.
        /// @return The router's server proxy.
        ["cpp:const"]
        idempotent Object* getServerProxy();

        /// Adds new proxy information to the router's routing table.
        /// @param proxies The proxies to add. Adding a null proxy is an error.
        /// @return Proxies discarded by the router. These proxies are all non-null.
        idempotent ObjectProxySeq addProxies(ObjectProxySeq proxies);
    }

    /// Provides access to a {@link Router} object via a fixed identity.
    /// A RouterFinder is always registered with identity `Ice/RouterFinder`. This allows clients to obtain the
    /// associated Router proxy with just the endpoint information of the object. For example, you can use the
    /// RouterFinder proxy `Ice/RouterFinder:tcp -h somehost -p 4061` to get the Router proxy
    /// `MyGlacier2/Router:tcp -h somehost -p 4061`.
    interface RouterFinder
    {
        /// Gets a proxy to the associated {@link Router}. The proxy might point to several replicas.
        /// @return The router proxy. This proxy is never null.
        Router* getRouter();
    }
}
