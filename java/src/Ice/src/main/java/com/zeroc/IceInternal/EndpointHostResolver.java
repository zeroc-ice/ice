// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package com.zeroc.IceInternal;

class EndpointHostResolver
{
    EndpointHostResolver(Instance instance)
    {
        _instance = instance;
        _protocol = instance.protocolSupport();
        _preferIPv6 = instance.preferIPv6();
        try
        {
            _threadName = Util.createThreadName(_instance.initializationData().properties, "Ice.HostResolver");
            _executor = java.util.concurrent.Executors.newFixedThreadPool(1,
                            Util.createThreadFactory(_instance.initializationData().properties, _threadName));
            updateObserver();
        }
        catch(RuntimeException ex)
        {
            String s = "cannot create thread for endpoint host resolver thread:\n" + Ex.toString(ex);
            _instance.initializationData().logger.error(s);
            throw ex;
        }
    }

    synchronized void resolve(final String host, final int port, final com.zeroc.Ice.EndpointSelectionType selType, 
                              final IPEndpointI endpoint, final EndpointI_connectors callback)
    {
        //
        // TODO: Optimize to avoid the lookup if the given host is a textual IPv4 or IPv6
        // address. This requires implementing parsing of IPv4/IPv6 addresses (Java does
        // not provide such methods).
        //

        assert(!_destroyed);

        NetworkProxy networkProxy = _instance.networkProxy();
        if(networkProxy == null)
        {
            java.util.List<java.net.InetSocketAddress> addrs = Network.getAddresses(host, port, _protocol, selType,
                                                                                    _preferIPv6, false);
            if(addrs != null)
            {
                callback.connectors(endpoint.connectors(addrs, networkProxy));
                return;
            }
        }

        final com.zeroc.Ice.Instrumentation.ThreadObserver threadObserver = _observer;
        final com.zeroc.Ice.Instrumentation.Observer observer = getObserver(endpoint);
        if(observer != null)
        {
            observer.attach();
        }

        _executor.execute(new Runnable()
            {
                @Override
                public void run()
                {
                    synchronized(EndpointHostResolver.this)
                    {
                        if(_destroyed)
                        {
                            com.zeroc.Ice.CommunicatorDestroyedException ex =
                                new com.zeroc.Ice.CommunicatorDestroyedException();
                            if(observer != null)
                            {
                                observer.failed(ex.ice_id());
                                observer.detach();
                            }
                            callback.exception(ex);
                            return;
                        }
                    }

                    if(threadObserver != null)
                    {
                        threadObserver.stateChanged(com.zeroc.Ice.Instrumentation.ThreadState.ThreadStateIdle,
                                                    com.zeroc.Ice.Instrumentation.ThreadState.ThreadStateInUseForOther);
                    }

                    try
                    {
                        int protocol = _protocol;
                        NetworkProxy networkProxy = _instance.networkProxy();
                        if(networkProxy != null)
                        {
                            networkProxy = networkProxy.resolveHost(_protocol);
                            if(networkProxy != null)
                            {
                                protocol = networkProxy.getProtocolSupport();
                            }
                        }

                        callback.connectors(endpoint.connectors(Network.getAddresses(host,
                                                                                     port,
                                                                                     protocol,
                                                                                     selType,
                                                                                     _preferIPv6,
                                                                                     true),
                                                                networkProxy));                        
                    }
                    catch(com.zeroc.Ice.LocalException ex)
                    {
                        if(observer != null)
                        {
                            observer.failed(ex.ice_id());
                        }
                        callback.exception(ex);
                    }
                    finally
                    {
                        if(threadObserver != null)
                        {
                            threadObserver.stateChanged(
                                com.zeroc.Ice.Instrumentation.ThreadState.ThreadStateInUseForOther,
                                com.zeroc.Ice.Instrumentation.ThreadState.ThreadStateIdle);
                        }
                        if(observer != null)
                        {
                            observer.detach();
                        }
                    }
                }
            });
    }

    synchronized void destroy()
    {
        assert(!_destroyed);
        _destroyed = true;

        //
        // Shutdown the executor. No new tasks will be accepted.
        // Existing tasks will execute.
        //
        _executor.shutdown();
    }

    void joinWithThread()
        throws InterruptedException
    {
        // Wait for the executor to terminate.
        try
        {
            while(!_executor.isTerminated())
            {
                // A very long time.
                _executor.awaitTermination(100000, java.util.concurrent.TimeUnit.SECONDS);
            }

        }
        finally
        {
            if(_observer != null)
            {
                _observer.detach();
            }
        }
    }

    synchronized void updateObserver()
    {
        com.zeroc.Ice.Instrumentation.CommunicatorObserver obsv = _instance.initializationData().observer;
        if(obsv != null)
        {
            _observer = obsv.getThreadObserver("Communicator", _threadName,
                                               com.zeroc.Ice.Instrumentation.ThreadState.ThreadStateIdle,
                                               _observer);
            if(_observer != null)
            {
                _observer.attach();
            }
        }
    }

    private com.zeroc.Ice.Instrumentation.Observer
    getObserver(IPEndpointI endpoint)
    {
        com.zeroc.Ice.Instrumentation.CommunicatorObserver obsv = _instance.initializationData().observer;
        if(obsv != null)
        {
            return obsv.getEndpointLookupObserver(endpoint);
        }
        return null;
    }

    private final Instance _instance;
    private final int _protocol;
    private final boolean _preferIPv6;
    private boolean _destroyed;
    private com.zeroc.Ice.Instrumentation.ThreadObserver _observer;
    private String _threadName;
    private java.util.concurrent.ExecutorService _executor;
}
