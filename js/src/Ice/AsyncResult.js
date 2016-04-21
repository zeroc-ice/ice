// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

var Ice = require("../Ice/ModuleRegistry").Ice;
Ice.__M.require(module,
    [
        "../Ice/Class",
        "../Ice/AsyncResultBase",
        "../Ice/Debug",
        "../Ice/Promise",
        "../Ice/Protocol",
        "../Ice/Exception",
        "../Ice/Stream"
    ]);

var AsyncResultBase = Ice.AsyncResultBase;
var Debug = Ice.Debug;
var Promise = Ice.Promise;
var Protocol = Ice.Protocol;
var UserException = Ice.UserException;
var OutputStream = Ice.OutputStream;

var AsyncResult = Ice.Class(AsyncResultBase, {
    __init__: function(com, op, connection, proxy, adapter, completedFn)
    {
        //
        // AsyncResult can be constructed by a sub-type's prototype, in which case the
        // arguments are undefined.
        //
        AsyncResultBase.call(this, com, op, connection, proxy, adapter);
        if(com === undefined)
        {
            return;
        }
        
        this._completed = completedFn;
        this._is = null;
        this._os = com !== null ? new OutputStream(this._instance, Protocol.currentProtocolEncoding) : null;
        this._state = 0;
        this._exception = null;
        this._sentSynchronously = false;
    },
    cancel: function()
    {
        this.__cancel(new Ice.InvocationCanceledException());
    },
    isCompleted: function()
    {
        return (this._state & AsyncResult.Done) > 0;
    },
    isSent: function()
    {
        return (this._state & AsyncResult.Sent) > 0;
    },
    throwLocalException: function()
    {
        if(this._exception !== null)
        {
            throw this._exception;
        }
    },
    sentSynchronously: function()
    {
        return this._sentSynchronously;
    },
    __markSent: function(done)
    {
        Debug.assert((this._state & AsyncResult.Done) === 0);
        this._state |= AsyncResult.Sent;
        if(done)
        {
            this._state |= AsyncResult.Done | AsyncResult.OK;
            this._cancellationHandler = null;
            this.succeed(this);
        }
    },
    __markFinished: function(ok, completed)
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
            this.succeed(this);
        }
    },
    __markFinishedEx: function(ex)
    {
        Debug.assert((this._state & AsyncResult.Done) === 0);
        this._exception = ex;
        this._state |= AsyncResult.Done;
        this._cancellationHandler = null;
        this.fail(ex, this);
    },
    __cancel: function(ex)
    {
        this._cancellationException = ex;
        if(this._cancellationHandler)
        {
            this._cancellationHandler.asyncRequestCanceled(this, ex);
        }
    },
    __cancelable: function(handler)
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
    },
    __os: function()
    {
        return this._os;
    },
    __is: function()
    {
        return this._is;
    },
    __startReadParams: function()
    {
        this._is.startEncapsulation();
        return this._is;
    },
    __endReadParams: function()
    {
        this._is.endEncapsulation();
    },
    __readEmptyParams: function()
    {
        this._is.skipEmptyEncapsulation(null);
    },
    __readParamEncaps: function()
    {
        return this._is.readEncapsulation(null);
    },
    __throwUserException: function()
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
    },
});

AsyncResult.OK = 0x1;
AsyncResult.Done = 0x2;
AsyncResult.Sent = 0x4;

Ice.AsyncResult = AsyncResult;
module.exports.Ice = Ice;
