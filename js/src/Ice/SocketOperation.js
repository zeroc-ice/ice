//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

const Ice = require("../Ice/ModuleRegistry").Ice;
Ice.SocketOperation =
{
    None: 0,
    Read: 1,
    Write: 2,
    Connect: 2 // Same as Write
};
module.exports.Ice = Ice;
