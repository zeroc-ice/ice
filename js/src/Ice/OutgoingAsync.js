// Copyright (c) ZeroC, Inc.

import { OutputStream } from "./OutputStream.js";
import { AsyncResult } from "./AsyncResult.js";
import { AsyncStatus } from "./AsyncStatus.js";
import { UserException } from "./UserException.js";
import { RetryException } from "./RetryException.js";
import { ReferenceMode } from "./ReferenceMode.js";
import { Ice as Ice_OperationMode } from "./OperationMode.js";
const { OperationMode } = Ice_OperationMode;
import {
    CloseConnectionException,
    ConnectionClosedException,
    CommunicatorDestroyedException,
    ConnectionAbortedException,
    FacetNotExistException,
    FeatureNotSupportedException,
    InvocationCanceledException,
    InvocationTimeoutException,
    MarshalException,
    ObjectAdapterDestroyedException,
    ObjectNotExistException,
    OperationNotExistException,
    RequestFailedException,
    UnknownException,
    UnknownLocalException,
    UnknownUserException,
} from "./LocalExceptions.js";
import { LocalException } from "./LocalException.js";
import { Ice as Ice_Context } from "./Context.js";
const { ContextHelper } = Ice_Context;
import { Protocol } from "./Protocol.js";
import { Ice as Ice_BuiltinSequences } from "./BuiltinSequences.js";
const { StringSeqHelper } = Ice_BuiltinSequences;
import { InputStream } from "./InputStream.js";

import { Ice as Ice_Identity } from "./Identity.js";
const { Identity } = Ice_Identity;
import { ReplyStatus } from "./ReplyStatus.js";

export class OutgoingAsyncBase extends AsyncResult {
    constructor(communicator, operation, proxy) {
        super(communicator, operation, proxy);
        this._os = new OutputStream(
            Protocol.currentProtocolEncoding,
            this._instance.defaultsAndOverrides().defaultFormat,
        );
    }

    getOs() {
        return this._os;
    }

    sent() {
        this.markSent(true);
    }

    exception(ex) {
        return this.markFinishedEx(ex);
    }
}

export class ProxyOutgoingAsyncBase extends OutgoingAsyncBase {
    constructor(prx, operation) {
        super(prx.ice_getCommunicator(), operation, prx);
        this._mode = null;
        this._cnt = 0;
        this._sent = false;
        this._handler = null;
    }

    completedEx(ex) {
        try {
            const interval = this.handleRetryAfterException(ex);
            this._instance.retryQueue().add(this, interval);
        } catch (ex) {
            this.markFinishedEx(ex);
        }
    }

    retryException() {
        try {
            // It's important to let the retry queue do the retry. This is
            // called from the connect request handler and the retry might
            // require could end up waiting for the flush of the
            // connection to be done.
            this._proxy._requestHandlerCache.clearCachedRequestHandler(this._handler);
            this._instance.retryQueue().add(this, 0);
        } catch (ex) {
            this.completedEx(ex);
        }
    }

    retry() {
        this.invokeImpl(false);
    }

    abort(ex) {
        this.markFinishedEx(ex);
    }

    invokeImpl(userThread) {
        try {
            if (userThread) {
                const invocationTimeout = this._proxy._getReference().getInvocationTimeout();
                if (invocationTimeout > 0) {
                    this._timeoutToken = this._instance.timer().schedule(() => {
                        this.cancelWithException(new InvocationTimeoutException());
                    }, invocationTimeout);
                }
            }

            while (true) {
                try {
                    this._sent = false;
                    this._handler = this._proxy._requestHandlerCache.requestHandler;
                    if ((this._handler.sendAsyncRequest(this) & AsyncStatus.Sent) > 0 && userThread) {
                        this._sentSynchronously = true;
                    }
                    return; // We're done!
                } catch (ex) {
                    if (ex instanceof RetryException) {
                        // Clear request handler and always retry
                        this._proxy._requestHandlerCache.clearCachedRequestHandler(this._handler);
                    } else {
                        const interval = this.handleRetryAfterException(ex);
                        if (interval > 0) {
                            this._instance.retryQueue().add(this, interval);
                            return;
                        }
                    }
                }
            }
        } catch (ex) {
            this.markFinishedEx(ex);
        }
    }

