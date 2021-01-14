// Copyright (c) ZeroC, Inc. All rights reserved.

using System;
using System.Collections.Generic;
using System.Collections.Immutable;
using System.Diagnostics;
using System.Linq;
using System.Threading;
using System.Threading.Tasks;

namespace ZeroC.Ice.Discovery
{
    /// <summary>Servant class that implements the Slice interface Ice::Locator.</summary>
    internal class Locator : IAsyncLocator
    {
        internal ILocatorPrx Proxy { get; }

        private readonly string _domainId;
        private readonly int _latencyMultiplier;

        private readonly ObjectAdapter _locatorAdapter;

        private readonly ILookupPrx _lookup;

        // The key is a single-endpoint datagram Lookup proxy extracted from the _lookup proxy.
        // The value is a dummy datagram proxy with usually a single endpoint that is one of _replyAdapter's endpoints
        // and that matches the interface of the key's endpoint.
        private readonly Dictionary<ILookupPrx, IObjectPrx> _lookups = new();

        private readonly ObjectAdapter _multicastAdapter;

        private readonly ILocatorRegistryPrx _registry;

        private readonly ObjectAdapter _replyAdapter;
        private readonly int _retryCount;
        private readonly TimeSpan _timeout;

        public async ValueTask<IObjectPrx?> FindAdapterByIdAsync(
            string adapterId,
            Current current,
            CancellationToken cancel)
        {
            using var replyServant = new FindAdapterByIdReply(_replyAdapter);
            return await InvokeAsync(
                (lookup, dummyReply) =>
                {
                    IFindAdapterByIdReplyPrx reply =
                        dummyReply.Clone(IFindAdapterByIdReplyPrx.Factory, identity: replyServant.Identity);

                    return lookup.FindAdapterByIdAsync(_domainId,
                                                      adapterId,
                                                      reply,
                                                      cancel: cancel);
                },
                replyServant).ConfigureAwait(false);
        }

        public async ValueTask<IObjectPrx?> FindObjectByIdAsync(
            Identity identity,
            string? facet,
            Current current,
            CancellationToken cancel)
        {
            using var replyServant = new FindObjectByIdReply(_replyAdapter);
            return await InvokeAsync(
                (lookup, dummyReply) =>
                {
                    IFindObjectByIdReplyPrx reply =
                        dummyReply.Clone(IFindObjectByIdReplyPrx.Factory, identity: replyServant.Identity);

                    return lookup.FindObjectByIdAsync(_domainId, identity, facet, reply, cancel: cancel);
                },
                replyServant).ConfigureAwait(false);
        }

        public ValueTask<ILocatorRegistryPrx?> GetRegistryAsync(Current current, CancellationToken cancel) =>
            new(_registry);

        public async ValueTask<IEnumerable<EndpointData>> ResolveLocationAsync(
            string[] location,
            Current current,
            CancellationToken cancel)
        {
            if (location.Length == 0)
            {
                throw new InvalidArgumentException("location cannot be empty", nameof(location));
            }
            else if (location.Length > 1)
            {
                // Ice discovery supports only single-segment locations.
                return ImmutableArray<EndpointData>.Empty;
            }

            string adapterId = location[0];

            using var replyServant = new ResolveAdapterIdReply(_replyAdapter);

            return await InvokeAsync(
                (lookup, dummyReply) =>
                {
                    IResolveAdapterIdReplyPrx reply =
                        dummyReply.Clone(IResolveAdapterIdReplyPrx.Factory, identity: replyServant.Identity);

                    return lookup.ResolveAdapterIdAsync(_domainId,
                                                        adapterId,
                                                        reply,
                                                        cancel: cancel);
                },
                replyServant).ConfigureAwait(false);
        }

        public async ValueTask<(IEnumerable<EndpointData>, IEnumerable<string>)> ResolveWellKnownProxyAsync(
            Identity identity,
            string facet,
            Current current,
            CancellationToken cancel)
        {
            using var replyServant = new ResolveWellKnownProxyReply(_replyAdapter);

            string adapterId = await InvokeAsync(
                (lookup, dummyReply) =>
                {
                    IResolveWellKnownProxyReplyPrx reply =
                            dummyReply.Clone(IResolveWellKnownProxyReplyPrx.Factory, identity: replyServant.Identity);

                    return lookup.ResolveWellKnownProxyAsync(_domainId,
                                                             identity,
                                                             facet,
                                                             reply,
                                                             cancel: cancel);
                },
                replyServant).ConfigureAwait(false);

            // We never return endpoints
            return (ImmutableArray<EndpointData>.Empty,
                    adapterId.Length > 0 ? ImmutableArray.Create(adapterId) : ImmutableArray<string>.Empty);
        }

