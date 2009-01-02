// **********************************************************************
//
// Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
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
                _outAsync.__send();
            }
            catch(Ice.LocalException ex)
            {
                _outAsync.__releaseCallback(ex);
            }
        }
    }

    public void
    destroy()
    {
        _outAsync.__releaseCallback(new Ice.CommunicatorDestroyedException());
    }

    private final RetryQueue _queue;
    private final OutgoingAsync _outAsync;
}
