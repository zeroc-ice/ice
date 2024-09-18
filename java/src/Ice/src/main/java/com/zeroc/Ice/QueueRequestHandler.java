//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package com.zeroc.Ice;

import java.util.concurrent.Callable;

final class QueueRequestHandler implements RequestHandler {
    public QueueRequestHandler(Instance instance, RequestHandler delegate) {
        _executor = instance.getQueueExecutor();
        assert (delegate != null);
        _delegate = delegate;
    }

    @Override
    public int sendAsyncRequest(final ProxyOutgoingAsyncBase out) throws RetryException {
        return _executor.execute(
                new Callable<Integer>() {
                    @Override
                    public Integer call() throws RetryException {
                        return _delegate.sendAsyncRequest(out);
                    }
                });
    }

    @Override
    public void asyncRequestCanceled(final OutgoingAsyncBase outAsync, final LocalException ex) {
        _executor.executeNoThrow(
                new Callable<Void>() {
                    @Override
                    public Void call() {
                        _delegate.asyncRequestCanceled(outAsync, ex);
                        return null;
                    }
                });
    }

    @Override
    public ConnectionI getConnection() {
        return _delegate.getConnection();
    }

    private final RequestHandler _delegate;
    private final QueueExecutorService _executor;
}
