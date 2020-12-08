// Copyright (c) ZeroC, Inc. All rights reserved.

using System;
using System.Collections.Generic;
using System.Collections.Immutable;
using System.Diagnostics;
using System.Linq;
using System.Text;
using System.Threading;
using System.Threading.Tasks;

namespace ZeroC.Ice.LocatorDiscovery
{
    /// <summary>Implements interface Ice::Locator by forwarding all calls to the discovered locator. We cannot simply
    /// forward the requests using ForwardAsync because we need to occasionally perform transcoding. This locator is
    /// hosted in an ice2 object adapter and typically receives 2.0-encoded requests, and the discovered locator proxy
    /// can be an ice1/1.1 proxy that understands only 1.1-encoded requests.</summary>
    internal class Locator : IAsyncLocator
    {
        internal ILocatorPrx Proxy { get; }

        private TaskCompletionSource<ILocatorPrx>? _completionSource;
        private Task<ILocatorPrx?>? _findLocatorTask;
        private string _instanceName;

        private ILocatorPrx? _locator;

        private readonly ObjectAdapter _locatorAdapter;

        private readonly ILookupPrx _lookup;
        private readonly Dictionary<ILookupPrx, ILookupReplyPrx> _lookups = new();

        private readonly string _lookupTraceCategory;
        private readonly int _lookupTraceLevel;
        private readonly object _mutex = new();
        private TimeSpan _nextRetry;

        private readonly ObjectAdapter _replyAdapter;

        private readonly int _retryCount;
        private readonly TimeSpan _retryDelay;
        private readonly TimeSpan _timeout;

        // "Overrides" the generated DispatchAsync to forward as-is when the encoding match (this includes unknown
        // operations and binary contexts). Otherwise, use the generated code to perform transcoding back and forth.
        public async ValueTask<OutgoingResponseFrame> DispatchAsync(
            IncomingRequestFrame request,
            Current current,
            CancellationToken cancel)
        {
            ILocatorPrx? locator = await GetLocatorAsync().ConfigureAwait(false);

            if (locator != null && current.Encoding == locator.Encoding)
            {
                return await ForwardRequestAsync(
                    locator =>
                    locator?.ForwardAsync(request, current.IsOneway, cancel: cancel).AsTask() ??
                        // In the unlikely event locator is now null (e.g. after a failed attempt), we use the
                        // "transcoding dispatch method" which will in turn return null/empty with a null locator.
                        // See comments below.
                        IAsyncLocator.DispatchAsync(this, request, current, cancel).AsTask()).ConfigureAwait(false);
            }
            else
            {
                // Calls the base DispatchAsync, which calls FindAdapterByIdAsync etc.
                // The transcoding is naturally limited to the known Ice::Locator operations. Other operations
                // cannot be transcoded and result in OperationNotExistException.
                return await IAsyncLocator.DispatchAsync(this, request, current, cancel).ConfigureAwait(false);
            }
        }

        // Forwards the request to the discovered locator; if this discovered locator is null, returns a null proxy.
        public ValueTask<IObjectPrx?> FindAdapterByIdAsync(
            string adapterId,
            Current current,
            CancellationToken cancel) =>
            ForwardRequestAsync(locator =>
                                locator?.FindAdapterByIdAsync(adapterId, current.Context, cancel: cancel) ??
                                    Task.FromResult<IObjectPrx?>(null));

        public ValueTask<IObjectPrx?> FindObjectByIdAsync(
            Identity identity,
            string? facet,
            Current current,
            CancellationToken cancel) =>
            ForwardRequestAsync(locator =>
                                locator?.FindObjectByIdAsync(identity, facet, cancel: cancel) ??
                                    Task.FromResult<IObjectPrx?>(null));
        public ValueTask<ILocatorRegistryPrx?> GetRegistryAsync(Current current, CancellationToken cancel) =>
            ForwardRequestAsync(locator =>
                                locator?.GetRegistryAsync(current.Context, cancel: cancel) ??
                                    Task.FromResult<ILocatorRegistryPrx?>(null));

        public ValueTask<IEnumerable<EndpointData>> ResolveLocationAsync(
            string[] location,
            Current current,
            CancellationToken cancel) =>
            ForwardRequestAsync<IEnumerable<EndpointData>>(
                async locator =>
                {
                    if (locator != null)
                    {
                        return await locator.ResolveLocationAsync(location,
                                                                  current.Context,
                                                                  cancel: cancel).ConfigureAwait(false);
                    }
                    else
                    {
                        return ImmutableArray<EndpointData>.Empty;
                    }
                });

