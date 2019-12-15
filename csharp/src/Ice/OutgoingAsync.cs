//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Collections.Generic;
using System.Diagnostics;
using System.Threading;
using System.Threading.Tasks;

namespace IceInternal
{
    public interface OutgoingAsyncCompletionCallback
    {
        void init(OutgoingAsyncBase og);

        bool handleSent(bool done, bool alreadySent, OutgoingAsyncBase og);
        bool handleException(Ice.Exception ex, OutgoingAsyncBase og);
        bool handleResponse(bool userThread, bool ok, OutgoingAsyncBase og);

        void handleInvokeSent(bool sentSynchronously, bool done, bool alreadySent, OutgoingAsyncBase og);
        void handleInvokeException(Ice.Exception ex, OutgoingAsyncBase og);
        void handleInvokeResponse(bool ok, OutgoingAsyncBase og);
    }

    public abstract class OutgoingAsyncBase
    {
        public virtual bool sent()
        {
            return sentImpl(true);
        }

        public virtual bool exception(Ice.Exception ex)
        {
            return exceptionImpl(ex);
        }

        public virtual bool response()
        {
            Debug.Assert(false); // Must be overriden by request that can handle responses
            return false;
        }

        public void invokeSentAsync()
        {
            //
            // This is called when it's not safe to call the sent callback
            // synchronously from this thread. Instead the exception callback
            // is called asynchronously from the client thread pool.
            //
            try
            {
                communicator_.clientThreadPool().dispatch(invokeSent, cachedConnection_);
            }
            catch (Ice.CommunicatorDestroyedException)
            {
            }
        }

        public void invokeExceptionAsync()
        {
            //
            // CommunicatorDestroyedCompleted is the only exception that can propagate directly
            // from this method.
            //
            communicator_.clientThreadPool().dispatch(invokeException, cachedConnection_);
        }

        public void invokeResponseAsync()
        {
            //
            // CommunicatorDestroyedCompleted is the only exception that can propagate directly
            // from this method.
            //
            communicator_.clientThreadPool().dispatch(invokeResponse, cachedConnection_);
        }

        public void invokeSent()
        {
            try
            {
                _completionCallback.handleInvokeSent(sentSynchronously_, _doneInSent, _alreadySent, this);
            }
            catch (System.Exception ex)
            {
                warning(ex);
            }

            if (observer_ != null && _doneInSent)
            {
                observer_.detach();
                observer_ = null;
            }
        }
        public void invokeException()
        {
            try
            {
                try
                {
                    throw _ex;
                }
                catch (Ice.Exception ex)
                {
                    _completionCallback.handleInvokeException(ex, this);
                }
            }
            catch (System.Exception ex)
            {
                warning(ex);
            }

            if (observer_ != null)
            {
                observer_.detach();
                observer_ = null;
            }
        }

        public void invokeResponse()
        {
            if (_ex != null)
            {
                invokeException();
                return;
            }

            try
            {
                try
                {
                    _completionCallback.handleInvokeResponse((state_ & StateOK) != 0, this);
                }
                catch (Ice.Exception ex)
                {
                    if (_completionCallback.handleException(ex, this))
                    {
                        _completionCallback.handleInvokeException(ex, this);
                    }
                }
                catch (System.AggregateException ex)
                {
                    throw ex.InnerException;
                }
            }
            catch (System.Exception ex)
            {
                warning(ex);
            }

            if (observer_ != null)
            {
                observer_.detach();
                observer_ = null;
            }
        }

        public virtual void cancelable(CancellationHandler handler)
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
        public void cancel()
        {
            cancel(new Ice.InvocationCanceledException());
        }

        public void attachRemoteObserver(Ice.ConnectionInfo info, Ice.Endpoint endpt, int requestId)
        {
            Ice.Instrumentation.InvocationObserver observer = getObserver();
            if (observer != null)
            {
                int size = os_.size() - Protocol.headerSize - 4;
                childObserver_ = observer.getRemoteObserver(info, endpt, requestId, size);
                if (childObserver_ != null)
                {
                    childObserver_.attach();
                }
            }
        }

