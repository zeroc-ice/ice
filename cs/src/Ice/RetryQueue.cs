// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

namespace IceInternal
{
    using System.Collections.Generic;

    public class RetryTask : TimerTask
    {
        public RetryTask(RetryQueue retryQueue, OutgoingAsync outAsync)
        {
            _retryQueue = retryQueue;
            _outAsync = outAsync;
        }

        public void runTimerTask()
        {
            if(_retryQueue.remove(this))
            {
                try
                {
                    _outAsync.send__(false);
                }
                catch(Ice.LocalException ex)
                {
                    _outAsync.exceptionAsync__(ex);
                }
            }
        }

        public void destroy()
        {
            _outAsync.exceptionAsync__(new Ice.CommunicatorDestroyedException());
        }

        private RetryQueue _retryQueue;
        private OutgoingAsync _outAsync;
    }

    public class RetryQueue
    {
        public RetryQueue(Instance instance)
        {
            _instance = instance;
        }

        public void add(OutgoingAsync outAsync, int interval)
        {
            lock(this)
            {
                RetryTask task = new RetryTask(this, outAsync);
                _instance.timer().schedule(task, interval);
                _requests.Add(task, null);
            }
        }

        public void
        destroy()
        {
            lock(this)
            {
                foreach(RetryTask task in _requests.Keys)
                {
                    _instance.timer().cancel(task);
                    task.destroy();
                }
                _requests.Clear();
            }
        }

        public bool
        remove(RetryTask task)
        {
            lock(this)
            {
                return _requests.Remove(task);
            }
        }

        private Instance _instance;
        private Dictionary<RetryTask, object> _requests = new Dictionary<RetryTask, object>();
    }
}
