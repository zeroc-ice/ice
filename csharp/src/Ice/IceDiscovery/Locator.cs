// Copyright (c) ZeroC, Inc. All rights reserved.

using System;
using System.Collections.Generic;
using System.Collections.Immutable;
using System.Diagnostics;
using System.Linq;
using System.Threading;
using System.Threading.Tasks;

using ZeroC.Ice;

namespace ZeroC.IceDiscovery
{
    /// <summary>Servant class that implements the Slice interface Ice::Locator.</summary>
    internal class Locator : IAsyncLocator
    {
        private readonly string _domainId;
        private readonly int _latencyMultiplier;

        private readonly ILookupPrx _lookup;

        // The key is a single-endpoint datagram Lookup proxy extracted from the _lookup proxy.
        // The value is a dummy datagram proxy with usually a single endpoint that is one of _replyAdapter's endpoints
        // and that matches the interface of the key's endpoint.
        private readonly Dictionary<ILookupPrx, IObjectPrx> _lookups = new ();

        private readonly ObjectAdapter _replyAdapter;

        private readonly ILocatorRegistryPrx _registry;
        private readonly int _retryCount;
        private readonly TimeSpan _timeout;

        public async ValueTask<IObjectPrx?> FindAdapterByIdAsync(
            string adapterId,
            Current current,
            CancellationToken cancel)
        {
            using var replyServant = new LookupReply();
            return await InvokeAsync(
                async (lookup, dummyReply, replyIdentity) =>
                {
                    try
                    {
                        ILookupReplyPrx lookupReply =
                            dummyReply.Clone(ILookupReplyPrx.Factory, identity: replyIdentity);

                        await lookup.FindAdapterByIdAsync(_domainId,
                                                          adapterId,
                                                          lookupReply,
                                                          cancel: cancel).ConfigureAwait(false);
                    }
                    catch (Exception ex)
                    {
                        // TODO: is InvalidOperationException really appropriate here?
                        throw new InvalidOperationException(
                            $"failed to lookup adapter `{adapterId}' with lookup proxy `{_lookup}'", ex);
                    }
                },
                replyServant.ReplyHandler).ConfigureAwait(false);

        }

        public async ValueTask<IObjectPrx?> FindObjectByIdAsync(
            Identity identity,
            Current current,
            CancellationToken cancel)
        {
            using var replyServant = new LookupReply();
            return await InvokeAsync(
                async (lookup, dummyReply, replyIdentity) =>
                {
                    try
                    {
                        ILookupReplyPrx lookupReply =
                            dummyReply.Clone(ILookupReplyPrx.Factory, identity: replyIdentity);

                        await lookup.FindObjectByIdAsync(_domainId,
                                                         identity,
                                                         lookupReply,
                                                         cancel: cancel).ConfigureAwait(false);
                    }
                    catch (Exception ex)
                    {
                        throw new InvalidOperationException(
                            $"failed to lookup object `{identity}' with lookup proxy `{_lookup}'", ex);
                    }
                },
                replyServant.ReplyHandler).ConfigureAwait(false);
        }

        public ValueTask<ILocatorRegistryPrx?> GetRegistryAsync(Current current, CancellationToken cancel) =>
            new (_registry);

        public async ValueTask<(IEnumerable<EndpointData>, IEnumerable<string>)> ResolveLocationAsync(
            string[] location,
            Current current,
            CancellationToken cancel)
        {
            if (location.Length == 0)
            {
                throw new InvalidArgumentException("location cannot be empty", nameof(location));
            }

            string adapterId = location[0];

            using var replyServant = new ResolveAdapterIdReply();

            IReadOnlyList<EndpointData> endpoints = await InvokeAsync(
                async (lookup, dummyReply, replyIdentity) =>
                {
                    try
                    {
                        IResolveAdapterIdReplyPrx reply =
                            dummyReply.Clone(IResolveAdapterIdReplyPrx.Factory, identity: replyIdentity);

                        await lookup.ResolveAdapterIdAsync(_domainId,
                                                            adapterId,
                                                            reply,
                                                            cancel: cancel).ConfigureAwait(false);
                    }
                    catch (Exception ex)
                    {
                        // TODO: is InvalidOperationException really appropriate here?
                        throw new InvalidOperationException(
                            $"failed to lookup adapter `{adapterId}' with lookup proxy `{_lookup}'", ex);
                    }
                },
                replyServant.ReplyHandler).ConfigureAwait(false);

            if (endpoints.Count > 0)
            {
                return (endpoints, location[1..]);
            }
            else
            {
                return (endpoints, ImmutableArray<string>.Empty);
            }
        }

