// Copyright (c) ZeroC, Inc.

package test.Ice.inactivityTimeout;

import com.zeroc.Ice.Current;

import test.Ice.inactivityTimeout.Test.TestIntf;

import java.util.concurrent.CompletableFuture;
import java.util.concurrent.CompletionStage;

class TestIntfI implements TestIntf {
    @Override
    public CompletionStage<Void> sleepAsync(int ms, Current current) {
        return CompletableFuture.runAsync(
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
