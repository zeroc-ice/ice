// Copyright (c) ZeroC, Inc.

using System.Diagnostics;

namespace Ice.Internal;

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
        Debug.Assert(false); // Must be overridden by request that can handle responses
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
            instance_.clientThreadPool().execute(invokeSent, cachedConnection_);
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
        instance_.clientThreadPool().execute(invokeException, cachedConnection_);
    }

    public void invokeResponseAsync()
    {
        //
        // CommunicatorDestroyedCompleted is the only exception that can propagate directly
        // from this method.
        //
        instance_.clientThreadPool().execute(invokeResponse, cachedConnection_);
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
        lock (mutex_)
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

    protected OutgoingAsyncBase(
        Instance instance,
        OutgoingAsyncCompletionCallback completionCallback,
        Ice.OutputStream os = null,
        Ice.InputStream iss = null)
    {
        instance_ = instance;
        sentSynchronously_ = false;
        synchronous_ = false;
        _doneInSent = false;
        _alreadySent = false;
        state_ = 0;
        os_ = os ?? new OutputStream(Ice.Util.currentProtocolEncoding, instance.defaultsAndOverrides().defaultFormat);
        is_ = iss ?? new Ice.InputStream(instance, Ice.Util.currentProtocolEncoding);
        _completionCallback = completionCallback;
        if (_completionCallback != null)
        {
            _completionCallback.init(this);
        }
    }

    protected virtual bool sentImpl(bool done)
    {
        lock (mutex_)
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
        lock (mutex_)
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
        lock (mutex_)
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
            lock (mutex_)
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

    private void warning(System.Exception ex)
    {
        if (instance_.initializationData().properties.getIcePropertyAsInt("Ice.Warn.AMICallback") > 0)
        {
            instance_.initializationData().logger.warning("exception raised by AMI callback:\n" + ex);
        }
    }

    //
    // This virtual method is necessary for the communicator flush
    // batch requests implementation.
    //
    protected virtual Ice.Instrumentation.InvocationObserver getObserver()
    {
        return observer_;
    }

    public bool sentSynchronously()
    {
        return sentSynchronously_;
    }

    protected Instance instance_;
    protected Ice.Connection cachedConnection_;
    protected bool sentSynchronously_;
    protected bool synchronous_;
    protected int state_;

    protected Ice.Instrumentation.InvocationObserver observer_;
    protected Ice.Instrumentation.ChildInvocationObserver childObserver_;

    protected Ice.OutputStream os_;
    protected Ice.InputStream is_;

    protected readonly object mutex_ = new();

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

    public override bool exception(Ice.Exception ex)
    {
        if (childObserver_ != null)
        {
            childObserver_.failed(ex.ice_id());
            childObserver_.detach();
            childObserver_ = null;
        }

        cachedConnection_ = null;

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
            instance_.retryQueue().add(this, handleRetryAfterException(ex));
            return false;
        }
        catch (Ice.Exception retryEx)
        {
            return exceptionImpl(retryEx); // No retries, we're done
        }
    }

    public void retryException()
    {
        try
        {
            // It's important to let the retry queue do the retry. This is
            // called from the connect request handler and the retry might
            // require could end up waiting for the flush of the
            // connection to be done.

            proxy_.iceGetRequestHandlerCache().clearCachedRequestHandler(handler_);
            instance_.retryQueue().add(this, 0);
        }
        catch (Ice.Exception ex)
        {
            if (exception(ex))
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

    protected ProxyOutgoingAsyncBase(
        Ice.ObjectPrxHelperBase prx,
        OutgoingAsyncCompletionCallback completionCallback,
        Ice.OutputStream os = null,
        Ice.InputStream iss = null)
        : base(prx.iceReference().getInstance(), completionCallback, os, iss)
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
                TimeSpan invocationTimeout = proxy_.iceReference().getInvocationTimeout();
                if (invocationTimeout > TimeSpan.Zero)
                {
                    instance_.timer().schedule(this, (long)invocationTimeout.TotalMilliseconds);
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
                    handler_ = proxy_.iceGetRequestHandlerCache().requestHandler;
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
                    // Clear request handler and always retry.
                    proxy_.iceGetRequestHandlerCache().clearCachedRequestHandler(handler_);
                }
                catch (Ice.Exception ex)
                {
                    if (childObserver_ != null)
                    {
                        childObserver_.failed(ex.ice_id());
                        childObserver_.detach();
                        childObserver_ = null;
                    }
                    int interval = handleRetryAfterException(ex);
                    if (interval > 0)
                    {
                        instance_.retryQueue().add(this, interval);
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
            // If called from the user thread we re-throw, the exception will caught by the caller and handled using
            // abort.
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
            if (proxy_.iceReference().getInvocationTimeout() > TimeSpan.Zero)
            {
                instance_.timer().cancel(this);
            }
        }
        return base.sentImpl(done);
    }

    protected override bool exceptionImpl(Ice.Exception ex)
    {
        if (proxy_.iceReference().getInvocationTimeout() > TimeSpan.Zero)
        {
            instance_.timer().cancel(this);
        }
        return base.exceptionImpl(ex);
    }

    protected override bool responseImpl(bool userThread, bool ok, bool invoke)
    {
        if (proxy_.iceReference().getInvocationTimeout() > TimeSpan.Zero)
        {
            instance_.timer().cancel(this);
        }
        return base.responseImpl(userThread, ok, invoke);
    }

    public void runTimerTask() => cancel(new Ice.InvocationTimeoutException());

    private int handleRetryAfterException(Ice.Exception ex)
    {
        // Clear the request handler
        proxy_.iceGetRequestHandlerCache().clearCachedRequestHandler(handler_);

        // We only retry local exception.
        //
        // A CloseConnectionException indicates graceful server shutdown, and is therefore
        // always repeatable without violating "at-most-once". That's because by sending a
        // close connection message, the server guarantees that all outstanding requests
        // can safely be repeated.
        //
        // An ObjectNotExistException can always be retried as well without violating
        // "at-most-once" (see the implementation of the checkRetryAfterException method
        // below for the reasons why it can be useful).
        //
        // If the request didn't get sent or if it's non-mutating or idempotent it can
        // also always be retried if the retry count isn't reached.
        bool shouldRetry = ex is LocalException && (!_sent ||
            mode_ == OperationMode.Nonmutating || mode_ == OperationMode.Idempotent ||
            ex is CloseConnectionException ||
            ex is ObjectNotExistException);

        if (shouldRetry)
        {
            try
            {
                return checkRetryAfterException((LocalException)ex);
            }
            catch (CommunicatorDestroyedException)
            {
                throw ex; // The communicator is already destroyed, so we cannot retry.
            }
        }
        else
        {
            throw ex; // Retry could break at-most-once semantics, don't retry.
        }
    }

    private int checkRetryAfterException(Ice.LocalException ex)
    {
        Reference @ref = proxy_.iceReference();
        Instance instance = @ref.getInstance();

        TraceLevels traceLevels = instance.traceLevels();
        Ice.Logger logger = instance.initializationData().logger!;

        // We don't retry batch requests because the exception might have caused
        // the all the requests batched with the connection to be aborted and we
        // want the application to be notified.
        if (@ref.getMode() == Reference.Mode.ModeBatchOneway || @ref.getMode() == Reference.Mode.ModeBatchDatagram)
        {
            throw ex;
        }

        // If it's a fixed proxy, retrying isn't useful as the proxy is tied to
        // the connection and the request will fail with the exception.
        if (@ref is FixedReference)
        {
            throw ex;
        }

        Ice.ObjectNotExistException one = ex as Ice.ObjectNotExistException;
        if (one is not null)
        {
            if (@ref.getRouterInfo() != null && one.operation == "ice_add_proxy")
            {
                // If we have a router, an ObjectNotExistException with an
                // operation name "ice_add_proxy" indicates to the client
                // that the router isn't aware of the proxy (for example,
                // because it was evicted by the router). In this case, we
                // must *always* retry, so that the missing proxy is added
                // to the router.

                @ref.getRouterInfo().clearCache(@ref);

                if (traceLevels.retry >= 1)
                {
                    string s = "retrying operation call to add proxy to router\n" + ex;
                    logger.trace(traceLevels.retryCat, s);
                }
                return 0; // We must always retry, so we don't look at the retry count.
            }
            else if (@ref.isIndirect())
            {
                // We retry ObjectNotExistException if the reference is indirect.

                if (@ref.isWellKnown())
                {
                    LocatorInfo li = @ref.getLocatorInfo();
                    if (li != null)
                    {
                        li.clearCache(@ref);
                    }
                }
            }
            else
            {
                // For all other cases, we don't retry ObjectNotExistException.
                throw ex;
            }
        }
        else if (ex is Ice.RequestFailedException)
        {
            throw ex;
        }

        // There is no point in retrying an operation that resulted in a
        // MarshalException. This must have been raised locally (because if
        // it happened in a server it would result in an UnknownLocalException
        // instead), which means there was a problem in this process that will
        // not change if we try again.
        //
        // A likely cause for a MarshalException is exceeding the
        // maximum message size. For example, a client can attempt to send a
        // message that exceeds the maximum memory size, or accumulate enough
        // batch requests without flushing that the maximum size is reached.
        //
        // This latter case is especially problematic, because if we were to
        // retry a batch request after a MarshalException, we would in fact
        // silently discard the accumulated requests and allow new batch
        // requests to accumulate. If the subsequent batched requests do not
        // exceed the maximum message size, it appears to the client that all
        // of the batched requests were accepted, when in reality only the
        // last few are actually sent.
        if (ex is Ice.MarshalException)
        {
            throw ex;
        }

        // Don't retry if the communicator is destroyed, object adapter is deactivated,
        // or connection is closed by the application.
        if (ex is CommunicatorDestroyedException ||
           ex is ObjectAdapterDeactivatedException ||
           ex is ObjectAdapterDestroyedException ||
           (ex is ConnectionAbortedException connectionAbortedException &&
            connectionAbortedException.closedByApplication) ||
           (ex is ConnectionClosedException connectionClosedException &&
            connectionClosedException.closedByApplication))
        {
            throw ex;
        }

        // Don't retry invocation timeouts.
        if (ex is Ice.InvocationTimeoutException || ex is Ice.InvocationCanceledException)
        {
            throw ex;
        }

        ++_cnt;
        Debug.Assert(_cnt > 0);

        var retryIntervals = instance.retryIntervals;

        int interval;
        if (_cnt == (retryIntervals.Length + 1) && ex is Ice.CloseConnectionException)
        {
            // A close connection exception is always retried at least once, even if the retry
            // limit is reached.
            interval = 0;
        }
        else if (_cnt > retryIntervals.Length)
        {
            if (traceLevels.retry >= 1)
            {
                string s = "cannot retry operation call because retry limit has been exceeded\n" + ex;
                logger.trace(traceLevels.retryCat, s);
            }
            throw ex;
        }
        else
        {
            interval = retryIntervals[_cnt - 1];
        }

        if (traceLevels.retry >= 1)
        {
            string s = "retrying operation call";
            if (interval > 0)
            {
                s += " in " + interval + "ms";
            }
            s += " because of exception\n" + ex;
            logger.trace(traceLevels.retryCat, s);
        }

        return interval;
    }

    protected readonly Ice.ObjectPrxHelperBase proxy_;
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
    public OutgoingAsync(
        Ice.ObjectPrxHelperBase prx,
        OutgoingAsyncCompletionCallback completionCallback,
        Ice.OutputStream os = null,
        Ice.InputStream iss = null)
        : base(prx, completionCallback, os, iss)
    {
        encoding_ = Protocol.getCompatibleEncoding(proxy_.iceReference().getEncoding());
        synchronous_ = false;
    }

    public void prepare(string operation, Ice.OperationMode mode, Dictionary<string, string> context)
    {
        if (proxy_.iceReference().getProtocol().major != Ice.Util.currentProtocol.major)
        {
            throw new FeatureNotSupportedException(
                $"Cannot send request using protocol version {proxy_.iceReference().getProtocol()}.");
        }

        mode_ = mode;

        observer_ = ObserverHelper.get(proxy_, operation, context);

        if (proxy_.iceReference().isBatch)
        {
            proxy_.iceReference().batchRequestQueue.prepareBatchRequest(os_);
        }
        else
        {
            os_.writeBlob(Protocol.requestHdr);
        }

        Reference rf = proxy_.iceReference();

        rf.getIdentity().ice_writeMembers(os_);

        //
        // For compatibility with the old FacetPath.
        //
        string facet = rf.getFacet();
        if (facet == null || facet.Length == 0)
        {
            os_.writeStringSeq(null);
        }
        else
        {
            string[] facetPath = { facet };
            os_.writeStringSeq(facetPath);
        }

        os_.writeString(operation);

        os_.writeByte((byte)mode);

        if (context != null)
        {
            //
            // Explicit context
            //
            Ice.ContextHelper.write(os_, context);
        }
        else
        {
            //
            // Implicit context
            //
            Ice.ImplicitContextI implicitContext = rf.getInstance().getImplicitContext();
            Dictionary<string, string> prxContext = rf.getContext();

            if (implicitContext == null)
            {
                Ice.ContextHelper.write(os_, prxContext);
            }
            else
            {
                implicitContext.write(prxContext, os_);
            }
        }
    }

    public override bool sent()
    {
        return sentImpl(!proxy_.ice_isTwoway()); // done = true if it's not a two-way proxy
    }

    public override bool response()
    {
        //
        // NOTE: this method is called from ConnectionI.parseMessage
        // with the connection locked. Therefore, it must not invoke
        // any user callbacks.
        //
        Debug.Assert(proxy_.ice_isTwoway()); // Can only be called for twoways.

        if (childObserver_ != null)
        {
            childObserver_.reply(is_.size() - Protocol.headerSize - 4);
            childObserver_.detach();
            childObserver_ = null;
        }

        try
        {
            // The generated code does not throw any exception, even if the byte we received is > 126.
            ReplyStatus replyStatus = ReplyStatusHelper.read(is_);

            switch (replyStatus)
            {
                case ReplyStatus.Ok:
                {
                    break;
                }
                case ReplyStatus.UserException:
                {
                    if (observer_ != null)
                    {
                        observer_.userException();
                    }
                    break;
                }

                case ReplyStatus.ObjectNotExist:
                case ReplyStatus.FacetNotExist:
                case ReplyStatus.OperationNotExist:
                {
                    var ident = new Ice.Identity(is_);

                    //
                    // For compatibility with the old FacetPath.
                    //
                    string[] facetPath = is_.readStringSeq();
                    string facet;
                    if (facetPath.Length > 0)
                    {
                        if (facetPath.Length > 1)
                        {
                            throw new MarshalException($"Received invalid facet path with {facetPath.Length} elements.");
                        }
                        facet = facetPath[0];
                    }
                    else
                    {
                        facet = "";
                    }

                    string operation = is_.readString();
                    throw replyStatus switch
                    {
                        ReplyStatus.ObjectNotExist => new ObjectNotExistException(ident, facet, operation),
                        ReplyStatus.FacetNotExist => new FacetNotExistException(ident, facet, operation),
                        _ => new OperationNotExistException(ident, facet, operation)
                    };
                }

                default:
                {
                    string message = is_.readString();
                    throw replyStatus switch
                    {
                        ReplyStatus.UnknownException => new UnknownException(message),
                        ReplyStatus.UnknownLocalException => new UnknownLocalException(message),
                        ReplyStatus.UnknownUserException => new UnknownUserException(message),
                        _ => new DispatchException(replyStatus, message),
                    };
                }
            }

            return responseImpl(false, replyStatus == ReplyStatus.Ok, true);
        }
        catch (Ice.Exception ex)
        {
            return exception(ex);
        }
    }

    public override int invokeRemote(Ice.ConnectionI connection, bool compress, bool response)
    {
        cachedConnection_ = connection;
        return connection.sendAsyncRequest(this, compress, response, 0);
    }

    public override int invokeCollocated(CollocatedRequestHandler handler)
    {
        // The stream cannot be cached if the proxy is not a twoway or there is an invocation timeout set.
        if (!proxy_.ice_isTwoway() || proxy_.iceReference().getInvocationTimeout() > TimeSpan.Zero)
        {
            // Disable caching by marking the streams as cached!
            state_ |= StateCachedBuffers;
        }
        return handler.invokeAsyncRequest(this, 0, synchronous_);
    }

    public new void abort(Ice.Exception ex)
    {
        if (proxy_.iceReference().isBatch)
        {
            proxy_.iceReference().batchRequestQueue.abortBatchRequest(os_);
        }

        base.abort(ex);
    }

    protected void invoke(string operation, bool synchronous)
    {
        synchronous_ = synchronous;
        if (proxy_.iceReference().isBatch)
        {
            sentSynchronously_ = true;
            proxy_.iceReference().batchRequestQueue.finishBatchRequest(os_, proxy_, operation);
            responseImpl(true, true, false); // Don't call sent/completed callback for batch AMI requests
            return;
        }

        // invokeImpl can throw
        invokeImpl(true); // userThread = true
    }

    public void invoke(
        string operation,
        Ice.OperationMode mode,
        Ice.FormatType? format,
        Dictionary<string, string> context,
        bool synchronous,
        System.Action<Ice.OutputStream> write)
    {
        try
        {
            prepare(operation, mode, context);
            if (write != null)
            {
                os_.startEncapsulation(encoding_, format);
                write(os_);
                os_.endEncapsulation();
            }
            else
            {
                os_.writeEmptyEncapsulation(encoding_);
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
            is_.startEncapsulation();
            is_.throwException();
        }
        catch (UserException ex)
        {
            is_.endEncapsulation();
            if (userException_ != null)
            {
                userException_.Invoke(ex);
            }
            throw UnknownUserException.fromTypeId(ex.ice_id());
        }
    }

    public override void cacheMessageBuffers()
    {
        if (proxy_.iceReference().getInstance().cacheMessageBuffers() > 0)
        {
            lock (mutex_)
            {
                if ((state_ & StateCachedBuffers) > 0)
                {
                    return;
                }
                state_ |= StateCachedBuffers;
            }

            if (is_ != null)
            {
                is_.reset();
            }
            os_.reset();

            proxy_.cacheMessageBuffers(is_, os_);

            is_ = null;
            os_ = null;
        }
    }

    protected readonly Ice.EncodingVersion encoding_;
    protected System.Action<Ice.UserException> userException_;
}

public class OutgoingAsyncT<T> : OutgoingAsync
{
    public OutgoingAsyncT(
        Ice.ObjectPrxHelperBase prx,
        OutgoingAsyncCompletionCallback completionCallback,
        Ice.OutputStream os = null,
        Ice.InputStream iss = null)
        : base(prx, completionCallback, os, iss)
    {
    }

    public void invoke(
        string operation,
        Ice.OperationMode mode,
        Ice.FormatType? format,
        Dictionary<string, string> context,
        bool synchronous,
        System.Action<Ice.OutputStream> write = null,
        System.Action<Ice.UserException> userException = null,
        System.Func<Ice.InputStream, T> read = null)
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
                        // If there's no response (oneway, batch-oneway proxies), we just set the result
                        // on completion without reading anything from the input stream. This is required for
                        // batch invocations.
                        //
                    }
                    else
                    {
                        is_.skipEmptyEncapsulation();
                    }
                    return default(T);
                }
                else
                {
                    is_.startEncapsulation();
                    T r = read_(is_);
                    is_.endEncapsulation();
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

    protected System.Func<Ice.InputStream, T> read_;
}

//
// Class for handling the proxy's begin_ice_flushBatchRequest request.
//
internal class ProxyFlushBatchAsync : ProxyOutgoingAsyncBase
{
    public ProxyFlushBatchAsync(Ice.ObjectPrxHelperBase prx, OutgoingAsyncCompletionCallback completionCallback)
        : base(prx, completionCallback)
    {
    }

    public override int invokeRemote(Ice.ConnectionI connection, bool compress, bool response)
    {
        if (_batchRequestNum == 0)
        {
            if (sent())
            {
                return AsyncStatusSent | AsyncStatusInvokeSentCallback;
            }
            else
            {
                return AsyncStatusSent;
            }
        }
        cachedConnection_ = connection;
        return connection.sendAsyncRequest(this, compress, false, _batchRequestNum);
    }

    public override int invokeCollocated(CollocatedRequestHandler handler)
    {
        if (_batchRequestNum == 0)
        {
            if (sent())
            {
                return AsyncStatusSent | AsyncStatusInvokeSentCallback;
            }
            else
            {
                return AsyncStatusSent;
            }
        }
        return handler.invokeAsyncRequest(this, _batchRequestNum, false);
    }

    public void invoke(string operation, bool synchronous)
    {
        if (proxy_.iceReference().getProtocol().major != Ice.Util.currentProtocol.major)
        {
            throw new FeatureNotSupportedException(
                $"Cannot send request using protocol version {proxy_.iceReference().getProtocol()}.");
        }
        try
        {
            synchronous_ = synchronous;
            observer_ = ObserverHelper.get(proxy_, operation, null);
            // Not used for proxy flush batch requests.
            _batchRequestNum = proxy_.iceReference().batchRequestQueue.swap(os_, out _);
            invokeImpl(true); // userThread = true
        }
        catch (Ice.Exception ex)
        {
            abort(ex);
        }
    }

    private int _batchRequestNum;
}

//
// Class for handling the proxy's begin_ice_getConnection request.
//
internal class ProxyGetConnection : ProxyOutgoingAsyncBase
{
    public ProxyGetConnection(Ice.ObjectPrxHelperBase prx, OutgoingAsyncCompletionCallback completionCallback)
        : base(prx, completionCallback)
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
        try
        {
            synchronous_ = synchronous;
            observer_ = ObserverHelper.get(proxy_, operation, null);
            invokeImpl(true); // userThread = true
        }
        catch (Ice.Exception ex)
        {
            abort(ex);
        }
    }
}

internal class ConnectionFlushBatchAsync : OutgoingAsyncBase
{
    public ConnectionFlushBatchAsync(
        Ice.ConnectionI connection,
        Instance instance,
        OutgoingAsyncCompletionCallback completionCallback)
        : base(instance, completionCallback)
    {
        _connection = connection;
    }

    public void invoke(string operation, Ice.CompressBatch compressBatch, bool synchronous)
    {
        synchronous_ = synchronous;
        observer_ = ObserverHelper.get(instance_, operation);
        try
        {
            int status;
            bool compress;
            int batchRequestNum = _connection.getBatchRequestQueue().swap(os_, out compress);
            if (batchRequestNum == 0)
            {
                status = AsyncStatusSent;
                if (sent())
                {
                    status = status | AsyncStatusInvokeSentCallback;
                }
            }
            else
            {
                bool comp;
                if (compressBatch == Ice.CompressBatch.Yes)
                {
                    comp = true;
                }
                else if (compressBatch == Ice.CompressBatch.No)
                {
                    comp = false;
                }
                else
                {
                    comp = compress;
                }
                status = _connection.sendAsyncRequest(this, comp, false, batchRequestNum);
            }

            if ((status & AsyncStatusSent) != 0)
            {
                sentSynchronously_ = true;
                if ((status & AsyncStatusInvokeSentCallback) != 0)
                {
                    invokeSent();
                }
            }
        }
        catch (RetryException ex)
        {
            try
            {
                throw ex.get();
            }
            catch (Ice.LocalException ee)
            {
                if (exception(ee))
                {
                    invokeExceptionAsync();
                }
            }
        }
        catch (Ice.Exception ex)
        {
            if (exception(ex))
            {
                invokeExceptionAsync();
            }
        }
    }

    private readonly Ice.ConnectionI _connection;
}

public class CommunicatorFlushBatchAsync : OutgoingAsyncBase
{
    private class FlushBatch : OutgoingAsyncBase
    {
        public FlushBatch(
            CommunicatorFlushBatchAsync outAsync,
            Instance instance,
            Ice.Instrumentation.InvocationObserver observer)
            : base(instance, null)
        {
            _outAsync = outAsync;
            _observer = observer;
        }

        public override bool
        sent()
        {
            if (childObserver_ != null)
            {
                childObserver_.detach();
                childObserver_ = null;
            }
            _outAsync.check(false);
            return false;
        }

        public override bool
        exception(Ice.Exception ex)
        {
            if (childObserver_ != null)
            {
                childObserver_.failed(ex.ice_id());
                childObserver_.detach();
                childObserver_ = null;
            }
            _outAsync.check(false);
            return false;
        }

        protected override Ice.Instrumentation.InvocationObserver
        getObserver()
        {
            return _observer;
        }

        private CommunicatorFlushBatchAsync _outAsync;
        private Ice.Instrumentation.InvocationObserver _observer;
    }

    public CommunicatorFlushBatchAsync(Instance instance, OutgoingAsyncCompletionCallback callback)
        : base(instance, callback)
    {
        //
        // _useCount is initialized to 1 to prevent premature callbacks.
        // The caller must invoke ready() after all flush requests have
        // been initiated.
        //
        _useCount = 1;
    }

    internal void flushConnection(Ice.ConnectionI con, Ice.CompressBatch compressBatch)
    {
        lock (mutex_)
        {
            ++_useCount;
        }

        try
        {
            var flushBatch = new FlushBatch(this, instance_, observer_);
            bool compress;
            int batchRequestNum = con.getBatchRequestQueue().swap(flushBatch.getOs(), out compress);
            if (batchRequestNum == 0)
            {
                flushBatch.sent();
            }
            else
            {
                bool comp;
                if (compressBatch == Ice.CompressBatch.Yes)
                {
                    comp = true;
                }
                else if (compressBatch == Ice.CompressBatch.No)
                {
                    comp = false;
                }
                else
                {
                    comp = compress;
                }
                con.sendAsyncRequest(flushBatch, comp, false, batchRequestNum);
            }
        }
        catch (Ice.LocalException)
        {
            check(false);
            throw;
        }
    }

    public void invoke(string operation, Ice.CompressBatch compressBatch, bool synchronous)
    {
        synchronous_ = synchronous;
        observer_ = ObserverHelper.get(instance_, operation);
        instance_.outgoingConnectionFactory().flushAsyncBatchRequests(compressBatch, this);
        instance_.objectAdapterFactory().flushAsyncBatchRequests(compressBatch, this);
        check(true);
    }

    public void check(bool userThread)
    {
        lock (mutex_)
        {
            Debug.Assert(_useCount > 0);
            if (--_useCount > 0)
            {
                return;
            }
        }

        if (sentImpl(true))
        {
            if (userThread)
            {
                sentSynchronously_ = true;
                invokeSent();
            }
            else
            {
                invokeSentAsync();
            }
        }
    }

    private int _useCount;
}

public abstract class TaskCompletionCallback<T> : TaskCompletionSource<T>, OutgoingAsyncCompletionCallback
{
    protected TaskCompletionCallback(System.IProgress<bool> progress, CancellationToken cancellationToken)
        : base(TaskCreationOptions.RunContinuationsAsynchronously)
    {
        progress_ = progress;
        _cancellationToken = cancellationToken;
    }

    public void init(OutgoingAsyncBase og)
    {
        if (_cancellationToken.CanBeCanceled)
        {
            _cancellationToken.Register(og.cancel);
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
        return done || (progress_ != null && !alreadySent); // Invoke the sent callback only if not already invoked.
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
        // If called from the user thread (only the case for batch requests) or if this
        // is a synchronous call, we can notify the task from this thread to avoid the
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
            SetResult(default);
        }
    }

    public void handleInvokeException(Ice.Exception ex, OutgoingAsyncBase og)
    {
        SetException(ex);
    }

    public abstract void handleInvokeResponse(bool ok, OutgoingAsyncBase og);

    private readonly CancellationToken _cancellationToken;

    protected readonly System.IProgress<bool> progress_;
}

public class OperationTaskCompletionCallback<T> : TaskCompletionCallback<T>
{
    public OperationTaskCompletionCallback(System.IProgress<bool> progress, CancellationToken cancellationToken)
        : base(progress, cancellationToken)
    {
    }

    public override void handleInvokeResponse(bool ok, OutgoingAsyncBase og)
    {
        SetResult(((OutgoingAsyncT<T>)og).getResult(ok));
    }
}

public class FlushBatchTaskCompletionCallback : TaskCompletionCallback<object>
{
    public FlushBatchTaskCompletionCallback(
        IProgress<bool> progress = null,
        CancellationToken cancellationToken = default)
        : base(progress, cancellationToken)
    {
    }

    public override void handleInvokeResponse(bool ok, OutgoingAsyncBase og)
    {
        SetResult(null);
    }
}
