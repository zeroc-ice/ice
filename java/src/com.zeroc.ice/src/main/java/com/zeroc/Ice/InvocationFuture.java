// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

import java.util.concurrent.CompletableFuture;
import java.util.concurrent.Executor;
import java.util.function.BiConsumer;

/**
 * An instance of an InvocationFuture subclass is the return value of an asynchronous invocation.
 * With this object, an application can obtain several attributes of the invocation.
 */
public abstract class InvocationFuture<T> extends CompletableFuture<T> {
    /**
     * If not completed, cancels the request. This is a local operation, it won't cancel the request
     * on the server side. Calling <code>cancel</code> prevents a queued request from being sent or
     * ignores a reply if the request has already been sent.
     *
     * @return True if this task is now cancelled.
     */
    public boolean cancel() {
        return cancel(false);
    }

    @Override
    public boolean cancel(boolean mayInterruptIfRunning) {
        //
        // Call super.cancel(boolean) first. This sets the result of the future.
        // Calling cancel(LocalException) also eventually attempts to complete the future
        // (exceptionally), but this result is ignored.
        //
        boolean r = super.cancel(mayInterruptIfRunning);
        cancel(new InvocationCanceledException());
        return r;
    }

    /**
     * Returns the communicator that sent the invocation.
     *
     * @return The communicator.
     */
    public Communicator getCommunicator() {
        return _communicator;
    }

    /**
     * Returns the connection that was used to start the invocation, or nil if this future was not
     * obtained via an asynchronous connection invocation (such as <code>flushBatchRequestsAsync
     * </code>).
     *
     * @return The connection.
     */
    public Connection getConnection() {
        return null;
    }

    /**
     * Returns the proxy that was used to start the asynchronous invocation, or nil if this object
     * was not obtained via an asynchronous proxy invocation.
     *
     * @return The proxy.
     */
    public ObjectPrx getProxy() {
        return null;
    }

    /**
     * Returns the name of the operation.
     *
     * @return The operation name.
     */
    public final String getOperation() {
        return _operation;
    }

    /** Blocks the caller until the result of the invocation is available. */
    public final void waitForCompleted() {
        try {
            join();
        } catch (java.util.concurrent.CompletionException completionException) {
            var cause = completionException.getCause();
            if (cause instanceof InterruptedException) {
                throw new OperationInterruptedException(cause);
            }
        } catch (Exception ex) {
        }
    }

    /**
     * When you start an asynchronous invocation, the Ice run time attempts to write the
     * corresponding request to the client-side transport. If the transport cannot accept the
     * request, the Ice run time queues the request for later transmission. This method returns true
     * if, at the time it is called, the request has been written to the local transport (whether it
     * was initially queued or not). Otherwise, if the request is still queued, this method returns
     * false.
     *
     * @return True if the request has been sent, or false if the request is queued.
     */
    public final boolean isSent() {
        synchronized (this) {
            return (_state & StateSent) > 0;
        }
    }

    /** Blocks the caller until the request has been written to the client-side transport. */
    public final synchronized void waitForSent() {
        while ((_state & StateSent) == 0 && _exception == null) {
            try {
                this.wait();
            } catch (InterruptedException ex) {
                throw new OperationInterruptedException(ex);
            }
        }
    }

    /**
     * Returns true if a request was written to the client-side transport without first being
     * queued. If the request was initially queued, this method returns false (independent of
     * whether the request is still in the queue or has since been written to the client-side
     * transport).
     *
     * @return True if the request was sent without being queued, or false otherwise.
     */
    public final boolean sentSynchronously() {
        return _sentSynchronously; // No lock needed, immutable
    }

    /**
     * Returns a future that completes when the entire request message has been accepted by the
     * transport and executes the given action. The boolean value indicates whether the message was
     * sent synchronously.
     *
     * @param action Executed when the future is completed successfully or exceptionally.
     * @return A future that completes when the message has been handed off to the transport.
     */
    public final synchronized CompletableFuture<Boolean> whenSent(
            BiConsumer<Boolean, ? super Throwable> action) {
        if (_sentFuture == null) {
            _sentFuture = new CompletableFuture<>();
        }

        CompletableFuture<Boolean> r = _sentFuture.whenComplete(action);

        //
        // Check if the request has already been sent.
        //
        if (((_state & StateSent) > 0 || _exception != null) && !_sentFuture.isDone()) {
            if (_exception != null) {
                _sentFuture.completeExceptionally(_exception);
            } else {
                _sentFuture.complete(_sentSynchronously);
            }
        }
        return r;
    }

