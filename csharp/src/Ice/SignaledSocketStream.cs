// Copyright (c) ZeroC, Inc. All rights reserved.

using System;
using System.Diagnostics;
using System.Threading;
using System.Threading.Tasks;
using System.Threading.Tasks.Sources;

namespace ZeroC.Ice
{
    /// <summary>The SignaledSocketStream abstract class provides signaling functionality using the
    /// IValueTaskSource interface. It's useful for stream implementations that depend on the socket
    /// for receiving data. The socket can easily signal the stream when new data is available.</summary>
    internal abstract class SignaledSocketStream<T> : SocketStream, IValueTaskSource<T>
    {
        internal bool IsSignaled => Thread.VolatileRead(ref _signaled) == 1;
        private volatile Exception? _exception;
        private int _signaled;
        private ManualResetValueTaskSourceCore<T> _source;
        private CancellationTokenRegistration _tokenRegistration;
        private static readonly Exception _disposedException =
            new ObjectDisposedException(nameof(SignaledSocketStream<T>));

        /// <summary>Aborts the stream. If the stream is waiting to be signaled and the stream is not signaled
        /// already, the stream will be signaled with the exception. If the stream is signaled, we save the
        /// exception to raise it after the stream consumes the signal and waits for a new signal</summary>
        public override void Abort(Exception ex)
        {
            if (_exception == null)
            {
                _exception = ex;

                // If the source isn't already signaled, signal completion by setting the exception. Otherwise if it's
                // already signaled, a result is pending. In this case, we keep track of the exception and we'll raise
                // the exception the next time the signal is awaited. This is necessary because
                // ManualResetValueTaskSourceCore is not thread safe and once an exception or result is set we can't
                // call again SetXxx until the source's result or exception is consumed.
                if (Interlocked.CompareExchange(ref _signaled, 1, 0) == 0)
                {
                    _source.RunContinuationsAsynchronously = true;
                    _source.SetException(ex);
                }
            }
        }

        protected SignaledSocketStream(MultiStreamSocket socket, long streamId)
            : base(socket, streamId)
        {
        }

        protected SignaledSocketStream(MultiStreamSocket socket, bool bidirectional, bool control)
            : base(socket, bidirectional, control)
        {
        }

        protected override void Dispose(bool disposing)
        {
            base.Dispose(disposing);
            if (disposing)
            {
                // Ensure the stream signaling fails after disposal
                Abort(_disposedException);

                // Unregister the cancellation token callback
                _tokenRegistration.Dispose();
            }
        }

        protected void SignalCompletion(T result, bool runContinuationAsynchronously = false)
        {
            if (Interlocked.CompareExchange(ref _signaled, 1, 0) == 0)
            {
                // If the source isn't already signaled, signal completion by setting the result.
                _source.RunContinuationsAsynchronously = runContinuationAsynchronously;
                _source.SetResult(result);
            }
            else
            {
                // The stream is already signaled because it got aborted.
                Debug.Assert(_exception != null);
                throw new InvalidOperationException("the stream is already signaled");
            }
        }

        protected ValueTask<T> WaitSignalAsync(CancellationToken cancel = default)
        {
            if (cancel.CanBeCanceled)
            {
                Debug.Assert(_tokenRegistration == default);
                cancel.ThrowIfCancellationRequested();
                _tokenRegistration = cancel.Register(() =>
                {
                    try
                    {
                        _tokenRegistration.Token.ThrowIfCancellationRequested();
                    }
                    catch (Exception ex)
                    {
                        Abort(ex);
                    }
                });
            }
            return new ValueTask<T>(this, _source.Version);
        }

        T IValueTaskSource<T>.GetResult(short token)
        {
            Debug.Assert(token == _source.Version);

            // Get the result. This will throw if the stream has been aborted. In this case, we let the
            // exception go through and don't reset the source.
            T result = _source.GetResult(token);

            // Reset the source to allow the stream to be signaled again.
            _tokenRegistration.Dispose();
            _tokenRegistration = default;
            _source.Reset();
            int value = Interlocked.CompareExchange(ref _signaled, 0, 1);
            Debug.Assert(value == 1);

            // If an exception is set, we try to set it on the source if it wasn't signaled in the meantime.
            if (_exception != null)
            {
                if (Interlocked.CompareExchange(ref _signaled, 1, 0) == 0)
                {
                    _source.RunContinuationsAsynchronously = true;
                    _source.SetException(_exception);
                }
            }
            return result;
        }

        ValueTaskSourceStatus IValueTaskSource<T>.GetStatus(short token)
        {
            Debug.Assert(token == _source.Version);
            return _source.GetStatus(token);
        }

        void IValueTaskSource<T>.OnCompleted(
            Action<object?> continuation,
            object? state,
            short token,
            ValueTaskSourceOnCompletedFlags flags)
        {
            Debug.Assert(token == _source.Version);
            _source.OnCompleted(continuation, state, token, flags);
        }
    }
}
