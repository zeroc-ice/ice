//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import { OutputStream } from "./Stream.js";
import { AsyncResult } from "./AsyncResult.js";
import { AsyncStatus } from "./AsyncStatus.js";
import { UserException } from "./Exception.js";
import { RetryException } from "./RetryException.js";
import {
    InvocationTimeoutException,
    MarshalException,
    ObjectNotExistException,
    FacetNotExistException,
    OperationNotExistException,
    UnknownException,
    UnknownLocalException,
    UnknownUserException,
    UnknownReplyStatusException,
} from "./LocalException.js";
import { Ice as Ice_Context } from "./Context.js";
const { ContextHelper } = Ice_Context;
import { Protocol } from "./Protocol.js";
import { Ice as Ice_BuiltinSequences } from "./BuiltinSequences.js";
const { StringSeqHelper } = Ice_BuiltinSequences;
import { InputStream } from "./Stream.js";

import { Ice as Ice_Identity } from "./Identity.js";
const { Identity } = Ice_Identity;
import { Debug } from "./Debug.js";

export class OutgoingAsyncBase extends AsyncResult {
    constructor(communicator, operation, connection, proxy, adapter) {
        super(communicator, operation, connection, proxy, adapter);
        this._os = new OutputStream(this._instance, Protocol.currentProtocolEncoding);
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
        if (prx) {
            super(prx.ice_getCommunicator(), operation, null, prx, null);
        } else {
            super();
        }
        this._mode = null;
        this._cnt = 0;
        this._sent = false;
        this._handler = null;
    }

    completedEx(ex) {
        try {
            const cnt = { value: this._cnt };
            this._instance
                .retryQueue()
                .add(
                    this,
                    this._proxy._requestHandlerCache.handleException(ex, this._handler, this._mode, this._sent, cnt),
                );
            this._cnt = cnt.value;
        } catch (ex) {
            this.markFinishedEx(ex);
        }
    }

    retryException(ex) {
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
                        const interval = this.handleException(ex);
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

    handleException(ex) {
        const cnt = { value: this._cnt };
        const interval = this._proxy._requestHandlerCache.handleException(
            ex,
            this._handler,
            this._mode,
            this._sent,
            cnt,
        );
        this._cnt = cnt.value;
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
        Protocol.checkSupportedProtocol(Protocol.getCompatibleProtocol(this._proxy._getReference().getProtocol()));

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
        Debug.assert(this._proxy.ice_isTwoway()); // Can only be called for twoways.

        let replyStatus;
        try {
            if (this._is === null) {
                // _is can already be initialized if the invocation is retried
                this._is = new InputStream(this._instance, Protocol.currentProtocolEncoding);
            }
            this._is.swap(istr);
            replyStatus = this._is.readByte();

            switch (replyStatus) {
                case Protocol.replyOK:
                case Protocol.replyUserException: {
                    break;
                }

                case Protocol.replyObjectNotExist:
                case Protocol.replyFacetNotExist:
                case Protocol.replyOperationNotExist: {
                    const id = new Identity();
                    id._read(this._is);

                    //
                    // For compatibility with the old FacetPath.
                    //
                    const facetPath = StringSeqHelper.read(this._is);
                    let facet;
                    if (facetPath.length > 0) {
                        if (facetPath.length > 1) {
                            throw new MarshalException();
                        }
                        facet = facetPath[0];
                    } else {
                        facet = "";
                    }

                    const operation = this._is.readString();

                    let rfe = null;
                    switch (replyStatus) {
                        case Protocol.replyObjectNotExist: {
                            rfe = new ObjectNotExistException();
                            break;
                        }

                        case Protocol.replyFacetNotExist: {
                            rfe = new FacetNotExistException();
                            break;
                        }

                        case Protocol.replyOperationNotExist: {
                            rfe = new OperationNotExistException();
                            break;
                        }

                        default: {
                            Debug.assert(false);
                            break;
                        }
                    }

                    rfe.id = id;
                    rfe.facet = facet;
                    rfe.operation = operation;
                    throw rfe;
                }

                case Protocol.replyUnknownException:
                case Protocol.replyUnknownLocalException:
                case Protocol.replyUnknownUserException: {
                    const unknown = this._is.readString();

                    let ue = null;
                    switch (replyStatus) {
                        case Protocol.replyUnknownException: {
                            ue = new UnknownException();
                            break;
                        }

                        case Protocol.replyUnknownLocalException: {
                            ue = new UnknownLocalException();
                            break;
                        }

                        case Protocol.replyUnknownUserException: {
                            ue = new UnknownUserException();
                            break;
                        }

                        default: {
                            Debug.assert(false);
                            break;
                        }
                    }

                    ue.unknown = unknown;
                    throw ue;
                }

                default: {
                    throw new UnknownReplyStatusException();
                }
            }

            this.markFinished(replyStatus == Protocol.replyOK, this._completed);
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
        Debug.assert((this._state & AsyncResult.Done) !== 0);
        if ((this._state & AsyncResult.OK) === 0) {
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
        Protocol.checkSupportedProtocol(Protocol.getCompatibleProtocol(this._proxy._getReference().getProtocol()));
        this.invokeImpl(true); // userThread = true
    }
}

export class ProxyGetConnection extends ProxyOutgoingAsyncBase {
    invokeRemote(connection, response) {
        this.markFinished(true, (r) => r.resolve(connection));
        return AsyncStatus.Sent;
    }

    invoke() {
        this.invokeImpl(true); // userThread = true
    }
}

export class ConnectionFlushBatch extends OutgoingAsyncBase {
    constructor(con, communicator, operation) {
        super(communicator, operation, con, null, null);
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
            this.completedEx(ex);
        }
    }
}

export class HeartbeatAsync extends OutgoingAsyncBase {
    constructor(con, communicator) {
        super(communicator, "heartbeat", con, null, null);
    }

    invoke() {
        try {
            this._os.writeBlob(Protocol.magic);
            Protocol.currentProtocol._write(this._os);
            Protocol.currentProtocolEncoding._write(this._os);
            this._os.writeByte(Protocol.validateConnectionMsg);
            this._os.writeByte(0);
            this._os.writeInt(Protocol.headerSize); // Message size.

            const status = this._connection.sendAsyncRequest(this, false, 0);
            if ((status & AsyncStatus.Sent) > 0) {
                this._sentSynchronously = true;
            }
        } catch (ex) {
            this.completedEx(ex);
        }
    }
}
