//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Threading;
using System.Threading.Tasks;
using Ice;

namespace IceInternal
{
    public interface IOutgoingAsyncCompletionCallback
    {
        void Init(OutgoingAsyncBase og);

        bool HandleSent(bool done, bool alreadySent, OutgoingAsyncBase og);
        bool HandleException(Ice.Exception ex, OutgoingAsyncBase og);
        bool HandleResponse(bool userThread, bool ok, OutgoingAsyncBase og);

        void HandleInvokeSent(bool sentSynchronously, bool done, bool alreadySent, OutgoingAsyncBase og);
        void HandleInvokeException(Ice.Exception ex, OutgoingAsyncBase og);
        void HandleInvokeResponse(bool ok, OutgoingAsyncBase og);
    }

    public abstract class OutgoingAsyncBase
    {
        public virtual bool Sent() => SentImpl(true);

        public virtual bool Exception(Ice.Exception ex) => ExceptionImpl(ex);

        public virtual bool Response()
        {
            Debug.Assert(false); // Must be overriden by request that can handle responses
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
                _completionCallback.HandleInvokeSent(sentSynchronously_, _doneInSent, _alreadySent, this);
            }
            catch (System.Exception ex)
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
            catch (System.Exception ex)
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
                catch (Ice.Exception ex)
                {
                    if (_completionCallback.HandleException(ex, this))
                    {
                        _completionCallback.HandleInvokeException(ex, this);
                    }
                }
                catch (System.AggregateException ex)
                {
                    throw ex.InnerException;
                }
            }
            catch (System.Exception ex)
            {
                Warning(ex);
            }

