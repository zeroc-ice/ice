// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceInternal;

class RetryTask implements TimerTask
{
    RetryTask(RetryQueue queue, OutgoingAsync outAsync)
    {
        _queue = queue;
        _outAsync = outAsync;
    }

    public void
    runTimerTask()
    {
        if(_queue.remove(this))
        {
            try
            {
                _outAsync.__send(false);
            }
            catch(Ice.LocalException ex)
            {
                _outAsync.__exceptionAsync(ex);
            }
        }
    }

    public void
    destroy()
    {
        _outAsync.__exceptionAsync(new Ice.CommunicatorDestroyedException());
    }

    private final RetryQueue _queue;
    private final OutgoingAsync _outAsync;
}
