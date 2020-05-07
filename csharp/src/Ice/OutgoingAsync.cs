//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using Ice;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Threading;
using System.Threading.Tasks;

namespace IceInternal
{
    public interface IOutgoingAsyncCompletionCallback
    {
        void Init(OutgoingAsyncBase og);

        bool HandleSent(bool done, bool alreadySent, OutgoingAsyncBase og);
        bool HandleException(Exception ex, OutgoingAsyncBase og);

        bool HandleResponse(bool userThread, bool ok, OutgoingAsyncBase og);

        void HandleInvokeSent(bool sentSynchronously, bool done, bool alreadySent, OutgoingAsyncBase og);
        void HandleInvokeException(Exception ex, OutgoingAsyncBase og);
        void HandleInvokeResponse(bool ok, OutgoingAsyncBase og);
    }

    public abstract class OutgoingAsyncBase
    {
        public virtual bool Sent() => SentImpl(true);

        public virtual bool Exception(Exception ex) => ExceptionImpl(ex);

        public virtual bool Response(ArraySegment<byte> data)
        {
            Debug.Assert(false); // Must be overridden by request that can handle responses
            return false;
        }

        public void InvokeSentAsync()
        {
            //
            // This is called when it's not safe to call the sent callback
            // synchronously from this thread. Instead the exception callback
            // is called asynchronously from the client thread pool.
            //
            try
            {
                Communicator.ClientThreadPool().Dispatch(InvokeSent);
            }
            catch (Ice.CommunicatorDestroyedException)
            {
            }
        }

        public void InvokeExceptionAsync()
        {
            //
            // CommunicatorDestroyedCompleted is the only exception that can propagate directly
            // from this method.
            //
            Communicator.ClientThreadPool().Dispatch(InvokeException);
        }

        public void InvokeResponseAsync()
        {
            //
            // CommunicatorDestroyedCompleted is the only exception that can propagate directly
            // from this method.
            //
            Communicator.ClientThreadPool().Dispatch(InvokeResponse);
        }

        public void InvokeSent()
        {
            try
            {
                _completionCallback.HandleInvokeSent(SentSynchronously, _doneInSent, _alreadySent, this);
            }
            catch (Exception ex)
            {
                Warning(ex);
            }

            if (Observer != null && _doneInSent)
            {
                Observer.Detach();
                Observer = null;
            }
        }
        public void InvokeException()
        {
            Debug.Assert(_ex != null);
            try
            {
                _completionCallback.HandleInvokeException(_ex, this);
            }
            catch (Exception ex)
            {
                Warning(ex);
            }

            if (Observer != null)
            {
                Observer.Detach();
                Observer = null;
            }
        }

        public void InvokeResponse()
        {
            if (_ex != null)
            {
                InvokeException();
                return;
            }

            try
            {
                try
                {
                    _completionCallback.HandleInvokeResponse((State & StateOK) != 0, this);
                }
                catch (AggregateException ex)
                {
                    Debug.Assert(ex.InnerException != null);
                    throw ex.InnerException;
                }
                catch (Exception ex)
                {
                    if (_completionCallback.HandleException(ex, this))
                    {
                        _completionCallback.HandleInvokeException(ex, this);
                    }
                }
            }
            catch (Exception ex)
            {
                Warning(ex);
            }

            if (Observer != null)
            {
                Observer.Detach();
                Observer = null;
            }
        }

        public void Cancelable(ICancellationHandler handler)
        {
            lock (this)
            {
                if (_cancellationException != null)
                {
                    try
                    {
                        throw _cancellationException;
                    }
                    catch (Exception)
                    {
                        _cancellationException = null;
                        throw;
                    }
                }
                _cancellationHandler = handler;
            }
        }

        // TODO: add more details in message
        public void Cancel() => Cancel(new OperationCanceledException("invocation on remote Ice object canceled"));

        public void AttachRemoteObserver(ConnectionInfo info, Endpoint endpt, int requestId, int size)
        {
            Ice.Instrumentation.IInvocationObserver? observer = GetObserver();
            if (observer != null)
            {
                ChildObserver = observer.GetRemoteObserver(info, endpt, requestId, size);
                if (ChildObserver != null)
                {
                    ChildObserver.Attach();
                }
            }
        }