        public void attachCollocatedObserver(Ice.ObjectAdapter adapter, int requestId)
        {
            Ice.Instrumentation.InvocationObserver observer = getObserver();
            if (observer != null)
            {
                int size = os_.size() - Protocol.headerSize - 4;
                childObserver_ = observer.getCollocatedObserver(adapter, requestId, size);
                if (childObserver_ != null)
                {
                    childObserver_.attach();
                }
            }
        }

        public Ice.OutputStream getOs()
        {
            return os_;
        }

        public Ice.InputStream getIs()
        {
            return is_;
        }

        public virtual void throwUserException()
        {
        }

        public virtual void cacheMessageBuffers()
        {
        }

        public bool isSynchronous()
        {
            return synchronous_;
        }

        protected OutgoingAsyncBase(Ice.Communicator communicator, OutgoingAsyncCompletionCallback completionCallback,
                                    Ice.OutputStream? os = null, Ice.InputStream? iss = null)
        {
            communicator_ = communicator;
            sentSynchronously_ = false;
            synchronous_ = false;
            _doneInSent = false;
            _alreadySent = false;
            state_ = 0;
            os_ = os ?? new Ice.OutputStream(communicator, Ice.Util.currentProtocolEncoding);
            is_ = iss ?? new Ice.InputStream(communicator, Ice.Util.currentProtocolEncoding);
            _completionCallback = completionCallback;
            if (_completionCallback != null)
            {
                _completionCallback.init(this);
            }
        }

        protected virtual bool sentImpl(bool done)
        {
            lock (this)
            {
                _alreadySent = (state_ & StateSent) > 0;
                state_ |= StateSent;
                if (done)
                {
                    _doneInSent = true;
                    if (childObserver_ != null)
                    {
                        childObserver_.detach();
                        childObserver_ = null;
                    }
                    _cancellationHandler = null;

                    //
                    // For oneway requests after the data has been sent
                    // the buffers can be reused unless this is a
                    // collocated invocation. For collocated invocations
                    // the buffer won't be reused because it has already
                    // been marked as cached in invokeCollocated.
                    //
                    cacheMessageBuffers();
                }

                bool invoke = _completionCallback.handleSent(done, _alreadySent, this);
                if (!invoke && _doneInSent && observer_ != null)
                {
                    observer_.detach();
                    observer_ = null;
                }
                return invoke;
            }
        }

        protected virtual bool exceptionImpl(Ice.Exception ex)
        {
            lock (this)
            {
                _ex = ex;
                if (childObserver_ != null)
                {
                    childObserver_.failed(ex.ice_id());
                    childObserver_.detach();
                    childObserver_ = null;
                }
                _cancellationHandler = null;

                if (observer_ != null)
                {
                    observer_.failed(ex.ice_id());
                }
                bool invoke = _completionCallback.handleException(ex, this);
                if (!invoke && observer_ != null)
                {
                    observer_.detach();
                    observer_ = null;
                }
                return invoke;
            }
        }
        protected virtual bool responseImpl(bool userThread, bool ok, bool invoke)
        {
            lock (this)
            {
                if (ok)
                {
                    state_ |= StateOK;
                }

                _cancellationHandler = null;

                try
                {
                    invoke &= _completionCallback.handleResponse(userThread, ok, this);
                }
                catch (Ice.Exception ex)
                {
                    _ex = ex;
                    invoke = _completionCallback.handleException(ex, this);
                }
                if (!invoke && observer_ != null)
                {
                    observer_.detach();
                    observer_ = null;
                }
                return invoke;
            }
        }

        protected void cancel(Ice.LocalException ex)
        {
            CancellationHandler handler;
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
            handler.asyncRequestCanceled(this, ex);
        }

        protected void warning(System.Exception ex)
        {
            if (communicator_.Properties.getPropertyAsIntWithDefault("Ice.Warn.AMICallback", 1) > 0)
            {
                communicator_.Logger.warning("exception raised by AMI callback:\n" + ex);
            }
        }

