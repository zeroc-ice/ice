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
        internal bool IsSignaled
        {
            get
            {
                bool lockTaken = false;
                try
                {
                    _lock.Enter(ref lockTaken);
                    return _source.GetStatus(_source.Version) != ValueTaskSourceStatus.Pending;
                }
                finally
                {
                    if (lockTaken)
                    {
                        _lock.Exit();
                    }
                }
            }
        }

        private Exception? _exception;
        // Provide thread safety using a spin lock to avoid having to create another object on the heap. The lock
        // is used to protect the setting of the signal value or exception with the manual reset value task source.
        private SpinLock _lock;
        private ManualResetValueTaskSourceCore<T> _source;
        private CancellationTokenRegistration _tokenRegistration;
        private static readonly Exception _disposedException =
            new ObjectDisposedException(nameof(SignaledSocketStream<T>));

        /// <summary>Aborts the stream. If the stream is waiting to be signaled and the stream is not signaled
        /// already, the stream will be signaled with the exception. If the stream is signaled, we save the
        /// exception to raise it after the stream consumes the signal and waits for a new signal</summary>
        public override void Abort(Exception ex)
        {
            bool lockTaken = false;
            try
            {
                _lock.Enter(ref lockTaken);
                if (_exception == null)
                {
                    _exception = ex;

                    // If the source isn't already signaled, signal completion by setting the exception. Otherwise
                    // if it's already signaled, a result is pending. In this case, we'll raise the exception the
                    // next time the signal is awaited. This is necessary because ManualResetValueTaskSourceCore is
                    // not thread safe and once an exception or result is set we can't call again SetXxx until the
                    // source's result or exception is consumed.
                    if (_source.GetStatus(_source.Version) == ValueTaskSourceStatus.Pending)
                    {
                        _source.SetException(ex);
                    }
                }
            }
            finally
            {
                if (lockTaken)
                {
                    _lock.Exit();
                }
            }
        }

        protected SignaledSocketStream(MultiStreamSocket socket, long streamId)
            : base(socket, streamId) => _source.RunContinuationsAsynchronously = true;

        protected SignaledSocketStream(MultiStreamSocket socket, bool bidirectional, bool control)
            : base(socket, bidirectional, control) => _source.RunContinuationsAsynchronously = true;

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

        protected void SignalCompletion(T result)
        {
            bool lockTaken = false;
            try
            {
                _lock.Enter(ref lockTaken);
                if (_source.GetStatus(_source.Version) == ValueTaskSourceStatus.Pending)
                {
                    // If the source isn't already signaled, signal completion by setting the result.
                    _source.SetResult(result);
                }
                else
                {
                    // The stream is already signaled because it got aborted.
                    Debug.Assert(_exception != null);
                    throw new InvalidOperationException("the stream is already signaled");
                }
            }
            finally
            {
                if (lockTaken)
                {
                    _lock.Exit();
                }
            }
        }

        protected ValueTask<T> WaitSignalAsync(CancellationToken cancel = default)
        {
            if (cancel.CanBeCanceled)
            {
                Debug.Assert(_tokenRegistration == default);
                cancel.ThrowIfCancellationRequested();
                _tokenRegistration = cancel.Register(() => Abort(new OperationCanceledException(cancel)));
            }
            return new ValueTask<T>(this, _source.Version);
        }

        T IValueTaskSource<T>.GetResult(short token)
        {
            bool lockTaken = false;
            try
            {
                _lock.Enter(ref lockTaken);
                Debug.Assert(token == _source.Version);

                // Get the result. This will throw if the stream has been aborted. In this case, we let the
                // exception go through and don't reset the source.
                T result = _source.GetResult(token);

                // Reset the source to allow the stream to be signaled again.
                _tokenRegistration.Dispose();
                _tokenRegistration = default;
                _source.Reset();

                // If an exception is set, we set it on the source.
                if (_exception != null)
                {
                    _source.SetException(_exception);
                }
                return result;
            }
            finally
            {
                if (lockTaken)
                {
                    _lock.Exit();
                }
            }
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
