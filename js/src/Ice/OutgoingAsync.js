// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_FOR_ACTIONSCRIPT_LICENSE file included in this distribution.
//
// **********************************************************************

const Ice = require("../Ice/ModuleRegistry").Ice;
Ice.__M.require(module,
    [
        "../Ice/AsyncStatus",
        "../Ice/AsyncResult",
        "../Ice/Stream",
        "../Ice/Debug",
        "../Ice/RetryException",
        "../Ice/Current",
        "../Ice/Protocol",
        "../Ice/BuiltinSequences",
        "../Ice/Exception",
        "../Ice/LocalException",
        "../Ice/Identity"
    ]);

const AsyncStatus = Ice.AsyncStatus;
const AsyncResult = Ice.AsyncResult;
const InputStream = Ice.InputStream;
const OutputStream = Ice.OutputStream;
const Debug = Ice.Debug;
const RetryException = Ice.RetryException;
const OperationMode = Ice.OperationMode;
const Protocol = Ice.Protocol;
const Identity = Ice.Identity;

class OutgoingAsyncBase extends AsyncResult
{
    constructor(communicator, operation, connection, proxy, adapter)
    {
        super(communicator, operation, connection, proxy, adapter);
        this._os = new OutputStream(this._instance, Protocol.currentProtocolEncoding);
    }

    __os()
    {
        return this._os;
    }

    __sent()
    {
        this.__markSent(true);
    }

    __completedEx(ex)
    {
        this.__markFinishedEx(ex);
    }
}

class ProxyOutgoingAsyncBase extends OutgoingAsyncBase
{
    constructor(prx, operation)
    {
        super(prx.ice_getCommunicator(), operation, null, prx, null);
        this._mode = null;
        this._cnt = 0;
        this._sent = false;
        this._handler = null;
    }

    __completedEx(ex)
    {
        try
        {
            this._instance.retryQueue().add(this, this.__handleException(ex));
        }
        catch(ex)
        {
            this.__markFinishedEx(ex);
        }
    }

    __retryException(ex)
    {
        try
        {
            this._proxy.__updateRequestHandler(this._handler, null); // Clear request handler and always retry.
            this._instance.retryQueue().add(this, 0);
        }
        catch(ex)
        {
            this.__completedEx(ex);
        }
    }

    __retry()
    {
        this.__invokeImpl(false);
    }

    __abort(ex)
    {
        this.__markFinishedEx(ex);
    }
    
