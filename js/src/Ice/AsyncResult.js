//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import { Promise } from "./Promise.js";
import { UserException } from "./UserException.js";
import { InvocationCanceledException } from "./LocalExceptions.js";
import { Debug } from "./Debug.js";

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

    sentSynchronously() {
        return this._sentSynchronously;
    }

    markSent(done) {
        Debug.assert((this._state & AsyncResult.Done) === 0);
        this._state |= AsyncResult.Sent;
        if (done) {
            this._state |= AsyncResult.Done | AsyncResult.Ok;
            this._cancellationHandler = null;
            this.resolve();
        }
    }

    markFinished(ok, completed) {
        Debug.assert((this._state & AsyncResult.Done) === 0);
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
        Debug.assert((this._state & AsyncResult.Done) === 0);
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

    startReadParams() {
        this._is.startEncapsulation();
        return this._is;
    }

    endReadParams() {
        this._is.endEncapsulation();
    }

    readEmptyParams() {
        this._is.skipEmptyEncapsulation();
    }

    throwUserException() {
        Debug.assert((this._state & AsyncResult.Done) !== 0);
        if ((this._state & AsyncResult.Ok) === 0) {
            try {
                this._is.startEncapsulation();
                this._is.throwException();
            } catch (ex) {
                if (ex instanceof UserException) {
                    this._is.endEncapsulation();
                }
                throw ex;
            }
        }
    }

    get communicator() {
        return this._communicator;
    }

    get connection() {
        return this._connection;
    }

    get proxy() {
        return this._proxy;
    }

    get adapter() {
        return this._adapter;
    }

    get operation() {
        return this._operation;
    }
}

AsyncResult.Ok = 0x1;
AsyncResult.Done = 0x2;
AsyncResult.Sent = 0x4;
