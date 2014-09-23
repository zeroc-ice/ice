// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

var Ice = require("../Ice/ModuleRegistry").Ice;
Ice.__M.require(module,
    [
        "../Ice/Class",
        "../Ice/AsyncResult",
        "../Ice/AsyncStatus",
        "../Ice/BasicStream",
        "../Ice/ConnectionRequestHandler",
        "../Ice/Debug",
        "../Ice/ExUtil",
        "../Ice/RetryException",
        "../Ice/OutgoingAsync",
        "../Ice/Protocol",
        "../Ice/ReferenceMode",
        "../Ice/Exception",
        "../Ice/Promise"
    ]);

var AsyncResult = Ice.AsyncResult;
var AsyncStatus = Ice.AsyncStatus;
var BasicStream = Ice.BasicStream;
var ConnectionRequestHandler = Ice.ConnectionRequestHandler;
var Debug = Ice.Debug;
var ExUtil = Ice.ExUtil;
var RetryException = Ice.RetryException;
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
        try
        {
            if(!this.initialized())
            {
                this._batchRequestInProgress = true;
                this._batchStream.swap(os);
                return;
            }
        }
        catch(ex)
        {
            throw new RetryException(ex);
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
        try
        {
            if(!this.initialized())
            {
                this._requests.push(new Request(out));
                return AsyncStatus.Queued;
            }
        }
        catch(ex)
        {
            throw new RetryException(ex);
        }
        return out.__send(this._connection, this._compress, this._response);
    },
    asyncRequestTimedOut: function(out)
    {
        if(this._exception !== null)
        {
            return; // The request has been notified of a failure already.
        }

        if(!this.initialized())
        {
            for(var i = 0; i < this._requests.length; i++)
            {
                if(this._requests[i].out === out)
                {
                    out.__finishedEx(new Ice.InvocationTimeoutException(), false);
                    this._requests.splice(i, 1);
                    return;
                }
            }
            Debug.assert(false); // The request has to be queued if it timed out and we're not initialized yet.
        }
        this._connection.asyncRequestTimedOut(out);
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
                    request.out.__send(this._connection, this._compress, this._response);
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
            if(ex instanceof RetryException)
            {
                //
                // If the connection dies shortly after connection
                // establishment, we don't systematically retry on
                // RetryException. We handle the exception like it
                // was an exception that occured while sending the
                // request.
                // 
                Debug.assert(this._exception === null && this._requests.length > 0);
                this._exception = ex.inner;
                this.flushRequestsWithException();
            }
            else if(ex instanceof LocalException)
            {
                Debug.assert(this._exception === null && this._requests.length > 0);
                this._exception = ex;
                this.flushRequestsWithException();
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
            this._proxy.__setRequestHandler(this, new ConnectionRequestHandler(this._reference, this._connection, 
                                                                                this._compress));
        }

        Debug.assert(!this._initialized);
        if(this._exception === null)
        {
            this._initialized = true;
            this._flushing = false;
        }
        this._proxy = null; // Break cyclic reference count.
    },
    flushRequestsWithException: function()
    {
        for(var i = 0; i < this._requests.length; ++i)
        {
            var request = this._requests[i];
            if(request.out !== null)
            {
                request.out.__finishedEx(this._exception, false);
            }
        }
        this._requests = [];
    }
});

Ice.ConnectRequestHandler = ConnectRequestHandler;
module.exports.Ice = Ice;

var Request = function(arg)
{
    this.os = null;
    this.out = null;

    if(arg instanceof BasicStream)
    {
        this.os = new BasicStream(arg.instance, Protocol.currentProtocolEncoding);
        this.os.swap(arg);
    }
    else
    {
        this.out = arg;
    }
};
