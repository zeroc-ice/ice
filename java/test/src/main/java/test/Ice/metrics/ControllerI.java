// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
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
