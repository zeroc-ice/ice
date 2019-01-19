//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package test.Ice.metrics;

import test.Ice.metrics.Test.Controller;

public final class ControllerI implements Controller
{
    public ControllerI(com.zeroc.Ice.ObjectAdapter adapter)
    {
        _adapter = adapter;
    }

    @Override
    public void hold(com.zeroc.Ice.Current current)
    {
        _adapter.hold();
        _adapter.waitForHold();
    }

    @Override
    public void resume(com.zeroc.Ice.Current current)
    {
        _adapter.activate();
    }

    final private com.zeroc.Ice.ObjectAdapter _adapter;
}
