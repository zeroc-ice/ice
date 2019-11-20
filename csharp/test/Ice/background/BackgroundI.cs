//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using Test;

internal class BackgroundI : Background
{
    public void op(Ice.Current current)
    {
        _controller.checkCallPause(current);
    }

    public void opWithPayload(byte[] seq, Ice.Current current)
    {
        _controller.checkCallPause(current);
    }

    public void shutdown(Ice.Current current)
    {
        current.adapter.GetCommunicator().shutdown();
    }

    internal BackgroundI(BackgroundControllerI controller)
    {
        _controller = controller;
    }

    private BackgroundControllerI _controller;
}
