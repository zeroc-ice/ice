//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

const Ice = require("../Ice/ModuleRegistry").Ice;

require("../Ice/AsyncResult");
require("../Ice/AsyncStatus");
require("../Ice/BuiltinSequences");
require("../Ice/Current");
require("../Ice/Debug");
require("../Ice/Exception");
require("../Ice/Identity");
require("../Ice/LocalException");
require("../Ice/Protocol");
require("../Ice/RetryException");
require("../Ice/Stream");

const AsyncResult = Ice.AsyncResult;
const AsyncStatus = Ice.AsyncStatus;
const Debug = Ice.Debug;
const Identity = Ice.Identity;
const InputStream = Ice.InputStream;
const OutputStream = Ice.OutputStream;
const Protocol = Ice.Protocol;
const RetryException = Ice.RetryException;

class OutgoingAsyncBase extends AsyncResult
{
    constructor(communicator, operation, connection, proxy, adapter)
    {
        super(communicator, operation, connection, proxy, adapter);
        this._os = new OutputStream(this._instance, Protocol.currentProtocolEncoding);
    }

    getOs()
    {
        return this._os;
    }

    sent()
    {
        this.markSent(true);
    }

    completedEx(ex)
    {
        this.markFinishedEx(ex);
    }
}

class ProxyOutgoingAsyncBase extends OutgoingAsyncBase
{
    constructor(prx, operation)
    {
        if (prx)
        {
            super(prx.ice_getCommunicator(), operation, null, prx, null);
        }
        else
        {
            super();
        }
        this._mode = null;
        this._cnt = 0;
        this._sent = false;
        this._handler = null;
    }

    completedEx(ex)
    {
        try
        {
            this._instance.retryQueue().add(this, this.handleException(ex));
        }
        catch(ex)
        {
            this.markFinishedEx(ex);
        }
    }

    retryException(ex)
    {
        try
        {
            this._proxy._updateRequestHandler(this._handler, null); // Clear request handler and always retry.
            this._instance.retryQueue().add(this, 0);
        }
        catch(ex)
        {
            this.completedEx(ex);
        }
    }

    retry()
    {
        this.invokeImpl(false);
    }

    abort(ex)
    {
        this.markFinishedEx(ex);
    }

    invokeImpl(userThread)
    {
        try
        {
            if(userThread)
            {
                const invocationTimeout = this._proxy._getReference().getInvocationTimeout();
                if(invocationTimeout > 0)
                {
                    this._timeoutToken = this._instance.timer().schedule(
                        () =>
                        {
                            this.cancelWithException(new Ice.InvocationTimeoutException());
                        },
                        invocationTimeout);
                }
            }

            while(true)
            {
                try
                {
                    this._sent = false;
                    this._handler = this._proxy._getRequestHandler();
                    if((this._handler.sendAsyncRequest(this) & AsyncStatus.Sent) > 0)
                    {
                        if(userThread)
                        {
                            this._sentSynchronously = true;
                        }
                    }
                    return; // We're done!
                }
                catch(ex)
                {
                    if(ex instanceof RetryException)
                    {
                        // Clear request handler and always retry
                        this._proxy._updateRequestHandler(this._handler, null);
                    }
                    else
                    {
                        const interval = this.handleException(ex);
                        if(interval > 0)
                        {
                            this._instance.retryQueue().add(this, interval);
                            return;
                        }
                    }
                }
            }
        }
        catch(ex)
        {
            this.markFinishedEx(ex);
        }
    }

    markSent(done)
    {
        this._sent = true;
        if(done)
        {
            if(this._timeoutToken)
            {
                this._instance.timer().cancel(this._timeoutToken);
            }
        }
        super.markSent.call(this, done);
    }

    markFinishedEx(ex)
    {
        if(this._timeoutToken)
        {
            this._instance.timer().cancel(this._timeoutToken);
        }
        super.markFinishedEx.call(this, ex);
    }

    handleException(ex)
    {
        const interval = {value: 0};
        this._cnt = this._proxy._handleException(ex, this._handler, this._mode, this._sent, interval, this._cnt);
        return interval.value;
    }
}

class OutgoingAsync extends ProxyOutgoingAsyncBase
{
    constructor(prx, operation, completed)
    {
        super(prx, operation);
        if (prx)
        {
            this._encoding = Protocol.getCompatibleEncoding(this._proxy._getReference().getEncoding());
            this._completed = completed;
        }
    }

