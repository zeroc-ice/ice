//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

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
        throw new UnexpectedObjectException("expected element of type `" + expectedType + "' but received `" +
                                            type + "'", type, expectedType);
    },
    throwMemoryLimitException: function(requested, maximum)
    {
        throw new MemoryLimitException("requested " + requested + " bytes, maximum allowed is " + maximum +
                                       " bytes (see Ice.MessageSizeMax)");
    }
};
module.exports.Ice = Ice;
