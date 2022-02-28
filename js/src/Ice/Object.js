//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

//
// Ice.Object
//
// Using IceObject in this file to avoid collisions with the native Object.
//
const Ice = require("../Ice/ModuleRegistry").Ice;

require("../Ice/Exception");
require("../Ice/FormatType");
require("../Ice/OptionalFormat");
require("../Ice/StreamHelpers");

Ice.Object = class
{
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

    //
    // _iceMostDerivedType returns the the most derived Ice generated class. This is
    // necessary because the user might extend Slice generated classes. The user
    // class extensions don't have _iceId, _iceIds, etc static members so the implementation
    // of ice_id and ice_ids would fail trying to access those members of the user
    // defined class. Instead, ice_id, ice_ids and ice_instanceof call _iceMostDerivedType
    // to get the most derived Ice class.
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

    static get _iceImplements()
    {
        return [];
    }
};

module.exports.Ice = Ice;
