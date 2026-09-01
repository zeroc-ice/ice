// Copyright (c) ZeroC, Inc.

package test.Ice.ami;

import com.zeroc.Ice.Current;
import com.zeroc.Ice.ObjectAdapter;

import test.Ice.ami.Test.TestIntfController;

class TestControllerI implements TestIntfController {
    @Override
    public void holdAdapter(Current current) {
        _adapter.hold();
    }

    @Override
    public void resumeAdapter(Current current) {
        _adapter.activate();
    }

    @Override
    public void waitForActiveSleepCalls(int count, Current current) {
        _test.waitForActiveSleepCalls(count);
    }

    public TestControllerI(ObjectAdapter adapter, TestI test) {
        _adapter = adapter;
        _test = test;
    }

    private final ObjectAdapter _adapter;
    private final TestI _test;
}
