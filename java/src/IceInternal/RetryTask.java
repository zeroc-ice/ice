// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceInternal;

class RetryTask implements Runnable, CancellationHandler
{
    RetryTask(RetryQueue queue, ProxyOutgoingAsyncBase outAsync)
    {
        _queue = queue;
        _outAsync = outAsync;
    }

    @Override
    public void run()
    {
        _outAsync.retry();
        
        //
        // NOTE: this must be called last, destroy() blocks until all task
        // are removed to prevent the client thread pool to be destroyed
        // (we still need the client thread pool at this point to call
        // exception callbacks with CommunicatorDestroyedException).
        //
        _queue.remove(this);
    }

    @Override
    public void asyncRequestCanceled(OutgoingAsyncBase outAsync, Ice.LocalException ex)
    {
        if(_queue.remove(this) && _future.cancel(false))
        {
            //
            // We just retry the outgoing async now rather than marking it
            // as finished. The retry will check for the cancellation
            // exception and terminate appropriately the request.
            //
            _outAsync.retry();
        }
    }

    public boolean destroy()
    {
        if(_future.cancel(false))
        {
            try
            {
                _outAsync.abort(new Ice.CommunicatorDestroyedException());
            }
            catch(Ice.CommunicatorDestroyedException ex)
            {
                // Abort can throw if there's no callback, just ignore in this case
            }
            return true;
        }
        return false;
    }

    public void setFuture(java.util.concurrent.Future<?> future)
    {
        _future = future;
    }

    private final RetryQueue _queue;
    private final ProxyOutgoingAsyncBase _outAsync;
    private java.util.concurrent.Future<?> _future;
}