        internal Locator(Communicator communicator)
        {
            const string defaultIPv4Endpoint = "udp -h 239.255.0.1 -p 4061";
            const string defaultIPv6Endpoint = "udp -h \"ff15::1\" -p 4061";

            if (communicator.GetProperty("Ice.Discovery.Multicast.Endpoints") == null)
            {
                communicator.SetProperty("Ice.Discovery.Multicast.Endpoints",
                                          $"{defaultIPv4Endpoint}:{defaultIPv6Endpoint}");
            }
            communicator.SetProperty("Ice.Discovery.Multicast.AcceptNonSecure", "Always");

            if (communicator.GetProperty("Ice.Discovery.Reply.Endpoints") == null)
            {
                communicator.SetProperty("Ice.Discovery.Reply.Endpoints", "udp -h \"::0\" -p 0");
            }

            // create datagram proxies
            communicator.SetProperty("Ice.Discovery.Reply.ProxyOptions", "-d");
            // datagram connection are nonsecure
            communicator.SetProperty("Ice.Discovery.Reply.AcceptNonSecure", "Always");

            _timeout = communicator.GetPropertyAsTimeSpan("Ice.Discovery.Timeout") ?? TimeSpan.FromMilliseconds(300);
            if (_timeout == Timeout.InfiniteTimeSpan)
            {
                _timeout = TimeSpan.FromMilliseconds(300);
            }

            _retryCount = communicator.GetPropertyAsInt("Ice.Discovery.RetryCount") ?? 3;

            _latencyMultiplier = communicator.GetPropertyAsInt("Ice.Discovery.LatencyMultiplier") ?? 1;
            if (_latencyMultiplier < 1)
            {
                throw new InvalidConfigurationException(
                    "the value of Ice.Discovery.LatencyMultiplier must be an integer greater than 0");
            }

            _domainId = communicator.GetProperty("Ice.Discovery.DomainId") ?? "";

            string? lookupEndpoints = communicator.GetProperty("Ice.Discovery.Lookup");
            if (lookupEndpoints == null)
            {
                var endpoints = new List<string>();
                List<string> ipv4Interfaces = Network.GetInterfacesForMulticast("0.0.0.0", Network.EnableIPv4);
                List<string> ipv6Interfaces = Network.GetInterfacesForMulticast("::0", Network.EnableIPv6);

                endpoints.AddRange(ipv4Interfaces.Select(i => $"{defaultIPv4Endpoint} --interface \"{i}\""));
                endpoints.AddRange(ipv6Interfaces.Select(i => $"{defaultIPv6Endpoint} --interface \"{i}\""));

                lookupEndpoints = string.Join(":", endpoints);
            }

            // Datagram proxies do not support SSL/TLS so they can only be used with PreferNonSecure set to
            // NonSecure.Always.
            _lookup = ILookupPrx.Parse($"IceDiscovery/Lookup -d:{lookupEndpoints}", communicator).Clone(
                clearRouter: true,
                invocationTimeout: _timeout,
                preferNonSecure: NonSecure.Always);

            _locatorAdapter = communicator.CreateObjectAdapter();
            Proxy = _locatorAdapter.AddWithUUID(this, ILocatorPrx.Factory);

            // Setup locator registry.
            var registryServant = new LocatorRegistry(communicator);
            _registry = _locatorAdapter.AddWithUUID(registryServant, ILocatorRegistryPrx.Factory);

            _multicastAdapter = communicator.CreateObjectAdapter("Ice.Discovery.Multicast");
            _replyAdapter = communicator.CreateObjectAdapter("Ice.Discovery.Reply");

            // Dummy proxy for replies which can have multiple endpoints (but see below).
            IObjectPrx lookupReply = _replyAdapter.CreateProxy("dummy", IObjectPrx.Factory);
            Debug.Assert(lookupReply.InvocationMode == InvocationMode.Datagram);

            // Create one lookup proxy per endpoint from the given proxy. We want to send a multicast datagram on
            // each of the lookup proxy.
            // TODO: this code is incorrect now that the default published endpoints are no longer an expansion
            // of the object adapter endpoints.
            foreach (Endpoint endpoint in _lookup.Endpoints)
            {
                if (!endpoint.IsDatagram)
                {
                    throw new InvalidConfigurationException("Ice.Discovery.Lookup can only have udp endpoints");
                }

                ILookupPrx key = _lookup.Clone(endpoints: ImmutableArray.Create(endpoint));
                if (endpoint["interface"] is string mcastInterface && mcastInterface.Length > 0)
                {
                    Endpoint? q = lookupReply.Endpoints.FirstOrDefault(e => e.Host == mcastInterface);
                    if (q != null)
                    {
                        _lookups[key] = lookupReply.Clone(endpoints: ImmutableArray.Create(q));
                    }
                }

                if (!_lookups.ContainsKey(key))
                {
                    // Fallback: just use the given lookup reply proxy if no matching endpoint found.
                    _lookups[key] = lookupReply;
                }
            }
            Debug.Assert(_lookups.Count > 0);

            // Add lookup Ice object
            _multicastAdapter.Add("IceDiscovery/Lookup", new Lookup(registryServant, communicator));
        }

