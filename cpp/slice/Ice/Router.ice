// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_ROUTER_ICE
#define ICE_ROUTER_ICE

#include <Ice/BuiltinSequences.ice>

module Ice
{

/**
 *
 * The &Ice; router interface. Routers can be set either globally with
 * [Communicator::setDefaultRouter], or with [ice_router] on specific
 * proxies.
 *
 **/
interface Router
{
    /**
     *
     * Get the router's client proxy, i.e., the proxy to use for
     * forwarding requests from the client to the router.
     *
     * @return The router's client proxy.
     *
     **/
    nonmutating Object* getClientProxy();

    /**
     *
     * Get the router's server proxy, i.e., the proxy to use for
     * forwarding requests from the server to the router.
     *
     * @return The router's server proxy.
     *
     **/
    nonmutating Object* getServerProxy();

    /**
     *
     * Add new proxy information to the router's routing table.
     *
     * <note><para>This operation is deprecated, and only used for old
     * Ice clients (older than version 3.1).</para></note>
     *
     * @param proxy The proxy to add.
     *
     **/
    ["deprecate:addProxy() has been deprecated, use addProxies() instead."]
    idempotent void addProxy(Object* proxy);

    /**
     *
     * Add new proxy information to the router's routing table.
     *
     * @param proxies The proxies to add.
     *
     * @return Proxies discarded by the router.
     *
     **/
    idempotent ObjectProxySeq addProxies(ObjectProxySeq proxies);
};

};

#endif
