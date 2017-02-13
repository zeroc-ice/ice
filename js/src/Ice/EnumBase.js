// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

var Ice = require("../Ice/Class").Ice;

//
// Ice.EnumBase
//
var EnumBase = Ice.Class({
    __init__: function(name, value)
    {
        this._name = name;
        this._value = value;
    },
    equals: function(rhs)
    {
        if(this === rhs)
        {
            return true;
        }

        var proto = Object.getPrototypeOf(this);
        if(!(rhs instanceof proto.constructor))
        {
            return false;
        }

        return this._value == rhs._value;
    },
    hashCode: function()
    {
        return this._value;
    },
    toString: function()
    {
        return this._name;
    }
});
Ice.EnumBase = EnumBase;

var prototype = EnumBase.prototype;

Object.defineProperty(prototype, 'name', {
    enumerable: true,
    get: function() { return this._name; }
});

Object.defineProperty(prototype, 'value', {
    enumerable: true,
    get: function() { return this._value; }
});

var EnumHelper = Ice.Class({
    __init__: function(enumType)
    {
        this._enumType = enumType;
    },
    write: function(os, v)
    {
        this._enumType.__write(os, v);
    },
    writeOpt: function(os, tag, v)
    {
        this._enumType.__writeOpt(os, tag, v);
    },
    read: function(is)
    {
        return this._enumType.__read(is);
    },
    readOpt: function(is, tag)
    {
        return this._enumType.__readOpt(is, tag);
    }
});

Ice.EnumHelper = EnumHelper;

var Slice = Ice.Slice;
Slice.defineEnum = function(enumerators)
{
    var type = function(n, v)
    {
        EnumBase.call(this, n, v);
    };

    type.prototype = new EnumBase();
    type.prototype.constructor = type;

    var enums = [];
    var maxValue = 0;
    var firstEnum = null;
    for(var idx in enumerators)
    {
        var e = enumerators[idx][0], value = enumerators[idx][1];
        var enumerator = new type(e, value);
        enums[value] = enumerator;
        if(!firstEnum)
        {
            firstEnum = enumerator;
        }
        Object.defineProperty(type, e, {
            enumerable: true,
            value: enumerator
        });
        if(value > maxValue)
        {
            maxValue = value;
        }
    }

    Object.defineProperty(type, "minWireSize", {
        get: function(){ return 1; }
    });

    type.__write = function(os, v)
    {
        if(v)
        {
            os.writeEnum(v);
        }
        else
        {
            os.writeEnum(firstEnum);
        }
    };
    type.__read = function(is)
    {
        return is.readEnum(type);
    };
    type.__writeOpt = function(os, tag, v)
    {
        if(v !== undefined)
        {
            if(os.writeOpt(tag, Ice.OptionalFormat.Size))
            {
                type.__write(os, v);
            }
        }
    };
    type.__readOpt = function(is, tag)
    {
        return is.readOptEnum(tag, type);
    };

    type.__helper = new EnumHelper(type);

    Object.defineProperty(type, 'valueOf', {
        value: function(v) {
            if(v === undefined)
            {
                return type;
            }
            return enums[v];
        }
    });

    Object.defineProperty(type, 'maxValue', {
        value: maxValue
    });

    Object.defineProperty(type.prototype, 'maxValue', {
        value: maxValue
    });

    return type;
};
module.exports.Ice = Ice;
