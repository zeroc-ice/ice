// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
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
    * The Ice objects referenced by this slice.
    **/
    this.objects = [];

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

var UnknownSlicedObject = Ice.Class(Ice.Object,
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
            os.startWriteObject(this._slicedData);
            os.endWriteObject();
        },
        __read: function(is)
        {
            is.startReadObject();
            this._slicedData = is.endReadObject(true);
        }
    });
Ice.UnknownSlicedObject = UnknownSlicedObject;

module.exports.Ice = Ice;
