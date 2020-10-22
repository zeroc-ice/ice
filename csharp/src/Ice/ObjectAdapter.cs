// Copyright (c) ZeroC, Inc. All rights reserved.

using System;
using System.Collections.Generic;
using System.Collections.Immutable;
using System.Diagnostics;
using System.Linq;
using System.Text;
using System.Threading;
using System.Threading.Tasks;

using ZeroC.Ice.Instrumentation;

namespace ZeroC.Ice
{
    /// <summary>The object adapter provides an up-call interface from the Ice run time to the implementation of Ice
    /// objects. The object adapter is responsible for receiving requests from endpoints, and for mapping between
    /// servants, identities, and proxies.</summary>
    public sealed class ObjectAdapter : IDisposable, IAsyncDisposable
    {
        /// <summary>Returns the communicator that created this object adapter.</summary>
        /// <value>The communicator.</value>
        public Communicator Communicator { get; }

        /// <summary>The Ice Locator associated with this object adapter, if any. The object adapter registers itself
        /// with this locator during <see cref="ActivateAsync"/>.</summary>
        /// <value>The locator proxy.</value>
        public ILocatorPrx? Locator
        {
            get => _locatorInfo?.Locator;
            set => _locatorInfo = Communicator.GetLocatorInfo(value);
        }

        /// <summary>Returns the name of this object adapter. This name is used as prefix for the object adapter's
        /// configuration properties.</summary>
        /// <value>The object adapter's name.</value>
        public string Name { get; }

        /// <summary>Gets the protocol of this object adapter. The format of this object adapter's Endpoints property
        /// determines this protocol.</summary>
        public Protocol Protocol { get; }

        /// <summary>Indicates whether or not this object adapter serializes the dispatching of requests received
        /// over the same connection.</summary>
        /// <value>The serialize dispatch value.</value>
        public bool SerializeDispatch { get; }

        /// <summary>Returns the TaskScheduler used to dispatch requests.</summary>
        public TaskScheduler? TaskScheduler { get; }

        internal int IncomingFrameSizeMax { get; }

        internal List<DispatchInterceptor> Interceptors { get; } = new List<DispatchInterceptor>();

        private static readonly string[] _suffixes =
        {
            "AdapterId",
            "Endpoints",
            "IncomingFrameSizeMax",
            "Locator",
            "Locator.Encoding",
            "Locator.EndpointSelection",
            "Locator.ConnectionCached",
            "Locator.PreferNonSecure",
            "Locator.Router",
            "PublishedEndpoints",
            "ReplicaGroupId",
            "Router",
            "Router.Encoding",
            "Router.EndpointSelection",
            "Router.ConnectionCached",
            "Router.PreferNonSecure",
            "Router.Locator",
            "Router.Locator.EndpointSelection",
            "Router.Locator.ConnectionCached",
            "Router.Locator.PreferNonSecure",
            "Router.Locator.LocatorCacheTimeout",
            "Router.Locator.InvocationTimeout",
            "Router.LocatorCacheTimeout",
            "Router.InvocationTimeout",
            "ProxyOptions"
        };

        private Task? _activateTask;
        private readonly Dictionary<CategoryPlusFacet, IObject> _categoryServantMap =
            new Dictionary<CategoryPlusFacet, IObject>();
        private AcceptorIncomingConnectionFactory? _colocatedConnectionFactory;
        private readonly Dictionary<string, IObject> _defaultServantMap = new Dictionary<string, IObject>();
        private Task? _disposeTask;
        private readonly string _id; // adapter id
        private readonly Dictionary<IdentityPlusFacet, IObject> _identityServantMap =
            new Dictionary<IdentityPlusFacet, IObject>();
        private readonly List<IncomingConnectionFactory> _incomingConnectionFactories =
            new List<IncomingConnectionFactory>();
        private readonly InvocationMode _invocationMode = InvocationMode.Twoway;
        private volatile LocatorInfo? _locatorInfo;
        private readonly object _mutex = new object();
        private IReadOnlyList<Endpoint> _publishedEndpoints;
        private readonly string _replicaGroupId;
        private readonly RouterInfo? _routerInfo;

        /// <summary>Activates all endpoints of this object adapter. After activation, the object adapter can dispatch
        /// requests received through these endpoints. Activate also registers this object adapter with the locator (if
        /// set).</summary>
        /// <param name="interceptors">The dispatch interceptors to register with the object adapter.</param>
        public void Activate(params DispatchInterceptor[] interceptors)
        {
            try
            {
                ActivateAsync(interceptors).Wait();
            }
            catch (AggregateException ex)
            {
                Debug.Assert(ex.InnerException != null);
                throw ExceptionUtil.Throw(ex.InnerException);
            }
        }

        /// <summary>Activates all endpoints of this object adapter. After activation, the object adapter can dispatch
        /// requests received through these endpoints. ActivateAsync also registers this object adapter with the
        /// locator (if set).</summary>
        /// <param name="interceptors">The dispatch interceptors to register with the object adapter.</param>
        public async Task ActivateAsync(params DispatchInterceptor[] interceptors)
        {
            lock (_mutex)
            {
                if (_disposeTask != null)
                {
                    throw new ObjectDisposedException($"{typeof(ObjectAdapter).FullName}:{Name}");
                }

                // Activating twice the object adapter is incorrect
                if (_activateTask != null)
                {
                    throw new InvalidOperationException($"object adapter {Name} already activated");
                }

                Interceptors.AddRange(Communicator.DispatchInterceptors);
                Interceptors.AddRange(interceptors);

                // Activate the incoming connection factories to start accepting connections
                foreach (IncomingConnectionFactory factory in _incomingConnectionFactories)
                {
                    factory.Activate();
                }

                _activateTask ??= UpdateLocatorRegistryAsync(_locatorInfo,
                                                             CreateDirectProxy(new Identity("dummy", ""),
                                                             IObjectPrx.Factory));
            }
            await _activateTask.ConfigureAwait(false);

            if ((Communicator.GetPropertyAsBool("Ice.PrintAdapterReady") ?? false) && Name.Length > 0)
            {
                Console.Out.WriteLine($"{Name} ready");
            }
        }

        /// <summary>Releases resources used by the object adapter.</summary>
        public void Dispose() => DisposeAsync().AsTask().Wait();

