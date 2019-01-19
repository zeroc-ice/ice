//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package test.Ice.background;

import test.Ice.background.Test.Background;

class BackgroundI implements Background
{
    @Override
    public void op(com.zeroc.Ice.Current current)
    {
        _controller.checkCallPause(current);
    }

    @Override
    public void opWithPayload(byte[] seq, com.zeroc.Ice.Current current)
    {
        _controller.checkCallPause(current);
    }

    @Override
    public void shutdown(com.zeroc.Ice.Current current)
    {
        current.adapter.getCommunicator().shutdown();
    }

    BackgroundI(BackgroundControllerI controller)
    {
        _controller = controller;
    }

    private final BackgroundControllerI _controller;
}
