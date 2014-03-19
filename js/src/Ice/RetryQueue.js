// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

(function(global){
    require("Ice/Class");
    require("Ice/Debug");
    require("Ice/LocalException");

    var Ice = global.Ice || {};

    var Debug = Ice.Debug;

    var Class = Ice.Class;
    
    var RetryQueue = Class({
        __init__: function(instance)
        {
            this._instance = instance;
            this._requests = [];
        },
        add: function(outAsync, interval)
        {
            var task = new RetryTask(this, outAsync);
            this._instance.timer().schedule(function()
                {
                    task.run();
                }, interval);
            this._requests.push(task);
        },
        destroy: function()
        {
            for(var i = 0; i < this._requests.length; ++i)
            {
                this._requests[i].destroy();
            }
            this._requests = [];
        },
        remove: function(task)
        {
            var idx = this._requests.indexOf(task);
            if(idx >= 0)
            {
                this._requests.splice(idx, 1);
                return true;
            }
            return false;
        }
    });
    Ice.RetryQueue = RetryQueue;

    var RetryTask = Class({
        __init__: function(queue, outAsync, interval)
        {
            this.queue = queue;
            this.outAsync = outAsync;
        },
        run: function()
        {
            if(this.queue.remove(this))
            {
                try
                {
                    this.outAsync.__send();
                }
                catch(ex)
                {
                    this.outAsync.__exception(ex);
                }
            }
        },
        destroy: function()
        {
            this.outAsync.__exception(new Ice.CommunicatorDestroyedException());
        }
    });
    global.Ice = Ice;
}(typeof (global) === "undefined" ? window : global));
