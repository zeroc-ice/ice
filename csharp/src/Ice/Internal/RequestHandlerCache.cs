// Copyright (c) ZeroC, Inc.

#nullable enable

using System.Diagnostics;

namespace Ice.Internal;

internal sealed class RequestHandlerCache
{
    internal RequestHandler? requestHandler
    {
        get
        {
            if (_cacheConnection)
            {
                lock (_mutex)
                {
                    if (_cachedRequestHandler != null)
                    {
                        return _cachedRequestHandler;
                    }
                }
            }

            RequestHandler handler = _reference.getRequestHandler();
            if (_cacheConnection)
            {
                lock (_mutex)
                {
                    if (_cachedRequestHandler is null)
                    {
                        _cachedRequestHandler = handler;
                    }
                    // else ignore handler
                    return _cachedRequestHandler;
                }
            }
            else
            {
                return handler;
            }
        }
    }

    internal Connection? cachedConnection
    {
        get
        {
            if (_cacheConnection)
            {
                RequestHandler? handler;
                lock (_mutex)
                {
                    handler = _cachedRequestHandler;
                }
                if (handler is not null)
                {
                    return handler.getConnection();
                }
            }
            return null;
        }
    }

    private readonly Reference _reference;
    private readonly bool _cacheConnection;
    private readonly object _mutex = new();
    private RequestHandler? _cachedRequestHandler;

    internal RequestHandlerCache(Reference reference)
    {
        _reference = reference;
        _cacheConnection = reference.getCacheConnection();
    }

    internal void clearCachedRequestHandler(RequestHandler handler)
    {
        if (_cacheConnection)
        {
            lock (_mutex)
            {
                if (handler == _cachedRequestHandler)
                {
                    _cachedRequestHandler = null;
                }
            }
        }
    }

    internal int handleException(Exception ex, RequestHandler handler, OperationMode mode, bool sent, ref int cnt)
    {
        clearCachedRequestHandler(handler);

        //
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
        //
        if (ex is LocalException && (!sent ||
                                    mode == OperationMode.Nonmutating || mode == OperationMode.Idempotent ||
                                    ex is CloseConnectionException ||
                                    ex is ObjectNotExistException))
        {
            try
            {
                return checkRetryAfterException((LocalException)ex, _reference, ref cnt);
            }
            catch (CommunicatorDestroyedException)
            {
                //
                // The communicator is already destroyed, so we cannot retry.
                //
                throw ex;
            }
        }
        else
        {
            throw ex; // Retry could break at-most-once semantics, don't retry.
        }
    }

    private static int checkRetryAfterException(Ice.LocalException ex, Reference @ref, ref int cnt)
    {
        Instance instance = @ref.getInstance();

        TraceLevels traceLevels = instance.traceLevels();
        Ice.Logger logger = instance.initializationData().logger!;

        //
        // We don't retry batch requests because the exception might have caused
        // the all the requests batched with the connection to be aborted and we
        // want the application to be notified.
        //
        if (@ref.getMode() == Reference.Mode.ModeBatchOneway || @ref.getMode() == Reference.Mode.ModeBatchDatagram)
        {
            throw ex;
        }

        //
        // If it's a fixed proxy, retrying isn't useful as the proxy is tied to
        // the connection and the request will fail with the exception.
        //
        if (@ref is FixedReference)
        {
            throw ex;
        }

        Ice.ObjectNotExistException? one = ex as Ice.ObjectNotExistException;
        if (one is not null)
        {
            if (@ref.getRouterInfo() != null && one.operation == "ice_add_proxy")
            {
                //
                // If we have a router, an ObjectNotExistException with an
                // operation name "ice_add_proxy" indicates to the client
                // that the router isn't aware of the proxy (for example,
                // because it was evicted by the router). In this case, we
                // must *always* retry, so that the missing proxy is added
                // to the router.
                //

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
                //
                // We retry ObjectNotExistException if the reference is
                // indirect.
                //

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
                //
                // For all other cases, we don't retry ObjectNotExistException.
                //
                throw ex;
            }
        }
        else if (ex is Ice.RequestFailedException)
        {
            throw ex;
        }

        //
        // There is no point in retrying an operation that resulted in a
        // MarshalException. This must have been raised locally (because if
        // it happened in a server it would result in an UnknownLocalException
        // instead), which means there was a problem in this process that will
        // not change if we try again.
        //
        // The most likely cause for a MarshalException is exceeding the
        // maximum message size, which is represented by the subclass
        // MemoryLimitException. For example, a client can attempt to send a
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
        //
        if (ex is Ice.MarshalException)
        {
            throw ex;
        }

        //
        // Don't retry if the communicator is destroyed, object adapter is deactivated,
        // or connection is manually closed.
        //
        if (ex is Ice.CommunicatorDestroyedException ||
           ex is Ice.ObjectAdapterDeactivatedException ||
           ex is Ice.ConnectionManuallyClosedException)
        {
            throw ex;
        }

        //
        // Don't retry invocation timeouts.
        //
        if (ex is Ice.InvocationTimeoutException || ex is Ice.InvocationCanceledException)
        {
            throw ex;
        }

        ++cnt;
        Debug.Assert(cnt > 0);

        var retryIntervals = instance.retryIntervals;

        int interval;
        if (cnt == (retryIntervals.Length + 1) && ex is Ice.CloseConnectionException)
        {
            //
            // A close connection exception is always retried at least once, even if the retry
            // limit is reached.
            //
            interval = 0;
        }
        else if (cnt > retryIntervals.Length)
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
            interval = retryIntervals[cnt - 1];
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
}
