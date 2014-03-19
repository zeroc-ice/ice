// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

(function(global){
    require("Ice/HashMap");
    require("Ice/LocalException");
    require("Ice/Class");
    
    var Ice = global.Ice || {};
    
    var HashMap = Ice.HashMap;
    var CommunicatorDestroyedException = Ice.CommunicatorDestroyedException;
    
    var Timer = Ice.Class({
        __init__: function(instance)
        {
            this._instance = instance;
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

            var id = setTimeout(function() { self.handleTimeout(token); }, delay);
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

            var id = setInterval(function() { self.handleInterval(token); }, period);
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
                clearInterval(token.id);
            }
            else
            {
                clearTimeout(token.id);
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
                token.callback();
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
                token.callback();
            }
        }
    });

    Ice.Timer = Timer;
    global.Ice = Ice;
}(typeof (global) === "undefined" ? window : global));
