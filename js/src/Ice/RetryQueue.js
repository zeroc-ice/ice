// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

var Ice = require("../Ice/ModuleRegistry").Ice;
Ice.__M.require(module, ["../Ice/Class", "../Ice/LocalException"]);

var Class = Ice.Class;

var RetryQueue = Class({
    __init__: function(instance)
    {
        this._instance = instance;
        this._requests = [];
    },
    add: function(outAsync, interval)
    {
        if(this._instance === null)
        {
            throw new Ice.CommunicatorDestroyedException();
        }
        var task = new RetryTask(this._instance, this, outAsync);
        outAsync.__cancelable(task); // This will throw if the request is canceled
        task.token = this._instance.timer().schedule(function()
                                                     {
                                                         task.run();
                                                     }, interval);
        this._requests.push(task);
    },
    destroy: function()
    {
        for(var i = 0; i < this._requests.length; ++i)
        {
            this._instance.timer().cancel(this._requests[i].token);
            this._requests[i].destroy();
        }
        this._requests = [];
        this._instance = null;
    },
    remove: function(task)
    {
        var idx = this._requests.indexOf(task);
        if(idx >= 0)
        {
            this._requests.splice(idx, 1);
        }
    },
    cancel: function(task)
    {
        var idx = this._requests.indexOf(task);
        if(idx >= 0)
        {
            this._requests.splice(idx, 1);
            return this._instance.timer().cancel(task.token);
        }
        return false;
    }
});
Ice.RetryQueue = RetryQueue;

var RetryTask = Class({
    __init__: function(instance, queue, outAsync, interval)
    {
        this._instance = instance;
        this._queue = queue;
        this._outAsync = outAsync;
    },
    run: function()
    {
        this._outAsync.__retry();
        this._queue.remove(this);
    },
    destroy: function()
    {
        try
        {
            this._outAsync.__abort(new Ice.CommunicatorDestroyedException());
        }
        catch(ex)
        {
            // Abort shouldn't throw if there's no callback, ignore.
        }
    },
    asyncRequestCanceled: function(outAsync, ex)
    {
        if(this._queue.cancel(this))
        {
            if(this._instance.traceLevels().retry >= 1)
            {
                this._instance.initializationData().logger.trace(this._instance.traceLevels().retryCat,
                                                                 "operation retry canceled\n" + ex.toString());
            }
            this._outAsync.__completedEx(ex);
        }
    }
});
module.exports.Ice = Ice;
