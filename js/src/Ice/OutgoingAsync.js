// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_FOR_ACTIONSCRIPT_LICENSE file included in this distribution.
//
// **********************************************************************

var Ice = require("../Ice/ModuleRegistry").Ice;
Ice.__M.require(module,
    [
        "../Ice/Class",
        "../Ice/AsyncStatus",
        "../Ice/AsyncResult",
        "../Ice/Stream",
        "../Ice/Debug",
        "../Ice/HashMap",
        "../Ice/RetryException",
        "../Ice/Current",
        "../Ice/Protocol",
        "../Ice/BuiltinSequences",
        "../Ice/Exception",
        "../Ice/LocalException",
        "../Ice/Identity"
    ]);

var AsyncStatus = Ice.AsyncStatus;
var AsyncResult = Ice.AsyncResult;
var InputStream = Ice.InputStream;
var OutputStream = Ice.OutputStream;
var Debug = Ice.Debug;
var HashMap = Ice.HashMap;
var RetryException = Ice.RetryException;
var OperationMode = Ice.OperationMode;
var Protocol = Ice.Protocol;
var Identity = Ice.Identity;

var OutgoingAsyncBase = Ice.Class(AsyncResult, {
    __init__ : function(communicator, operation, connection, proxy, adapter)
    {
        if(communicator !== undefined)
        {
            AsyncResult.call(this, communicator, operation, connection, proxy, adapter);
            this._os = new OutputStream(this._instance, Protocol.currentProtocolEncoding);
        }
        else
        {
            AsyncResult.call(this);
        }
    },
    __os: function()
    {
        return this._os;
    },
    __sent: function()
    {
        this.__markSent(true);
    },
    __completedEx: function(ex)
    {
        this.__markFinishedEx(ex);
    }
});


