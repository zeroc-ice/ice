// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

const Ice = require("../Ice/EnumBase").Ice;
Ice.ToStringMode = Ice.Slice.defineEnum(
    [
        ['Unicode', 0],
        ['ASCII', 1],
        ['Compat', 2]
    ]);
module.exports.Ice = Ice;
