// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

const Ice = require("../Ice/ModuleRegistry").Ice;
Ice.__M.require(module,
    [
        "../Ice/AsyncResultBase",
        "../Ice/Debug",
        "../Ice/Protocol",
        "../Ice/Exception",
        "../Ice/Stream"
    ]);

const AsyncResultBase = Ice.AsyncResultBase;
const Debug = Ice.Debug;
const Protocol = Ice.Protocol;
const UserException = Ice.UserException;
const OutputStream = Ice.OutputStream;

class AsyncResult extends AsyncResultBase
{
    constructor(com, op, connection, proxy, adapter, completedFn)
    {
        super(com, op, connection, proxy, adapter);
        this._completed = completedFn;
        this._is = null;
        this._os = com !== null ? new OutputStream(this._instance, Protocol.currentProtocolEncoding) : null;
        this._state = 0;
        this._exception = null;
        this._sentSynchronously = false;
    }

    cancel()
    {
        this.__cancel(new Ice.InvocationCanceledException());
    }

    isCompleted()
    {
        return (this._state & AsyncResult.Done) > 0;
    }

    isSent()
    {
        return (this._state & AsyncResult.Sent) > 0;
    }

    throwLocalException()
    {
        if(this._exception !== null)
        {
            throw this._exception;
        }
    }

    sentSynchronously()
    {
        return this._sentSynchronously;
    }

    __markSent(done)
    {
        Debug.assert((this._state & AsyncResult.Done) === 0);
        this._state |= AsyncResult.Sent;
        if(done)
        {
            this._state |= AsyncResult.Done | AsyncResult.OK;
            this._cancellationHandler = null;
            this.resolve();
        }
    }

    __markFinished(ok, completed)
    {
        Debug.assert((this._state & AsyncResult.Done) === 0);
        this._state |= AsyncResult.Done;
        if(ok)
        {
            this._state |= AsyncResult.OK;
        }
        this._cancellationHandler = null;
        if(completed)
        {
            completed(this);
        }
        else
        {
            this.resolve();
        }
    }

    __markFinishedEx(ex)
    {
        Debug.assert((this._state & AsyncResult.Done) === 0);
        this._exception = ex;
        this._state |= AsyncResult.Done;
        this._cancellationHandler = null;
        this.reject(ex);
    }

    __cancel(ex)
    {
        this._cancellationException = ex;
        if(this._cancellationHandler)
        {
            this._cancellationHandler.asyncRequestCanceled(this, ex);
        }
    }

    __cancelable(handler)
    {
        if(this._cancellationException)
        {
            try
            {
                throw this._cancellationException;
            }
            finally
            {
                this._cancellationException = null;
            }
        }
        this._cancellationHandler = handler;
    }

    __os()
    {
        return this._os;
    }

    __is()
    {
        return this._is;
    }
    
    __startReadParams()
    {
        this._is.startEncapsulation();
        return this._is;
    }
    
    __endReadParams()
    {
        this._is.endEncapsulation();
    }

    __readEmptyParams()
    {
        this._is.skipEmptyEncapsulation();
    }

    __readParamEncaps()
    {
        return this._is.readEncapsulation(null);
    }

    __throwUserException()
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
                if(ex instanceof UserException)
                {
                    this._is.endEncapsulation();
                }
                throw ex;
            }
        }
    }

}

AsyncResult.OK = 0x1;
AsyncResult.Done = 0x2;
AsyncResult.Sent = 0x4;

Ice.AsyncResult = AsyncResult;
module.exports.Ice = Ice;
