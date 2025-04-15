// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

import java.util.concurrent.Future;

class RetryTask implements Runnable, CancellationHandler {
    RetryTask(Instance instance, RetryQueue queue, ProxyOutgoingAsyncBase outAsync) {
        _instance = instance;
        _queue = queue;
        _outAsync = outAsync;
    }

    @Override
    public void run() {
        if (cancel()) {
            _outAsync.retry();

            //
            // NOTE: this must be called last, destroy() blocks until all task
            // are removed to prevent the client thread pool to be destroyed
            // (we still need the client thread pool at this point to call
            // exception callbacks with CommunicatorDestroyedException).
            //
            _queue.remove(this);
        }
    }

    @Override
    public void asyncRequestCanceled(OutgoingAsyncBase outAsync, LocalException ex) {
        if (_queue.remove(this) && cancel()) {
            if (_instance.traceLevels().retry >= 1) {
                StringBuilder s = new StringBuilder(128);
                s.append("operation retry canceled\n");
                s.append(Ex.toString(ex));
                _instance
                    .initializationData()
                    .logger
                    .trace(_instance.traceLevels().retryCat, s.toString());
            }
            if (_outAsync.completed(ex)) {
                _outAsync.invokeCompletedAsync();
            }
        }
    }

    public boolean destroy() {
        if (cancel()) {
            try {
                _outAsync.abort(new CommunicatorDestroyedException());
            } catch (CommunicatorDestroyedException ex) {
                // Abort can throw if there's no callback, just ignore in this case
            }
            return true;
        }
        return false;
    }

    public synchronized void setFuture(Future<?> future) {
        _future = future;
        if (_cancelled) {
            _future.cancel(false);
        }
    }

    private synchronized boolean cancel() {
        if (_cancelled) {
            return false;
        } else {
            if (_future != null) {
                _future.cancel(false);
            }
            _cancelled = true;
            return true;
        }
    }

    private final Instance _instance;
    private final RetryQueue _queue;
    private final ProxyOutgoingAsyncBase _outAsync;
    private Future<?> _future;
    private boolean _cancelled;
}