        /// <summary>Releases resources used by the object adapter.</summary>
        public async ValueTask DisposeAsync()
        {
            lock (_mutex)
            {
                _disposeTask ??= PerformDisposeAsync();
            }
            await _disposeTask.ConfigureAwait(false);

            async Task PerformDisposeAsync()
            {
                // Synchronously Dispose of the incoming connection factories to stop accepting new incoming requests
                // or connections. This ensures that once DisposeAsync returns, no new requests will be dispatched.
                // Calling ToArray is important here to ensure that all the DisposeAsync calls are executed before we
                // eventually hit an await (we want to make that once DisposeAsync returns a Task, all the connections
                // started closing).
                Task[] tasks =
                    _incomingConnectionFactories.Select(factory => factory.DisposeAsync().AsTask()).ToArray();

                // Wait for activation to complete. This is necessary avoid out of order locator updates.
                if (_activateTask != null)
                {
                    try
                    {
                        await _activateTask.ConfigureAwait(false);
                    }
                    catch
                    {
                        // Ignore
                    }
                }

                try
                {

                    await UpdateLocatorRegistryAsync(_locatorInfo, null).ConfigureAwait(false);
                }
                catch
                {
                    // We can't throw exceptions in deactivate so we ignore failures to update the locator registry.
                }

                if (_colocatedConnectionFactory != null)
                {
                    await _colocatedConnectionFactory.DisposeAsync().ConfigureAwait(false);
                }

                // Wait for the incoming connection factories to be disposed.
                await Task.WhenAll(tasks).ConfigureAwait(false);

                Communicator.OutgoingConnectionFactory.RemoveAdapter(this);
                Communicator.EraseRouterInfo(_routerInfo?.Router);
                Communicator.RemoveObjectAdapter(this);
            }
        }

        /// <summary>Finds a servant in the Active Servant Map (ASM), taking into account the servants and default
        /// servants currently in the ASM.</summary>
        /// <param name="identity">The identity of the Ice object.</param>
        /// <param name="facet">The facet of the Ice object.</param>
        /// <returns>The corresponding servant in the ASM, or null if the servant was not found.</returns>
        public IObject? Find(Identity identity, string facet = "")
        {
            lock (_mutex)
            {
                if (!_identityServantMap.TryGetValue(new IdentityPlusFacet(identity, facet), out IObject? servant))
                {
                    if (!_categoryServantMap.TryGetValue(new CategoryPlusFacet(identity.Category, facet), out servant))
                    {
                        _defaultServantMap.TryGetValue(facet, out servant);
                    }
                }
                return servant;
            }
        }

        /// <summary>Finds a servant in the Active Servant Map (ASM), taking into account the servants and default
        /// servants currently in the ASM.</summary>
        /// <param name="identityAndFacet">A relative URI string [category/]identity[#facet].</param>
        /// <returns>The corresponding servant in the ASM, or null if the servant was not found.</returns>
        public IObject? Find(string identityAndFacet)
        {
            (Identity identity, string facet) = UriParser.ParseIdentityAndFacet(identityAndFacet);
            return Find(identity, facet);
        }

        /// <summary>Adds a servant to this object adapter's Active Servant Map (ASM), using as key the provided
        /// identity and facet. Adding a servant with an identity and facet that are already in the ASM throws
        /// ArgumentException.</summary>
        /// <param name="identity">The identity of the Ice object incarnated by this servant. identity.Name cannot
        /// be empty.</param>
        /// <param name="facet">The facet of the Ice object.</param>
        /// <param name="servant">The servant to add.</param>
        /// <param name="proxyFactory">The proxy factory used to manufacture the returned proxy. Pass INamePrx.Factory
        /// for this parameter. See <see cref="CreateProxy{T}(Identity, string, ProxyFactory{T})"/>.</param>
        /// <returns>A proxy associated with this object adapter, object identity and facet.</returns>
        public T Add<T>(Identity identity, string facet, IObject servant, ProxyFactory<T> proxyFactory)
            where T : class, IObjectPrx
        {
            CheckIdentity(identity);
            lock (_mutex)
            {
                if (_disposeTask != null)
                {
                    throw new ObjectDisposedException($"{typeof(ObjectAdapter).FullName}:{Name}");
                }
                _identityServantMap.Add(new IdentityPlusFacet(identity, facet), servant);
                return CreateProxy(identity, facet, proxyFactory);
            }
        }

        /// <summary>Adds a servant to this object adapter's Active Servant Map (ASM), using as key the provided
        /// identity and facet. Adding a servant with an identity and facet that are already in the ASM throws
        /// ArgumentException.</summary>
        /// <param name="identity">The identity of the Ice object incarnated by this servant. identity.Name cannot
        /// be empty.</param>
        /// <param name="facet">The facet of the Ice object.</param>
        /// <param name="servant">The servant to add.</param>
        public void Add(Identity identity, string facet, IObject servant)
        {
            CheckIdentity(identity);
            lock (_mutex)
            {
                // We check for deactivation here because we don't want to keep this servant when the adapter is being
                // deactivated or destroyed. In other languages, notably C++, keeping such a servant could lead to
                // circular references and leaks.
                if (_disposeTask != null)
                {
                    throw new ObjectDisposedException($"{typeof(ObjectAdapter).FullName}:{Name}");
                }
                _identityServantMap.Add(new IdentityPlusFacet(identity, facet), servant);
            }
        }

        /// <summary>Adds a servant to this object adapter's Active Servant Map (ASM), using as key the provided
        /// identity and facet. Adding a servant with an identity and facet that are already in the ASM throws
        /// ArgumentException.</summary>
        /// <param name="identityAndFacet">A relative URI string [category/]identity[#facet].</param>
        /// <param name="servant">The servant to add.</param>
        /// <param name="proxyFactory">The proxy factory used to manufacture the returned proxy. Pass INamePrx.Factory
        /// for this parameter. See <see cref="CreateProxy{T}(string,ProxyFactory{T})"/>.</param>
        /// <returns>A proxy associated with this object adapter, object identity and facet.</returns>
        public T Add<T>(string identityAndFacet, IObject servant, ProxyFactory<T> proxyFactory)
            where T : class, IObjectPrx
        {
            (Identity identity, string facet) = UriParser.ParseIdentityAndFacet(identityAndFacet);
            return Add(identity, facet, servant, proxyFactory);
        }

