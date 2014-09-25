// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceInternal;

public class RetryQueue
{
    RetryQueue(Instance instance)
    {
        _instance = instance;
    }

    synchronized public void
    add(OutgoingAsyncMessageCallback outAsync, int interval)
    {
        if(_instance == null)
        {
            throw new Ice.CommunicatorDestroyedException();
        }
        RetryTask task = new RetryTask(this, outAsync);
        task.setFuture(_instance.timer().schedule(task, interval, java.util.concurrent.TimeUnit.MILLISECONDS));
        _requests.add(task);
    }

    synchronized public void
    destroy()
    {
        _instance = null;
        for(RetryTask task : _requests)
        {
            task.destroy();
        }
        _requests.clear();
    }

    synchronized boolean
    remove(RetryTask task)
    {
        return _requests.remove(task);
    }

    private Instance _instance;
    final private java.util.HashSet<RetryTask> _requests = new java.util.HashSet<RetryTask>();
}
