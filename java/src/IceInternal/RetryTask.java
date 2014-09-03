// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceInternal;

class RetryTask implements Runnable
{
    RetryTask(RetryQueue queue, OutgoingAsync outAsync)
    {
        _queue = queue;
        _outAsync = outAsync;
    }

    @Override
    public void
    run()
    {
        if(_queue.remove(this))
        {
            try
            {
                _outAsync.invoke(false);
            }
            catch(Ice.LocalException ex)
            {
                _outAsync.invokeExceptionAsync(ex);
            }
        }
    }

    public void
    destroy()
    {
        _future.cancel(false);
        _outAsync.invokeExceptionAsync(new Ice.CommunicatorDestroyedException());
    }

    public void setFuture(java.util.concurrent.Future<?> future)
    {
        _future = future;
    }

    private final RetryQueue _queue;
    private final OutgoingAsync _outAsync;
    private java.util.concurrent.Future<?> _future;
}
