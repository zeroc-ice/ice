// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
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
    add(OutgoingAsync outAsync, int interval)
    {
        RetryTask task = new RetryTask(this, outAsync);
        _instance.timer().schedule(task, interval);
        _requests.add(task);
    }

    synchronized public void
    destroy()
    {
        for(RetryTask task : _requests)
        {
            _instance.timer().cancel(task);
            task.destroy();
        }
        _requests.clear();
    }

    synchronized boolean
    remove(RetryTask task)
    {
        return _requests.remove(task);
    }

    final private Instance _instance;
    final private java.util.HashSet<RetryTask> _requests = new java.util.HashSet<RetryTask>();
}
