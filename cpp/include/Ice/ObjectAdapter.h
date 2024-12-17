//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_OBJECT_ADAPTER_H
#define ICE_OBJECT_ADAPTER_H

#include "CommunicatorF.h"
#include "Endpoint.h"
#include "FacetMap.h"
#include "ObjectAdapterF.h"
#include "Proxy.h"
#include "ProxyFunctions.h"
#include "ServantLocator.h"

#include <memory>
#include <optional>

namespace Ice
{
    class LocatorPrx;

    /**
     * The object adapter provides an up-call interface from the Ice run time to the implementation of Ice objects. The
     * object adapter is responsible for receiving requests from endpoints, and for mapping between servants,
     * identities, and proxies.
     * @see Communicator
     * @see ServantLocator
     * \headerfile Ice/Ice.h
     */
    class ObjectAdapter
    {
    public:
        virtual ~ObjectAdapter() = default;

        /**
         * Get the name of this object adapter.
         * @return This object adapter's name.
         */
        [[nodiscard]] virtual std::string getName() const noexcept = 0;

        /**
         * Get the communicator this object adapter belongs to.
         * @return This object adapter's communicator.
         * @see Communicator
         */
        [[nodiscard]] virtual CommunicatorPtr getCommunicator() const noexcept = 0;

        /**
         * Activate all endpoints that belong to this object adapter. After activation, the object adapter can dispatch
         * requests received through its endpoints.
         * @see #hold
         * @see #deactivate
         */
        virtual void activate() = 0;

        /**
         * Temporarily hold receiving and dispatching requests. The object adapter can be reactivated with the
         * {@link #activate} operation. <p class="Note"> Holding is not immediate, i.e., after {@link #hold} returns,
         * the object adapter might still be active for some time. You can use {@link #waitForHold} to wait until
         * holding is complete.
         * @see #activate
         * @see #deactivate
         * @see #waitForHold
         */
        virtual void hold() = 0;

        /**
         * Wait until the object adapter holds requests. Calling {@link #hold} initiates holding of requests, and
         * {@link #waitForHold} only returns when holding of requests has been completed.
         * @see #hold
         * @see #waitForDeactivate
         * @see Communicator#waitForShutdown
         */
        virtual void waitForHold() = 0;

        /**
         * Deactivates this object adapter: stop accepting new connections from clients and close gracefully all
         * incoming connections created by this object adapter once all outstanding dispatches have completed. If this
         * object adapter is indirect, this function also unregisters the object adapter from the Locator.
         * This function does not cancel outstanding dispatches--it lets them execute until completion. A new incoming
         * request on an existing connection will be accepted and can delay the closure of the connection.
         * A deactivated object adapter cannot be reactivated again; it can only be destroyed.
         * @see #waitForDeactivate
         * @see Communicator#shutdown
         */
        virtual void deactivate() noexcept = 0;

        /**
         * Wait until deactivate is called on this object adapter and all connections accepted by this object adapter
         * are closed. A connection is closed only after all outstanding dispatches on this connection have completed.
         * @see #deactivate
         * @see Communicator#waitForShutdown
         */
        virtual void waitForDeactivate() noexcept = 0;

        /**
         * Check whether object adapter has been deactivated.
         * @return Whether adapter has been deactivated.
         * @see Communicator#shutdown
         */
        [[nodiscard]] virtual bool isDeactivated() const noexcept = 0;

        /**
         * Destroys this object adapter and cleans up all resources held by this object adapter.
         * Once this function has returned, it is possible to create another object adapter with the same name.
         * @see Communicator#destroy
         */
        virtual void destroy() noexcept = 0;