        protected Ice.Instrumentation.InvocationObserver getObserver()
        {
            return observer_;
        }

        public bool sentSynchronously()
        {
            return sentSynchronously_;
        }

        protected Ice.Communicator communicator_;
        protected Ice.Connection cachedConnection_;
        protected bool sentSynchronously_;
        protected bool synchronous_;
        protected int state_;

        protected Ice.Instrumentation.InvocationObserver observer_;
        protected Ice.Instrumentation.ChildInvocationObserver childObserver_;

        protected Ice.OutputStream os_;
        protected Ice.InputStream is_;

        private bool _doneInSent;
        private bool _alreadySent;
        private Ice.Exception _ex;
        private Ice.LocalException _cancellationException;
        private CancellationHandler _cancellationHandler;
        private OutgoingAsyncCompletionCallback _completionCallback;

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
    public abstract class ProxyOutgoingAsyncBase : OutgoingAsyncBase, TimerTask
    {
        public abstract int invokeRemote(Ice.ConnectionI connection, bool compress, bool response);
        public abstract int invokeCollocated(CollocatedRequestHandler handler);

        public override bool exception(Ice.Exception exc)
        {
            if (childObserver_ != null)
            {
                childObserver_.failed(exc.ice_id());
                childObserver_.detach();
                childObserver_ = null;
            }

            cachedConnection_ = null;
            if (proxy_.IceReference.getInvocationTimeout() == -2)
            {
                communicator_.timer().cancel(this);
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
                communicator_.retryQueue().add(this, proxy_.IceHandleException(exc, handler_, mode_, _sent, ref _cnt));
                return false;
            }
            catch (Ice.Exception ex)
            {
                return exceptionImpl(ex); // No retries, we're done
            }
        }

        public override void cancelable(CancellationHandler handler)
        {
            if (proxy_.IceReference.getInvocationTimeout() == -2 && cachedConnection_ != null)
            {
                int timeout = cachedConnection_.timeout();
                if (timeout > 0)
                {
                    communicator_.timer().schedule(this, timeout);
                }
            }
            base.cancelable(handler);
        }

        public void retryException(Ice.Exception ex)
        {
            try
            {
                //
                // It's important to let the retry queue do the retry. This is
                // called from the connect request handler and the retry might
                // require could end up waiting for the flush of the
                // connection to be done.
                //
                proxy_.IceUpdateRequestHandler(handler_, null); // Clear request handler and always retry.
                communicator_.retryQueue().add(this, 0);
            }
            catch (Ice.Exception exc)
            {
                if (exception(exc))
                {
                    invokeExceptionAsync();
                }
            }
        }

