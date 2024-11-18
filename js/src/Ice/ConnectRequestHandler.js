//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import { AsyncStatus } from "./AsyncStatus.js";
import { LocalException } from "./LocalException.js";
import { RetryException } from "./RetryException.js";

export class ConnectRequestHandler {
    constructor(reference) {
        this._reference = reference;
        this._response = reference.isTwoway;
        this._initialized = false;
        this._connection = null;
        this._exception = null;
        this._requests = [];
    }

    sendAsyncRequest(out) {
        if (!this._initialized) {
            out.cancelable(this); // This will throw if the request is canceled
        }

        if (!this.initialized()) {
            this._requests.push(out);
            return AsyncStatus.Queued;
        }
        return out.invokeRemote(this._connection, this._response);
    }

    asyncRequestCanceled(out, ex) {
        if (this._exception !== null) {
            return; // The request has been notified of a failure already.
        }

        if (!this.initialized()) {
            for (let i = 0; i < this._requests.length; i++) {
                if (this._requests[i] === out) {
                    out.completedEx(ex);
                    this._requests.splice(i, 1);
                    return;
                }
            }
            DEV: console.assert(false); // The request has to be queued if it timed out and we're not initialized yet.
        }
        this._connection.asyncRequestCanceled(out, ex);
    }

    getConnection() {
        // First check for the connection, it's important otherwise the user could first get a connection
        // and then the exception if he tries to obtain the proxy cached connection multiple times (the
        // exception can be set after the connection is set if the flush of pending requests fails).
        if (this._connection !== null) {
            return this._connection;
        } else if (this._exception !== null) {
            throw this._exception;
        }
        return null;
    }

    //
    // Implementation of Reference_GetConnectionCallback
    //
    setConnection(connection) {
        DEV: console.assert(this._exception === null && this._connection === null);

        this._connection = connection;

        // If this proxy is for a non-local object, and we are using a router, then add this proxy to the router info
        // object.
        const ri = this._reference.getRouterInfo();
        if (ri !== null) {
            ri.addProxy(this._reference).then(
                // The proxy was added to the router info, we're now ready to send the queued requests.
                () => this.flushRequests(),
                ex => this.setException(ex),
            );
            return; // The request handler will be initialized once addProxy completes.
        }

        // We can now send the queued requests.
        this.flushRequests();
    }

    setException(ex) {
        DEV: console.assert(!this._initialized && this._exception === null);

        this._exception = ex;

        for (const request of this._requests) {
            request.completedEx(this._exception);
        }
        this._requests.length = 0;
    }

    initialized() {
        if (this._initialized) {
            DEV: console.assert(this._connection !== null);
            return true;
        } else {
            if (this._exception !== null) {
                if (this._connection !== null) {
                    // Only throw if the connection didn't get established. If it died after being established, we allow
                    // the caller to retry the connection establishment by not throwing here (the connection will throw
                    // RetryException).
                    return true;
                }
                throw this._exception;
            } else {
                return this._initialized;
            }
        }
    }

    flushRequests() {
        DEV: console.assert(this._connection !== null && !this._initialized);
        let exception = null;
        for (const request of this._requests) {
            try {
                request.invokeRemote(this._connection, this._response);
            } catch (ex) {
                if (ex instanceof RetryException) {
                    exception = ex.inner;

                    // Remove the request handler before retrying.
                    this._reference.getInstance().requestHandlerFactory().removeRequestHandler(this._reference, this);
                    request.retryException();
                } else {
                    DEV: console.assert(ex instanceof LocalException, ex);
                    exception = ex;
                    request.out.completedEx(ex);
                }
            }
        }
        this._requests.length = 0;

        DEV: console.assert(!this._initialized);
        this._exception = exception;
        this._initialized = this._exception === null;
    }
}
