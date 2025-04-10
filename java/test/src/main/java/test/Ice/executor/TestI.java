// Copyright (c) ZeroC, Inc.

package test.Ice.executor;

import com.zeroc.Ice.Current;

import test.Ice.executor.Test.TestIntf;

public class TestI implements TestIntf {
    private static void test(boolean b) {
        if (!b) {
            throw new RuntimeException();
        }
    }

    TestI(CustomExecutor executor) {
        _executor = executor;
    }

    @Override
    public void op(Current current) {
        test(_executor.isCustomExecutorThread());
    }

    @Override
    public void sleep(int to, Current current) {
        try {
            Thread.sleep(to);
        } catch (InterruptedException ex) {
            System.err.println("sleep interrupted");
        }
    }

    @Override
    public void opWithPayload(byte[] seq, Current current) {
        test(_executor.isCustomExecutorThread());
    }

    @Override
    public void shutdown(Current current) {
        current.adapter.getCommunicator().shutdown();
    }

    private final CustomExecutor _executor;
}
