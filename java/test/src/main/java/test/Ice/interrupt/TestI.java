// Copyright (c) ZeroC, Inc.

package test.Ice.interrupt;

import com.zeroc.Ice.Current;
import com.zeroc.Ice.UnknownException;

import test.Ice.interrupt.Test.InterruptedException;
import test.Ice.interrupt.Test.TestIntf;

public class TestI implements TestIntf {
    TestI(TestControllerI controller) {
        _controller = controller;
    }

    @Override
    public void op(Current current) {
    }

    @Override
    public void opIdempotent(Current current) {
        throw new UnknownException("dummy");
    }

    @Override
    public void sleep(int to, Current current)
        throws InterruptedException {
        _controller.addUpcallThread();
        try {
            Thread.sleep(to);
        } catch (java.lang.InterruptedException ex) {
            throw new InterruptedException();
        } finally {
            _controller.removeUpcallThread();
        }
    }

    @Override
    public void opWithPayload(byte[] seq, Current current) {
    }

    @Override
    public void shutdown(Current current) {
        current.adapter.getCommunicator().shutdown();
    }

    private final TestControllerI _controller;
}