var ProxyOutgoingAsyncBase = Ice.Class(OutgoingAsyncBase, {
    __init__ : function(prx, operation)
    {
        if(prx !== undefined)
        {
            OutgoingAsyncBase.call(this, prx.ice_getCommunicator(), operation, null, prx, null);
            this._mode = null;
            this._cnt = 0;
            this._sent = false;
            this._handler = null;
        }
        else
        {
            AsyncResult.call(this);
        }
    },
    __completedEx: function(ex)
    {
        try
        {
            this._instance.retryQueue().add(this, this.__handleException(ex));
        }
        catch(ex)
        {
            this.__markFinishedEx(ex);
        }
    },
    __retryException: function(ex)
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
    },
    __retry: function()
    {
        this.__invokeImpl(false);
    },
    __abort: function(ex)
    {
        this.__markFinishedEx(ex);
    },
    __invokeImpl: function(userThread)
    {
        try
        {
            if(userThread)
            {
                var invocationTimeout = this._proxy.__reference().getInvocationTimeout();
                if(invocationTimeout > 0)
                {
                    var self = this;
                    this._timeoutToken = this._instance.timer().schedule(
                        function()
                        {
                            self.__cancel(new Ice.InvocationTimeoutException());
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
                    var status = this._handler.sendAsyncRequest(this);
                    if((status & AsyncStatus.Sent) > 0)
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
                        var interval = this.__handleException(ex);
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
    },
    __markSent: function(done)
    {
        this._sent = true;
        if(done)
        {
            if(this._timeoutToken)
            {
                this._instance.timer().cancel(this._timeoutToken);
            }
        }
        OutgoingAsyncBase.prototype.__markSent.call(this, done);
    },
    __markFinishedEx: function(ex)
    {
        if(this._timeoutToken)
        {
            this._instance.timer().cancel(this._timeoutToken);
        }
        OutgoingAsyncBase.prototype.__markFinishedEx.call(this, ex);
    },
    __handleException: function(ex)
    {
        var interval = { value: 0 };
        this._cnt = this._proxy.__handleException(ex, this._handler, this._mode, this._sent, interval, this._cnt);
        return interval.value;
    }
});

var OutgoingAsync = Ice.Class(ProxyOutgoingAsyncBase, {
    __init__: function(prx, operation, completed)
    {
        //
        // OutgoingAsync can be constructed by a sub-type's prototype, in which case the
        // arguments are undefined.
        //
        if(prx !== undefined)
        {
            ProxyOutgoingAsyncBase.call(this, prx, operation);
            this._encoding = Protocol.getCompatibleEncoding(this._proxy.__reference().getEncoding());
            this._completed = completed;
        }
        else
        {
            ProxyOutgoingAsyncBase.call(this);
        }
    },
    __prepare: function(op, mode, ctx)
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

        var ref = this._proxy.__reference();

        ref.getIdentity().__write(this._os);

        //
        // For compatibility with the old FacetPath.
        //
        var facet = ref.getFacet();
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
            if(ctx !== null && !(ctx instanceof HashMap))
            {
                throw new Error("illegal context value, expecting null or HashMap");
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
            var implicitContext = ref.getInstance().getImplicitContext();
            var prxContext = ref.getContext();

            if(implicitContext === null)
            {
                Ice.ContextHelper.write(this._os, prxContext);
            }
            else
            {
                implicitContext.write(prxContext, this._os);
            }
        }
    },
    __sent: function()
    {
        this.__markSent(!this._proxy.ice_isTwoway());
    },
    __invokeRemote: function(connection, compress, response)
    {
        return connection.sendAsyncRequest(this, compress, response, 0);
    },
    __abort: function(ex)
    {
        if(this._proxy.ice_isBatchOneway() || this._proxy.ice_isBatchDatagram())
        {
            this._proxy.__getBatchRequestQueue().abortBatchRequest(this._os);
        }
        ProxyOutgoingAsyncBase.prototype.__abort.call(this, ex);
    },
    __invoke: function()
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
    },
    __completed: function(istr)
    {
        Debug.assert(this._proxy.ice_isTwoway()); // Can only be called for twoways.

        var replyStatus;
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
                    var id = new Identity();
                    id.__read(this._is);

                    //
                    // For compatibility with the old FacetPath.
                    //
                    var facetPath = Ice.StringSeqHelper.read(this._is);
                    var facet;
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

                    var operation = this._is.readString();

                    var rfe = null;
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
                    var unknown = this._is.readString();

                    var ue = null;
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
    },
    __startWriteParams: function(format)
    {
        this._os.startEncapsulation(this._encoding, format);
        return this._os;
    },
    __endWriteParams: function()
    {
        this._os.endEncapsulation();
    },
    __writeEmptyParams: function()
    {
        this._os.writeEmptyEncapsulation(this._encoding);
    },
    __writeParamEncaps: function(encaps)
    {
        if(encaps === null || encaps.length === 0)
        {
            this._os.writeEmptyEncapsulation(this._encoding);
        }
        else
        {
            this._os.writeEncapsulation(encaps);
        }
    },
    __is: function()
    {
        return this._is;
    },
    __startReadParams: function()
    {
        this._is.startEncapsulation();
        return this._is;
    },
    __endReadParams: function()
    {
        this._is.endEncapsulation();
    },
    __readEmptyParams: function()
    {
        this._is.skipEmptyEncapsulation(null);
    },
    __readParamEncaps: function()
    {
        return this._is.readEncapsulation(null);
    },
    __throwUserException: function()
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
    },
});
OutgoingAsync._emptyContext = new HashMap();

var ProxyFlushBatch = Ice.Class(ProxyOutgoingAsyncBase, {
    __init__ : function(prx, operation)
    {
        ProxyOutgoingAsyncBase.call(this, prx, operation);
        this._batchRequestNum = prx.__getBatchRequestQueue().swap(this._os);
    },
    __invokeRemote: function(connection, compress, response)
    {
        if(this._batchRequestNum === 0)
        {
            this.__sent();
            return AsyncStatus.Sent;
        }
        return connection.sendAsyncRequest(this, compress, response, this._batchRequestNum);
    },
    __invoke: function()
    {
        Protocol.checkSupportedProtocol(Protocol.getCompatibleProtocol(this._proxy.__reference().getProtocol()));
        this.__invokeImpl(true); // userThread = true
    },
});

var ProxyGetConnection = Ice.Class(ProxyOutgoingAsyncBase, {
    __init__ : function(prx, operation)
    {
        ProxyOutgoingAsyncBase.call(this, prx, operation);
    },
    __invokeRemote: function(connection, compress, response)
    {
        this.__markFinished(true,
                            function(r)
                            {
                                r.succeed(connection);
                            });
        return AsyncStatus.Sent;
    },
    __invoke: function()
    {
        this.__invokeImpl(true); // userThread = true
    }
});

var ConnectionFlushBatch = Ice.Class(OutgoingAsyncBase, {
    __init__: function(con, communicator, operation)
    {
        OutgoingAsyncBase.call(this, communicator, operation, con, null, null);
    },
    __invoke: function()
    {
        try
        {
            var batchRequestNum = this._connection.getBatchRequestQueue().swap(this._os);
            var status;
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
});

Ice.OutgoingAsync = OutgoingAsync;
Ice.ProxyFlushBatch = ProxyFlushBatch;
Ice.ProxyGetConnection = ProxyGetConnection;
Ice.ConnectionFlushBatch = ConnectionFlushBatch;

module.exports.Ice = Ice;
