// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************
    

var Ice = require("../Ice/ModuleRegistry").Ice;
Ice.Address = function(host, port)
{
    this.host = host;
    this.port = port;
};
module.exports.Ice = Ice;
