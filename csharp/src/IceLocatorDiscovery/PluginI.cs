//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using ZeroC.Ice;

namespace ZeroC.IceLocatorDiscovery
{
    public sealed class PluginFactory : IPluginFactory
    {
        public IPlugin Create(Communicator communicator, string name, string[] args) =>
            new Plugin(name, communicator);

        public static void Register(bool loadOnInitialize) =>
           Communicator.RegisterPluginFactory("IceLocatorDiscovery", new PluginFactory(), loadOnInitialize);
    }

    internal class VoidLocator : ILocator
    {
        public ValueTask<IObjectPrx?> FindAdapterByIdAsync(string id, Current current) => default;
        public ValueTask<IObjectPrx?> FindObjectByIdAsync(Identity id, Current current) => default;
        public ILocatorRegistryPrx? GetRegistry(Current current) => null;
    }

    internal class Locator : IObject
    {
        private TaskCompletionSource<ILocatorPrx>? _completionSource;
        private Task<ILocatorPrx>? _findLocatorTask;
        private string _instanceName;
        private ILocatorPrx? _locator;
        private readonly ILookupPrx _lookup;
        private readonly Dictionary<ILookupPrx, ILookupReplyPrx> _lookups =
            new Dictionary<ILookupPrx, ILookupReplyPrx>();
        private readonly object _mutex = new object();
        private long _nextRetry;
        private readonly int _retryCount;
        private readonly int _retryDelay;
        private readonly int _timeout;
        private readonly int _traceLevel;
        private readonly ILocatorPrx _voidLocator;
        private bool _warned;

