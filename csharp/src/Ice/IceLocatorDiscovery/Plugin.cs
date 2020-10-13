// Copyright (c) ZeroC, Inc. All rights reserved.

using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using ZeroC.Ice;

namespace ZeroC.IceLocatorDiscovery
{
    /// <summary>The IceLocatorDiscovery plug-in's factory.</summary>
    public sealed class PluginFactory : IPluginFactory
    {
        /// <inheritdoc/>
        public IPlugin Create(Communicator communicator, string name, string[] args) => new Plugin(name, communicator);
    }

    internal class VoidLocator : ILocator
    {
        public ValueTask<IObjectPrx?> FindAdapterByIdAsync(string id, Current current, CancellationToken cancel) =>
            default;
        public ValueTask<IObjectPrx?> FindObjectByIdAsync(Identity id, Current current, CancellationToken cancel) =>
            default;
        public ILocatorRegistryPrx? GetRegistry(Current current, CancellationToken cancel) => null;
    }

    internal class Locator : IObject
    {
        private TaskCompletionSource<ILocatorPrx>? _completionSource;
        private Task<ILocatorPrx>? _findLocatorTask;
        private string _instanceName;
        private ILocatorPrx? _locator;
        private readonly ILookupPrx _lookup;
        private readonly Dictionary<ILookupPrx, ILookupReplyPrx> _lookups = new ();
        private readonly object _mutex = new object();
        private TimeSpan _nextRetry;
        private readonly int _retryCount;
        private readonly TimeSpan _retryDelay;
        private readonly TimeSpan _timeout;
        private readonly int _traceLevel;
        private readonly ILocatorPrx _voidLocator;

