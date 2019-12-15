//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

namespace IceInternal
{
    using System.Collections.Generic;
    using System.Diagnostics;

    public class RetryTask : TimerTask, CancellationHandler
    {
        public RetryTask(Ice.Communicator communicator, RetryQueue retryQueue, ProxyOutgoingAsyncBase outAsync)
        {
            _communicator = communicator;
            _retryQueue = retryQueue;
            _outAsync = outAsync;
        }

        public void runTimerTask()
        {
            _outAsync.retry();

            //
            // NOTE: this must be called last, destroy() blocks until all task
            // are removed to prevent the client thread pool to be destroyed
            // (we still need the client thread pool at this point to call
            // exception callbacks with CommunicatorDestroyedException).
            //
            _retryQueue.remove(this);
        }

        public void asyncRequestCanceled(OutgoingAsyncBase outAsync, Ice.LocalException ex)
        {
            Debug.Assert(_outAsync == outAsync);
            if (_retryQueue.cancel(this))
            {
                if (_communicator.traceLevels().retry >= 1)
                {
                    _communicator.Logger.trace(_communicator.traceLevels().retryCat,
                                                                string.Format("operation retry canceled\n{0}", ex));
                }
                if (_outAsync.exception(ex))
                {
                    _outAsync.invokeExceptionAsync();
                }
            }
        }

        public void destroy()
        {
            try
            {
                _outAsync.abort(new Ice.CommunicatorDestroyedException());
            }
            catch (Ice.CommunicatorDestroyedException)
            {
                // Abort can throw if there's no callback, just ignore in this case
            }
        }

        private Ice.Communicator _communicator;
        private RetryQueue _retryQueue;
        private ProxyOutgoingAsyncBase _outAsync;
    }

    public class RetryQueue
    {
        public RetryQueue(Ice.Communicator communicator)
        {
            _communicator = communicator;
        }

        public void add(ProxyOutgoingAsyncBase outAsync, int interval)
        {
            lock (this)
            {
                if (_communicator == null)
                {
                    throw new Ice.CommunicatorDestroyedException();
                }
                RetryTask task = new RetryTask(_communicator, this, outAsync);
                outAsync.cancelable(task); // This will throw if the request is canceled.
                _communicator.timer().schedule(task, interval);
                _requests.Add(task, null);
            }
        }

        public void destroy()
        {
            lock (this)
            {
                Dictionary<RetryTask, object> keep = new Dictionary<RetryTask, object>();
                foreach (RetryTask task in _requests.Keys)
                {
                    if (_communicator.timer().cancel(task))
                    {
                        task.destroy();
                    }
                    else
                    {
                        keep.Add(task, null);
                    }
                }
                _requests = keep;
                _communicator = null;
                while (_requests.Count > 0)
                {
                    System.Threading.Monitor.Wait(this);
                }
            }
        }

        public void remove(RetryTask task)
        {
            lock (this)
            {
                if (_requests.Remove(task))
                {
                    if (_communicator == null && _requests.Count == 0)
                    {
                        // If we are destroying the queue, destroy is probably waiting on the queue to be empty.
                        System.Threading.Monitor.Pulse(this);
                    }
                }
            }
        }

        public bool cancel(RetryTask task)
        {
            lock (this)
            {
                if (_requests.Remove(task))
                {
                    if (_communicator == null && _requests.Count == 0)
                    {
                        // If we are destroying the queue, destroy is probably waiting on the queue to be empty.
                        System.Threading.Monitor.Pulse(this);
                    }
                    return _communicator.timer().cancel(task);
                }
                return false;
            }
        }

        private Ice.Communicator _communicator;
        private Dictionary<RetryTask, object> _requests = new Dictionary<RetryTask, object>();
    }
}
