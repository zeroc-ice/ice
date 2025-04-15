// Copyright (c) ZeroC, Inc.

package test.Ice.metrics;

import com.zeroc.Ice.Instrumentation.ConnectionObserver;

class ConnectionObserverI extends ObserverI
    implements ConnectionObserver {
    @Override
    public synchronized void reset() {
        super.reset();
        received = 0;
        sent = 0;
    }

    @Override
    public synchronized void sentBytes(int s) {
        sent += s;
    }

    @Override
    public synchronized void receivedBytes(int s) {
        received += s;
    }

    int sent;
    int received;
}
