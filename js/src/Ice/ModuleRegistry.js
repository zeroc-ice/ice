// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

var __modules__ = {};

var __M =
{
    module: function(name)
    {
        var m =  __modules__[name];
        if(m === undefined)
        {
            m = {};
            __modules__[name] =  m;
        }
        return m;
    },
    require: function(m, paths)
    {
        var i = 0, length = paths.length, o;
        for(; i < length; ++i)
        {
            o = m.require(paths[i]);
        }
        return o;
    },
    type: function(scoped)
    {
        if(scoped === undefined)
        {
            return undefined;
        }
        var components = scoped.split(".");
        var T = __modules__;

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
Ice.Slice = Ice.Slice || {};
Ice.__M = __M;
exports.Ice = Ice;