        public ValueTask<(IEnumerable<EndpointData>, IEnumerable<string>)> ResolveWellKnownProxyAsync(
            Identity identity,
            string facet,
            Current current,
            CancellationToken cancel) =>
            ForwardRequestAsync<(IEnumerable<EndpointData>, IEnumerable<string>)>(
                async locator =>
                {
                    if (locator != null)
                    {
                        return await locator.ResolveWellKnownProxyAsync(
                            identity,
                            facet,
                            current.Context,
                            cancel: cancel).ConfigureAwait(false);
                    }
                    else
                    {
                        return (ImmutableArray<EndpointData>.Empty, ImmutableArray<string>.Empty);
                    }
                });

        internal Locator(Communicator communicator)
        {
            const string defaultIPv4Endpoint = "udp -h 239.255.0.1 -p 4061";
            const string defaultIPv6Endpoint = "udp -h \"ff15::1\" -p 4061";

            string? lookupEndpoints = communicator.GetProperty("Ice.LocatorDiscovery.Lookup");
            if (lookupEndpoints == null)
            {
                List<string> endpoints = new();
                List<string> ipv4Interfaces = Network.GetInterfacesForMulticast("0.0.0.0", Network.EnableIPv4);
                List<string> ipv6Interfaces = Network.GetInterfacesForMulticast("::0", Network.EnableIPv6);

                endpoints.AddRange(ipv4Interfaces.Select(i => $"{defaultIPv4Endpoint} --interface \"{i}\""));
                endpoints.AddRange(ipv6Interfaces.Select(i => $"{defaultIPv6Endpoint} --interface \"{i}\""));

                lookupEndpoints = string.Join(":", endpoints);
            }

            _timeout = communicator.GetPropertyAsTimeSpan("Ice.LocatorDiscovery.Timeout") ??
                 TimeSpan.FromMilliseconds(300);
            if (_timeout == Timeout.InfiniteTimeSpan)
            {
                _timeout = TimeSpan.FromMilliseconds(300);
            }

            _lookup = ILookupPrx.Parse($"IceLocatorDiscovery/Lookup -d:{lookupEndpoints}", communicator).Clone(
                    clearRouter: true,
                    invocationTimeout: _timeout,
                    preferNonSecure: NonSecure.Always);

            if (communicator.GetProperty("Ice.LocatorDiscovery.Reply.Endpoints") == null)
            {
                communicator.SetProperty("Ice.LocatorDiscovery.Reply.Endpoints", "udp -h \"::0\" -p 0");
            }
            communicator.SetProperty("Ice.LocatorDiscovery.Reply.ProxyOptions", "-d");
            communicator.SetProperty("Ice.LocatorDiscovery.Reply.AcceptNonSecure", "Always");

            _instanceName = communicator.GetProperty("Ice.LocatorDiscovery.InstanceName") ?? "";

            _replyAdapter = communicator.CreateObjectAdapter("Ice.LocatorDiscovery.Reply");
            _locatorAdapter = communicator.CreateObjectAdapter();

            var locatorIdentity = new Identity("Locator",
                                               _instanceName.Length > 0 ? _instanceName : Guid.NewGuid().ToString());
            Proxy = _locatorAdapter.Add(locatorIdentity, this, ILocatorPrx.Factory);

            var lookupReplyId = new Identity(Guid.NewGuid().ToString(), "");
            ILookupReplyPrx lookupReply = _replyAdapter.CreateProxy(lookupReplyId, ILookupReplyPrx.Factory);
            Debug.Assert(lookupReply.InvocationMode == InvocationMode.Datagram);

            _replyAdapter.Add(lookupReplyId, new LookupReply(this));

            _retryCount = Math.Max(communicator.GetPropertyAsInt("Ice.LocatorDiscovery.RetryCount") ?? 3, 1);
            _retryDelay = communicator.GetPropertyAsTimeSpan("Ice.LocatorDiscovery.RetryDelay") ??
                TimeSpan.FromMilliseconds(2000);
            _lookupTraceLevel = communicator.GetPropertyAsInt("Ice.LocatorDiscovery.Trace.Lookup") ?? 0;
            _lookupTraceCategory = "Ice.LocatorDiscovery.Lookup";

            // Create one lookup proxy per endpoint from the given proxy. We want to send a multicast datagram on each
            // endpoint.

            // TODO: revisit, as it no longer works properly with the new default published endpoints.
            foreach (Endpoint endpoint in _lookup.Endpoints)
            {
                if (!endpoint.IsDatagram)
                {
                    throw new InvalidConfigurationException("Ice.LocatorDiscovery.Lookup can only have udp endpoints");
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
        }

        internal void FoundLocator(ILocatorPrx locator)
        {
            lock (_mutex)
            {
                if (_instanceName.Length > 0 && locator.Identity.Category != _instanceName)
                {
                    if (_lookupTraceLevel > 2)
                    {
                        _lookup.Communicator.Logger.Trace(_lookupTraceCategory,
                            @$"ignoring locator reply: instance name doesn't match\nexpected = {_instanceName
                            } received = {locator.Identity.Category}");
                    }
                    return;
                }

                // If we already have a locator assigned, ensure the given locator has the same identity, facet and
                // protocol, otherwise ignore it.
                if (_locator != null)
                {
                    if (locator.Identity != _locator.Identity || locator.Facet != _locator.Facet)
                    {
                        var sb = new StringBuilder(
                            "Ice locator discovery: received Ice locator with different identities:\n");
                        sb.Append("using = `").Append(_locator).Append("'\n")
                          .Append("received = `").Append(locator).Append("'\n")
                          .Append("This is typically the case if multiple Ice locators with different ")
                          .Append("instance names are deployed and the property ")
                          .Append("`Ice.LocatorDiscovery.InstanceName' is not set.");
                        locator.Communicator.Logger.Warning(sb.ToString());
                        return;
                    }

                    if (locator.Protocol != _locator.Protocol)
                    {
                        var sb = new StringBuilder(
                            "Ice locator discovery: ignoring Ice locator with different protocol:\n");
                        sb.Append("using = ").Append(_locator.Protocol.GetName()).Append('\n')
                          .Append("received = ").Append(locator.Protocol.GetName()).Append('\n');
                        locator.Communicator.Logger.Warning(sb.ToString());
                        return;
                    }
                }

                if (_lookupTraceLevel > 0)
                {
                    var sb = new StringBuilder("locator lookup succeeded:\nlocator = ");
                    sb.Append(locator);
                    if (_instanceName.Length > 0)
                    {
                        sb.Append("\ninstance name = ").Append(_instanceName);
                    }

                    _lookup.Communicator.Logger.Trace(_lookupTraceCategory, sb.ToString());
                }

                if (_locator == null)
                {
                    _locator = locator;
                    if (_instanceName.Length == 0)
                    {
                        _instanceName = _locator.Identity.Category; // Stick to the first locator
                    }
                    Debug.Assert(_completionSource != null);
                    _completionSource.TrySetResult(locator);
                }
                else
                {
                    // We found another locator replica, append its endpoints to the current locator proxy endpoints,
                    // while eliminating duplicates.
                    _locator = _locator.Clone(endpoints: _locator.Endpoints.Concat(locator.Endpoints).Distinct());
                }
            }
        }

        internal Task ActivateAsync(CancellationToken cancel) =>
            Task.WhenAll(_locatorAdapter.ActivateAsync(cancel), _replyAdapter.ActivateAsync(cancel));

        private async Task<ILocatorPrx?> FindLocatorAsync()
        {
            lock (_mutex)
            {
                Debug.Assert(_locator == null);
                Debug.Assert(_findLocatorTask == null);
                _completionSource = new TaskCompletionSource<ILocatorPrx>();
            }

            if (_lookupTraceLevel > 1)
            {
                var sb = new StringBuilder("looking up locator:\nlookup = ");
                sb.Append(_lookup);
                if (_instanceName.Length > 0)
                {
                    sb.Append("\ninstance name = ").Append(_instanceName);
                }
                _lookup.Communicator.Logger.Trace(_lookupTraceCategory, sb.ToString());
            }

            // We retry only when at least one send succeeds and we don't get any reply.
            // TODO: this _retryCount is really an attempt count not a retry count.
            for (int i = 0; i < _retryCount; ++i)
            {
                var timeoutTask = Task.Delay(_timeout);

                var sendTask = Task.WhenAll(_lookups.Select(
                    entry =>
                    {
                        try
                        {
                            return entry.Key.FindLocatorAsync(_instanceName, entry.Value);
                        }
                        catch (Exception ex)
                        {
                            return Task.FromException(ex);
                        }
                    }));

                Task task = await Task.WhenAny(sendTask, _completionSource.Task, timeoutTask).ConfigureAwait(false);

                if (task == sendTask)
                {
                    if (sendTask.Status == TaskStatus.Faulted)
                    {
                        if (sendTask.Exception!.InnerExceptions.Count == _lookups.Count)
                        {
                            // All the tasks failed: trace and return null (no retry)
                            if (_lookupTraceLevel > 0)
                            {
                                var sb = new StringBuilder("locator lookup failed:\nlookup = ");
                                sb.Append(_lookup);
                                if (_instanceName.Length > 0)
                                {
                                    sb.Append("\ninstance name = ").Append(_instanceName);
                                }
                                sb.Append('\n');
                                sb.Append(sendTask.Exception!.InnerException!);
                                _lookup.Communicator.Logger.Trace(_lookupTraceCategory, sb.ToString());
                            }
                            return null;
                        }
                    }
                    // For Canceled or RanToCompletion, we assume at least one send was successful. If we're wrong,
                    // we'll timeout soon anyways.

                    task = await Task.WhenAny(_completionSource.Task, timeoutTask).ConfigureAwait(false);
                }
                // else, we either completed or timed out and don't care about sendTask anymore

                if (task == _completionSource.Task)
                {
                    return await _completionSource.Task.ConfigureAwait(false);
                }
                // else timeout, and retry until we reach _retryCount
            }

            lock (_mutex)
            {
                if (_completionSource.Task.IsCompleted)
                {
                    // we got a concurrent reply after the timeout
                    Debug.Assert(_locator != null);
                    return _locator;
                }
                else
                {
                    // Locator lookup timeout and no more retries
                    if (_lookupTraceLevel > 0)
                    {
                        var sb = new StringBuilder("locator lookup timed out:\nlookup = ");
                        sb.Append(_lookup);
                        if (_instanceName.Length > 0)
                        {
                            sb.Append("\ninstance name = ").Append(_instanceName);
                        }
                        _lookup.Communicator.Logger.Trace(_lookupTraceCategory, sb.ToString());
                    }

                    _nextRetry = Time.Elapsed + _retryDelay;
                    return null;
                }
            }
        }

        // This helper method calls "callAsync" with the discovered locator or null when no locator was discovered.
        private async ValueTask<TResult> ForwardRequestAsync<TResult>(Func<ILocatorPrx?, Task<TResult>> callAsync)
        {
            ILocatorPrx? badLocator = null;
            Exception? exception = null;
            while (true)
            {
                // Get the locator to send the request to (this will return null if no locator is found)
                ILocatorPrx? newLocator = await GetLocatorAsync().ConfigureAwait(false);
                if (newLocator != null && !newLocator.Equals(badLocator))
                {
                    try
                    {
                        return await callAsync(newLocator).ConfigureAwait(false);
                    }
                    catch (RemoteException ex)
                    {
                        // If we receive a RemoteException, we just forward it as-is to the caller (typically a
                        // colocated LocatorInfo).
                        ex.ConvertToUnhandled = false;
                        throw;
                    }
                    catch (Exception ex)
                    {
                        badLocator = newLocator;

                        // If we get some local exception, we attempt to find a new locator and try again.
                        // TODO: this could lead to an infinite loop if we keep alternating between 2 different
                        // locators.
                        lock (_mutex)
                        {
                            // If the current locator is equal to the one we use to send the request,
                            // clear it and retry, this will trigger the lookup of a new locator.
                            if (_locator != null && _locator.Equals(newLocator))
                            {
                                _locator = null;
                            }
                        }
                        exception = ex;
                    }
                }
                else
                {
                    if (exception != null)
                    {
                        // Could not find any locator or we got the same locator or a null locator after a failure.
                        _lookup.Communicator.Logger.Warning(
                            $"Ice locator discovery: failed to send request to discovered locator:\n{exception}");
                    }

                    return await callAsync(null).ConfigureAwait(false);
                }
            }
        }

        private async Task<ILocatorPrx?> GetLocatorAsync()
        {
            Task<ILocatorPrx?> findLocatorTask;
            lock (_mutex)
            {
                if (_locator != null)
                {
                    // If we already have a locator we use it.
                    return _locator;
                }
                else if (Time.Elapsed < _nextRetry)
                {
                    // If the retry delay has not elapsed since the last failure return null
                    return null;
                }
                else if (_findLocatorTask == null)
                {
                    // If a locator lookup is running we await on it otherwise we start a new lookup.
                    _findLocatorTask = FindLocatorAsync();
                }
                findLocatorTask = _findLocatorTask;
            }

            ILocatorPrx? locator = await findLocatorTask.ConfigureAwait(false);
            lock (_mutex)
            {
                _findLocatorTask = null;
            }
            return locator;
        }
    }

    internal class LookupReply : ILookupReply
    {
        private readonly Locator _locatorServant;

        public void FoundLocator(ILocatorPrx locator, Current current, CancellationToken cancel) =>
            _locatorServant.FoundLocator(locator);

        internal LookupReply(Locator locatorServant) => _locatorServant = locatorServant;
    }
}
