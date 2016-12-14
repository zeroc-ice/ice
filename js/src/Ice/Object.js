// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

//
// Ice.Object
//
// Using IceObject in this file to avoid collisions with the native Object.
//
const Ice = require("../Ice/ModuleRegistry").Ice;
Ice._ModuleRegistry.require(module,
    [
        "../Ice/Exception",
        "../Ice/FormatType",
        "../Ice/StreamHelpers",
        "../Ice/OptionalFormat"
    ]);

let nextAddress = 0;

const ids = ["::Ice::Object"];

Ice.Object = class
{
    constructor()
    {
        // Fake Address used as the hashCode for this object instance.
        this._iceAddress = nextAddress++;
    }

    hashCode()
    {
        return this._iceAddress;
    }

    ice_isA(s, current)
    {
        return this._iceMostDerivedType()._iceIds.indexOf(s) >= 0;
    }

    ice_ping(current)
    {
    }

    ice_ids(current)
    {
        return this._iceMostDerivedType()._iceIds;
    }

    ice_id(current)
    {
        return this._iceMostDerivedType()._iceId;
    }

    toString()
    {
        return "[object " + this.ice_id() + "]";
    }

    ice_preMarshal()
    {
    }

    ice_postUnmarshal()
    {
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

    ice_instanceof(T)
    {
        if(T)
        {
            if(this instanceof T)
            {
                return true;
            }
            return this._iceMostDerivedType()._iceInstanceof(T);
        }
        return false;
    }

    //
    // _iceMostDerivedType returns the the most derived Ice generated class. This is
    // necessary because the user might extend Slice generated classes. The user
    // class extensions don't have _iceId, _iceIds, _iceInstanceof etc static members so
    // the implementation of ice_id, ice_ids and ice_instanceof would fail trying
    // to access those members of the user defined class. Instead, ice_id, ice_ids
    // and ice_instanceof call _iceMostDerivedType to get the most derived Ice class.
    //
    // The _iceMostDerivedType is overriden by each Slice generated class, see the
    // Slice.defineObject method implementation for details.
    //
    _iceMostDerivedType()
    {
        return Ice.Object;
    }

    //
    // The default implementation of equals compare references.
    //
    equals(other)
    {
        return this === other;
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
        const v = { value: null };
        is.readValue(o => v.value = o, this);
        return v;
    }

    static readOptional(is, tag)
    {
        const v = { value: undefined };
        is.readOptionalValue(tag, o => v.value = o, this);
        return v;
    }

    static ice_staticId()
    {
        return this._iceId;
    }

    static _iceInstanceof(T)
    {
        if(T === this)
        {
            return true;
        }

        if(this._iceImplements.some(i => i._iceInstanceof(T)))
        {
            return true;
        }

        if(this._iceParent)
        {
            return this._iceParent._iceInstanceof(T);
        }
        return false;
    }

    static get _iceIds()
    {
        return ids;
    }

    static get _iceId()
    {
        return ids[0];
    }

    static get _iceImplements()
    {
        return [];
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

    if(type === undefined || type === Ice.Object)
    {
        return; // Don't marshal anything for Ice.Object
    }

    os.startSlice(type._iceId,
                  Object.prototype.hasOwnProperty.call(type, '_iceCompactId') ? type._iceCompactId : -1 ,
                  type._iceParent === Ice.Object);
    if(type.prototype._iceWriteMemberImpl)
    {
        type.prototype._iceWriteMemberImpl.call(obj, os);
    }
    os.endSlice();
    writeImpl(obj, os, type._iceParent);
};

const readImpl = function(obj, is, type)
{
    //
    // The readImpl method is a recursive method that goes down the
    // class hierarchy to marshal each slice of the class using the
    // generated _iceReadMemberImpl method.
    //

    if(type === undefined || type === Ice.Object)
    {
        return; // Don't marshal anything for Ice.Object
    }

    is.startSlice();
    if(type.prototype._iceReadMemberImpl)
    {
        type.prototype._iceReadMemberImpl.call(obj, is);
    }
    is.endSlice();
    readImpl(obj, is, type._iceParent);
};

const writePreserved = function(os)
{
    //
    // For Slice classes which are marked "preserved", the implementation of this method
    // replaces the Ice.Object.prototype._iceWrite method.
    //
    os.startValue(this._iceSlicedData);
    writeImpl(this, os, this._iceMostDerivedType());
    os.endValue();
};

const readPreserved = function(is)
{
    //
    // For Slice classes which are marked "preserved", the implementation of this method
    // replaces the Ice.Object.prototype._iceRead method.
    //
    is.startValue();
    readImpl(this, is, this._iceMostDerivedType());
    this._iceSlicedData = is.endValue(true);
};


const Slice = Ice.Slice;

Slice.PreservedObject = function(obj)
{
    obj.prototype._iceWrite = writePreserved;
    obj.prototype._iceRead = readPreserved;
};

module.exports.Ice = Ice;