        /**
         * Add a middleware to the dispatch pipeline of this object adapter.
         * @param middlewareFactory The middleware factory that creates the new middleware when this object adapter
         * creates its dispatch pipeline. A middleware factory is a function that takes an ObjectPtr (the next element
         * in the dispatch pipeline) and returns a new ObjectPtr (the middleware you want to install in the pipeline).
         * @return This object adapter.
         * @remark All middleware must be installed before the first dispatch.
         * @remark The middleware are executed in the order they are installed.
         */
        virtual ObjectAdapterPtr use(std::function<ObjectPtr(ObjectPtr)> middlewareFactory) = 0;

        /**
         * Add a servant to this object adapter's Active Servant Map. Note that one servant can implement several Ice
         * objects by registering the servant with multiple identities. Adding a servant with an identity that is in the
         * map already throws {@link AlreadyRegisteredException}.
         * @param servant The servant to add.
         * @param id The identity of the Ice object that is implemented by the servant.
         * @return A proxy that matches the given identity and this object adapter.
         * @see Identity
         * @see #addFacet
         * @see #addWithUUID
         * @see #remove
         * @see #find
         */
        template<typename Prx = ObjectPrx, std::enable_if_t<std::is_base_of<ObjectPrx, Prx>::value, bool> = true>
        Prx add(const ObjectPtr& servant, const Identity& id)
        {
            return uncheckedCast<Prx>(_add(servant, id));
        }

        /**
         * Like {@link #add}, but with a facet. Calling <code>add(servant, id)</code> is equivalent to calling
         * {@link #addFacet} with an empty facet.
         * @param servant The servant to add.
         * @param id The identity of the Ice object that is implemented by the servant.
         * @param facet The facet. An empty facet means the default facet.
         * @return A proxy that matches the given identity, facet, and this object adapter.
         * @see Identity
         * @see #add
         * @see #addFacetWithUUID
         * @see #removeFacet
         * @see #findFacet
         */
        template<typename Prx = ObjectPrx, std::enable_if_t<std::is_base_of<ObjectPrx, Prx>::value, bool> = true>
        Prx addFacet(ObjectPtr servant, Identity id, std::string facet)
        {
            return uncheckedCast<Prx>(_addFacet(std::move(servant), std::move(id), std::move(facet)));
        }

        /**
         * Add a servant to this object adapter's Active Servant Map, using an automatically generated UUID as its
         * identity. Note that the generated UUID identity can be accessed using the proxy's
         * <code>ice_getIdentity</code> operation.
         * @param servant The servant to add.
         * @return A proxy that matches the generated UUID identity and this object adapter.
         * @see Identity
         * @see #add
         * @see #addFacetWithUUID
         * @see #remove
         * @see #find
         */
        template<typename Prx = ObjectPrx, std::enable_if_t<std::is_base_of<ObjectPrx, Prx>::value, bool> = true>
        Prx addWithUUID(ObjectPtr servant)
        {
            return uncheckedCast<Prx>(_addWithUUID(std::move(servant)));
        }

        /**
         * Like {@link #addWithUUID}, but with a facet. Calling <code>addWithUUID(servant)</code> is equivalent to
         * calling
         * {@link #addFacetWithUUID} with an empty facet.
         * @param servant The servant to add.
         * @param facet The facet. An empty facet means the default facet.
         * @return A proxy that matches the generated UUID identity, facet, and this object adapter.
         * @see Identity
         * @see #addFacet
         * @see #addWithUUID
         * @see #removeFacet
         * @see #findFacet
         */
        template<typename Prx = ObjectPrx, std::enable_if_t<std::is_base_of<ObjectPrx, Prx>::value, bool> = true>
        Prx addFacetWithUUID(ObjectPtr servant, std::string facet)
        {
            return uncheckedCast<Prx>(_addFacetWithUUID(std::move(servant), std::move(facet)));
        }

