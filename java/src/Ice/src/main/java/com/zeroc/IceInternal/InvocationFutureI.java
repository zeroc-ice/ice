//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package com.zeroc.IceInternal;

import java.util.concurrent.CompletableFuture;
import java.util.concurrent.Executor;
import java.util.function.BiConsumer;

import com.zeroc.Ice.Communicator;
import com.zeroc.Ice.CommunicatorDestroyedException;
import com.zeroc.Ice.Connection;

public abstract class InvocationFutureI<T> extends com.zeroc.Ice.InvocationFuture<T>
{
    @Override
    public boolean cancel()
    {
        return cancel(false);
    }

    @Override
    public boolean cancel(boolean mayInterruptIfRunning)
    {
        //
        // Call super.cancel(boolean) first. This sets the result of the future.
        // Calling cancel(LocalException) also eventually attempts to complete the future
        // (exceptionally), but this result is ignored.
        //
        boolean r = super.cancel(mayInterruptIfRunning);
        cancel(new com.zeroc.Ice.InvocationCanceledException());
        return r;
    }

    @Override
    public Communicator getCommunicator()
    {
        return _communicator;
    }

    @Override
    public com.zeroc.Ice.Connection getConnection()
    {
        return null;
    }

    @Override
    public com.zeroc.Ice.ObjectPrx getProxy()
    {
        return null;
    }

    @Override
    public final String getOperation()
    {
        return _operation;
    }

    @Override
    public final void waitForCompleted()
    {
        if(Thread.interrupted())
        {
            throw new com.zeroc.Ice.OperationInterruptedException();
        }
        try
        {
            join();
        }
        catch(Exception ex)
        {
        }
    }

    @Override
    public final boolean isSent()
    {
        synchronized(this)
        {
            return (_state & StateSent) > 0;
        }
    }

    @Override
    synchronized public final void waitForSent()
    {
        if(Thread.interrupted())
        {
            throw new com.zeroc.Ice.OperationInterruptedException();
        }
        while((_state & StateSent) == 0 && _exception == null)
        {
            try
            {
                this.wait();
            }
            catch(InterruptedException ex)
            {
                throw new com.zeroc.Ice.OperationInterruptedException();
            }
        }
    }

    @Override
    public final boolean sentSynchronously()
    {
        return _sentSynchronously; // No lock needed, immutable
    }

    @Override
    synchronized public final CompletableFuture<Boolean> whenSent(BiConsumer<Boolean, ? super Throwable> action)
    {
        if(_sentFuture == null)
        {
            _sentFuture = new CompletableFuture<>();
        }

        CompletableFuture<Boolean> r = _sentFuture.whenComplete(action);

        //
        // Check if the request has already been sent.
        //
        if(((_state & StateSent) > 0 || _exception != null) && !_sentFuture.isDone())
        {
            if(_exception != null)
            {
                _sentFuture.completeExceptionally(_exception);
            }
            else
            {
                _sentFuture.complete(_sentSynchronously);
            }
        }
        return r;
    }

    @Override
    synchronized public final CompletableFuture<Boolean> whenSentAsync(BiConsumer<Boolean, ? super Throwable> action)
    {
        return whenSentAsync(action, null);
    }

    @Override
    synchronized public final CompletableFuture<Boolean> whenSentAsync(BiConsumer<Boolean, ? super Throwable> action,
                                                                       Executor executor)
    {
        if(_sentFuture == null)
        {
            _sentFuture = new CompletableFuture<>();
        }

        CompletableFuture<Boolean> r;
        if(executor == null)
        {
            r = _sentFuture.whenCompleteAsync(action);
        }
        else
        {
            r = _sentFuture.whenCompleteAsync(action, executor);
        }

        //
        // Check if the request has already been sent.
        //
        if(((_state & StateSent) > 0 || _exception != null) && !_sentFuture.isDone())
        {
            if(_exception != null)
            {
                _sentFuture.completeExceptionally(_exception);
            }
            else
            {
                _sentFuture.complete(_sentSynchronously);
            }
        }
        return r;
    }

    public final void invokeSent()
    {
        try
        {
            synchronized(this)
            {
                if(_sentFuture != null && !_sentFuture.isDone())
                {
                    _sentFuture.complete(_sentSynchronously);
                }
            }

            if(_doneInSent)
            {
                markCompleted();
            }
        }
        catch(java.lang.RuntimeException ex)
        {
            warning(ex);
        }
        catch(java.lang.Error exc)
        {
            error(exc);
            if(!(exc instanceof java.lang.AssertionError || exc instanceof java.lang.OutOfMemoryError))
            {
                throw exc;
            }
        }

        if(_observer != null)
        {
            com.zeroc.Ice.ObjectPrx proxy = getProxy();
            if(proxy == null || !proxy.ice_isTwoway())
            {
                _observer.detach();
                _observer = null;
            }
        }
    }

    abstract protected void markCompleted();

    public final void invokeCompleted()
    {
        try
        {
            if(_exception != null)
            {
                synchronized(this)
                {
                    if(_sentFuture != null && !_sentFuture.isDone())
                    {
                        _sentFuture.completeExceptionally(_exception);
                    }
                }
                completeExceptionally(_exception);
            }
            else
            {
                markCompleted();
            }
        }
        catch(RuntimeException ex)
        {
            warning(ex);
        }
        catch(AssertionError exc)
        {
            error(exc);
        }
        catch(OutOfMemoryError exc)
        {
            error(exc);
        }

        if(_observer != null)
        {
            _observer.detach();
            _observer = null;
        }
    }

