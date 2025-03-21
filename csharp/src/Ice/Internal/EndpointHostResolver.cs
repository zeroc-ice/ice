// Copyright (c) ZeroC, Inc.

using System.Diagnostics;
using System.Net;

namespace Ice.Internal;

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
                instance.initializationData().properties.getIceProperty("Ice.ThreadPriority")));
    }

    public void resolve(string host, int port, IPEndpointI endpoint, EndpointI_connectors callback)
    {
        //
        // Try to get the addresses without DNS lookup. If this doesn't work, we queue a resolve
        // entry and the thread will take care of getting the endpoint addresses.
        //
        NetworkProxy networkProxy = _instance.networkProxy();
        if (networkProxy == null)
        {
            try
            {
                List<EndPoint> addrs = Network.getAddresses(host, port, _protocol, _preferIPv6, false);
                if (addrs.Count > 0)
                {
                    callback.connectors(endpoint.connectors(addrs, null));
                    return;
                }
            }
            catch (Ice.LocalException ex)
            {
                callback.exception(ex);
                return;
            }
        }

        lock (_mutex)
        {
            Debug.Assert(!_destroyed);

            ResolveEntry entry = new ResolveEntry();
            entry.host = host;
            entry.port = port;
            entry.endpoint = endpoint;
            entry.callback = callback;

            Ice.Instrumentation.CommunicatorObserver obsv = _instance.initializationData().observer;
            if (obsv != null)
            {
                entry.observer = obsv.getEndpointLookupObserver(endpoint);
                entry.observer?.attach();
            }

            _queue.AddLast(entry);
            Monitor.Pulse(_mutex);
        }
    }

    public void destroy()
    {
        lock (_mutex)
        {
            Debug.Assert(!_destroyed);
            _destroyed = true;
            Monitor.Pulse(_mutex);
        }
    }

    public void joinWithThread()
    {
        _thread?.Join();
    }

    public void run()
    {
        while (true)
        {
            ResolveEntry r;
            Ice.Instrumentation.ThreadObserver threadObserver;

            lock (_mutex)
            {
                while (!_destroyed && _queue.Count == 0)
                {
                    Monitor.Wait(_mutex);
                }

                if (_destroyed)
                {
                    break;
                }

                r = _queue.First.Value;
                _queue.RemoveFirst();
                threadObserver = _observer;
            }

            threadObserver?.stateChanged(
                    Ice.Instrumentation.ThreadState.ThreadStateIdle,
                    Ice.Instrumentation.ThreadState.ThreadStateInUseForOther);

            try
            {
                NetworkProxy networkProxy = _instance.networkProxy();
                int protocol = _protocol;
                if (networkProxy != null)
                {
                    networkProxy = networkProxy.resolveHost(protocol);
                    if (networkProxy != null)
                    {
                        protocol = networkProxy.getProtocolSupport();
                    }
                }

                List<EndPoint> addrs = Network.getAddresses(r.host, r.port, protocol, _preferIPv6, true);
                if (r.observer != null)
                {
                    r.observer.detach();
                    r.observer = null;
                }

                r.callback.connectors(r.endpoint.connectors(addrs, networkProxy));
            }
            catch (Ice.LocalException ex)
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
                threadObserver?.stateChanged(
                        Ice.Instrumentation.ThreadState.ThreadStateInUseForOther,
                        Ice.Instrumentation.ThreadState.ThreadStateIdle);
            }
        }

        foreach (ResolveEntry entry in _queue)
        {
            Ice.CommunicatorDestroyedException ex = new Ice.CommunicatorDestroyedException();
            if (entry.observer != null)
            {
                entry.observer.failed(ex.ice_id());
                entry.observer.detach();
            }
            entry.callback.exception(ex);
        }
        _queue.Clear();

        _observer?.detach();
    }

    public void
    updateObserver()
    {
        lock (_mutex)
        {
            Ice.Instrumentation.CommunicatorObserver obsv = _instance.initializationData().observer;
            if (obsv != null)
            {
                _observer = obsv.getThreadObserver(
                    "Communicator",
                    _thread.getName(),
                    Ice.Instrumentation.ThreadState.ThreadStateIdle,
                    _observer);
                _observer?.attach();
            }
        }
    }

    private class ResolveEntry
    {
        internal string host;
        internal int port;
        internal IPEndpointI endpoint;
        internal EndpointI_connectors callback;
        internal Ice.Instrumentation.Observer observer;
    }

    private readonly Instance _instance;
    private readonly int _protocol;
    private readonly bool _preferIPv6;
    private bool _destroyed;
    private readonly LinkedList<ResolveEntry> _queue = new LinkedList<ResolveEntry>();
    private Ice.Instrumentation.ThreadObserver _observer;

    private sealed class HelperThread
    {
        internal HelperThread(EndpointHostResolver resolver)
        {
            _resolver = resolver;
            _name = _resolver._instance.initializationData().properties.getIceProperty("Ice.ProgramName");
            if (_name.Length > 0)
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
            catch (System.Exception ex)
            {
                string s = "exception in endpoint host resolver thread " + _name + ":\n" + ex;
                _resolver._instance.initializationData().logger.error(s);
            }
        }

        public string getName()
        {
            return _name;
        }

        private readonly EndpointHostResolver _resolver;
        private readonly string _name;
        private Thread _thread;
    }

    private readonly HelperThread _thread;
    private readonly object _mutex = new();
}
