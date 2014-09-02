// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
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
    require: function(m, name, paths)
    {
        var i = 0, length = paths.length, _m, o;
        for(; i < length; ++i)
        {
            o = m.require(paths[i])[name];
            if(o)
            {
                _m = o;
            }
        }
        return _m;
    },
    type: function(scoped)
    {
        if(scoped == undefined)
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
Ice.__ICE_NODEJS__ = true;
Ice.Slice = {};
Ice.__M = __M;
module.exports.Ice = Ice;
