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
        this._connect = true;
        this._response = ref.getMode() === ReferenceMode.ModeTwoway;
        this._proxy = proxy;
        this._proxies = [];
        this._initialized = false;
        this._batchRequestInProgress = false;
        this._batchStream = new BasicStream(ref.getInstance(), Protocol.currentProtocolEncoding);

        this._connection = null;
        this._compress = false;
        this._exception = null;
        this._requests = [];
    },
    connect: function(proxy)
    {
        var self = this;
        if(proxy === this._proxy && this._connect)
        {
            this._connect = false; // Call getConnection only once
            this._reference.getConnection().then(function(connection, compress)
                                                 {
                                                     self.setConnection(connection, compress);
                                                 }, 
                                                 function(ex)
                                                 {
                                                     self.setException(ex);
                                                 });
        }

        try
        {
            if(!this.initialized())
            {
                this._proxies.push(proxy);
                return this;
            }
        }
        catch(ex)
        {
            throw ex;
        }

        if(this._connectionRequestHandler)
        {
            proxy.__setRequestHandler(this, this._connectionRequestHandler);
            return this._connectionRequestHandler;
        }
        else
        {
            return this;
        }
    },
    update: function(previousHandler, newHandler)
    {
        return previousHandler === this ? newHandler : this;
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

            var dummy = new BasicStream(this._reference.getInstance(), Protocol.currentProtocolEncoding);
            this._batchStream.swap(dummy);
            return;
        }
        this._connection.abortBatchRequest();
    },
    sendAsyncRequest: function(out)
    {
        if(!this._initialized)
        {
            out.__cancelable(this); // This will throw if the request is canceled
        }

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
    asyncRequestCanceled: function(out, ex)
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
                    out.__completedEx(ex);
                    this._requests.splice(i, 1);
                    return;
                }
            }
            Debug.assert(false); // The request has to be queued if it timed out and we're not initialized yet.
        }
        this._connection.asyncRequestCanceled(out, ex);
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
            ri.addProxy(this._proxy).then(function()
                                          {
                                              //
                                              // The proxy was added to the router info, we're now ready to send the
                                              // queued requests.
                                              //
                                              self.flushRequests();
                                          },
                                          function(ex)
                                          {
                                              self.setException(ex);
                                          });
            return; // The request handler will be initialized once addProxy completes.
        }

        //
        // We can now send the queued requests.
        //
        this.flushRequests();
    },
    setException: function(ex)
    {
        Debug.assert(!this._initialized && this._exception === null);

        this._exception = ex;
        this._proxies.length = 0;
        this._proxy = null; // Break cyclic reference count.

        //
        // NOTE: remove the request handler *before* notifying the
        // requests that the connection failed. It's important to ensure
        // that future invocations will obtain a new connect request
        // handler once invocations are notified.
        //
        try
        {
            this._reference.getInstance().requestHandlerFactory().removeRequestHandler(this._reference, this);
        }
        catch(exc)
        {
            // Ignore
        }

        this.flushRequestsWithException(ex);
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

        if(this._reference.getCacheConnection() && this._exception === null)
        {
            this._connectionRequestHandler = new ConnectionRequestHandler(this._reference,
                                                                          this._connection,
                                                                          this._compress);
            for(var i in this._proxies)
            {
                this._proxies[i].__setRequestHandler(this, this._connectionRequestHandler);
            }
        }

        Debug.assert(!this._initialized);
        if(this._exception === null)
        {
            this._initialized = true;
        }
        try
        {
            this._reference.getInstance().requestHandlerFactory().removeRequestHandler(this._reference, this);
        }
        catch(exc)
        {
            // Ignore
        }
        this._proxies.length = 0;
        this._proxy = null; // Break cyclic reference count.
    },
    flushRequestsWithException: function()
    {
        for(var i = 0; i < this._requests.length; ++i)
        {
            var request = this._requests[i];
            if(request.out !== null)
            {
                request.out.__completedEx(this._exception);
            }
        }
        this._requests.length = 0;
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
