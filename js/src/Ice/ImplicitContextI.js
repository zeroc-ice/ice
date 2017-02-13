// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

var Ice = require("../Ice/ModuleRegistry").Ice;
Ice.__M.require(module, ["../Ice/LocalException", "../Ice/Current", "../Ice/Class"]);

var Context = Ice.Context;
var InitializationException = Ice.InitializationException;

//
// The base class for all ImplicitContext implementations
//
var ImplicitContextI = Ice.Class({
    __init__: function()
    {
        this._context = new Context();
    },
    getContext: function()
    {
        return new Context(this._context);
    },
    setContext: function(context)
    {
        if(context !== null && context.size > 0)
        {
            this._context = new Context(context);
        }
        else
        {
            this._context.clear();
        }
    },
    containsKey: function(key)
    {
        if(key === null)
        {
            key = "";
        }

        return this._context.has(key);
    },
    get: function(key)
    {
        if(key === null)
        {
            key = "";
        }

        var val = this._context.get(key);
        if(val === null)
        {
            val = "";
        }

        return val;
    },
    put: function(key, value)
    {
        if(key === null)
        {
            key = "";
        }
        if(value === null)
        {
            value = "";
        }

        var oldVal = this._context.get(key);
        if(oldVal === null)
        {
            oldVal = "";
        }

        this._context.set(key, value);

        return oldVal;
    },
    remove: function(key)
    {
        if(key === null)
        {
            key = "";
        }

        var val = this._context.get(key);
        this._context.delete(key);

        if(val === null)
        {
            val = "";
        }
        return val;
    },
    write: function(prxContext, os)
    {
        if(prxContext.size === 0)
        {
            Ice.ContextHelper.write(os, this._context);
        }
        else
        {
            var ctx = null;
            if(this._context.size === 0)
            {
                ctx = prxContext;
            }
            else
            {
                ctx = new Context(this._context);
                ctx.merge(prxContext);
            }
            Ice.ContextHelper.write(os, ctx);
        }
    }
});

ImplicitContextI.create = function(kind)
{
    if(kind.length === 0 || kind === "None")
    {
        return null;
    }
    else if(kind === "Shared")
    {
        return new ImplicitContextI();
    }
    else
    {
        throw new InitializationException("'" + kind + "' is not a valid value for Ice.ImplicitContext");
    }
};
Ice.ImplicitContextI = ImplicitContextI;
module.exports.Ice = Ice;
