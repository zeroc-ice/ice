// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

var Ice = require("../Ice/StringUtil").Ice;
var StringUtil = Ice.StringUtil;

Ice.HashUtil =
{
    addBoolean: function(h, b)
    {
        return ((h << 5) + h) ^ (b ? 0 : 1);
    },
    addString: function(h, str)
    {
        if(str !== undefined && str !== null)
        {
            h = ((h << 5) + h) ^ StringUtil.hashCode(str);
        }
        return h;
    },
    addNumber: function(h, num)
    {
        return ((h << 5) + h) ^ num;
    },
    addHashable: function(h, obj)
    {
        if(obj !== undefined && obj !== null)
        {
            h = ((h << 5) + h) ^ obj.hashCode();
        }
        return h;
    },
    addArray: function(h, arr, hashCode)
    {
        if(arr !== undefined && arr !== null)
        {
            for(var i = 0; i < arr.length; ++i)
            {
                h = hashCode(h, arr[i]);
            }
        }
        return h;
    }
};
module.exports.Ice = Ice;
