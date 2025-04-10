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

    public TestControllerI(ObjectAdapter adapter) {
        _adapter = adapter;
    }

    private final ObjectAdapter _adapter;
}