        public void AttachCollocatedObserver(ObjectAdapter adapter, int requestId, int size)
        {
            Ice.Instrumentation.IInvocationObserver? observer = GetObserver();
            if (observer != null)
            {
                ChildObserver = observer.GetCollocatedObserver(adapter, requestId, size);
                if (ChildObserver != null)
                {
                    ChildObserver.Attach();
                }
            }
        }

        public virtual void ThrowUserException()
        {
        }

        public bool IsSynchronous() => Synchronous;

        protected OutgoingAsyncBase(Communicator communicator, IOutgoingAsyncCompletionCallback completionCallback)
        {
            Communicator = communicator;
            SentSynchronously = false;
            Synchronous = false;
            _doneInSent = false;
            _alreadySent = false;
            State = 0;

            _completionCallback = completionCallback;
            _completionCallback.Init(this);
        }

        public abstract List<ArraySegment<byte>> GetRequestData(int requestId);

        protected virtual bool SentImpl(bool done)
        {
            lock (this)
            {
                _alreadySent = (State & StateSent) > 0;
                State |= StateSent;
                if (done)
                {
                    _doneInSent = true;
                    if (ChildObserver != null)
                    {
                        ChildObserver.Detach();
                        ChildObserver = null;
                    }
                    _cancellationHandler = null;
                }

                bool invoke = _completionCallback.HandleSent(done, _alreadySent, this);
                if (!invoke && _doneInSent && Observer != null)
                {
                    Observer.Detach();
                    Observer = null;
                }
                return invoke;
            }
        }

        protected virtual bool ExceptionImpl(Exception ex)
        {
            lock (this)
            {
                _ex = ex;

                if (ChildObserver != null)
                {
                    ChildObserver.Failed(ex.GetType().FullName ?? "System.Exception");
                    ChildObserver.Detach();
                    ChildObserver = null;
                }
                _cancellationHandler = null;

                if (Observer != null)
                {
                    Observer.Failed(ex.GetType().FullName ?? "System.Exception");
                }
                bool invoke = _completionCallback.HandleException(ex, this);
                if (!invoke && Observer != null)
                {
                    Observer.Detach();
                    Observer = null;
                }
                return invoke;
            }
        }

        protected virtual bool ResponseImpl(bool userThread, bool ok, bool invoke)
        {
            lock (this)
            {
                if (ok)
                {
                    State |= StateOK;
                }

                _cancellationHandler = null;

                try
                {
                    invoke &= _completionCallback.HandleResponse(userThread, ok, this);
                }
                catch (Exception ex)
                {
                    _ex = ex;
                    invoke = _completionCallback.HandleException(ex, this);
                }
                if (!invoke && Observer != null)
                {
                    Observer.Detach();
                    Observer = null;
                }
                return invoke;
            }
        }

        protected void Cancel(Exception ex)
        {
            ICancellationHandler handler;
            {
                lock (this)
                {
                    if (_cancellationHandler == null)
                    {
                        _cancellationException = ex;
                        return;
                    }
                    handler = _cancellationHandler;
                }
            }
            handler.AsyncRequestCanceled(this, ex);
        }

        protected void Warning(Exception ex)
        {
            if (Communicator.GetPropertyAsBool("Ice.Warn.AMICallback") ?? true)
            {
                Communicator.Logger.Warning("exception raised by AMI callback:\n" + ex);
            }
        }

        protected Ice.Instrumentation.IInvocationObserver? GetObserver() => Observer;

        public bool SentSynchronously { get; protected set; }

        protected Communicator Communicator;
        protected Connection? CachedConnection;
        protected bool Synchronous;
        protected int State;

        protected Ice.Instrumentation.IInvocationObserver? Observer;
        protected Ice.Instrumentation.IChildInvocationObserver? ChildObserver;

