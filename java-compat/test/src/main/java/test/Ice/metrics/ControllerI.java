// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

package test.Ice.metrics;
import test.Ice.metrics.Test._ControllerDisp;

public final class ControllerI extends _ControllerDisp
{
    public ControllerI(Ice.ObjectAdapter adapter)
    {
        _adapter = adapter;
    }

    @Override
    public void hold(Ice.Current current)
    {
        _adapter.hold();
        _adapter.waitForHold();
    }

    @Override
    public void resume(Ice.Current current)
    {
        _adapter.activate();
    }

    final private Ice.ObjectAdapter _adapter;
};
