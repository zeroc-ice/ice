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
#include <Ice/ServantLocatorF.ice>
#include <Ice/RouterF.ice>
#include <Ice/Identity.ice>

module Ice
{

/**
 *
 * The Object Adapter, which is responsible for receiving requests
 * from Endpoints, and for mapping between Servant, Identities, and
 * Proxies.
 *
 * @see Communicator
 * @see ServantLocator
 *
 **/
local interface ObjectAdapter
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
     * Adapter can be reactivated with the [activate] operation.
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
     * requests through its Endpoints. Object Adapter's that have been
     * deactivated must not be reactivated again, i.e., the
     * deactivation is permanent and [activate] or [hold] must not be
     * called after calling [deactivate]. Attempting to do so results
     * in an [ObjectAdapterDeactivatedException] being thrown. Calls
     * to [deactivate] on an already deactivated Object Adapter are
     * ignored.
     *
     * @see activate
     * @see hold
     * @see Communicator::shutdown
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
     * @see Identity
     * @see addWithUUID
     * @see remove
     *
     **/
    Object* add(Object servant, Identity identity);

    /**
     *
     * Add a Servant to this Object Adapter's Active Servant Map,
     * using an automatically generated UUID as identity. Note that
     * the generated UUID identity can be accessed using the Proxy's
     * [ice_getIdentity] operation.
     *
     * @param servant The Servant to add.
     *
     * @return A Proxy that matches the generated UUID identity and
     * this Object Adapter.
     *
     * @see Identity
     * @see add
     * @see remove
     *
     **/
    Object* addWithUUID(Object servant);

    /**
     *
     * Remove a Servant from the Object Adapter's Active Servant Map.
     *
     * @param identity The identity of the Ice Object that is
     * implemented by the Servant. If the Servant implements multiple
     * Ice Objects, [remove] has to be called for all such Ice
     * Objects.
     *
     * @see Identity
     * @see add
     * @see addWithUUID
     *
     **/
    void remove(Identity identity);

    /**
     *
     * Add a Servant Locator to this Object Adapter. If a locator has
     * already been installed for the given category, the current
     * locator for this category is replaced by the new one. To dispatch
     * operation calls on Servants, the Object Adapter tries to find a
     * Servant for a given Ice Object identity in the following order:
     *
     * <orderedlist>
     *
     * <listitem><para>The Object Adapter tries to find a Servant for
     * the identity in the Active Servant Map.</para></listitem>
     *
     * <listitem><para>If no Servant has been found in the Active
     * Servant Map, the Object Adapter tries to find a locator for the
     * category component of the identity. If a locator is found, the
     * Object Adapter tries to find a Servant using this
     * locator.</para></listitem>
     *
     * <listitem><para>If no Servant has been found by any of the
     * preceding steps, the Object Adapter tries to find a locator for
     * an empty category, regardless of the category contained in the
     * identity. If a locator is found, the Object Adapter tries to
     * find a Servant using this locator.</para></listitem>
     *
     * <listitem><para>If no Servant has been found with any of the
     * preceding steps, the Object Adapter gives up and the caller
     * will receive an [ObjectNotExistException].</para></listitem>
     *
     * </orderedlist>
     *
     * <note><para>Only one locator for an empty category can be
     * installed.</para></note>
     *
     * @param locator The locator to add.
     *
     * @param category The category for which the Servant Locator can
     * locate Servants, or an empty string if the Servant Locator does
     * not belong to any category.
     *
     * @see Identity
     * @see removeServantLocator
     * @see findServantLocator
     * @see ServantLocator
     *
     **/
    void addServantLocator(ServantLocator locator, string category);

    /**
     *
     * Remove a Servant locator from this Object Adapter. This
     * operation does nothing if no locator for the given category has
     * been installed.
     *
     * @param category The category for which the Servant Locator can
     * locate Servants, or an empty string if the Servant Locator does
     * not belong to any category.
     *
     * @see Identity
     * @see addServantLocator
     * @see findServantLocator
     * @see ServantLocator
     *
     **/
    void removeServantLocator(string category);

    /**
     *
     * Find a Servant Locator installed with this Object Adapter.
     *
     * @param category The category for which the Servant Locator can
     * locate Servants, or an empty string if the Servant Locator does
     * not belong to any category.
     *
     * @return The Servant Locator, or null if no Servant Locator was
     * found for the given category.
     *
     * @see Identity
     * @see addServantLocator
     * @see removeServantLocator
     * @see ServantLocator
     *
     **/
    ServantLocator findServantLocator(string category);

    /**
     *
     * Look up a Servant in this Object Adapter's Active Servant Map
     * by the identity of the Ice Object it implements.
     *
     * <note><para>This operation only tries to lookup a Servant in
     * the Active Servant Map. It does not attempt to find a Servant
     * by using any installed [ServantLocator].</para></note>
     *
     * @param identity The identity of the Ice Object for which the
     * Servant should be returned.
     *
     * @return The Servant that implements the Ice Object with the
     * given identity, or null if no such Servant has been found.
     *
     * @see Identity
     * @see proxyToServant
     *
     **/
    Object identityToServant(Identity identity);

    /**
     *
     * Look up a Servant in this Object Adapter's Active Servant Map,
     * given a Proxy.
     *
     * <note><para>This operation only tries to lookup a Servant in
     * the Active Servant Map. It does not attempt to find a Servant
     * via any installed [ServantLocator]s.</para></note>
     *
     * @param Proxy The proxy for which the Servant should be returned.
     *
     * @return The Servant that matches the Proxy, or null if no such
     * Servant has been found.
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
     * @see Identity
     *
     **/
    Object* createProxy(Identity identity);

    /**
     *
     * Create a "reverse Proxy" that matches this Object Adapter and
     * the given identity. A reverse Proxy uses connections that have
     * been established from a client to this Object Adapter.
     *
     * <note><para> Like the [Router] interface, this operation is
     * intended to be used by router implementations. Regular user
     * code should not attempt to use this operation
     * directly. </para></note>
     *
     * @param identity The identity for which a Proxy is to be created.
     *
     * @return A "reverse Proxy" that matches the given identity and
     * this Object Adapter.
     *
     * @see Identity
     *
     **/
    Object* createReverseProxy(Identity identity);

    /**
     *
     * Add a &Glacier; router to this Object Adapter. By doing so,
     * this object adapter can receive callbacks from this router,
     * using connections which are established from this process to
     * the router, instead of the router having to establish a
     * connection back to this object adapter.
     *
     * <note><para> You can only add a particular router to one single
     * Object Adapter. Adding a router to more than one Object Adapter
     * will result in undefined behavior. However, it is possible to
     * add different routers to different Object
     * Adapters. </para></note>
     *
     * @param router The router to add to this Object Adapter.
     *
     * @see Router
     * @see Communicator::setDefaultRouter
     *
     **/
    void addRouter(Router* router);
};

};

#endif
