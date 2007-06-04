// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_OBJECT_ADAPTER_ICE
#define ICE_OBJECT_ADAPTER_ICE

#include <Ice/CommunicatorF.ice>
#include <Ice/ServantLocatorF.ice>
#include <Ice/RouterF.ice>
#include <Ice/LocatorF.ice>
#include <Ice/Identity.ice>
#include <Ice/FacetMap.ice>

module Ice
{

/**
 *
 * The object adapter provides an up-call interface from the Ice
 * run time to the implementation of Ice objects.
 *
 * The object adapter is responsible for receiving requests
 * from endpoints, and for mapping between servants, identities, and
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
     * Get the name of this object adapter.
     *
     * @return This object adapter's name.
     *
     **/
    ["cpp:const"] string getName();

    /**
     *
     * Get the communicator this object adapter belongs to.
     *
     * @return This object adapter's communicator.
     *
     * @see Communicator
     *
     **/
    ["cpp:const"] Communicator getCommunicator();

    /**
     *
     * Activate all endpoints that belong to this object
     * adapter. After activation, the object adapter can dispatch
     * requests received through its endpoints.
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
     * <p class="Note"> Holding is not immediate, i.e., after [hold]
     * returns, the object adapter might still be active for some
     * time. You can use [waitForHold] to wait until holding is
     * complete.
     *
     * @see activate
     * @see deactivate
     * @see waitForHold
     *
     **/
    void hold();

    /**
     *
     * Wait until the object adapter holds requests. Calling [hold]
     * initiates holding of requests, and [waitForHold] only returns
     * when holding of requests has been completed.
     *
     * @see hold
     * @see waitForDeactivate
     * @see Communicator::waitForShutdown
     *
     **/
    void waitForHold();

    /**
     *
     * Deactivate all endpoints that belong to this object
     * adapter. After deactivation, the object adapter stops receiving
     * requests through its endpoints. Object adapters that have been
     * deactivated must not be reactivated again, and cannot be used
     * otherwise. Attempts to use a deactivated object adapter raise
     * [ObjectAdapterDeactivatedException]; however, attempts to
     * [deactivate] an already deactivated object adapter are ignored
     * and do nothing.
     *
     * <p class="Note"> After [deactivate] returns, no new requests are
     * processed by the object adapter. However, requests that have
     * been started before [deactivate] was called might still be
     * active. You can use [waitForDeactivate] to wait for the
     * completion of all requests for this object
     * adapter.
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
     * [waitForDeactivate] only returns when deactivation has
     * been completed. 
     *
     * @see deactivate
     * @see waitForHold
     * @see Communicator::waitForShutdown
     *
     **/
    void waitForDeactivate();

    /**
     *
     * Check whether object adapter has been deacivated.
     *
     * @return Whether adapter has been deactivated.
     *
     * @see Communicator::shutdown
     *
     **/
    ["cpp:const"] bool isDeactivated();

    /**
     *
     * Destroys the object adapter and cleans up all resources
     * held by the object adapter. If object adapter has not yet
     * been deactivated, [destroy] implicitly initiates the
     * deactivation and waits for it to finish. Once [destroy] has
     * returned it is possible to recreate an object adapter with
     * the same name. Subsequent calls to [destroy] are ignored.
     * 
     * @see deactivate
     * @see waitForDeactivate
     * @see Communicator::destroy
     *
     **/
    void destroy();

    /**
     *
     * Add a servant to this object adapter's Active Servant Map. Note
     * that one servant can implement several Ice objects by
     * registering the servant with multiple identities. Adding a
     * servant with an identity that is in the map already throws
     * [AlreadyRegisteredException].
     *
     * @param servant The servant to add.
     *
     * @param id The identity of the Ice object that is
     * implemented by the servant.
     *
     * @return A proxy that matches the given identity and this object
     * adapter.
     *
     * @see Identity
     * @see addFacet
     * @see addWithUUID
     * @see remove
     * @see find
     *
     **/
    Object* add(Object servant, Identity id);

    /**
     *
     * Like [add], but with a facet. Calling <tt>add(servant, * id)</tt>
     * is equivalent to calling [addFacet] with an empty
     * facet.
     *
     * @param servant The servant to add.
     *
     * @param id The identity of the Ice object that is
     * implemented by the servant.
     *
     * @param facet The facet. An empty facet means the default
     * facet.
     *
     * @return A proxy that matches the given identity, facet, and
     * this object adapter.
     *
     * @see Identity
     * @see add
     * @see addFacetWithUUID
     * @see removeFacet
     * @see findFacet
     *
     **/
    Object* addFacet(Object servant, Identity id, string facet);

