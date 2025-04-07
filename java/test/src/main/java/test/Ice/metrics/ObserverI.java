// Copyright (c) ZeroC, Inc.

package test.Ice.metrics;

import com.zeroc.Ice.Instrumentation.Observer;

class ObserverI implements Observer {
    public synchronized void reset() {
        total = 0;
        current = 0;
        failedCount = 0;
    }

    @Override
    public synchronized void attach() {
        ++total;
        ++current;
    }

    @Override
    public synchronized void detach() {
        --current;
    }

    @Override
    public synchronized void failed(String s) {
        ++failedCount;
    }

    synchronized int getTotal() {
        return total;
    }

    synchronized int getCurrent() {
        return current;
    }

    synchronized int getFailedCount() {
        return failedCount;
    }

    int total;
    int current;
    int failedCount;
}
