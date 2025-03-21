// Copyright (c) ZeroC, Inc.

using System.Diagnostics;

namespace Ice.Internal;

public class RetryTask : TimerTask, CancellationHandler
{
    public RetryTask(Instance instance, RetryQueue retryQueue, ProxyOutgoingAsyncBase outAsync)
    {
        _instance = instance;
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
            if (_instance.traceLevels().retry >= 1)
            {
                _instance.initializationData().logger.trace(
                    _instance.traceLevels().retryCat,
                    $"operation retry canceled\n{ex}");
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

    private readonly Instance _instance;
    private readonly RetryQueue _retryQueue;
    private readonly ProxyOutgoingAsyncBase _outAsync;
}

public class RetryQueue
{
    public RetryQueue(Instance instance)
    {
        _instance = instance;
    }

    public void add(ProxyOutgoingAsyncBase outAsync, int interval)
    {
        lock (_mutex)
        {
            if (_instance == null)
            {
                throw new Ice.CommunicatorDestroyedException();
            }
            RetryTask task = new RetryTask(_instance, this, outAsync);
            outAsync.cancelable(task); // This will throw if the request is canceled.
            _instance.timer().schedule(task, interval);
            _requests.Add(task, null);
        }
    }

    public void destroy()
    {
        lock (_mutex)
        {
            Dictionary<RetryTask, object> keep = new Dictionary<RetryTask, object>();
            foreach (RetryTask task in _requests.Keys)
            {
                if (_instance.timer().cancel(task))
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
            while (_requests.Count > 0)
            {
                System.Threading.Monitor.Wait(_mutex);
            }
        }
    }

    public void remove(RetryTask task)
    {
        lock (_mutex)
        {
            if (_requests.Remove(task))
            {
                if (_instance == null && _requests.Count == 0)
                {
                    // If we are destroying the queue, destroy is probably waiting on the queue to be empty.
                    System.Threading.Monitor.Pulse(_mutex);
                }
            }
        }
    }

    public bool cancel(RetryTask task)
    {
        lock (_mutex)
        {
            if (_requests.Remove(task))
            {
                if (_instance != null)
                {
                    return _instance.timer().cancel(task);
                }
                else if (_requests.Count == 0)
                {
                    // If we are destroying the queue, destroy is probably waiting on the queue to be empty.
                    System.Threading.Monitor.Pulse(_mutex);
                }
            }
            return false;
        }
    }

    private Instance _instance;
    private Dictionary<RetryTask, object> _requests = new();
    private readonly object _mutex = new();
}
