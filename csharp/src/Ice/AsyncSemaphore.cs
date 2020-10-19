// Copyright (c) ZeroC, Inc. All rights reserved.

using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Threading;
using System.Threading.Tasks;

namespace ZeroC.Ice
{
    /// <summary>A lightweight semaphore implementation that provides FIFO guarantee for WaitAsync. WaitAsync also
    /// relies on ManualResetValueTaskCompletionSource to minimize heap allocations.</summary>
    internal class AsyncSemaphore
    {
        private int _currentCount;
        private readonly int _maxCount;
        private readonly object _mutex = new object();
        private readonly Queue<ManualResetValueTaskCompletionSource<bool>> _queue = new ();

        internal AsyncSemaphore(int initialCount)
        {
            _currentCount = initialCount;
            _maxCount = initialCount;
        }

        internal async ValueTask WaitAsync(CancellationToken cancel = default)
        {
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
                _queue.Enqueue(taskCompletionSource);
            }

            try
            {
                await taskCompletionSource.ValueTask.WaitAsync(cancel).ConfigureAwait(false);
            }
            catch (Exception ex)
            {
                taskCompletionSource.TrySetException(ex);
                throw;
            }
        }

        internal void Release()
        {
            ManualResetValueTaskCompletionSource<bool> taskCompletionSource;
            do
            {
                lock (_mutex)
                {
                    if (_queue.Count == 0)
                    {
                        if (_currentCount == _maxCount)
                        {
                            throw new SemaphoreFullException($"semaphore maximum count of {_maxCount} already reached");
                        }
                        ++_currentCount;
                        return;
                    }
                    taskCompletionSource = _queue.Dequeue();
                }
            }
            while (!taskCompletionSource.TrySetResult(true));
        }
    }
}
