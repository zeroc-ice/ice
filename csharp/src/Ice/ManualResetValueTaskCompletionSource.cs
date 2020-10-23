// Copyright (c) ZeroC, Inc. All rights reserved.

using System;
using System.Threading.Tasks;
using System.Threading.Tasks.Sources;

namespace ZeroC.Ice
{
    /// <summary>A manual reset task completion source for ValueTask. It provides the same functionality as the
    /// TaskCompletionSource class but with ValueTask support instead. It's useful for hot code paths that
    /// require to minimize heap allocations required by the Task class. This class is NOT thread safe.</summary>
    internal class ManualResetValueTaskCompletionSource<T> : IValueTaskSource<T>
    {
        internal bool IsCompleted => _source.GetStatus(_source.Version) != ValueTaskSourceStatus.Pending;
        internal bool RunContinuationAsynchronously
        {
            get => _source.RunContinuationsAsynchronously;
            set => _source.RunContinuationsAsynchronously = value;
        }
        internal ValueTask<T> ValueTask => new ValueTask<T>(this, _source.Version);
        private ManualResetValueTaskSourceCore<T> _source;
        private readonly bool _autoReset;

        /// <summary>Initializes a new instance of ManualResetValueTaskCompletionSource with a boolean indicating
        /// if the source should be reset after the result is obtained. If the auto reset is disabled, the Reset
        /// method needs to be called explicitly before re-using the source.</summary>
        /// <param name="autoReset">The source is reset automatically after the result is retrieve.</param>
        internal ManualResetValueTaskCompletionSource(bool autoReset = true) => _autoReset = autoReset;

        internal void Reset() => _source.Reset();

        internal void SetException(Exception exception) => _source.SetException(exception);

        internal void SetResult(T value) => _source.SetResult(value);

        T IValueTaskSource<T>.GetResult(short token)
        {
            bool isValid = token == _source.Version;
            try
            {
                return _source.GetResult(token);
            }
            finally
            {
                if (isValid && _autoReset)
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
