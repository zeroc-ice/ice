// Copyright (c) ZeroC, Inc.

import { Promise } from "./Promise.js";
import { InvocationCanceledException } from "./LocalExceptions.js";

export class AsyncResult extends Promise {
    constructor(communicator, op, proxy, completed) {
        super();
        this._communicator = communicator;
        this._instance = communicator ? communicator.instance : null;
        this._operation = op;
        this._proxy = proxy;
        this._completed = completed;
        this._is = null;
        this._state = 0;
        this._sentSynchronously = false;
        this._exception = null;
    }

    cancel() {
        this.cancelWithException(new InvocationCanceledException());
    }

    isCompleted() {
        return (this._state & AsyncResult.Done) > 0;
    }

    isSent() {
        return (this._state & AsyncResult.Sent) > 0;
    }

    throwLocalException() {
        if (this._exception !== null) {
            throw this._exception;
        }
    }

    sentSynchronously() {
        return this._sentSynchronously;
    }

    markSent(done) {
        DEV: console.assert((this._state & AsyncResult.Done) === 0);
        this._state |= AsyncResult.Sent;
        if (done) {
            this._state |= AsyncResult.Done | AsyncResult.Ok;
            this._cancellationHandler = null;
            this.resolve();
        }
    }

    markFinished(ok, completed) {
        DEV: console.assert((this._state & AsyncResult.Done) === 0);
        this._state |= AsyncResult.Done;
        if (ok) {
            this._state |= AsyncResult.Ok;
        }
        this._cancellationHandler = null;
        if (completed) {
            completed(this);
        } else {
            this.resolve();
        }
    }

    markFinishedEx(ex) {
        DEV: console.assert((this._state & AsyncResult.Done) === 0);
        this._exception = ex;
        this._state |= AsyncResult.Done;
        this._cancellationHandler = null;
        this.reject(ex);
    }

    cancelWithException(ex) {
        if (this._cancellationHandler) {
            this._cancellationHandler.asyncRequestCanceled(this, ex);
        } else {
            this._cancellationException = ex;
        }
    }

    cancelable(handler) {
        if (this._cancellationException) {
            try {
                throw this._cancellationException;
            } finally {
                this._cancellationException = null;
            }
        }
        this._cancellationHandler = handler;
    }

    get communicator() {
        return this._communicator;
    }

    get proxy() {
        return this._proxy;
    }

    get operation() {
        return this._operation;
    }
}

AsyncResult.Ok = 0x1;
AsyncResult.Done = 0x2;
AsyncResult.Sent = 0x4;
