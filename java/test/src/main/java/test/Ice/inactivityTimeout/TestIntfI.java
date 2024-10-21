// Copyright (c) ZeroC, Inc.

package test.Ice.inactivityTimeout;

import com.zeroc.Ice.Current;

import test.Ice.inactivityTimeout.Test.TestIntf;

class TestIntfI implements TestIntf {
    @Override
    public java.util.concurrent.CompletionStage<Void> sleepAsync(int ms, Current current) {
        return java.util.concurrent.CompletableFuture.runAsync(
                () -> {
                    try {
                        Thread.sleep(ms);
                    } catch (InterruptedException ex) {
                        // ignored
                    }
                });
    }

    @Override
    public void shutdown(Current current) {
        current.adapter.getCommunicator().shutdown();
    }
}
