// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************
package test.Ice.background;

import test.Ice.background.Test._BackgroundDisp;

class BackgroundI extends _BackgroundDisp
{
    @Override
    public void
    op(Ice.Current current)
    {
        _controller.checkCallPause(current);
    }

    @Override
    public void
    opWithPayload(byte[] seq, Ice.Current current)
    {
        _controller.checkCallPause(current);
    }

    @Override
    public void
    shutdown(Ice.Current current)
    {
        current.adapter.getCommunicator().shutdown();
    }

    BackgroundI(BackgroundControllerI controller)
    {
        _controller = controller;
    }

    private final BackgroundControllerI _controller;
}
