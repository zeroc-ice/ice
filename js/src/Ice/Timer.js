// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

var Ice = require("../Ice/ModuleRegistry").Ice;
Ice.__M.require(module, ["../Ice/HashMap", "../Ice/LocalException", "../Ice/Class", "../Ice/TimerUtil"]);

var HashMap = Ice.HashMap;
var CommunicatorDestroyedException = Ice.CommunicatorDestroyedException;

var Timer = Ice.Class({
    __init__: function(logger)
    {
        this._logger = logger;
        this._destroyed = false;
        this._tokenId = 0;
        this._tokens = new HashMap();
    },
    destroy: function()
    {
        var self = this;
        this._tokens.forEach(function(key, value){
            self.cancel(key);
        });
        this._destroyed = true;
        this._tokens.clear();
    },
    schedule: function(callback, delay)
    {
        if(this._destroyed)
        {
            throw new CommunicatorDestroyedException();
        }

        var token = this._tokenId++;
        var self = this;
        var id = Timer.setTimeout(function() { self.handleTimeout(token); }, delay);
        this._tokens.set(token, { callback: callback, id: id, isInterval: false });

        return token;
    },
    scheduleRepeated: function(callback, period)
    {
        if(this._destroyed)
        {
            throw new CommunicatorDestroyedException();
        }

        var token = this._tokenId++;
        var self = this;

        var id = Timer.setInterval(function() { self.handleInterval(token); }, period);
        this._tokens.set(token, { callback: callback, id: id, isInterval: true });

        return token;
    },
    cancel: function(id)
    {
        if(this._destroyed)
        {
            return false;
        }

        var token = this._tokens.get(id);
        if(token === undefined)
        {
            return false;
        }

        this._tokens.delete(id);
        if(token.isInterval)
        {
            Timer.clearInterval(token.id);
        }
        else
        {
            Timer.clearTimeout(token.id);
        }

        return true;
    },
    handleTimeout: function(id)
    {
        if(this._destroyed)
        {
            return;
        }

        var token = this._tokens.get(id);
        if(token !== undefined)
        {
            this._tokens.delete(id);
            try
            {
                token.callback();
            }
            catch(ex)
            {
                this._logger.warning("uncaught exception while executing timer:\n" + ex);
            }
        }
    },
    handleInterval: function(id)
    {
        if(this._destroyed)
        {
            return;
        }

        var token = this._tokens.get(id);
        if(token !== undefined)
        {
            try
            {
                token.callback();
            }
            catch(ex)
            {
                this._logger.warning("uncaught exception while executing timer:\n" + ex);
            }
        }
    }
});

Timer.setTimeout = Ice.Timer.setTimeout;
Timer.clearTimeout = Ice.Timer.clearTimeout;
Timer.setInterval = Ice.Timer.setInterval;
Timer.clearInterval = Ice.Timer.clearInterval;
Timer.setImmediate = Ice.Timer.setImmediate;

Ice.Timer = Timer;
module.exports.Ice = Ice;
