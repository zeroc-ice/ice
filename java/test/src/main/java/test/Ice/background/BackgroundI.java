// Copyright (c) ZeroC, Inc.

package test.Ice.background;

import com.zeroc.Ice.Current;

import test.Ice.background.Test.Background;

class BackgroundI implements Background {
    @Override
    public void op(Current current) {
        _controller.checkCallPause(current);
    }

    @Override
    public void opWithPayload(byte[] seq, Current current) {
        _controller.checkCallPause(current);
    }

    @Override
    public void shutdown(Current current) {
        current.adapter.getCommunicator().shutdown();
    }

    BackgroundI(BackgroundControllerI controller) {
        _controller = controller;
    }

    private final BackgroundControllerI _controller;
}
