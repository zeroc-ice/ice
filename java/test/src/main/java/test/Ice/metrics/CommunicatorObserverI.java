// Copyright (c) ZeroC, Inc.

package test.Ice.metrics;

import com.zeroc.Ice.ConnectionInfo;
import com.zeroc.Ice.Current;
import com.zeroc.Ice.Endpoint;
import com.zeroc.Ice.ObjectPrx;
import com.zeroc.Ice.Instrumentation.CommunicatorObserver;
import com.zeroc.Ice.Instrumentation.ConnectionObserver;
import com.zeroc.Ice.Instrumentation.ConnectionState;
import com.zeroc.Ice.Instrumentation.DispatchObserver;
import com.zeroc.Ice.Instrumentation.InvocationObserver;
import com.zeroc.Ice.Instrumentation.Observer;
import com.zeroc.Ice.Instrumentation.ObserverUpdater;
import com.zeroc.Ice.Instrumentation.ThreadObserver;
import com.zeroc.Ice.Instrumentation.ThreadState;

import java.util.Map;

class CommunicatorObserverI implements CommunicatorObserver {
    private static void test(boolean b) {
        if (!b) {
            throw new RuntimeException();
        }
    }

    @Override
    public void setObserverUpdater(ObserverUpdater u) {
        updater = u;
    }

    @Override
    public synchronized Observer getConnectionEstablishmentObserver(
            Endpoint e, String s) {
        if (connectionEstablishmentObserver == null) {
            connectionEstablishmentObserver = new ObserverI();
            connectionEstablishmentObserver.reset();
        }
        return connectionEstablishmentObserver;
    }

    @Override
    public synchronized Observer getEndpointLookupObserver(
            Endpoint e) {
        if (endpointLookupObserver == null) {
            endpointLookupObserver = new ObserverI();
            endpointLookupObserver.reset();
        }
        return endpointLookupObserver;
    }

    @Override
    public synchronized ConnectionObserver getConnectionObserver(
            ConnectionInfo c,
            Endpoint e,
            ConnectionState s,
            ConnectionObserver old) {
        test(old == null || old instanceof ConnectionObserverI);
        if (connectionObserver == null) {
            connectionObserver = new ConnectionObserverI();
            connectionObserver.reset();
        }
        return connectionObserver;
    }

    @Override
    public synchronized ThreadObserver getThreadObserver(
            String p,
            String id,
            ThreadState s,
            ThreadObserver old) {
        test(old == null || old instanceof ThreadObserverI);
        if (threadObserver == null) {
            threadObserver = new ThreadObserverI();
            threadObserver.reset();
        }
        return threadObserver;
    }

    @Override
    public synchronized InvocationObserver getInvocationObserver(
            ObjectPrx p, String op, Map<String, String> ctx) {
        if (invocationObserver == null) {
            invocationObserver = new InvocationObserverI();
            invocationObserver.reset();
        }
        return invocationObserver;
    }

    @Override
    public synchronized DispatchObserver getDispatchObserver(
            Current current, int s) {
        if (dispatchObserver == null) {
            dispatchObserver = new DispatchObserverI();
            dispatchObserver.reset();
        }
        return dispatchObserver;
    }

    synchronized void reset() {
        if (connectionEstablishmentObserver != null) {
            connectionEstablishmentObserver.reset();
        }
        if (endpointLookupObserver != null) {
            endpointLookupObserver.reset();
        }
        if (connectionObserver != null) {
            connectionObserver.reset();
        }
        if (threadObserver != null) {
            threadObserver.reset();
        }
        if (invocationObserver != null) {
            invocationObserver.reset();
        }
        if (dispatchObserver != null) {
            dispatchObserver.reset();
        }
    }

    ObserverUpdater updater;

    ObserverI connectionEstablishmentObserver;
    ObserverI endpointLookupObserver;
    ConnectionObserverI connectionObserver;
    ThreadObserverI threadObserver;
    InvocationObserverI invocationObserver;
    DispatchObserverI dispatchObserver;
}
