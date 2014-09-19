// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceInternal;

public class BatchOutgoingAsync extends OutgoingAsyncBase implements OutgoingAsyncMessageCallback
{
    BatchOutgoingAsync(Ice.Communicator communicator, Instance instance, String operation, CallbackBase callback)
    {
        super(communicator, instance, operation, callback);
    }

    @Override
    public int
    send(Ice.ConnectionI connection, boolean compress, boolean response)
    {
        _cachedConnection = connection;
        return connection.flushAsyncBatchRequests(this);
    }

    @Override
    public int
    invokeCollocated(CollocatedRequestHandler handler)
    {
        return handler.invokeAsyncBatchRequests(this);
    }

    @Override
    public boolean
    sent()
    {
        synchronized(_monitor)
        {
            _state |= StateDone | StateOK | StateSent;
            //_os.resize(0, false); // Don't clear the buffer now, it's needed for the collocation optimization
            if(_childObserver != null)
            {
                _childObserver.detach();
                _childObserver = null;
            }
            if(_timeoutRequestHandler != null)
            {
                _future.cancel(false);
                _future = null;
                _timeoutRequestHandler = null;
            }
            _monitor.notifyAll();

            if(_callback == null || !_callback.__hasSentCallback())
            {
                if(_observer != null)
                {
                    _observer.detach();
                    _observer = null;
                }
                return false;
            }
            return true;
        }
    }

    @Override
    public void
    invokeSent()
    {
        invokeSentInternal();
    }

    @Override
    public void
    finished(Ice.Exception exc)
    {
        synchronized(_monitor)
        {
            if(_childObserver != null)
            {
                _childObserver.failed(exc.ice_name());
                _childObserver.detach();
                _childObserver = null;
            }
            if(_timeoutRequestHandler != null)
            {
                _future.cancel(false);
                _future = null;
                _timeoutRequestHandler = null;
            }
        }
        invokeException(exc);
    }

    @Override
    public void
    processRetry(boolean destroyed)
    {
        // Does not implement retry
    }

    @Override
    public void
    dispatchInvocationCancel(final Ice.LocalException ex, ThreadPool threadPool, Ice.Connection connection)
    {
        threadPool.dispatch(new DispatchWorkItem(connection)
        {
            @Override
            public void run()
            {
                BatchOutgoingAsync.this.finished(ex);
            }
        });
    }
}
