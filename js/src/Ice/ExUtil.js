// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

const Ice = require("../Ice/LocalException").Ice;

//
// Local aliases.
//
const UnexpectedObjectException = Ice.UnexpectedObjectException;
const MemoryLimitException = Ice.MemoryLimitException;

//
// Exception utilities
//

Ice.ExUtil =
{
    throwUOE: function(expectedType, v)
    {
        const type = v.ice_id();
        throw new UnexpectedObjectException("expected element of type `" + expectedType + "' but received '" +
                                            type, type, expectedType);
    },
    throwMemoryLimitException: function(requested, maximum)
    {
        throw new MemoryLimitException("requested " + requested + " bytes, maximum allowed is " + maximum +
                                       " bytes (see Ice.MessageSizeMax)");
    }
};
module.exports.Ice = Ice;