    prepare(op, mode, ctx)
    {
        Protocol.checkSupportedProtocol(Protocol.getCompatibleProtocol(this._proxy._getReference().getProtocol()));

        this._mode = mode;
        if(ctx === null)
        {
            ctx = OutgoingAsync._emptyContext;
        }

        if(this._proxy.ice_isBatchOneway() || this._proxy.ice_isBatchDatagram())
        {
            this._proxy._getBatchRequestQueue().prepareBatchRequest(this._os);
        }
        else
        {
            this._os.writeBlob(Protocol.requestHdr);
        }

        const ref = this._proxy._getReference();

        ref.getIdentity()._write(this._os);

        //
        // For compatibility with the old FacetPath.
        //
        const facet = ref.getFacet();
        if(facet === null || facet.length === 0)
        {
            Ice.StringSeqHelper.write(this._os, null);
        }
        else
        {
            Ice.StringSeqHelper.write(this._os, [facet]);
        }

        this._os.writeString(this._operation);

        this._os.writeByte(mode.value);

        if(ctx !== undefined)
        {
            if(ctx !== null && !(ctx instanceof Map))
            {
                throw new RangeError("illegal context value, expecting null or Map");
            }

            //
            // Explicit context
            //
            Ice.ContextHelper.write(this._os, ctx);
        }
        else
        {
            //
            // Implicit context
            //
            const implicitContext = ref.getInstance().getImplicitContext();
            const prxContext = ref.getContext();

            if(implicitContext === null)
            {
                Ice.ContextHelper.write(this._os, prxContext);
            }
            else
            {
                implicitContext.write(prxContext, this._os);
            }
        }
    }

    sent()
    {
        this.markSent(!this._proxy.ice_isTwoway());
    }

    invokeRemote(connection, response)
    {
        return connection.sendAsyncRequest(this, response, 0);
    }

    abort(ex)
    {
        if(this._proxy.ice_isBatchOneway() || this._proxy.ice_isBatchDatagram())
        {
            this._proxy._getBatchRequestQueue().abortBatchRequest(this._os);
        }
        super.abort(ex);
    }

