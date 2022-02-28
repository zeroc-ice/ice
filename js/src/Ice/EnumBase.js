//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

const Ice = require("../Ice/ModuleRegistry").Ice;

//
// Ice.EnumBase
//
class EnumBase
{
    constructor(name, value)
    {
        this._name = name;
        this._value = value;
    }

    equals(rhs)
    {
        if(this === rhs)
        {
            return true;
        }

        if(!(rhs instanceof Object.getPrototypeOf(this).constructor))
        {
            return false;
        }

        return this._value == rhs._value;
    }

    hashCode()
    {
        return this._value;
    }

    toString()
    {
        return this._name;
    }

    get name()
    {
        return this._name;
    }

    get value()
    {
        return this._value;
    }
}
Ice.EnumBase = EnumBase;

class EnumHelper
{
    constructor(enumType)
    {
        this._enumType = enumType;
    }

    write(os, v)
    {
        this._enumType._write(os, v);
    }

    writeOptional(os, tag, v)
    {
        this._enumType._writeOpt(os, tag, v);
    }

    read(is)
    {
        return this._enumType._read(is);
    }

    readOptional(is, tag)
    {
        return this._enumType._readOpt(is, tag);
    }
}

Ice.EnumHelper = EnumHelper;

const Slice = Ice.Slice;
Slice.defineEnum = function(enumerators)
{
    const type = class extends EnumBase
    {
    };

    const enums = [];
    let maxValue = 0;
    let firstEnum = null;

    for(const idx in enumerators)
    {
        const e = enumerators[idx][0];
        const value = enumerators[idx][1];
        const enumerator = new type(e, value);
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

    Object.defineProperty(type, "minWireSize", {get: () => 1});

    type._write = function(os, v)
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

    type._read = function(is)
    {
        return is.readEnum(type);
    };

    type._writeOpt = function(os, tag, v)
    {
        if(v !== undefined)
        {
            if(os.writeOptional(tag, Ice.OptionalFormat.Size))
            {
                type._write(os, v);
            }
        }
    };

    type._readOpt = function(is, tag)
    {
        return is.readOptionalEnum(tag, type);
    };

    type._helper = new EnumHelper(type);

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
