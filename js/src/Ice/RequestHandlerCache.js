//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import { Debug } from "./Debug.js";
import { Ice as Ice_OperationMode } from "./OperationMode.js";
const { OperationMode } = Ice_OperationMode;
import { ReferenceMode } from "./ReferenceMode.js";
import { LocalException } from "./Exception.js";
import {
    CloseConnectionException,
    ObjectNotExistException,
    CommunicatorDestroyedException,
    RequestFailedException,
    MarshalException,
    ObjectAdapterDeactivatedException,
    ConnectionManuallyClosedException,
    InvocationCanceledException,
    InvocationTimeoutException,
} from "./LocalException.js";

export class RequestHandlerCache {
    get requestHandler() {
        if (this._cacheConnection) {
            if (this._cachedRequestHandler !== null) {
                return this._cachedRequestHandler;
            }
        }

        let handler = this._reference.getRequestHandler();
        if (this._cacheConnection) {
            if (this._cachedRequestHandler === null) {
                this._cachedRequestHandler = handler;
            }
            // else ignore handler
            return this._cachedRequestHandler;
        } else {
            return handler;
        }
    }

    get cachedConnection() {
        if (this._cacheConnection) {
            let handler = this._cachedRequestHandler;
            if (handler !== null) {
                return handler.getConnection();
            }
        }
        return null;
    }

    constructor(reference) {
        this._reference = reference;
        this._cacheConnection = reference.getCacheConnection();
        this._cachedRequestHandler = null;
    }

    clearCachedRequestHandler(handler) {
        if (this._cacheConnection) {
            if (handler == this._cachedRequestHandler) {
                this._cachedRequestHandler = null;
            }
        }
    }

    handleException(ex, handler, mode, sent, cnt) {
        this.clearCachedRequestHandler(handler);

        // We only retry local exception.
        //
        // A CloseConnectionException indicates graceful server shutdown, and is therefore always repeatable without
        // violating "at-most-once". That's because by sending a close connection message, the server guarantees that
        // all outstanding requests can safely be repeated.
        //
        // An ObjectNotExistException can always be retried as well without violating "at-most-once" (see the
        // implementation of the checkRetryAfterException method below for the reasons why it can be useful).
        //
        // If the request didn't get sent or if it's non-mutating or idempotent it can also always be retried if the
        // retry count isn't reached.
        if (
            ex instanceof LocalException &&
            (!sent ||
                mode == OperationMode.Nonmutating ||
                mode == OperationMode.Idempotent ||
                ex instanceof CloseConnectionException ||
                ex instanceof ObjectNotExistException)
        ) {
            return RequestHandlerCache.checkRetryAfterException(ex, this._reference, cnt);
        } else {
            throw ex; // Retry could break at-most-once semantics, don't retry.
        }
    }

    static checkRetryAfterException(ex, ref, cnt) {
        const instance = ref.getInstance();
        const traceLevels = instance.traceLevels();
        const logger = instance.initializationData().logger;

        // We don't retry batch requests because the exception might have caused that all the requests batched with the
        // connection to be aborted and we want the application to be notified.
        if (ref.getMode() == ReferenceMode.ModeBatchOneway) {
            throw ex;
        }

        // If it's a fixed proxy, retrying isn't useful as the proxy is tied to the connection and the request will
        // fail with the exception.
        if (ref.isFixed()) {
            throw ex;
        }

        if (ex instanceof ObjectNotExistException) {
            if (ref.getRouterInfo() != null && ex.operation == "ice_add_proxy") {
                // If we have a router, an ObjectNotExistException with an operation name "ice_add_proxy" indicates to
                // the client that the router isn't aware of the proxy (for example, because it was evicted by the
                // router). In this case, we must *always* retry, so that the missing proxy is added to the router.
                ref.getRouterInfo().clearCache(ref);

                if (traceLevels.retry >= 1) {
                    logger.trace(traceLevels.retryCat, "retrying operation call to add proxy to router\n" + ex);
                }
                // We must always retry, so we don't look at the retry count.
                return 0;
            } else if (ref.isIndirect()) {
                // We retry ObjectNotExistException if the reference is indirect.
                if (ref.isWellKnown()) {
                    const li = ref.getLocatorInfo();
                    if (li !== null) {
                        li.clearCache(ref);
                    }
                }
            } else {
                // For all other cases, we don't retry ObjectNotExistException.
                throw ex;
            }
        } else if (ex instanceof RequestFailedException) {
            throw ex;
        }

        // There is no point in retrying an operation that resulted in a MarshalException. This must have been raised
        // locally (because if it happened in a server it would result in an UnknownLocalException  instead), which
        // means there was a problem in this process that will not change if we try again.
        //
        // The most likely cause for a MarshalException is exceeding the maximum message size, which is represented by
        // the subclass MemoryLimitException. For example, a client can attempt to send a message that exceeds the
        // maximum memory size, or accumulate enough batch requests without flushing before the maximum size is reached.
        //
        // This latter case is especially problematic, because if we were to retry a batch request after a
        // MarshalException, we would in fact silently discard the accumulated requests and allow new batch requests to
        // accumulate. If the subsequent batched requests do not exceed the maximum message size, it appears to the
        // client that all of the batched requests were accepted, when in reality only the last few are actually sent.
        if (ex instanceof MarshalException) {
            throw ex;
        }

        // Don't retry if the communicator is destroyed, object adapter is deactivated, or connection is manually closed.
        if (
            ex instanceof CommunicatorDestroyedException ||
            ex instanceof ObjectAdapterDeactivatedException ||
            ex instanceof ConnectionManuallyClosedException
        ) {
            throw ex;
        }

        // Don't retry invocation timeouts.
        if (ex instanceof InvocationTimeoutException || ex instanceof InvocationCanceledException) {
            throw ex;
        }

        ++cnt.value;
        Debug.assert(cnt.value > 0);

        var retryIntervals = instance._retryIntervals;

        let interval = 0;
        if (cnt.value == retryIntervals.length + 1 && ex instanceof CloseConnectionException) {
            // A close connection exception is always retried at least once, even if the retry limit is reached.
            interval = 0;
        } else if (cnt.value > retryIntervals.length) {
            if (traceLevels.retry >= 1) {
                logger.trace(
                    traceLevels.retryCat,
                    "cannot retry operation call because retry limit has been exceeded\n" + ex,
                );
            }
            throw ex;
        } else {
            interval = retryIntervals[cnt.value - 1];
        }

        if (traceLevels.retry >= 1) {
            let s = "retrying operation call";
            if (interval > 0) {
                s += " in " + interval + "ms";
            }
            s += " because of exception\n" + ex;
            logger.trace(traceLevels.retryCat, s);
        }

        return interval;
    }
}
