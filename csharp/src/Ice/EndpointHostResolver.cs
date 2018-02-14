// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

namespace IceInternal
{
    using System.Collections.Generic;
    using System.Diagnostics;
    using System.Net;
    using System.Threading;

    public class EndpointHostResolver
    {
        internal EndpointHostResolver(Instance instance)
        {
            _instance = instance;
            _protocol = instance.protocolSupport();
            _preferIPv6 = instance.preferIPv6();
            _thread = new HelperThread(this);
            updateObserver();
            _thread.Start(Util.stringToThreadPriority(
                    instance.initializationData().properties.getProperty("Ice.ThreadPriority")));
        }

        public void resolve(string host, int port, Ice.EndpointSelectionType selType, IPEndpointI endpoint,
                            EndpointI_connectors callback)
        {
            //
            // Try to get the addresses without DNS lookup. If this doesn't work, we queue a resolve
            // entry and the thread will take care of getting the endpoint addresses.
            //
            NetworkProxy networkProxy = _instance.networkProxy();
            if(networkProxy == null)
            {
                try
                {
                    List<EndPoint> addrs = Network.getAddresses(host, port, _protocol, selType, _preferIPv6, false);
                    if(addrs.Count > 0)
                    {
                        callback.connectors(endpoint.connectors(addrs, null));
                        return;
                    }
                }
                catch(Ice.LocalException ex)
                {
                    callback.exception(ex);
                    return;
                }
            }

            lock(this)
            {
                Debug.Assert(!_destroyed);

                ResolveEntry entry = new ResolveEntry();
                entry.host = host;
                entry.port = port;
                entry.selType = selType;
                entry.endpoint = endpoint;
                entry.callback = callback;

                Ice.Instrumentation.CommunicatorObserver obsv = _instance.initializationData().observer;
                if(obsv != null)
                {
                    entry.observer = obsv.getEndpointLookupObserver(endpoint);
                    if(entry.observer != null)
                    {
                        entry.observer.attach();
                    }
                }

                _queue.AddLast(entry);
                Monitor.Pulse(this);
            }
        }

        public void destroy()
        {
            lock(this)
            {
                Debug.Assert(!_destroyed);
                _destroyed = true;
                Monitor.Pulse(this);
            }
        }

        public void joinWithThread()
        {
            if(_thread != null)
            {
                _thread.Join();
            }
        }

        public void run()
        {
            while(true)
            {
                ResolveEntry r;
                Ice.Instrumentation.ThreadObserver threadObserver;

                lock(this)
                {
                    while(!_destroyed && _queue.Count == 0)
                    {
                        Monitor.Wait(this);
                    }

                    if(_destroyed)
                    {
                        break;
                    }

                    r = _queue.First.Value;
                    _queue.RemoveFirst();
                    threadObserver = _observer;
                }

                if(threadObserver != null)
                {
                    threadObserver.stateChanged(Ice.Instrumentation.ThreadState.ThreadStateIdle,
                                                Ice.Instrumentation.ThreadState.ThreadStateInUseForOther);
                }

                try
                {

                    NetworkProxy networkProxy = _instance.networkProxy();
                    int protocol = _protocol;
                    if(networkProxy != null)
                    {
                        networkProxy = networkProxy.resolveHost(protocol);
                        if(networkProxy != null)
                        {
                            protocol = networkProxy.getProtocolSupport();
                        }
                    }

                    List<EndPoint> addrs = Network.getAddresses(r.host, r.port, protocol, r.selType, _preferIPv6, true);
                    if(r.observer != null)
                    {
                        r.observer.detach();
                        r.observer = null;
                    }

                    r.callback.connectors(r.endpoint.connectors(addrs, networkProxy));
                }
                catch(Ice.LocalException ex)
                {
                    if(r.observer != null)
                    {
                        r.observer.failed(ex.ice_id());
                        r.observer.detach();
                    }
                    r.callback.exception(ex);
                }
                finally
                {
                    if(threadObserver != null)
                    {
                        threadObserver.stateChanged(Ice.Instrumentation.ThreadState.ThreadStateInUseForOther,
                                                    Ice.Instrumentation.ThreadState.ThreadStateIdle);
                    }
                }
            }

            foreach(ResolveEntry entry in _queue)
            {
                Ice.CommunicatorDestroyedException ex = new Ice.CommunicatorDestroyedException();
                if(entry.observer != null)
                {
                    entry.observer.failed(ex.ice_id());
                    entry.observer.detach();
                }
                entry.callback.exception(ex);
            }
            _queue.Clear();

            if(_observer != null)
            {
                _observer.detach();
            }
        }

        public void
        updateObserver()
        {
            lock(this)
            {
                Ice.Instrumentation.CommunicatorObserver obsv = _instance.initializationData().observer;
                if(obsv != null)
                {
                    _observer = obsv.getThreadObserver("Communicator",
                                                       _thread.getName(),
                                                       Ice.Instrumentation.ThreadState.ThreadStateIdle,
                                                       _observer);
                    if(_observer != null)
                    {
                        _observer.attach();
                    }
                }
            }
        }

        private class ResolveEntry
        {
            internal string host;
            internal int port;
            internal Ice.EndpointSelectionType selType;
            internal IPEndpointI endpoint;
            internal EndpointI_connectors callback;
            internal Ice.Instrumentation.Observer observer;
        }

        private readonly Instance _instance;
        private readonly int _protocol;
        private readonly bool _preferIPv6;
        private bool _destroyed;
        private LinkedList<ResolveEntry> _queue = new LinkedList<ResolveEntry>();
        private Ice.Instrumentation.ThreadObserver _observer;

        private sealed class HelperThread
        {
            internal HelperThread(EndpointHostResolver resolver)
            {
                _resolver = resolver;
                _name = _resolver._instance.initializationData().properties.getProperty("Ice.ProgramName");
                if(_name.Length > 0)
                {
                    _name += "-";
                }
                _name += "Ice.HostResolver";
            }

            public void Join()
            {
                _thread.Join();
            }

            public void Start(ThreadPriority priority)
            {
                _thread = new Thread(new ThreadStart(Run));
                _thread.IsBackground = true;
                _thread.Name = _name;
                _thread.Priority = priority;
                _thread.Start();
            }

            public void Run()
            {
                try
                {
                    _resolver.run();
                }
                catch(System.Exception ex)
                {
                    string s = "exception in endpoint host resolver thread " + _name + ":\n" + ex;
                    _resolver._instance.initializationData().logger.error(s);
                }
            }

            public string getName()
            {
                return _name;
            }

            private EndpointHostResolver _resolver;
            private string _name;
            private Thread _thread;
        }

        private HelperThread _thread;
    }
}
