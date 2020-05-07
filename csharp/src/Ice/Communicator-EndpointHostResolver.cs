//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using IceInternal;
using System.Collections.Generic;
using System.Diagnostics;
using System.Net;
using System.Threading;

namespace Ice
{
    public sealed partial class Communicator
    {
        private sealed class HelperThread
        {
            public string Name => _thread.Name!;

            private readonly Communicator _communicator;
            private readonly Thread _thread;

            internal HelperThread(Communicator communicator)
            {
                _communicator = communicator;
                string name = communicator.GetProperty("Ice.ProgramName") ?? "";
                if (name.Length > 0)
                {
                    name += "-";
                }
                name += "Ice.HostResolver";

                _thread = new Thread(new ThreadStart(Run));
                _thread.IsBackground = true;
                _thread.Name = name;
            }

            public void Join() => _thread.Join();

            public void Run()
            {
                try
                {
                    _communicator.RunEndpointHostResolver();
                }
                catch (System.Exception ex)
                {
                    _communicator.Logger.Error($"exception in endpoint host resolver thread {_thread.Name}:\n{ex}");
                }
            }

            public void Start(ThreadPriority priority)
            {
                _thread.Priority = priority;
                _thread.Start();
            }
        }

        private class ResolveEntry
        {
            internal IEndpointConnectors Callback;
            internal IPEndpoint Endpoint;
            internal string Host;
            internal Instrumentation.IObserver? Observer;
            internal int Port;
            internal EndpointSelectionType SelType;

            internal ResolveEntry(string host, int port, EndpointSelectionType selType, IPEndpoint endpoint, IEndpointConnectors callback)
            {
                Host = host;
                Port = port;
                SelType = selType;
                Endpoint = endpoint;
                Callback = callback;
            }
        }

        private bool _endpointHostResolverDestroyed;
        private readonly LinkedList<ResolveEntry> _enpointHostResolverQueue = new LinkedList<ResolveEntry>();
        private Instrumentation.IThreadObserver? _endpointHostResolverObserver;
        private readonly HelperThread _endpointHostResolverThread;

        public void Resolve(string host, int port, EndpointSelectionType selType, IPEndpoint endpoint,
            IEndpointConnectors callback)
        {
            //
            // Try to get the addresses without DNS lookup. If this doesn't work, we queue a resolve
            // entry and the thread will take care of getting the endpoint addresses.
            //
            INetworkProxy? networkProxy = NetworkProxy;
            if (networkProxy == null)
            {
                try
                {
                    List<IPEndPoint> addrs = Network.GetAddresses(host, port, IPVersion, selType, PreferIPv6, false);
                    if (addrs.Count > 0)
                    {
                        callback.Connectors(endpoint.Connectors(addrs, null));
                        return;
                    }
                }
                catch (System.Exception ex)
                {
                    callback.Exception(ex);
                    return;
                }
            }

            lock (_endpointHostResolverThread)
            {
                Debug.Assert(!_endpointHostResolverDestroyed);

                var entry = new ResolveEntry(host, port, selType, endpoint, callback);

                Instrumentation.ICommunicatorObserver? obsv = Observer;
                if (obsv != null)
                {
                    entry.Observer = obsv.GetEndpointLookupObserver(endpoint);
                    if (entry.Observer != null)
                    {
                        entry.Observer.Attach();
                    }
                }

                _enpointHostResolverQueue.AddLast(entry);
                Monitor.Pulse(_endpointHostResolverThread);
            }
        }

        public void RunEndpointHostResolver()
        {
            while (true)
            {
                ResolveEntry r;
                Instrumentation.IThreadObserver? threadObserver;

                lock (_endpointHostResolverThread)
                {
                    while (!_endpointHostResolverDestroyed && _enpointHostResolverQueue.Count == 0)
                    {
                        Monitor.Wait(_endpointHostResolverThread);
                    }

                    if (_endpointHostResolverDestroyed)
                    {
                        break;
                    }

                    r = _enpointHostResolverQueue.First!.Value;
                    _enpointHostResolverQueue.RemoveFirst();
                    threadObserver = _endpointHostResolverObserver;
                }

                if (threadObserver != null)
                {
                    threadObserver.StateChanged(Instrumentation.ThreadState.ThreadStateIdle,
                                                Instrumentation.ThreadState.ThreadStateInUseForOther);
                }

                try
                {
                    INetworkProxy? networkProxy = NetworkProxy;
                    int ipVersion = IPVersion;
                    if (networkProxy != null)
                    {
                        networkProxy = networkProxy.ResolveHost(ipVersion);
                        if (networkProxy != null)
                        {
                            ipVersion = networkProxy.GetIPVersion();
                        }
                    }

                    List<IPEndPoint> addrs = Network.GetAddresses(r.Host, r.Port, ipVersion, r.SelType,
                        PreferIPv6, true);
                    if (r.Observer != null)
                    {
                        r.Observer.Detach();
                        r.Observer = null;
                    }

                    r.Callback.Connectors(r.Endpoint.Connectors(addrs, networkProxy));
                }
                catch (System.Exception ex)
                {
                    if (r.Observer != null)
                    {
                        r.Observer.Failed(ex.GetType().FullName ?? "System.Exception");
                        r.Observer.Detach();
                    }
                    r.Callback.Exception(ex);
                }
                finally
                {
                    if (threadObserver != null)
                    {
                        threadObserver.StateChanged(Instrumentation.ThreadState.ThreadStateInUseForOther,
                                                    Instrumentation.ThreadState.ThreadStateIdle);
                    }
                }
            }

            foreach (ResolveEntry entry in _enpointHostResolverQueue)
            {
                var ex = new CommunicatorDestroyedException();
                if (entry.Observer != null)
                {
                    entry.Observer.Failed(ex.GetType().FullName ?? "System.Exception");
                    entry.Observer.Detach();
                }
                entry.Callback.Exception(ex);
            }
            _enpointHostResolverQueue.Clear();

            if (_endpointHostResolverObserver != null)
            {
                _endpointHostResolverObserver.Detach();
            }
        }

        public void
        UpdateEndpointHostResolverObserver()
        {
            lock (_endpointHostResolverThread)
            {
                Instrumentation.ICommunicatorObserver? obsv = Observer;
                if (obsv != null)
                {
                    _endpointHostResolverObserver = obsv.GetThreadObserver("Communicator",
                                                       _endpointHostResolverThread.Name,
                                                       Instrumentation.ThreadState.ThreadStateIdle,
                                                       _endpointHostResolverObserver);
                    if (_endpointHostResolverObserver != null)
                    {
                        _endpointHostResolverObserver.Attach();
                    }
                }
            }
        }
    }
}