    /**
     * Returns a future that completes when the entire request message has been accepted by the
     * transport and executes the given action using the default executor. The boolean value
     * indicates whether the message was sent synchronously.
     *
     * @param action Executed when the future is completed successfully or exceptionally.
     * @return A future that completes when the message has been handed off to the transport.
     */
    public final synchronized CompletableFuture<Boolean> whenSentAsync(
            BiConsumer<Boolean, ? super Throwable> action) {
        return whenSentAsync(action, null);
    }

    /**
     * Returns a future that completes when the entire request message has been accepted by the
     * transport and executes the given action using the executor. The boolean value indicates
     * whether the message was sent synchronously.
     *
     * @param action Executed when the future is completed successfully or exceptionally.
     * @param executor The executor to use for asynchronous execution.
     * @return A future that completes when the message has been handed off to the transport.
     */
    public final synchronized CompletableFuture<Boolean> whenSentAsync(
            BiConsumer<Boolean, ? super Throwable> action, Executor executor) {
        if (_sentFuture == null) {
            _sentFuture = new CompletableFuture<>();
        }

        CompletableFuture<Boolean> r;
        if (executor == null) {
            r = _sentFuture.whenCompleteAsync(action);
        } else {
            r = _sentFuture.whenCompleteAsync(action, executor);
        }

        //
        // Check if the request has already been sent.
        //
        if (((_state & StateSent) > 0 || _exception != null) && !_sentFuture.isDone()) {
            if (_exception != null) {
                _sentFuture.completeExceptionally(_exception);
            } else {
                _sentFuture.complete(_sentSynchronously);
            }
        }
        return r;
    }

    public final void invokeSent() {
        try {
            synchronized (this) {
                if (_sentFuture != null && !_sentFuture.isDone()) {
                    _sentFuture.complete(_sentSynchronously);
                }
            }

            if (_doneInSent) {
                markCompleted();
            }
        } catch (RuntimeException ex) {
            warning(ex);
        } catch (Error exc) {
            error(exc);
            if (!(exc instanceof AssertionError || exc instanceof OutOfMemoryError)) {
                throw exc;
            }
        }

        if (_observer != null) {
            ObjectPrx proxy = getProxy();
            if (proxy == null || !proxy.ice_isTwoway()) {
                _observer.detach();
                _observer = null;
            }
        }
    }

    protected abstract void markCompleted();

    public final void invokeCompleted() {
        try {
            if (_exception != null) {
                synchronized (this) {
                    if (_sentFuture != null && !_sentFuture.isDone()) {
                        _sentFuture.completeExceptionally(_exception);
                    }
                }
                completeExceptionally(_exception);
            } else {
                markCompleted();
            }
        } catch (RuntimeException ex) {
            warning(ex);
        } catch (AssertionError exc) {
            error(exc);
        } catch (OutOfMemoryError exc) {
            error(exc);
        }

        if (_observer != null) {
            _observer.detach();
            _observer = null;
        }
    }

    public final void invokeCompletedAsync() {
        //
        // CommunicatorDestroyedException is the only exception that can propagate directly from
        // this method.
        //
        _instance
                .clientThreadPool()
                .dispatch(
                        new RunnableThreadPoolWorkItem(_cachedConnection) {
                            @Override
                            public void run() {
                                invokeCompleted();
                            }
                        });
    }

    public synchronized void cancelable(final CancellationHandler handler) {
        if (_cancellationException != null) {
            try {
                throw _cancellationException;
            } finally {
                _cancellationException = null;
            }
        }
        _cancellationHandler = handler;
    }

    protected InvocationFuture(Communicator communicator, Instance instance, String op) {
        _communicator = communicator;
        _instance = instance;
        _operation = op;
        _state = 0;
        _sentSynchronously = false;
        _doneInSent = false;
        _synchronous = false;
        _exception = null;
    }

    protected void cacheMessageBuffers() {}

