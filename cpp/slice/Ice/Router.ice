// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_ROUTER_ICE
#define ICE_ROUTER_ICE

module Ice
{

/**
 *
 * The Ice router interface. Routers can be set either globally with
 * [Communicator::setGlobalRouter], or with [ice_router] on specific
 * Proxies.
 *
 * <note><para> This router interface is intended to be used by Ice
 * internals and by router implementations. Regular user code should
 * not attempt to use any functionality of this interface
 * directly. </para></note>
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
     * @param proxy The proxy to add.
     *
     **/
    void addProxy(Object* proxy);
};

};

#endif
