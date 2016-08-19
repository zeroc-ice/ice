// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

/* globals self */
const __root = typeof(window) !== "undefined" ? window : typeof(global) !== "undefined" ? global : typeof(self) !== "undefined" ? self : {};
/* globals -self */

class __M
{
    static module(name)
    {
        var m =  __root[name];
        if(m === undefined)
        {
            m = {};
            __root[name] =  m;
        }
        return m;
    }
    
    static require(name)
    {
        return __root;
    }
    
    static type(scoped)
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
}

const Ice = __M.module("Ice");

Ice.__require = function()
{
    return __root;
};

Ice.Slice = Ice.Slice || {};
Ice.__M = __M;