        private bool _doneInSent;
        private bool _alreadySent;
        private Exception? _ex;
        private Exception? _cancellationException;
        private ICancellationHandler? _cancellationHandler;
        private readonly IOutgoingAsyncCompletionCallback _completionCallback;

        protected const int StateOK = 0x1;
        protected const int StateDone = 0x2;
        protected const int StateSent = 0x4;
        protected const int StateEndCalled = 0x8;
        protected const int StateCachedBuffers = 0x10;

        public const int AsyncStatusQueued = 0;
        public const int AsyncStatusSent = 1;
        public const int AsyncStatusInvokeSentCallback = 2;
    }

    //
    // Base class for proxy based invocations. This class handles the
    // retry for proxy invocations. It also ensures the child observer is
    // correct notified of failures and make sure the retry task is
    // correctly canceled when the invocation completes.
    //
    public abstract class ProxyOutgoingAsyncBase : OutgoingAsyncBase, ITimerTask
    {
        public OutgoingRequestFrame RequestFrame { get; protected set; }
        public IncomingResponseFrame? ResponseFrame { get; protected set; }
        public abstract int InvokeRemote(Connection connection, bool compress, bool response);
        public abstract int InvokeCollocated(CollocatedRequestHandler handler);

        public override List<ArraySegment<byte>> GetRequestData(int requestId) =>
            Ice1Definitions.GetRequestData(RequestFrame, requestId);

        public override bool Exception(Exception exc)
        {
            if (ChildObserver != null)
            {
                ChildObserver.Failed(exc.GetType().FullName ?? "System.Exception");
                ChildObserver.Detach();
                ChildObserver = null;
            }

            CachedConnection = null;

            //
            // NOTE: at this point, synchronization isn't needed, no other threads should be
            // calling on the callback.
            //
            try
            {
                //
                // It's important to let the retry queue do the retry even if
                // the retry interval is 0. This method can be called with the
                // connection locked so we can't just retry here.
                //
                Communicator.AddRetryTask(this, Proxy.IceHandleException(exc, Handler, IsIdempotent, _sent, ref _cnt));
                return false;
            }
            catch (Exception ex)
            {
                return ExceptionImpl(ex); // No retries, we're done
            }
        }

        public void RetryException()
        {
            try
            {
                //
                // It's important to let the retry queue do the retry. This is
                // called from the connect request handler and the retry might
                // require could end up waiting for the flush of the
                // connection to be done.
                //

                // Clear request handler and always retry.
                if (!Proxy.IceReference.IsFixed)
                {
                    Proxy.IceReference.UpdateRequestHandler(Handler, null);
                }
                Communicator.AddRetryTask(this, 0);
            }
            catch (Exception ex)
            {
                if (Exception(ex))
                {
                    InvokeExceptionAsync();
                }
            }
        }

        public void Retry() => InvokeImpl(false);
        public void Abort(Exception ex)
        {
            Debug.Assert(ChildObserver == null);
            if (ExceptionImpl(ex))
            {
                InvokeExceptionAsync();
            }
            else if (ex is CommunicatorDestroyedException)
            {
                //
                // If it's a communicator destroyed exception, swallow
                // it but instead notify the user thread. Even if no callback
                // was provided.
                //
                throw ex;
            }
        }

        protected ProxyOutgoingAsyncBase(IObjectPrx prx,
                                         IOutgoingAsyncCompletionCallback completionCallback,
                                         OutgoingRequestFrame requestFrame) :
            base(prx.Communicator, completionCallback)
        {
            Proxy = prx;
            IsOneway = false;
            _cnt = 0;
            _sent = false;
            RequestFrame = requestFrame;
        }

