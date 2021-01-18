// Copyright (c) ZeroC, Inc. All rights reserved.

using System.Threading;
using System.Threading.Tasks;

namespace ZeroC.Ice
{
    /// <summary>WaitAsync task extensions allow to cancel the wait for the task completion without canceling the
    /// task. For example, the user might want to cancel an invocation that is waiting for connection establishment.
    /// Instead of canceling the connection establishment which might be shared by other invocations we cancel the wait
    /// on the connection establishment for the invocation. The same applies for invocations which are waiting on a
    /// connection to be sent.</summary>
    public static class TaskExtensions
    {
        /// <summary>Waits for the task to complete and allows the wait to be canceled.</summary>
        /// <param name="task">The task to wait for.</param>
        /// <param name="cancel">The cancellation token.</param>
        public static async Task WaitAsync(this Task task, CancellationToken cancel)
        {
            // Optimization: if the given task is already completed or the cancellation token is not cancelable,
            // not need to wait for these two.
            if (cancel.CanBeCanceled && !task.IsCompleted)
            {
                await Task.WhenAny(task, Task.Delay(-1, cancel)).ConfigureAwait(false);
                cancel.ThrowIfCancellationRequested();
            }
            await task.ConfigureAwait(false);
        }

        /// <summary>Waits for the task to complete and allows the wait to be canceled.</summary>
        /// <param name="task">The task to wait for.</param>
        /// <param name="cancel">The cancellation token.</param>
        internal static async ValueTask<T> WaitAsync<T>(this ValueTask<T> task, CancellationToken cancel)
        {
            // Optimization: if the given task is already completed or the cancellation token is not cancelable,
            // not need to wait for these two.
            if (cancel.CanBeCanceled && !task.IsCompleted)
            {
                Task<T> asTask = task.AsTask();
                await Task.WhenAny(asTask, Task.Delay(-1, cancel)).ConfigureAwait(false);
                cancel.ThrowIfCancellationRequested();
                return await asTask.ConfigureAwait(false);
            }
            else
            {
                return await task.ConfigureAwait(false);
            }
        }

        /// <summary>Waits for the task to complete and allows the wait to be canceled.</summary>
        /// <param name="task">The task to wait for.</param>
        /// <param name="cancel">The cancellation token.</param>
        internal static async Task<T> WaitAsync<T>(this Task<T> task, CancellationToken cancel)
        {
            // Optimization: if the given task is already completed or the cancellation token is not cancelable,
            // not need to wait for these two.
            if (cancel.CanBeCanceled && !task.IsCompleted)
            {
                await Task.WhenAny(task, Task.Delay(-1, cancel)).ConfigureAwait(false);
                cancel.ThrowIfCancellationRequested();
            }
            return await task.ConfigureAwait(false);
        }
    }
}
