// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

(function(global){
    var Ice = global.Ice || {};
    var Slice = global.Slice || {};
    var ArrayUtil = {};

    ArrayUtil.clone = function(arr)
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
    };

    ArrayUtil.equals = function(v1, v2, equalFn)
    {
        var i, length;
        
        if(v1.length != v2.length)
        {
            return false;
        }

        if(equalFn !== undefined && equalFn !== null)
        {
            for(i = 0, length = v1.length; i < length; ++i)
            {
                if(!equalFn.call(equalFn, v1[i], v2[i]))
                {
                    return false;
                }
            }
        }
        else
        {
            for(i = 0, length = v1.length; i < length; ++i)
            {
                if(v1[i] != v2[i])
                {
                    return false;
                }
            }
        }

        return true;
    };

    ArrayUtil.shuffle = function(arr)
    {
        for(var i = arr.length; i > 1; --i)
        {
            var e = arr[i - 1];
            var rand = Math.floor(Math.random() * i);
            arr[i - 1] = arr[rand];
            arr[rand] = e;
        }
    };

    ArrayUtil.indexOf = function(arr, elem, equalFn)
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
    };

    ArrayUtil.filter = function(arr, includeFn, obj)
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
    };
    
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
                        helper = Ice.StreamHelpers.generateSeqHelper(eval(valueHelper), fixed, eval(elementType));
                        /*jshint +W061 */
                    }
                    return helper;
                }
        });
    };

    Ice.ArrayUtil = ArrayUtil;
    global.Slice = Slice;
    global.Ice = Ice;
}(typeof (global) === "undefined" ? window : global));