        protected void InvokeImpl(bool userThread)
        {
            try
            {
                if (userThread)
                {
                    int invocationTimeout = Proxy.IceReference.InvocationTimeout;
                    if (invocationTimeout > 0)
                    {
                        Communicator.Timer().Schedule(this, invocationTimeout);
                    }
                }
                else if (Observer != null)
                {
                    Observer.Retried();
                }

                while (true)
                {
                    try
                    {
                        _sent = false;
                        Handler = Proxy.IceReference.GetRequestHandler();
                        int status = Handler.SendAsyncRequest(this);
                        if ((status & AsyncStatusSent) != 0)
                        {
                            if (userThread)
                            {
                                SentSynchronously = true;
                                if ((status & AsyncStatusInvokeSentCallback) != 0)
                                {
                                    InvokeSent(); // Call the sent callback from the user thread.
                                }
                            }
                            else
                            {
                                if ((status & AsyncStatusInvokeSentCallback) != 0)
                                {
                                    InvokeSentAsync(); // Call the sent callback from a client thread pool thread.
                                }
                            }
                        }
                        return; // We're done!
                    }
                    catch (RetryException)
                    {
                        // Clear request handler and always retry.
                        if (!Proxy.IceReference.IsFixed)
                        {
                            Proxy.IceReference.UpdateRequestHandler(Handler, null);
                        }
                    }
                    catch (Exception ex)
                    {
                        if (ChildObserver != null)
                        {
                            ChildObserver.Failed(ex.GetType().FullName ?? "System.Exception");
                            ChildObserver.Detach();
                            ChildObserver = null;
                        }
                        int interval = Proxy.IceHandleException(ex, Handler, IsIdempotent, _sent, ref _cnt);
                        if (interval > 0)
                        {
                            Communicator.AddRetryTask(this, interval);
                            return;
                        }
                        else if (Observer != null)
                        {
                            Observer.Retried();
                        }
                    }
                }
            }
            catch (Exception ex)
            {
                //
                // If called from the user thread we re-throw, the exception
                // will be caught by the caller and abort() will be called.
                //
                if (userThread)
                {
                    throw;
                }
                else if (ExceptionImpl(ex)) // No retries, we're done
                {
                    InvokeExceptionAsync();
                }
            }
        }
        protected override bool SentImpl(bool done)
        {
            _sent = true;
            if (done)
            {
                ResponseFrame = new IncomingResponseFrame(Communicator, Ice1Definitions.EmptyResponsePayload);
                if (Proxy.IceReference.InvocationTimeout != -1)
                {
                    Communicator.Timer().Cancel(this);
                }
            }
            return base.SentImpl(done);
        }
        protected override bool ExceptionImpl(Exception ex)
        {
            if (Proxy.IceReference.InvocationTimeout != -1)
            {
                Communicator.Timer().Cancel(this);
            }
            return base.ExceptionImpl(ex);
        }

        protected override bool ResponseImpl(bool userThread, bool ok, bool invoke)
        {
            if (Proxy.IceReference.InvocationTimeout != -1)
            {
                Communicator.Timer().Cancel(this);
            }
            return base.ResponseImpl(userThread, ok, invoke);
        }

        // TODO: add facet and operation to message
        public void RunTimerTask()
            => Cancel(new TimeoutException($"invocation on remote Ice object `{Proxy.Identity}' timed out"));

        protected IReadOnlyDictionary<string, string> ProxyAndCurrentContext()
        {
            IReadOnlyDictionary<string, string> context;

            if (Proxy.Context.Count == 0)
            {
                context = Proxy.Communicator.CurrentContext;
            }
            else if (Proxy.Communicator.CurrentContext.Count == 0)
            {
                context = Proxy.Context;
            }
            else
            {
                var combinedContext = new Dictionary<string, string>(Proxy.Communicator.CurrentContext);
                foreach ((string key, string value) in Proxy.Context)
                {
                    combinedContext[key] = value;  // the proxy Context entry prevails.
                }
                context = combinedContext;
            }
            return context;
        }

        protected bool IsIdempotent;
        protected readonly IObjectPrx Proxy;
        protected IRequestHandler? Handler;

        // true for a oneway-capable operation called on a oneway proxy, false otherwise
        protected internal bool IsOneway;

        private int _cnt;
        private bool _sent;
    }

    //
    // Class for handling Slice operation invocations
    //
    public class OutgoingAsync : ProxyOutgoingAsyncBase
    {
        public OutgoingAsync(IObjectPrx prx, IOutgoingAsyncCompletionCallback completionCallback,
            OutgoingRequestFrame requestFrame, bool oneway = false)
            : base(prx, completionCallback, requestFrame)
        {
            Encoding = Proxy.Encoding;
            Synchronous = false;
            IsOneway = oneway;
            IsIdempotent = requestFrame.IsIdempotent;
        }

