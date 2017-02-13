// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

var Ice = require("../Ice/ModuleRegistry").Ice;
Ice.__M.require(module,
    [
        "../Ice/HashUtil",
        "../Ice/ArrayUtil",
        "../Ice/StreamHelpers"
    ]);

var ArrayUtil = Ice.ArrayUtil;

//
// Use generic equality test from ArrayUtil.
//
var eq = ArrayUtil.eq;

var equals = function(other)
{
    if(this === other)
    {
        return true;
    }

    if(other === null || other === undefined)
    {
        return false;
    }

    if(this.prototype !== other.prototype)
    {
        return false;
    }

    var e1, e2;
    for(var key in this)
    {
        e1 = this[key];
        e2 = other[key];
        if(typeof e1 == "function")
        {
            continue; // Don't need to compare functions
        }
        else if(!eq(e1, e2))
        {
            return false;
        }
    }
    return true;
};

var clone = function()
{
    var other = new this.constructor();
    var e;
    for(var key in this)
    {
        e = this[key];
        if(e === undefined || e === null)
        {
            other[key] = e;
        }
        else if(typeof e == "function")
        {
            continue;
        }
        else if(typeof e.clone == "function")
        {
            other[key] = e.clone();
        }
        else if(e instanceof Array)
        {
            other[key] = ArrayUtil.clone(e);
        }
        else
        {
            other[key] = e;
        }
    }
    return other;
};

var memberHashCode = function(h, e)
{
    if(typeof e.hashCode == "function")
    {
        return Ice.HashUtil.addHashable(h, e);
    }
    else if(e instanceof Array)
    {
        return Ice.HashUtil.addArray(h, e, memberHashCode);
    }
    else
    {
        var t = typeof(e);
        if(e instanceof String || t == "string")
        {
            return Ice.HashUtil.addString(h, e);
        }
        else if(e instanceof Number || t == "number")
        {
            return Ice.HashUtil.addNumber(h, e);
        }
        else if(e instanceof Boolean || t == "boolean")
        {
            return Ice.HashUtil.addBoolean(h, e);
        }
    }
};

var hashCode = function()
{
    var __h = 5381;
    var e;
    for(var key in this)
    {
        e = this[key];
        if(e === undefined || e === null || typeof e == "function")
        {
            continue;
        }
        __h = memberHashCode(__h, e);
    }
    return __h;
};

Ice.Slice.defineStruct = function(constructor, legalKeyType, writeImpl, readImpl, minWireSize, fixed)
{
    var obj = constructor;

    obj.prototype.clone = clone;

    obj.prototype.equals = equals;

    //
    // Only generate hashCode if this structure type is a legal dictionary key type.
    //
    if(legalKeyType)
    {
        obj.prototype.hashCode = hashCode;
    }

    if(readImpl && writeImpl)
    {
        obj.prototype.__write = writeImpl;
        obj.prototype.__read = readImpl;
        obj.write = function(os, v)
        {
            if(!v)
            {
                if(!obj.prototype._nullMarshalValue)
                {
                    obj.prototype._nullMarshalValue = new this();
                }
                v = obj.prototype._nullMarshalValue;
            }
            v.__write(os);
        };
        obj.read = function(is, v)
        {
            if(!v || !(v instanceof this))
            {
                v = new this();
            }
            v.__read(is);
            return v;
        };
        Object.defineProperty(obj, "minWireSize", {
            get: function() { return minWireSize; }
        });
        if(fixed)
        {
            Ice.StreamHelpers.VSizeOptHelper.call(obj);
        }
        else
        {
            Ice.StreamHelpers.FSizeOptHelper.call(obj);
        }
    }
    return obj;
};
module.exports.Ice = Ice;
