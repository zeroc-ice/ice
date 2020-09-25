//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Threading.Tasks;
using System.Threading.Tasks.Sources;

namespace ZeroC.Ice
{
    /// <summary>A manual reset task completion source for ValueTask. It provides the same functionality as the
    /// TaskCompletionSource class but with ValueTask support instead. It's useful for hot code paths that
    /// require to minimize heap allocations required by the Task class.</summary>
    internal class ManualResetValueTaskCompletionSource<T> : IValueTaskSource<T>
    {
        internal bool IsCompleted => _source.GetStatus(_source.Version) != ValueTaskSourceStatus.Pending;

        internal ValueTask<T> ValueTask => new ValueTask<T>(this, _source.Version);

        internal void SetResult(T value) => _source.SetResult(value);

        internal void SetException(Exception exception) => _source.SetException(exception);

        internal bool TrySetException(Exception exception)
        {
            try
            {
                _source.SetException(exception);
            }
            catch
            {
                return false;
            }
            return true;
        }

        private ManualResetValueTaskSourceCore<T> _source;

        T IValueTaskSource<T>.GetResult(short token)
        {
            bool isValid = token == _source.Version;
            try
            {
                return _source.GetResult(token);
            }
            finally
            {
                if (isValid)
                {
                    _source.Reset();
                }
            }
        }

        ValueTaskSourceStatus IValueTaskSource<T>.GetStatus(short token) => _source.GetStatus(token);

        void IValueTaskSource<T>.OnCompleted(
            Action<object?> continuation,
            object? state,
            short token,
            ValueTaskSourceOnCompletedFlags flags) => _source.OnCompleted(continuation, state, token, flags);
    }
}
