// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

const Ice = require("../Ice/ModuleRegistry").Ice;

class Address
{
    constructor(host, port)
    {
        this.host = host;
        this.port = port;
    }
}

Ice.Address = Address;
module.exports.Ice = Ice;