        public async ValueTask<(IEnumerable<EndpointData>, IEnumerable<string>)> ResolveWellKnownProxyAsync(
            Identity identity,
            Current current,
            CancellationToken cancel)
        {
            using var replyServant = new ResolveWellKnownProxyReply();

            (IReadOnlyList<EndpointData> endpoints, string adapterId) = await InvokeAsync(
                async (lookup, dummyReply, replyIdentity) =>
                {
                    try
                    {
                        IResolveWellKnownProxyReplyPrx reply =
                            dummyReply.Clone(IResolveWellKnownProxyReplyPrx.Factory, identity: replyIdentity);

                        await lookup.ResolveWellKnownProxyAsync(_domainId,
                                                                identity,
                                                                reply,
                                                                cancel: cancel).ConfigureAwait(false);
                    }
                    catch (Exception ex)
                    {
                        throw new InvalidOperationException(
                            $"failed to lookup object `{identity}' with lookup proxy `{_lookup}'", ex);
                    }
                },
                replyServant.ReplyHandler).ConfigureAwait(false);

            if (endpoints.Count > 0)
            {
                return (endpoints, ImmutableArray<string>.Empty);
            }
            else
            {
                return (endpoints, ImmutableArray.Create(adapterId));
            }
        }

        internal Locator(ILocatorRegistryPrx registry, ILookupPrx lookup, ObjectAdapter replyAdapter)
        {
            _lookup = lookup;
            _registry = registry;
            _replyAdapter = replyAdapter;

            Communicator communicator = replyAdapter.Communicator;

            _timeout = communicator.GetPropertyAsTimeSpan("IceDiscovery.Timeout") ?? TimeSpan.FromMilliseconds(300);
            if (_timeout == Timeout.InfiniteTimeSpan)
            {
                _timeout = TimeSpan.FromMilliseconds(300);
            }
            _retryCount = communicator.GetPropertyAsInt("IceDiscovery.RetryCount") ?? 3;

            _latencyMultiplier = communicator.GetPropertyAsInt("IceDiscovery.LatencyMultiplier") ?? 1;
            if (_latencyMultiplier < 1)
            {
                throw new InvalidConfigurationException(
                    "The value of `IceDiscovery.LatencyMultiplier' must be a positive integer greater than 0");
            }

            _domainId = communicator.GetProperty("IceDiscovery.DomainId") ?? "";

            // Create one lookup proxy per endpoint from the given proxy. We want to send a multicast datagram on each
            // of the lookup proxy.

            // Dummy proxy for replies which can have multiple endpoints (but see below).
            IObjectPrx lookupReply = _replyAdapter.CreateProxy(
                "dummy",
                IObjectPrx.Factory).Clone(invocationMode: InvocationMode.Datagram);

            foreach (Endpoint endpoint in lookup.Endpoints)
            {
                // lookup's invocation mode is Datagram
                Debug.Assert(endpoint.Transport == Transport.UDP);

                ILookupPrx key = lookup.Clone(endpoints: ImmutableArray.Create(endpoint));
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
        }

        /// <summary>Invokes a find or resolve request on a Lookup object and processes the reply(ies).</summary>
        /// <param name="find">A delegate that performs the remote call. The first two parameters correspond to an
        /// entry in the _lookups dictionary, and the third parameter is the unique identity of the reply object
        /// registered with the _replyAdapter.</param>
        /// <param name="replyHandler">The reply handler.</param>
        private async Task<TResult> InvokeAsync<TResult>(
            Func<ILookupPrx, IObjectPrx, Identity, Task> find,
            ReplyHandler<TResult> replyHandler)
        {
            Identity replyIdentity =
                _replyAdapter.AddWithUUID(replyHandler.Servant, ILocatorRegistryPrx.Factory).Identity;

            try
            {
                for (int i = 0; i < _retryCount; ++i)
                {
                    TimeSpan start = Time.Elapsed;
                    int failureCount = 0;
                    foreach ((ILookupPrx lookup, IObjectPrx dummyReply) in _lookups)
                    {
                        try
                        {
                            await find(lookup, dummyReply, replyIdentity);
                        }
                        catch (Exception ex)
                        {
                            if (++failureCount == _lookups.Count)
                            {
                                _replyAdapter.Communicator.Logger.Warning(ex.ToString());
                                replyHandler.SetEmptyResult();
                            }
                        }
                    }

                    Task? t = await Task.WhenAny(replyHandler.Task,
                        Task.Delay(_timeout, replyHandler.CancellationToken)).ConfigureAwait(false);
                    if (t == replyHandler.Task)
                    {
                        return await replyHandler.Task.ConfigureAwait(false);
                    }
                    else if (t.IsCanceled)
                    {
                        // If the timeout was canceled we delay the completion of the request to give a chance to other
                        // members of this replica group to reply
                        return await replyHandler.WaitForReplicaGroupRepliesAsync(start, _latencyMultiplier)
                            .ConfigureAwait(false);
                    }
                    // else timeout, so we retry until _retryCount
                }
                replyHandler.SetEmptyResult(); // Timeout
                return await replyHandler.Task.ConfigureAwait(false);
            }
            finally
            {
                _replyAdapter.Remove(replyIdentity);
            }
        }
    }

    /// <summary>Helps collect / gather the reply(ies) to a lookup request.</summary>
    internal sealed class ReplyHandler<TResult> : IDisposable
    {
        internal CancellationToken CancellationToken => _cancellationSource.Token;
        internal Task<TResult> Task => _completionSource.Task;
        internal IObject Servant { get; }

