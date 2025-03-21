// Copyright (c) ZeroC, Inc.

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

    private readonly BackgroundControllerI _controller;
}
