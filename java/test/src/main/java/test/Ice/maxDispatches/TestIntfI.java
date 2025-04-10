// Copyright (c) ZeroC, Inc.

package test.Ice.maxDispatches;

import com.zeroc.Ice.Current;

import test.Ice.maxDispatches.Test.TestIntf;

import java.util.concurrent.CompletableFuture;
import java.util.concurrent.CompletionStage;

class TestIntfI implements TestIntf {
    private int _dispatchCount;
    private int _maxDispatchCount;
    private final ResponderI _responder;

    @Override
    public CompletionStage<Void> opAsync(Current current) {
        synchronized (this) {
            _dispatchCount++;
            _maxDispatchCount = Math.max(_maxDispatchCount, _dispatchCount);
        }

        var future = new CompletableFuture<Void>();

        _responder.queueResponse(
            () -> {
                decDispatchCount();
                future.complete(null);
            });

        return future;
    }

    @Override
    public synchronized int resetMaxConcurrentDispatches(Current current) {
        int result = _maxDispatchCount;
        _maxDispatchCount = 0;
        return result;
    }

    @Override
    public void shutdown(Current current) {
        current.adapter.getCommunicator().shutdown();
    }

    TestIntfI(ResponderI responder) {
        _responder = responder;
    }

    private synchronized void decDispatchCount() {
        _dispatchCount--;
    }
}