        /**
         * Add a default servant to handle requests for a specific category. Adding a default servant for a category for
         * which a default servant is already registered throws {@link AlreadyRegisteredException}. To dispatch
         * operation calls on servants, the object adapter tries to find a servant for a given Ice object identity and
         * facet in the following order: <ol> <li>The object adapter tries to find a servant for the identity and facet
         * in the Active Servant Map.</li> <li>If no servant has been found in the Active Servant Map, the object
         * adapter tries to find a default servant for the category component of the identity.</li> <li>If no servant
         * has been found by any of the preceding steps, the object adapter tries to find a default servant for an empty
         * category, regardless of the category contained in the identity.</li> <li>If no servant has been found by any
         * of the preceding steps, the object adapter gives up and the caller receives {@link ObjectNotExistException}
         * or {@link FacetNotExistException}.</li>
         * </ol>
         * @param servant The default servant.
         * @param category The category for which the default servant is registered. An empty category means it will
         * handle all categories.
         * @see #removeDefaultServant
         * @see #findDefaultServant
         */
        virtual void addDefaultServant(ObjectPtr servant, std::string category) = 0;

        /**
         * Remove a servant (that is, the default facet) from the object adapter's Active Servant Map.
         * @param id The identity of the Ice object that is implemented by the servant. If the servant implements
         * multiple Ice objects, {@link #remove} has to be called for all those Ice objects. Removing an identity that
         * is not in the map throws {@link NotRegisteredException}.
         * @return The removed servant.
         * @see Identity
         * @see #add
         * @see #addWithUUID
         */
        virtual ObjectPtr remove(const Identity& id) = 0;

        /**
         * Like {@link #remove}, but with a facet. Calling <code>remove(id)</code> is equivalent to calling
         * {@link #removeFacet} with an empty facet.
         * @param id The identity of the Ice object that is implemented by the servant.
         * @param facet The facet. An empty facet means the default facet.
         * @return The removed servant.
         * @see Identity
         * @see #addFacet
         * @see #addFacetWithUUID
         */
        virtual ObjectPtr removeFacet(const Identity& id, std::string_view facet) = 0;

        /**
         * Remove all facets with the given identity from the Active Servant Map. The operation completely removes the
         * Ice object, including its default facet. Removing an identity that is not in the map throws
         * {@link NotRegisteredException}.
         * @param id The identity of the Ice object to be removed.
         * @return A collection containing all the facet names and servants of the removed Ice object.
         * @see #remove
         * @see #removeFacet
         */
        virtual FacetMap removeAllFacets(const Identity& id) = 0;

        /**
         * Remove the default servant for a specific category. Attempting to remove a default servant for a category
         * that is not registered throws {@link NotRegisteredException}.
         * @param category The category of the default servant to remove.
         * @return The default servant.
         * @see #addDefaultServant
         * @see #findDefaultServant
         */
        virtual ObjectPtr removeDefaultServant(std::string_view category) = 0;

        /**
         * Look up a servant in this object adapter's Active Servant Map by the identity of the Ice object it
         * implements. <p class="Note">This operation only tries to look up a servant in the Active Servant Map. It does
         * not attempt to find a servant by using any installed {@link ServantLocator}.
         * @param id The identity of the Ice object for which the servant should be returned.
         * @return The servant that implements the Ice object with the given identity, or null if no such servant has
         * been found.
         * @see Identity
         * @see #findFacet
         * @see #findByProxy
         */
        [[nodiscard]] virtual ObjectPtr find(const Identity& id) const = 0;

        /**
         * Like {@link #find}, but with a facet. Calling <code>find(id)</code> is equivalent to calling {@link
         * #findFacet} with an empty facet.
         * @param id The identity of the Ice object for which the servant should be returned.
         * @param facet The facet. An empty facet means the default facet.
         * @return The servant that implements the Ice object with the given identity and facet, or null if no such
         * servant has been found.
         * @see Identity
         * @see #find
         * @see #findByProxy
         */
        [[nodiscard]] virtual ObjectPtr findFacet(const Identity& id, std::string_view facet) const = 0;

