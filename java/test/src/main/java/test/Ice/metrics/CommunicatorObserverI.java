//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package test.Ice.metrics;

class CommunicatorObserverI implements com.zeroc.Ice.Instrumentation.CommunicatorObserver
{
    private static void test(boolean b)
    {
        if(!b)
        {
            throw new RuntimeException();
        }
    }

    @Override
    public void setObserverUpdater(com.zeroc.Ice.Instrumentation.ObserverUpdater u)
    {
        updater = u;
    }

    @Override
    synchronized public com.zeroc.Ice.Instrumentation.Observer getConnectionEstablishmentObserver(
        com.zeroc.Ice.Endpoint e, String s)
    {
        if(connectionEstablishmentObserver == null)
        {
            connectionEstablishmentObserver = new ObserverI();
            connectionEstablishmentObserver.reset();
        }
        return connectionEstablishmentObserver;
    }

    @Override
    synchronized public com.zeroc.Ice.Instrumentation.Observer getEndpointLookupObserver(com.zeroc.Ice.Endpoint e)
    {
        if(endpointLookupObserver == null)
        {
            endpointLookupObserver = new ObserverI();
            endpointLookupObserver.reset();
        }
        return endpointLookupObserver;
    }

    @Override
    synchronized public com.zeroc.Ice.Instrumentation.ConnectionObserver getConnectionObserver(
        com.zeroc.Ice.ConnectionInfo c,
        com.zeroc.Ice.Endpoint e,
        com.zeroc.Ice.Instrumentation.ConnectionState s,
        com.zeroc.Ice.Instrumentation.ConnectionObserver old)
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
    synchronized public com.zeroc.Ice.Instrumentation.ThreadObserver getThreadObserver(
        String p,
        String id,
        com.zeroc.Ice.Instrumentation.ThreadState s,
        com.zeroc.Ice.Instrumentation.ThreadObserver old)
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
    synchronized public com.zeroc.Ice.Instrumentation.InvocationObserver getInvocationObserver(
        com.zeroc.Ice.ObjectPrx p,
        String op,
        java.util.Map<String, String> ctx)
    {
        if(invocationObserver == null)
        {
            invocationObserver = new InvocationObserverI();
            invocationObserver.reset();
        }
        return invocationObserver;
    }

    @Override
    synchronized public com.zeroc.Ice.Instrumentation.DispatchObserver getDispatchObserver(
        com.zeroc.Ice.Current current,
        int s)
    {
        if(dispatchObserver == null)
        {
            dispatchObserver = new DispatchObserverI();
            dispatchObserver.reset();
        }
        return dispatchObserver;
    }

    synchronized void reset()
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

    com.zeroc.Ice.Instrumentation.ObserverUpdater updater;

    ObserverI connectionEstablishmentObserver;
    ObserverI endpointLookupObserver;
    ConnectionObserverI connectionObserver;
    ThreadObserverI threadObserver;
    InvocationObserverI invocationObserver;
    DispatchObserverI dispatchObserver;
}
