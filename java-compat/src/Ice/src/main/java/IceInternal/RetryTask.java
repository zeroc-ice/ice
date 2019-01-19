//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package IceInternal;

class RetryTask implements Runnable, CancellationHandler
{
    RetryTask(Instance instance, RetryQueue queue, ProxyOutgoingAsyncBase outAsync)
    {
        _instance = instance;
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
            if(_instance.traceLevels().retry >= 1)
            {
                StringBuilder s = new StringBuilder(128);
                s.append("operation retry canceled\n");
                s.append(Ex.toString(ex));
                _instance.initializationData().logger.trace(_instance.traceLevels().retryCat, s.toString());
            }
            if(_outAsync.completed(ex))
            {
                _outAsync.invokeCompletedAsync();
            }
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

    private final Instance _instance;
    private final RetryQueue _queue;
    private final ProxyOutgoingAsyncBase _outAsync;
    private java.util.concurrent.Future<?> _future;
}
