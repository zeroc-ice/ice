// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

const Ice = require("../Ice/ModuleRegistry").Ice;
Ice.SocketOperation =
{
    None: 0,
    Read: 1,
    Write: 2,
    Connect: 2 // Same as Write
};
module.exports.Ice = Ice;
