// **********************************************************************
//
// Copyright (c) 2003-2012 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#if !SILVERLIGHT
namespace IceInternal
{
    using System;
    using System.Collections.Generic;
    using System.Diagnostics;
    using System.Net;
    using System.Threading;

    public class EndpointHostResolver
    {
        internal EndpointHostResolver(Instance instance)
        {
            _instance = instance;
            _thread = new HelperThread(this);
            updateObserver();
            if(instance.initializationData().properties.getProperty("Ice.ThreadPriority").Length > 0)
            {
                ThreadPriority priority = IceInternal.Util.stringToThreadPriority(
                                           instance.initializationData().properties.getProperty("Ice.ThreadPriority"));
                _thread.Start(priority);
            }
            else
            {
                _thread.Start(ThreadPriority.Normal);
            }
        }

        
        public List<Connector> resolve(string host, int port, EndpointI endpoint)
        {
            //
            // Try to get the addresses without DNS lookup. If this doesn't
            // work, we retry with DNS lookup (and observer).
            //
            List<EndPoint> addrs = Network.getAddresses(host, port, _instance.protocolSupport(), false);
            if(addrs.Count > 0)
            {
                return endpoint.connectors(addrs);
            }

            Ice.Instrumentation.CommunicatorObserver obsv = _instance.initializationData().observer;
            Ice.Instrumentation.Observer observer = null;
            if(obsv != null)
            {
                observer = obsv.getEndpointLookupObserver(endpoint);
                if(observer != null)
                {
                    observer.attach();
                }
            }
    
            List<Connector> connectors = null;
            try 
            {
                connectors = endpoint.connectors(Network.getAddresses(host, port, _instance.protocolSupport(), true));
            }
            catch(Ice.LocalException ex)
            {
                if(observer != null)
                {
                    observer.failed(ex.ice_name());
                }
                throw ex;
            }
            finally
            {
                if(observer != null)
                {
                    observer.detach();
                }
            }
            return connectors;
        }

        public void resolve(string host, int port, EndpointI endpoint, EndpointI_connectors callback)
        {
            //
            // Try to get the addresses without DNS lookup. If this doesn't work, we queue a resolve
            // entry and the thread will take care of getting the endpoint addresses.
            //
            try
            {
                List<EndPoint> addrs = Network.getAddresses(host, port, _instance.protocolSupport(), false);
                if(addrs.Count > 0)
                {
                    callback.connectors(endpoint.connectors(addrs));
                    return;
                }
            }
            catch(Ice.LocalException ex)
            {
                callback.exception(ex);
                return;
            }

            _m.Lock();
            try
            {
                Debug.Assert(!_destroyed);

                ResolveEntry entry = new ResolveEntry();
                entry.host = host;
                entry.port = port;
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
                _m.Notify();
            }
            finally
            {
                _m.Unlock();
            }
        }

        public void destroy()
        {
            _m.Lock();
            try
            {
                Debug.Assert(!_destroyed);
                _destroyed = true;
                _m.Notify();
            }
            finally
            {
                _m.Unlock();
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

                _m.Lock();
                try
                {
                    while(!_destroyed && _queue.Count == 0)
                    {
                        _m.Wait();
                    }

                    if(_destroyed)
                    {
                        break;
                    }

                    r = _queue.First.Value;
                    _queue.RemoveFirst();
                    threadObserver = _observer;
                }
                finally
                {
                    _m.Unlock();
                }

                int protocol = _instance.protocolSupport();
                try
                {
                    if(threadObserver != null)
                    {
                        threadObserver.stateChanged(Ice.Instrumentation.ThreadState.ThreadStateIdle, 
                                                    Ice.Instrumentation.ThreadState.ThreadStateInUseForOther);
                    }

                    r.callback.connectors(r.endpoint.connectors(Network.getAddresses(r.host, r.port, protocol, true)));

                    if(threadObserver != null)
                    {
                        threadObserver.stateChanged(Ice.Instrumentation.ThreadState.ThreadStateInUseForOther, 
                                                    Ice.Instrumentation.ThreadState.ThreadStateIdle);
                    }
                }
                catch(Ice.LocalException ex)
                {
                    if(r.observer != null)
                    {
                        r.observer.failed(ex.ice_name());
                    }
                    r.callback.exception(ex);
                }
                finally
                {
                    if(r.observer != null)
                    {
                        r.observer.detach();
                    }
                }
            }

            foreach(ResolveEntry entry in _queue)
            {
                Ice.CommunicatorDestroyedException ex = new Ice.CommunicatorDestroyedException();
                entry.callback.exception(ex);
                if(entry.observer != null)
                {
                    entry.observer.failed(ex.ice_name());
                    entry.observer.detach();
                }
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
            _m.Lock();
            try
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
            finally
            {
                _m.Unlock();
            }
        }

        private class ResolveEntry
        {
            internal string host;
            internal int port;
            internal EndpointI endpoint;
            internal EndpointI_connectors callback;
            internal Ice.Instrumentation.Observer observer;
        }

        private Instance _instance;
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

        private readonly IceUtilInternal.Monitor _m = new IceUtilInternal.Monitor();
    }
}
#endif