    __invokeImpl(userThread)
    {
        try
        {
            if(userThread)
            {
                const invocationTimeout = this._proxy.__reference().getInvocationTimeout();
                if(invocationTimeout > 0)
                {
                    this._timeoutToken = this._instance.timer().schedule(
                        () =>
                        {
                            this.__cancel(new Ice.InvocationTimeoutException());
                        },
                        invocationTimeout);
                }
            }

            while(true)
            {
                try
                {
                    this._sent  = false;
                    this._handler = this._proxy.__getRequestHandler();
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
                        this._proxy.__updateRequestHandler(this._handler, null);
                    }
                    else
                    {
                        const interval = this.__handleException(ex);
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
            this.__markFinishedEx(ex);
        }
    }

    __markSent(done)
    {
        this._sent = true;
        if(done)
        {
            if(this._timeoutToken)
            {
                this._instance.timer().cancel(this._timeoutToken);
            }
        }
        super.__markSent.call(this, done);
    }

    __markFinishedEx(ex)
    {
        if(this._timeoutToken)
        {
            this._instance.timer().cancel(this._timeoutToken);
        }
        super.__markFinishedEx.call(this, ex);
    }
    
    __handleException(ex)
    {
        const interval = { value: 0 };
        this._cnt = this._proxy.__handleException(ex, this._handler, this._mode, this._sent, interval, this._cnt);
        return interval.value;
    }
}

class OutgoingAsync extends ProxyOutgoingAsyncBase
{
    constructor(prx, operation, completed)
    {
        super(prx, operation);
        this._encoding = Protocol.getCompatibleEncoding(this._proxy.__reference().getEncoding());
        this._completed = completed;
    }

    __prepare(op, mode, ctx)
    {
        Protocol.checkSupportedProtocol(Protocol.getCompatibleProtocol(this._proxy.__reference().getProtocol()));

        this._mode = mode;
        if(ctx === null)
        {
            ctx = OutgoingAsync._emptyContext;
        }

        if(this._proxy.ice_isBatchOneway() || this._proxy.ice_isBatchDatagram())
        {
            this._proxy.__getBatchRequestQueue().prepareBatchRequest(this._os);
        }
        else
        {
            this._os.writeBlob(Protocol.requestHdr);
        }

        const ref = this._proxy.__reference();

        ref.getIdentity().__write(this._os);

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
            Ice.StringSeqHelper.write(this._os, [ facet ]);
        }

        this._os.writeString(this._operation);

        this._os.writeByte(mode.value);

        if(ctx !== undefined)
        {
            if(ctx !== null && !(ctx instanceof Map))
            {
                throw new Error("illegal context value, expecting null or Map");
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

    __sent()
    {
        this.__markSent(!this._proxy.ice_isTwoway());
    }

    __invokeRemote(connection, compress, response)
    {
        return connection.sendAsyncRequest(this, compress, response, 0);
    }

    __abort(ex)
    {
        if(this._proxy.ice_isBatchOneway() || this._proxy.ice_isBatchDatagram())
        {
            this._proxy.__getBatchRequestQueue().abortBatchRequest(this._os);
        }
        super.__abort(ex);
    }

    __invoke()
    {
        if(this._proxy.ice_isBatchOneway() || this._proxy.ice_isBatchDatagram())
        {
            this._sentSynchronously = true;
            this._proxy.__getBatchRequestQueue().finishBatchRequest(this._os, this._proxy, this._operation);
            this.__markFinished(true);
            return;
        }

        //
        // NOTE: invokeImpl doesn't throw so this can be called from the
        // try block with the catch block calling abort() in case of an
        // exception.
        //
        this.__invokeImpl(true); // userThread = true
    }

    __completed(istr)
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
                    id.__read(this._is);

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

            this.__markFinished(replyStatus == Protocol.replyOK, this._completed);
        }
        catch(ex)
        {
            this.__completedEx(ex);
        }
    }

    __startWriteParams(format)
    {
        this._os.startEncapsulation(this._encoding, format);
        return this._os;
    }

    __endWriteParams()
    {
        this._os.endEncapsulation();
    }

    __writeEmptyParams()
    {
        this._os.writeEmptyEncapsulation(this._encoding);
    }

    __writeParamEncaps(encaps)
    {
        if(encaps === null || encaps.length === 0)
        {
            this._os.writeEmptyEncapsulation(this._encoding);
        }
        else
        {
            this._os.writeEncapsulation(encaps);
        }
    }

    __is()
    {
        return this._is;
    }

    __startReadParams()
    {
        this._is.startEncapsulation();
        return this._is;
    }

    __endReadParams()
    {
        this._is.endEncapsulation();
    }

    __readEmptyParams()
    {
        this._is.skipEmptyEncapsulation();
    }

    __readParamEncaps()
    {
        return this._is.readEncapsulation(null);
    }

    __throwUserException()
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
        this._batchRequestNum = prx.__getBatchRequestQueue().swap(this._os);
    }

    __invokeRemote(connection, compress, response)
    {
        if(this._batchRequestNum === 0)
        {
            this.__sent();
            return AsyncStatus.Sent;
        }
        return connection.sendAsyncRequest(this, compress, response, this._batchRequestNum);
    }

    __invoke()
    {
        Protocol.checkSupportedProtocol(Protocol.getCompatibleProtocol(this._proxy.__reference().getProtocol()));
        this.__invokeImpl(true); // userThread = true
    }
}

class ProxyGetConnection extends ProxyOutgoingAsyncBase
{
    constructor(prx, operation)
    {
        super(prx, operation);
    }

    __invokeRemote(connection, compress, response)
    {
        this.__markFinished(true, r => r.resolve(connection));
        return AsyncStatus.Sent;
    }
    
    __invoke()
    {
        this.__invokeImpl(true); // userThread = true
    }
}

class ConnectionFlushBatch extends OutgoingAsyncBase
{
    constructor(con, communicator, operation)
    {
        super(communicator, operation, con, null, null);
    }

    __invoke()
    {
        try
        {
            const batchRequestNum = this._connection.getBatchRequestQueue().swap(this._os);
            let status;
            if(batchRequestNum === 0)
            {
                this.__sent();
                status = AsyncStatus.Sent;
            }
            else
            {
                status = this._connection.sendAsyncRequest(this, false, false, batchRequestNum);
            }

            if((status & AsyncStatus.Sent) > 0)
            {
                this._sentSynchronously = true;
            }
        }
        catch(ex)
        {
            this.__completedEx(ex);
        }
    }
}

Ice.OutgoingAsync = OutgoingAsync;
Ice.ProxyFlushBatch = ProxyFlushBatch;
Ice.ProxyGetConnection = ProxyGetConnection;
Ice.ConnectionFlushBatch = ConnectionFlushBatch;

module.exports.Ice = Ice;
