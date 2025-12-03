// Copyright (c) ZeroC, Inc.

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

    /// An object adapter is the main server-side Ice API. It has two main purposes:
    /// - accept incoming connections from clients and dispatch requests received over these connections (see
    ///   #activate); and
    /// - maintain a dispatch pipeline and servants that handle the requests (see #add, #addDefaultServant, and #use).
    ///
    /// An object adapter can dispatch "bidirectional requests"--requests it receives over an outgoing connection
    /// instead of a more common incoming connection. It can also dispatch collocated requests (with no connection at
    /// all).
    /// @see Communicator::createObjectAdapter
    /// @headerfile Ice/Ice.h
    class ICE_API ObjectAdapter
    {
    public:
        virtual ~ObjectAdapter();

        /// Gets the name of this object adapter.
        /// @return This object adapter's name.
        [[nodiscard]] virtual const std::string& getName() const noexcept = 0;

        /// Gets the communicator that created this object adapter.
        /// @return This object adapter's communicator.
        [[nodiscard]] virtual CommunicatorPtr getCommunicator() const noexcept = 0;

        /// Starts receiving and dispatching requests received over incoming connections.
        /// @remark When this object adapter is an indirect object adapter configured with a locator proxy, this
        /// function also registers the object adapter's published endpoints with this locator.
        /// @see #deactivate
        /// @see #getLocator
        /// @see #getPublishedEndpoints
        virtual void activate() = 0;

        /// Stops reading requests from incoming connections. Outstanding dispatches are not affected. The object
        /// adapter can be reactivated with #activate.
        /// @remark This function is provided for backward compatibility with older versions of Ice. Don't use it in
        /// new applications.
        virtual void hold() = 0;

        /// Waits until the object adapter is in the holding state (see #hold) and the dispatch of requests received
        /// over incoming connection has completed.
        /// @remark This function is provided for backward compatibility with older versions of Ice. Don't use it in
        /// new applications.
        virtual void waitForHold() = 0;

        /// Deactivates this object adapter: stops accepting new connections from clients and closes gracefully all
        /// incoming connections created by this object adapter once all outstanding dispatches have completed. If this
        /// object adapter is indirect, this function also unregisters the object adapter from the locator
        /// (see #activate).
        /// This function does not cancel outstanding dispatches: it lets them execute until completion.
        /// A deactivated object adapter cannot be reactivated again; it can only be destroyed.
        /// @see #waitForDeactivate
        /// @see Communicator#shutdown
        virtual void deactivate() noexcept = 0;

        /// Waits until #deactivate is called on this object adapter and all connections accepted by this object adapter
        /// are closed. A connection is closed only after all outstanding dispatches on this connection have completed.
        /// @see Communicator#waitForShutdown
        virtual void waitForDeactivate() noexcept = 0;

        /// Checks whether or not #deactivate was called on this object adapter.
        /// @return `true` if #deactivate was called on this object adapter, `false` otherwise.
        [[nodiscard]] virtual bool isDeactivated() const noexcept = 0;

        /// Destroys this object adapter and cleans up all resources associated with it. Once this function has
        /// returned, you can recreate another object adapter with the same name.
        /// @see Communicator#destroy
        virtual void destroy() noexcept = 0;

        /// Adds a middleware to the dispatch pipeline of this object adapter.
        /// @param middlewareFactory The middleware factory that creates the new middleware when this object adapter
        /// creates its dispatch pipeline. A middleware factory is a function that takes an ObjectPtr (the next element
        /// in the dispatch pipeline) and returns a new ObjectPtr (the middleware you want to install in the pipeline).
        /// @return This object adapter.
        /// @remark All middleware must be installed before the first dispatch.
        /// @remark The middleware are executed in the order they are installed.
        virtual ObjectAdapterPtr use(std::function<ObjectPtr(ObjectPtr)> middlewareFactory) = 0;

        /// Adds a servant to this object adapter's Active Servant Map (ASM).
        /// The ASM is a map {identity, facet} -> servant.
        /// @tparam Prx The type of the proxy to return.
        /// @param servant The servant to add.
        /// @param id The identity of the Ice object that is implemented by the servant.
        /// @return A proxy for @p id, created by this object adapter.
        /// @throws AlreadyRegisteredException Thrown when a servant with the same identity is already registered.
        /// @remark This function is equivalent to calling #addFacet with an empty facet.
        template<typename Prx = ObjectPrx, std::enable_if_t<std::is_base_of_v<ObjectPrx, Prx>, bool> = true>
        Prx add(const ObjectPtr& servant, const Identity& id)
        {
            return uncheckedCast<Prx>(_add(servant, id));
        }

        /// Adds a servant to this object adapter's Active Servant Map (ASM), while specifying a facet.
        /// The ASM is a map {identity, facet} -> servant.
        /// @tparam Prx The type of the proxy to return.
        /// @param servant The servant to add.
        /// @param id The identity of the Ice object that is implemented by the servant.
        /// @param facet The facet of the Ice object that is implemented by the servant.
        /// @return A proxy for @p id and @p facet, created by this object adapter.
        /// @throws AlreadyRegisteredException Thrown when a servant with the same identity and facet is already
        /// registered.
        template<typename Prx = ObjectPrx, std::enable_if_t<std::is_base_of_v<ObjectPrx, Prx>, bool> = true>
        Prx addFacet(ObjectPtr servant, Identity id, std::string facet)
        {
            return uncheckedCast<Prx>(_addFacet(std::move(servant), std::move(id), std::move(facet)));
        }

        /// Adds a servant to this object adapter's Active Servant Map (ASM), using an automatically generated UUID as
        /// its identity.
        /// @tparam Prx The type of the proxy to return.
        /// @param servant The servant to add.
        /// @return A proxy with the generated UUID identity created by this object adapter.
        template<typename Prx = ObjectPrx, std::enable_if_t<std::is_base_of_v<ObjectPrx, Prx>, bool> = true>
        [[nodiscard]] Prx addWithUUID(ObjectPtr servant)
        {
            return uncheckedCast<Prx>(_addWithUUID(std::move(servant)));
        }

        /// Adds a servant to this object adapter's Active Servant Map (ASM), using an automatically generated UUID as
        /// its identity. Also specifies a facet.
        /// @tparam Prx The type of the proxy to return.
        /// @param servant The servant to add.
        /// @param facet The facet of the Ice object that is implemented by the servant.
        /// @return A proxy with the generated UUID identity and the specified facet.
        template<typename Prx = ObjectPrx, std::enable_if_t<std::is_base_of_v<ObjectPrx, Prx>, bool> = true>
        [[nodiscard]] Prx addFacetWithUUID(ObjectPtr servant, std::string facet)
        {
            return uncheckedCast<Prx>(_addFacetWithUUID(std::move(servant), std::move(facet)));
        }

        /// Adds a default servant to handle requests for a specific category. When an object adapter dispatches an
        /// incoming request, it tries to find a servant for the identity and facet carried by the request in the
        /// following order:
        ///  - The object adapter tries to find a servant for the identity and facet in the Active Servant Map.
        ///  - If this fails, the object adapter tries to find a default servant for the category component of the
        ///    identity.
        ///  - If this fails, the object adapter tries to find a default servant for the empty category, regardless of
        ///    the category contained in the identity.
        ///  - If this fails, the object adapter tries to find a servant locator for the category component of the
        ///    identity. If there is no such servant locator, the object adapter tries to find a servant locator for the
        ///    empty category.
        ///    - If a servant locator is found, the object adapter tries to find a servant using this servant locator.
        ///  - If all the previous steps fail, the object adapter gives up and the caller receives an
        ///    ObjectNotExistException or a FacetNotExistException.
        /// @param servant The default servant to add.
        /// @param category The category for which the default servant is registered. The empty category means it
        /// handles all categories.
        /// @throws AlreadyRegisteredException Thrown when a default servant with the same category is already
        /// registered.
        virtual void addDefaultServant(ObjectPtr servant, std::string category) = 0;

        /// Removes a servant from the object adapter's Active Servant Map.
        /// @param id The identity of the Ice object that is implemented by the servant.
        /// @return The removed servant.
        /// @throws NotRegisteredException Thrown when no servant with the given identity is registered.
        virtual ObjectPtr remove(const Identity& id) = 0;

        /// Removes a servant from the object adapter's Active Servant Map, while specifying a facet.
        /// @param id The identity of the Ice object that is implemented by the servant.
        /// @param facet The facet. An empty facet means the default facet.
        /// @return The removed servant.
        /// @throws NotRegisteredException Thrown when no servant with the given identity and facet is registered.
        virtual ObjectPtr removeFacet(const Identity& id, std::string_view facet) = 0;

        /// Removes all facets with the given identity from the Active Servant Map. The function completely removes the
        /// Ice object, including its default facet.
        /// @param id The identity of the Ice object to be removed.
        /// @return A collection containing all the facet names and servants of the removed Ice object.
        /// @throws NotRegisteredException Thrown when no servant with the given identity is registered.
        virtual FacetMap removeAllFacets(const Identity& id) = 0;

        /// Removes the default servant for a specific category.
        /// @param category The category of the default servant to remove.
        /// @return The default servant.
        /// @throws NotRegisteredException Thrown when no default servant is registered for the given category.
        virtual ObjectPtr removeDefaultServant(std::string_view category) = 0;

        /// Looks up a servant.
        /// @param id The identity of an Ice object.
        /// @return The servant that implements the Ice object with the given identity,
        /// or nullptr if no such servant has been found.
        /// @remark This function only tries to find the servant in the ASM and among the default servants. It does not
        /// attempt to locate a servant using servant locators.
        [[nodiscard]] virtual ObjectPtr find(const Identity& id) const = 0;

        /// Looks up a servant with an identity and facet.
        /// @param id The identity of an Ice object.
        /// @param facet The facet of an Ice object. An empty facet means the default facet.
        /// @return The servant that implements the Ice object with the given identity and facet,
        /// or nullptr if no such servant has been found.
        /// @remark This function only tries to find the servant in the ASM and among the default servants. It does not
        /// attempt to locate a servant using servant locators.
        [[nodiscard]] virtual ObjectPtr findFacet(const Identity& id, std::string_view facet) const = 0;

        /// Finds all facets for a given identity in the Active Servant Map.
        /// @param id The identity.
        /// @return A collection containing all the facet names and servants that have been found. Can be empty.
        [[nodiscard]] virtual FacetMap findAllFacets(const Identity& id) const = 0;

        /// Looks up a servant with an identity and a facet. It's equivalent to calling #findFacet.
        /// @param proxy The proxy that provides the identity and facet to search.
        /// @return The servant that matches the identity and facet carried by @p proxy, or nullptr if no such servant
        /// has been found.
        [[nodiscard]] virtual ObjectPtr findByProxy(const ObjectPrx& proxy) const = 0;

        /// Adds a ServantLocator to this object adapter for a specific category.
        /// @param locator The servant locator to add.
        /// @param category The category. The empty category means @p locator handles all categories.
        /// @throws AlreadyRegisteredException Thrown when a servant locator with the same category is already
        /// registered.
        /// @see #addDefaultServant
        virtual void addServantLocator(ServantLocatorPtr locator, std::string category) = 0;

        /// Removes a ServantLocator from this object adapter.
        /// @param category The category.
        /// @return The servant locator.
        /// @throws NotRegisteredException Thrown when no ServantLocator with the given category is registered.
        virtual ServantLocatorPtr removeServantLocator(std::string_view category) = 0;

        /// Finds a ServantLocator registered with this object adapter.
        /// @param category The category.
        /// @return The servant locator, or nullptr if not found.
        [[nodiscard]] virtual ServantLocatorPtr findServantLocator(std::string_view category) const = 0;

        /// Finds the default servant for a specific category.
        /// @param category The category.
        /// @return The default servant, or nullptr if not found.
        [[nodiscard]] virtual ObjectPtr findDefaultServant(std::string_view category) const = 0;

        /// Gets the dispatch pipeline of this object adapter.
        /// @return The dispatch pipeline. The returned value is never nullptr.
        [[nodiscard]] virtual const ObjectPtr& dispatchPipeline() const noexcept = 0;

        /// Creates a proxy from an Ice identity. If this object adapter is configured with an adapter ID, the proxy
        /// is an indirect proxy that refers to this adapter ID. If a replica group ID is also defined, the proxy is an
        /// indirect proxy that refers to this replica group ID. Otherwise, the proxy is a direct proxy containing this
        /// object adapter's published endpoints.
        /// @tparam Prx The type of the proxy to return.
        /// @param id An Ice identity.
        /// @return A proxy with the given identity.
        template<typename Prx = ObjectPrx, std::enable_if_t<std::is_base_of_v<ObjectPrx, Prx>, bool> = true>
        Prx createProxy(Identity id)
        {
            return uncheckedCast<Prx>(_createProxy(std::move(id)));
        }

        /// Creates a direct proxy from an Ice identity.
        /// @tparam Prx The type of the proxy to return.
        /// @param id An Ice identity.
        /// @return A proxy with the given identity and this published endpoints of this object adapter.
        template<typename Prx = ObjectPrx, std::enable_if_t<std::is_base_of_v<ObjectPrx, Prx>, bool> = true>
        Prx createDirectProxy(Identity id)
        {
            return uncheckedCast<Prx>(_createDirectProxy(std::move(id)));
        }

        /// Creates an indirect proxy for an Ice identity.
        /// @tparam Prx The type of the proxy to return.
        /// @param id An Ice identity.
        /// @return An indirect proxy with the given identity. If this object adapter is not configured with an adapter
        /// ID or a replica group ID, the new proxy is a well-known proxy (i.e., an identity-only proxy).
        template<typename Prx = ObjectPrx, std::enable_if_t<std::is_base_of_v<ObjectPrx, Prx>, bool> = true>
        Prx createIndirectProxy(Identity id)
        {
            return uncheckedCast<Prx>(_createIndirectProxy(std::move(id)));
        }

        /// Sets an Ice locator on this object adapter.
        /// @param loc The locator used by this object adapter.
        virtual void setLocator(std::optional<LocatorPrx> loc) = 0;

        /// Gets the Ice locator used by this object adapter.
        /// @return The locator used by this object adapter, or nullptr if no locator is used by this object adapter.
        [[nodiscard]] virtual std::optional<LocatorPrx> getLocator() const noexcept = 0;

        /// Gets the set of endpoints configured on this object adapter.
        /// @return The set of endpoints.
        /// @remark This function remains usable after the object adapter has been deactivated.
        [[nodiscard]] virtual EndpointSeq getEndpoints() const = 0;

        /// Gets the set of endpoints that proxies created by this object adapter will contain.
        /// @return The set of published endpoints.
        /// @remark This function remains usable after the object adapter has been deactivated.
        [[nodiscard]] virtual EndpointSeq getPublishedEndpoints() const = 0;

        /// Sets the endpoints that proxies created by this object adapter will contain.
        /// @param newEndpoints The new set of endpoints that the object adapter will embed in proxies.
        /// @throws invalid_argument Thrown when @p newEndpoints is empty or this adapter is associated with a router.
        virtual void setPublishedEndpoints(EndpointSeq newEndpoints) = 0;

    protected:
        /// @cond INTERNAL
        virtual ObjectPrx _add(ObjectPtr servant, Identity id) = 0;
        virtual ObjectPrx _addFacet(ObjectPtr servant, Identity id, std::string facet) = 0;
        virtual ObjectPrx _addWithUUID(ObjectPtr servant) = 0;
        virtual ObjectPrx _addFacetWithUUID(ObjectPtr servant, std::string facet) = 0;
        [[nodiscard]] virtual ObjectPrx _createProxy(Identity id) const = 0;
        [[nodiscard]] virtual ObjectPrx _createDirectProxy(Identity id) const = 0;
        [[nodiscard]] virtual ObjectPrx _createIndirectProxy(Identity id) const = 0;
        /// @endcond
    };
}

#endif
