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
    using System.Diagnostics;

    public class RetryTask : TimerTask
    {
        public RetryTask(RetryQueue retryQueue, OutgoingAsyncMessageCallback outAsync)
        {
            _retryQueue = retryQueue;
            _outAsync = outAsync;
        }

        public void runTimerTask()
        {
            _outAsync.processRetry(false);

            //
            // NOTE: this must be called last, destroy() blocks until all task
            // are removed to prevent the client thread pool to be destroyed
            // (we still need the client thread pool at this point to call
            // exception callbacks with CommunicatorDestroyedException).
            //
            _retryQueue.remove(this);
        }

        public void destroy()
        {
            _outAsync.processRetry(true);
        }

        private RetryQueue _retryQueue;
        private OutgoingAsyncMessageCallback _outAsync;
    }

    public class RetryQueue
    {
        public RetryQueue(Instance instance)
        {
            _instance = instance;
        }

        public void add(OutgoingAsyncMessageCallback outAsync, int interval)
        {
            lock(this)
            {
                if(_instance == null)
                {
                    throw new Ice.CommunicatorDestroyedException();
                }
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
                Dictionary<RetryTask, object> keep = new Dictionary<RetryTask, object>();
                foreach(RetryTask task in _requests.Keys)
                {
                    if(_instance.timer().cancel(task))
                    {
                        task.destroy();
                    }
                    else
                    {
                        keep.Add(task, null);
                    }
                }
                _requests = keep;
                _instance = null;
                while(_requests.Count > 0)
                {
                    System.Threading.Monitor.Wait(this);
                }
            }
        }

        public void
        remove(RetryTask task)
        {
            lock(this)
            {
                Debug.Assert(_requests.ContainsKey(task));
                _requests.Remove(task);
                if(_instance == null && _requests.Count == 0)
                {
                    // If we are destroying the queue, destroy is probably waiting on the queue to be empty.
                    System.Threading.Monitor.Pulse(this);
                }
            }
        }

        private Instance _instance;
        private Dictionary<RetryTask, object> _requests = new Dictionary<RetryTask, object>();
    }
}
