// Copyright (c) ZeroC, Inc.

package test.IceGrid.simple;

import com.zeroc.Ice.Current;

import test.IceGrid.simple.Test.TestIntf;

public class TestI implements TestIntf {
    public TestI() {
    }

    @Override
    public void shutdown(Current current) {
        current.adapter.getCommunicator().shutdown();
    }
}