        public override bool Sent() => SentImpl(IsOneway); // done = true

        public override bool Response(ArraySegment<byte> data)
        {
            ResponseFrame = new IncomingResponseFrame(Communicator,
                data.Slice(Ice1Definitions.HeaderSize + 4));
            //
            // NOTE: this method is called from ConnectionI.parseMessage
            // with the connection locked. Therefore, it must not invoke
            // any user callbacks.
            //
            Debug.Assert(!IsOneway); // Can only be called for twoways.

            if (ChildObserver != null)
            {
                ChildObserver.Reply(ResponseFrame.Size);
                ChildObserver.Detach();
                ChildObserver = null;
            }

            try
            {
                switch (ResponseFrame.ReplyStatus)
                {
                    case ReplyStatus.OK:
                    {
                        break;
                    }
                    case ReplyStatus.UserException:
                    {
                        if (Observer != null)
                        {
                            Observer.RemoteException();
                        }
                        break;
                    }
                    case ReplyStatus.ObjectNotExistException:
                    case ReplyStatus.FacetNotExistException:
                    case ReplyStatus.OperationNotExistException:
                    {
                        throw ResponseFrame.ReadDispatchException();
                    }
                    case ReplyStatus.UnknownException:
                    case ReplyStatus.UnknownLocalException:
                    case ReplyStatus.UnknownUserException:
                    {
                        throw ResponseFrame.ReadUnhandledException();
                    }
                }
                return ResponseImpl(false, ResponseFrame.ReplyStatus == ReplyStatus.OK, true);
            }
            catch (Exception ex)
            {
                return Exception(ex);
            }
        }

        public override int InvokeRemote(Connection connection, bool compress, bool response)
        {
            CachedConnection = connection;
            return connection.SendAsyncRequest(this, compress, response);
        }

        public override int InvokeCollocated(CollocatedRequestHandler handler)
        {
            // The stream cannot be cached if the proxy is not a twoway or there is an invocation timeout set.
            if (IsOneway || Proxy.IceReference.InvocationTimeout != -1)
            {
                // Disable caching by marking the streams as cached!
                State |= StateCachedBuffers;
            }
            return handler.InvokeAsyncRequest(this, Synchronous);
        }

        public new void Abort(Exception ex)
        {
            InvocationMode mode = Proxy.IceReference.InvocationMode;
            // not implemented
            Debug.Assert(mode != InvocationMode.BatchOneway && mode != InvocationMode.BatchDatagram);
            base.Abort(ex);
        }

        // Called by IceInvokeAsync
        internal void Invoke(string operation, IReadOnlyDictionary<string, string>? context, bool synchronous)
        {
            context ??= ProxyAndCurrentContext();
            Observer = ObserverHelper.GetInvocationObserver(Proxy, operation, context);
            Invoke(synchronous);
        }

        protected void Invoke(bool synchronous)
        {
            Synchronous = synchronous;
            InvocationMode mode = Proxy.IceReference.InvocationMode;
            if (mode == InvocationMode.BatchOneway || mode == InvocationMode.BatchDatagram)
            {
                Debug.Assert(false); // not implemented
                return;
            }

            if (mode == InvocationMode.Datagram && !IsOneway)
            {
                throw new InvalidOperationException("cannot make two-way call on a datagram proxy");
            }

            //
            // NOTE: invokeImpl doesn't throw so this can be called from the
            // try block with the catch block calling abort() in case of an
            // exception.
            //
            InvokeImpl(true); // userThread = true
        }

