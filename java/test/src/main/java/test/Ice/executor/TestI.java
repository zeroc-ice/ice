// Copyright (c) ZeroC, Inc.

package test.Ice.executor;

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
    public void op(com.zeroc.Ice.Current current) {
        test(_executor.isCustomExecutorThread());
    }

    @Override
    public void sleep(int to, com.zeroc.Ice.Current current) {
        try {
            Thread.sleep(to);
        } catch (InterruptedException ex) {
            System.err.println("sleep interrupted");
        }
    }

    @Override
    public void opWithPayload(byte[] seq, com.zeroc.Ice.Current current) {
        test(_executor.isCustomExecutorThread());
    }

    @Override
    public void shutdown(com.zeroc.Ice.Current current) {
        current.adapter.getCommunicator().shutdown();
    }

    private CustomExecutor _executor;
}