        /// <summary>Adds a servant to this object adapter's Active Servant Map (ASM), using as key the provided
        /// identity and facet. Adding a servant with an identity and facet that are already in the ASM throws
        /// ArgumentException.</summary>
        /// <param name="identityAndFacet">A relative URI string [category/]identity[#facet].</param>
        /// <param name="servant">The servant to add.</param>
        public void Add(string identityAndFacet, IObject servant)
        {
            (Identity identity, string facet) = UriParser.ParseIdentityAndFacet(identityAndFacet);
            Add(identity, facet, servant);
        }

        /// <summary>Adds a servant to this object adapter's Active Servant Map (ASM), using as key the provided
        /// identity and the default (empty) facet.</summary>
        /// <param name="identity">The identity of the Ice object incarnated by this servant. identity.Name cannot
        /// be empty.</param>
        /// <param name="servant">The servant to add.</param>
        /// <param name="proxyFactory">The proxy factory used to manufacture the returned proxy. Pass INamePrx.Factory
        /// for this parameter. See <see cref="CreateProxy{T}(Identity, ProxyFactory{T})"/>.</param>
        /// <returns>A proxy associated with this object adapter, object identity and the default facet.</returns>
        public T Add<T>(Identity identity, IObject servant, ProxyFactory<T> proxyFactory)
            where T : class, IObjectPrx =>
            Add(identity, "", servant, proxyFactory);

        /// <summary>Adds a servant to this object adapter's Active Servant Map (ASM), using as key the provided
        /// identity and the default (empty) facet.</summary>
        /// <param name="identity">The identity of the Ice object incarnated by this servant. identity.Name cannot
        /// be empty.</param>
        /// <param name="servant">The servant to add.</param>
        public void Add(Identity identity, IObject servant) => Add(identity, "", servant);

        /// <summary>Adds a servant to this object adapter's Active Servant Map (ASM), using as key a unique identity
        /// and the provided facet. This method creates the unique identity with a UUID name and an empty category.
        /// </summary>
        /// <param name="facet">The facet of the Ice object.</param>
        /// <param name="servant">The servant to add.</param>
        /// <param name="proxyFactory">The proxy factory used to manufacture the returned proxy. Pass INamePrx.Factory
        /// for this parameter. See <see cref="CreateProxy{T}(Identity, string, ProxyFactory{T})"/>.</param>
        /// <returns>A proxy associated with this object adapter, object identity and facet.</returns>
        public T AddWithUUID<T>(string facet, IObject servant, ProxyFactory<T> proxyFactory)
            where T : class, IObjectPrx =>
            Add(new Identity(Guid.NewGuid().ToString(), ""), facet, servant, proxyFactory);

        /// <summary>Adds a servant to this object adapter's Active Servant Map (ASM), using as key a unique identity
        /// and the default (empty) facet. This method creates the unique identity with a UUID name and an empty
        /// category.</summary>
        /// <param name="servant">The servant to add.</param>
        /// <param name="proxyFactory">The proxy factory used to manufacture the returned proxy. Pass INamePrx.Factory
        /// for this parameter. See <see cref="CreateProxy{T}(Identity, ProxyFactory{T})"/>.</param>
        /// <returns>A proxy associated with this object adapter, object identity and the default facet.</returns>
        public T AddWithUUID<T>(IObject servant, ProxyFactory<T> proxyFactory) where T : class, IObjectPrx =>
            AddWithUUID("", servant, proxyFactory);

        /// <summary>Removes a servant previously added to the Active Servant Map (ASM) using Add.</summary>
        /// <param name="identity">The identity of the Ice object.</param>
        /// <param name="facet">The facet of the Ice object.</param>
        /// <returns>The servant that was just removed from the ASM, or null if the servant was not found.</returns>
        public IObject? Remove(Identity identity, string facet = "")
        {
            lock (_mutex)
            {
                var key = new IdentityPlusFacet(identity, facet);
                if (_identityServantMap.TryGetValue(key, out IObject? servant))
                {
                    _identityServantMap.Remove(key);
                }
                return servant;
            }
        }

        /// <summary>Removes a servant previously added to the Active Servant Map (ASM) using Add.</summary>
        /// <param name="identityAndFacet">A relative URI string [category/]identity[#facet].</param>
        /// <returns>The servant that was just removed from the ASM, or null if the servant was not found.</returns>
        public IObject? Remove(string identityAndFacet)
        {
            (Identity identity, string facet) = UriParser.ParseIdentityAndFacet(identityAndFacet);
            return Remove(identity, facet);
        }

        /// <summary>Adds a category-specific default servant to this object adapter's Active Servant Map (ASM), using
        /// as key the provided category and facet.</summary>
        /// <param name="category">The object identity category.</param>
        /// <param name="facet">The facet.</param>
        /// <param name="servant">The default servant to add.</param>
        public void AddDefaultForCategory(string category, string facet, IObject servant)
        {
            lock (_mutex)
            {
                if (_disposeTask != null)
                {
                    throw new ObjectDisposedException($"{typeof(ObjectAdapter).FullName}:{Name}");
                }
                _categoryServantMap.Add(new CategoryPlusFacet(category, facet), servant);
            }
        }

        /// <summary>Adds a category-specific default servant to this object adapter's Active Servant Map (ASM), using
        /// as key the provided category and the default (empty) facet.</summary>
        /// <param name="category">The object identity category.</param>
        /// <param name="servant">The default servant to add.</param>
        public void AddDefaultForCategory(string category, IObject servant) =>
            AddDefaultForCategory(category, "", servant);

        /// <summary>Removes a category-specific default servant previously added to the Active Servant Map (ASM) using
        /// AddDefaultForCategory.</summary>
        /// <param name="category">The category associated with this default servant.</param>
        /// <param name="facet">The facet.</param>
        /// <returns>The servant that was just removed from the ASM, or null if the servant was not found.</returns>
        public IObject? RemoveDefaultForCategory(string category, string facet = "")
        {
            lock (_mutex)
            {
                var key = new CategoryPlusFacet(category, facet);
                if (_categoryServantMap.TryGetValue(key, out IObject? servant))
                {
                    _categoryServantMap.Remove(key);
                }
                return servant;
            }
        }