            if (Observer != null)
            {
                Observer.Detach();
                Observer = null;
            }
        }

        public virtual void Cancelable(ICancellationHandler handler)
        {
            lock (this)
            {
                if (_cancellationException != null)
                {
                    try
                    {
                        throw _cancellationException;
                    }
                    catch (Ice.LocalException)
                    {
                        _cancellationException = null;
                        throw;
                    }
                }
                _cancellationHandler = handler;
            }
        }
        public void Cancel() => Cancel(new Ice.InvocationCanceledException());

        public void AttachRemoteObserver(Ice.ConnectionInfo info, Ice.IEndpoint endpt, int requestId)
        {
            Ice.Instrumentation.IInvocationObserver? observer = GetObserver();
            if (observer != null)
            {
                int size = Os!.Size - Ice1Definitions.HeaderSize - 4;
                ChildObserver = observer.GetRemoteObserver(info, endpt, requestId, size);
                if (ChildObserver != null)
                {
                    ChildObserver.Attach();
                }
            }
        }

        public void AttachCollocatedObserver(Ice.ObjectAdapter adapter, int requestId)
        {
            Ice.Instrumentation.IInvocationObserver? observer = GetObserver();
            if (observer != null)
            {
                int size = Os!.Size - Ice1Definitions.HeaderSize - 4;
                ChildObserver = observer.GetCollocatedObserver(adapter, requestId, size);
                if (ChildObserver != null)
                {
                    ChildObserver.Attach();
                }
            }
        }

        public Ice.OutputStream GetOs() => Os!;

        public Ice.InputStream GetIs() => Is!;

        public virtual void ThrowUserException()
        {
        }

        public bool IsSynchronous() => Synchronous;

        protected OutgoingAsyncBase(Ice.Communicator communicator, IOutgoingAsyncCompletionCallback completionCallback,
                                    Ice.OutputStream? os = null, Ice.InputStream? iss = null)
        {
            Communicator = communicator;
            sentSynchronously_ = false;
            Synchronous = false;
            _doneInSent = false;
            _alreadySent = false;
            State = 0;
            Os = os ?? new Ice.OutputStream(communicator, Ice.Util.CurrentProtocolEncoding);
            Is = iss ?? new Ice.InputStream(communicator, Ice.Util.CurrentProtocolEncoding);
            _completionCallback = completionCallback;
            if (_completionCallback != null)
            {
                _completionCallback.Init(this);
            }
        }

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

        protected virtual bool ExceptionImpl(Ice.Exception ex)
        {
            lock (this)
            {
                _ex = ex;
                if (ChildObserver != null)
                {
                    ChildObserver.Failed(ex.ice_id());
                    ChildObserver.Detach();
                    ChildObserver = null;
                }
                _cancellationHandler = null;

                if (Observer != null)
                {
                    Observer.Failed(ex.ice_id());
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
                catch (Ice.Exception ex)
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

        protected void Cancel(Ice.LocalException ex)
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

        protected void Warning(System.Exception ex)
        {
            if ((Communicator.GetPropertyAsInt("Ice.Warn.AMICallback") ?? 1) > 0)
            {
                Communicator.Logger.Warning("exception raised by AMI callback:\n" + ex);
            }
        }

        protected Ice.Instrumentation.IInvocationObserver? GetObserver() => Observer;

        public bool SentSynchronously() => sentSynchronously_;

        protected Communicator Communicator;
        protected Connection? CachedConnection;
        protected bool sentSynchronously_;
        protected bool Synchronous;
        protected int State;

        protected Ice.Instrumentation.IInvocationObserver? Observer;
        protected Ice.Instrumentation.IChildInvocationObserver? ChildObserver;

        protected OutputStream? Os;
        protected InputStream? Is;

        private bool _doneInSent;
        private bool _alreadySent;
        private Ice.Exception? _ex;
        private LocalException? _cancellationException;
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
        public abstract int InvokeRemote(Ice.Connection connection, bool compress, bool response);
        public abstract int InvokeCollocated(CollocatedRequestHandler handler);

        public override bool Exception(Ice.Exception exc)
        {
            if (ChildObserver != null)
            {
                ChildObserver.Failed(exc.ice_id());
                ChildObserver.Detach();
                ChildObserver = null;
            }

            CachedConnection = null;
            if (Proxy.IceReference.GetInvocationTimeout() == -2)
            {
                Communicator.Timer().Cancel(this);
            }

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
            catch (Ice.Exception ex)
            {
                return ExceptionImpl(ex); // No retries, we're done
            }
        }

        public override void Cancelable(ICancellationHandler handler)
        {
            if (Proxy.IceReference.GetInvocationTimeout() == -2 && CachedConnection != null)
            {
                int timeout = CachedConnection.Timeout;
                if (timeout > 0)
                {
                    Communicator.Timer().Schedule(this, timeout);
                }
            }
            base.Cancelable(handler);
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
                Proxy.IceUpdateRequestHandler(Handler, null); // Clear request handler and always retry.
                Communicator.AddRetryTask(this, 0);
            }
            catch (Ice.Exception ex)
            {
                if (Exception(ex))
                {
                    InvokeExceptionAsync();
                }
            }
        }

        public void Retry() => InvokeImpl(false);
        public void Abort(Ice.Exception ex)
        {
            Debug.Assert(ChildObserver == null);
            if (ExceptionImpl(ex))
            {
                InvokeExceptionAsync();
            }
            else if (ex is Ice.CommunicatorDestroyedException)
            {
                //
                // If it's a communicator destroyed exception, swallow
                // it but instead notify the user thread. Even if no callback
                // was provided.
                //
                throw ex;
            }
        }

        protected ProxyOutgoingAsyncBase(Ice.IObjectPrx prx,
                                         IOutgoingAsyncCompletionCallback completionCallback,
                                         Ice.OutputStream? os = null,
                                         Ice.InputStream? iss = null) :
            base(prx.Communicator, completionCallback, os, iss)
        {
            Proxy = prx;
            IsIdempotent = false;
            _cnt = 0;
            _sent = false;
        }

        protected void InvokeImpl(bool userThread)
        {
            try
            {
                if (userThread)
                {
                    int invocationTimeout = Proxy.IceReference.GetInvocationTimeout();
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
                        Handler = Proxy.IceGetRequestHandler();
                        int status = Handler.SendAsyncRequest(this);
                        if ((status & AsyncStatusSent) != 0)
                        {
                            if (userThread)
                            {
                                sentSynchronously_ = true;
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
                        Proxy.IceUpdateRequestHandler(Handler, null); // Clear request handler and always retry.
                    }
                    catch (Ice.Exception ex)
                    {
                        if (ChildObserver != null)
                        {
                            ChildObserver.Failed(ex.ice_id());
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
            catch (Ice.Exception ex)
            {
                //
                // If called from the user thread we re-throw, the exception
                // will be catch by the caller and abort() will be called.
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
                if (Proxy.IceReference.GetInvocationTimeout() != -1)
                {
                    Communicator.Timer().Cancel(this);
                }
            }
            return base.SentImpl(done);
        }
        protected override bool ExceptionImpl(Ice.Exception ex)
        {
            if (Proxy.IceReference.GetInvocationTimeout() != -1)
            {
                Communicator.Timer().Cancel(this);
            }
            return base.ExceptionImpl(ex);
        }

        protected override bool ResponseImpl(bool userThread, bool ok, bool invoke)
        {
            if (Proxy.IceReference.GetInvocationTimeout() != -1)
            {
                Communicator.Timer().Cancel(this);
            }
            return base.ResponseImpl(userThread, ok, invoke);
        }

        public void RunTimerTask()
        {
            if (Proxy.IceReference.GetInvocationTimeout() == -2)
            {
                Cancel(new Ice.ConnectionTimeoutException());
            }
            else
            {
                Cancel(new Ice.InvocationTimeoutException());
            }
        }

        protected readonly Ice.IObjectPrx Proxy;
        protected IRequestHandler? Handler;
        protected bool IsIdempotent;

        private int _cnt;
        private bool _sent;
    }

    //
    // Class for handling Slice operation invocations
    //
    public class OutgoingAsync : ProxyOutgoingAsyncBase
    {
        public OutgoingAsync(Ice.IObjectPrx prx, IOutgoingAsyncCompletionCallback completionCallback,
                             Ice.OutputStream? os = null, Ice.InputStream? iss = null) :
            base(prx, completionCallback, os, iss)
        {
            Encoding = Proxy.Encoding;
            Synchronous = false;
        }

        public void Prepare(string operation, bool idempotent, Dictionary<string, string>? context)
        {
            Debug.Assert(Os != null);
            Protocol.CheckSupportedProtocol(Proxy.IceReference.GetProtocol());

            IsIdempotent = idempotent;

            Observer = ObserverHelper.get(Proxy, operation, context);

            switch (Proxy.IceReference.GetMode())
            {
                case Ice.InvocationMode.Twoway:
                case Ice.InvocationMode.Oneway:
                case Ice.InvocationMode.Datagram:
                    {
                        Os.WriteSpan(Ice1Definitions.RequestHeader.AsSpan());
                        break;
                    }

                case Ice.InvocationMode.BatchOneway:
                case Ice.InvocationMode.BatchDatagram:
                    {
                        Debug.Assert(false); // not implemented
                        break;
                    }
            }

            Reference rf = Proxy.IceReference;

            rf.GetIdentity().IceWrite(Os);

            //
            // For compatibility with the old FacetPath.
            //
            string facet = rf.GetFacet();
            if (facet == null || facet.Length == 0)
            {
                Os.WriteStringSeq(System.Array.Empty<string>());
            }
            else
            {
                Os.WriteStringSeq(new string[]{ facet });
            }

            Os.WriteString(operation);
            Os.Write(idempotent ? OperationMode.Idempotent : OperationMode.Normal);

            if (context != null)
            {
                //
                // Explicit context
                //
                Ice.ContextHelper.Write(Os, context);
            }
            else
            {
                //
                // Implicit context
                //
                var implicitContext = (Ice.ImplicitContext?)rf.GetCommunicator().GetImplicitContext();
                Dictionary<string, string> prxContext = rf.GetContext();

                if (implicitContext == null)
                {
                    Ice.ContextHelper.Write(Os, prxContext);
                }
                else
                {
                    implicitContext.Write(prxContext, Os);
                }
            }
        }
        public override bool Sent() => base.SentImpl(Proxy.IsOneway); // done = true if it's not a two-way proxy

        public override bool Response()
        {
            Debug.Assert(Is != null);
            //
            // NOTE: this method is called from ConnectionI.parseMessage
            // with the connection locked. Therefore, it must not invoke
            // any user callbacks.
            //
            Debug.Assert(!Proxy.IsOneway); // Can only be called for twoways.

            if (ChildObserver != null)
            {
                ChildObserver.Reply(Is.Size - Ice1Definitions.HeaderSize - 4);
                ChildObserver.Detach();
                ChildObserver = null;
            }

            ReplyStatus replyStatus;
            try
            {
                replyStatus = (ReplyStatus)Is.ReadByte();

                switch (replyStatus)
                {
                    case ReplyStatus.OK:
                        {
                            break;
                        }
                    case ReplyStatus.UserException:
                        {
                            if (Observer != null)
                            {
                                Observer.UserException();
                            }
                            break;
                        }

                    case ReplyStatus.ObjectNotExistException:
                    case ReplyStatus.FacetNotExistException:
                    case ReplyStatus.OperationNotExistException:
                        {
                            var ident = new Ice.Identity(Is);

                            //
                            // For compatibility with the old FacetPath.
                            //
                            string[] facetPath = Is.ReadStringArray();
                            string facet;
                            if (facetPath.Length > 0)
                            {
                                if (facetPath.Length > 1)
                                {
                                    throw new Ice.MarshalException();
                                }
                                facet = facetPath[0];
                            }
                            else
                            {
                                facet = "";
                            }

                            string operation = Is.ReadString();

                            Ice.RequestFailedException ex;
                            switch (replyStatus)
                            {
                                case ReplyStatus.ObjectNotExistException:
                                    {
                                        ex = new Ice.ObjectNotExistException();
                                        break;
                                    }

                                case ReplyStatus.FacetNotExistException:
                                    {
                                        ex = new Ice.FacetNotExistException();
                                        break;
                                    }

                                case ReplyStatus.OperationNotExistException:
                                    {
                                        ex = new Ice.OperationNotExistException();
                                        break;
                                    }

                                default:
                                    {
                                        Debug.Assert(false);
                                        throw new System.InvalidOperationException();
                                    }
                            }

                            ex.Id = ident;
                            ex.Facet = facet;
                            ex.Operation = operation;
                            throw ex;
                        }

                    case ReplyStatus.UnknownException:
                    case ReplyStatus.UnknownLocalException:
                    case ReplyStatus.UnknownUserException:
                        {
                            string unknown = Is.ReadString();

                            Ice.UnknownException ex;
                            switch (replyStatus)
                            {
                                case ReplyStatus.UnknownException:
                                    {
                                        ex = new Ice.UnknownException();
                                        break;
                                    }

                                case ReplyStatus.UnknownLocalException:
                                    {
                                        ex = new Ice.UnknownLocalException();
                                        break;
                                    }

                                case ReplyStatus.UnknownUserException:
                                    {
                                        ex = new Ice.UnknownUserException();
                                        break;
                                    }

                                default:
                                    {
                                        Debug.Assert(false);
                                        throw new System.InvalidOperationException();
                                    }
                            }

                            ex.Unknown = unknown;
                            throw ex;
                        }

                    default:
                        {
                            throw new Ice.UnknownReplyStatusException();
                        }
                }

                return ResponseImpl(false, replyStatus == ReplyStatus.OK, true);
            }
            catch (Ice.Exception ex)
            {
                return Exception(ex);
            }
        }

        public override int InvokeRemote(Ice.Connection connection, bool compress, bool response)
        {
            CachedConnection = connection;
            return connection.SendAsyncRequest(this, compress, response);
        }

        public override int InvokeCollocated(CollocatedRequestHandler handler)
        {
            // The stream cannot be cached if the proxy is not a twoway or there is an invocation timeout set.
            if (Proxy.IsOneway || Proxy.IceReference.GetInvocationTimeout() != -1)
            {
                // Disable caching by marking the streams as cached!
                State |= StateCachedBuffers;
            }
            return handler.InvokeAsyncRequest(this, Synchronous);
        }

        public new void Abort(Ice.Exception ex)
        {
            Ice.InvocationMode mode = Proxy.IceReference.GetMode();

            Debug.Assert(mode != Ice.InvocationMode.BatchOneway &&
                         mode != Ice.InvocationMode.BatchDatagram); // not implemented
            base.Abort(ex);
        }

        // Called by IceInvokeAsync
        internal void Invoke(string operation, Dictionary<string, string>? context, bool synchronous)
        {
            Observer = ObserverHelper.get(Proxy, operation, context);
            Invoke(synchronous);
        }

        protected void Invoke(bool synchronous)
        {
            Synchronous = synchronous;
            Ice.InvocationMode mode = Proxy.IceReference.GetMode();
            if (mode == Ice.InvocationMode.BatchOneway || mode == Ice.InvocationMode.BatchDatagram)
            {
                Debug.Assert(false); // not implemented
                return;
            }

            //
            // NOTE: invokeImpl doesn't throw so this can be called from the
            // try block with the catch block calling abort() in case of an
            // exception.
            //
            InvokeImpl(true); // userThread = true
        }

        public void Invoke(string operation,
                           bool idempotent,
                           Ice.FormatType? format,
                           Dictionary<string, string>? context,
                           bool synchronous,
                           System.Action<Ice.OutputStream>? write)
        {
            Debug.Assert(Os != null);
            try
            {
                Prepare(operation, idempotent, context);
                if (write != null)
                {
                    Os.StartEncapsulation(Encoding, format);
                    write(Os);
                    Os.EndEncapsulation();
                }
                else
                {
                    Os.WriteEmptyEncapsulation(Encoding);
                }
                Invoke(synchronous);
            }
            catch (Ice.Exception ex)
            {
                Abort(ex);
            }
        }

        public override void ThrowUserException()
        {
            Debug.Assert(Is != null);
            try
            {
                Is.StartEncapsulation();
                Is.ThrowException();
            }
            catch (Ice.UserException ex)
            {
                Is.EndEncapsulation();
                if (UserException != null)
                {
                    UserException.Invoke(ex);
                }
                throw new Ice.UnknownUserException(ex.ice_id());
            }
        }

        protected readonly Ice.Encoding Encoding;
        protected System.Action<Ice.UserException>? UserException;
    }

    public class OutgoingAsyncT<T> : OutgoingAsync
    {
        public OutgoingAsyncT(Ice.IObjectPrx prx,
                              IOutgoingAsyncCompletionCallback completionCallback,
                              Ice.OutputStream? os = null,
                              Ice.InputStream? iss = null) :
            base(prx, completionCallback, os, iss)
        {
        }

        public void Invoke(string operation,
                           bool idempotent,
                           Ice.FormatType? format,
                           Dictionary<string, string>? context,
                           bool synchronous,
                           System.Action<Ice.OutputStream>? write = null,
                           System.Action<Ice.UserException>? userException = null,
                           System.Func<Ice.InputStream, T>? read = null)
        {
            Read = read;
            UserException = userException;
            base.Invoke(operation, idempotent, format, context, synchronous, write);
        }

        public T GetResult(bool ok)
        {
            if (ok)
            {
                if (Read == null)
                {
                    if (Is == null || Is.IsEmpty)
                    {
                        //
                        // If there's no response (oneway), we just set the result
                        // on completion without reading anything from the input stream.
                        //
                    }
                    else
                    {
                        Is.SkipEmptyEncapsulation();
                    }
                    return default;
                }
                else
                {
                    Debug.Assert(Is != null);
                    Is.StartEncapsulation();
                    T r = Read(Is);
                    Is.EndEncapsulation();
                    return r;
                }
            }
            else
            {
                ThrowUserException();
                return default; // make compiler happy
            }
        }

        protected System.Func<Ice.InputStream, T>? Read;
    }

    //
    // Class for handling the proxy's begin_ice_getConnection request.
    //
    internal class ProxyGetConnection : ProxyOutgoingAsyncBase
    {
        public ProxyGetConnection(Ice.IObjectPrx prx, IOutgoingAsyncCompletionCallback completionCallback) :
            base(prx, completionCallback)
        {
        }

        public override int InvokeRemote(Ice.Connection connection, bool compress, bool response)
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

        public Ice.Connection? GetConnection() => CachedConnection;

        public void Invoke(string operation, bool synchronous)
        {
            Synchronous = synchronous;
            Observer = ObserverHelper.get(Proxy, operation, null);
            InvokeImpl(true); // userThread = true
        }
    }

    public abstract class TaskCompletionCallback<T> : TaskCompletionSource<T>, IOutgoingAsyncCompletionCallback
    {
        public TaskCompletionCallback(System.IProgress<bool>? progress, CancellationToken cancellationToken)
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

        public bool HandleException(Ice.Exception ex, OutgoingAsyncBase og)
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

        public void HandleInvokeException(Ice.Exception ex, OutgoingAsyncBase og) => SetException(ex);

        public abstract void HandleInvokeResponse(bool ok, OutgoingAsyncBase og);

        private readonly CancellationToken _cancellationToken;

        protected readonly System.IProgress<bool>? Progress;
    }

    public class OperationTaskCompletionCallback<T> : TaskCompletionCallback<T>
    {
        public OperationTaskCompletionCallback(System.IProgress<bool>? progress, CancellationToken cancellationToken) :
            base(progress, cancellationToken)
        {
        }

        public override void HandleInvokeResponse(bool ok, OutgoingAsyncBase og) => SetResult(((OutgoingAsyncT<T>)og).GetResult(ok));
    }
}
