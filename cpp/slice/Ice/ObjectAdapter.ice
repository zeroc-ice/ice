// **********************************************************************
//
// Copyright (c) 2001
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
//
// **********************************************************************

#ifndef ICE_OBJECT_ADAPTER_ICE
#define ICE_OBJECT_ADAPTER_ICE

#include <Ice/CommunicatorF.ice>
#include <Ice/ServantLocatorF.ice>
#include <Ice/RouterF.ice>
#include <Ice/LocatorF.ice>
#include <Ice/Identity.ice>

module Ice
{

/**
 *
 * The object adapter, which is responsible for receiving requests
 * from Endpoints, and for mapping between Servant, Identities, and
 * proxies.
 *
 * @see Communicator
 * @see ServantLocator
 *
 **/
local interface ObjectAdapter
{
    /**
     *
     * Get the communicator this object adapter belongs to.
     *
     * @return This object adapter's communicator.
     *
     * @see Communicator
     *
     **/
    Communicator getCommunicator();

    /**
     *
     * Activate all Endpoints that belong to this object
     * adapter. After activation, the object adapter can dispatch
     * requests received through its Endpoints.
     *
     * @see hold
     * @see deactivate
     *
     **/
    void activate();

    /**
     *
     * Temporarily hold receiving and dispatching requests. The object
     * adapter can be reactivated with the [activate] operation.
     *
     * @see activate
     * @see deactivate
     *
     **/
    void hold();

    /**
     *
     * Deactivate all Endpoints that belong to this object
     * adapter. After deactivation, the object adapter stops receiving
     * requests through its Endpoints. object adapter's that have been
     * deactivated must not be reactivated again, i.e., the
     * deactivation is permanent and [activate] or [hold] must not be
     * called after calling [deactivate]. Attempting to do so results
     * in an [ObjectAdapterDeactivatedException] being thrown. Calls
     * to [deactivate] on an already deactivated object adapter are
     * ignored.
     *
     * <note><para> Deactivate is not immediate, i.e., after
     * [deactivate] returns, the object adapter might still be active
     * for some time. You can use [waitForDeactivate] to wait until
     * deactivation is complete. </para></note>
     *
     * @see activate
     * @see hold
     * @see waitForDeactivate
     * @see Communicator::shutdown
     *
     **/
    void deactivate();

    /**
     *
     * Wait until the object adapter has deactivated. Calling
     * [deactivate] initiates object adapter deactivation, and
     * [waitForDeactivate] only returns when such deactivation has
     * been completed.
     *
     * @see deactivate
     * @see Communicator::waitForShutdown
     *
     **/
    void waitForDeactivate();

    /**
     *
     * Add a Servant to this object adapter's Active Servant Map. Note
     * that one Servant can implement several &Ice; objects by
     * registering the Servant with multiple identities.
     *
     * @param servant The Servant to add.
     *
     * @param id The identity of the &Ice; object that is
     * implemented by the Servant.
     *
     * @return A proxy that matches the given identity and this object
     * adapter.
     *
     * @see Identity
     * @see addWithUUID
     * @see remove
     *
     **/
    Object* add(Object servant, Identity id);

    /**
     *
     * Add a Servant to this object adapter's Active Servant Map,
     * using an automatically generated UUID as identity. Note that
     * the generated UUID identity can be accessed using the proxy's
     * [ice_getIdentity] operation.
     *
     * @param servant The Servant to add.
     *
     * @return A proxy that matches the generated UUID identity and
     * this object adapter.
     *
     * @see Identity
     * @see add
     * @see remove
     *
     **/
    Object* addWithUUID(Object servant);

    /**
     *
     * Remove a Servant from the object adapter's Active Servant Map.
     *
     * @param id The identity of the &Ice; object that is
     * implemented by the Servant. If the Servant implements multiple
     * &Ice; objects, [remove] has to be called for all such &Ice;
     * objects.
     *
     * @see Identity
     * @see add
     * @see addWithUUID
     *
     **/
    void remove(Identity id);

    /**
     *
     * Add a Servant Locator to this object adapter. If a locator has
     * already been installed for the given category, the current
     * locator for this category is replaced by the new one. To dispatch
     * operation calls on Servants, the object adapter tries to find a
     * Servant for a given &Ice; object identity in the following order:
     *
     * <orderedlist>
     *
     * <listitem><para>The object adapter tries to find a Servant for
     * the identity in the Active Servant Map.</para></listitem>
     *
     * <listitem><para>If no Servant has been found in the Active
     * Servant Map, the object adapter tries to find a locator for the
     * category component of the identity. If a locator is found, the
     * object adapter tries to find a Servant using this
     * locator.</para></listitem>
     *
     * <listitem><para>If no Servant has been found by any of the
     * preceding steps, the object adapter tries to find a locator for
     * an empty category, regardless of the category contained in the
     * identity. If a locator is found, the object adapter tries to
     * find a Servant using this locator.</para></listitem>
     *
     * <listitem><para>If no Servant has been found with any of the
     * preceding steps, the object adapter gives up and the caller
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
     * not belong to any specific category.
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
     * Remove a Servant Locator from this object adapter. This
     * operation does nothing if no locator for the given category has
     * been installed.
     *
     * @param category The category for which the Servant Locator can
     * locate Servants, or an empty string if the Servant Locator does
     * not belong to any specific category.
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
     * Find a Servant Locator installed with this object adapter.
     *
     * @param category The category for which the Servant Locator can
     * locate Servants, or an empty string if the Servant Locator does
     * not belong to any specific category.
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
     * Look up a Servant in this object adapter's Active Servant Map
     * by the identity of the &&Ice;; object it implements.
     *
     * <note><para>This operation only tries to lookup a Servant in
     * the Active Servant Map. It does not attempt to find a Servant
     * by using any installed [ServantLocator].</para></note>
     *
     * @param id The identity of the &&Ice;; object for which the
     * Servant should be returned.
     *
     * @return The Servant that implements the &&Ice;; object with the
     * given identity, or null if no such Servant has been found.
     *
     * @see Identity
     * @see proxyToServant
     *
     **/
    Object identityToServant(Identity id);

    /**
     *
     * Look up a Servant in this object adapter's Active Servant Map,
     * given a proxy.
     *
     * <note><para>This operation only tries to lookup a Servant in
     * the Active Servant Map. It does not attempt to find a Servant
     * via any installed [ServantLocator]s.</para></note>
     *
     * @param proxy The proxy for which the Servant should be returned.
     *
     * @return The Servant that matches the proxy, or null if no such
     * Servant has been found.
     *
     * see identityToServant
     *
     **/
    Object proxyToServant(Object* proxy);

    /**
     *
     * Create a proxy that matches this object adapter and the given
     * identity.
     *
     * @param id The identity for which a proxy is to be created.
     *
     * @return A proxy that matches the given identity and this object
     * adapter.
     *
     * @see Identity
     *
     **/
    Object* createProxy(Identity id);

    /**
     *
     * Create a "direct proxy" that matches this object adapter and
     * the given identity. A direct proxy always contains the current
     * adapter endpoints.
     *
     * <note><para> This operation is intended to be used by locator
     * implementations. Regular user code should not attempt to use
     * this operation.</para></note>
     *
     * @param id The identity for which a proxy is to be created.
     *
     * @return A proxy that matches the given identity and this object
     * adapter.
     *
     * @see Identity
     *
     **/
    Object* createDirectProxy(Identity id);

    /**
     *
     * Create a "reverse proxy" that matches this object adapter and
     * the given identity. A reverse proxy uses connections that have
     * been established from a client to this object adapter.
     *
     * <note><para> Like the [Router] interface, this operation is
     * intended to be used by router implementations. Regular user
     * code should not attempt to use this operation.
     * </para></note>
     *
     * @param id The identity for which a proxy is to be created.
     *
     * @return A "reverse proxy" that matches the given identity and
     * this object adapter.
     *
     * @see Identity
     *
     **/
    Object* createReverseProxy(Identity id);

    /**
     *
     * Add a &Glacier; router to this object adapter. By doing so,
     * this object adapter can receive callbacks from this router,
     * using connections which are established from this process to
     * the router, instead of the router having to establish a
     * connection back to this object adapter.
     *
     * <note><para> You can only add a particular router to one single
     * object adapter. Adding a router to more than one object adapter
     * will result in undefined behavior. However, it is possible to
     * add different routers to different object
     * adapters. </para></note>
     *
     * @param rtr The router to add to this object adapter.
     *
     * @see Router
     * @see Communicator::setDefaultRouter
     *
     **/
    void addRouter(Router* rtr);

    /**
     * Set an &Ice; locator for this object adapter. By doing so, the
     * object adapter will register itself with the locator registry
     * when it's activated for the first time and the proxies created
     * with the object adapter will contain the adapter name instead
     * of its endpoints.
     *
     * @param loc The locator used by this object adapter.
     *
     * @see createDirectProxy
     * @see Locator
     * @see LocatorRegistry
     * 
     **/
    void setLocator(Locator* loc);
};

};

#endif
