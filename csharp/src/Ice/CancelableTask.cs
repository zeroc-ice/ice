//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Threading;
using System.Threading.Tasks;

namespace ZeroC.Ice
{
    internal static class CancelableTask
    {
        internal static async ValueTask WhenAny(ValueTask task, CancellationToken cancel)
        {
            if (cancel.CanBeCanceled && !task.IsCompleted)
            {
                await Task.WhenAny(task.AsTask(), Task.Delay(-1, cancel)).ConfigureAwait(false);
                cancel.ThrowIfCancellationRequested();
            }
            await task.ConfigureAwait(false);
        }

        internal static async Task WhenAny(Task task, CancellationToken cancel)
        {
            if (cancel.CanBeCanceled && !task.IsCompleted)
            {
                await Task.WhenAny(task, Task.Delay(-1, cancel)).ConfigureAwait(false);
                cancel.ThrowIfCancellationRequested();
            }
            await task.ConfigureAwait(false);
        }

        internal static async ValueTask<T> WhenAny<T>(ValueTask<T> task, CancellationToken cancel)
        {
            if (cancel.CanBeCanceled && !task.IsCompleted)
            {
                await Task.WhenAny(task.AsTask(), Task.Delay(-1, cancel)).ConfigureAwait(false);
                cancel.ThrowIfCancellationRequested();
            }
            return await task.ConfigureAwait(false);
        }

        internal static async Task<T> WhenAny<T>(Task<T> task, CancellationToken cancel)
        {
            if (cancel.CanBeCanceled && !task.IsCompleted)
            {
                await Task.WhenAny(task, Task.Delay(-1, cancel)).ConfigureAwait(false);
                cancel.ThrowIfCancellationRequested();
            }
            return await task.ConfigureAwait(false);
        }
    }
}
