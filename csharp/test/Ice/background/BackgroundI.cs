// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using Test;

internal class BackgroundI : BackgroundDisp_
{
    public override void op(Ice.Current current)
    {
        _controller.checkCallPause(current);
    }

    public override void opWithPayload(byte[] seq, Ice.Current current)
    {
        _controller.checkCallPause(current);
    }

    public override void shutdown(Ice.Current current)
    {
        current.adapter.getCommunicator().shutdown();
    }

    internal BackgroundI(BackgroundControllerI controller)
    {
        _controller = controller;
    }

    private BackgroundControllerI _controller;
}
