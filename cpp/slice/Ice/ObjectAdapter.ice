// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_OBJECT_ADAPTER_ICE
#define ICE_OBJECT_ADAPTER_ICE

#include <Ice/CommunicatorF.ice>

module Ice
{

local class ServantLocator;

/**
 *
 * The Object Adapter, which is responsible for receiving requests
 * from Endpoints, and for mapping between Servant, Identities, and
 * Proxies. Object Adapters are created with the Communicator's
 * <literal>createObjectAdapter</literal> and
 * <literal>createObjectAdapterWithEndpoints</literal> operations.
 *
 * @see Communicator
 * @see ServantLocator
 *
 **/
local class ObjectAdapter
{
    /**
     *
     * Get the name of this Object Adapter. The name is mainly used
     * for configuration purposes with Properties.
     *
     * @return The name of this Object Adapter.
     *
     * @see Properties
     *
     **/
    string getName();

    /**
     *
     * Get the Communicator this Object Adapter belongs to.
     *
     * @return This Object Adapter's Communicator.
     *
     * @see Communicator
     *
     **/
    Communicator getCommunicator();

    /**
     *
     * Activate all Endpoints that belong to this Object
     * Adapter. After activation, the Object Adapter can dispatch
     * requests received through its Endpoints.
     *
     * @see hold
     * @see deactivate
     *
     **/
    void activate();

    /**
     *
     * Temporarily hold receiving and dispatching requests. The Object
     * Adapter can be reactivated with the <literal>activate</literal>
     * operation.
     *
     * @see activate
     * @see deactivate
     *
     **/
    void hold();

    /**
     *
     * Deactivate all Endpoints that belong to this Object
     * Adapter. After deactivation, the Object Adapter stops receiving
     * requests through its Endpoints. Object Adapter's that have
     * been deactivated must not be reactivated again, i.e., the
     * deactivation is permanent and <literal>activate</literal> or
     * <literal>hold</literal> must not be called after calling
     * <literal>deactivate</literal>.
     *
     * @see activate
     * @see hold
     *
     **/
    void deactivate();

    /**
     *
     * Add a Servant to this Object Adapter's Active Servant Map. Note
     * that one Servant can implement several Ice Objects by
     * registering the Servant with multiple identities.
     *
     * @param servant The Servant to add.
     *
     * @param identity The identity of the Ice Object that is
     * implemented by the Servant.
     *
     * @return A Proxy that matches the given identity and this Object
     * Adapter.
     *
     * @see addTemporary
     * @see remove
     *
     **/
    Object* add(Object servant, string identity);

    /**
     *
     * Add a temporary Servant to this Object Adapter's Active Servant
     * Map. "Temporary" means that the Ice Object implemented by the
     * Servant does not have a fixed identity. Instead, a temporary
     * identity is assigned by the Object Adapter. Such temporary
     * identity is only valid for the lifetime of this Object Adapter,
     * or until the Servant is removed with <literal>remove</literal>.
     *
     * @param servant The Servant to add.
     *
     * @return A Proxy that matches the temporary identity and this
     * Object Adapter.
     *
     * @see add
     * @see remove
     *
     **/
    Object* addTemporary(Object servant);

    /**
     *
     * Remove a Servant from the Object Adapter's Active Servant Map.
     *
     * @param identity The identity of the Ice Object that is
     * implemented by the Servant. If the Servant implements multiple
     * Ice Objects, <literal>remove</literal> has to be called for all
     * such Ice Objects.
     *
     * @see add
     * @see addTemporary
     *
     **/
    void remove(string identity);

    /**
     *
     * Set a Servant Locator for this Object Adapter.
     *
     * @param locator The Servant Locator to set.     
     *
     * @see ServantLocator
     * @see getServantLocator
     *
     **/
    void setServantLocator(ServantLocator locator);

    /**
     *
     * Get the Object Locator for this Object Adapter.
     *
     * @return The Servant Locator for this Object Adapter. If no
     * Servant Locator is set, null is returned.
     *
     * @see ServantLocator
     * @see setServantLocator
     *
     **/
    ServantLocator getServantLocator();

    /**
     *
     * Look up a Servant in this Object Adapter's Active Servant Map by
     * the identity of the Ice Object it implements.
     *
     * @param identity The identity of the Ice Object for which the
     * Servant should be returned.
     *
     * @return The Servant that implements the Ice Object with the
     * given identity.
     *
     * see proxyToServant
     *
     **/
    Object identityToServant(string identity);

    /**
     *
     * Look up a Servant in this Object Adapter's Active Servant Map,
     * given a Proxy.
     *
     * @param Proxy A proxy for which the Servant should be returned.
     *
     * @return The Servant that matches the Proxy.
     *
     * see identityToServant
     *
     **/
    Object proxyToServant(Object* proxy);

    /**
     *
     * Create a Proxy that matches this Object Adapter and the given
     * identity.
     *
     * @param identity The identity for which a Proxy is to be created.
     *
     * @return A Proxy that matches the given identity and this Object
     * Adapter.
     *
     **/
    Object* createProxy(string identity);
};

/**
 *
 * The Servant Locator, which is called by the Object Adapter to
 * locate Servants, which it doesn't find in its Active Servant
 * Map. The Servant Locator must be set with the Object Adapter's
 * <literal>setServantLocator</literal> operation.
 *
 * @see ObjectAdapter
 * @see ObjectAdapter::setServantLocator
 * @see ObjectAdapter::getServantLocator
 *
 **/
local class ServantLocator
{
    /**
     *
     * Called by the Object Adapter before a request, in case a
     * Servant cannot be found in the Object Adapter's Active Servant
     * Map. Note that the Object Adapter does not automatically insert
     * the returned Servant into it's Active Servant Map. This must be
     * done by the Servant Locator's implementation, if desired.
     *
     * @param adapter The Object Adapter that calls the Servant Locator.
     *
     * @param identity The identity of the Ice Object to locate a
     * Servant for.
     *
     * @param cookie A "cookie", which is returned to
     * <literal>finished</literal>.
     *
     * @return The located Servant, or null if no suitable Servant has
     * been found.
     *
     * @see ObjectAdapter
     * @see finished
     *
     **/
    Object locate(ObjectAdapter adapter, string identity; Object cookie);

    /**
     *
     * Called by the Object Adapter after a request. This operation is
     * only called if <literal>locate</literal> did not return null,
     * or if <literal>locate</literal> threw a
     * <literal>LocationForward</literal> exception. This operation
     * can be used for cleanup after a request.
     *
     * @param adapter The Object Adapter that calls the Servant Locator.
     *
     * @param identity The identity of the Ice Object for which a
     * Servant was located by <literal>locate</literal>.
     *
     * @param servant The Servant that was returned by
     * <literal>locate</literal>.
     *
     * @param cookie The cookie that was returned by
     * <literal>locate</literal>.
     *
     * @see ObjectAdapter
     * @see locate
     *
     **/
    void finished(ObjectAdapter adapter, string identity, Object servant, Object cookie);
};

};

#endif