    protected boolean sent(boolean done) {
        synchronized (this) {
            assert (_exception == null);

            boolean alreadySent = (_state & StateSent) != 0;
            _state |= StateSent;
            if (done) {
                _state |= StateDone | StateOK;
                _cancellationHandler = null;
                _doneInSent = true;

                //
                // For oneway requests after the data has been sent the buffers can be reused unless this is a collocated invocation. For collocated invocations the buffer won't be reused because it has already
                // been marked as cached in invokeCollocated.
                //
                cacheMessageBuffers();
            }

            if (_synchronous && done) {
                if (_observer != null) {
                    _observer.detach();
                    _observer = null;
                }

                markCompleted();
                return false;
            } else {
                this.notifyAll();
                boolean invoke = (!alreadySent && _sentFuture != null || done) && !_synchronous;
                return invoke;
            }
        }
    }

    protected boolean finished(boolean ok, boolean invoke) {
        synchronized (this) {
            _state |= StateDone;
            if (ok) {
                _state |= StateOK;
            }
            _cancellationHandler = null;

            invoke &= !_synchronous;
            if (!invoke && _observer != null) {
                _observer.detach();
                _observer = null;
            }

            if (!invoke) {
                if (_exception != null) {
                    completeExceptionally(_exception);
                } else {
                    markCompleted();
                }
                return false;
            } else {
                this.notifyAll();
                return invoke;
            }
        }
    }

    protected boolean finished(LocalException ex) {
        synchronized (this) {
            _state |= StateDone;
            _exception = ex;
            _cancellationHandler = null;
            if (_observer != null) {
                _observer.failed(ex.ice_id());
            }

            boolean invoke = !_synchronous;
            if (!invoke && _observer != null) {
                _observer.detach();
                _observer = null;
            }

            if (!invoke) {
                if (_exception != null) {
                    completeExceptionally(_exception);
                } else {
                    markCompleted();
                }
                return false;
            } else {
                this.notifyAll();
                return invoke;
            }
        }
    }

    public final void invokeSentAsync() {
        //
        // This is called when it's not safe to call the sent callback synchronously from this thread. Instead the future is completed asynchronously from a client in the client thread pool.
        //
        dispatch(() -> invokeSent());
    }

    protected void cancel(LocalException ex) {
        CancellationHandler handler;
        synchronized (this) {
            if (_cancellationHandler == null) {
                _cancellationException = ex;
                return;
            }
            handler = _cancellationHandler;
        }
        handler.asyncRequestCanceled((OutgoingAsyncBase) this, ex);
    }

    protected com.zeroc.Ice.Instrumentation.InvocationObserver getObserver() {
        return _observer;
    }

    protected void dispatch(final Runnable runnable) {
        try {
            _instance
                    .clientThreadPool()
                    .dispatch(
                            new RunnableThreadPoolWorkItem(_cachedConnection) {
                                @Override
                                public void run() {
                                    runnable.run();
                                }
                            });
        } catch (CommunicatorDestroyedException ex) {
        }
    }

    private void warning(RuntimeException ex) {
        if (_instance.initializationData().properties.getIcePropertyAsInt("Ice.Warn.AMICallback")
                > 0) {
            String s = "exception raised by AMI callback:\n" + Ex.toString(ex);
            _instance.initializationData().logger.warning(s);
        }
    }

    private void error(Error error) {
        String s = "error raised by AMI callback:\n" + Ex.toString(error);
        _instance.initializationData().logger.error(s);
    }

    protected final Instance _instance;
    protected com.zeroc.Ice.Instrumentation.InvocationObserver _observer;
    protected Connection _cachedConnection;
    protected boolean _sentSynchronously;
    protected boolean _doneInSent;
    // True if this AMI request is being used for a generated synchronous invocation.
    protected boolean _synchronous;
    protected CompletableFuture<Boolean> _sentFuture;

    protected final Communicator _communicator;
    protected final String _operation;

    protected LocalException _exception;

    private CancellationHandler _cancellationHandler;
    private LocalException _cancellationException;

    protected static final byte StateOK = 0x1;
    protected static final byte StateDone = 0x2;
    protected static final byte StateSent = 0x4;
    protected static final byte StateCachedBuffers = 0x08;
    protected byte _state;
}
