// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

const Ice = require("../Ice/EnumBase").Ice;
Ice.FormatType = Ice.Slice.defineEnum(
    [
        ['DefaultFormat', 0],
        ['CompactFormat', 1],
        ['SlicedFormat', 2]
    ]);
module.exports.Ice = Ice;
