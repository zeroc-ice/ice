// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
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
        this._proxies = [];
        this._initialized = false;

        this._connection = null;
        this._compress = false;
        this._exception = null;
        this._requests = [];
    },
    connect: function(proxy)
    {
        if(!this.initialized())
        {
            this._proxies.push(proxy);
        }
        return this._requestHandler ? this._requestHandler : this;
    },
    update: function(previousHandler, newHandler)
    {
        return previousHandler === this ? newHandler : this;
    },
    sendAsyncRequest: function(out)
    {
        if(!this._initialized)
        {
            out.__cancelable(this); // This will throw if the request is canceled
        }

        if(!this.initialized())
        {
            this._requests.push(out);
            return AsyncStatus.Queued;
        }
        return out.__invokeRemote(this._connection, this._compress, this._response);
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
                if(this._requests[i] === out)
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

        for(var i = 0; i < this._requests.length; ++i)
        {
            var request = this._requests[i];
            if(request !== null)
            {
                request.__completedEx(this._exception);
            }
        }
        this._requests.length = 0;
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
                if(this._connection !== null)
                {
                    //
                    // Only throw if the connection didn't get established. If
                    // it died after being established, we allow the caller to
                    // retry the connection establishment by not throwing here
                    // (the connection will throw RetryException).
                    //
                    return true;
                }
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

        var exception = null;
        for(var i = 0; i < this._requests.length; ++i)
        {
            var request = this._requests[i];
            try
            {
                request.__invokeRemote(this._connection, this._compress, this._response);
            }
            catch(ex)
            {
                if(ex instanceof RetryException)
                {
                    exception = ex.inner;

                    // Remove the request handler before retrying.
                    this._reference.getInstance().requestHandlerFactory().removeRequestHandler(this._reference, this);

                    request.__retryException(ex.inner);
                }
                else
                {
                    Debug.assert(ex instanceof LocalException);
                    exception = ex;
                    request.out.__completedEx(ex);
                }
            }
        }
        this._requests.length = 0;

        if(this._reference.getCacheConnection() && exception === null)
        {
            this._requestHandler = new ConnectionRequestHandler(this._reference, this._connection, this._compress);
            for(var k = 0; k < this._proxies.length; ++k)
            {
                this._proxies[k].__updateRequestHandler(this, this._requestHandler);
            }
        }

        Debug.assert(!this._initialized);
        this._exception = exception;
        this._initialized = this._exception === null;

        //
        // Only remove once all the requests are flushed to
        // guarantee serialization.
        //
        this._reference.getInstance().requestHandlerFactory().removeRequestHandler(this._reference, this);

        this._proxies.length = 0;
        this._proxy = null; // Break cyclic reference count.
    }
});

Ice.ConnectRequestHandler = ConnectRequestHandler;
module.exports.Ice = Ice;
