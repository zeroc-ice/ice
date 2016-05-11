// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

var Ice = require("../Ice/ModuleRegistry").Ice;
Ice.__M.require(module, ["../Ice/Object", "../Ice/Class"]);
    
var SliceInfo = function()
{
    /**
    * The Slice type ID for this slice.
    **/
    this.typeId = "";

    /**
    * The Slice compact type ID for this slice.
    **/
    this.compactId = -1;

    /**
    * The encoded bytes for this slice, including the leading size integer.
    **/
    this.bytes = [];

    /**
    * The class instances referenced by this slice.
    **/
    this.instances = [];

    /**
    * Whether or not the slice contains optional members.
    **/
    this.hasOptionalMembers = false;

    /**
    * Whether or not this is the last slice.
    **/
    this.isLastSlice = false;
};
Ice.SliceInfo = SliceInfo;

var SlicedData = function(slices)
{
    this.slices = slices;
};
Ice.SlicedData = SlicedData;

var UnknownSlicedValue = Ice.Class(Ice.Object,
    {
        __init__: function(unknownTypeId)
        {
            this._unknownTypeId = unknownTypeId;
        },
        getUnknownTypeId: function()
        {
            return this._unknownTypeId;
        },
        __write: function(os)
        {
            os.startValue(this._slicedData);
            os.endValue();
        },
        __read: function(is)
        {
            is.startValue();
            this._slicedData = is.endValue(true);
        }
    });
Ice.UnknownSlicedValue = UnknownSlicedValue;

module.exports.Ice = Ice;
