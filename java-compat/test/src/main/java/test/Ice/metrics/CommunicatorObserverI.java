// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.metrics;

class CommunicatorObserverI implements Ice.Instrumentation.CommunicatorObserver
{
    private static void
    test(boolean b)
    {
        if(!b)
        {
            throw new RuntimeException();
        }
    }

    @Override
    public void 
    setObserverUpdater(Ice.Instrumentation.ObserverUpdater u)
    {
        updater = u;
    }
 
    @Override
    synchronized public Ice.Instrumentation.Observer 
    getConnectionEstablishmentObserver(Ice.Endpoint e, String s)
    {
        if(connectionEstablishmentObserver == null)
        {
            connectionEstablishmentObserver = new ObserverI();
            connectionEstablishmentObserver.reset();
        }
        return connectionEstablishmentObserver;
    }

 
    @Override
    synchronized public Ice.Instrumentation.Observer 
    getEndpointLookupObserver(Ice.Endpoint e)
    {
        if(endpointLookupObserver == null)
        {
            endpointLookupObserver = new ObserverI();
            endpointLookupObserver.reset();
        }
        return endpointLookupObserver;
    }
    
    @Override
    synchronized public Ice.Instrumentation.ConnectionObserver 
    getConnectionObserver(Ice.ConnectionInfo c,
                          Ice.Endpoint e,
                          Ice.Instrumentation.ConnectionState s, 
                          Ice.Instrumentation.ConnectionObserver old)
    {
        test(old == null || old instanceof ConnectionObserverI);
        if(connectionObserver == null)
        {
            connectionObserver = new ConnectionObserverI();
            connectionObserver.reset();
        }
        return connectionObserver;
    }

    @Override
    synchronized public Ice.Instrumentation.ThreadObserver 
    getThreadObserver(String p, String id, Ice.Instrumentation.ThreadState s,
                      Ice.Instrumentation.ThreadObserver old)
    {
        test(old == null || old instanceof ThreadObserverI);
        if(threadObserver == null)
        {
            threadObserver = new ThreadObserverI();
            threadObserver.reset();
        }
        return threadObserver; 
   }

    @Override
    synchronized public Ice.Instrumentation.InvocationObserver
    getInvocationObserver(Ice.ObjectPrx p, String op, java.util.Map<String, String> ctx)
    {
        if(invocationObserver == null)
        {
            invocationObserver = new InvocationObserverI();
            invocationObserver.reset();
        }
        return invocationObserver;
    }

    @Override
    synchronized public Ice.Instrumentation.DispatchObserver
    getDispatchObserver(Ice.Current current, int s)
    {
        if(dispatchObserver == null)
        {
            dispatchObserver = new DispatchObserverI();
            dispatchObserver.reset();
        }
        return dispatchObserver;
    }

    synchronized void
    reset()
    {
        if(connectionEstablishmentObserver != null)
        {
            connectionEstablishmentObserver.reset();
        }
        if(endpointLookupObserver != null)
        {
            endpointLookupObserver.reset();
        }
        if(connectionObserver != null)
        {
            connectionObserver.reset();
        }
        if(threadObserver != null)
        {
            threadObserver.reset();
        }
        if(invocationObserver != null)
        {
            invocationObserver.reset();
        }
        if(dispatchObserver != null)
        {
            dispatchObserver.reset();
        }
    }
    
    Ice.Instrumentation.ObserverUpdater updater;

    ObserverI connectionEstablishmentObserver;
    ObserverI endpointLookupObserver;
    ConnectionObserverI connectionObserver;
    ThreadObserverI threadObserver;
    InvocationObserverI invocationObserver;
    DispatchObserverI dispatchObserver;
};

