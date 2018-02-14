// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

var Ice = require("../Ice/ModuleRegistry").Ice;
Ice.__M.require(module, ["../Ice/Class", "../Ice/TcpEndpointI"]);

var TcpEndpointI = Ice.TcpEndpointI;

var TcpEndpointFactory = Ice.Class({
    __init__: function(instance)
    {
        this._instance = instance;
    },
    type: function()
    {
        return this._instance.type();
    },
    protocol: function()
    {
        return this._instance.protocol();
    },
    create: function(args, oaEndpoint)
    {
        var e = new TcpEndpointI(this._instance);
        e.initWithOptions(args, oaEndpoint);
        return e;
    },
    read: function(s)
    {
        var e = new TcpEndpointI(this._instance);
        e.initWithStream(s);
        return e;
    },
    destroy: function()
    {
        this._instance = null;
    },
    clone:function(instance)
    {
        return new TcpEndpointFactory(instance);
    }
});

Ice.TcpEndpointFactory = TcpEndpointFactory;
module.exports.Ice = Ice;