        /// <summary>Adds a default servant to this object adapter's Active Servant Map (ASM), using as key the provided
        /// facet.</summary>
        /// <param name="facet">The facet.</param>
        /// <param name="servant">The default servant to add.</param>
        public void AddDefault(string facet, IObject servant)
        {
            lock (_mutex)
            {
                if (_disposeTask != null)
                {
                    throw new ObjectDisposedException($"{typeof(ObjectAdapter).FullName}:{Name}");
                }
                _defaultServantMap.Add(facet, servant);
            }
        }

        /// <summary>Adds a default servant to this object adapter's Active Servant Map (ASM), using as key the default
        /// (empty) facet.</summary>
        /// <param name="servant">The default servant to add.</param>
        public void AddDefault(IObject servant) => AddDefault("", servant);

        /// <summary>Removes a default servant previously added to the Active Servant Map (ASM) using AddDefault.
        /// </summary>
        /// <param name="facet">The facet.</param>
        /// <returns>The servant that was just removed from the ASM, or null if the servant was not found.</returns>
        public IObject? RemoveDefault(string facet = "")
        {
            lock (_mutex)
            {
                if (_defaultServantMap.TryGetValue(facet, out IObject? servant))
                {
                    _defaultServantMap.Remove(facet);
                }
                return servant;
            }
        }

        /// <summary>Creates a proxy for the object with the given identity and facet. If this object adapter is
        /// configured with an adapter id, creates an indirect proxy that refers to the adapter id. If a replica group
        /// id is also defined, creates an indirect proxy that refers to the replica group id. Otherwise, if no adapter
        /// id is defined, creates a direct proxy containing this object adapter's published endpoints.</summary>
        /// <param name="identity">The object's identity.</param>
        /// <param name="facet">The facet.</param>
        /// <param name="factory">The proxy factory. Use INamePrx.Factory for this parameter, where INamePrx is the
        /// desired proxy type.</param>
        /// <returns>A proxy for the object with the given identity and facet.</returns>
        public T CreateProxy<T>(Identity identity, string facet, ProxyFactory<T> factory) where T : class, IObjectPrx
        {
            if (_id.Length == 0)
            {
                return CreateDirectProxy(identity, facet, factory);
            }
            else if (_replicaGroupId.Length == 0)
            {
                return CreateIndirectProxy(identity, facet, factory);
            }
            else
            {
                return factory(CreateReference(identity, facet, ImmutableArray.Create(_replicaGroupId)));
            }
        }

        /// <summary>Creates a proxy for the object with the given identity. If this object adapter is configured with
        /// an adapter id, creates an indirect proxy that refers to the adapter id. If a replica group id is also
        /// defined, creates an indirect proxy that refers to the replica group id. Otherwise, if no adapter
        /// id is defined, creates a direct proxy containing this object adapter's published endpoints.</summary>
        /// <param name="identity">The object's identity.</param>
        /// <param name="factory">The proxy factory. Use INamePrx.Factory for this parameter, where INamePrx is the
        /// desired proxy type.</param>
        /// <returns>A proxy for the object with the given identity.</returns>
        public T CreateProxy<T>(Identity identity, ProxyFactory<T> factory) where T : class, IObjectPrx =>
            CreateProxy(identity, "", factory);

        /// <summary>Creates a proxy for the object with the given identity and facet. If this object adapter is
        /// configured with an adapter id, creates an indirect proxy that refers to the adapter id. If a replica group
        /// id is also defined, creates an indirect proxy that refers to the replica group id. Otherwise, if no adapter
        /// id is defined, creates a direct proxy containing this object adapter's published endpoints.</summary>
        /// <param name="identityAndFacet">A relative URI string [category/]identity[#facet].</param>
        /// <param name="factory">The proxy factory. Use INamePrx.Factory for this parameter, where INamePrx is the
        /// desired proxy type.</param>
        /// <returns>A proxy for the object with the given identity and facet.</returns>
        public T CreateProxy<T>(string identityAndFacet, ProxyFactory<T> factory) where T : class, IObjectPrx
        {
            (Identity identity, string facet) = UriParser.ParseIdentityAndFacet(identityAndFacet);
            return CreateProxy(identity, facet, factory);
        }

        /// <summary>Creates a direct proxy for the object with the given identity and facet. The returned proxy
        /// contains this object adapter's published endpoints.</summary>
        /// <param name="identity">The object's identity.</param>
        /// <param name="facet">The facet.</param>
        /// <param name="factory">The proxy factory. Use INamePrx.Factory for this parameter, where INamePrx is the
        /// desired proxy type.</param>
        /// <returns>A proxy for the object with the given identity and facet.</returns>
        public T CreateDirectProxy<T>(Identity identity, string facet, ProxyFactory<T> factory)
            where T : class, IObjectPrx => factory(CreateReference(identity, facet, ImmutableArray<string>.Empty));

        /// <summary>Creates a direct proxy for the object with the given identity. The returned proxy contains this
        /// object adapter's published endpoints.</summary>
        /// <param name="identity">The object's identity.</param>
        /// <param name="factory">The proxy factory. Use INamePrx.Factory for this parameter, where INamePrx is the
        /// desired proxy type.</param>
        /// <returns>A proxy for the object with the given identity.</returns>
        public T CreateDirectProxy<T>(Identity identity, ProxyFactory<T> factory) where T : class, IObjectPrx =>
            CreateDirectProxy(identity, "", factory);

        /// <summary>Creates a direct proxy for the object with the given identity and facet. The returned proxy
        /// contains this object adapter's published endpoints.</summary>
        /// <param name="identityAndFacet">A relative URI string [category/]identity[#facet].</param>
        /// <param name="factory">The proxy factory. Use INamePrx.Factory for this parameter, where INamePrx is the
        /// desired proxy type.</param>
        /// <returns>A proxy for the object with the given identity and facet.</returns>
        public T CreateDirectProxy<T>(string identityAndFacet, ProxyFactory<T> factory)
            where T : class, IObjectPrx
        {
            (Identity identity, string facet) = UriParser.ParseIdentityAndFacet(identityAndFacet);
            return CreateDirectProxy(identity, facet, factory);
        }

        /// <summary>Creates an indirect proxy for the object with the given identity and facet.</summary>
        /// <param name="identity">The object's identity.</param>
        /// <param name="facet">The facet.</param>
        /// <param name="factory">The proxy factory. Use INamePrx.Factory for this parameter, where INamePrx is the
        /// desired proxy type.</param>
        /// <returns>A proxy for the object with the given identity and facet.</returns>
        public T CreateIndirectProxy<T>(Identity identity, string facet, ProxyFactory<T> factory)
            where T : class, IObjectPrx => factory(CreateReference(identity, facet, ImmutableArray.Create(_id)));