        internal Task ActivateAsync(CancellationToken cancel) =>
            Task.WhenAll(_locatorAdapter.ActivateAsync(cancel),
                         _multicastAdapter.ActivateAsync(cancel),
                         _replyAdapter.ActivateAsync(cancel));

        /// <summary>Invokes a find or resolve request on a Lookup object and processes the reply(ies).</summary>
        /// <param name="findAsync">A delegate that performs the remote call. Its parameters correspond to an entry in
        /// the _lookups dictionary.</param>
        /// <param name="replyServant">The reply servant.</param>
        private async Task<TResult> InvokeAsync<TResult>(
            Func<ILookupPrx, IObjectPrx, Task> findAsync,
            ReplyServant<TResult> replyServant)
        {
            // We retry only when at least one findAsync request is sent successfully and we don't get any reply.
            // TODO: this _retryCount is really an attempt count not a retry count.
            for (int i = 0; i < _retryCount; ++i)
            {
                TimeSpan start = Time.Elapsed;

                var timeoutTask = Task.Delay(_timeout, replyServant.CancellationToken);

                var sendTask = Task.WhenAll(_lookups.Select(
                    entry =>
                    {
                        try
                        {
                            return findAsync(entry.Key, entry.Value);
                        }
                        catch (Exception ex)
                        {
                            return Task.FromException(ex);
                        }
                    }));

                Task task = await Task.WhenAny(sendTask, replyServant.Task, timeoutTask).ConfigureAwait(false);

                if (task == sendTask)
                {
                    if (sendTask.Status == TaskStatus.Faulted)
                    {
                        if (sendTask.Exception!.InnerExceptions.Count == _lookups.Count)
                        {
                            // All the tasks failed: log warning and return empty result (no retry)
                            _replyAdapter.Communicator.Logger.Warning(
                                @$"Ice discovery failed to send lookup request using `{_lookup
                                    }':\n{sendTask.Exception!.InnerException!}");
                            replyServant.SetEmptyResult();
                            return await replyServant.Task.ConfigureAwait(false);
                        }
                    }
                    // For Canceled or RanToCompletion, we assume at least one send was successful. If we're wrong,
                    // we'll timeout soon anyways.

                    task = await Task.WhenAny(replyServant.Task, timeoutTask).ConfigureAwait(false);
                }

                if (task == replyServant.Task)
                {
                    return await replyServant.Task.ConfigureAwait(false);
                }
                else if (task.IsCanceled)
                {
                    // If the timeout was canceled we delay the completion of the request to give a chance to other
                    // members of this replica group to reply
                    return await
                        replyServant.GetReplicaGroupRepliesAsync(start, _latencyMultiplier).ConfigureAwait(false);
                }
                // else timeout, so we retry until _retryCount
            }

            replyServant.SetEmptyResult(); // _retryCount exceeded
            return await replyServant.Task.ConfigureAwait(false);
        }
    }

    /// <summary>The base class of all Reply servant that helps collect / gather the reply(ies) to a lookup reques.
    /// </summary>
    internal class ReplyServant<TResult> : IObject, IDisposable
    {
        internal CancellationToken CancellationToken => _cancellationSource.Token;
        internal Identity Identity { get; }

        internal Task<TResult> Task => _completionSource.Task;

        private readonly CancellationTokenSource _cancellationSource;
        private readonly TaskCompletionSource<TResult> _completionSource;
        private readonly TResult _emptyResult;

        private readonly ObjectAdapter _replyAdapter;

        public void Dispose()
        {
            _cancellationSource.Dispose();
            _replyAdapter.Remove(Identity);
        }

        internal async Task<TResult> GetReplicaGroupRepliesAsync(TimeSpan start, int latencyMultiplier)
        {
            // This method is called by InvokeAsync after the first reply from a replica group to wait for additional
            // replies from the replica group.
            TimeSpan latency = (Time.Elapsed - start) * latencyMultiplier;
            if (latency == TimeSpan.Zero)
            {
                latency = TimeSpan.FromMilliseconds(1);
            }
            await System.Threading.Tasks.Task.Delay(latency).ConfigureAwait(false);

            SetResult(CollectReplicaReplies());
            return await Task.ConfigureAwait(false);
        }