        public void retry()
        {
            invokeImpl(false);
        }
        public void abort(Ice.Exception ex)
        {
            Debug.Assert(childObserver_ == null);
            if (exceptionImpl(ex))
            {
                invokeExceptionAsync();
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
                                         OutgoingAsyncCompletionCallback completionCallback,
                                         Ice.OutputStream? os = null,
                                         Ice.InputStream? iss = null) :
            base(prx.Communicator, completionCallback, os, iss)
        {
            proxy_ = prx;
            mode_ = Ice.OperationMode.Normal;
            _cnt = 0;
            _sent = false;
        }

        protected void invokeImpl(bool userThread)
        {
            try
            {
                if (userThread)
                {
                    int invocationTimeout = proxy_.IceReference.getInvocationTimeout();
                    if (invocationTimeout > 0)
                    {
                        communicator_.timer().schedule(this, invocationTimeout);
                    }
                }
                else if (observer_ != null)
                {
                    observer_.retried();
                }

                while (true)
                {
                    try
                    {
                        _sent = false;
                        handler_ = proxy_.IceGetRequestHandler();
                        int status = handler_.sendAsyncRequest(this);
                        if ((status & AsyncStatusSent) != 0)
                        {
                            if (userThread)
                            {
                                sentSynchronously_ = true;
                                if ((status & AsyncStatusInvokeSentCallback) != 0)
                                {
                                    invokeSent(); // Call the sent callback from the user thread.
                                }
                            }
                            else
                            {
                                if ((status & AsyncStatusInvokeSentCallback) != 0)
                                {
                                    invokeSentAsync(); // Call the sent callback from a client thread pool thread.
                                }
                            }
                        }
                        return; // We're done!
                    }
                    catch (RetryException)
                    {
                        proxy_.IceUpdateRequestHandler(handler_, null); // Clear request handler and always retry.
                    }
                    catch (Ice.Exception ex)
                    {
                        if (childObserver_ != null)
                        {
                            childObserver_.failed(ex.ice_id());
                            childObserver_.detach();
                            childObserver_ = null;
                        }
                        int interval = proxy_.IceHandleException(ex, handler_, mode_, _sent, ref _cnt);
                        if (interval > 0)
                        {
                            communicator_.retryQueue().add(this, interval);
                            return;
                        }
                        else if (observer_ != null)
                        {
                            observer_.retried();
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
                else if (exceptionImpl(ex)) // No retries, we're done
                {
                    invokeExceptionAsync();
                }
            }
        }
        protected override bool sentImpl(bool done)
        {
            _sent = true;
            if (done)
            {
                if (proxy_.IceReference.getInvocationTimeout() != -1)
                {
                    communicator_.timer().cancel(this);
                }
            }
            return base.sentImpl(done);
        }
        protected override bool exceptionImpl(Ice.Exception ex)
        {
            if (proxy_.IceReference.getInvocationTimeout() != -1)
            {
                communicator_.timer().cancel(this);
            }
            return base.exceptionImpl(ex);
        }

        protected override bool responseImpl(bool userThread, bool ok, bool invoke)
        {
            if (proxy_.IceReference.getInvocationTimeout() != -1)
            {
                communicator_.timer().cancel(this);
            }
            return base.responseImpl(userThread, ok, invoke);
        }

        public void runTimerTask()
        {
            if (proxy_.IceReference.getInvocationTimeout() == -2)
            {
                cancel(new Ice.ConnectionTimeoutException());
            }
            else
            {
                cancel(new Ice.InvocationTimeoutException());
            }
        }

        protected readonly Ice.IObjectPrx proxy_;
        protected RequestHandler handler_;
        protected Ice.OperationMode mode_;

        private int _cnt;
        private bool _sent;
    }

    //
    // Class for handling Slice operation invocations
    //
    public class OutgoingAsync : ProxyOutgoingAsyncBase
    {
        public OutgoingAsync(Ice.IObjectPrx prx, OutgoingAsyncCompletionCallback completionCallback,
                             Ice.OutputStream? os = null, Ice.InputStream? iss = null) :
            base(prx, completionCallback, os, iss)
        {
            encoding_ = Protocol.getCompatibleEncoding(proxy_.IceReference.getEncoding());
            synchronous_ = false;
        }

        public void prepare(string operation, Ice.OperationMode mode, Dictionary<string, string>? context)
        {
            Protocol.checkSupportedProtocol(Protocol.getCompatibleProtocol(proxy_.IceReference.getProtocol()));

            mode_ = mode;

            observer_ = ObserverHelper.get(proxy_, operation, context);

            switch (proxy_.IceReference.getMode())
            {
                case Ice.InvocationMode.Twoway:
                case Ice.InvocationMode.Oneway:
                case Ice.InvocationMode.Datagram:
                    {
                        os_.WriteBlob(Protocol.requestHdr);
                        break;
                    }

                case Ice.InvocationMode.BatchOneway:
                case Ice.InvocationMode.BatchDatagram:
                    {
                        Debug.Assert(false); // not implemented
                        break;
                    }
            }

            Reference rf = proxy_.IceReference;

            rf.getIdentity().ice_writeMembers(os_);

            //
            // For compatibility with the old FacetPath.
            //
            string facet = rf.getFacet();
            if (facet == null || facet.Length == 0)
            {
                os_.WriteStringSeq(null);
            }
            else
            {
                string[] facetPath = { facet };
                os_.WriteStringSeq(facetPath);
            }

            os_.WriteString(operation);

            os_.WriteByte((byte)mode);

            if (context != null)
            {
                //
                // Explicit context
                //
                Ice.ContextHelper.Write(os_, context);
            }
            else
            {
                //
                // Implicit context
                //
                Ice.ImplicitContextI implicitContext = (Ice.ImplicitContextI)rf.getCommunicator().getImplicitContext();
                Dictionary<string, string> prxContext = rf.getContext();

                if (implicitContext == null)
                {
                    Ice.ContextHelper.Write(os_, prxContext);
                }
                else
                {
                    implicitContext.write(prxContext, os_);
                }
            }
        }
        public override bool sent()
        {
            return base.sentImpl(!proxy_.IsTwoway); // done = true if it's not a two-way proxy
        }

        public override bool response()
        {
            //
            // NOTE: this method is called from ConnectionI.parseMessage
            // with the connection locked. Therefore, it must not invoke
            // any user callbacks.
            //
            Debug.Assert(proxy_.IsTwoway); // Can only be called for twoways.

            if (childObserver_ != null)
            {
                childObserver_.reply(is_.size() - Protocol.headerSize - 4);
                childObserver_.detach();
                childObserver_ = null;
            }

            byte replyStatus;
            try
            {
                replyStatus = is_.ReadByte();

                switch (replyStatus)
                {
                    case ReplyStatus.replyOK:
                        {
                            break;
                        }
                    case ReplyStatus.replyUserException:
                        {
                            if (observer_ != null)
                            {
                                observer_.userException();
                            }
                            break;
                        }

                    case ReplyStatus.replyObjectNotExist:
                    case ReplyStatus.replyFacetNotExist:
                    case ReplyStatus.replyOperationNotExist:
                        {
                            Ice.Identity ident = new Ice.Identity();
                            ident.ice_readMembers(is_);

                            //
                            // For compatibility with the old FacetPath.
                            //
                            string[] facetPath = is_.ReadStringSeq();
                            ;
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

                            string operation = is_.ReadString();

                            Ice.RequestFailedException ex = null;
                            switch (replyStatus)
                            {
                                case ReplyStatus.replyObjectNotExist:
                                    {
                                        ex = new Ice.ObjectNotExistException();
                                        break;
                                    }

                                case ReplyStatus.replyFacetNotExist:
                                    {
                                        ex = new Ice.FacetNotExistException();
                                        break;
                                    }

                                case ReplyStatus.replyOperationNotExist:
                                    {
                                        ex = new Ice.OperationNotExistException();
                                        break;
                                    }

                                default:
                                    {
                                        Debug.Assert(false);
                                        break;
                                    }
                            }

                            ex.id = ident;
                            ex.facet = facet;
                            ex.operation = operation;
                            throw ex;
                        }

                    case ReplyStatus.replyUnknownException:
                    case ReplyStatus.replyUnknownLocalException:
                    case ReplyStatus.replyUnknownUserException:
                        {
                            string unknown = is_.ReadString();

                            Ice.UnknownException ex = null;
                            switch (replyStatus)
                            {
                                case ReplyStatus.replyUnknownException:
                                    {
                                        ex = new Ice.UnknownException();
                                        break;
                                    }

                                case ReplyStatus.replyUnknownLocalException:
                                    {
                                        ex = new Ice.UnknownLocalException();
                                        break;
                                    }

                                case ReplyStatus.replyUnknownUserException:
                                    {
                                        ex = new Ice.UnknownUserException();
                                        break;
                                    }

                                default:
                                    {
                                        Debug.Assert(false);
                                        break;
                                    }
                            }

                            ex.unknown = unknown;
                            throw ex;
                        }

                    default:
                        {
                            throw new Ice.UnknownReplyStatusException();
                        }
                }

                return responseImpl(false, replyStatus == ReplyStatus.replyOK, true);
            }
            catch (Ice.Exception ex)
            {
                return exception(ex);
            }
        }

        public override int invokeRemote(Ice.ConnectionI connection, bool compress, bool response)
        {
            cachedConnection_ = connection;
            return connection.sendAsyncRequest(this, compress, response);
        }

        public override int invokeCollocated(CollocatedRequestHandler handler)
        {
            // The stream cannot be cached if the proxy is not a twoway or there is an invocation timeout set.
            if (!proxy_.IsTwoway || proxy_.IceReference.getInvocationTimeout() != -1)
            {
                // Disable caching by marking the streams as cached!
                state_ |= StateCachedBuffers;
            }
            return handler.invokeAsyncRequest(this, synchronous_);
        }

        public new void abort(Ice.Exception ex)
        {
            Ice.InvocationMode mode = proxy_.IceReference.getMode();

            Debug.Assert(mode != Ice.InvocationMode.BatchOneway &&
                         mode != Ice.InvocationMode.BatchDatagram); // not implemented
            base.abort(ex);
        }

        protected void invoke(string operation, bool synchronous)
        {
            synchronous_ = synchronous;
            Ice.InvocationMode mode = proxy_.IceReference.getMode();
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
            invokeImpl(true); // userThread = true
        }

        public void invoke(string operation,
                           Ice.OperationMode mode,
                           Ice.FormatType format,
                           Dictionary<string, string>? context,
                           bool synchronous,
                           System.Action<Ice.OutputStream> write)
        {
            try
            {
                prepare(operation, mode, context);
                if (write != null)
                {
                    os_.StartEncapsulation(encoding_, format);
                    write(os_);
                    os_.EndEncapsulation();
                }
                else
                {
                    os_.WriteEmptyEncapsulation(encoding_);
                }
                invoke(operation, synchronous);
            }
            catch (Ice.Exception ex)
            {
                abort(ex);
            }
        }

        public override void throwUserException()
        {
            try
            {
                is_.StartEncapsulation();
                is_.ThrowException();
            }
            catch (Ice.UserException ex)
            {
                is_.EndEncapsulation();
                if (userException_ != null)
                {
                    userException_.Invoke(ex);
                }
                throw new Ice.UnknownUserException(ex.ice_id());
            }
        }

        public override void cacheMessageBuffers()
        {
            if (proxy_.Communicator.CacheMessageBuffers > 0)
            {
                lock (this)
                {
                    if ((state_ & StateCachedBuffers) > 0)
                    {
                        return;
                    }
                    state_ |= StateCachedBuffers;
                }

                if (is_ != null)
                {
                    is_.Reset();
                }
                os_.Reset();

                proxy_.CacheMessageBuffers(is_, os_);

                is_ = null;
                os_ = null;
            }
        }

        protected readonly Ice.EncodingVersion encoding_;
        protected System.Action<Ice.UserException> userException_;
    }

    public class OutgoingAsyncT<T> : OutgoingAsync
    {
        public OutgoingAsyncT(Ice.IObjectPrx prx,
                              OutgoingAsyncCompletionCallback completionCallback,
                              Ice.OutputStream? os = null,
                              Ice.InputStream? iss = null) :
            base(prx, completionCallback, os, iss)
        {
        }

        public void invoke(string operation,
                           Ice.OperationMode mode,
                           Ice.FormatType format,
                           Dictionary<string, string>? context,
                           bool synchronous,
                           System.Action<Ice.OutputStream>? write = null,
                           System.Action<Ice.UserException>? userException = null,
                           System.Func<Ice.InputStream, T>? read = null)
        {
            read_ = read;
            userException_ = userException;
            base.invoke(operation, mode, format, context, synchronous, write);
        }

        public T getResult(bool ok)
        {
            try
            {
                if (ok)
                {
                    if (read_ == null)
                    {
                        if (is_ == null || is_.isEmpty())
                        {
                            //
                            // If there's no response (oneway), we just set the result
                            // on completion without reading anything from the input stream.
                            //
                        }
                        else
                        {
                            is_.SkipEmptyEncapsulation();
                        }
                        return default(T);
                    }
                    else
                    {
                        is_.StartEncapsulation();
                        T r = read_(is_);
                        is_.EndEncapsulation();
                        return r;
                    }
                }
                else
                {
                    throwUserException();
                    return default(T); // make compiler happy
                }
            }
            finally
            {
                cacheMessageBuffers();
            }
        }

        protected System.Func<Ice.InputStream, T>? read_;
    }

    //
    // Class for handling the proxy's begin_ice_getConnection request.
    //
    internal class ProxyGetConnection : ProxyOutgoingAsyncBase
    {
        public ProxyGetConnection(Ice.IObjectPrx prx, OutgoingAsyncCompletionCallback completionCallback) :
            base(prx, completionCallback)
        {
        }

        public override int invokeRemote(Ice.ConnectionI connection, bool compress, bool response)
        {
            cachedConnection_ = connection;
            if (responseImpl(false, true, true))
            {
                invokeResponseAsync();
            }
            return AsyncStatusSent;
        }

        public override int invokeCollocated(CollocatedRequestHandler handler)
        {
            if (responseImpl(false, true, true))
            {
                invokeResponseAsync();
            }
            return AsyncStatusSent;
        }

        public Ice.Connection getConnection()
        {
            return cachedConnection_;
        }

        public void invoke(string operation, bool synchronous)
        {
            synchronous_ = synchronous;
            observer_ = ObserverHelper.get(proxy_, operation, null);
            invokeImpl(true); // userThread = true
        }
    }

    public abstract class TaskCompletionCallback<T> : TaskCompletionSource<T>, OutgoingAsyncCompletionCallback
    {
        public TaskCompletionCallback(System.IProgress<bool>? progress, CancellationToken cancellationToken)
        {
            progress_ = progress;
            _cancellationToken = cancellationToken;
        }

        public void init(OutgoingAsyncBase outgoing)
        {
            if (_cancellationToken.CanBeCanceled)
            {
                _cancellationToken.Register(outgoing.cancel);
            }
        }

        public bool handleSent(bool done, bool alreadySent, OutgoingAsyncBase og)
        {
            if (done && og.isSynchronous())
            {
                Debug.Assert(progress_ == null);
                handleInvokeSent(false, done, alreadySent, og);
                return false;
            }
            return done || progress_ != null && !alreadySent; // Invoke the sent callback only if not already invoked.
        }

        public bool handleException(Ice.Exception ex, OutgoingAsyncBase og)
        {
            //
            // If this is a synchronous call, we can notify the task from this thread to avoid
            // the thread context switch. We know there aren't any continuations setup with the
            // task.
            //
            if (og.isSynchronous())
            {
                handleInvokeException(ex, og);
                return false;
            }
            else
            {
                return true;
            }
        }

        public bool handleResponse(bool userThread, bool ok, OutgoingAsyncBase og)
        {
            //
            // If this is a synchronous call, we can notify the task from this thread to avoid the
            // thread context switch. We know there aren't any continuations setup with the
            // task.
            //
            if (userThread || og.isSynchronous())
            {
                handleInvokeResponse(ok, og);
                return false;
            }
            else
            {
                return true;
            }
        }

        public virtual void handleInvokeSent(bool sentSynchronously, bool done, bool alreadySent, OutgoingAsyncBase og)
        {
            if (progress_ != null && !alreadySent)
            {
                progress_.Report(sentSynchronously);
            }
            if (done)
            {
                SetResult(default(T));
            }
        }

        public void handleInvokeException(Ice.Exception ex, OutgoingAsyncBase og)
        {
            SetException(ex);
        }

        public abstract void handleInvokeResponse(bool ok, OutgoingAsyncBase og);

        private readonly CancellationToken _cancellationToken;

        protected readonly System.IProgress<bool>? progress_;
    }

    public class OperationTaskCompletionCallback<T> : TaskCompletionCallback<T>
    {
        public OperationTaskCompletionCallback(System.IProgress<bool>? progress, CancellationToken cancellationToken) :
            base(progress, cancellationToken)
        {
        }

        public override void handleInvokeResponse(bool ok, OutgoingAsyncBase og)
        {
            SetResult(((OutgoingAsyncT<T>)og).getResult(ok));
        }
    }
}