        /**
         * Find all facets with the given identity in the Active Servant Map.
         * @param id The identity of the Ice object for which the facets should be returned.
         * @return A collection containing all the facet names and servants that have been found, or an empty map if
         * there is no facet for the given identity.
         * @see #find
         * @see #findFacet
         */
        [[nodiscard]] virtual FacetMap findAllFacets(const Identity& id) const = 0;

        /**
         * Look up a servant in this object adapter's Active Servant Map, given a proxy.
         * <p class="Note">This operation only tries to lookup a servant in the Active Servant Map. It does not attempt
         * to find a servant by using any installed {@link ServantLocator}.
         * @param proxy The proxy for which the servant should be returned.
         * @return The servant that matches the proxy, or null if no such servant has been found.
         * @see #find
         * @see #findFacet
         */
        [[nodiscard]] virtual ObjectPtr findByProxy(const ObjectPrx& proxy) const = 0;

        /**
         * Add a Servant Locator to this object adapter. Adding a servant locator for a category for which a servant
         * locator is already registered throws {@link AlreadyRegisteredException}. To dispatch operation calls on
         * servants, the object adapter tries to find a servant for a given Ice object identity and facet in the
         * following order: <ol> <li>The object adapter tries to find a servant for the identity and facet in the Active
         * Servant Map.</li> <li>If no servant has been found in the Active Servant Map, the object adapter tries to
         * find a servant locator for the category component of the identity. If a locator is found, the object adapter
         * tries to find a servant using this locator.</li> <li>If no servant has been found by any of the preceding
         * steps, the object adapter tries to find a locator for an empty category, regardless of the category contained
         * in the identity. If a locator is found, the object adapter tries to find a servant using this locator.</li>
         * <li>If no servant has been found by any of the preceding steps, the object adapter gives up and the caller
         * receives {@link ObjectNotExistException} or {@link FacetNotExistException}.</li>
         * </ol>
         * <p class="Note">Only one locator for the empty category can be installed.
         * @param locator The locator to add.
         * @param category The category for which the Servant Locator can locate servants, or an empty string if the
         * Servant Locator does not belong to any specific category.
         * @see Identity
         * @see #removeServantLocator
         * @see #findServantLocator
         * @see ServantLocator
         */
        virtual void addServantLocator(ServantLocatorPtr locator, std::string category) = 0;

        /**
         * Remove a Servant Locator from this object adapter.
         * @param category The category for which the Servant Locator can locate servants, or an empty string if the
         * Servant Locator does not belong to any specific category.
         * @return The Servant Locator, or throws {@link NotRegisteredException} if no Servant Locator was found for the
         * given category.
         * @see Identity
         * @see #addServantLocator
         * @see #findServantLocator
         * @see ServantLocator
         */
        virtual ServantLocatorPtr removeServantLocator(std::string_view category) = 0;

        /**
         * Find a Servant Locator installed with this object adapter.
         * @param category The category for which the Servant Locator can locate servants, or an empty string if the
         * Servant Locator does not belong to any specific category.
         * @return The Servant Locator, or null if no Servant Locator was found for the given category.
         * @see Identity
         * @see #addServantLocator
         * @see #removeServantLocator
         * @see ServantLocator
         */
        [[nodiscard]] virtual ServantLocatorPtr findServantLocator(std::string_view category) const = 0;

        /**
         * Find the default servant for a specific category.
         * @param category The category of the default servant to find.
         * @return The default servant or null if no default servant was registered for the category.
         * @see #addDefaultServant
         * @see #removeDefaultServant
         */
        [[nodiscard]] virtual ObjectPtr findDefaultServant(std::string_view category) const = 0;

        /**
         * Get the dispatch pipeline of this object adapter.
         * @return The dispatch pipeline. This shared_ptr is never null.
         */
        [[nodiscard]] virtual const ObjectPtr& dispatchPipeline() const noexcept = 0;