        /// <summary>Creates an indirect proxy for the object with the given identity.</summary>
        /// <param name="identity">The object's identity.</param>
        /// <param name="factory">The proxy factory. Use INamePrx.Factory for this parameter, where INamePrx is the
        /// desired proxy type.</param>
        /// <returns>A proxy for the object with the given identity.</returns>
        public T CreateIndirectProxy<T>(Identity identity, ProxyFactory<T> factory) where T : class, IObjectPrx =>
            CreateIndirectProxy(identity, "", factory);

        /// <summary>Creates an indirect proxy for the object with the given identity and facet.</summary>
        /// <param name="identityAndFacet">A relative URI string [category/]identity[#facet].</param>
        /// <param name="factory">The proxy factory. Use INamePrx.Factory for this parameter, where INamePrx is the
        /// desired proxy type.</param>
        /// <returns>A proxy for the object with the given identity and facet.</returns>
        public T CreateIndirectProxy<T>(string identityAndFacet, ProxyFactory<T> factory)
            where T : class, IObjectPrx
        {
            (Identity identity, string facet) = UriParser.ParseIdentityAndFacet(identityAndFacet);
            return CreateIndirectProxy(identity, facet, factory);
        }

        /// <summary>Retrieves the endpoints configured with this object adapter.</summary>
        /// <returns>The endpoints.</returns>
        public IReadOnlyList<Endpoint> GetEndpoints()
        {
            lock (_mutex)
            {
                return _incomingConnectionFactories.Select(factory => factory.PublishedEndpoint).ToArray();
            }
        }

        /// <summary>Refreshes the set of published endpoints. The runtime rereads the PublishedEndpoints property
        /// (if set) and rereads the list of local interfaces if the adapter is configured to listen on all endpoints.
        /// This method is useful when the network interfaces of the host changes: it allows you to refresh the
        /// endpoint information published in the proxies created by this object adapter.</summary>
        public void RefreshPublishedEndpoints()
        {
            try
            {
                RefreshPublishedEndpointsAsync().Wait();
            }
            catch (AggregateException ex)
            {
                Debug.Assert(ex.InnerException != null);
                throw ExceptionUtil.Throw(ex.InnerException);
            }
        }

        /// <summary>Refreshes the set of published endpoints. The runtime rereads the PublishedEndpoints property
        /// (if set) and rereads the list of local interfaces if the adapter is configured to listen on all endpoints.
        /// This method is useful when the network interfaces of the host changes: it allows you to refresh the
        /// endpoint information published in the proxies created by this object adapter.</summary>
        public async Task RefreshPublishedEndpointsAsync()
        {
            IReadOnlyList<Endpoint> publishedEndpoints = await ComputePublishedEndpointsAsync().ConfigureAwait(false);

            lock (_mutex)
            {
                if (_disposeTask != null)
                {
                    throw new ObjectDisposedException($"{typeof(ObjectAdapter).FullName}:{Name}");
                }

                (publishedEndpoints, _publishedEndpoints) = (_publishedEndpoints, publishedEndpoints);
            }

            try
            {
                await UpdateLocatorRegistryAsync(_locatorInfo,
                                                 CreateDirectProxy(new Identity("dummy", ""),
                                                 IObjectPrx.Factory));
            }
            catch
            {
                lock (_mutex)
                {
                    // Restore the old published endpoints.
                    _publishedEndpoints = publishedEndpoints;
                    throw;
                }
            }
        }

        /// <summary>Retrieves the endpoints that would be listed in a proxy created by this object adapter.
        /// </summary>
        /// <returns>The published endpoints.</returns>
        public IReadOnlyList<Endpoint> GetPublishedEndpoints()
        {
            lock (_mutex)
            {
                return _publishedEndpoints;
            }
        }

        /// <summary>Sets the endpoints that from now on will be listed in the proxies created by this object adapter.
        /// </summary>
        /// <param name="newEndpoints">The new published endpoints.</param>
        public void SetPublishedEndpoints(IEnumerable<Endpoint> newEndpoints)
        {
            try
            {
                SetPublishedEndpointsAsync(newEndpoints).Wait();
            }
            catch (AggregateException ex)
            {
                Debug.Assert(ex.InnerException != null);
                throw ExceptionUtil.Throw(ex.InnerException);
            }
        }

        /// <summary>Sets the endpoints that from now on will be listed in the proxies created by this object adapter.
        /// </summary>
        /// <param name="newEndpoints">The new published endpoints.</param>
        public async Task SetPublishedEndpointsAsync(IEnumerable<Endpoint> newEndpoints)
        {
            IReadOnlyList<Endpoint> oldPublishedEndpoints;

            lock (_mutex)
            {
                if (Name.Length == 0)
                {
                    throw new ArgumentException("cannot set published endpoints on a nameless object adapter");
                }

                if (newEndpoints.Select(endpoint => endpoint.Protocol).Distinct().Count() > 1)
                {
                    throw new ArgumentException("all published endpoints must use the same protocol");
                }

                if (_disposeTask != null)
                {
                    throw new ObjectDisposedException($"{typeof(ObjectAdapter).FullName}:{Name}");
                }

                if (_routerInfo != null)
                {
                    throw new InvalidOperationException(
                        "cannot set published endpoints on an object adapter associated with a router");
                }

                oldPublishedEndpoints = _publishedEndpoints;
                _publishedEndpoints = newEndpoints.ToArray();
                if (_publishedEndpoints.Count == 0)
                {
                    _publishedEndpoints = Array.Empty<Endpoint>();
                }
            }

            try
            {
                await UpdateLocatorRegistryAsync(_locatorInfo,
                                                 CreateDirectProxy(new Identity("dummy", ""),
                                                 IObjectPrx.Factory));
            }
            catch
            {
                lock (_mutex)
                {
                    // Restore the old published endpoints.
                    _publishedEndpoints = oldPublishedEndpoints;
                    throw;
                }
            }
        }