        internal Locator(
            string name,
            ILookupPrx lookup,
            Communicator communicator,
            string instanceName,
            ILocatorPrx voidLocator,
            ILookupReplyPrx lookupReply)
        {
            _lookup = lookup;
            _timeout = communicator.GetPropertyAsInt($"{name}.Timeout") ?? 300;
            if (_timeout < 0)
            {
                _timeout = 300;
            }
            _retryCount = Math.Max(communicator.GetPropertyAsInt($"{name}.RetryCount") ?? 3, 1);
            _retryDelay = Math.Max(communicator.GetPropertyAsInt($"{name}.RetryDelay") ?? 2000, 0);
            _traceLevel = communicator.GetPropertyAsInt($"{name}.Trace.Lookup") ?? 0;
            _instanceName = instanceName;
            _warned = false;
            _locator = lookup.Communicator.DefaultLocator;
            _voidLocator = voidLocator;

            // Create one lookup proxy per endpoint from the given proxy. We want to send a multicast
            // datagram on each endpoint.
            var single = new Endpoint[1];
            foreach (UdpEndpoint endpoint in lookup.Endpoints.Cast<UdpEndpoint>())
            {
                single[0] = endpoint;
                ILookupPrx key = lookup.Clone(endpoints: single);
                if (endpoint.McastInterface.Length > 0)
                {
                    IPEndpoint? q = lookupReply.Endpoints.Cast<IPEndpoint>().FirstOrDefault(
                        e => e is IPEndpoint ipEndpoint && ipEndpoint.Host.Equals(endpoint.McastInterface));

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
            Current current)
        {
            ILocatorPrx? locator = null;
            Exception? exception = null;
            while (true)
            {
                // Get the locator to send the request to (this will return the void locator if no locator is found)
                ILocatorPrx newLocator = await GetLocatorAsync().ConfigureAwait(false);
                if (locator != newLocator)
                {
                    var outgoingRequest = new OutgoingRequestFrame(
                        newLocator, current.Operation, current.IsIdempotent, current.Context, incomingRequest.Payload);
                    try
                    {
                        IncomingResponseFrame incomingResponse =
                            await newLocator.InvokeAsync(outgoingRequest).ConfigureAwait(false);
                        return new OutgoingResponseFrame(current.Encoding, incomingResponse.Payload);
                    }
                    catch (DispatchException)
                    {
                        throw;
                    }
                    catch (NoEndpointException)
                    {
                        throw new ObjectNotExistException(current);
                    }
                    catch (ObjectAdapterDeactivatedException)
                    {
                        throw new ObjectNotExistException(current);
                    }
                    catch (CommunicatorDestroyedException)
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

        internal void FoundLocator(ILocatorPrx? locator)
        {
            lock (_mutex)
            {
                if (locator == null)
                {
                    if (_traceLevel > 2)
                    {
                        _lookup.Communicator.Logger.Trace("Lookup", "ignoring locator reply: (null locator)");
                    }
                    return;
                }

                if (_instanceName.Length > 0 && !locator.Identity.Category.Equals(_instanceName))
                {
                    if (_traceLevel > 2)
                    {
                        _lookup.Communicator.Logger.Trace("Lookup",
                            @$"ignoring locator reply: instance name doesn't match\nexpected = {_instanceName
                            } received = {locator.Identity.Category}");
                    }
                    return;
                }

                // If we already have a locator assigned, ensure the given locator has the same identity, otherwise
                // ignore it.
                if (_locator != null && !locator.Identity.Category.Equals(_locator.Identity.Category))
                {
                    if (!_warned)
                    {
                        _warned = true; // Only warn once

                        var s = new StringBuilder();
                        s.Append("received Ice locator with different instance name:\n")
                         .Append("using = `").Append(_locator.Identity.Category).Append("'\n")
                         .Append("received = `").Append(locator.Identity.Category).Append("'\n")
                         .Append("This is typically the case if multiple Ice locators with different ")
                         .Append("instance names are deployed and the property `IceLocatorDiscovery.InstanceName' ")
                         .Append("is not set.");
                        locator.Communicator.Logger.Warning(s.ToString());
                    }
                    return;
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
                else if (Time.CurrentMonotonicTimeMillis() < _nextRetry)
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
                var s = new StringBuilder("looking up locator:\nlookup = ");
                s.Append(_lookup);
                if (_instanceName.Length > 0)
                {
                    s.Append("\ninstance name = ").Append(_instanceName);
                }
                _lookup.Communicator.Logger.Trace("Lookup", s.ToString());
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
                                    var s = new StringBuilder("locator lookup failed:\nlookup = ");
                                    s.Append(_lookup);
                                    if (_instanceName.Length > 0)
                                    {
                                        s.Append("\ninstance name = ").Append(_instanceName);
                                    }
                                    s.Append("\nwith lookup proxy `{_lookup}':\n");
                                    s.Append(ex);
                                    _lookup.Communicator.Logger.Trace("Lookup", s.ToString());
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
                        var s = new StringBuilder("locator lookup timed out:\nlookup = ");
                        s.Append(_lookup);
                        if (_instanceName.Length > 0)
                        {
                            s.Append("\ninstance name = ").Append(_instanceName);
                        }
                        _lookup.Communicator.Logger.Trace("Lookup", s.ToString());
                    }

                    _nextRetry = Time.CurrentMonotonicTimeMillis() + _retryDelay;
                    return _voidLocator;
                }
            }
        }
    }

    internal class LookupReply : ILookupReply
    {
        private readonly Locator _locator;
        public void FoundLocator(ILocatorPrx? locator, Current current) => _locator.FoundLocator(locator);
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

        public void Destroy()
        {
            _replyAdapter?.Destroy();
            _locatorAdapter?.Destroy();

            if (IObjectPrx.Equals(_communicator.DefaultLocator, _locatorPrx))
            {
                // Restore original default locator proxy, if the user didn't change it in the meantime
                _communicator.DefaultLocator = _defaultLocator;
            }
        }

        public void Initialize()
        {
            bool ipv4 = _communicator.GetPropertyAsBool("Ice.IPv4") ?? true;
            bool preferIPv6 = _communicator.GetPropertyAsBool("Ice.PreferIPv6Address") ?? false;

            string address;
            if (ipv4 && !preferIPv6)
            {
                address = _communicator.GetProperty($"{_name}.Address") ?? "239.255.0.1";
            }
            else
            {
                address = _communicator.GetProperty($"{_name}.Address") ?? "ff15::1";
            }
            int port = _communicator.GetPropertyAsInt($"{_name}.Port") ?? 4061;
            string intf = _communicator.GetProperty($"{_name}.Interface") ?? "";

            string lookupEndpoints = _communicator.GetProperty($"{_name}.Lookup") ?? "";
            if (lookupEndpoints.Length == 0)
            {
                int ipVersion = ipv4 && !preferIPv6 ? Network.EnableIPv4 : Network.EnableIPv6;
                List<string> interfaces = Network.GetInterfacesForMulticast(intf, ipVersion);
                lookupEndpoints = string.Join(":", interfaces.Select(
                    intf => $"udp -h \"{address}\" -p {port} --interface \"{intf}\""));
            }

            if (_communicator.GetProperty($"{_name}.Reply.Endpoints") == null)
            {
                _communicator.SetProperty($"{_name}.Reply.Endpoints",
                    intf.Length == 0 ? "udp -h *" : $"udp -h \"{intf}\"");
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
            // No collocation optimization or router for the multicast proxy!
            lookupPrx = lookupPrx.Clone(clearRouter: false, collocationOptimized: false);

            ILocatorPrx voidLocator = _locatorAdapter.AddWithUUID(new VoidLocator(), ILocatorPrx.Factory);

            var lookupReplyId = new Identity(Guid.NewGuid().ToString(), "");
            ILookupReplyPrx? locatorReplyPrx = _replyAdapter.CreateProxy(lookupReplyId, ILookupReplyPrx.Factory).Clone(
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