        internal void SetEmptyResult() => _completionSource.SetResult(_emptyResult);

        private protected ReplyServant(TResult emptyResult, ObjectAdapter replyAdapter)
        {
            // Add servant (this) to object adapter with new UUID identity.
            Identity = replyAdapter.AddWithUUID(this, IObjectPrx.Factory).Identity;

            _cancellationSource = new();
            _completionSource = new();
            _emptyResult = emptyResult;
            _replyAdapter = replyAdapter;
        }

        private protected void Cancel() => _cancellationSource.Cancel();

        private protected virtual TResult CollectReplicaReplies()
        {
            Debug.Assert(false); // must be overridden if called by WaitForReplicaGroupRepliesAsync
            return _emptyResult;
        }

        private protected void SetResult(TResult result) => _completionSource.SetResult(result);
    }

    /// <summary>Servant class that implements the Slice interface FindAdapterByIdReply.</summary>
    internal sealed class FindAdapterByIdReply : ReplyServant<IObjectPrx?>, IAsyncFindAdapterByIdReply
    {
        private readonly object _mutex = new();
        private readonly HashSet<IObjectPrx> _proxies = new();

        public ValueTask FoundAdapterByIdAsync(
            string adapterId,
            IObjectPrx proxy,
            bool isReplicaGroup,
            Current current,
            CancellationToken cancel)
        {
            if (isReplicaGroup)
            {
                lock (_mutex)
                {
                    _proxies.Add(proxy);
                    if (_proxies.Count == 1)
                    {
                        // Cancel WhenAny and let InvokeAsync wait for additional replies from the replica group, and
                        // later call CollectReplicaReplies.
                        Cancel();
                    }
                }
            }
            else
            {
                SetResult(proxy);
            }
            return default;
        }

        internal FindAdapterByIdReply(ObjectAdapter replyAdapter)
            : base(emptyResult: null, replyAdapter)
        {
        }

        private protected override IObjectPrx? CollectReplicaReplies()
        {
            lock (_mutex)
            {
                Debug.Assert(_proxies.Count > 0);
                var endpoints = new List<Endpoint>();
                IObjectPrx result = _proxies.First();
                foreach (IObjectPrx prx in _proxies)
                {
                    endpoints.AddRange(prx.Endpoints);
                }
                return result.Clone(endpoints: endpoints);
            }
        }
    }

    /// <summary>Servant class that implements the Slice interface FindObjectByIdReply.</summary>
    internal class FindObjectByIdReply : ReplyServant<IObjectPrx?>, IAsyncFindObjectByIdReply
    {
        public ValueTask FoundObjectByIdAsync(Identity id, IObjectPrx proxy, Current current, CancellationToken cancel)
        {
            SetResult(proxy);
            return default;
        }

        internal FindObjectByIdReply(ObjectAdapter replyAdapter)
            : base(emptyResult: null, replyAdapter)
        {
        }
    }

    /// <summary>Servant class that implements the Slice interface ResolveAdapterIdReply.</summary>
    internal sealed class ResolveAdapterIdReply : ReplyServant<IReadOnlyList<EndpointData>>, IAsyncResolveAdapterIdReply
    {
        private readonly object _mutex = new();
        private readonly HashSet<EndpointData> _endpointDataSet = new();

        public ValueTask FoundAdapterIdAsync(
            EndpointData[] endpoints,
            bool isReplicaGroup,
            Current current,
            CancellationToken cancel)
        {
            if (isReplicaGroup)
            {
                lock (_mutex)
                {
                    bool firstReply = _endpointDataSet.Count == 0;

                    _endpointDataSet.UnionWith(endpoints);
                    if (firstReply)
                    {
                        Cancel();
                    }
                }
            }
            else
            {
                SetResult(endpoints);
            }
            return default;
        }

        internal ResolveAdapterIdReply(ObjectAdapter replyAdapter)
            : base(ImmutableArray<EndpointData>.Empty, replyAdapter)
        {
        }

        private protected override IReadOnlyList<EndpointData> CollectReplicaReplies()
        {
            lock (_mutex)
            {
                Debug.Assert(_endpointDataSet.Count > 0);
                return _endpointDataSet.ToList();
            }
        }
    }

    /// <summary>Servant class that implements the Slice interface ResolveWellKnownProxyReply.</summary>
    internal class ResolveWellKnownProxyReply : ReplyServant<string>, IAsyncResolveWellKnownProxyReply
    {
        public ValueTask FoundWellKnownProxyAsync(string adapterId, Current current, CancellationToken cancel)
        {
            SetResult(adapterId);
            return default;
        }

        internal ResolveWellKnownProxyReply(ObjectAdapter replyAdapter)
            : base(emptyResult: "", replyAdapter)
        {
        }
    }
}
