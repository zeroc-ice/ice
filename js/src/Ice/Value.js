//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

//
// Ice.Value
//
const Ice = require("../Ice/ModuleRegistry").Ice;
require("../Ice/Exception");
require("../Ice/FormatType");
require("../Ice/StreamHelpers");
require("../Ice/OptionalFormat");

Ice.Value = class
{
    ice_preMarshal()
    {
    }

    ice_postUnmarshal()
    {
    }

    ice_getSlicedData()
    {
        return null;
    }

    _iceWrite(os)
    {
        os.startValue(null);
        writeImpl(this, os, this._iceMostDerivedType());
        os.endValue();
    }

    _iceRead(is)
    {
        is.startValue();
        readImpl(this, is, this._iceMostDerivedType());
        is.endValue(false);
    }

    //
    // These methods are used for object parameters.
    //
    static write(os, v)
    {
        os.writeValue(v);
    }

    static writeOptional(os, tag, v)
    {
        os.writeOptionalValue(tag, v);
    }

    static read(is)
    {
        const v = {value: null};
        is.readValue(o =>
                     {
                         v.value = o;
                     }, this);
        return v;
    }

    static readOptional(is, tag)
    {
        const v = {value: undefined};
        is.readOptionalValue(tag, o =>
                             {
                                 v.value = o;
                             }, this);
        return v;
    }
};

Ice.InterfaceByValue = class extends Ice.Value
{
    constructor(id)
    {
        super();
        this._id = id;
    }

    ice_id()
    {
        return this._id;
    }

    _iceWrite(os)
    {
        os.startValue(null);
        os.startSlice(this.ice_id(), -1, true);
        os.endSlice();
        os.endValue();
    }

    _iceRead(is)
    {
        is.startValue();
        is.startSlice();
        is.endSlice();
        is.endValue(false);
    }
};

//
// Private methods
//
const writeImpl = function(obj, os, type)
{
    //
    // The writeImpl method is a recursive method that goes down the
    // class hierarchy to marshal each slice of the class using the
    // generated _iceWriteMemberImpl method.
    //

    if(type === undefined || type === Ice.Value)
    {
        return; // Don't marshal anything for Ice.Value
    }

    os.startSlice(type.ice_staticId(),
                  Object.prototype.hasOwnProperty.call(type, '_iceCompactId') ? type._iceCompactId : -1,
                  Object.getPrototypeOf(type) === Ice.Value);
    if(type.prototype.hasOwnProperty('_iceWriteMemberImpl'))
    {
        type.prototype._iceWriteMemberImpl.call(obj, os);
    }
    os.endSlice();
    writeImpl(obj, os, Object.getPrototypeOf(type));
};

const readImpl = function(obj, is, type)
{
    //
    // The readImpl method is a recursive method that goes down the
    // class hierarchy to unmarshal each slice of the class using the
    // generated _iceReadMemberImpl method.
    //

    if(type === undefined || type === Ice.Value)
    {
        return; // Don't unmarshal anything for Ice.Value
    }

    is.startSlice();
    if(type.prototype.hasOwnProperty('_iceReadMemberImpl'))
    {
        type.prototype._iceReadMemberImpl.call(obj, is);
    }
    is.endSlice();
    readImpl(obj, is, Object.getPrototypeOf(type));
};

function writePreserved(os)
{
    //
    // For Slice classes which are marked "preserved", the implementation of this method
    // replaces the Ice.Value.prototype._iceWrite method.
    //
    os.startValue(this._iceSlicedData);
    writeImpl(this, os, this._iceMostDerivedType());
    os.endValue();
}

function readPreserved(is)
{
    //
    // For Slice classes which are marked "preserved", the implementation of this method
    // replaces the Ice.Value.prototype._iceRead method.
    //
    is.startValue();
    readImpl(this, is, this._iceMostDerivedType());
    this._iceSlicedData = is.endValue(true);
}

function ice_getSlicedData()
{
    return this._iceSlicedData;
}

const Slice = Ice.Slice;

Slice.defineValue = function(valueType, id, preserved, compactId = 0)
{
    valueType.prototype.ice_id = function()
    {
        return id;
    };

    valueType.prototype._iceMostDerivedType = function()
    {
        return valueType;
    };

    valueType.ice_staticId = function()
    {
        return id;
    };

    if(preserved)
    {
        valueType.prototype.ice_getSlicedData = ice_getSlicedData;
        valueType.prototype._iceWrite = writePreserved;
        valueType.prototype._iceRead = readPreserved;
    }

    if(compactId > 0)
    {
        Ice.CompactIdRegistry.set(compactId, id);
    }
};
Slice.defineValue(Ice.Value, "::Ice::Object");

module.exports.Ice = Ice;
