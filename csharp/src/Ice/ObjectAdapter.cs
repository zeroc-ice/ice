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
        /// <summary>Returns the adapter ID of this object adapter, or the empty string if this object adapter does not
        /// have an adapter ID.</summary>
        public string AdapterId { get; }

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

        /// <summary>Returns the endpoints listed in a direct proxy created by this object adapter.
        /// <seealso cref="SetPublishedEndpoints"/>
        /// <seealso cref="SetPublishedEndpointsAsync"/></summary>
        public IReadOnlyList<Endpoint> PublishedEndpoints
        {
            get
            {
                lock (_mutex)
                {
                    return _publishedEndpoints;
                }
            }
        }

        /// <summary>Returns the replica group ID of this object adapter, or the empty string if this object adapter
        /// does not belong to a replica group.</summary>
        public string ReplicaGroupId { get; }

        /// <summary>Indicates whether or not this object adapter serializes the dispatching of requests received
        /// over the same connection.</summary>
        /// <value>The serialize dispatch value.</value>
        public bool SerializeDispatch { get; }

        /// <summary>Returns the TaskScheduler used to dispatch requests.</summary>
        public TaskScheduler? TaskScheduler { get; }

        internal int IncomingFrameSizeMax { get; }

        private static readonly string[] _suffixes =
        {
            "ACM",
            "ACM.Timeout",
            "ACM.Heartbeat",
            "ACM.Close",
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

        private readonly Acm _acm;
        private Task? _activateTask;

        private readonly Dictionary<(string Category, string Facet), IObject> _categoryServantMap = new ();
        private AcceptorIncomingConnectionFactory? _colocatedConnectionFactory;
        private readonly Dictionary<string, IObject> _defaultServantMap = new ();
        private Task? _disposeTask;
        private readonly Dictionary<(Identity Identity, string Facet), IObject> _identityServantMap = new ();

        private readonly List<IncomingConnectionFactory> _incomingConnectionFactories = new ();
        private readonly List<DispatchInterceptor> _interceptors = new ();
        private readonly InvocationMode _invocationMode = InvocationMode.Twoway;

        private volatile LocatorInfo? _locatorInfo;
        private readonly object _mutex = new ();
        private IReadOnlyList<Endpoint> _publishedEndpoints;

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
        // TODO: would be nice to add a CancellationToken parameter
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

                _interceptors.AddRange(Communicator.DispatchInterceptors);
                _interceptors.AddRange(interceptors);

                // Activate the incoming connection factories to start accepting connections
                foreach (IncomingConnectionFactory factory in _incomingConnectionFactories)
                {
                    factory.Activate();
                }

                // In the event _publishedEndpoints is empty, RegisterEndpointsAsync does nothing.
                _activateTask ??= RegisterEndpointsAsync(_publishedEndpoints, default);
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
                    await UnregisterEndpointsAsync(default).ConfigureAwait(false);
                }
                catch
                {
                    // We can't throw exceptions in deactivate so we ignore failures to unregister endpoints
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
                if (!_identityServantMap.TryGetValue((identity, facet), out IObject? servant))
                {
                    if (!_categoryServantMap.TryGetValue((identity.Category, facet), out servant))
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
            Add(identity, facet, servant);
            return CreateProxy(identity, facet, proxyFactory);
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
                _identityServantMap.Add((identity, facet), servant);
            }
        }

        /// <summary>Adds a servant to this object adapter's Active Servant Map (ASM), using as key the provided
        /// identity and facet. Adding a servant with an identity and facet that are already in the ASM throws
        /// ArgumentException.</summary>
        /// <param name="identityAndFacet">A relative URI string [category/]identity[#facet].</param>
        /// <param name="servant">The servant to add.</param>
        /// <param name="proxyFactory">The proxy factory used to manufacture the returned proxy. Pass INamePrx.Factory
        /// for this parameter. See <see cref="CreateProxy{T}(string, ProxyFactory{T})"/>.</param>
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
        /// for this parameter. See <see cref="CreateProxy{T}(Identity, string, ProxyFactory{T})"/>.
        /// </param>
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
                if (_identityServantMap.TryGetValue((identity, facet), out IObject? servant))
                {
                    _identityServantMap.Remove((identity, facet));
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
                _categoryServantMap.Add((category, facet), servant);
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
                if (_categoryServantMap.TryGetValue((category, facet), out IObject? servant))
                {
                    _categoryServantMap.Remove((category, facet));
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
            CheckIdentity(identity);

            lock (_mutex)
            {
                if (_disposeTask != null)
                {
                    throw new ObjectDisposedException($"{typeof(ObjectAdapter).FullName}:{Name}");
                }

                ImmutableArray<string> location = ReplicaGroupId.Length > 0 ? ImmutableArray.Create(ReplicaGroupId) :
                    AdapterId.Length > 0 ? ImmutableArray.Create(AdapterId) : ImmutableArray<string>.Empty;

                Protocol protocol = _publishedEndpoints.Count > 0 ? _publishedEndpoints[0].Protocol : Protocol;

                return factory(new Reference(Communicator,
                                             protocol.GetEncoding(),
                                             endpoints: AdapterId.Length == 0 ?
                                                _publishedEndpoints : ImmutableArray<Endpoint>.Empty,
                                             facet,
                                             identity,
                                             _invocationMode,
                                             location,
                                             protocol));
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

        /// <summary>Retrieves the endpoints configured with this object adapter.</summary>
        /// <returns>The endpoints.</returns>
        // TODO: do we need this method? If yes, provide better documentation.
        public IReadOnlyList<Endpoint> GetEndpoints()
        {
            lock (_mutex)
            {
                return _incomingConnectionFactories.Select(factory => factory.PublishedEndpoint).ToImmutableArray();
            }
        }

        /// <summary>Refreshes the set of published endpoints. The runtime rereads the PublishedEndpoints property
        /// (if set) and rereads the list of local interfaces if the adapter is configured to listen on all endpoints.
        /// This method is useful when the network interfaces of the host changes: it allows you to refresh the
        /// endpoint information published in the proxies created by this object adapter.</summary>
        /// <param name="cancel">The cancellation token.</param>
        public void RefreshPublishedEndpoints(CancellationToken cancel = default)
        {
            try
            {
                RefreshPublishedEndpointsAsync(cancel).Wait(cancel);
            }
            catch (AggregateException ex)
            {
                Debug.Assert(ex.InnerException != null);
                throw ExceptionUtil.Throw(ex.InnerException);
            }
        }

        /// <summary>Refreshes the set of published endpoints. The Ice runtime rereads the PublishedEndpoints property
        /// (if set) and rereads the list of local interfaces if the adapter is configured to listen on all endpoints.
        /// This method is useful when the network interfaces of the host changes: it allows you to refresh the
        /// endpoint information published in the proxies created by this object adapter.</summary>
        /// <param name="cancel">The cancellation token.</param>
        public async Task RefreshPublishedEndpointsAsync(CancellationToken cancel = default)
        {
            IReadOnlyList<Endpoint> publishedEndpoints = _routerInfo == null ? ComputePublishedEndpoints() :
                await _routerInfo.Router.GetServerEndpointsAsync(cancel).ConfigureAwait(false);

            // We always call the LocatorRegistry, even nothing changed.
            if (publishedEndpoints.Count > 0)
            {
                await RegisterEndpointsAsync(publishedEndpoints, cancel).ConfigureAwait(false);
            }
            else
            {
                // This means the object adapter lost all its published endpoints, which should be very unusual.
                await UnregisterEndpointsAsync(cancel).ConfigureAwait(false);
            }

            lock (_mutex)
            {
                if (_disposeTask != null)
                {
                    throw new ObjectDisposedException($"{typeof(ObjectAdapter).FullName}:{Name}");
                }

                _publishedEndpoints = publishedEndpoints;
            }
        }

        /// <summary>Sets the endpoints that from now on will be listed in the proxies created by this object adapter.
        /// </summary>
        /// <param name="newEndpoints">The new published endpoints.</param>
        /// <param name="cancel">The cancellation token.</param>
        public void SetPublishedEndpoints(IEnumerable<Endpoint> newEndpoints, CancellationToken cancel = default)
        {
            try
            {
                SetPublishedEndpointsAsync(newEndpoints, cancel).Wait(cancel);
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
        /// <param name="cancel">The cancellation token.</param>
        public async Task SetPublishedEndpointsAsync(
            IEnumerable<Endpoint> newEndpoints,
            CancellationToken cancel = default)
        {
            if (Name.Length == 0)
            {
                throw new InvalidOperationException("cannot set published endpoints on a nameless object adapter");
            }

            IReadOnlyList<Endpoint> publishedEndpoints = newEndpoints.ToImmutableArray();

            if (publishedEndpoints.Count == 0)
            {
                throw new ArgumentException("the new endpoints cannot be empty", nameof(newEndpoints));
            }

            if (publishedEndpoints.Select(endpoint => endpoint.Protocol).Distinct().Count() > 1)
            {
                throw new ArgumentException("all endpoints must use the same protocol", nameof(newEndpoints));
            }

            if (_routerInfo != null)
            {
                throw new InvalidOperationException(
                    "cannot set published endpoints on an object adapter associated with a router");
            }

            await RegisterEndpointsAsync(publishedEndpoints, cancel).ConfigureAwait(false);

            lock (_mutex)
            {
                if (_disposeTask != null)
                {
                    throw new ObjectDisposedException($"{typeof(ObjectAdapter).FullName}:{Name}");
                }

                _publishedEndpoints = publishedEndpoints;
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

            AdapterId = "";
            ReplicaGroupId = "";
            _acm = Communicator.ServerAcm;
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

            AdapterId = Communicator.GetProperty($"{Name}.AdapterId") ?? "";
            ReplicaGroupId = Communicator.GetProperty($"{Name}.ReplicaGroupId") ?? "";

            _acm = new Acm(Communicator, $"{Name}.ACM", Communicator.ServerAcm);
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
                        throw new ArgumentException($"router `{router}' already registered with an object adapter",
                            nameof(router));
                    }

                    // Associate this object adapter with the router. This way, new outgoing connections to the
                    // router's client proxy will use this object adapter for callbacks.
                    _routerInfo.Adapter = this;

                    // Also modify all existing outgoing connections to the router's client proxy to use this object
                    // adapter for callbacks.

                    // Often makes a synchronous remote call.
                    Communicator.OutgoingConnectionFactory.SetRouterInfo(_routerInfo);

                    // Synchronous remote call!
                    _publishedEndpoints = router.GetServerEndpoints();
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
                                    new AcceptorIncomingConnectionFactory(this, expanded, publishedEndpoint, _acm))));
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
                            Communicator.Logger.Trace(TraceLevels.TransportCategory,
                                                      $"created adapter `{Name}' without endpoints");
                        }
                    }

                    _publishedEndpoints = ComputePublishedEndpoints();
                }

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
                        ReplicaGroupId.Length == 0 ? RetryPolicy.NoRetry : RetryPolicy.OtherReplica);
                }

                // TODO: support input streamable data if Current.EndOfStream == false and output streamable data.

                ValueTask<OutgoingResponseFrame> DispatchAsync(int i)
                {
                    if (i < _interceptors.Count)
                    {
                        DispatchInterceptor interceptor = _interceptors[i++];
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
                    // TODO: ACM configuration?
                    _colocatedConnectionFactory = new AcceptorIncomingConnectionFactory(this,
                                                                                        new ColocatedEndpoint(this),
                                                                                        null,
                                                                                        new Acm());

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
                    return _identityServantMap.ContainsKey((r.Identity, r.Facet));
                }
            }
            else if (r.IsIndirect)
            {
                // Proxy is local if the reference's location matches this adapter id or replica group id.
                return r.Location.Count == 1 && (r.Location[0] == AdapterId || r.Location[0] == ReplicaGroupId);
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

        private static void CheckIdentity(Identity identity)
        {
            if (identity.Name.Length == 0)
            {
                throw new ArgumentException("identity name cannot be empty", nameof(identity));
            }
        }

        private IReadOnlyList<Endpoint> ComputePublishedEndpoints()
        {
            Debug.Assert(_routerInfo == null);
            IReadOnlyList<Endpoint> endpoints = ImmutableArray<Endpoint>.Empty;

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

            if (endpoints.Count == 0)
            {
                // If the PublishedEndpoints property isn't set, we compute the published endpoints from the OA
                // endpoints, expanding any endpoint that may be listening on INADDR_ANY to include actual addresses
                // in the published endpoints.
                // We also filter out duplicate endpoints, this might occur if an endpoint with a DNS name
                // expands to multiple addresses. In this case, multiple incoming connection factories can point to
                // the same published endpoint.

                endpoints = _incomingConnectionFactories.SelectMany(factory =>
                    factory.PublishedEndpoint.ExpandIfWildcard()).Distinct().ToImmutableArray();
            }

            if (Communicator.TraceLevels.Transport >= 1 && endpoints.Count > 0)
            {
                var sb = new StringBuilder("published endpoints for object adapter `");
                sb.Append(Name);
                sb.Append("':\n");
                sb.AppendEndpointList(endpoints);
                Communicator.Logger.Trace(TraceLevels.TransportCategory, sb.ToString());
            }

            return endpoints;
        }

        private async Task RegisterEndpointsAsync(IReadOnlyList<Endpoint> endpoints, CancellationToken cancel)
        {
            // Use a stack copy in case another thread updates volatile _locatorInfo.
            LocatorInfo? locatorInfo = _locatorInfo;

            if (endpoints.Count == 0 || AdapterId.Length == 0 || locatorInfo == null)
            {
                return; // Nothing to update.
            }

            ILocatorRegistryPrx? locatorRegistry = await locatorInfo.GetLocatorRegistryAsync().ConfigureAwait(false);

            if (locatorRegistry == null)
            {
                return;
            }

            try
            {
                if (Protocol == Protocol.Ice1)
                {
                    IObjectPrx proxy = IObjectPrx.Factory(new Reference(Communicator,
                                                                        Protocol.GetEncoding(),
                                                                        endpoints,
                                                                        facet: "",
                                                                        new Identity("dummy", ""),
                                                                        invocationMode: default,
                                                                        location: ImmutableArray<string>.Empty,
                                                                        protocol: endpoints[0].Protocol));
                    if (ReplicaGroupId.Length > 0)
                    {
                        await locatorRegistry.SetReplicatedAdapterDirectProxyAsync(
                            AdapterId,
                            ReplicaGroupId,
                            proxy,
                            cancel: cancel).ConfigureAwait(false);
                    }
                    else
                    {
                        await locatorRegistry.SetAdapterDirectProxyAsync(AdapterId,
                                                                         proxy,
                                                                         cancel: cancel).ConfigureAwait(false);
                    }
                }
                else
                {
                    await locatorRegistry.RegisterAdapterEndpointsAsync(
                            AdapterId,
                            ReplicaGroupId,
                            endpoints.ToEndpointDataList(),
                            cancel: cancel).ConfigureAwait(false);
                }
            }
            catch (Exception ex)
            {
                if (Communicator.TraceLevels.Locator >= 1)
                {
                    Communicator.Logger.Trace(
                        TraceLevels.LocatorCategory,
                        @$"failed to register the endpoints of object adapter `{
                            Name}' with the locator registry:\n{ex}");
                }
                throw;
            }

            if (Communicator.TraceLevels.Locator >= 1)
            {
                var sb = new StringBuilder("registered the endpoints of object adapter `");
                sb.Append(Name);
                sb.Append("' with the locator registry\nendpoints = ");
                sb.AppendEndpointList(endpoints);

                Communicator.Logger.Trace(TraceLevels.LocatorCategory, sb.ToString());
            }
        }

        private async Task UnregisterEndpointsAsync(CancellationToken cancel)
        {
            // Use a stack copy in case another thread updates volatile _locatorInfo.
            LocatorInfo? locatorInfo = _locatorInfo;

            if (AdapterId.Length == 0 || locatorInfo == null)
            {
                return; // Nothing to update.
            }

            ILocatorRegistryPrx? locatorRegistry = await locatorInfo.GetLocatorRegistryAsync().ConfigureAwait(false);

            if (locatorRegistry == null)
            {
                return;
            }

            try
            {
                if (Protocol == Protocol.Ice1)
                {
                    if (ReplicaGroupId.Length > 0)
                    {
                        await locatorRegistry.SetReplicatedAdapterDirectProxyAsync(
                            AdapterId,
                            ReplicaGroupId,
                            proxy: null,
                            cancel: cancel).ConfigureAwait(false);
                    }
                    else
                    {
                        await locatorRegistry.SetAdapterDirectProxyAsync(AdapterId,
                                                                         proxy: null,
                                                                         cancel: cancel).ConfigureAwait(false);
                    }
                }
                else
                {
                    await locatorRegistry.UnregisterAdapterEndpointsAsync(
                            AdapterId,
                            ReplicaGroupId,
                            cancel: cancel).ConfigureAwait(false);
                }
            }
            catch (ObjectDisposedException)
            {
                // Expected if colocated call and OA is deactivated or the communicator is disposed, ignore.
            }
            catch (Exception ex)
            {
                if (Communicator.TraceLevels.Locator >= 1)
                {
                    Communicator.Logger.Trace(
                        TraceLevels.LocatorCategory,
                        @$"failed to unregister the endpoints of object adapter `{
                            Name}' from the locator registry:\n{ex}");
                }
                throw;
            }

            if (Communicator.TraceLevels.Locator >= 1)
            {
                Communicator.Logger.Trace(
                    TraceLevels.LocatorCategory,
                    $"unregistered the endpoints of object adapter `{Name}' from the locator registry");
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
    }
}
