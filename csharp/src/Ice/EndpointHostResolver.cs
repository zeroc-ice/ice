//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Collections.Generic;
using System.Diagnostics;
using System.Net;
using System.Threading;
using IceInternal;

namespace Ice
{
    public sealed partial class Communicator
    {
        private sealed class HelperThread
        {
            public string Name => _thread.Name;

            private Communicator _communicator;
            private Thread _thread;

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
                    _communicator.Logger.error($"exception in endpoint host resolver thread {_thread.Name}:\n{ex}");
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
            internal IEndpointConnectors callback;
            internal IPEndpoint endpoint;
            internal string host;
            internal Instrumentation.IObserver? observer;
            internal int port;
            internal EndpointSelectionType selType;

            internal ResolveEntry(string host, int port, EndpointSelectionType selType, IPEndpoint endpoint, IEndpointConnectors callback)
            {
                this.host = host;
                this.port = port;
                this.selType = selType;
                this.endpoint = endpoint;
                this.callback = callback;
            }
        }

        private bool _endpointHostResolverDestroyed;
        private LinkedList<ResolveEntry> _enpointHostResolverQueue = new LinkedList<ResolveEntry>();
        private Instrumentation.IThreadObserver? _endpointHostResolverObserver;
        private HelperThread _endpointHostResolverThread;

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
                    List<EndPoint> addrs = Network.getAddresses(host, port, ProtocolSupport, selType, PreferIPv6, false);
                    if (addrs.Count > 0)
                    {
                        callback.connectors(endpoint.connectors(addrs, null));
                        return;
                    }
                }
                catch (LocalException ex)
                {
                    callback.exception(ex);
                    return;
                }
            }

            lock (_endpointHostResolverThread)
            {
                Debug.Assert(!_endpointHostResolverDestroyed);

                ResolveEntry entry = new ResolveEntry(host, port, selType, endpoint, callback);

                Instrumentation.ICommunicatorObserver? obsv = Observer;
                if (obsv != null)
                {
                    entry.observer = obsv.getEndpointLookupObserver(endpoint);
                    if (entry.observer != null)
                    {
                        entry.observer.attach();
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

                    r = _enpointHostResolverQueue.First.Value;
                    _enpointHostResolverQueue.RemoveFirst();
                    threadObserver = _endpointHostResolverObserver;
                }

                if (threadObserver != null)
                {
                    threadObserver.stateChanged(Instrumentation.ThreadState.ThreadStateIdle,
                                                Instrumentation.ThreadState.ThreadStateInUseForOther);
                }

                try
                {

                    INetworkProxy? networkProxy = NetworkProxy;
                    int protocol = ProtocolSupport;
                    if (networkProxy != null)
                    {
                        networkProxy = networkProxy.resolveHost(protocol);
                        if (networkProxy != null)
                        {
                            protocol = networkProxy.getProtocolSupport();
                        }
                    }

                    List<EndPoint> addrs = Network.getAddresses(r.host, r.port, protocol, r.selType, PreferIPv6, true);
                    if (r.observer != null)
                    {
                        r.observer.detach();
                        r.observer = null;
                    }

                    r.callback.connectors(r.endpoint.connectors(addrs, networkProxy));
                }
                catch (LocalException ex)
                {
                    if (r.observer != null)
                    {
                        r.observer.failed(ex.ice_id());
                        r.observer.detach();
                    }
                    r.callback.exception(ex);
                }
                finally
                {
                    if (threadObserver != null)
                    {
                        threadObserver.stateChanged(Instrumentation.ThreadState.ThreadStateInUseForOther,
                                                    Instrumentation.ThreadState.ThreadStateIdle);
                    }
                }
            }

            foreach (ResolveEntry entry in _enpointHostResolverQueue)
            {
                CommunicatorDestroyedException ex = new CommunicatorDestroyedException();
                if (entry.observer != null)
                {
                    entry.observer.failed(ex.ice_id());
                    entry.observer.detach();
                }
                entry.callback.exception(ex);
            }
            _enpointHostResolverQueue.Clear();

            if (_endpointHostResolverObserver != null)
            {
                _endpointHostResolverObserver.detach();
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
                    _endpointHostResolverObserver = obsv.getThreadObserver("Communicator",
                                                       _endpointHostResolverThread.Name,
                                                       Instrumentation.ThreadState.ThreadStateIdle,
                                                       _endpointHostResolverObserver);
                    if (_endpointHostResolverObserver != null)
                    {
                        _endpointHostResolverObserver.attach();
                    }
                }
            }
        }
    }
}