        // Called by Communicator to create a nameless ObjectAdapter
        internal ObjectAdapter(
            Communicator communicator,
            bool serializeDispatch,
            TaskScheduler? scheduler,
            Protocol protocol)
        {
            Communicator = communicator;
            Name = "";
            SerializeDispatch = serializeDispatch;
            TaskScheduler = scheduler;

            _publishedEndpoints = Array.Empty<Endpoint>();
            _routerInfo = null;

            _id = "";
            _replicaGroupId = "";
            Protocol = protocol;
        }

        // Called by Communicator.
        internal ObjectAdapter(
            Communicator communicator,
            string name,
            bool serializeDispatch,
            TaskScheduler? scheduler,
            IRouterPrx? router)
        {
            Debug.Assert(name.Length != 0);

            Communicator = communicator;
            Name = name;
            SerializeDispatch = serializeDispatch;
            TaskScheduler = scheduler;

            _publishedEndpoints = Array.Empty<Endpoint>();
            _routerInfo = null;

            (bool noProps, List<string> unknownProps) = FilterProperties();

            // Warn about unknown object adapter properties.
            if (unknownProps.Count != 0 && Communicator.WarnUnknownProperties)
            {
                var message = new StringBuilder("found unknown properties for object adapter `");
                message.Append(Name);
                message.Append("':");
                foreach (string s in unknownProps)
                {
                    message.Append("\n    ");
                    message.Append(s);
                }
                Communicator.Logger.Warning(message.ToString());
            }

            // Make sure named adapter has configuration.
            if (router == null && noProps)
            {
                throw new InvalidConfigurationException($"object adapter `{Name}' requires configuration");
            }

            _id = Communicator.GetProperty($"{Name}.AdapterId") ?? "";
            _replicaGroupId = Communicator.GetProperty($"{Name}.ReplicaGroupId") ?? "";

            int frameSizeMax =
                Communicator.GetPropertyAsByteSize($"{Name}.IncomingFrameSizeMax") ?? Communicator.IncomingFrameSizeMax;
            IncomingFrameSizeMax = frameSizeMax == 0 ? int.MaxValue : frameSizeMax;

            try
            {
                router ??= Communicator.GetPropertyAsProxy($"{Name}.Router", IRouterPrx.Factory);

                if (router != null)
                {
                    Protocol = router.Protocol;
                    _routerInfo = Communicator.GetRouterInfo(router);
                    Debug.Assert(_routerInfo != null);

                    // Make sure this router is not already registered with another adapter.
                    if (_routerInfo.Adapter != null)
                    {
                        string routerStr = router.Identity.ToString(Communicator.ToStringMode);
                        throw new ArgumentException($"router `{routerStr}' already registered with an object adapter",
                            nameof(router));
                    }

                    // Associate this object adapter with the router. This way, new outgoing connections to the
                    // router's client proxy will use this object adapter for callbacks.
                    _routerInfo.Adapter = this;

                    // Also modify all existing outgoing connections to the router's client proxy to use this object
                    // adapter for callbacks.
                    Communicator.OutgoingConnectionFactory.SetRouterInfo(_routerInfo);
                }
                else
                {
                    IReadOnlyList<Endpoint>? endpoints = null;

                    // Parse the endpoints, but don't store them in the adapter. The connection factory might change
                    // it, for example, to fill in the real port number.
                    if (Communicator.GetProperty($"{Name}.Endpoints") is string value)
                    {
                        if (UriParser.IsEndpointUri(value))
                        {
                            Protocol = Protocol.Ice2;
                            endpoints = UriParser.ParseEndpoints(value, Communicator);
                        }
                        else
                        {
                            Protocol = Protocol.Ice1;
                            endpoints = Ice1Parser.ParseEndpoints(value, communicator);
                            _invocationMode = Ice1Parser.ParseProxyOptions(Name, communicator);
                        }

                        _incomingConnectionFactories.AddRange(endpoints.SelectMany(endpoint =>
                            endpoint.ExpandHost(out Endpoint? publishedEndpoint).Select(expanded =>
                                expanded.IsDatagram ?
                                    (IncomingConnectionFactory)new DatagramIncomingConnectionFactory(
                                        this,
                                        expanded,
                                         publishedEndpoint) :
                                    new AcceptorIncomingConnectionFactory(this, expanded, publishedEndpoint))));
                    }
                    else
                    {
                        // This OA is mostly likely used for colocation, unless a router is set.
                        Protocol = router != null ? router.Protocol : Protocol.Ice2;
                    }

                    if (endpoints == null || endpoints.Count == 0)
                    {
                        if (Communicator.TraceLevels.Transport >= 2)
                        {
                            Communicator.Logger.Trace(Communicator.TraceLevels.TransportCategory,
                                                      $"created adapter `{Name}' without endpoints");
                        }
                    }
                }

                // Parse published endpoints.
                _publishedEndpoints = ComputePublishedEndpointsAsync().Result;
                Locator = Communicator.GetPropertyAsProxy($"{Name}.Locator", ILocatorPrx.Factory)
                    ?? Communicator.DefaultLocator;
            }
            catch (AggregateException ex)
            {
                Dispose();
                Debug.Assert(ex.InnerException != null);
                throw ExceptionUtil.Throw(ex.InnerException);
            }
            catch
            {
                Dispose();
                throw;
            }
        }

