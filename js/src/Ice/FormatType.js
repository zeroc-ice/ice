//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

const Ice = require("../Ice/EnumBase").Ice;
Ice.FormatType = Ice.Slice.defineEnum(
    [
        ['DefaultFormat', 0],
        ['CompactFormat', 1],
        ['SlicedFormat', 2]
    ]);
module.exports.Ice = Ice;
