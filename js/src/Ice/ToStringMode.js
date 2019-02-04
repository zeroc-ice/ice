//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

const Ice = require("../Ice/EnumBase").Ice;
Ice.ToStringMode = Ice.Slice.defineEnum(
    [
        ['Unicode', 0],
        ['ASCII', 1],
        ['Compat', 2]
    ]);
module.exports.Ice = Ice;
