//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

const Ice = require("../Ice/EnumBase").Ice;
/**
 *  The output mode for xxxToString method such as identityToString and proxyToString. The actual encoding format for
 *  the string is the same for all modes: you don't need to specify an encoding format or mode when reading such a
 *  string.
 **/
Ice.ToStringMode = Ice.Slice.defineEnum(
    [
        ['Unicode', 0],
        ['ASCII', 1],
        ['Compat', 2]
    ]);
module.exports.Ice = Ice;
