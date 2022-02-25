//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

const Ice = require("../Ice/ModuleRegistry").Ice;

require("../Ice/AsyncStatus");
require("../Ice/ConnectionRequestHandler");
require("../Ice/Debug");
require("../Ice/Exception");
require("../Ice/ReferenceMode");
require("../Ice/RetryException");

const AsyncStatus = Ice.AsyncStatus;
const ConnectionRequestHandler = Ice.ConnectionRequestHandler;
const Debug = Ice.Debug;
const RetryException = Ice.RetryException;
const ReferenceMode = Ice.ReferenceMode;
const LocalException = Ice.LocalException;

class ConnectRequestHandler
{
    constructor(ref, proxy)
    {
        this._reference = ref;
        this._response = ref.getMode() === ReferenceMode.ModeTwoway;
        this._proxy = proxy;
        this._proxies = [];
        this._initialized = false;

        this._connection = null;
        this._exception = null;
        this._requests = [];
    }

    connect(proxy)
    {
        if(!this.initialized())
        {
            this._proxies.push(proxy);
        }
        return this._requestHandler ? this._requestHandler : this;
    }

    update(previousHandler, newHandler)
    {
        return previousHandler === this ? newHandler : this;
    }

    sendAsyncRequest(out)
    {
        if(!this._initialized)
        {
            out.cancelable(this); // This will throw if the request is canceled
        }

        if(!this.initialized())
        {
            this._requests.push(out);
            return AsyncStatus.Queued;
        }
        return out.invokeRemote(this._connection, this._response);
    }

    asyncRequestCanceled(out, ex)
    {
        if(this._exception !== null)
        {
            return; // The request has been notified of a failure already.
        }

        if(!this.initialized())
        {
            for(let i = 0; i < this._requests.length; i++)
            {
                if(this._requests[i] === out)
                {
                    out.completedEx(ex);
                    this._requests.splice(i, 1);
                    return;
                }
            }
            Debug.assert(false); // The request has to be queued if it timed out and we're not initialized yet.
        }
        this._connection.asyncRequestCanceled(out, ex);
    }

    getReference()
    {
        return this._reference;
    }

    getConnection()
    {
        if(this._exception !== null)
        {
            throw this._exception;
        }
        else
        {
            return this._connection;
        }
    }

    //
    // Implementation of Reference_GetConnectionCallback
    //
    setConnection(connection)
    {
        Debug.assert(this._exception === null && this._connection === null);

        this._connection = connection;

        //
        // If this proxy is for a non-local object, and we are using a router, then
        // add this proxy to the router info object.
        //
        const ri = this._reference.getRouterInfo();
        if(ri !== null)
        {
            ri.addProxy(this._proxy).then(
                //
                // The proxy was added to the router
                // info, we're now ready to send the
                // queued requests.
                //
                () => this.flushRequests(),
                ex => this.setException(ex));
            return; // The request handler will be initialized once addProxy completes.
        }

        //
        // We can now send the queued requests.
        //
        this.flushRequests();
    }

    setException(ex)
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

        this._requests.forEach(request =>
            {
                if(request !== null)
                {
                    request.completedEx(this._exception);
                }
            });
        this._requests.length = 0;
    }

    initialized()
    {
        if(this._initialized)
        {
            Debug.assert(this._connection !== null);
            return true;
        }
        else if(this._exception !== null)
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

    flushRequests()
    {
        Debug.assert(this._connection !== null && !this._initialized);

        let exception = null;
        this._requests.forEach(request =>
            {
                try
                {
                    request.invokeRemote(this._connection, this._response);
                }
                catch(ex)
                {
                    if(ex instanceof RetryException)
                    {
                        exception = ex.inner;

                        // Remove the request handler before retrying.
                        this._reference.getInstance().requestHandlerFactory().removeRequestHandler(this._reference, this);
                        request.retryException(ex.inner);
                    }
                    else
                    {
                        Debug.assert(ex instanceof LocalException);
                        exception = ex;
                        request.out.completedEx(ex);
                    }
                }
            });
        this._requests.length = 0;

        if(this._reference.getCacheConnection() && exception === null)
        {
            this._requestHandler = new ConnectionRequestHandler(this._reference, this._connection);
            this._proxies.forEach(proxy => proxy._updateRequestHandler(this, this._requestHandler));
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
}

Ice.ConnectRequestHandler = ConnectRequestHandler;
module.exports.Ice = Ice;
