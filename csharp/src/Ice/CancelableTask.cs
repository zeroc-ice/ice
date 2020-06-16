//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Threading;
using System.Threading.Tasks;

namespace ZeroC.Ice
{
    /// <summary>CancelableTask provides WhenAny static methods which creates a task that is completed either when the
    /// given task completes or when the given cancellation token is canceled. This is useful when canceling the given
    /// task isn't an option. For example, the user might want to cancel an invocation that is waiting for connection
    /// establishment. Instead of canceling the connection establishment which might be shared by other invocations we
    /// just cancel the wait on the connection esablishement for the invocation. The same applies for invocations which
    /// are waiting on a connection to be sent.</summary>
    internal static class CancelableTask
    {
        internal static async ValueTask WhenAny(ValueTask task, CancellationToken cancel)
        {
            // Optimization: if the given task is already completed or the cancellation token is not cancelable,
            // not need to wait for these two.
            if (cancel.CanBeCanceled && !task.IsCompleted)
            {
                await Task.WhenAny(task.AsTask(), Task.Delay(-1, cancel)).ConfigureAwait(false);
                cancel.ThrowIfCancellationRequested();
            }
            await task.ConfigureAwait(false);
        }

        internal static async Task WhenAny(Task task, CancellationToken cancel)
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

        internal static async ValueTask<T> WhenAny<T>(ValueTask<T> task, CancellationToken cancel)
        {
            // Optimization: if the given task is already completed or the cancellation token is not cancelable,
            // not need to wait for these two.
            if (cancel.CanBeCanceled && !task.IsCompleted)
            {
                await Task.WhenAny(task.AsTask(), Task.Delay(-1, cancel)).ConfigureAwait(false);
                cancel.ThrowIfCancellationRequested();
            }
            return await task.ConfigureAwait(false);
        }

        internal static async Task<T> WhenAny<T>(Task<T> task, CancellationToken cancel)
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
