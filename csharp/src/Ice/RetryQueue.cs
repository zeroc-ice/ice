//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Collections.Generic;
using System.Diagnostics;
using IceInternal;

namespace Ice
{
    public class RetryTask : ITimerTask, ICancellationHandler
    {
        public RetryTask(Communicator communicator, ProxyOutgoingAsyncBase outAsync)
        {
            _communicator = communicator;
            _outAsync = outAsync;
        }

        public void RunTimerTask()
        {
            _outAsync.retry();

            //
            // NOTE: this must be called last, destroy() blocks until all task
            // are removed to prevent the client thread pool to be destroyed
            // (we still need the client thread pool at this point to call
            // exception callbacks with CommunicatorDestroyedException).
            //
            _communicator.RemoveRetryTask(this);
        }

        public void AsyncRequestCanceled(OutgoingAsyncBase outAsync, Ice.LocalException ex)
        {
            Debug.Assert(_outAsync == outAsync);
            if (_communicator.CancelRetryTask(this))
            {
                if (_communicator.TraceLevels.retry >= 1)
                {
                    _communicator.Logger.trace(_communicator.TraceLevels.retryCat, $"operation retry canceled\n{ex}");
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
                _outAsync.abort(new CommunicatorDestroyedException());
            }
            catch (CommunicatorDestroyedException)
            {
                // Abort can throw if there's no callback, just ignore in this case
            }
        }

        private Communicator _communicator;
        private ProxyOutgoingAsyncBase _outAsync;
    }

    public sealed partial class Communicator
    {
        internal void AddRetryTask(ProxyOutgoingAsyncBase outAsync, int interval)
        {
            lock (this)
            {
                if (_state == StateDestroyed)
                {
                    throw new CommunicatorDestroyedException();
                }
                RetryTask task = new RetryTask(this, outAsync);
                outAsync.cancelable(task); // This will throw if the request is canceled.
                _timer.schedule(task, interval);
                _requests.Add(task, null);
            }
        }

        internal void DestroyRetryTask()
        {
            lock (this)
            {
                Dictionary<RetryTask, object?> keep = new Dictionary<RetryTask, object?>();
                foreach (RetryTask task in _requests.Keys)
                {
                    if (_timer.cancel(task))
                    {
                        task.destroy();
                    }
                    else
                    {
                        keep.Add(task, null);
                    }
                }
                _requests = keep;
                while (_requests.Count > 0)
                {
                    System.Threading.Monitor.Wait(this);
                }
            }
        }

        internal void RemoveRetryTask(RetryTask task)
        {
            lock (this)
            {
                if (_requests.Remove(task))
                {
                    if (_state == StateDestroyed && _requests.Count == 0)
                    {
                        // If we are destroying the queue, destroy is probably waiting on the queue to be empty.
                        System.Threading.Monitor.Pulse(this);
                    }
                }
            }
        }

        internal bool CancelRetryTask(RetryTask task)
        {
            lock (this)
            {
                if (_requests.Remove(task))
                {
                    if (_state == StateDestroyed && _requests.Count == 0)
                    {
                        // If we are destroying the queue, destroy is probably waiting on the queue to be empty.
                        System.Threading.Monitor.Pulse(this);
                    }
                    return _timer.cancel(task);
                }
                return false;
            }
        }

        private Dictionary<RetryTask, object?> _requests = new Dictionary<RetryTask, object?>();
    }
}