        private readonly CancellationTokenSource _cancellationSource;
        private readonly TaskCompletionSource<TResult> _completionSource;

        private readonly Func<TResult> _emptyResult;
        private readonly Func<TResult>? _replicaReplyCollector;

        public void Dispose() => _cancellationSource.Dispose();

        internal ReplyHandler(IObject servant, Func<TResult> emptyResult, Func<TResult>? replicaReplyCollector = null)
        {
            Servant = servant;
            _cancellationSource = new CancellationTokenSource();
            _completionSource = new TaskCompletionSource<TResult>();
            _emptyResult = emptyResult;
            _replicaReplyCollector = replicaReplyCollector;
        }

        internal void Cancel() => _cancellationSource.Cancel();

        internal void SetEmptyResult() => _completionSource.SetResult(_emptyResult());
        internal void SetResult(TResult result) => _completionSource.SetResult(result);

        internal async Task<TResult> WaitForReplicaGroupRepliesAsync(TimeSpan start, int latencyMultiplier)
        {
            Debug.Assert(_replicaReplyCollector != null);

            // This method is called by InvokeAsync after the first reply from a replica group to wait for additional
            // replies from the replica group.
            TimeSpan latency = (Time.Elapsed - start) * latencyMultiplier;
            if (latency == TimeSpan.Zero)
            {
                latency = TimeSpan.FromMilliseconds(1);
            }
            await System.Threading.Tasks.Task.Delay(latency);

            SetResult(_replicaReplyCollector());
            return await Task;
        }
    }

    /// <summary>Servant class that implements the Slice interface LookupReply.</summary>
    internal class LookupReply : ILookupReply, IDisposable
    {
        internal ReplyHandler<IObjectPrx?> ReplyHandler { get; }

        private readonly object _mutex = new object();
        private readonly HashSet<IObjectPrx> _proxies = new HashSet<IObjectPrx>();

        public void Dispose() => ReplyHandler.Dispose();

        public void FoundObjectById(Identity id, IObjectPrx proxy, Current current, CancellationToken cancel) =>
            ReplyHandler.SetResult(proxy);

        public void FoundAdapterById(
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
                        ReplyHandler.Cancel();
                    }
                }
            }
            else
            {
                ReplyHandler.SetResult(proxy);
            }
        }

        internal LookupReply() =>
            ReplyHandler = new (this, () => null, CollectReplicaReplies);

        private IObjectPrx? CollectReplicaReplies()
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

    /// <summary>Servant class that implements the Slice interface ResolveAdapterIdReply.</summary>
    internal class ResolveAdapterIdReply : IResolveAdapterIdReply, IDisposable
    {
        internal ReplyHandler<IReadOnlyList<EndpointData>> ReplyHandler { get; }

        private readonly object _mutex = new object();

        private readonly HashSet<EndpointData> _endpointDataSet = new (EndpointDataComparer.Instance);

        public void Dispose() => ReplyHandler.Dispose();

        public void Found(
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
                        ReplyHandler.Cancel();
                    }
                }
            }
            else
            {
                ReplyHandler.SetResult(endpoints);
            }
        }

        internal ResolveAdapterIdReply() =>
            ReplyHandler = new (this, () => ImmutableArray<EndpointData>.Empty, CollectReplicaReplies);

        private IReadOnlyList<EndpointData> CollectReplicaReplies()
        {
            lock (_mutex)
            {
                Debug.Assert(_endpointDataSet.Count > 0);
                return _endpointDataSet.ToList();
            }
        }
    }

    /// <summary>Servant class that implements the Slice interface ResolveWellKnownProxyReply.</summary>
    internal class ResolveWellKnownProxyReply : IResolveWellKnownProxyReply, IDisposable
    {
        internal ReplyHandler<(IReadOnlyList<EndpointData>, string)> ReplyHandler { get; }

        public void Dispose() => ReplyHandler.Dispose();

        public void FoundAdapterId(string adapterId, Current current, CancellationToken cancel) =>
            ReplyHandler.SetResult((ImmutableArray<EndpointData>.Empty, adapterId));
        public void FoundEndpoints(EndpointData[] endpoints, Current current, CancellationToken cancel) =>
            ReplyHandler.SetResult((endpoints, ""));

        internal ResolveWellKnownProxyReply() =>
            ReplyHandler = new (this, () => (ImmutableArray<EndpointData>.Empty, ""));
    }

    // Temporary helper class
    internal sealed class EndpointDataComparer : IEqualityComparer<EndpointData>
    {
        internal static readonly EndpointDataComparer Instance = new ();

        public bool Equals(EndpointData x, EndpointData y) =>
            x.Transport == y.Transport &&
            x.Host == y.Host &&
            x.Port == y.Port &&
            x.Options.SequenceEqual(y.Options);

        public int GetHashCode(EndpointData obj)
        {
            var hash = new HashCode();
            hash.Add(obj.Transport);
            hash.Add(obj.Host);
            hash.Add(obj.Port);
            foreach (string s in obj.Options)
            {
                hash.Add(s);
            }
            return hash.ToHashCode();
        }
    }
}
