// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

(function(global){
    require("Ice/StringUtil");
    
    var Ice = global.Ice || {};
    
    var StringUtil = Ice.StringUtil;

    var HashUtil = {};

    HashUtil.addBoolean = function(h, b)
    {
        return ((h << 5) + h) ^ (b ? 0 : 1);
    };

    HashUtil.addString = function(h, str)
    {
        if(str !== undefined && str !== null)
        {
            h = ((h << 5) + h) ^ StringUtil.hashCode(str);
        }
        return h;
    };

    HashUtil.addNumber = function(h, num)
    {
        return ((h << 5) + h) ^ num;
    };

    HashUtil.addHashable = function(h, obj)
    {
        if(obj !== undefined && obj !== null)
        {
            h = ((h << 5) + h) ^ obj.hashCode();
        }
        return h;
    };
    
    HashUtil.addArray = function(h, arr, hashCode)
    {
        if(arr !== undefined && arr !== null)
        {
            for(var i = 0; i < arr.length; ++i)
            {
                h = hashCode(h, arr[i]);
            }
        }
        return h;
    };

    Ice.HashUtil = HashUtil;
    global.Ice = Ice;
}(typeof (global) === "undefined" ? window : global));
