// Copyright (c) ZeroC, Inc. All rights reserved.

using System;
using System.Diagnostics;
using System.Threading;
using System.Threading.Tasks;
using System.Threading.Tasks.Sources;

namespace ZeroC.Ice
{
    /// <summary>The SignaledTransceiverStream abstract class provides signaling functionality using the
    /// IValueTaskSource interface. It's useful for stream implementations that depend on the transceiver
    /// for receiving data. The transceiver can easily signal the stream when new data is available.</summary>
    internal abstract class SignaledTransceiverStream<T> : TransceiverStream, IValueTaskSource<T>
    {
        internal bool IsSignaled => _source.GetStatus(_source.Version) != ValueTaskSourceStatus.Pending;
        private ManualResetValueTaskSourceCore<T> _source;
        private volatile Exception? _exception;
        private volatile int _signaled;

        /// <summary>Aborts the stream. If the stream is waiting to be signaled and the stream is not signaled
        /// already the stream will be signaled with the exception. If the stream is signaled, we save the
        /// exception, it will be raised after the stream consumes the signal and wait for a new signal</summary>
        public override void Abort(Exception ex)
        {
            // If the source isn't already signaled, signal completion by setting the exception. Otherwise if it's
            // already signaled, it's because a result is pending. In this case, we keep track of the exception and
            // we'll raise the exception the next time the signal is awaited. This is necessary because
            // ManualResetValueTaskSourceCore is not thread safe and once an exception or result is set we can't
            // call again until the source's result or exception is consumed.
            if (Interlocked.CompareExchange(ref _signaled, 1, 0) == 0)
            {
                _source.RunContinuationsAsynchronously = true;
                _source.SetException(ex);
            }
            else
            {
                _exception = ex;
            }
        }

        protected SignaledTransceiverStream(long streamId, MultiStreamTransceiver transceiver)
            : base(streamId, transceiver)
        {
        }

        protected SignaledTransceiverStream(bool bidirectional, MultiStreamTransceiver transceiver)
            : base(bidirectional, transceiver)
        {
        }

        protected bool SignalCompletion(T result, bool runContinuationAsynchronously = false)
        {
            // If the source isn't already signaled, signal completion by setting the result. Otherwise if it's
            // already signaled, it's because the stream got aborted and the exception is set on the source.
            if (Interlocked.CompareExchange(ref _signaled, 1, 0) == 0)
            {
                _source.RunContinuationsAsynchronously = runContinuationAsynchronously;
                _source.SetResult(result);
                return true;
            }
            else
            {
                // The stream is already signaled because it got aborted.
                return false;
            }
        }

        protected ValueTask<T> WaitSignalAsync(CancellationToken cancel = default)
        {
            if (cancel.CanBeCanceled)
            {
                return new ValueTask<T>(this, _source.Version).WaitAsync(cancel);
            }
            else
            {
                return new ValueTask<T>(this, _source.Version);
            }
        }

        T IValueTaskSource<T>.GetResult(short token)
        {
            Debug.Assert(token == _source.Version);
            try
            {
                return _source.GetResult(token);
            }
            finally
            {
                Debug.Assert(_signaled == 1);
                _source.Reset();
                _signaled = 0;
                if (_exception != null)
                {
                    _source.SetException(_exception);
                    _signaled = 1;
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
