// Copyright (c) ZeroC, Inc. All rights reserved.

using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Threading;
using System.Threading.Tasks;

namespace ZeroC.Ice
{
    /// <summary>A lightweight semaphore implementation that provides FIFO guarantee for WaitAsync. WaitAsync also
    /// relies on ManualResetValueTaskCompletionSource to minimize heap allocations and provide a ValueTask based
    /// WaitAsync operation.</summary>
    internal class AsyncSemaphore
    {
        private int _currentCount;
        private readonly int _maxCount;
        private readonly object _mutex = new object();
        private readonly Queue<ManualResetValueTaskCompletionSource<bool>> _queue = new();

        internal AsyncSemaphore(int initialCount)
        {
            _currentCount = initialCount;
            _maxCount = initialCount;
        }

        internal void CancelAwaiters(Exception exception)
        {
            lock (_mutex)
            {
                // While we could instead use the WaitAsync cancellation token, it's a lot more efficient to trigger
                // the cancellation directly by setting the exception on the task completion source. It also ensures
                // the awaiters will throw the given exception instead of a generic OperationCanceledException.
                foreach (ManualResetValueTaskCompletionSource<bool> source in _queue)
                {
                    source.SetException(exception);
                }
                _queue.Clear();
            }
        }

        internal async ValueTask WaitAsync(CancellationToken cancel = default)
        {
            cancel.ThrowIfCancellationRequested();

            ManualResetValueTaskCompletionSource<bool> taskCompletionSource;
            lock (_mutex)
            {
                if (_currentCount > 0)
                {
                    Debug.Assert(_queue.Count == 0);
                    --_currentCount;
                    return;
                }
                // Don't auto reset the task completion source after obtaining the result. This is necessary to
                // ensure that the exception won't be cleared if the task is canceled.
                taskCompletionSource = new ManualResetValueTaskCompletionSource<bool>(autoReset: false);
                taskCompletionSource.RunContinuationAsynchronously = true;
                _queue.Enqueue(taskCompletionSource);
            }

            try
            {
                await taskCompletionSource.ValueTask.WaitAsync(cancel).ConfigureAwait(false);
            }
            catch (Exception ex)
            {
                lock (_mutex)
                {
                    if (_queue.Contains(taskCompletionSource))
                    {
                        taskCompletionSource.SetException(ex);
                    }
                }
                throw;
            }
        }

        internal void Release()
        {
            lock (_mutex)
            {
                if (_currentCount == _maxCount)
                {
                    throw new SemaphoreFullException($"semaphore maximum count of {_maxCount} already reached");
                }

                while (_queue.Count > 0)
                {
                    try
                    {
                        _queue.Dequeue().SetResult(true);
                        return;
                    }
                    catch
                    {
                        // Ignore, this can occur if WaitAsync is canceled.
                    }
                }

                // Increment the semaphore if there's no waiter.
                ++_currentCount;
            }
        }
    }
}
