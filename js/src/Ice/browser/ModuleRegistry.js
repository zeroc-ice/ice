// **********************************************************************
//
// Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

var __M =
{
    module: function(name)
    {
        var m =  window[name];
        if(m === undefined)
        {
            m = {};
            window[name] =  m;
        }
        return m;
    },
    require: function(name)
    {
        return window;
    },
    type: function(scoped)
    {
        if(scoped === undefined)
        {
            return undefined;
        }
        var components = scoped.split(".");
        var T = window;

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
    return window;
};

Ice.Slice = Ice.Slice || {};
Ice.__M = __M;