        /**
         * Create a proxy for the object with the given identity. If this object adapter is configured with an adapter
         * id, the return value is an indirect proxy that refers to the adapter id. If a replica group id is also
         * defined, the return value is an indirect proxy that refers to the replica group id. Otherwise, if no adapter
         * id is defined, the return value is a direct proxy containing this object adapter's published endpoints.
         * @param id The object's identity.
         * @return A proxy for the object with the given identity.
         * @see Identity
         */
        template<typename Prx = ObjectPrx, std::enable_if_t<std::is_base_of<ObjectPrx, Prx>::value, bool> = true>
        Prx createProxy(Identity id)
        {
            return uncheckedCast<Prx>(_createProxy(std::move(id)));
        }

        /**
         * Create a direct proxy for the object with the given identity. The returned proxy contains this object
         * adapter's published endpoints.
         * @param id The object's identity.
         * @return A proxy for the object with the given identity.
         * @see Identity
         */
        template<typename Prx = ObjectPrx, std::enable_if_t<std::is_base_of<ObjectPrx, Prx>::value, bool> = true>
        Prx createDirectProxy(Identity id)
        {
            return uncheckedCast<Prx>(_createDirectProxy(std::move(id)));
        }

        /**
         * Create an indirect proxy for the object with the given identity. If this object adapter is configured with an
         * adapter id, the return value refers to the adapter id. Otherwise, the return value contains only the object
         * identity.
         * @param id The object's identity.
         * @return A proxy for the object with the given identity.
         * @see Identity
         */
        template<typename Prx = ObjectPrx, std::enable_if_t<std::is_base_of<ObjectPrx, Prx>::value, bool> = true>
        Prx createIndirectProxy(Identity id)
        {
            return uncheckedCast<Prx>(_createIndirectProxy(std::move(id)));
        }

        /**
         * Set an Ice locator for this object adapter. By doing so, the object adapter will register itself with the
         * locator registry when it is activated for the first time. Furthermore, the proxies created by this object
         * adapter will contain the adapter identifier instead of its endpoints. The adapter identifier must be
         * configured using the AdapterId property.
         * @param loc The locator used by this object adapter.
         * @see #createDirectProxy
         * @see Locator
         * @see LocatorRegistry
         */
        virtual void setLocator(std::optional<LocatorPrx> loc) = 0;

        /**
         * Get the Ice locator used by this object adapter.
         * @return The locator used by this object adapter, or null if no locator is used by this object adapter.
         * @see Locator
         * @see #setLocator
         */
        [[nodiscard]] virtual std::optional<LocatorPrx> getLocator() const noexcept = 0;

        /**
         * Get the set of endpoints configured with this object adapter.
         * @return The set of endpoints.
         * @see Endpoint
         */
        [[nodiscard]] virtual EndpointSeq getEndpoints() const noexcept = 0;

        /**
         * Get the set of endpoints that proxies created by this object adapter will contain.
         * @return The set of published endpoints.
         * @see Endpoint
         */
        [[nodiscard]] virtual EndpointSeq getPublishedEndpoints() const noexcept = 0;

        /**
         * Set of the endpoints that proxies created by this object adapter will contain.
         * @param newEndpoints The new set of endpoints that the object adapter will embed in proxies.
         * @see Endpoint
         */
        virtual void setPublishedEndpoints(EndpointSeq newEndpoints) = 0;

    protected:
        virtual ObjectPrx _add(ObjectPtr servant, Identity id) = 0;
        virtual ObjectPrx _addFacet(ObjectPtr servant, Identity id, std::string facet) = 0;
        virtual ObjectPrx _addWithUUID(ObjectPtr servant) = 0;
        virtual ObjectPrx _addFacetWithUUID(ObjectPtr servant, std::string facet) = 0;
        [[nodiscard]] virtual ObjectPrx _createProxy(Identity id) const = 0;
        [[nodiscard]] virtual ObjectPrx _createDirectProxy(Identity id) const = 0;
        [[nodiscard]] virtual ObjectPrx _createIndirectProxy(Identity id) const = 0;
    };
}

#endif
