// Copyright (c) ZeroC, Inc.

package test.Ice.faultTolerance;

import com.zeroc.Ice.Current;

import test.Ice.faultTolerance.Test.TestIntf;

public final class TestI implements TestIntf {
    public TestI(int port) {
        _pseudoPid = port; // We use the port number instead of the process ID in Java.
    }

    @Override
    public void shutdown(Current current) {
        current.adapter.getCommunicator().shutdown();
    }

    @Override
    public void abort(Current current) {
        Runtime.getRuntime().halt(0);
    }

    @Override
    public void idempotentAbort(Current current) {
        Runtime.getRuntime().halt(0);
    }

    @Override
    public int pid(Current current) {
        return _pseudoPid;
    }

    private final int _pseudoPid;
}
