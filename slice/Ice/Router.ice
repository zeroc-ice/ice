// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#pragma once

[["cpp:header-ext:h", "objc:header-dir:objc", "js:ice-build"]]

#include <Ice/BuiltinSequences.ice>

["objc:prefix:ICE"]
module Ice
{

/**
 *
 * The Ice router interface. Routers can be set either globally with
 * {@link Communicator#setDefaultRouter}, or with <tt>ice_router</tt> on specific
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
     * If a null proxy is returned, the client will forward requests
     * to the router's endpoints.
     *
     * @return The router's client proxy.
     *
     **/
    ["nonmutating", "cpp:const"] idempotent Object* getClientProxy();

    /**
     *
     * Get the router's server proxy, i.e., the proxy to use for
     * forwarding requests from the server to the router.
     *
     * @return The router's server proxy.
     *
     **/
    ["nonmutating", "cpp:const"] idempotent Object* getServerProxy();

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

/**
 *
 * This inferface should be implemented by services implementing the
 * Ice::Router interface. It should be advertised through an Ice
 * object with the identity `Ice/RouterFinder'. This allows clients to
 * retrieve the router proxy with just the endpoint information of the
 * service.
 *
 **/
interface RouterFinder
{
    /**
     *
     * Get the router proxy implemented by the process hosting this
     * finder object. The proxy might point to several replicas.
     *
     * @return The router proxy.
     *
     **/
    Router* getRouter();
};

};