    public final void invokeCompletedAsync()
    {
        //
        // CommunicatorDestroyedException is the only exception that can propagate directly from this method.
        //
        _instance.clientThreadPool().dispatch(
            new DispatchWorkItem(_cachedConnection)
                {
                    @Override
                    public void run()
                    {
                        invokeCompleted();
                    }
                });
    }

    synchronized public void cancelable(final CancellationHandler handler)
    {
        if(_cancellationException != null)
        {
            try
            {
                throw _cancellationException;
            }
            finally
            {
                _cancellationException = null;
            }
        }
        _cancellationHandler = handler;
    }

    protected InvocationFutureI(Communicator communicator, Instance instance, String op)
    {
        _communicator = communicator;
        _instance = instance;
        _operation = op;
        _state = 0;
        _sentSynchronously = false;
        _doneInSent = false;
        _synchronous = false;
        _exception = null;
    }

    protected void cacheMessageBuffers()
    {
    }

    protected boolean sent(boolean done)
    {
        synchronized(this)
        {
            assert(_exception == null);

            boolean alreadySent = (_state & StateSent) != 0;
            _state |= StateSent;
            if(done)
            {
                _state |= StateDone | StateOK;
                _cancellationHandler = null;
                _doneInSent = true;

                //
                // For oneway requests after the data has been sent
                // the buffers can be reused unless this is a
                // collocated invocation. For collocated invocations
                // the buffer won't be reused because it has already
                // been marked as cached in invokeCollocated.
                //
                cacheMessageBuffers();
            }

            boolean invoke = (!alreadySent && _sentFuture != null || done) && !_synchronous;
            if(!invoke && done && _observer != null)
            {
                _observer.detach();
                _observer = null;
            };

            if(!invoke && done)
            {
                markCompleted();
                return false;
            }
            else
            {
                this.notifyAll();
                return invoke;
            }
        }
    }

    protected boolean finished(boolean ok, boolean invoke)
    {
        synchronized(this)
        {
            _state |= StateDone;
            if(ok)
            {
                _state |= StateOK;
            }
            _cancellationHandler = null;

            invoke &= !_synchronous;
            if(!invoke && _observer != null)
            {
                _observer.detach();
                _observer = null;
            }

            if(!invoke)
            {
                if(_exception != null)
                {
                    completeExceptionally(_exception);
                }
                else
                {
                    markCompleted();
                }
                return false;
            }
            else
            {
                this.notifyAll();
                return invoke;
            }
        }
    }

    protected boolean finished(com.zeroc.Ice.Exception ex)
    {
        synchronized(this)
        {
            _state |= StateDone;
            _exception = ex;
            _cancellationHandler = null;
            if(_observer != null)
            {
                _observer.failed(ex.ice_id());
            }

            boolean invoke = !_synchronous;
            if(!invoke && _observer != null)
            {
                _observer.detach();
                _observer = null;
            }

            if(!invoke)
            {
                if(_exception != null)
                {
                    completeExceptionally(_exception);
                }
                else
                {
                    markCompleted();
                }
                return false;
            }
            else
            {
                this.notifyAll();
                return invoke;
            }
        }
    }

    public final void invokeSentAsync()
    {
        //
        // This is called when it's not safe to call the sent callback
        // synchronously from this thread. Instead the future is completed
        // asynchronously from a client in the client thread pool.
        //
        dispatch(() -> invokeSent());
    }

    protected void cancel(com.zeroc.Ice.LocalException ex)
    {
        CancellationHandler handler;
        synchronized(this)
        {
            _cancellationException = ex;
            if(_cancellationHandler == null)
            {
                return;
            }
            handler = _cancellationHandler;
        }
        handler.asyncRequestCanceled((OutgoingAsyncBase)this, ex);
    }

    protected com.zeroc.Ice.Instrumentation.InvocationObserver getObserver()
    {
        return _observer;
    }

    protected void dispatch(final Runnable runnable)
    {
        try
        {
            _instance.clientThreadPool().dispatch(
                new DispatchWorkItem(_cachedConnection)
                {
                    @Override
                    public void run()
                    {
                        runnable.run();
                    }
                });
        }
        catch(CommunicatorDestroyedException ex)
        {
        }
    }

    private void warning(RuntimeException ex)
    {
        if(_instance.initializationData().properties.getPropertyAsIntWithDefault("Ice.Warn.AMICallback", 1) > 0)
        {
            String s = "exception raised by AMI callback:\n" + Ex.toString(ex);
            _instance.initializationData().logger.warning(s);
        }
    }

    private void error(Error error)
    {
        String s = "error raised by AMI callback:\n" + Ex.toString(error);
        _instance.initializationData().logger.error(s);
    }

    protected final Instance _instance;
    protected com.zeroc.Ice.Instrumentation.InvocationObserver _observer;
    protected Connection _cachedConnection;
    protected boolean _sentSynchronously;
    protected boolean _doneInSent;
    protected boolean _synchronous; // True if this AMI request is being used for a generated synchronous invocation.
    protected CompletableFuture<Boolean> _sentFuture;

    protected final Communicator _communicator;
    protected final String _operation;

    protected com.zeroc.Ice.Exception _exception;

    private CancellationHandler _cancellationHandler;
    private com.zeroc.Ice.LocalException _cancellationException;

    protected static final byte StateOK = 0x1;
    protected static final byte StateDone = 0x2;
    protected static final byte StateSent = 0x4;
    protected static final byte StateCachedBuffers = 0x08;
    protected byte _state;
}
