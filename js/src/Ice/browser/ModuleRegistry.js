// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
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
    }
};

var Ice = __M.module("Ice");

Ice.__require = function()
{
    return window;
};

Ice.Slice = Ice.Slice || {};
Ice.__M = __M;