        public void Invoke(string operation,
                           bool oneway,
                           IReadOnlyDictionary<string, string>? context,
                           bool synchronous)
        {
            try
            {
                Proxy.IceReference.Protocol.CheckSupported();
                IsOneway = oneway;
                context ??= ProxyAndCurrentContext();
                Observer = ObserverHelper.GetInvocationObserver(Proxy, operation, context);

                switch (Proxy.IceReference.InvocationMode)
                {
                    case InvocationMode.BatchOneway:
                    case InvocationMode.BatchDatagram:
                    {
                        Debug.Assert(false); // not implemented
                        break;
                    }
                }
                Invoke(synchronous);
            }
            catch (Exception ex)
            {
                Abort(ex);
            }
        }

        protected readonly Encoding Encoding;
    }

    //
    // Class for handling the proxy's begin_ice_getConnection request.
    //
    internal class ProxyGetConnection : ProxyOutgoingAsyncBase
    {
        public ProxyGetConnection(IObjectPrx prx, IOutgoingAsyncCompletionCallback completionCallback)
            : base(prx, completionCallback, null!) => IsIdempotent = false;

        public override int InvokeRemote(Connection connection, bool compress, bool response)
        {
            CachedConnection = connection;
            if (ResponseImpl(false, true, true))
            {
                InvokeResponseAsync();
            }
            return AsyncStatusSent;
        }

        public override int InvokeCollocated(CollocatedRequestHandler handler)
        {
            if (ResponseImpl(false, true, true))
            {
                InvokeResponseAsync();
            }
            return AsyncStatusSent;
        }

        public Connection? GetConnection() => CachedConnection;

        public void Invoke(string operation, bool synchronous)
        {
            // GetConnection succeeds for oneway, twoway and datagram proxies, and is not considered two-way only
            // since it's a local operation.
            Debug.Assert(!IsOneway); // always constructed with IsOneway set to false
            Synchronous = synchronous;
            Observer = ObserverHelper.GetInvocationObserver(Proxy, operation, Reference.EmptyContext);
            InvokeImpl(true); // userThread = true
        }
    }

    public abstract class TaskCompletionCallback<T> : TaskCompletionSource<T>, IOutgoingAsyncCompletionCallback
    {
        public TaskCompletionCallback(IProgress<bool>? progress, CancellationToken cancellationToken)
        {
            Progress = progress;
            _cancellationToken = cancellationToken;
        }

        public void Init(OutgoingAsyncBase outgoing)
        {
            if (_cancellationToken.CanBeCanceled)
            {
                _cancellationToken.Register(outgoing.Cancel);
            }
        }

        public bool HandleSent(bool done, bool alreadySent, OutgoingAsyncBase og)
        {
            if (done && og.IsSynchronous())
            {
                Debug.Assert(Progress == null);
                HandleInvokeSent(false, done, alreadySent, og);
                return false;
            }
            return done || (Progress != null && !alreadySent); // Invoke the sent callback only if not already invoked.
        }

        public bool HandleException(Exception ex, OutgoingAsyncBase og)
        {
            //
            // If this is a synchronous call, we can notify the task from this thread to avoid
            // the thread context switch. We know there aren't any continuations setup with the
            // task.
            //
            if (og.IsSynchronous())
            {
                HandleInvokeException(ex, og);
                return false;
            }
            else
            {
                return true;
            }
        }

        public bool HandleResponse(bool userThread, bool ok, OutgoingAsyncBase og)
        {
            //
            // If this is a synchronous call, we can notify the task from this thread to avoid the
            // thread context switch. We know there aren't any continuations setup with the
            // task.
            //
            if (userThread || og.IsSynchronous())
            {
                HandleInvokeResponse(ok, og);
                return false;
            }
            else
            {
                return true;
            }
        }

        public virtual void HandleInvokeSent(bool sentSynchronously, bool done, bool alreadySent, OutgoingAsyncBase og)
        {
            if (Progress != null && !alreadySent)
            {
                Progress.Report(sentSynchronously);
            }
            if (done)
            {
                SetResult(default!);
            }
        }

        public void HandleInvokeException(Exception ex, OutgoingAsyncBase og) => SetException(ex);

        public abstract void HandleInvokeResponse(bool ok, OutgoingAsyncBase og);

        private readonly CancellationToken _cancellationToken;

        protected readonly IProgress<bool>? Progress;
    }
}