    /**
     *
     * Add a servant to this object adapter's Active Servant Map,
     * using an automatically generated UUID as its identity. Note that
     * the generated UUID identity can be accessed using the proxy's
     * [ice_getIdentity] operation.
     *
     * @param servant The servant to add.
     *
     * @return A proxy that matches the generated UUID identity and
     * this object adapter.
     *
     * @see Identity
     * @see add
     * @see addFacetWithUUID
     * @see remove
     * @see find
     *
     **/
    Object* addWithUUID(Object servant);

    /**
     *
     * Like [addWithUUID], but with a facet. Calling
     * <tt>addWithUUID(servant)</tt> is equivalent to calling
     * [addFacetWithUUID] with an empty facet.
     *
     * @param servant The servant to add.
     *
     * @param facet The facet. An empty facet means the default
     * facet.
     *
     * @return A proxy that matches the generated UUID identity,
     * facet, and this object adapter.
     *
     * @see Identity
     * @see addFacet
     * @see addWithUUID
     * @see removeFacet
     * @see findFacet
     *
     **/
    Object* addFacetWithUUID(Object servant, string facet);

    /**
     *
     * Remove a servant (that is, the default facet) from the
     * object adapter's Active Servant Map.
     *
     * @param id The identity of the Ice object that is
     * implemented by the servant. If the servant implements multiple
     * Ice objects, [remove] has to be called for all those Ice
     * objects. Removing an identity that is not in the map throws
     * [NotRegisteredException].
     *
     * @return The removed servant.
     *
     * @see Identity
     * @see add
     * @see addWithUUID
     *
     **/
    Object remove(Identity id);

    /**
     *
     * Like [remove], but with a facet. Calling
     * <tt>remove(id)</tt> is equivalent to calling
     * [removeFacet] with an empty facet.
     *
     * @param id The identity of the Ice object that is
     * implemented by the servant.
     *
     * @param facet The facet. An empty facet means the default
     * facet.
     *
     * @return The removed servant.
     *
     * @see Identity
     * @see addFacet
     * @see addFacetWithUUID
     *
     **/
    Object removeFacet(Identity id, string facet);

    /**
     *
     * Remove all facets with the given identity from the Active
     * Servant Map. The operation completely removes the Ice object,
     * including its default facet. Removing an identity that
     * is not in the map throws [NotRegisteredException].
     *
     * @param id The identity of the Ice object to be removed.
     *
     * @return A collection containing all the facet names and
     * servants of the removed Ice object.
     *
     * @see remove
     * @see removeFacet
     *
     **/
    FacetMap removeAllFacets(Identity id);

    /**
     *
     * Look up a servant in this object adapter's Active Servant Map
     * by the identity of the Ice object it implements.
     *
     * <p class="Note">This operation only tries to lookup a servant in
     * the Active Servant Map. It does not attempt to find a servant
     * by using any installed [ServantLocator].
     *
     * @param id The identity of the Ice object for which the
     * servant should be returned.
     *
     * @return The servant that implements the Ice object with the
     * given identity, or null if no such servant has been found.
     *
     * @see Identity
     * @see findFacet
     * @see findByProxy
     *
     **/
    ["cpp:const"] Object find(Identity id);

    /**
     *
     * Like [find], but with a facet. Calling
     * <tt>find(id)</tt> is equivalent to calling
     * [findFacet] with an empty facet.
     *
     * @param id The identity of the Ice object for which the
     * servant should be returned.
     *
     * @param facet The facet. An empty facet means the default
     * facet.
     *
     * @return The servant that implements the Ice object with the
     * given identity and facet, or null if no such servant has been
     * found.
     *
     * @see Identity
     * @see find
     * @see findByProxy
     *
     **/
    ["cpp:const"] Object findFacet(Identity id, string facet);

    /**
     *
     * Find all facets with the given identity in the Active Servant
     * Map.
     *
     * @param id The identity of the Ice object for which the facets
     * should be returned.
     *
     * @return A collection containing all the facet names and
     * servants that have been found, or an empty map if there is no
     * facet for the given identity.
     *
     * @see find
     * @see findFacet
     *
     **/
    ["cpp:const"] FacetMap findAllFacets(Identity id);

    /**
     *
     * Look up a servant in this object adapter's Active Servant Map,
     * given a proxy.
     *
     * <p class="Note">This operation only tries to lookup a servant in
     * the Active Servant Map. It does not attempt to find a servant
     * via any installed [ServantLocator]s.
     *
     * @param proxy The proxy for which the servant should be returned.
     *
     * @return The servant that matches the proxy, or null if no such
     * servant has been found.
     *
     * @see find
     * @see findFacet
     *
     **/
    ["cpp:const"] Object findByProxy(Object* proxy);

