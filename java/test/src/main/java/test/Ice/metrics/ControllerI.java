// Copyright (c) ZeroC, Inc.

package test.Ice.metrics;

import com.zeroc.Ice.Current;
import com.zeroc.Ice.ObjectAdapter;

import test.Ice.metrics.Test.Controller;

public final class ControllerI implements Controller {
    public ControllerI(ObjectAdapter adapter) {
        _adapter = adapter;
    }

    @Override
    public void hold(Current current) {
        _adapter.hold();
        _adapter.waitForHold();
    }

    @Override
    public void resume(Current current) {
        _adapter.activate();
    }

    private final ObjectAdapter _adapter;
}
