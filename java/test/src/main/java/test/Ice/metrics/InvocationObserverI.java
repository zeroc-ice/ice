// Copyright (c) ZeroC, Inc.

package test.Ice.metrics;

import com.zeroc.Ice.ConnectionInfo;
import com.zeroc.Ice.Endpoint;
import com.zeroc.Ice.Instrumentation.CollocatedObserver;
import com.zeroc.Ice.Instrumentation.InvocationObserver;
import com.zeroc.Ice.Instrumentation.RemoteObserver;
import com.zeroc.Ice.ObjectAdapter;

class InvocationObserverI extends ObserverI
    implements InvocationObserver {
    @Override
    public synchronized void reset() {
        super.reset();
        retriedCount = 0;
        userExceptionCount = 0;
        if (remoteObserver != null) {
            remoteObserver.reset();
        }
        if (collocatedObserver != null) {
            collocatedObserver.reset();
        }
    }

    @Override
    public synchronized void retried() {
        ++retriedCount;
    }

    @Override
    public synchronized void userException() {
        ++userExceptionCount;
    }

    @Override
    public synchronized RemoteObserver getRemoteObserver(
            ConnectionInfo c, Endpoint e, int a, int b) {
        if (remoteObserver == null) {
            remoteObserver = new RemoteObserverI();
            remoteObserver.reset();
        }
        return remoteObserver;
    }

    @Override
    public synchronized CollocatedObserver getCollocatedObserver(
            ObjectAdapter adapter, int a, int b) {
        if (collocatedObserver == null) {
            collocatedObserver = new CollocatedObserverI();
            collocatedObserver.reset();
        }
        return collocatedObserver;
    }

    int userExceptionCount;
    int retriedCount;

    RemoteObserverI remoteObserver;
    CollocatedObserverI collocatedObserver;
}
