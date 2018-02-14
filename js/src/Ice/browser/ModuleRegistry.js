// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

/* globals self */
var __root = typeof(window) !== "undefined" ? window : self;
/* globals -self */

var __M =
{
    module: function(name)
    {
        var m =  __root[name];
        if(m === undefined)
        {
            m = {};
            __root[name] =  m;
        }
        return m;
    },
    require: function(name)
    {
        return __root;
    },
    type: function(scoped)
    {
        if(scoped === undefined)
        {
            return undefined;
        }
        var components = scoped.split(".");
        var T = __root;

        for(var i = 0, length = components.length; i < length; ++i)
        {
            T = T[components[i]];
            if(T === undefined)
            {
                return undefined;
            }
        }
        return T;
    }
};

var Ice = __M.module("Ice");

Ice.__require = function()
{
    return __root;
};

Ice.Slice = Ice.Slice || {};
Ice.__M = __M;