    invoke()
    {
        if(this._proxy.ice_isBatchOneway() || this._proxy.ice_isBatchDatagram())
        {
            this._sentSynchronously = true;
            this._proxy._getBatchRequestQueue().finishBatchRequest(this._os, this._proxy, this._operation);
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

    completed(istr)
    {
        Debug.assert(this._proxy.ice_isTwoway()); // Can only be called for twoways.

        let replyStatus;
        try
        {
            if(this._is === null) // _is can already be initialized if the invocation is retried
            {
                this._is = new InputStream(this._instance, Protocol.currentProtocolEncoding);
            }
            this._is.swap(istr);
            replyStatus = this._is.readByte();

            switch(replyStatus)
            {
                case Protocol.replyOK:
                case Protocol.replyUserException:
                {
                    break;
                }

                case Protocol.replyObjectNotExist:
                case Protocol.replyFacetNotExist:
                case Protocol.replyOperationNotExist:
                {
                    const id = new Identity();
                    id._read(this._is);

                    //
                    // For compatibility with the old FacetPath.
                    //
                    const facetPath = Ice.StringSeqHelper.read(this._is);
                    let facet;
                    if(facetPath.length > 0)
                    {
                        if(facetPath.length > 1)
                        {
                            throw new Ice.MarshalException();
                        }
                        facet = facetPath[0];
                    }
                    else
                    {
                        facet = "";
                    }

                    const operation = this._is.readString();

                    let rfe = null;
                    switch(replyStatus)
                    {
                    case Protocol.replyObjectNotExist:
                    {
                        rfe = new Ice.ObjectNotExistException();
                        break;
                    }

                    case Protocol.replyFacetNotExist:
                    {
                        rfe = new Ice.FacetNotExistException();
                        break;
                    }

                    case Protocol.replyOperationNotExist:
                    {
                        rfe = new Ice.OperationNotExistException();
                        break;
                    }

                    default:
                    {
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
                case Protocol.replyUnknownUserException:
                {
                    const unknown = this._is.readString();

                    let ue = null;
                    switch(replyStatus)
                    {
                    case Protocol.replyUnknownException:
                    {
                        ue = new Ice.UnknownException();
                        break;
                    }

                    case Protocol.replyUnknownLocalException:
                    {
                        ue = new Ice.UnknownLocalException();
                        break;
                    }

                    case Protocol.replyUnknownUserException:
                    {
                        ue = new Ice.UnknownUserException();
                        break;
                    }

                    default:
                    {
                        Debug.assert(false);
                        break;
                    }
                    }

                    ue.unknown = unknown;
                    throw ue;
                }

                default:
                {
                    throw new Ice.UnknownReplyStatusException();
                }
            }

            this.markFinished(replyStatus == Protocol.replyOK, this._completed);
        }
        catch(ex)
        {
            this.completedEx(ex);
        }
    }

    startWriteParams(format)
    {
        this._os.startEncapsulation(this._encoding, format);
        return this._os;
    }

    endWriteParams()
    {
        this._os.endEncapsulation();
    }

    writeEmptyParams()
    {
        this._os.writeEmptyEncapsulation(this._encoding);
    }

    startReadParams()
    {
        this._is.startEncapsulation();
        return this._is;
    }

    endReadParams()
    {
        this._is.endEncapsulation();
    }

    readEmptyParams()
    {
        this._is.skipEmptyEncapsulation();
    }

    throwUserException()
    {
        Debug.assert((this._state & AsyncResult.Done) !== 0);
        if((this._state & AsyncResult.OK) === 0)
        {
            try
            {
                this._is.startEncapsulation();
                this._is.throwException();
            }
            catch(ex)
            {
                if(ex instanceof Ice.UserException)
                {
                    this._is.endEncapsulation();
                }
                throw ex;
            }
        }
    }
}

OutgoingAsync._emptyContext = new Map(); // Map<string, string>

class ProxyFlushBatch extends ProxyOutgoingAsyncBase
{
    constructor(prx, operation)
    {
        super(prx, operation);
        this._batchRequestNum = prx._getBatchRequestQueue().swap(this._os);
    }

    invokeRemote(connection, response)
    {
        if(this._batchRequestNum === 0)
        {
            this.sent();
            return AsyncStatus.Sent;
        }
        return connection.sendAsyncRequest(this, response, this._batchRequestNum);
    }

    invoke()
    {
        Protocol.checkSupportedProtocol(Protocol.getCompatibleProtocol(this._proxy._getReference().getProtocol()));
        this.invokeImpl(true); // userThread = true
    }
}

class ProxyGetConnection extends ProxyOutgoingAsyncBase
{
    invokeRemote(connection, response)
    {
        this.markFinished(true, r => r.resolve(connection));
        return AsyncStatus.Sent;
    }

    invoke()
    {
        this.invokeImpl(true); // userThread = true
    }
}

class ConnectionFlushBatch extends OutgoingAsyncBase
{
    constructor(con, communicator, operation)
    {
        super(communicator, operation, con, null, null);
    }

    invoke()
    {
        try
        {
            const batchRequestNum = this._connection.getBatchRequestQueue().swap(this._os);
            let status;
            if(batchRequestNum === 0)
            {
                this.sent();
                status = AsyncStatus.Sent;
            }
            else
            {
                status = this._connection.sendAsyncRequest(this, false, batchRequestNum);
            }

            if((status & AsyncStatus.Sent) > 0)
            {
                this._sentSynchronously = true;
            }
        }
        catch(ex)
        {
            this.completedEx(ex);
        }
    }
}

class HeartbeatAsync extends OutgoingAsyncBase
{
    constructor(con, communicator)
    {
        super(communicator, "heartbeat", con, null, null);
    }

    invoke()
    {
        try
        {
            this._os.writeBlob(Protocol.magic);
            Protocol.currentProtocol._write(this._os);
            Protocol.currentProtocolEncoding._write(this._os);
            this._os.writeByte(Protocol.validateConnectionMsg);
            this._os.writeByte(0);
            this._os.writeInt(Protocol.headerSize); // Message size.

            const status = this._connection.sendAsyncRequest(this, false, 0);
            if((status & AsyncStatus.Sent) > 0)
            {
                this._sentSynchronously = true;
            }
        }
        catch(ex)
        {
            this.completedEx(ex);
        }
    }
}

Ice.OutgoingAsync = OutgoingAsync;
Ice.ProxyFlushBatch = ProxyFlushBatch;
Ice.ProxyGetConnection = ProxyGetConnection;
Ice.ConnectionFlushBatch = ConnectionFlushBatch;
Ice.HeartbeatAsync = HeartbeatAsync;

module.exports.Ice = Ice;
