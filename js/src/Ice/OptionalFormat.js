// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

const Ice = require("../Ice/EnumBase").Ice;
Ice.OptionalFormat = Ice.Slice.defineEnum([['F1', 0], ['F2', 1], ['F4', 2], ['F8', 3], ['Size', 4], ['VSize', 5], ['FSize', 6], ['Class', 7]]);
module.exports.Ice = Ice;
