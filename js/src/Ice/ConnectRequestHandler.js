// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

(function(global){
    require("Ice/Class");
    require("Ice/AsyncResult");
    require("Ice/AsyncStatus");
    require("Ice/BasicStream");
    require("Ice/BatchOutgoingAsync");
    require("Ice/ConnectionRequestHandler");
    require("Ice/Debug");
    require("Ice/ExUtil");
    require("Ice/LocalExceptionWrapper");
    require("Ice/OutgoingAsync");
    require("Ice/Protocol");
    require("Ice/ReferenceMode");
    require("Ice/Exception");
    require("Ice/Promise");

    var Ice = global.Ice || {};

    var AsyncResult = Ice.AsyncResult;
    var AsyncStatus = Ice.AsyncStatus;
    var BasicStream = Ice.BasicStream;
    var BatchOutgoingAsync = Ice.BatchOutgoingAsync;
    var ConnectionRequestHandler = Ice.ConnectionRequestHandler;
    var Debug = Ice.Debug;
    var ExUtil = Ice.ExUtil;
    var LocalExceptionWrapper = Ice.LocalExceptionWrapper;
    var OutgoingAsync = Ice.OutgoingAsync;
    var Protocol = Ice.Protocol;
    var ReferenceMode = Ice.ReferenceMode;
    var LocalException = Ice.LocalException;
    var Promise = Ice.Promise;

    var ConnectRequestHandler = Ice.Class({
        __init__: function(ref, proxy)
        {
            this._reference = ref;
            this._response = ref.getMode() === ReferenceMode.ModeTwoway;
            this._proxy = proxy;
            this._batchAutoFlush = ref.getInstance().initializationData().properties.getPropertyAsIntWithDefault(
                "Ice.BatchAutoFlush", 1) > 0 ? true : false;
            this._initialized = false;
            this._flushing = false;
            this._batchRequestInProgress = false;
            this._batchRequestsSize = Protocol.requestBatchHdr.length;
            this._batchStream =
                new BasicStream(ref.getInstance(), Protocol.currentProtocolEncoding, this._batchAutoFlush);
            this._updateRequestHandler = false;

            this._connection = null;
            this._compress = false;
            this._exception = null;
            this._requests = [];
            this._updateRequestHandler = false;
            this._pendingPromises = [];
        },
        connect: function()
        {
            var self = this;
            this._reference.getConnection().then(
                function(connection, compress)
                {
                    self.setConnection(connection, compress);
                }).exception(
                    function(ex)
                    {
                        self.setException(ex);
                    });

            if(this.initialized())
            {
                Debug.assert(this._connection !== null);
                return new ConnectionRequestHandler(this._reference, this._connection, this._compress);
            }
            else
            {
                // The proxy request handler will be updated when the connection is set.
                this._updateRequestHandler = true;
                return this;
            }
        },
        prepareBatchRequest: function(os)
        {
            if(!this.initialized())
            {
                this._batchRequestInProgress = true;
                this._batchStream.swap(os);
                return;
            }

            this._connection.prepareBatchRequest(os);
        },
        finishBatchRequest: function(os)
        {
            if(!this.initialized())
            {
                Debug.assert(this._batchRequestInProgress);
                this._batchRequestInProgress = false;

                this._batchStream.swap(os);

                if(!this._batchAutoFlush &&
                    this._batchStream.size + this._batchRequestsSize > this._reference.getInstance().messageSizeMax())
                {
                    ExUtil.throwMemoryLimitException(this._batchStream.size + this._batchRequestsSize,
                                                        this._reference.getInstance().messageSizeMax());
                }

                this._requests.push(new Request(this._batchStream));
                return;
            }
            this._connection.finishBatchRequest(os, this._compress);
        },
        abortBatchRequest: function()
        {
            if(!this.initialized())
            {
                Debug.assert(this._batchRequestInProgress);
                this._batchRequestInProgress = false;

                var dummy = new BasicStream(this._reference.getInstance(), Protocol.currentProtocolEncoding,
                                            this._batchAutoFlush);
                this._batchStream.swap(dummy);
                this._batchRequestsSize = Protocol.requestBatchHdr.length;

                return;
            }
            this._connection.abortBatchRequest();
        },
        sendAsyncRequest: function(out)
        {
            if(!this.initialized())
            {
                this._requests.push(new Request(out));
                return AsyncStatus.Queued;
            }
            return this._connection.sendAsyncRequest(out, this._compress, this._response);
        },
        flushAsyncBatchRequests: function(out)
        {
            if(!this.initialized())
            {
                this._requests.push(new Request(out));
                return AsyncStatus.Queued;
            }
            return this._connection.flushAsyncBatchRequests(out);
        },
        getReference: function()
        {
            return this._reference;
        },
        getConnection: function()
        {
            if(this._exception !== null)
            {
                throw this._exception;
            }
            else
            {
                return this._connection;
            }
        },
        onConnection: function(r)
        {
            //
            // Called by ObjectPrx.ice_getConnection
            //

            if(this._exception !== null)
            {
                r.__exception(this._exception);
            }
            else if(this._connection !== null)
            {
                Debug.assert(this._initialized);
                r.succeed(this._connection, r);
            }
            else
            {
                this._pendingPromises.push(r);
            }
        },
        //
        // Implementation of Reference_GetConnectionCallback
        //

        setConnection: function(connection, compress)
        {
            Debug.assert(this._exception === null && this._connection === null);
            Debug.assert(this._updateRequestHandler || this._requests.length === 0);

            this._connection = connection;
            this._compress = compress;

            //
            // If this proxy is for a non-local object, and we are using a router, then
            // add this proxy to the router info object.
            //
            var ri = this._reference.getRouterInfo();
            if(ri !== null)
            {
                var self = this;
                var promise = ri.addProxy(this._proxy).then(
                    function()
                    {
                        //
                        // The proxy was added to the router info, we're now ready to send the
                        // queued requests.
                        //
                        self.flushRequests();
                    }).exception(
                        function(ex)
                        {
                            self.setException(ex);
                        });

                if(!promise.completed())
                {
                    return; // The request handler will be initialized once addProxy completes.
                }
            }

            //
            // We can now send the queued requests.
            //
            this.flushRequests();
        },
        setException: function(ex)
        {
            Debug.assert(!this._initialized && this._exception === null);
            Debug.assert(this._updateRequestHandler || this._requests.length === 0);

            this._exception = ex;
            this._proxy = null; // Break cyclic reference count.

            //
            // If some requests were queued, we notify them of the failure.
            //
            if(this._requests.length > 0)
            {
                this.flushRequestsWithException(ex);
            }

            for(var i = 0; i < this._pendingPromises.length; ++i)
            {
                this._pendingPromises[i].fail(ex);
            }
            this._pendingPromises = [];
        },
        initialized: function()
        {
            if(this._initialized)
            {
                Debug.assert(this._connection !== null);
                return true;
            }
            else
            {
                if(this._exception !== null)
                {
                    throw this._exception;
                }
                else
                {
                    return this._initialized;
                }
            }
        },
        flushRequests: function()
        {
            Debug.assert(this._connection !== null && !this._initialized);

            //
            // We set the _flushing flag to true to prevent any additional queuing. Callers
            // might block for a little while as the queued requests are being sent but this
            // shouldn't be an issue as the request sends are non-blocking.
            //
            this._flushing = true;

            try
            {
                while(this._requests.length > 0)
                {
                    var request = this._requests[0];
                    if(request.out !== null)
                    {
                        this._connection.sendAsyncRequest(request.out, this._compress, this._response);
                    }
                    else if(request.batchOut !== null)
                    {
                        this._connection.flushAsyncBatchRequests(request.batchOut);
                    }
                    else
                    {
                        var os = new BasicStream(request.os.instance, Protocol.currentProtocolEncoding);
                        this._connection.prepareBatchRequest(os);
                        try
                        {
                            request.os.pos = 0;
                            os.writeBlob(request.os.readBlob(request.os.size));
                        }
                        catch(ex)
                        {
                            this._connection.abortBatchRequest();
                            throw ex;
                        }
                        this._connection.finishBatchRequest(os, this._compress);
                    }
                    this._requests.shift();
                }
            }
            catch(ex)
            {
                if(ex instanceof LocalExceptionWrapper)
                {
                    Debug.assert(this._exception === null && this._requests.length > 0);
                    this._exception = ex.inner;
                    this.flushRequestsWithExceptionWrapper(ex);
                }
                else if(ex instanceof LocalException)
                {
                    Debug.assert(this._exception === null && this._requests.length > 0);
                    this._exception = ex;
                    this.flushRequestsWithException(ex);
                }
                else
                {
                    throw ex;
                }
            }

            //
            // We've finished sending the queued requests and the request handler now send
            // the requests over the connection directly. It's time to substitute the
            // request handler of the proxy with the more efficient connection request
            // handler which does not have any synchronization. This also breaks the cyclic
            // reference count with the proxy.
            //
            // NOTE: _updateRequestHandler is immutable once _flushing = true
            //
            if(this._updateRequestHandler && this._exception === null)
            {
                this._proxy.__setRequestHandler(
                    new ConnectionRequestHandler(this._reference, this._connection, this._compress));
            }

            Debug.assert(!this._initialized);
            if(this._exception === null)
            {
                this._initialized = true;
                this._flushing = false;
            }
            this._proxy = null; // Break cyclic reference count.

            var p;
            for(var i = 0; i < this._pendingPromises.length; ++i)
            {
                p = this._pendingPromises[i];
                p.succeed(this._connection, p);
            }
            this._pendingPromises = [];
        },
        flushRequestsWithException: function(ex)
        {
            for(var i = 0; i < this._requests.length; ++i)
            {
                var request = this._requests[i];
                if(request.out !== null)
                {
                    request.out.__finishedEx(ex, false);
                }
                else if(request.batchOut !== null)
                {
                    request.batchOut.__finishedEx(ex, false);
                }
            }
            this._requests = [];
        },
        flushRequestsWithExceptionWrapper: function(ex)
        {
            for(var i = 0; i < this._requests.length; ++i)
            {
                var request = this._requests[i];
                if(request.out !== null)
                {
                    request.out.__finishedWrapper(ex);
                }
                else if(request.batchOut !== null)
                {
                    request.batchOut.__finishedEx(ex.inner, false);
                }
            }
            this._requests = [];
        }
    });
    
    Ice.ConnectRequestHandler = ConnectRequestHandler;
    global.Ice = Ice;

    var Request = function(arg)
    {
        this.os = null;
        this.out = null;
        this.batchOut = null;

        if(arg instanceof BasicStream)
        {
            this.os = new BasicStream(arg.instance, Protocol.currentProtocolEncoding);
            this.os.swap(arg);
        }
        else if(arg instanceof OutgoingAsync)
        {
            this.out = arg;
        }
        else
        {
            Debug.assert(arg instanceof BatchOutgoingAsync);
            this.batchOut = arg;
        }
    };
}(typeof (global) === "undefined" ? window : global));