        internal async ValueTask<(OutgoingResponseFrame, bool)> DispatchAsync(
            IncomingRequestFrame request,
            Current current,
            CancellationToken cancel)
        {
            IDispatchObserver? dispatchObserver = Communicator.Observer?.GetDispatchObserver(current,
                                                                                             current.StreamId,
                                                                                             request.Size);
            dispatchObserver?.Attach();
            try
            {
                Debug.Assert(current.Adapter == this);
                IObject? servant = Find(current.Identity, current.Facet);
                if (servant == null)
                {
                    throw new ObjectNotExistException(
                        _replicaGroupId.Length == 0 ? RetryPolicy.NoRetry : RetryPolicy.OtherReplica);
                }

                // TODO: support input streamable data if Current.EndOfStream == false and output streamable data.

                ValueTask<OutgoingResponseFrame> DispatchAsync(int i)
                {
                    if (i < Interceptors.Count)
                    {
                        DispatchInterceptor interceptor = Interceptors[i++];
                        return interceptor(request, current, (request, current, cancel) => DispatchAsync(i), cancel);
                    }
                    else
                    {
                        return servant.DispatchAsync(request, current, cancel);
                    }
                }

                OutgoingResponseFrame response = await DispatchAsync(0).ConfigureAwait(false);
                if (!current.IsOneway)
                {
                    response.Finish();
                }
                dispatchObserver?.Reply(response.Size);
                return (response, true);
            }
            catch (Exception ex)
            {
                if (!current.IsOneway)
                {
                    RemoteException actualEx;
                    if (ex is RemoteException remoteEx && !remoteEx.ConvertToUnhandled)
                    {
                        actualEx = remoteEx;
                        dispatchObserver?.RemoteException();
                    }
                    else
                    {
                        actualEx = new UnhandledException(ex);
                        dispatchObserver?.Failed(actualEx.InnerException!.GetType().FullName ?? "System.Exception");
                        if (Communicator.WarnDispatch)
                        {
                            Warning(ex);
                        }
                    }

                    var response = new OutgoingResponseFrame(request, actualEx);
                    response.Finish();
                    dispatchObserver?.Reply(response.Size);
                    return (response, true);
                }
                else
                {
                    if (Communicator.WarnDispatch)
                    {
                        Warning(ex);
                    }
                    dispatchObserver?.Failed(ex.GetType().FullName ?? "System.Exception");
                    return (OutgoingResponseFrame.WithVoidReturnValue(current), true);
                }
            }
            finally
            {
                dispatchObserver?.Detach();
            }

            void Warning(Exception ex)
            {
                var output = new StringBuilder();
                output.Append("dispatch exception:");
                output.Append("\nidentity: ").Append(current.Identity.ToString(Communicator.ToStringMode));
                output.Append("\nfacet: ").Append(StringUtil.EscapeString(current.Facet, Communicator.ToStringMode));
                output.Append("\noperation: ").Append(current.Operation);
                if ((current.Connection as IPConnection)?.RemoteEndpoint is System.Net.IPEndPoint remoteEndpoint)
                {
                    output.Append("\nremote address: ").Append(remoteEndpoint);
                }
                output.Append('\n');
                output.Append(ex.ToString());
                Communicator.Logger.Warning(output.ToString());
            }
        }

        internal Endpoint GetColocatedEndpoint()
        {
            lock (_mutex)
            {
                if (_disposeTask != null)
                {
                    throw new ObjectDisposedException($"{typeof(ObjectAdapter).FullName}:{Name}");
                }

                if (_colocatedConnectionFactory == null)
                {
                    _colocatedConnectionFactory = new AcceptorIncomingConnectionFactory(this,
                                                                                        new ColocatedEndpoint(this),
                                                                                        null);

                    // It's safe to start the connection within the synchronization, this isn't supposed to block for
                    // colocated connections.
                    _colocatedConnectionFactory.Activate();
                }
            }
            return _colocatedConnectionFactory.PublishedEndpoint;
        }

        internal bool IsLocal(Reference r)
        {
            // The proxy protocol must match the object adapter's protocol.
            if (r.Protocol != Protocol)
            {
                return false;
            }

            // NOTE: it's important that IsLocal() doesn't perform any blocking operations as
            // it can be called for AMI invocations if the proxy has no delegate set yet.
            if (r.IsWellKnown)
            {
                lock (_mutex)
                {
                    // Is the servant in the ASM?
                    // TODO: Currently doesn't check default servants - should we?
                    return _identityServantMap.ContainsKey(new IdentityPlusFacet(r.Identity, r.Facet));
                }
            }
            else if (r.IsIndirect)
            {
                // Proxy is local if the reference's location matches this adapter id or replica group id.
                return r.Location.Count == 1 && (r.Location[0] == _id || r.Location[0] == _replicaGroupId);
            }
            else
            {
                lock (_mutex)
                {
                    if (_disposeTask != null)
                    {
                        throw new ObjectDisposedException($"{typeof(ObjectAdapter).FullName}:{Name}");
                    }

                    // Proxies which have at least one endpoint in common with the endpoints used by this object
                    // adapter's incoming connection factories are considered local.
                    return r.Endpoints.Any(endpoint =>
                        _publishedEndpoints.Any(publishedEndpoint => endpoint.IsLocal(publishedEndpoint)) ||
                        _incomingConnectionFactories.Any(factory => factory.IsLocal(endpoint)));
                }
            }
        }

        internal void UpdateConnectionObservers()
        {
            foreach (IncomingConnectionFactory factory in _incomingConnectionFactories)
            {
                factory.UpdateConnectionObservers();
            }
        }

        private static void CheckIdentity(Identity ident)
        {
            if (ident.Name.Length == 0)
            {
                throw new ArgumentException("identity name cannot be empty", nameof(ident));
            }
        }

        private Reference CreateReference(Identity identity, string facet, IReadOnlyList<string> location)
        {
            CheckIdentity(identity);
            lock (_mutex)
            {
                if (_disposeTask != null)
                {
                    throw new ObjectDisposedException($"{typeof(ObjectAdapter).FullName}:{Name}");
                }

                // TODO: revisit location/endpoints logic with ice2.
                return new Reference(communicator: Communicator,
                                     encoding: Protocol.GetEncoding(),
                                     endpoints: location.Count == 0 ?
                                        _publishedEndpoints : ImmutableArray<Endpoint>.Empty,
                                     facet: facet,
                                     identity: identity,
                                     invocationMode: _invocationMode,
                                     location: location,
                                     protocol: _publishedEndpoints.Count > 0 ?
                                               _publishedEndpoints[0].Protocol : Protocol);
            }
        }