        internal Locator(
            string name,
            ILookupPrx lookup,
            Communicator communicator,
            string instanceName,
            ILocatorPrx voidLocator,
            ILookupReplyPrx lookupReply)
        {
            _lookup = lookup;
            _timeout = communicator.GetPropertyAsTimeSpan($"{name}.Timeout") ?? TimeSpan.FromMilliseconds(300);
            if (_timeout == System.Threading.Timeout.InfiniteTimeSpan)
            {
                _timeout = TimeSpan.FromMilliseconds(300);
            }
            _retryCount = Math.Max(communicator.GetPropertyAsInt($"{name}.RetryCount") ?? 3, 1);
            _retryDelay = communicator.GetPropertyAsTimeSpan($"{name}.RetryDelay") ?? TimeSpan.FromMilliseconds(2000);
            _traceLevel = communicator.GetPropertyAsInt($"{name}.Trace.Lookup") ?? 0;
            _instanceName = instanceName;
            _locator = lookup.Communicator.DefaultLocator;
            _voidLocator = voidLocator;

            // Create one lookup proxy per endpoint from the given proxy. We want to send a multicast
            // datagram on each endpoint.
            var single = new Endpoint[1];
            foreach (Endpoint endpoint in lookup.Endpoints)
            {
                // lookup's invocation mode is Datagram
                Debug.Assert(endpoint.Transport == Transport.UDP);

                single[0] = endpoint;
                ILookupPrx key = lookup.Clone(endpoints: single);
                if (endpoint["interface"] is string mcastInterface && mcastInterface.Length > 0)
                {
                    Endpoint? q = lookupReply.Endpoints.FirstOrDefault(e => e.Host == mcastInterface);

                    if (q != null)
                    {
                        single[0] = q;
                        _lookups[key] = lookupReply.Clone(endpoints: single);
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

        public async ValueTask<OutgoingResponseFrame> DispatchAsync(
            IncomingRequestFrame incomingRequest,
            Current current,
            CancellationToken cancel)
        {
            ILocatorPrx? locator = null;
            Exception? exception = null;
            while (true)
            {
                // Get the locator to send the request to (this will return the void locator if no locator is found)
                ILocatorPrx newLocator = await GetLocatorAsync().ConfigureAwait(false);
                if (!newLocator.Equals(locator))
                {
                    try
                    {
                        return
                            await newLocator.ForwardAsync(false, incomingRequest, cancel: cancel).ConfigureAwait(false);
                    }
                    catch (ObjectNotExistException)
                    {
                        throw;
                    }
                    catch (NoEndpointException)
                    {
                        throw new ObjectNotExistException(current);
                    }
                    catch (ObjectDisposedException)
                    {
                        throw new ObjectNotExistException(current);
                    }
                    catch (Exception ex)
                    {
                        lock (_mutex)
                        {
                            locator = newLocator;
                            // If the current locator is equal to the one we use to send the request,
                            // clear it and retry, this will trigger the lookup of a new locator.
                            if (_locator == newLocator)
                            {
                                _locator = null;
                            }
                        }
                        exception = ex;
                    }
                }
                else
                {
                    // We got the same locator after a previous failure, throw the saved exception now.
                    Debug.Assert(exception != null);
                    throw exception;
                }
            }
        }

        internal void FoundLocator(ILocatorPrx locator)
        {
            lock (_mutex)
            {
                if (_instanceName.Length > 0 && locator.Identity.Category != _instanceName)
                {
                    if (_traceLevel > 2)
                    {
                        _lookup.Communicator.Logger.Trace("Lookup",
                            @$"ignoring locator reply: instance name doesn't match\nexpected = {_instanceName
                            } received = {locator.Identity.Category}");
                    }
                    return;
                }

                // If we already have a locator assigned, ensure the given locator has the same identity and protocol,
                // otherwise ignore it.
                if (_locator != null)
                {
                    if (locator.Identity.Category != _locator.Identity.Category)
                    {
                        var sb = new StringBuilder();
                        sb.Append("received Ice locator with different instance name:\n")
                          .Append("using = `").Append(_locator.Identity.Category).Append("'\n")
                          .Append("received = `").Append(locator.Identity.Category).Append("'\n")
                          .Append("This is typically the case if multiple Ice locators with different ")
                          .Append("instance names are deployed and the property `IceLocatorDiscovery.InstanceName' ")
                          .Append("is not set.");
                        locator.Communicator.Logger.Warning(sb.ToString());
                        return;
                    }

                    if (locator.Protocol != _locator.Protocol)
                    {
                        var sb = new StringBuilder();
                        sb.Append("ignoring Ice locator with different protocol:\n")
                          .Append("using = `").Append(_locator.Protocol).Append("'\n")
                          .Append("received = `").Append(locator.Protocol).Append("'\n");
                        locator.Communicator.Logger.Warning(sb.ToString());
                        return;
                    }
                }

                if (_traceLevel > 0)
                {
                    var s = new StringBuilder("locator lookup succeeded:\nlocator = ");
                    s.Append(locator);
                    if (_instanceName.Length > 0)
                    {
                        s.Append("\ninstance name = ").Append(_instanceName);
                    }
                    _lookup.Communicator.Logger.Trace("Lookup", s.ToString());
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

        private async Task<ILocatorPrx> GetLocatorAsync()
        {
            Task<ILocatorPrx> findLocatorTask;
            lock (_mutex)
            {
                // If we already have a locator we use it.
                if (_locator != null)
                {
                    return _locator;
                }
                // If the retry delay has not elapsed since the last failure return the void locator that always
                // replies with a null proxy.
                else if (Time.Elapsed < _nextRetry)
                {
                    return _voidLocator;
                }
                // If a locator lookup is running we await on it otherwise we start a new lookup.
                else if (_findLocatorTask == null)
                {
                    _findLocatorTask = FindLocatorAsync();
                }
                findLocatorTask = _findLocatorTask;
            }
            ILocatorPrx locator = await findLocatorTask.ConfigureAwait(false);
            lock (_mutex)
            {
                _findLocatorTask = null;
            }
            return locator;
        }

        private async Task<ILocatorPrx> FindLocatorAsync()
        {
            lock (_mutex)
            {
                Debug.Assert(_locator == null);
                Debug.Assert(_findLocatorTask == null);
                _completionSource = new TaskCompletionSource<ILocatorPrx>();
            }

            if (_traceLevel > 1)
            {
                var sb = new StringBuilder("looking up locator:\nlookup = ");
                sb.Append(_lookup);
                if (_instanceName.Length > 0)
                {
                    sb.Append("\ninstance name = ").Append(_instanceName);
                }
                _lookup.Communicator.Logger.Trace("Lookup", sb.ToString());
            }

            int failureCount = 0;
            for (int i = 0; i < _retryCount; ++i)
            {
                foreach ((ILookupPrx lookup, ILookupReplyPrx lookupReply) in _lookups)
                {
                    try
                    {
                        await lookup.FindLocatorAsync(_instanceName, lookupReply).ConfigureAwait(false);
                    }
                    catch (Exception ex)
                    {
                        lock (_mutex)
                        {
                            if (++failureCount == _lookups.Count)
                            {
                                // All the lookup calls failed propagate the error to the requests.
                                if (_traceLevel > 0)
                                {
                                    var sb = new StringBuilder("locator lookup failed:\nlookup = ");
                                    sb.Append(_lookup);
                                    if (_instanceName.Length > 0)
                                    {
                                        sb.Append("\ninstance name = ").Append(_instanceName);
                                    }
                                    sb.Append("\nwith lookup proxy `{_lookup}':\n");
                                    sb.Append(ex);
                                    _lookup.Communicator.Logger.Trace("Lookup", sb.ToString());
                                }
                                return _voidLocator;
                            }
                        }
                    }
                }

                Task t = await Task.WhenAny(_completionSource.Task, Task.Delay(_timeout)).ConfigureAwait(false);
                if (t == _completionSource.Task)
                {
                    return await _completionSource.Task.ConfigureAwait(false);
                }
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
                    if (_traceLevel > 0)
                    {
                        var sb = new StringBuilder("locator lookup timed out:\nlookup = ");
                        sb.Append(_lookup);
                        if (_instanceName.Length > 0)
                        {
                            sb.Append("\ninstance name = ").Append(_instanceName);
                        }
                        _lookup.Communicator.Logger.Trace("Lookup", sb.ToString());
                    }

                    _nextRetry = Time.Elapsed + _retryDelay;
                    return _voidLocator;
                }
            }
        }
    }

    internal class LookupReply : ILookupReply
    {
        private readonly Locator _locator;
        public void FoundLocator(ILocatorPrx locator, Current current, CancellationToken cancel) =>
            _locator.FoundLocator(locator);
        internal LookupReply(Locator locator) => _locator = locator;
    }

    internal class Plugin : IPlugin
    {
        private readonly Communicator _communicator;
        private ILocatorPrx? _defaultLocator;
        private Locator? _locator;
        private ObjectAdapter? _locatorAdapter;
        private ILocatorPrx? _locatorPrx;
        private readonly string _name;
        private ObjectAdapter? _replyAdapter;

        public async ValueTask DisposeAsync()
        {
            if (_replyAdapter != null)
            {
                await _replyAdapter.DisposeAsync().ConfigureAwait(false);
            }

            if (_locatorAdapter != null)
            {
                await _locatorAdapter.DisposeAsync().ConfigureAwait(false);
            }

            if (IObjectPrx.Equals(_communicator.DefaultLocator, _locatorPrx))
            {
                // Restore original default locator proxy, if the user didn't change it in the meantime
                _communicator.DefaultLocator = _defaultLocator;
            }
        }

        public void Initialize(PluginInitializationContext context)
        {
            const string defaultIPv4Endpoint = "udp -h 239.255.0.1 -p 4061";
            const string defaultIPv6Endpoint = "udp -h \"ff15::1\" -p 4061";

            string lookupEndpoints;
            if (_communicator.GetProperty($"{_name}.Lookup") is string prop)
            {
                lookupEndpoints = prop;
            }
            else
            {

                List<string> endpoints = new ();
                List<string> ipv4Interfaces = Network.GetInterfacesForMulticast("0.0.0.0", Network.EnableIPv4);
                List<string> ipv6Interfaces = Network.GetInterfacesForMulticast("::0", Network.EnableIPv6);

                endpoints.AddRange(ipv4Interfaces.Select(i => $"{defaultIPv4Endpoint} --interface \"{i}\""));
                endpoints.AddRange(ipv6Interfaces.Select(i => $"{defaultIPv6Endpoint} --interface \"{i}\""));

                lookupEndpoints = string.Join(":", endpoints);
            }

            if (_communicator.GetProperty($"{_name}.Reply.Endpoints") == null)
            {
                _communicator.SetProperty($"{_name}.Reply.Endpoints", "udp -h \"::0\" -p 0");
            }

            if (_communicator.GetProperty($"{_name}.Locator.Endpoints") == null)
            {
                _communicator.SetProperty($"{_name}.Locator.AdapterId", Guid.NewGuid().ToString());
            }

            _replyAdapter = _communicator.CreateObjectAdapter(_name + ".Reply");
            _locatorAdapter = _communicator.CreateObjectAdapter(_name + ".Locator");

            // We don't want those adapters to be registered with the locator so clear their locator.
            _replyAdapter.Locator = null;
            _locatorAdapter.Locator = null;

            var lookupPrx = ILookupPrx.Parse($"IceLocatorDiscovery/Lookup -d:{lookupEndpoints}", _communicator);
            lookupPrx = lookupPrx.Clone(clearRouter: false);

            ILocatorPrx voidLocator = _locatorAdapter.AddWithUUID(new VoidLocator(), ILocatorPrx.Factory);

            var lookupReplyId = new Identity(Guid.NewGuid().ToString(), "");
            ILookupReplyPrx locatorReplyPrx = _replyAdapter.CreateProxy(lookupReplyId, ILookupReplyPrx.Factory).Clone(
                invocationMode: InvocationMode.Datagram);
            _defaultLocator = _communicator.DefaultLocator;

            string instanceName = _communicator.GetProperty($"{_name}.InstanceName") ?? "";
            var locatorId = new Identity("Locator", instanceName.Length > 0 ? instanceName : Guid.NewGuid().ToString());
            _locator = new Locator(_name, lookupPrx, _communicator, instanceName, voidLocator, locatorReplyPrx);
            _locatorPrx = _locatorAdapter.Add(locatorId, _locator, ILocatorPrx.Factory);
            _communicator.DefaultLocator = _locatorPrx;

            _replyAdapter.Add(lookupReplyId, new LookupReply(_locator));

            _replyAdapter.Activate();
            _locatorAdapter.Activate();
        }

        internal Plugin(string name, Communicator communicator)
        {
            _name = name;
            _communicator = communicator;
        }
    }
}
