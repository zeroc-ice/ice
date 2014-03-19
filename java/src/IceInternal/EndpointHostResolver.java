// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceInternal;

public class EndpointHostResolver
{
    EndpointHostResolver(Instance instance)
    {
        _instance = instance;
        _protocol = instance.protocolSupport();
        _preferIPv6 = instance.preferIPv6();
        try
        {
            _thread = new HelperThread();
            updateObserver();
            if(_instance.initializationData().properties.getProperty("Ice.ThreadPriority").length() > 0)
            {
                _thread.setPriority(Util.getThreadPriorityProperty(_instance.initializationData().properties, "Ice"));
            }
            _thread.start();
        }
        catch(RuntimeException ex)
        {
            String s = "cannot create thread for endpoint host resolver thread:\n" + Ex.toString(ex);
            _instance.initializationData().logger.error(s);
            throw ex;
        }
    }

    public java.util.List<Connector>
    resolve(String host, int port, Ice.EndpointSelectionType selType, EndpointI endpoint)
    {
        //
        // Try to get the addresses without DNS lookup. If this doesn't
        // work, we retry with DNS lookup (and observer).
        //
        NetworkProxy networkProxy = _instance.networkProxy();
        if(networkProxy == null)
        {
            java.util.List<java.net.InetSocketAddress> addrs =
                Network.getAddresses(host, port, _protocol, selType, _preferIPv6);
            if(!addrs.isEmpty())
            {
                return endpoint.connectors(addrs, null);
            }
        }

        Ice.Instrumentation.CommunicatorObserver obsv = _instance.getObserver();
        Ice.Instrumentation.Observer observer = null;
        if(obsv != null)
        {
            observer = obsv.getEndpointLookupObserver(endpoint);
            if(observer != null)
            {
                observer.attach();
            }
        }

        java.util.List<Connector> connectors = null;
        try
        {
            if(networkProxy != null)
            {
                networkProxy = networkProxy.resolveHost();
            }

            connectors = endpoint.connectors(Network.getAddresses(host, port, _protocol, selType, _preferIPv6),
                                             networkProxy);
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

    synchronized public void
    resolve(String host, int port, Ice.EndpointSelectionType selType, EndpointI endpoint, EndpointI_connectors callback)
    {
        //
        // TODO: Optimize to avoid the lookup if the given host is a textual IPv4 or IPv6
        // address. This requires implementing parsing of IPv4/IPv6 addresses (Java does
        // not provide such methods).
        //

        assert(!_destroyed);

        ResolveEntry entry = new ResolveEntry();
        entry.host = host;
        entry.port = port;
        entry.selType = selType;
        entry.endpoint = endpoint;
        entry.callback = callback;

        Ice.Instrumentation.CommunicatorObserver obsv = _instance.getObserver();
        if(obsv != null)
        {
            entry.observer = obsv.getEndpointLookupObserver(endpoint);
            if(entry.observer != null)
            {
                entry.observer.attach();
            }
        }

        _queue.add(entry);
        notify();
    }

    synchronized public void
    destroy()
    {
        assert(!_destroyed);
        _destroyed = true;
        notify();
    }

    public void
    joinWithThread()
    {
        if(_thread != null)
        {
            try
            {
                _thread.join();
            }
            catch(InterruptedException ex)
            {
            }
            if(_observer != null)
            {
                _observer.detach();
            }
        }
    }

    public void
    run()
    {
        while(true)
        {
            ResolveEntry r;
            Ice.Instrumentation.ThreadObserver threadObserver;
            synchronized(this)
            {
                while(!_destroyed && _queue.isEmpty())
                {
                    try
                    {
                        wait();
                    }
                    catch(java.lang.InterruptedException ex)
                    {
                    }
                }

                if(_destroyed)
                {
                    break;
                }

                r = (ResolveEntry)_queue.removeFirst();
                threadObserver = _observer;
            }

            try
            {
                if(threadObserver != null)
                {
                    threadObserver.stateChanged(Ice.Instrumentation.ThreadState.ThreadStateIdle,
                                                Ice.Instrumentation.ThreadState.ThreadStateInUseForOther);
                }

                NetworkProxy networkProxy = _instance.networkProxy();
                if(networkProxy != null)
                {
                    networkProxy = networkProxy.resolveHost();
                }

                r.callback.connectors(r.endpoint.connectors(Network.getAddresses(r.host,
                                                                                 r.port,
                                                                                 _protocol,
                                                                                 r.selType,
                                                                                 _preferIPv6),
                                                            networkProxy));

                if(threadObserver != null)
                {
                    threadObserver.stateChanged(Ice.Instrumentation.ThreadState.ThreadStateInUseForOther,
                                                Ice.Instrumentation.ThreadState.ThreadStateIdle);
                }

                if(r.observer != null)
                {
                    r.observer.detach();
                }
            }
            catch(Ice.LocalException ex)
            {
                if(r.observer != null)
                {
                    r.observer.failed(ex.ice_name());
                    r.observer.detach();
                }
                r.callback.exception(ex);
            }
        }

        for(ResolveEntry entry : _queue)
        {
            Ice.CommunicatorDestroyedException ex = new Ice.CommunicatorDestroyedException();
            if(entry.observer != null)
            {
                entry.observer.failed(ex.ice_name());
                entry.observer.detach();
            }
            entry.callback.exception(ex);
        }
        _queue.clear();
    }

    synchronized public void
    updateObserver()
    {
        Ice.Instrumentation.CommunicatorObserver obsv = _instance.getObserver();
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

    static class ResolveEntry
    {
        String host;
        int port;
        Ice.EndpointSelectionType selType;
        EndpointI endpoint;
        EndpointI_connectors callback;
        Ice.Instrumentation.Observer observer;
    }

    private final Instance _instance;
    private final int _protocol;
    private final boolean _preferIPv6;
    private boolean _destroyed;
    private java.util.LinkedList<ResolveEntry> _queue = new java.util.LinkedList<ResolveEntry>();
    private Ice.Instrumentation.ThreadObserver _observer;

    private final class HelperThread extends Thread
    {
        HelperThread()
        {
            String threadName = _instance.initializationData().properties.getProperty("Ice.ProgramName");
            if(threadName.length() > 0)
            {
                threadName += "-";
            }
            setName(threadName + "Ice.HostResolver");
        }

        public void
        run()
        {
            try
            {
                EndpointHostResolver.this.run();
            }
            catch(java.lang.Exception ex)
            {
                String s = "exception in endpoint host resolver thread " + getName() + ":\n" + Ex.toString(ex);
                _instance.initializationData().logger.error(s);
            }
        }
    }

    private HelperThread _thread;
}
