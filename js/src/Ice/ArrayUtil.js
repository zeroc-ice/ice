// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************
    
var Ice = require("../Ice/ModuleRegistry").Ice;
var __M = Ice.__M;
var Slice = Ice.Slice;

var eq = function(e1, e2)
{
    if(e1 === e2)
    {
        return true; // If identity compare equals members are equal.
    }
    else if(e1 === null || e1 === undefined || e2 === null || e2 === undefined)
    {
        return false;
    }
    else if(e1.prototype !== e2.prototype)
    {
        return false;
    }
    else if(typeof e1.equals == "function")
    {
        return e1.equals(e2);
    }
    else if(e1 instanceof Array)
    {
        return ArrayUtil.equals(e1, e2, eq);
    }
    return false;
};

var ArrayUtil =
{
    clone: function(arr)
    {
        if(arr === undefined)
        {
            return arr;
        }
        else if(arr === null)
        {
            return [];
        }
        else
        {
            return arr.slice();
        }
    },
    equals: function(v1, v2, valuesEqual)
    {        
        if(v1.length != v2.length)
        {
            return false;
        }

        var i, length, equalFn = valuesEqual || eq;
        for(i = 0, length = v1.length; i < length; ++i)
        {
            if(!equalFn.call(equalFn, v1[i], v2[i]))
            {
                return false;
            }
        }

        return true;
    },
    shuffle: function(arr)
    {
        for(var i = arr.length; i > 1; --i)
        {
            var e = arr[i - 1];
            var rand = Math.floor(Math.random() * i);
            arr[i - 1] = arr[rand];
            arr[rand] = e;
        }
    },
    indexOf: function(arr, elem, equalFn)
    {
        if(equalFn !== undefined && equalFn !== null)
        {
            for(var i = 0; i < arr.length; ++i)
            {
                if(equalFn.call(equalFn, arr[i], elem))
                {
                    return i;
                }
            }
        }
        else
        {
            return arr.indexOf(elem);
        }

        return -1;
    },
    filter: function(arr, includeFn, obj)
    {
        obj = obj === undefined ? includeFn : obj;
        var result = [];
        for(var i = 0; i < arr.length; ++i)
        {
            if(includeFn.call(obj, arr[i], i, arr))
            {
                result.push(arr[i]);
            }
        }
        return result;
    }
};

ArrayUtil.eq = eq;

Slice.defineSequence = function(module, name, valueHelper, fixed, elementType)
{
    var helper = null;
    Object.defineProperty(module, name, 
    {
        get: function()
            {
                if(helper === null)
                {
                    /*jshint -W061 */
                    helper = Ice.StreamHelpers.generateSeqHelper(__M.type(valueHelper), fixed, __M.type(elementType));
                    /*jshint +W061 */
                }
                return helper;
            }
    });
};

Ice.ArrayUtil = ArrayUtil;
module.exports.Ice = Ice;
