// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

var Ice = require("../Ice/ModuleRegistry").Ice;
Ice.__M.require(module, ["../Ice/Class", "../Ice/WSEndpoint"]);

var WSEndpoint = Ice.WSEndpoint;

var WSEndpointFactory = Ice.Class({
    __init__:function(instance, delegate)
    {
        this._instance = instance;
        this._delegate = delegate;
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
        var e = new WSEndpoint(this._instance, this._delegate.create(args, oaEndpoint));
        e.initWithOptions(args);
        return e;
    },
    read: function(s)
    {
        var e = new WSEndpoint(this._instance, this._delegate.read(s));
        e.initWithStream(s);
        return e;
    },
    destroy: function()
    {
        this._delegate.destroy();
        this._instance = null;
    }
});
Ice.WSEndpointFactory = WSEndpointFactory;
exports.Ice = Ice;