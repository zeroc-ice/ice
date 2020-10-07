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
    /// for receiving data: the transceiver can easily signal the stream when new data is available.</summary>
    internal abstract class SignaledTransceiverStream<T> : TransceiverStream, IValueTaskSource<T>
    {
        internal bool IsSignaled => _source.GetStatus(_source.Version) != ValueTaskSourceStatus.Pending;
        private ManualResetValueTaskSourceCore<T> _source;

        public override void Abort(Exception ex)
        {
            try
            {
                _source.RunContinuationsAsynchronously = true;
                _source.SetException(ex);
            }
            catch
            {
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

        protected void SignalCompletion(T result, bool runContinuationAsynchronously = false)
        {
            _source.RunContinuationsAsynchronously = runContinuationAsynchronously;
            _source.SetResult(result);
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
                _source.Reset();
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
