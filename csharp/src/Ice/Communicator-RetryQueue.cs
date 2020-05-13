//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using IceInternal;
using System.Collections.Generic;
using System.Diagnostics;
using System.Threading.Tasks;

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
            _outAsync.Retry();

            //
            // NOTE: this must be called last, destroy() blocks until all task
            // are removed to prevent the client thread pool to be destroyed
            // (we still need the client thread pool at this point to call
            // exception callbacks with CommunicatorDestroyedException).
            //
            _communicator.RemoveRetryTask(this);
        }

        public void AsyncRequestCanceled(OutgoingAsyncBase outAsync, System.Exception ex)
        {
            Debug.Assert(_outAsync == outAsync);
            if (_communicator.CancelRetryTask(this))
            {
                if (_communicator.TraceLevels.Retry >= 1)
                {
                    _communicator.Logger.Trace(_communicator.TraceLevels.RetryCat, $"operation retry canceled\n{ex}");
                }
                if (_outAsync.Exception(ex))
                {
                    Task.Run(_outAsync.InvokeException);
                }
            }
        }

        public void Destroy()
        {
            try
            {
                _outAsync.Abort(new CommunicatorDestroyedException());
            }
            catch (CommunicatorDestroyedException)
            {
                // Abort can throw if there's no callback, just ignore in this case
            }
        }

        private readonly Communicator _communicator;
        private readonly ProxyOutgoingAsyncBase _outAsync;
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
                var task = new RetryTask(this, outAsync);
                outAsync.Cancelable(task); // This will throw if the request is canceled.
                _timer.Schedule(task, interval);
                _requests.Add(task, null);
            }
        }

        internal void DestroyRetryQueue()
        {
            lock (this)
            {
                Debug.Assert(_state == StateDestroyInProgress);
                var keep = new Dictionary<RetryTask, object?>();
                foreach (RetryTask task in _requests.Keys)
                {
                    if (_timer.Cancel(task))
                    {
                        task.Destroy();
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
                    if (_state > StateActive && _requests.Count == 0)
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
                    if (_state < StateDestroyInProgress)
                    {
                        return _timer.Cancel(task);
                    }
                    else if (_requests.Count == 0)
                    {
                        // If we are destroying the queue, destroy is probably waiting on the queue to be empty.
                        System.Threading.Monitor.Pulse(this);
                    }
                }
                return false;
            }
        }

        private Dictionary<RetryTask, object?> _requests = new Dictionary<RetryTask, object?>();
    }
}
