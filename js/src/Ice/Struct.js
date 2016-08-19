// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

const Ice = require("../Ice/ModuleRegistry").Ice;
Ice.__M.require(module,
    [
        "../Ice/HashUtil",
        "../Ice/ArrayUtil",
        "../Ice/StreamHelpers"
    ]);

const ArrayUtil = Ice.ArrayUtil;

//
// Use generic equality test from ArrayUtil.
//
const eq = ArrayUtil.eq;

function equals(other)
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

    for(let key in this)
    {
        let e1 = this[key];
        let e2 = other[key];
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
}

function clone()
{
    const other = new this.constructor();
    for(let key in this)
    {
        let e = this[key];
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
}

function memberHashCode(h, e)
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
        const t = typeof(e);
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
}

function hashCode()
{
    let __h = 5381;
    for(let key in this)
    {
        let e = this[key];
        if(e === undefined || e === null || typeof e == "function")
        {
            continue;
        }
        __h = memberHashCode(__h, e);
    }
    return __h;
}

Ice.Slice.defineStruct = function(obj, legalKeyType, variableLength)
{
    obj.prototype.clone = clone;

    obj.prototype.equals = equals;

    //
    // Only generate hashCode if this structure type is a legal dictionary key type.
    //
    if(legalKeyType)
    {
        obj.prototype.hashCode = hashCode;
    }

    if(obj.prototype.__write && obj.prototype.__read)
    {
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
     
        if(variableLength)
        {
            Ice.StreamHelpers.FSizeOptHelper.call(obj);
        }
        else
        {
            Ice.StreamHelpers.VSizeOptHelper.call(obj);
        }
    }
    return obj;
};
module.exports.Ice = Ice;