    markSent(done) {
        this._sent = true;
        if (done) {
            if (this._timeoutToken) {
                this._instance.timer().cancel(this._timeoutToken);
            }
        }
        super.markSent.call(this, done);
    }

    markFinishedEx(ex) {
        if (this._timeoutToken) {
            this._instance.timer().cancel(this._timeoutToken);
        }
        super.markFinishedEx.call(this, ex);
    }

    handleRetryAfterException(ex) {
        // Clear the request handler
        this._proxy._requestHandlerCache.clearCachedRequestHandler(this._handler);

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
            (!this._sent ||
                this._mode == OperationMode.Nonmutating ||
                this._mode == OperationMode.Idempotent ||
                ex instanceof CloseConnectionException ||
                ex instanceof ObjectNotExistException)
        ) {
            try {
                return this.checkRetryAfterException(ex);
            } catch (e) {
                if (e instanceof CommunicatorDestroyedException) {
                    e = ex; // The communicator is already destroyed, so we cannot retry.
                }
                throw e;
            }
        } else {
            throw ex; // Retry could break at-most-once semantics, don't retry.
        }
    }

    checkRetryAfterException(ex) {
        const ref = this._proxy._reference;
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
        // A likely cause for a MarshalException is exceeding the maximum message size. For example, a client
        // can attempt to send a message that exceeds the maximum memory size, or accumulate enough batch requests
        // without flushing before the maximum size is reached.
        //
        // This latter case is especially problematic, because if we were to retry a batch request after a
        // MarshalException, we would in fact silently discard the accumulated requests and allow new batch requests to
        // accumulate. If the subsequent batched requests do not exceed the maximum message size, it appears to the
        // client that all of the batched requests were accepted, when in reality only the last few are actually sent.
        if (ex instanceof MarshalException) {
            throw ex;
        }

        // Don't retry if the communicator is destroyed, object adapter is destroyed, or connection is manually closed.
        if (
            ex instanceof CommunicatorDestroyedException ||
            ex instanceof ObjectAdapterDestroyedException ||
            (ex instanceof ConnectionAbortedException && ex.closedByApplication) ||
            (ex instanceof ConnectionClosedException && ex.closedByApplication)
        ) {
            throw ex;
        }

        // Don't retry invocation timeouts.
        if (ex instanceof InvocationTimeoutException || ex instanceof InvocationCanceledException) {
            throw ex;
        }

        ++this._cnt;
        DEV: console.assert(this._cnt > 0);

        const retryIntervals = instance._retryIntervals;

        let interval = 0;
        if (this._cnt == retryIntervals.length + 1 && ex instanceof CloseConnectionException) {
            // A close connection exception is always retried at least once, even if the retry limit is reached.
            interval = 0;
        } else if (this._cnt > retryIntervals.length) {
            if (traceLevels.retry >= 1) {
                logger.trace(
                    traceLevels.retryCat,
                    "cannot retry operation call because retry limit has been exceeded\n" + ex,
                );
            }
            throw ex;
        } else {
            interval = retryIntervals[this._cnt - 1];
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

export class OutgoingAsync extends ProxyOutgoingAsyncBase {
    constructor(prx, operation, completed) {
        super(prx, operation);
        if (prx) {
            this._encoding = Protocol.getCompatibleEncoding(this._proxy._getReference().getEncoding());
            this._completed = completed;
        }
    }

    prepare(op, mode, ctx) {
        const protocol = this._proxy._getReference().getProtocol();
        if (protocol.major != Protocol.currentProtocol.major) {
            throw new FeatureNotSupportedException(
                `Cannot send request using protocol version ${protocol.major}.${protocol.minor}`,
            );
        }

        this._mode = mode;
        if (ctx === null) {
            ctx = OutgoingAsync._emptyContext;
        }

        if (this._proxy.ice_isBatchOneway() || this._proxy.ice_isBatchDatagram()) {
            this._proxy._reference.batchRequestQueue.prepareBatchRequest(this._os);
        } else {
            this._os.writeBlob(Protocol.requestHdr);
        }

        const ref = this._proxy._getReference();

        ref.getIdentity()._write(this._os);

        //
        // For compatibility with the old FacetPath.
        //
        const facet = ref.getFacet();
        if (facet === null || facet.length === 0) {
            StringSeqHelper.write(this._os, null);
        } else {
            StringSeqHelper.write(this._os, [facet]);
        }

        this._os.writeString(this._operation);

        this._os.writeByte(mode.value);

        if (ctx !== undefined) {
            if (ctx !== null && !(ctx instanceof Map)) {
                throw new RangeError("illegal context value, expecting null or Map");
            }

            //
            // Explicit context
            //
            ContextHelper.write(this._os, ctx);
        } else {
            //
            // Implicit context
            //
            const implicitContext = ref.getInstance().getImplicitContext();
            const prxContext = ref.getContext();

            if (implicitContext === null) {
                ContextHelper.write(this._os, prxContext);
            } else {
                implicitContext.write(prxContext, this._os);
            }
        }
    }

    sent() {
        this.markSent(!this._proxy.ice_isTwoway());
    }

    invokeRemote(connection, response) {
        return connection.sendAsyncRequest(this, response, 0);
    }

    abort(ex) {
        if (this._proxy.ice_isBatchOneway() || this._proxy.ice_isBatchDatagram()) {
            this._proxy._reference.batchRequestQueue.abortBatchRequest(this._os);
        }
        super.abort(ex);
    }

    invoke() {
        if (this._proxy.ice_isBatchOneway() || this._proxy.ice_isBatchDatagram()) {
            this._sentSynchronously = true;
            this._proxy._reference.batchRequestQueue.finishBatchRequest(this._os, this._proxy, this._operation);
            this.markFinished(true);
            return;
        }

        //
        // NOTE: invokeImpl doesn't throw so this can be called from the
        // try block with the catch block calling abort() in case of an
        // exception.
        //
        this.invokeImpl(true); // userThread = true
    }

    completed(istr) {
        DEV: console.assert(this._proxy.ice_isTwoway()); // Can only be called for twoways.

        let replyStatus;
        try {
            if (this._is === null) {
                // _is can already be initialized if the invocation is retried
                this._is = new InputStream(this._instance, Protocol.currentProtocolEncoding);
            }
            this._is.swap(istr);
            replyStatus = ReplyStatus.valueOf(this._is.readByte());

            switch (replyStatus) {
                case ReplyStatus.Ok:
                case ReplyStatus.UserException: {
                    break;
                }

                case ReplyStatus.ObjectNotExist:
                case ReplyStatus.FacetNotExist:
                case ReplyStatus.OperationNotExist: {
                    const id = new Identity();
                    id._read(this._is);

                    //
                    // For compatibility with the old facet path.
                    //
                    const facetPath = StringSeqHelper.read(this._is);
                    let facet;
                    if (facetPath.length > 0) {
                        if (facetPath.length > 1) {
                            throw new MarshalException(
                                `Received invalid facet path with ${facetPath.length} elements.`,
                            );
                        }
                        facet = facetPath[0];
                    } else {
                        facet = "";
                    }

                    const operation = this._is.readString();

                    switch (replyStatus) {
                        case ReplyStatus.ObjectNotExist: {
                            throw new ObjectNotExistException(id, facet, operation);
                        }

                        case ReplyStatus.FacetNotExist: {
                            throw new FacetNotExistException(id, facet, operation);
                        }

                        case ReplyStatus.OperationNotExist: {
                            throw new OperationNotExistException(id, facet, operation);
                        }
                    }
                }

                case ReplyStatus.UnknownException:
                case ReplyStatus.UnknownLocalException:
                case ReplyStatus.UnknownUserException: {
                    const unknown = this._is.readString();

                    let ue = null;
                    switch (replyStatus) {
                        case ReplyStatus.UnknownException: {
                            ue = new UnknownException(unknown);
                            break;
                        }

                        case ReplyStatus.UnknownLocalException: {
                            ue = new UnknownLocalException(unknown);
                            break;
                        }

                        case ReplyStatus.UnknownUserException: {
                            ue = new UnknownUserException(unknown);
                            break;
                        }

                        default: {
                            DEV: console.assert(false);
                            break;
                        }
                    }
                    throw ue;
                }

                default: {
                    throw new MarshalException(`Received reply message with unknown reply status ${replyStatus}.`);
                }
            }

            this.markFinished(replyStatus == ReplyStatus.Ok, this._completed);
        } catch (ex) {
            this.completedEx(ex);
        }
    }

    startWriteParams(format) {
        this._os.startEncapsulation(this._encoding, format);
        return this._os;
    }

    endWriteParams() {
        this._os.endEncapsulation();
    }

    writeEmptyParams() {
        this._os.writeEmptyEncapsulation(this._encoding);
    }

    startReadParams() {
        this._is.startEncapsulation();
        return this._is;
    }

    endReadParams() {
        this._is.endEncapsulation();
    }

    readEmptyParams() {
        this._is.skipEmptyEncapsulation();
    }

    throwUserException() {
        DEV: console.assert((this._state & AsyncResult.Done) !== 0);
        if ((this._state & AsyncResult.Ok) === 0) {
            try {
                this._is.startEncapsulation();
                this._is.throwException();
            } catch (ex) {
                if (ex instanceof UserException) {
                    this._is.endEncapsulation();
                }
                throw ex;
            }
        }
    }
}

// TODO: We should use an immutable map.
OutgoingAsync._emptyContext = new Map(); // Map<string, string>

export class ProxyFlushBatch extends ProxyOutgoingAsyncBase {
    constructor(prx, operation) {
        super(prx, operation);
        this._batchRequestNum = prx._reference.batchRequestQueue.swap(this._os);
    }

    invokeRemote(connection, response) {
        if (this._batchRequestNum === 0) {
            this.sent();
            return AsyncStatus.Sent;
        }
        return connection.sendAsyncRequest(this, response, this._batchRequestNum);
    }

    invoke() {
        const protocol = this._proxy._getReference().getProtocol();
        if (protocol.major != Protocol.currentProtocol.major) {
            throw new FeatureNotSupportedException(
                `Cannot send request using protocol version ${protocol.major}.${protocol.minor}`,
            );
        }
        this.invokeImpl(true); // userThread = true
    }
}

export class ProxyGetConnection extends ProxyOutgoingAsyncBase {
    invokeRemote(connection, response) {
        this.markFinished(true, r => r.resolve(connection));
        return AsyncStatus.Sent;
    }

    invoke() {
        this.invokeImpl(true); // userThread = true
    }
}

export class ConnectionFlushBatch extends OutgoingAsyncBase {
    constructor(connection, communicator, operation) {
        super(communicator, operation);
        this._connection = connection;
    }

    invoke() {
        try {
            const batchRequestNum = this._connection.getBatchRequestQueue().swap(this._os);
            let status;
            if (batchRequestNum === 0) {
                this.sent();
                status = AsyncStatus.Sent;
            } else {
                status = this._connection.sendAsyncRequest(this, false, batchRequestNum);
            }

            if ((status & AsyncStatus.Sent) > 0) {
                this._sentSynchronously = true;
            }
        } catch (ex) {
            this.markFinishedEx(ex);
        }
    }
}
