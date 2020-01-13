//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using Test;

internal class Background : IBackground
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
        current.Adapter.Communicator.shutdown();
    }

    internal Background(BackgroundController controller)
    {
        _controller = controller;
    }

    private BackgroundController _controller;
}
