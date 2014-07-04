// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_FOR_ACTIONSCRIPT_LICENSE file included in this distribution.
//
// **********************************************************************

(function(global){
    require("Ice/Class");
    require("Ice/AsyncStatus");
    require("Ice/AsyncResult");
    require("Ice/BasicStream");
    require("Ice/Debug");
    require("Ice/HashMap");
    require("Ice/RetryException");
    require("Ice/Current");
    require("Ice/Protocol");
    require("Ice/BuiltinSequences");
    require("Ice/Exception");
    require("Ice/LocalException");
    require("Ice/Identity");
    
    var Ice = global.Ice || {};
    
    var AsyncStatus = Ice.AsyncStatus;
    var AsyncResult = Ice.AsyncResult;
    var BasicStream = Ice.BasicStream;
    var Debug = Ice.Debug;
    var HashMap = Ice.HashMap;
    var RetryException = Ice.RetryException;
    var OperationMode = Ice.OperationMode;
    var Protocol = Ice.Protocol;
    var Identity = Ice.Identity;

    var OutgoingAsync = Ice.Class(AsyncResult, {
        __init__: function(prx, operation, completed, sent)
        {
            //
            // OutgoingAsync can be constructed by a sub-type's prototype, in which case the
            // arguments are undefined.
            //
            if(prx !== undefined)
            {
                AsyncResult.call(this, prx.ice_getCommunicator(), operation, null, prx, null, completed, sent);
                this._batch = this._proxy.ice_isBatchOneway() || this._proxy.ice_isBatchDatagram();

                this._batchStarted = false;

                this._handler = null;
                this._encoding = Protocol.getCompatibleEncoding(this._proxy.__reference().getEncoding());
                this._cnt = 0;
                this._mode = null;
            }
            else
            {
                AsyncResult.call(this);
            }
        },
        __prepare: function(op, mode, ctx)
        {
            this._handler = null;
            this._cnt = 0;
            this._mode = mode;

            Protocol.checkSupportedProtocol(Protocol.getCompatibleProtocol(this._proxy.__reference().getProtocol()));

            if(ctx === null)
            {
                ctx = OutgoingAsync._emptyContext;
            }

            if(this._batch)
            {
                while(true)
                {
                    try
                    {
                        this._handler = this._proxy.__getRequestHandler();
                        this._handler.prepareBatchRequest(this._os);
                        this._batchStarted = true;
                        break;
                    }
                    catch(ex)
                    {
                        if(ex instanceof RetryException)
                        {
                            this._proxy.__setRequestHandler(this._handler, null); // Clear request handler and retry.
                        }
                        else
                        {
                            this._proxy.__setRequestHandler(this._handler, null); // Clear request handler and retry.
                            throw ex;
                        }
                    }
                }
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
        __send: function(connection, compress, response)
        {
            return connection.sendAsyncRequest(this, compress, response);
        },
        __sent: function()
        {
            this._state |= AsyncResult.Sent;

            Debug.assert((this._state & AsyncResult.Done) === 0);

            if(!this._proxy.ice_isTwoway())
            {
                if(this._timeoutRequestHandler)
                {
                    this._instance.timer().cancel(this._timeoutToken);
                    this._timeoutRequestHandler = null;
                }
                this._state |= AsyncResult.Done | AsyncResult.OK;
                this._os.resize(0);
                if(this._sent)
                {
                    this._sent.call(null, this);
                }
                else
                {
                    this.succeed(this);
                }
            }
        },
        __finishedEx: function(exc, sent)
        {
            Debug.assert((this._state & AsyncResult.Done) === 0);
            if(this._timeoutRequestHandler)
            {
                this._instance.timer().cancel(this._timeoutToken);
                this._timeoutRequestHandler = null;
            }

            try
            {
                if(!this.handleException(exc, sent))
                {
                    return; // Can't be retried immediately.
                }

                this.__invoke();
            }
            catch(ex)
            {
                if(ex instanceof Ice.Exception)
                {
                    this.__invokeException(ex);
                }
                else
                {
                    this.fail(ex, this);
                }
            }
        },
        __finished: function(istr)
        {
            Debug.assert(this._proxy.ice_isTwoway()); // Can only be called for twoways.

            var replyStatus;
            try
            {
                Debug.assert(this._exception === null && (this._state & AsyncResult.Done) === 0);

                if(this._timeoutRequestHandler)
                {
                    this._instance.timer().cancel(this._timeoutToken);
                    this._timeoutRequestHandler = null;
                }

                if(this._is === null) // _is can already be initialized if the invocation is retried
                {
                    this._is = new BasicStream(this._instance, Protocol.currentProtocolEncoding, false);
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

                this._state |= AsyncResult.Done;
                if(replyStatus == Protocol.replyOK)
                {
                    this._state |= AsyncResult.OK;
                }
            }
            catch(ex)
            {
                if(ex instanceof Ice.LocalException)
                {
                    this.__finishedEx(ex, true);
                    return;
                }
                else
                {
                    this.fail(ex, this);
                    return;
                }
            }

            Debug.assert(replyStatus === Protocol.replyOK || replyStatus === Protocol.replyUserException);
            this.__invokeCompleted();
        },
        __invoke: function()
        {
            if(this._batch)
            {
                Debug.assert(this._handler !== null);
                this._handler.finishBatchRequest(this._os);
                this.succeed(this);
                return;
            }

            while(true)
            {
                var interval = 0;
                try
                {
                    this._handler = this._proxy.__getRequestHandler();
                    var status = this._handler.sendAsyncRequest(this);
                    if(this._proxy.ice_isTwoway() || (status & AsyncStatus.Sent) === 0)
                    {
                        Debug.assert((this._state & AsyncResult.Done) === 0);
                        
                        var invocationTimeout = this._handler.getReference().getInvocationTimeout();
                        if(invocationTimeout > 0)
                        {
                            var self = this;
                            this._timeoutToken = this._instance.timer().schedule(function() 
                                                                                 { 
                                                                                     self.__runTimerTask(); 
                                                                                 }, 
                                                                                 invocationTimeout);
                            this._timeoutRequestHandler = this._handler;
                        }
                    }
                    break;
                }
                catch(ex)
                {
                    if(ex instanceof RetryException)
                    {
                        this._proxy.__setRequestHandler(this._handler, null); // Clear request handler and retry.
                    }
                    else if(ex instanceof Ice.Exception)
                    {
                        if(!this.handleException(ex, false)) // This will throw if the invocation can't be retried.
                        {
                            break; // Can't be retried immediately.
                        }
                    }
                    else
                    {
                        throw ex;
                    }
                }
            }
        },
        __startWriteParams: function(format)
        {
            this._os.startWriteEncaps(this._encoding, format);
            return this._os;
        },
        __endWriteParams: function()
        {
            this._os.endWriteEncaps();
        },
        __writeEmptyParams: function()
        {
            this._os.writeEmptyEncaps(this._encoding);
        },
        __writeParamEncaps: function(encaps)
        {
            if(encaps === null || encaps.length === 0)
            {
                this._os.writeEmptyEncaps(this._encoding);
            }
            else
            {
                this._os.writeEncaps(encaps);
            }
        },
        __invokeException: function(ex)
        {
            AsyncResult.prototype.__invokeException.call(this, ex);

            if(this._batchStarted)
            {
                Debug.assert(this._handler !== null);
                this._handler.abortBatchRequest();
            }
        },
        handleException: function(exc, sent)
        {
            var interval = { value: 0 };
            this._cnt = this._proxy.__handleException(exc, this._handler, this._mode, sent, interval, this._cnt);
            if(interval.value > 0)
            {
                this._instance.retryQueue().add(this, interval.value);
                return false; // Don't retry immediately, the retry queue will take care of the retry.
            }
            else
            {
                return true; // Retry immediately.
            }
        },
    });    
    OutgoingAsync._emptyContext = new HashMap();
    
    var BatchOutgoingAsync = Ice.Class(AsyncResult, {
        __init__: function(communicator, operation)
        {
            AsyncResult.call(this, communicator, operation, null, null, null, null);
        },
        __send: function(connection, compress, response)
        {
            return connection.flushAsyncBatchRequests(this);
        },
        __sent: function()
        {
            this._state |= AsyncResult.Done | AsyncResult.OK | AsyncResult.Sent;
            this._os.resize(0);
            if(this._timeoutRequestHandler)
            {
                this._instance.timer().cancel(this._timeoutToken);
                this._timeoutRequestHandler = null;
            }
            this.succeed(this);
        },
        __finishedEx: function(exc, sent)
        {
            if(this._timeoutRequestHandler)
            {
                this._instance.timer().cancel(this._timeoutToken);
                this._timeoutRequestHandler = null;
            }
            this.__invokeException(exc);
        }
    });

    var ProxyBatchOutgoingAsync = Ice.Class(BatchOutgoingAsync, {
        __init__: function(prx, operation)
        {
            BatchOutgoingAsync.call(this, prx.ice_getCommunicator(), operation);
            this._proxy = prx;
        },
        __invoke: function()
        {
            Protocol.checkSupportedProtocol(this._proxy.__reference().getProtocol());

            var handler = null;
            try
            {
                handler = this._proxy.__getRequestHandler();
                var status = handler.sendAsyncRequest(this);
                if((status & AsyncStatus.Sent) === 0)
                {
                    var invocationTimeout = handler.getReference().getInvocationTimeout();
                    if(invocationTimeout > 0)
                    {
                        var self = this;
                        this._timeoutToken = this._instance.timer().schedule(function() 
                                                                             { 
                                                                                 self.__runTimerTask(); 
                                                                             }, 
                                                                             invocationTimeout);
                        this._timeoutRequestHandler = handler;
                    }
                }
            }
            catch(__ex)
            {
                if(__ex instanceof RetryException)
                {
                    //
                    // Clear request handler but don't retry or throw. Retrying
                    // isn't useful, there were no batch requests associated with
                    // the proxy's request handler.
                    //
                    this._proxy.__setRequestHandler(handler, null);
                }
                else
                {
                    this._proxy.__setRequestHandler(handler, null); // Clear request handler
                    throw __ex; // Throw to notify the user lthat batch requests were potentially lost.
                }
            }
        }
    });
    
    var GetConnectionOutgoingAsync = Ice.Class(AsyncResult, {
        __init__: function(proxy)
        {
            AsyncResult.call(this, proxy.ice_getCommunicator(), "ice_getConnection", null, proxy, null, null, null);
        },
        __send: function(connection, compress, response)
        {
            this.succeed(connection, this);
            return true;
        },
        __sent: function()
        {
            Debug.assert(false);
        },
        __finishedEx: function(exc, sent)
        {
            this.__invokeException(exc);
        }
    });

    var ConnectionBatchOutgoingAsync = Ice.Class(BatchOutgoingAsync, {
        __init__: function(con, communicator, operation)
        {
            BatchOutgoingAsync.call(this, communicator, operation);
            this._connection = con;
        },
        __invoke: function()
        {
            this._connection.flushAsyncBatchRequests(this);
        }
    });

    Ice.OutgoingAsync = OutgoingAsync;
    Ice.BatchOutgoingAsync = BatchOutgoingAsync;
    Ice.ProxyBatchOutgoingAsync = ProxyBatchOutgoingAsync;
    Ice.ConnectionBatchOutgoingAsync = ConnectionBatchOutgoingAsync;
    Ice.GetConnectionOutgoingAsync = GetConnectionOutgoingAsync;

    global.Ice = Ice;
}(typeof (global) === "undefined" ? window : global));