    /**
     *
     * Add a Servant Locator to this object adapter. Adding a servant
     * locator for a category for which a servant locator is already
     * registered throws [AlreadyRegisteredException]. To dispatch
     * operation calls on servants, the object adapter tries to find a
     * servant for a given Ice object identity and facet in the
     * following order:
     *
     * <ol>
     *
     * <li>The object adapter tries to find a servant for
     * the identity and facet in the Active Servant
     * Map.</li>
     *
     * <li>If no servant has been found in the Active
     * Servant Map, the object adapter tries to find a locator for the
     * category component of the identity. If a locator is found, the
     * object adapter tries to find a servant using this
     * locator.</li>
     *
     * <li>If no servant has been found by any of the
     * preceding steps, the object adapter tries to find a locator for
     * an empty category, regardless of the category contained in the
     * identity. If a locator is found, the object adapter tries to
     * find a servant using this locator.</li>
     *
     * <li>If no servant has been found with any of the
     * preceding steps, the object adapter gives up and the caller
     * receives [ObjectNotExistException] or
     * [FacetNotExistException].</li>
     *
     * </ol>
     *
     * <p class="Note">Only one locator for the empty category can be
     * installed.
     *
     * @param locator The locator to add.
     *
     * @param category The category for which the Servant Locator can
     * locate servants, or an empty string if the Servant Locator does
     * not belong to any specific category.
     *
     * @see Identity
     * @see findServantLocator
     * @see ServantLocator
     *
     **/
    void addServantLocator(ServantLocator locator, string category);

    /**
     *
     * Find a Servant Locator installed with this object adapter.
     *
     * @param category The category for which the Servant Locator can
     * locate servants, or an empty string if the Servant Locator does
     * not belong to any specific category.
     *
     * @return The Servant Locator, or null if no Servant Locator was
     * found for the given category.
     *
     * @see Identity
     * @see addServantLocator
     * @see ServantLocator
     *
     **/
    ["cpp:const"] ServantLocator findServantLocator(string category);

    /**
     *
     * Create a proxy for the object with the given identity. If this
     * object adapter is configured with an adapter id, the return
     * value is an indirect proxy that refers to the adapter id. If
     * a replica group id is also defined, the return value is an
     * indirect proxy that refers to the replica group id. Otherwise,
     * if no adapter id is defined, the return value is a direct
     * proxy containing this object adapter's published endpoints.
     *
     * @param id The object's identity.
     *
     * @return A proxy for the object with the given identity.
     *
     * @see Identity
     *
     **/
    ["cpp:const"] Object* createProxy(Identity id);

    /**
     *
     * Create a direct proxy for the object with the given identity.
     * The returned proxy contains this object adapter's published
     * endpoints.
     *
     * @param id The object's identity.
     *
     * @return A proxy for the object with the given identity.
     *
     * @see Identity
     *
     **/
    ["cpp:const"] Object* createDirectProxy(Identity id);

    /**
     *
     * Create an indirect proxy for the object with the given identity.
     * If this object adapter is configured with an adapter id, the
     * return value refers to the adapter id. Otherwise, the return
     * value contains only the object identity.
     *
     * @param id The object's identity.
     *
     * @return A proxy for the object with the given identity.
     *
     * @see Identity
     *
     **/
    ["cpp:const"] Object* createIndirectProxy(Identity id);

    /**
     *
     * Create a "reverse proxy" for the object with the given identity.
     * A reverse proxy uses the incoming connections that have been
     * established from a client to this object adapter.
     *
     * <p class="Note"> This operation is intended to be used by special
     * services, such as [Router] implementations. Regular user code
     * should not attempt to use this operation.
     *
     * @param id The identity for which a proxy is to be created.
     *
     * @return A "reverse proxy" that matches the given identity and
     * uses the incoming connections of this object adapter.
     *
     * @see Identity
     *
     **/
    ["cpp:const"] Object* createReverseProxy(Identity id);

    /**
     * Set an Ice locator for this object adapter. By doing so, the
     * object adapter will register itself with the locator registry
     * when it is activated for the first time. Furthermore, the proxies
     * created by this object adapter will contain the adapter name instead
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

    /**
     * Refresh the set of published endpoints. The PublishedEndpoints
     * property will be reread if it is set. Also the list of local
     * interfaces will be rechecked for any endpoints configured to listen
     * on INADDR_ANY and published endpoints will be updated with new
     * address list.
     *
     **/
    void refreshPublishedEndpoints();
};

};

#endif