        private async Task<IReadOnlyList<Endpoint>> ComputePublishedEndpointsAsync()
        {
            IReadOnlyList<Endpoint>? endpoints = null;
            if (_routerInfo != null)
            {
                // Get the router's server proxy endpoints and use them as the published endpoints.
                endpoints = await _routerInfo.GetServerEndpointsAsync().ConfigureAwait(false);
                endpoints = endpoints.Distinct().ToArray();
            }
            else
            {
                // Parse published endpoints. If set, these are used in proxies instead of the connection factory
                // endpoints.
                if (Name.Length > 0 && Communicator.GetProperty($"{Name}.PublishedEndpoints") is string value)
                {
                    if (UriParser.IsEndpointUri(value))
                    {
                        endpoints = UriParser.ParseEndpoints(value, Communicator);
                    }
                    else
                    {
                        endpoints = Ice1Parser.ParseEndpoints(value, Communicator, oaEndpoints: false);
                    }
                }
                if (endpoints == null || endpoints.Count == 0)
                {
                    // If the PublishedEndpoints property isn't set, we compute the published endpoints from the OA
                    // endpoints, expanding any endpoint that may be listening on INADDR_ANY to include actual addresses
                    // in the published endpoints.
                    // We also filter out duplicate endpoints, this might occur if an endpoint with a DNS name
                    // expands to multiple addresses. In this case, multiple incoming connection factories can point to
                    // the same published endpoint.

                    endpoints = _incomingConnectionFactories.SelectMany(factory =>
                        factory.PublishedEndpoint.ExpandIfWildcard()).Distinct().ToArray();
                }
            }

            if (Communicator.TraceLevels.Transport >= 1 && endpoints.Count > 0)
            {
                var sb = new StringBuilder("published endpoints for object adapter `");
                sb.Append(Name);
                sb.Append("':\n");
                sb.AppendEndpointList(endpoints);
                Communicator.Logger.Trace(Communicator.TraceLevels.TransportCategory, sb.ToString());
            }

            return endpoints;
        }

        // TODO: split between register and unregister. And add cancellation token?
        private async Task UpdateLocatorRegistryAsync(LocatorInfo? locatorInfo, IObjectPrx? proxy)
        {
            if (_id.Length == 0 || locatorInfo == null)
            {
                return; // Nothing to update.
            }

            ILocatorRegistryPrx? locatorRegistry =
                await locatorInfo.GetLocatorRegistryAsync().ConfigureAwait(false);
            if (locatorRegistry == null)
            {
                return;
            }

            try
            {
                if (locatorRegistry.Protocol == Protocol.Ice1)
                {
#pragma warning disable CS0618 // calling deprecated methods
                    if (_replicaGroupId.Length == 0)
                    {
                        await locatorRegistry.SetAdapterDirectProxyAsync(_id, proxy).ConfigureAwait(false);
                    }
                    else
                    {
                        await locatorRegistry.SetReplicatedAdapterDirectProxyAsync(
                            _id,
                            _replicaGroupId,
                            proxy).ConfigureAwait(false);
                    }
#pragma warning restore CS0618
                }
                else
                {
                    if (proxy == null)
                    {
                        await locatorRegistry.UnregisterAdapterEndpointsAsync(_id,
                                                                              _replicaGroupId,
                                                                              Protocol).ConfigureAwait(false);
                    }
                    else
                    {
                        await locatorRegistry.RegisterAdapterEndpointsAsync(_id,
                                                                            _replicaGroupId,
                                                                            proxy).ConfigureAwait(false);
                    }
                }
            }
            catch (ObjectDisposedException)
            {
                // Expected if colocated call and OA is deactivated or the communicator is disposed, ignore.
            }
            catch (Exception ex)
            {
                if (Communicator.TraceLevels.Location >= 1)
                {
                    if (_replicaGroupId.Length == 0)
                    {
                        Communicator.Logger.Trace(Communicator.TraceLevels.LocatorCategory,
                            $"could not update the endpoints of object adapter `{_id}' in the locator registry:\n{ex}");
                    }
                    else
                    {
                        Communicator.Logger.Trace(Communicator.TraceLevels.LocatorCategory,
                            @$"could not update the endpoints of object adapter `{_id
                                }' with replica group `{_replicaGroupId}' in the locator registry:\n{ex}");
                    }
                }
                throw;
            }

            if (Communicator.TraceLevels.Location >= 1)
            {
                var sb = new StringBuilder();
                sb.Append($"updated object adapter `{_id}' endpoints with the locator registry\n");
                sb.Append("endpoints = ");
                if (proxy != null)
                {
                    sb.Append(string.Join(":", proxy.Endpoints));
                }
                Communicator.Logger.Trace(Communicator.TraceLevels.LocatorCategory, sb.ToString());
            }
        }

        private (bool NoProps, List<string> UnknownProps) FilterProperties()
        {
            // Do not create unknown properties list if Ice prefix, i.e. Ice, Glacier2, etc.
            bool addUnknown = true;
            string prefix = $"{Name}.";
            foreach (string propertyName in PropertyNames.ClassPropertyNames)
            {
                if (prefix.StartsWith($"{propertyName}.", StringComparison.Ordinal))
                {
                    addUnknown = false;
                    break;
                }
            }

            bool noProps = true;
            var unknownProps = new List<string>();
            Dictionary<string, string> props = Communicator.GetProperties(forPrefix: prefix);
            foreach (string prop in props.Keys)
            {
                bool valid = false;
                for (int i = 0; i < _suffixes.Length; ++i)
                {
                    if (prop.Equals(prefix + _suffixes[i]))
                    {
                        noProps = false;
                        valid = true;
                        break;
                    }
                }

                if (!valid && addUnknown)
                {
                    unknownProps.Add(prop);
                }
            }
            return (noProps, unknownProps);
        }

        private readonly struct IdentityPlusFacet : IEquatable<IdentityPlusFacet>
        {
            internal readonly Identity Identity;
            internal readonly string Facet;

            public bool Equals(IdentityPlusFacet other) =>
                Identity.Equals(other.Identity) && Facet.Equals(other.Facet);

            public override bool Equals(object? obj) => obj is IdentityPlusFacet value && Equals(value);

            // Since facet is often empty, we don't want the empty facet to contribute to the hash value.
            public override int GetHashCode() =>
                Facet.Length == 0 ? Identity.GetHashCode() : HashCode.Combine(Identity, Facet);

            internal IdentityPlusFacet(Identity identity, string facet)
            {
                Identity = identity;
                Facet = facet;
            }
        }

        private readonly struct CategoryPlusFacet : IEquatable<CategoryPlusFacet>
        {
            internal readonly string Category;
            internal readonly string Facet;

            public bool Equals(CategoryPlusFacet other) =>
                Category.Equals(other.Category) && Facet.Equals(other.Facet);

            public override bool Equals(object? obj) => obj is CategoryPlusFacet value && Equals(value);

            public override int GetHashCode() =>
                Facet.Length == 0 ? Category.GetHashCode() : HashCode.Combine(Category, Facet);

            internal CategoryPlusFacet(string category, string facet)
            {
                Category = category;
                Facet = facet;
            }
        }
    }
}
